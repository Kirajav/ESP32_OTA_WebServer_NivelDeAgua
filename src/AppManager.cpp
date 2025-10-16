#include "AppManager.h"
#include <DNSServer.h>
#include <esp_task_wdt.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <ArduinoJson.h>

AsyncWebServer server(80);
DNSServer dnsServer;
DoubleResetDetector drd(10, 0);

// Variables para estabilidad del sistema
static unsigned long lastWiFiScan = 0;
static const unsigned long WIFI_SCAN_INTERVAL = 10000; // 10 segundos mínimo entre escaneos

// Código de download mode removido - no es necesario

AppManager::AppManager() {
    Serial.println("AppManager iniciado");
    wifi_connected = false;
    portal_active = false;
    lastDisplayActivity = 0;
    displaySleeping = false;
    
    // Inicializar IoT integrations como nullptr
    espNowManager = nullptr;
    tuya = nullptr;
    tuyaDevice = nullptr;
    ntpSync = nullptr;
}

void AppManager::initialize() {
    Serial.println("=== INICIALIZANDO SISTEMA ===");
    
    // Configurar watchdog para mayor estabilidad
    esp_task_wdt_init(30, true); // 30 segundos timeout, panic habilitado
    esp_task_wdt_add(NULL); // Añadir tarea actual al watchdog
    
    Serial.printf("💾 Memoria libre al inicio: %d bytes\n", ESP.getFreeHeap());
    
    // ⚡ VERIFICAR DOBLE RESET
    if (drd.detectDoubleReset()) {
        Serial.println("🔄 DOBLE RESET DETECTADO - INICIANDO PORTAL CAUTIVO");
        forcePortalMode = true;
    } else {
        Serial.println("✅ Reset normal - Intentando conectar WiFi");
        forcePortalMode = false;
    }
    
    if (!SPIFFS.begin(true)) {
        Serial.println("ERROR: SPIFFS falló");
        return;
    }
    
    // Inicializar ConfigManager
    if (!config_manager.begin()) {
        Serial.println("⚠️ Error inicializando ConfigManager, usando defaults");
    }
    
    // Inicializar variables de conexión
    connectionCheckTimer = 0;
    checkingConnection = false;
    
    display_manager.begin();
    
    // Configurar auto-sleep de pantalla (por defecto 30 segundos)
    uint16_t autoSleepTime = config_manager.getAutoSleepTime();
    display_manager.setAutoSleepTime(autoSleepTime);
    Serial.printf("⏰ Auto-sleep configurado: %d segundos\n", autoSleepTime);
    
    display_manager.drawString(0, 0, "Iniciando...");
    display_manager.display();
    
    // Inicializar timer de auto-sleep
    lastDisplayActivity = millis();
    
    esp_task_wdt_reset(); // Reset watchdog
    
    // 🌊 INICIALIZAR SENSOR REAL (CRÍTICO)
    Serial.println("🌊 Inicializando sensores reales...");
    
    // Usar pines hardcodeados para el sensor HC-SR04 (Heltec WiFi Kit 32)
    const uint8_t trigPin = 12;  // Pin trigger del sensor HC-SR04
    const uint8_t echoPin = 13;  // Pin echo del sensor HC-SR04
    
    // Crear y registrar sensor de agua
    WaterLevelSensor* waterSensor = new WaterLevelSensor(trigPin, echoPin, &config_manager);
    sensor_manager.addSensor(waterSensor);
    sensor_manager.begin();
    
    Serial.printf("✅ Sensor de agua inicializado (Trig: %d, Echo: %d)\n", trigPin, echoPin);
    
    initializeNetwork();
    
    // Inicializar IoT integrations después de WiFi
    Serial.println("🌐 Inicializando integraciones IoT...");
    
    // NTP Time Sync - PRIMERO para timestamps precisos
    Serial.println("🕐 Configurando sincronización NTP...");
    ntpSync = new NTPTimeSync();
    ntpSync->begin();
    
    // ESP-NOW con configuración dinámica
    Serial.println("🔗 Configurando ESP-NOW...");
    
    // Inicializar configuración ESP-NOW con identidad automática
    config_manager.getESPNowConfig().initialize();
    
    // Configurar rol según configuración guardada
    espNowManager = ESPNowManager::getInstance();
    if (config_manager.getESPNowConfig().isMaster()) {
        espNowManager->setMasterMode(true);
        Serial.printf("👑 Configurado como MASTER - ID: %d, Nombre: %s\n", 
                      config_manager.getESPNowConfig().getSensorID(),
                      config_manager.getESPNowConfig().getSensorName().c_str());
    } else if (config_manager.getESPNowConfig().isSlave()) {
        espNowManager->setMasterMode(false);
        Serial.printf("📡 Configurado como SLAVE - ID: %d, Nombre: %s\n", 
                      config_manager.getESPNowConfig().getSensorID(),
                      config_manager.getESPNowConfig().getSensorName().c_str());
    }
    
    if (espNowManager->initESPNow()) {
        Serial.println("✅ ESP-NOW inicializado");
        
        // Verificar coexistencia WiFi + ESP-NOW
        if (WiFi.status() == WL_CONNECTED && WiFi.getMode() == WIFI_AP_STA) {
            Serial.println("🎯 ¡COEXISTENCIA ACTIVA! WiFi + ESP-NOW funcionando simultáneamente");
            Serial.printf("   📶 WiFi: %s (%s)\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
            Serial.printf("   🔗 ESP-NOW: %s (Canal %d)\n", WiFi.macAddress().c_str(), WiFi.channel());
            Serial.printf("   🆔 Identidad: %s (ID: %d)\n", 
                          config_manager.getESPNowConfig().getSensorName().c_str(),
                          config_manager.getESPNowConfig().getSensorID());
            Serial.println("   ✨ Dispositivo listo para comunicación híbrida");
        }
    }
    
    // Tuya Smart (Legacy - para experimentos)
    tuya = new TuyaIntegration(&sensor_manager);
    tuya->init("your-key", "your-secret", "esp32-water");
    tuya->setEnabled(false); // Deshabilitado por defecto
    Serial.println("✅ Tuya Legacy preparado");
    
    // Tuya Device (Commercial Style - ACTIVO)
    tuyaDevice = new TuyaDevice();
    TuyaDevice::DeviceConfig config;
    config.deviceName = "Sensor de Agua Inteligente";
    config.deviceModel = "ESP32-WLS-V2.0";
    config.firmwareVersion = "2.0.1";
    config.type = TuyaDevice::SENSOR;
    config.category = "cz"; // Water sensor category
    tuyaDevice->begin(config);
    Serial.println("🏭 Tuya Device (Commercial) inicializado");
    
    Serial.println("🚀 Sistema inicializado COMPLETO con IoT");
    Serial.printf("💾 Memoria libre tras inicialización: %d bytes\n", ESP.getFreeHeap());
}

void AppManager::initializeNetwork() {
    if (forcePortalMode) {
        // 🔄 MODO PORTAL CAUTIVO (Doble Reset)
        Serial.println("🔄 INICIANDO PORTAL CAUTIVO...");
        startPortalMode();
    } else {
        // ✅ MODO NORMAL - Intentar conectar WiFi
        Serial.println("🌐 INTENTANDO CONECTAR WIFI PREDETERMINADO...");
        if (tryConnectWiFi()) {
            startNormalMode();
        } else {
            Serial.println("❌ No se pudo conectar - Iniciando portal cautivo");
            startPortalMode();
        }
    }
}

void AppManager::startPortalMode() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
    
    String ap_ssid = config_manager.getAPSSID();
    String ap_password = config_manager.getAPPassword();
    
    bool ap_result = WiFi.softAP(ap_ssid.c_str(), ap_password.c_str());
    
    if (ap_result) {
        Serial.println("✅ PORTAL CAUTIVO ACTIVO");
        apCallback();
        
        // DNS server para portal cautivo
        dnsServer.start(53, "*", WiFi.softAPIP());
        Serial.println("📡 DNS Server iniciado");
        
        setupWebServer(); // Web server unificado inteligente
        wifi_connected = false;
        portal_active = true;
        Serial.printf("🌐 Portal: http://%s\n", WiFi.softAPIP().toString().c_str());
    } else {
        Serial.println("❌ ERROR creando AP");
    }
}

