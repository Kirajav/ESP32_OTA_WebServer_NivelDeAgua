#include "AppManager.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include <Arduino.h>
#include <WiFi.h>
#include <heltec.h>
#include "SPIFFS.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncDNSServer.h>
#include <ESPAsync_WiFiManager.h>
#include <ArduinoJson.h>
#include <ESP_DoubleResetDetector.h>
#include <nvs_flash.h>
#include "ConfigManager.h"
#include "SensorManager.h"
#include "WaterLevelSensor.h"
#include "DisplayManager.h"
#include "WebManager.h"
#include "SystemStatus.h"
#include "OTAUpdater.h"

AppManager::AppManager() :
    drd(DRD_TIMEOUT, DRD_ADDRESS),
    wifiConnected(false),
    lastWifiRetryMillis(0),
    previousMillis(0),
    server(80),
    dnsServer(),
    configManager(),
    displayManager(),
    sensorManager(),
    waterLevelSensor(TRIGGER_PIN, ECHO_PIN_1, &configManager),
    systemStatus(&displayManager, &configManager),
    otaUpdater(&displayManager, &systemStatus)
{
    _instance = this; // Set the static instance
    wifiManager = new ESPAsync_WiFiManager(&server, &dnsServer, "TinacoESP");
    webManager = new WebManager(&server, &sensorManager, &configManager, &displayManager, &systemStatus);
}

AppManager* AppManager::_instance = nullptr; // Initialize static member

void AppManager::saveConfigCallback() {
    if (_instance) {
        _instance->saveWiFiManagerParams();
    }
}

void AppManager::configModeCallback(ESPAsync_WiFiManager* myWiFiManager) {
    Serial.println("=== PORTAL CAPTIVO INICIADO ===");
    Serial.println("SSID: ESP32_Sensor");
    Serial.println("Password: 12345");
    Serial.println("IP: 192.168.1.1");
    Serial.println("=================================");
    
    if (_instance) {
        // Actualizar pantalla con información del portal
        _instance->displayManager.clear();
        _instance->displayManager.setFont(ArialMT_Plain_10);
        _instance->displayManager.setTextAlignment(TEXT_ALIGN_CENTER);
        _instance->displayManager.drawString(64, 0, "PORTAL ACTIVO");
        _instance->displayManager.setTextAlignment(TEXT_ALIGN_LEFT);
        _instance->displayManager.drawString(0, 12, "SSID: ESP32_Sensor");
        _instance->displayManager.drawString(0, 22, "Pass: 12345");
        _instance->displayManager.drawString(0, 32, "IP: 192.168.1.1");
        _instance->displayManager.drawString(0, 42, "Abre el navegador");
        _instance->displayManager.display();
    }
}

void AppManager::begin() {
    Serial.begin(115200);
    Serial.println(F("\nIniciando Sensor de Nivel de Agua..."));
    
    // FORZAR RESET TOTAL DE CREDENCIALES WIFI HASTA QUE FUNCIONE EL PORTAL ABIERTO
    Serial.println("=== FORZANDO BORRADO TOTAL DE CREDENCIALES WiFi ===");
    WiFi.mode(WIFI_OFF);
    delay(100);
    WiFi.disconnect(true);
    delay(100);
    esp_wifi_restore();  // Borra configuración WiFi del ESP32
    nvs_flash_erase();   // Borra NVS completo
    nvs_flash_init();    // Reinicia NVS
    delay(500);
    
    // Verificar si la detección de doble reset es correcta
    bool isDoubleReset = drd.detectDoubleReset();
    Serial.print("Detect Double Reset: ");
    Serial.println(isDoubleReset);

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

    WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
        switch(event) {
            case SYSTEM_EVENT_STA_CONNECTED:
                Serial.println(F("WiFi Event: CONNECTED"));
                this->wifiConnected = true;
                break;
            case SYSTEM_EVENT_STA_DISCONNECTED:
                Serial.println(F("WiFi Event: DISCONNECTED"));
                this->wifiConnected = false;
                break;
            case SYSTEM_EVENT_STA_GOT_IP:
                Serial.println("WiFi Event: GOT IP: " + WiFi.localIP().toString());
                break;
            default:
                Serial.printf("WiFi Event: %d\n", event);
                break;
        }
    });
    
    displayManager.begin();
    
    configManager.begin();

    sensorManager.addSensor(&waterLevelSensor);
    sensorManager.begin();
    
    systemStatus.begin();
    systemStatus.setMonitorEnabled(true); // Habilitar el monitoreo serial del sensor
    configManager.setDisplayOn(true);     // Forzar el display a estar encendido
    
    if (isDoubleReset) {
        Serial.println("=== DOBLE RESET CONFIRMADO ===");
        displayManager.clear();
        displayManager.setTextAlignment(TEXT_ALIGN_CENTER);
        displayManager.drawString(64, 10, "Modo Config WiFi");
        displayManager.drawString(64, 20, "Borrando config...");
        displayManager.display();
        
        // Borrar TODA la configuración
        WiFi.mode(WIFI_OFF);
        delay(100);
        WiFi.disconnect(true);
        delay(100);
        esp_wifi_restore();  // Borra configuración WiFi del ESP32
        wifiManager->resetSettings();
        
        // Forzar borrado de preferencias
        nvs_flash_erase();
        nvs_flash_init();
        
        delay(2000);
        ESP.restart();
    } else {
        Serial.println("=== INICIO NORMAL ===");
    }
    
    // WiFiManager setup - RESET COMPLETO
    wifiManager->resetSettings();  // Borrar configuraciones del WiFiManager
    Serial.println("=== Configuraciones WiFiManager borradas ===");
    
    wifiManager->setDebugOutput(true);
    wifiManager->setMinimumSignalQuality(-1);
    wifiManager->setBreakAfterConfig(false);  // No romper después de guardar configuración
    wifiManager->setSaveConfigCallback(AppManager::saveConfigCallback);
    wifiManager->setConfigPortalTimeout(180);  // 3 minutos de timeout para el portal
    wifiManager->setConnectTimeout(15000);     // 15 segundos timeout de conexión
    wifiManager->setAPCallback(AppManager::configModeCallback);
    
    // Configuración específica para red abierta
    wifiManager->setConfigPortalTimeout(300);  // 5 minutos timeout
    wifiManager->setConnectTimeout(20000);     // 20 segundos timeout
    
    // Optimizaciones para estabilidad de red
    wifiManager->setConfigPortalChannel(6);     // Canal fijo 6 (menos congestionado que 1,11)
    
    // Configurar IP personalizada para el SoftAP (volver a 192.168.1.1)
    IPAddress apIP(192, 168, 1, 1);        // IP del ESP32 como AP
    IPAddress gateway(192, 168, 1, 1);     // Gateway
    IPAddress subnet(255, 255, 255, 0);    // Máscara de subred
    wifiManager->setAPStaticIPConfig(apIP, gateway, subnet);
    
    // Configuraciones adicionales para estabilidad
    WiFi.setTxPower(WIFI_POWER_19_5dBm);   // Potencia media para mejor estabilidad
    WiFi.setSleep(false);                   // Deshabilitar sleep para mejor latencia
    
    Serial.println("=== WiFiManager configurado ===");
    
    wifiManager->setCustomHeadElement("<style>\n        .networks-list .wifi-name:after {content: ' (Guardada)'; color: green; font-weight: bold;} \n        .panel-title { margin-bottom: 10px; text-align: center; color: #069; } \n        .saved-wifi { background-color: #e8f5e9; border-left: 3px solid #4caf50; } \n        .networks-list { margin-top: 10px; } \n        </style>");

    // Títulos de secciones
    wifiManager->addParameter(new ESPAsync_WMParameter("<hr><h3 style='text-align:center'>Configuración del sensor</h3>"));
    
    const char* tipo_actual = configManager.getTipoContenedorStr();
    custom_tipo_contenedor = new ESPAsync_WMParameter(
        "tipo_contenedor", "Tipo de contenedor", tipo_actual, 20,
        "required onchange='this.form.tipo_contenedor.value=this.value' list='tipos_contenedor'");
    wifiManager->addParameter(new ESPAsync_WMParameter(
        "<datalist id='tipos_contenedor'>\n        <option value='tinaco'>Tinaco</option>\n        <option value='cisterna'>Cisterna</option>\n        <option value='otro contenedor'>Otro contenedor</option>\n        </datalist>"));
        
    custom_altura_max = new ESPAsync_WMParameter(
        "altura_max", "Altura máxima del nivel del agua en el contenedor (cm)", 
        String(configManager.getAlturaMax()).c_str(), 10,
        "required type='number' step='0.1' min='0'");
        
    custom_capacidad = new ESPAsync_WMParameter(
        "capacidad", "Capacidad del contenedor aprox. (L)", 
        String(configManager.getCapacidad()).c_str(), 10,
        "required type='number' step='0.1' min='0'");
        
    custom_distancia_min = new ESPAsync_WMParameter(
        "distancia_min", "Distancia mínima que puede leer el sensor (cm)", 
        String(configManager.getDistanciaMin()).c_str(), 10,
        "required type='number' step='0.1' min='0'");
        
    // Agregar sección de configuración de red
    wifiManager->addParameter(new ESPAsync_WMParameter("<hr><h3 style='text-align:center'>Configuración de red</h3>"));
    
    custom_hostname = new ESPAsync_WMParameter(
        "hostname", "Nombre del dispositivo en la red", 
        configManager.getHostname().c_str(), 40,
        "placeholder='ESP32_Sensor_1' pattern='^[^-\\.]{2,32}$' title='Entre 2 y 32 caracteres sin puntos ni guiones'");
        
    custom_check_updates = new ESPAsync_WMParameter(
        "check_updates", "¿Buscar actualizaciones al iniciar?",
        configManager.getCheckUpdates() ? "Sí" : "No", 10,
        "required");
        
    // Agregar parámetros al WiFiManager
    wifiManager->addParameter(custom_tipo_contenedor);
    wifiManager->addParameter(custom_altura_max);
    wifiManager->addParameter(custom_capacidad);
    wifiManager->addParameter(custom_distancia_min);
    wifiManager->addParameter(custom_hostname);
    wifiManager->addParameter(custom_check_updates);
    
    // Configurar callbacks
    // The original setSaveConfigCallback was removed because it was causing a restart.
    // The saveWiFiManagerParams() method will be called when the parameters are saved.

    const char* AP_SSID = "ESP32_Sensor";
    const char* AP_PASS = "12345";  // Contraseña simple del 1 al 5

    displayManager.clear();
    displayManager.setFont(ArialMT_Plain_16);
    displayManager.setTextAlignment(TEXT_ALIGN_CENTER);
    displayManager.drawString(64, 0, F("SENSOR DE"));
    displayManager.drawString(64, 16, F("NIVEL DE AGUA"));
    
    displayManager.clear();
    displayManager.setFont(ArialMT_Plain_10);
    displayManager.setTextAlignment(TEXT_ALIGN_CENTER);
    displayManager.drawString(64, 0, F("SENSOR DE NIVEL"));
    displayManager.setTextAlignment(TEXT_ALIGN_LEFT);
    displayManager.drawString(0, 12, F("Conectando WiFi..."));
    displayManager.drawString(0, 22, "SSID: ESP32_Sensor");
    displayManager.drawString(0, 32, "Pass: RED ABIERTA");
    displayManager.drawString(0, 42, "IP: 192.168.4.1");
    displayManager.display();
    
    // FORZAR MODO PORTAL CAPTIVO DIRECTO - CON CONTRASEÑA SIMPLE
    // Esto evita que intente usar credenciales almacenadas anteriormente
    displayManager.drawString(0, 22, "SSID: ESP32_Sensor");
    displayManager.drawString(0, 32, "Pass: 12345");
    displayManager.drawString(0, 42, "IP: 192.168.1.1");
    displayManager.display();
    
    Serial.println("=== FORZANDO PORTAL CAPTIVO DIRECTO ===");
    Serial.println("Saltando autoConnect y iniciando portal captivo directamente");
    Serial.println("Contraseña: 12345");
    
    // Usar startConfigPortal con contraseña
    if (wifiManager->startConfigPortal(AP_SSID, AP_PASS)) {
        Serial.println(F("=== WiFi CONECTADO desde portal ==="));
        Serial.print(F("Red: "));
        Serial.println(WiFi.SSID());
        Serial.print(F("IP: "));
        Serial.println(WiFi.localIP());
        
        wifiConnected = true;
        
        WiFi.setHostname(configManager.getHostname().c_str());
        WiFi.setAutoReconnect(true);
        WiFi.persistent(true);
        
        displayManager.clear();
        displayManager.setTextAlignment(TEXT_ALIGN_CENTER);
        displayManager.drawString(64, 0, F("WiFi Conectado"));
        displayManager.setTextAlignment(TEXT_ALIGN_LEFT);
        displayManager.drawString(0, 15, "Red: " + WiFi.SSID());
        displayManager.drawString(0, 25, "IP: " + WiFi.localIP().toString());
        displayManager.drawString(0, 35, "Iniciando servidor...");
        displayManager.display();
        delay(2000);
        
    } else {
        Serial.println(F("=== Portal captivo terminado ==="));
        Serial.println(F("Puede que el usuario haya cancelado o timeout"));
        wifiConnected = false;
        
        // El startConfigPortal ya ha manejado la interfaz
        displayManager.clear();
        displayManager.setTextAlignment(TEXT_ALIGN_CENTER);
        displayManager.drawString(64, 0, "PORTAL TERMINADO");
        displayManager.setTextAlignment(TEXT_ALIGN_LEFT);
        displayManager.drawString(0, 12, "SSID: ESP32_Sensor");
        displayManager.drawString(0, 22, "Pass: 12345");
        displayManager.drawString(0, 32, "IP: 192.168.1.1");
        displayManager.drawString(0, 42, "Reiniciar para reconfigurar");
        displayManager.display();
    }
    
    webManager->begin();
    
    // Configurar callback para reset WiFi
    webManager->setWiFiResetCallback(AppManager::resetWiFiCallback);
    
    otaUpdater.begin(&server);
    
    server.begin();
    
    previousMillis = millis();
}