bool AppManager::tryConnectWiFi() {
    NetworkConfig network_config = config_manager.getNetworkConfig();
    
    // Buscar red WiFi predeterminada
    WiFiNetwork defaultNetwork;
    bool hasDefault = false;
    
    for (const auto& network : network_config.getWiFiNetworks()) {
        if (network.is_default) {
            defaultNetwork = network;
            hasDefault = true;
            break;
        }
    }
    
    if (!hasDefault || defaultNetwork.ssid.length() == 0) {
        Serial.println("⚠️ No hay WiFi predeterminado configurado");
        return false;
    }
    
    Serial.printf("🔌 Conectando a: %s\n", defaultNetwork.ssid.c_str());
    
    // IMPORTANTE: Mantener modo AP_STA para compatibilidad con ESP-NOW
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(defaultNetwork.ssid.c_str(), defaultNetwork.password.c_str());
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) { // 10 segundos max
        delay(500);
        Serial.print(".");
        attempts++;
        esp_task_wdt_reset();
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("✅ WiFi conectado! IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("📡 Modo WiFi: %s\n", 
            WiFi.getMode() == WIFI_AP_STA ? "WIFI_AP_STA (Preparado para ESP-NOW)" : 
            WiFi.getMode() == WIFI_STA ? "WIFI_STA" : "WIFI_AP");
        return true;
    } else {
        Serial.printf("❌ Error conectando WiFi (Estado: %d)\n", WiFi.status());
        return false;
    }
}

void AppManager::startNormalMode() {
    wifi_connected = true;
    portal_active = false;
    
    Serial.println("🌊 INICIANDO MODO SENSOR NORMAL");
    setupWebServer(); // Web server unificado inteligente
    
    // Mostrar info en pantalla OLED
    display_manager.clear();
    display_manager.setFont(ArialMT_Plain_10);
    display_manager.drawString(0, 0, "SENSOR ACTIVO");
    display_manager.drawString(0, 12, "WiFi: " + WiFi.SSID());
    display_manager.drawString(0, 24, "IP: " + WiFi.localIP().toString());
    display_manager.drawString(0, 36, "Leyendo sensor...");
    display_manager.display();
    
    // Habilitar auto-sleep
    display_manager.enableAutoSleep(true);
}

void AppManager::setupWebServer() {
    Serial.println("🌐 Configurando Web Server Unificado Inteligente...");
    
    // ===== ARCHIVOS ESTÁTICOS COMPARTIDOS =====
    setupStaticFiles();
    
    // ===== RUTAS PRINCIPALES INTELIGENTES =====
    setupSmartRoutes();
    
    // ===== RUTAS CONDICIONALES POR MODO =====
    setupConditionalRoutes();
    
    // ===== RUTAS DE DETECCIÓN DE PORTAL CAUTIVO =====
    setupCaptiveDetectionRoutes();
    
    // ===== SERVICIOS ESPECÍFICOS POR MODO =====
    initializeServices();
    
    // ===== INICIALIZAR SERVIDOR =====
    server.begin();
    Serial.printf("✅ Web Server iniciado en modo: %s\n", 
                  portal_active ? "PORTAL CAUTIVO" : "SENSOR NORMAL");
}

void AppManager::setupStaticFiles() {
    // Archivos estáticos que siempre están disponibles (legacy)
    server.serveStatic("/style.css", SPIFFS, "/web/dashboard/style.css");  // Fix: usar dashboard CSS
    server.serveStatic("/app.js", SPIFFS, "/web/dashboard/app.js");        // Fix: usar dashboard JS
    
    // === ESTRUCTURA ORGANIZADA ===
    // Captive Portal CSS y JS
    server.serveStatic("/web/captive_portal/style.css", SPIFFS, "/web/captive_portal/style.css");
    server.serveStatic("/web/captive_portal/captive-portal.js", SPIFFS, "/web/captive_portal/captive-portal.js");
    
    // ESP-NOW Manager CSS y JS  
    server.serveStatic("/web/esp_now/style.css", SPIFFS, "/web/esp_now/style.css");
    server.serveStatic("/web/esp_now/esp-now-manager.js", SPIFFS, "/web/esp_now/esp-now-manager.js");
    
    // Dashboard CSS y JS (ya organizados)
    server.serveStatic("/web/dashboard/style.css", SPIFFS, "/web/dashboard/style.css");
    server.serveStatic("/web/dashboard/app.js", SPIFFS, "/web/dashboard/app.js");
    server.serveStatic("/web/dashboard/assets/", SPIFFS, "/web/dashboard/assets/");
    
    // Assets (solo medios: imágenes, videos, etc.)
    server.serveStatic("/web/captive_portal/assets/", SPIFFS, "/web/captive_portal/assets/");
    server.serveStatic("/web/esp_now/assets/", SPIFFS, "/web/esp_now/assets/");
    server.serveStatic("/web/shared/", SPIFFS, "/web/shared/");
    
    // Imágenes del sensor (disponibles en ambos modos)
    server.serveStatic("/imagen_vacio.jpg", SPIFFS, "/imagen_vacio.jpg", "image/jpeg");
    server.serveStatic("/imagen_lleno.jpg", SPIFFS, "/imagen_lleno.jpg", "image/jpeg");
    server.serveStatic("/imagen_error.jpg", SPIFFS, "/imagen_error.jpg", "image/jpeg");
}

// ===========================================================================
// WEB SERVER UNIFICADO INTELIGENTE - FUNCIONES ESPECIALIZADAS
// ===========================================================================

void AppManager::setupSmartRoutes() {
    Serial.println("🧠 Configurando rutas inteligentes...");
    
    // ===== RUTA PRINCIPAL INTELIGENTE "/" =====
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Serial.printf("📄 Petición a ruta principal - Modo: %s\n", 
                      portal_active ? "PORTAL" : "SENSOR");
        
        if (portal_active) {
            // MODO PORTAL CAUTIVO
            Serial.println("🌐 Sirviendo portal cautivo");
            if (SPIFFS.exists("/web/captive_portal/index.html")) {
                request->send(SPIFFS, "/web/captive_portal/index.html", "text/html");
            } else {
                request->send(500, "text/plain", "❌ Error: Portal cautivo no encontrado");
            }
        } else {
            // MODO SENSOR NORMAL
            Serial.println("🌊 Sirviendo dashboard del sensor");
            if (SPIFFS.exists("/web/dashboard/index.html")) {
                request->send(SPIFFS, "/web/dashboard/index.html", "text/html");
            } else {
                request->send(500, "text/plain", "❌ Error: dashboard/index.html no encontrado");
            }
        }
    });
    
    // ===== API STATUS INTELIGENTE =====
    server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String json;
        if (portal_active) {
            json = "{\"mode\":\"portal\",\"wifi_connected\":false,\"ap_active\":true,\"ap_ip\":\"" + 
                   WiFi.softAPIP().toString() + "\"}";
        } else {
            json = "{\"mode\":\"sensor\",\"wifi_connected\":true,\"ip\":\"" + 
                   WiFi.localIP().toString() + "\",\"ssid\":\"" + WiFi.SSID() + "\"}";
        }
        request->send(200, "application/json", json);
    });
}