void AppManager::saveWiFiManagerParams() {
    Serial.println(F("Callback de guardado de parámetros"));
    
    if (custom_altura_max && custom_capacidad && custom_distancia_min && custom_tipo_contenedor &&
        custom_hostname && custom_check_updates) {
        
        configManager.setAlturaMax(atof(custom_altura_max->getValue()));
        configManager.setCapacidad(atof(custom_capacidad->getValue()));
        Serial.print(F("Valor de custom_distancia_min->getValue(): "));
        Serial.println(custom_distancia_min->getValue());
        configManager.setDistanciaMin(atof(custom_distancia_min->getValue()));
        
        // Obtener y procesar el tipo de contenedor
        String tipoStr = String(custom_tipo_contenedor->getValue());
        tipoStr.toLowerCase();  // Convertir a minúsculas
        tipoStr.trim();        // Eliminar espacios
        
        if (tipoStr == "tinaco") configManager.setTipoContenedor(0);
        else if (tipoStr == "cisterna") configManager.setTipoContenedor(1);
        else if (tipoStr == "otro contenedor") configManager.setTipoContenedor(2);
        else configManager.setTipoContenedor(0);  // Por defecto
        
        // Guardar parámetros de red
        configManager.setHostname(custom_hostname->getValue());
        configManager.setCheckUpdates(String(custom_check_updates->getValue()) == "Sí");
        
        configManager.saveConfig();
        
        Serial.println(F("Parámetros guardados:"));
        Serial.println("Tipo de contenedor: " + tipoStr);
        Serial.println("Altura máxima: " + String(configManager.getAlturaMax()));
        Serial.println("Capacidad: " + String(configManager.getCapacidad()));
        Serial.println("Distancia mínima: " + String(configManager.getDistanciaMin()));
        Serial.println("Hostname: " + configManager.getHostname());
        Serial.println("Buscar actualizaciones: " + String(configManager.getCheckUpdates() ? "Sí" : "No"));
    }
}