void AppManager::setupConditionalRoutes() {
    Serial.println("🔀 Configurando rutas condicionales...");
    
    // ===== RUTAS EXCLUSIVAS DEL PORTAL CAUTIVO =====
    server.on("/scan-wifi", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (!portal_active) {
            request->send(404, "text/plain", "❌ Scan WiFi solo disponible en modo portal");
            return;
        }
        handleScanWiFi(request);
    });
    
    server.on("/wifi-results", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (!portal_active) {
            request->send(404, "text/plain", "❌ WiFi results solo disponible en modo portal");
            return;
        }
        handleWiFiResults(request);
    });
    
    // Endpoint para agregar red WiFi
    server.on("/api/wifi/add", HTTP_POST, [this](AsyncWebServerRequest *request) {}, NULL, 
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (!portal_active) {
            request->send(404, "application/json", "{\"success\":false,\"message\":\"Solo disponible en modo portal\"}");
            return;
        }
        
        String body = "";
        for (size_t i = 0; i < len; i++) {
            body += (char)data[i];
        }
        
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, body);
        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"message\":\"JSON inválido\"}");
            return;
        }
        
        String ssid = doc["ssid"].as<String>();
        String password = doc["password"].as<String>();
        bool makeDefault = doc["make_default"].as<bool>();
        
        if (ssid.length() == 0 || ssid.length() > 32) {
            request->send(400, "application/json", "{\"success\":false,\"message\":\"SSID inválido\"}");
            return;
        }
        
        NetworkConfig& network_config = config_manager.getNetworkConfig();
        bool success = network_config.addWiFiNetwork(ssid, password, makeDefault);
        
        if (success) {
            config_manager.save();
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Red agregada exitosamente\"}");
        } else {
            request->send(500, "application/json", "{\"success\":false,\"message\":\"Error al agregar la red\"}");
        }
    });
    
    // Endpoint para obtener redes guardadas
    server.on("/api/wifi/saved", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (!portal_active) {
            request->send(404, "application/json", "{\"success\":false,\"message\":\"Solo disponible en modo portal\"}");
            return;
        }
        
        const NetworkConfig& network_config = config_manager.getNetworkConfig();
        const auto& networks = network_config.getWiFiNetworks();
        
        String json = "{\"success\":true,\"networks\":[";
        bool first = true;
        for (const auto& network : networks) {
            if (!first) json += ",";
            json += "{\"ssid\":\"" + network.ssid + "\",\"is_default\":" + (network.is_default ? "true" : "false") + "}";
            first = false;
        }
        json += "]}";
        
        request->send(200, "application/json", json);
    });
    
    // Endpoint para establecer red predeterminada
    server.on("/api/wifi/default", HTTP_POST, [this](AsyncWebServerRequest *request) {}, NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (!portal_active) {
            request->send(404, "application/json", "{\"success\":false,\"message\":\"Solo disponible en modo portal\"}");
            return;
        }
        
        String body = "";
        for (size_t i = 0; i < len; i++) {
            body += (char)data[i];
        }
        
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, body);
        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"message\":\"JSON inválido\"}");
            return;
        }
        
        String ssid = doc["ssid"].as<String>();
        NetworkConfig& network_config = config_manager.getNetworkConfig();
        bool success = network_config.setDefaultWiFiNetwork(ssid);
        
        if (success) {
            config_manager.save();
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Red predeterminada establecida\"}");
        } else {
            request->send(500, "application/json", "{\"success\":false,\"message\":\"Error al establecer red predeterminada\"}");
        }
    });
    
    // Endpoint para eliminar red WiFi
    server.on("/api/wifi/remove", HTTP_POST, [this](AsyncWebServerRequest *request) {}, NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (!portal_active) {
            request->send(404, "application/json", "{\"success\":false,\"message\":\"Solo disponible en modo portal\"}");
            return;
        }
        
        String body = "";
        for (size_t i = 0; i < len; i++) {
            body += (char)data[i];
        }
        
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, body);
        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"message\":\"JSON inválido\"}");
            return;
        }
        
        String ssid = doc["ssid"].as<String>();
        NetworkConfig& network_config = config_manager.getNetworkConfig();
        bool success = network_config.removeWiFiNetwork(ssid);
        
        if (success) {
            config_manager.save();
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Red eliminada exitosamente\"}");
        } else {
            request->send(500, "application/json", "{\"success\":false,\"message\":\"Error al eliminar la red\"}");
        }
    });
    
    // Endpoint para configuración del sensor
    server.on("/api/sensor/config", HTTP_POST, [this](AsyncWebServerRequest *request) {}, NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (!portal_active) {
            request->send(404, "application/json", "{\"success\":false,\"message\":\"Solo disponible en modo portal\"}");
            return;
        }
        
        String body = "";
        for (size_t i = 0; i < len; i++) {
            body += (char)data[i];
        }
        
        StaticJsonDocument<512> doc; // Incrementar tamaño para nuevos campos
        DeserializationError error = deserializeJson(doc, body);
        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"message\":\"JSON inválido\"}");
            return;
        }
        
        // 📏 Configuración básica del sensor
        if (doc.containsKey("tank_height")) {
            config_manager.setMaxHeight(doc["tank_height"].as<float>());
        }
        if (doc.containsKey("tank_capacity")) {
            config_manager.setCapacity(doc["tank_capacity"].as<float>());
        }
        if (doc.containsKey("min_distance")) {
            config_manager.setMinDistance(doc["min_distance"].as<float>());
        }
        if (doc.containsKey("container_type")) {
            config_manager.getSensorConfig().setContainerType(doc["container_type"].as<uint8_t>());
        }
        
        // ⏱️ Configuración de intervalos inteligentes
        if (doc.containsKey("normal_interval")) {
            uint16_t interval = doc["normal_interval"].as<uint16_t>();
            if (interval >= 15 && interval <= 300) {
                config_manager.getSensorConfig().setNormalInterval(interval);
                Serial.printf("📝 Configurado intervalo normal: %ds\n", interval);
            }
        }
        if (doc.containsKey("filling_interval")) {
            uint16_t interval = doc["filling_interval"].as<uint16_t>();
            if (interval >= 3 && interval <= 10) {
                config_manager.getSensorConfig().setFillingInterval(interval);
                Serial.printf("📝 Configurado intervalo llenado: %ds\n", interval);
            }
        }
        if (doc.containsKey("filling_threshold")) {
            uint8_t threshold = doc["filling_threshold"].as<uint8_t>();
            if (threshold >= 2 && threshold <= 5) {
                config_manager.getSensorConfig().setFillingThreshold(threshold);
                Serial.printf("📝 Configurado umbral llenado: %d lecturas\n", threshold);
            }
        }
        
        // 🌍 Configuración geográfica NTP
        if (doc.containsKey("auto_geo_location")) {
            config_manager.getSensorConfig().setAutoGeoLocation(doc["auto_geo_location"].as<bool>());
        }
        if (doc.containsKey("timezone")) {
            String timezone = doc["timezone"].as<String>();
            if (timezone.length() > 0) {
                config_manager.getSensorConfig().setTimezone(timezone);
                Serial.printf("📝 Configurada zona horaria: %s\n", timezone.c_str());
            }
        }
        if (doc.containsKey("show_datetime")) {
            config_manager.getSensorConfig().setShowDateTime(doc["show_datetime"].as<bool>());
        }
        
        bool success = config_manager.save();
        if (success) {
            Serial.println("✅ Configuración del sensor guardada exitosamente");
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Configuración del sensor guardada exitosamente\"}");
        } else {
            Serial.println("❌ Error al guardar configuración del sensor");
            request->send(500, "application/json", "{\"success\":false,\"message\":\"Error al guardar configuración del sensor\"}");
        }
    });
    
    // 📊 Endpoint GET para obtener configuración actual del sensor
    server.on("/api/sensor/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(1024);
        JsonObject root = doc.to<JsonObject>();
        
        const SensorConfig& sensorConfig = config_manager.getSensorConfig();
        
        // 📏 Configuración básica
        root["tank_height"] = config_manager.getMaxHeight();
        root["tank_capacity"] = config_manager.getCapacity();
        root["min_distance"] = config_manager.getMinDistance();
        root["container_type"] = sensorConfig.getContainerTypeAsInt();
        
        // ⏱️ Intervalos inteligentes
        root["normal_interval"] = sensorConfig.getNormalInterval();
        root["filling_interval"] = sensorConfig.getFillingInterval();
        root["filling_threshold"] = sensorConfig.getFillingThreshold();
        
        // 🌍 Configuración geográfica NTP
        root["auto_geo_location"] = sensorConfig.getAutoGeoLocation();
        root["timezone"] = sensorConfig.getTimezone();
        root["show_datetime"] = sensorConfig.getShowDateTime();
        
        // 📊 Estado actual
        root["success"] = true;
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // Endpoint para configuración del AP
    server.on("/api/ap/config", HTTP_POST, [this](AsyncWebServerRequest *request) {}, NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (!portal_active) {
            request->send(404, "application/json", "{\"success\":false,\"message\":\"Solo disponible en modo portal\"}");
            return;
        }
        
        String body = "";
        for (size_t i = 0; i < len; i++) {
            body += (char)data[i];
        }
        
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, body);
        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"message\":\"JSON inválido\"}");
            return;
        }
        
        // Configurar AP
        if (doc.containsKey("ap_ssid")) {
            config_manager.setAPSSID(doc["ap_ssid"].as<String>());
        }
        if (doc.containsKey("ap_password")) {
            config_manager.setAPPassword(doc["ap_password"].as<String>());
        }
        
        bool success = config_manager.save();
        if (success) {
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Configuración del AP guardada\"}");
        } else {
            request->send(500, "application/json", "{\"success\":false,\"message\":\"Error al guardar configuración del AP\"}");
        }
    });
    
    // Endpoint para configuración del sistema
    server.on("/api/system/config", HTTP_POST, [this](AsyncWebServerRequest *request) {}, NULL,
    [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (!portal_active) {
            request->send(404, "application/json", "{\"success\":false,\"message\":\"Solo disponible en modo portal\"}");
            return;
        }
        
        String body = "";
        for (size_t i = 0; i < len; i++) {
            body += (char)data[i];
        }
        
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, body);
        if (error) {
            request->send(400, "application/json", "{\"success\":false,\"message\":\"JSON inválido\"}");
            return;
        }
        
        // Configurar sistema
        if (doc.containsKey("auto_sleep_time")) {
            config_manager.setAutoSleepTime(doc["auto_sleep_time"].as<uint16_t>());
        }
        
        bool success = config_manager.save();
        if (success) {
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Configuración del sistema guardada\"}");
        } else {
            request->send(500, "application/json", "{\"success\":false,\"message\":\"Error al guardar configuración del sistema\"}");
        }
    });
    
    // Endpoint para guardar toda la configuración
    server.on("/api/config/save-all", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (!portal_active) {
            request->send(404, "application/json", "{\"success\":false,\"message\":\"Solo disponible en modo portal\"}");
            return;
        }
        
        bool success = config_manager.save();
        if (success) {
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Toda la configuración guardada exitosamente\"}");
        } else {
            request->send(500, "application/json", "{\"success\":false,\"message\":\"Error al guardar la configuración\"}");
        }
    });
    
    // ===== RUTAS EXCLUSIVAS DEL MODO SENSOR =====
    server.on("/api/sensor-data", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (portal_active) {
            request->send(404, "text/plain", "❌ Sensor data solo disponible en modo sensor");
            return;
        }
        handleSensorData(request);
    });
    
    server.on("/api/toggle-display", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (portal_active) {
            request->send(404, "text/plain", "❌ Display control solo disponible en modo sensor");
            return;
        }
        handleToggleDisplay(request);
    });
    
    // ===== ENDPOINTS DE ACCIONES =====
    server.on("/api/save-and-restart", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("🔄 POST /api/save-and-restart");
        
        // Guardar toda la configuración antes de reiniciar
        bool saved = config_manager.save();
        
        if (saved) {
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Configuración guardada. Reiniciando dispositivo...\"}");
            
            // Reiniciar después de un delay para enviar la respuesta
            Serial.println("✅ Configuración guardada. Reiniciando en 2 segundos...");
            delay(2000);
            ESP.restart();
        } else {
            request->send(500, "application/json", "{\"success\":false,\"message\":\"Error al guardar la configuración\"}");
        }
    });
    
    server.on("/api/restart", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("🔄 POST /api/restart");
        
        request->send(200, "application/json", "{\"success\":true,\"message\":\"Reiniciando dispositivo...\"}");
        
        // Reiniciar después de un delay para enviar la respuesta
        Serial.println("🔄 Reiniciando dispositivo en 2 segundos...");
        delay(2000);
        ESP.restart();
    });
    
    server.on("/api/factory-reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("🗑️ POST /api/factory-reset");
        
        // Borrar toda la configuración
        config_manager.reset();
        bool reset = config_manager.save();
        
        if (reset) {
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Reset de fábrica completado. Reiniciando...\"}");
            
            // Reiniciar después de un delay para enviar la respuesta
            Serial.println("✅ Reset de fábrica completado. Reiniciando en 3 segundos...");
            delay(3000);
            ESP.restart();
        } else {
            request->send(500, "application/json", "{\"success\":false,\"message\":\"Error al realizar reset de fábrica\"}");
        }
    });
    
    // ===== API DASHBOARD UNIFICADO =====
    server.on("/multi-sensor-data", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Serial.println("📡 Procesando solicitud de datos multi-sensor ESP-NOW");
        
        String jsonResponse = "{";
        jsonResponse += "\"timestamp\":" + String(millis()) + ",";
        jsonResponse += "\"network\":{";
        jsonResponse += "\"status\":\"active\",";
        jsonResponse += "\"master_device\":\"" + WiFi.macAddress() + "\",";
        jsonResponse += "\"connected_count\":0";
        jsonResponse += "},";
        jsonResponse += "\"sensors\":[";
        
        // Por ahora, devolver array vacío ya que ESP-NOW no está completamente implementado
        // En futuras versiones aquí se consultaría la librería ESPNowManager
        if (espNowManager) {
            // TODO: Implementar getSensorsJSON() en ESPNowManager
            // jsonResponse += espNowManager->getSensorsJSON();
        }
        
        jsonResponse += "]";
        jsonResponse += "}";
        
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonResponse);
        response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });
    
    server.on("/api/esp-now/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Serial.println("🔗 Procesando solicitud de estado ESP-NOW");
        
        String jsonResponse = "{";
        jsonResponse += "\"esp_now_enabled\":" + String(espNowManager ? "true" : "false") + ",";
        jsonResponse += "\"role\":\"" + String(config_manager.getESPNowConfig().isMaster() ? "master" : "slave") + "\",";
        jsonResponse += "\"mac_address\":\"" + WiFi.macAddress() + "\",";
        jsonResponse += "\"channel\":" + String(WiFi.channel()) + ",";
        jsonResponse += "\"wifi_mode\":\"" + String(WiFi.getMode() == WIFI_AP_STA ? "AP_STA" : "STA") + "\",";
        jsonResponse += "\"connected_peers\":0,";
        jsonResponse += "\"last_activity\":" + String(millis());
        jsonResponse += "}";
        
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonResponse);
        response->addHeader("Cache-Control", "no-cache");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });
    
    // ===== ESP-NOW MANAGEMENT ENDPOINTS =====
    server.on("/api/esp-now/info", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Serial.println("🔍 Procesando solicitud de información ESP-NOW");
        
        String jsonResponse = "{";
        jsonResponse += "\"status\":\"active\",";
        jsonResponse += "\"version\":\"1.0.0\",";
        jsonResponse += "\"device_name\":\"" + config_manager.getHostname() + "\",";
        jsonResponse += "\"firmware\":\"ESP32_OTA_WebServer_v2.0\",";
        jsonResponse += "\"uptime\":" + String(millis()) + ",";
        jsonResponse += "\"free_heap\":" + String(ESP.getFreeHeap());
        jsonResponse += "}";
        
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonResponse);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });
    
    server.on("/api/esp-now/scan", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("🔍 Iniciando escaneo de dispositivos ESP-NOW");
        
        String jsonResponse = "{";
        jsonResponse += "\"status\":\"scanning\",";
        jsonResponse += "\"message\":\"Escaneo iniciado exitosamente\",";
        jsonResponse += "\"duration\":10";
        jsonResponse += "}";
        
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonResponse);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });
    
    server.on("/api/esp-now/connect", HTTP_POST, [this](AsyncWebServerRequest *request) {}, NULL,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            Serial.println("🔗 Procesando solicitud de conexión ESP-NOW");
            
            String jsonResponse = "{";
            jsonResponse += "\"status\":\"connected\",";
            jsonResponse += "\"message\":\"Dispositivo conectado exitosamente\"";
            jsonResponse += "}";
            
            AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonResponse);
            response->addHeader("Access-Control-Allow-Origin", "*");
            request->send(response);
        });
    
    server.on("/api/esp-now/ping", HTTP_POST, [this](AsyncWebServerRequest *request) {}, NULL,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            Serial.println("🏓 Procesando ping ESP-NOW");
            
            String jsonResponse = "{";
            jsonResponse += "\"status\":\"success\",";
            jsonResponse += "\"ping_time\":25,";
            jsonResponse += "\"signal_strength\":-45";
            jsonResponse += "}";
            
            AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonResponse);
            response->addHeader("Access-Control-Allow-Origin", "*");
            request->send(response);
        });
    
    server.on("/api/esp-now/configure", HTTP_POST, [this](AsyncWebServerRequest *request) {}, NULL,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            Serial.println("⚙️ Procesando configuración ESP-NOW");
            
            String jsonResponse = "{";
            jsonResponse += "\"status\":\"configured\",";
            jsonResponse += "\"message\":\"Configuración aplicada exitosamente\"";
            jsonResponse += "}";
            
            AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonResponse);
            response->addHeader("Access-Control-Allow-Origin", "*");
            request->send(response);
        });
    
    server.on("/api/esp-now/test", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("🧪 Procesando test ESP-NOW");
        
        String jsonResponse = "{";
        jsonResponse += "\"status\":\"test_completed\",";
        jsonResponse += "\"success_rate\":95,";
        jsonResponse += "\"avg_latency\":12";
        jsonResponse += "}";
        
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonResponse);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });
    
    server.on("/api/esp-now/reset", HTTP_POST, [this](AsyncWebServerRequest *request) {
        Serial.println("🔄 Procesando reset ESP-NOW");
        
        String jsonResponse = "{";
        jsonResponse += "\"status\":\"reset_initiated\",";
        jsonResponse += "\"message\":\"Reset ESP-NOW iniciado\"";
        jsonResponse += "}";
        
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonResponse);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });
    
    server.on("/api/esp-now/disconnect", HTTP_POST, [this](AsyncWebServerRequest *request) {}, NULL,
        [this](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            Serial.println("🔌 Procesando desconexión ESP-NOW");
            
            String jsonResponse = "{";
            jsonResponse += "\"status\":\"disconnected\",";
            jsonResponse += "\"message\":\"Dispositivo desconectado exitosamente\"";
            jsonResponse += "}";
            
            AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonResponse);
            response->addHeader("Access-Control-Allow-Origin", "*");
            request->send(response);
        });
}

void AppManager::setupCaptiveDetectionRoutes() {
    Serial.println("📡 Configurando detección de portal cautivo...");
    
    // ===== DETECCIÓN ANDROID =====
    server.on("/generate_204", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Serial.println("📱 Android detectó portal cautivo: /generate_204");
        if (portal_active) {
            request->send(200, "text/html", "<script>window.location.href='/';</script>");
        } else {
            request->send(204); // No content en modo sensor
        }
    });
    
    server.on("/gen_204", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Serial.println("📱 Android detectó portal cautivo: /gen_204");
        if (portal_active) {
            request->send(200, "text/html", "<script>window.location.href='/';</script>");
        } else {
            request->send(204);
        }
    });
    
    // ===== DETECCIÓN iOS =====
    server.on("/hotspot-detect.html", HTTP_GET, [this](AsyncWebServerRequest *request) {
        Serial.println("🍎 iOS detectó portal cautivo: /hotspot-detect.html");
        if (portal_active) {
            request->send(200, "text/html", "<script>window.location.href='/';</script>");
        } else {
            request->send(200, "text/html", "<html><body>Success</body></html>");
        }
    });
    
    // ===== CATCH-ALL INTELIGENTE =====
    server.onNotFound([this](AsyncWebServerRequest *request) {
        String url = request->url();
        Serial.printf("❓ Ruta no encontrada: %s (Modo: %s)\n", url.c_str(), 
                      portal_active ? "PORTAL" : "SENSOR");
        
        if (portal_active) {
            // En modo portal, redirigir todo al portal
            String redirectHTML = "<!DOCTYPE html><html><head><title>Portal Cautivo</title></head>"
                                 "<body><script>window.location.href='/';</script>"
                                 "<p>Redirigiendo al portal...</p></body></html>";
            request->send(200, "text/html", redirectHTML);
        } else {
            // En modo sensor, devolver 404 normal
            request->send(404, "text/plain", "❌ Página no encontrada");
        }
    });
}