void AppManager::loop() {
    //Serial.println("WiFi Status: " + String(WiFi.status()));
    //Serial.println("SystemStatus isMonitorEnabled: " + String(systemStatus.isMonitorEnabled()));
    //Serial.println("ConfigManager isDisplayOn: " + String(configManager.isDisplayOn()));
    
    otaUpdater.loop();
    systemStatus.loop();
    
    // Actualizar efecto fade del LED si está activo
    displayManager.updateFadeEffect();
    
    unsigned long currentMillis = millis();
    
    // Verificar y manejar la conexión WiFi
    if (WiFi.status() != WL_CONNECTED && wifiConnected) {
        Serial.println("=== WiFi desconectado ===");
        wifiConnected = false;
        lastWifiRetryMillis = currentMillis;
        
        if (configManager.isDisplayOn()) {
            displayManager.clear();
            displayManager.setFont(ArialMT_Plain_10);
            displayManager.setTextAlignment(TEXT_ALIGN_CENTER);
            displayManager.drawString(64, 0, F("WiFi perdido"));
            displayManager.drawString(64, 15, F("Reintentando..."));
            displayManager.drawString(64, 30, F("Portal activo:"));
            displayManager.drawString(64, 40, F("ESP32_Sensor"));
            displayManager.drawString(64, 50, F("192.168.4.1"));
            displayManager.display();
        }
    }
    
    // Reintento de conexión WiFi cada 5 minutos
    if (!wifiConnected && (currentMillis - lastWifiRetryMillis >= wifiRetryInterval)) {
        Serial.println(F("=== Reintentando conexión WiFi ==="));
        lastWifiRetryMillis = currentMillis;
        
        // Intentar reconexión
        WiFi.reconnect();
        
        // Esperar hasta 15 segundos
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 30) {
            delay(500);
            attempts++;
            Serial.print(".");
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n=== WiFi RECONECTADO ===");
            Serial.print("IP: ");
            Serial.println(WiFi.localIP());
            wifiConnected = true;
            
            if (configManager.isDisplayOn()) {
                displayManager.clear();
                displayManager.setTextAlignment(TEXT_ALIGN_CENTER);
                displayManager.drawString(64, 0, F("WiFi OK"));
                displayManager.drawString(64, 20, WiFi.localIP().toString());
                displayManager.display();
                delay(2000);
            }
        } else {
            Serial.println("\n=== Falló reconexión ===");
        }
    }
    
    //Serial.println("AppManager::loop() - Checking update interval.");
    if (currentMillis - previousMillis >= interval) {
        // Reducir logs solo cada 30 segundos
        static unsigned long lastVerboseLog = 0;
        bool shouldLog = (currentMillis - lastVerboseLog) > 30000;
        
        if (shouldLog) {
            Serial.println("AppManager::loop() - Actualizando sensores...");
            lastVerboseLog = currentMillis;
        }
        
        previousMillis = currentMillis;
        
        sensorManager.update();
        
        if (systemStatus.isMonitorEnabled() && shouldLog) {
            String sensorJson = sensorManager.getSensorJson("WaterLevel");
            Serial.println("JSON del sensor: " + sensorJson);
        }
        
        if (configManager.isDisplayOn()) {
            StaticJsonDocument<256> doc;
            String sensorJson = sensorManager.getSensorJson("WaterLevel");
            DeserializationError error = deserializeJson(doc, sensorJson);
            if (!error) {
                String distancia = doc["distancia_cm"].as<String>();
                String litros = doc["litros"].as<String>();
                if (shouldLog) {
                    Serial.println("Actualizando display - Distancia: " + distancia + ", Litros: " + litros);
                }
                displayManager.updateDisplay(WiFi.localIP().toString(), distancia, litros);
            } else {
                Serial.println("Error deserializando JSON del sensor");
            }
        }
    }
    
    if (configManager.isDisplayOn() && (currentMillis - systemStatus.getOledStartTime() >= systemStatus.getOledTimeout())) {
        //Serial.println(F("Apagando display por timeout"));
        systemStatus.setDisplayEnabled(false);
        displayManager.displayOff();
        configManager.saveConfig();
    }
}

void AppManager::resetWiFiFromWeb() {
    Serial.println("=== RESET WiFi SOLICITADO DESDE WEB ===");
    
    // Borrar TODA la configuración WiFi
    WiFi.mode(WIFI_OFF);
    delay(100);
    WiFi.disconnect(true);
    delay(100);
    esp_wifi_restore();  // Borra configuración WiFi del ESP32
    
    // Borrar configuración del WiFiManager
    if (wifiManager) {
        wifiManager->resetSettings();
        Serial.println("Configuración WiFiManager borrada");
    }
    
    // Forzar borrado de preferencias
    nvs_flash_erase();
    nvs_flash_init();
    
    Serial.println("Configuración WiFi completamente borrada");
    Serial.println("Reiniciando para entrar en modo configuración...");
    delay(1000);
    ESP.restart();
}

// Callback estático para reset WiFi
void AppManager::resetWiFiCallback() {
    if (_instance) {
        _instance->resetWiFiFromWeb();
    }
}