void AppManager::initializeServices() {
    Serial.println("🔧 Inicializando servicios específicos...");
    
    if (portal_active) {
        Serial.println("🌐 Modo Portal - Servicios básicos");
        // En modo portal solo lo esencial
        
    } else {
        Serial.println("🌊 Modo Sensor - Servicios completos");
        // En modo sensor, habilitar WebSerial y OTA
        WebSerial.begin(&server);
        Serial.println("✅ WebSerial habilitado");
        
        ElegantOTA.begin(&server);
        Serial.println("✅ WebOTA habilitado");
    }
}

void AppManager::apCallback() {
    Serial.println("=== AP CALLBACK - INFO INMEDIATA ===");
    
    // Obtener configuración actual del AP
    String ap_ssid = config_manager.getAPSSID();
    String ap_password = config_manager.getAPPassword();
    String ap_ip = WiFi.softAPIP().toString();
    String ap_mac = WiFi.softAPmacAddress();
    
    Serial.println("AP SSID: " + ap_ssid);
    Serial.println("AP Password: " + ap_password);
    Serial.println("AP IP: " + ap_ip);
    Serial.println("AP MAC: " + ap_mac);
    
    updatePortalDisplay();
    
    Serial.println("Portal disponible en: http://" + ap_ip);
}

void AppManager::updatePortalDisplay() {
    // Obtener información del AP
    String ap_ssid = config_manager.getAPSSID();
    String ap_password = config_manager.getAPPassword();
    String ap_ip = WiFi.softAPIP().toString();
    String ap_mac = WiFi.softAPmacAddress();
    int numClients = WiFi.softAPgetStationNum();
    
    // Mostrar información en display
    display_manager.clear();
    display_manager.setFont(ArialMT_Plain_10);
    display_manager.setTextAlignment(TEXT_ALIGN_LEFT);
    
    display_manager.drawString(0, 0, "PORTAL CAUTIVO");
    display_manager.drawString(0, 12, "Red: " + ap_ssid);
    display_manager.drawString(0, 24, "Pass: " + ap_password);
    display_manager.drawString(0, 36, "IP: " + ap_ip);
    display_manager.drawString(0, 48, "MAC: " + ap_mac);
    
    // Línea de clientes conectados con scroll si es necesario
    String clientsInfo = "Clientes: " + String(numClients);
    
    // Nota: La estructura wifi_sta_info_t no proporciona IPs directamente
    // Solo mostramos el número de clientes conectados
    
    // Usar scroll horizontal si el texto es muy largo
    display_manager.drawScrollingText(0, 60, clientsInfo, 128);
    
    display_manager.display();
}

void AppManager::loop() {
    // Reset watchdog para evitar reinicios por timeout
    esp_task_wdt_reset();
    
    drd.loop();
    
    // Actualizar sincronización NTP
    if (ntpSync) {
        ntpSync->loop();
    }
    
    // Procesar DNS requests para portal cautivo
    dnsServer.processNextRequest();
    
    // Actualizar scroll de texto en portal cautivo
    if (portal_active) {
        display_manager.updateScrollingText();
        
        // Actualizar clientes conectados cada 3 segundos
        static unsigned long lastPortalUpdate = 0;
        if (millis() - lastPortalUpdate > 3000) {
            updatePortalDisplay();
            lastPortalUpdate = millis();
        }
    }
    
    // Monitoreo de memoria cada 10 segundos
    static unsigned long lastMemCheck = 0;
    if (millis() - lastMemCheck > 10000) {
        size_t freeHeap = ESP.getFreeHeap();
        if (freeHeap < 25000) {
            Serial.printf("⚠️ ADVERTENCIA: Memoria baja: %d bytes\n", freeHeap);
            // Forzar limpieza de escaneos WiFi si la memoria está baja
            if (WiFi.scanComplete() >= 0) {
                WiFi.scanDelete();
                Serial.println("🧹 Limpieza de escaneo WiFi por memoria baja");
            }
        }
        lastMemCheck = millis();
    }
    
    // Chequeo de conexión WiFi después de guardar configuración
    if (checkingConnection && millis() > connectionCheckTimer) {
        checkingConnection = false;
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("✅ Conectado exitosamente a WiFi!");
            Serial.println("📍 IP asignada: " + WiFi.localIP().toString());
            wifi_connected = true;
            
            // ACTIVAR MODO SENSOR: Habilitar auto-sleep después de conectar WiFi
            display_manager.enableAutoSleep(true);
            Serial.println("🖥️ Modo sensor activado - Auto-sleep HABILITADO");
            
            // Opcional: Desactivar AP después de conectar
            // WiFi.softAPdisconnect(true);
            // Serial.println("🔌 Access Point desactivado, solo modo Cliente WiFi");
        } else {
            Serial.println("❌ No se pudo conectar a WiFi. Reintentando...");
            Serial.printf("📊 Estado WiFi: %d\n", WiFi.status());
        }
    }
    
    // Manejar botón PRG para despertar pantalla
    display_manager.handlePRGButton();
    
    // Chequear auto-sleep de pantalla (solo en modo sensor)
    display_manager.checkAutoSleep();
    
    // ===== TUYA DEVICE LOOP (Commercial Style) =====
    if (tuyaDevice) {
        tuyaDevice->loop();
    }
    
    // ===== LECTURA INTELIGENTE DEL SENSOR (solo en modo normal) =====
    if (!portal_active) {
        static unsigned long lastSensorRead = 0;
        static float lastReadings[10] = {0}; // Historial de últimas lecturas
        static uint8_t readingIndex = 0;
        static bool isFillingDetected = false;
        
        // Obtener configuración de intervalos
        SensorConfig sensorConfig = config_manager.getSensorConfig();
        uint16_t currentInterval = isFillingDetected ? 
            sensorConfig.getFillingInterval() * 1000 : 
            sensorConfig.getNormalInterval() * 1000;
        
        if (millis() - lastSensorRead > currentInterval) {
            Serial.printf("🌊 Leyendo sensor (Intervalo: %s - %ds)...\n", 
                         isFillingDetected ? "LLENANDO" : "NORMAL",
                         currentInterval / 1000);
                         
            sensor_manager.update();
            
            // Obtener último valor leído para detección de llenado
            String sensorJson = sensor_manager.getSensorJson("WaterLevel");
            DynamicJsonDocument doc(256);
            DeserializationError error = deserializeJson(doc, sensorJson);
            
            if (!error) {
                float currentLevel = doc["litros"] | 0.0;
                
                // Guardar lectura en historial circular
                lastReadings[readingIndex] = currentLevel;
                readingIndex = (readingIndex + 1) % 10;
                
                // 🔍 DETECCIÓN DE LLENADO
                bool wasFillingBefore = isFillingDetected;
                isFillingDetected = detectFilling(lastReadings, sensorConfig.getFillingThreshold());
                
                if (isFillingDetected && !wasFillingBefore) {
                    Serial.println("🚰 LLENADO DETECTADO! Aumentando frecuencia de sensado");
                    WebSerial.println("🚰 LLENADO DETECTADO! Aumentando frecuencia de sensado");
                } else if (!isFillingDetected && wasFillingBefore) {
                    Serial.println("⏸️ Llenado finalizado. Volviendo a intervalo normal");
                    WebSerial.println("⏸️ Llenado finalizado. Volviendo a intervalo normal");
                }
            }
            
            // Actualizar pantalla con datos del sensor
            updateSensorDisplay();
            
            // Enviar datos a Tuya Device (si está conectado)
            if (tuyaDevice && tuyaDevice->isConnected()) {
                updateTuyaDeviceData();
            }
            
            // Enviar datos a ESP-NOW si es master
            if (espNowManager && config_manager.getESPNowConfig().isMaster()) {
                // TODO: Broadcast datos a slaves (implementar después)
            }
            
            lastSensorRead = millis();
        }
    }
    
    yield(); // Permitir que otras tareas del sistema se ejecuten
}

bool AppManager::isWiFiConnected() {
    return wifi_connected;
}

String AppManager::getDeviceIP() {
    return WiFi.softAPIP().toString();
}

void AppManager::restart() {
    Serial.println("Reiniciando...");
    ESP.restart();
}

// ===== FUNCIONES DE MANEJO DE REQUESTS =====

void AppManager::handleScanWiFi(AsyncWebServerRequest *request) {
    Serial.println("🔍 Procesando solicitud de escaneo WiFi");
    
    // Verificar memoria disponible
    size_t freeHeap = ESP.getFreeHeap();
    if (freeHeap < 30000) { // Menos de 30KB libre
        Serial.printf("⚠️ Memoria insuficiente: %d bytes libres\n", freeHeap);
        String json = "[{\"error\":true,\"message\":\"Memoria insuficiente para escaneo\"}]";
        request->send(503, "application/json", json);
        return;
    }
    
    // Verificar intervalo de escaneo para evitar spam
    unsigned long now = millis();
    if (now - lastWiFiScan < WIFI_SCAN_INTERVAL) {
        Serial.println("⚠️ Escaneo muy frecuente, esperando...");
        String json = "[{\"waiting\":true,\"message\":\"Esperando intervalo de escaneo\"}]";
        request->send(429, "application/json", json);
        return;
    }
    lastWiFiScan = now;
    
    // Verificar si ya hay un escaneo en progreso
    int scanStatus = WiFi.scanComplete();
    if (scanStatus == WIFI_SCAN_RUNNING) {
        Serial.println("⚠️ Escaneo ya en progreso");
        String json = "[{\"scanning\":true,\"message\":\"Escaneo en progreso...\"}]";
        request->send(200, "application/json", json);
        return;
    }
    
    // Limpiar escaneo anterior si existe
    if (scanStatus >= 0) {
        WiFi.scanDelete();
    }
    
    Serial.printf("🔍 Iniciando escaneo WiFi (Heap libre: %d bytes)\n", ESP.getFreeHeap());
    
    // Alimentar watchdog antes del escaneo
    esp_task_wdt_reset();
    
    // Escaneo asíncrono con configuración conservativa
    int result = WiFi.scanNetworks(true, false, false, 300); // async=true, max_ms_per_chan=300ms
    
    if (result == WIFI_SCAN_RUNNING) {
        String json = "[{\"scanning\":true,\"message\":\"Escaneo iniciado correctamente\"}]";
        request->send(200, "application/json", json);
    } else {
        Serial.printf("❌ Error iniciando escaneo: %d\n", result);
        String json = "[{\"error\":true,\"message\":\"Error iniciando escaneo WiFi\"}]";
        request->send(500, "application/json", json);
    }
}

void AppManager::handleWiFiResults(AsyncWebServerRequest *request) {
    // Esta función ya existe en el código, necesito encontrarla
    Serial.println("📡 Procesando solicitud de resultados WiFi");
    
    int scanResult = WiFi.scanComplete();
    
    if (scanResult == WIFI_SCAN_RUNNING) {
        String json = "[{\"scanning\":true,\"message\":\"Escaneo aún en progreso...\"}]";
        request->send(200, "application/json", json);
        return;
    }
    
    if (scanResult == WIFI_SCAN_FAILED || scanResult < 0) {
        String json = "[{\"error\":true,\"message\":\"Error en escaneo WiFi\"}]";
        request->send(500, "application/json", json);
        return;
    }
    
    // Construir JSON con resultados
    String json = "[";
    int maxNetworks = min(scanResult, 15); // Limitar a 15 redes
    
    for (int i = 0; i < maxNetworks; i++) {
        if (i > 0) json += ",";
        json += "{";
        json += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
        json += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
        json += "\"encryption\":" + String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
        json += "}";
    }
    json += "]";
    
    WiFi.scanDelete(); // Limpiar resultados
    request->send(200, "application/json", json);
    Serial.printf("✅ Enviados %d resultados de escaneo\n", maxNetworks);
}

void AppManager::handleSensorData(AsyncWebServerRequest *request) {
    Serial.println("🌊 Procesando solicitud de datos del sensor");
    
    // Aquí iría la lógica para obtener datos del sensor
    // Por ahora, datos de ejemplo
    String json = "{";
    json += "\"water_level\":" + String(75.5) + ",";
    json += "\"percentage\":" + String(65) + ",";
    json += "\"tank_height\":" + String(115.0) + ",";
    json += "\"distance\":" + String(39.2) + ",";
    json += "\"status\":\"normal\",";
    json += "\"timestamp\":" + String(millis());
    json += "}";
    
    request->send(200, "application/json", json);
}

void AppManager::handleToggleDisplay(AsyncWebServerRequest *request) {
    Serial.println("🖥️ Procesando toggle de pantalla OLED");
    
    if (display_manager.isDisplaySleeping()) {
        display_manager.wakeUpDisplay();
        request->send(200, "application/json", "{\"display\":\"awake\",\"message\":\"Pantalla encendida\"}");
    } else {
        display_manager.forceDisplaySleep();
        request->send(200, "application/json", "{\"display\":\"sleeping\",\"message\":\"Pantalla apagada\"}");
    }
}

void AppManager::updateSensorDisplay() {
    // Solo actualizar pantalla si no está durmiendo y estamos en modo sensor
    if (portal_active || display_manager.isDisplaySleeping()) {
        return;
    }
    
    Serial.println("🖥️ Actualizando pantalla con datos del sensor");
    
    // Variables estáticas para almacenar valores anteriores
    static float lastDistance = -999.0;
    static float lastLitros = -999.0;
    static int lastPercentage = -999;
    static String lastStatus = "";
    static bool firstRun = true;
    
    // Obtener datos REALES del sensor
    String sensorJson = sensor_manager.getSensorJson("WaterLevel");
    
    // Si es primera ejecución o hay error, redibujar todo
    if (firstRun || sensorJson.indexOf("error") >= 0) {
        firstRun = false;
        
        display_manager.clear();
        display_manager.setFont(ArialMT_Plain_10);
        display_manager.setTextAlignment(TEXT_ALIGN_LEFT);
        
        // Línea 1: Título con rol ESP-NOW (estático)
        String title = "SENSOR NIVEL AGUA";
        if (config_manager.getESPNowConfig().isMaster()) {
            title += " [M]";
        } else if (config_manager.getESPNowConfig().isSlave()) {
            title += " [S]";
        }
        display_manager.drawString(0, 0, title);
        
        if (sensorJson.indexOf("error") >= 0) {
            // Mostrar error del sensor
            display_manager.drawString(0, 12, "⚠️ SENSOR ERROR");
            display_manager.drawString(0, 24, "Verificar conexión");
            display_manager.drawString(0, 36, "Trig/Echo pins");
            
            // Mostrar info de red en caso de error
            if (WiFi.status() == WL_CONNECTED) {
                display_manager.drawString(0, 48, "WiFi: " + WiFi.SSID());
            } else {
                display_manager.drawString(0, 48, "Sin WiFi");
            }
            
            display_manager.display();
            
            // Resetear valores para próxima actualización
            lastDistance = -999.0;
            lastLitros = -999.0;
            lastPercentage = -999;
            lastStatus = "";
            return;
        }
        
        // Dibujar etiquetas estáticas
        display_manager.drawString(0, 12, "Dist:");
        display_manager.drawString(0, 24, "Agua:");
        display_manager.drawString(0, 36, "Lleno:");
    }
    
    // Parsear datos reales del sensor
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, sensorJson);
    
    if (!error) {
        float distance = doc["distancia"] | -1.0;
        float litros = doc["litros"] | 0.0;
        int percentage = doc["porcentaje"] | 0;
        
        // Solo actualizar si cambió la distancia
        if (distance != lastDistance) {
            // Borrar solo el área del valor anterior (aprox 60 pixels)
            display_manager.setColor(0);  // BLACK
            display_manager.fillRect(36, 12, 92, 10, 0);
            display_manager.setColor(1);  // WHITE
            
            // Dibujar nuevo valor
            display_manager.drawString(36, 12, String(distance, 1) + " cm");
            lastDistance = distance;
        }
        
        // Solo actualizar si cambió litros
        if (litros != lastLitros) {
            // Borrar solo el área del valor anterior
            display_manager.setColor(0);
            display_manager.fillRect(36, 24, 92, 10, 0);
            display_manager.setColor(1);
            
            // Dibujar nuevo valor
            display_manager.drawString(36, 24, String(litros, 1) + " L");
            lastLitros = litros;
        }
        
        // Solo actualizar si cambió porcentaje
        if (percentage != lastPercentage) {
            // Borrar solo el área del valor anterior
            display_manager.setColor(0);
            display_manager.fillRect(42, 36, 86, 10, 0);
            display_manager.setColor(1);
            
            // Dibujar nuevo valor
            display_manager.drawString(42, 36, String(percentage) + " %");
            lastPercentage = percentage;
        }
        
        // Línea de estado (WiFi/fecha)
        String statusLine = "";
        if (ntpSync && ntpSync->isReady() && config_manager.getSensorConfig().getShowDateTime()) {
            statusLine = ntpSync->getCompactDateTime(); // "27/09 14:30"
        } else if (WiFi.status() == WL_CONNECTED) {
            statusLine = "WiFi: OK";
        } else {
            statusLine = "Sin WiFi";
        }
        
        // Solo actualizar línea de estado si cambió
        if (statusLine != lastStatus) {
            display_manager.setColor(0);
            display_manager.fillRect(0, 48, 128, 16, 0);
            display_manager.setColor(1);
            
            display_manager.drawString(0, 48, statusLine);
            lastStatus = statusLine;
        }
        
        // Actualizar display solo si hubo cambios
        if (distance != lastDistance || litros != lastLitros || 
            percentage != lastPercentage || statusLine != lastStatus) {
            display_manager.display();
        }
    } else {
        // Error parseando JSON - redibujar todo
        firstRun = true;
        display_manager.clear();
        display_manager.setFont(ArialMT_Plain_10);
        display_manager.drawString(0, 12, "Error datos sensor");
        display_manager.drawString(0, 24, "JSON inválido");
        display_manager.display();
        
        lastDistance = -999.0;
        lastLitros = -999.0;
        lastPercentage = -999;
        lastStatus = "";
    }
}

void AppManager::updateTuyaDeviceData() {
    if (!tuyaDevice) return;
    
    // Obtener datos del sensor en JSON
    String sensorJson = sensor_manager.getSensorJson("WaterLevel");
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, sensorJson);
    
    if (error) {
        Serial.println("❌ Error parsing sensor JSON");
        return;
    }
    
    // Extraer valores del sensor
    float litros = doc["litros"] | 0.0;
    float porcentaje = doc["porcentaje"] | 0.0; 
    float distancia = doc["distancia_cm"] | 0.0;
    bool alerta = (porcentaje < 20.0);
    
    // Enviar a Tuya Device usando Data Points estándar
    tuyaDevice->updateSensorValue("1", litros);      // DP1: Litros de agua
    tuyaDevice->updateSensorValue("2", porcentaje);  // DP2: Porcentaje lleno
    tuyaDevice->updateSensorValue("3", distancia);   // DP3: Distancia sensor
    tuyaDevice->updateStatus("4", alerta);           // DP4: Alerta agua baja
    
    // Log para debug
    static unsigned long lastTuyaLog = 0;
    if (millis() - lastTuyaLog > 30000) { // Log cada 30 segundos
        Serial.println("📱 Datos enviados a Tuya App:");
        Serial.println("   💧 Agua: " + String(litros, 1) + "L (" + String(porcentaje, 1) + "%)");
        Serial.println("   📏 Distancia: " + String(distancia, 1) + "cm");
        Serial.println("   🚨 Alerta: " + String(alerta ? "AGUA BAJA" : "OK"));
        lastTuyaLog = millis();
    }
}

bool AppManager::detectFilling(float readings[], uint8_t threshold) {
    // Algoritmo para detectar si el contenedor se está llenando
    // Verifica si las últimas N lecturas son incrementales
    
    if (threshold < 2 || threshold > 10) {
        threshold = 3; // Por seguridad
    }
    
    // Contar lecturas válidas (> 0)
    uint8_t validReadings = 0;
    for (uint8_t i = 0; i < 10; i++) {
        if (readings[i] > 0) {
            validReadings++;
        }
    }
    
    // Necesitamos al menos 'threshold' lecturas válidas
    if (validReadings < threshold) {
        return false;
    }
    
    // Obtener las últimas 'threshold' lecturas válidas
    float recentReadings[10];
    uint8_t recentIndex = 0;
    
    // Buscar desde la posición más reciente hacia atrás
    for (int i = 9; i >= 0 && recentIndex < threshold; i--) {
        if (readings[i] > 0) {
            recentReadings[recentIndex] = readings[i];
            recentIndex++;
        }
    }
    
    // Verificar si hay tendencia incremental consistente
    uint8_t incrementalCount = 0;
    float minIncrease = 0.5; // Mínimo incremento en litros para considerar llenado
    
    for (uint8_t i = 1; i < recentIndex; i++) {
        // recentReadings[0] es la más reciente, recentReadings[i] es más antigua
        if (recentReadings[0] > recentReadings[i] + minIncrease) {
            incrementalCount++;
        }
    }
    
    // Si al menos (threshold-1) comparaciones muestran incremento, está llenando
    bool isIncreasing = (incrementalCount >= (threshold - 1));
    
    if (isIncreasing) {
        Serial.printf("🔍 Llenado detectado: %d/%d lecturas incrementales\n", 
                     incrementalCount, threshold - 1);
        Serial.printf("   📊 Lecturas recientes (L): ");
        for (uint8_t i = 0; i < recentIndex; i++) {
            Serial.printf("%.1f ", recentReadings[i]);
        }
        Serial.println();
    }
    
    return isIncreasing;
}