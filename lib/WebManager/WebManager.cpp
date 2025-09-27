#include "WebManager.h"
#include "SPIFFS.h"

// Estructura para definir comandos de WebSerial
struct WebSerialCommand {
    const char* name;
    std::function<void()> handler;
};

WebManager::WebManager(AsyncWebServer* server, SensorManager* sensorManager, ConfigManager* configManager, DisplayManager* displayManager, HardwareBoardStatus* systemStatus) :
_server(server), _sensorManager(sensorManager), _configManager(configManager), _displayManager(displayManager), _systemStatus(systemStatus), _ws("/ws"), _wifiResetCallback(nullptr) {
}

void WebManager::begin() {
    initWebSocket();
    initWebSerial();

    _server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html", false, [this](const String& var){ return processor(var); });
    });

    // ESP-NOW Manager Page
    _server->on("/espnow-manager", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/espnow-manager.html", "text/html", false, [this](const String& var){ return processor(var); });
    });

    _server->on("/Sensor", HTTP_GET, [this](AsyncWebServerRequest *request) {
        // Añadir headers para reducir latencia
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", _sensorManager->getSensorJson("WaterLevel"));
        response->addHeader("Cache-Control", "no-cache");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });

    _server->on("/Litros", HTTP_GET, [this](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", getSensorValue("WaterLevel", "litros"));
        response->addHeader("Cache-Control", "no-cache");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });

    _server->on("/Distancia", HTTP_GET, [this](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", getSensorValue("WaterLevel", "distancia_cm"));
        response->addHeader("Cache-Control", "no-cache");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });

    _server->on("/imagen", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String imageName = getImageForLevel();
        AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/" + imageName, "image/jpeg");
        // Cache de imágenes por 5 minutos
        response->addHeader("Cache-Control", "max-age=300");
        request->send(response);
    });

    _server->on("/toggleDisplay", HTTP_GET, [this](AsyncWebServerRequest *request) {
        bool newState = !_systemStatus->isDisplayEnabled();
        _systemStatus->setDisplayEnabled(newState);
        if (newState) {
            _displayManager->displayOn();
            _systemStatus->resetOledTimeout();
        } else {
            _displayManager->displayOff();
        }
        _configManager->save();
        
        // Enviar estado actualizado por WebSocket
        String message = newState ? "1" : "0";
        _ws.textAll(message);
        
        request->send(200, "text/plain", newState ? "Display ON" : "Display OFF");
    });

    _server->on("/displayStatus", HTTP_GET, [this](AsyncWebServerRequest *request) {
        bool displayState = _systemStatus->isDisplayEnabled();
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", 
            "{\"enabled\":" + String(displayState ? "true" : "false") + 
            ",\"status\":\"" + String(displayState ? "Encendido" : "Apagado") + "\"}");
        response->addHeader("Cache-Control", "no-cache");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });

    _server->on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Reiniciando...");
        delay(100);
        ESP.restart();
    });

    _server->on("/reset-wifi", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Borrando configuración WiFi y reiniciando...");
        delay(100);
        
        // Usar el callback si está disponible
        if (_wifiResetCallback) {
            _wifiResetCallback();
        } else {
            // Fallback anterior si no hay callback
            Serial.println("=== RESET WiFi SOLICITADO DESDE WEB (FALLBACK) ===");
            WiFi.disconnect(true);
            delay(100);
            ESP.restart();
        }
    });

    _server->serveStatic("/", SPIFFS, "/");
    
    // Inicializar ESP-NOW y endpoints
    _espNowManager = ESPNowManager::getInstance();
    _espNowManager->initESPNow();
    setupESPNowEndpoints();
    
    // Inicializar Google Home
    _googleHome = new GoogleHomeIntegration(_sensorManager);
    _googleHome->init("smart-water-sensor", "esp32-main");
    _googleHome->setEnabled(true);
    
    // Endpoint para Google Assistant fulfillment
    _server->on("/google-fulfillment", HTTP_POST, [this](AsyncWebServerRequest *request) {
        String requestBody;
        if (request->hasParam("plain", true)) {
            requestBody = request->getParam("plain", true)->value();
        }
        
        String response = _googleHome->processGoogleRequest(requestBody);
        request->send(200, "application/json", response);
    });
    
    // Inicializar Alexa
    _alexa = new AlexaIntegration(_sensorManager);
    _alexa->init("amzn1.ask.skill.your-skill-id");
    _alexa->setEnabled(true);
    
    // Endpoint para Alexa Skills Kit
    _server->on("/alexa-fulfillment", HTTP_POST, [this](AsyncWebServerRequest *request) {
        String requestBody;
        if (request->hasParam("plain", true)) {
            requestBody = request->getParam("plain", true)->value();
        }
        
        String response = _alexa->processAlexaRequest(requestBody);
        request->send(200, "application/json", response);
    });
    
    // Inicializar Tuya
    _tuya = new TuyaIntegration(_sensorManager);
    _tuya->init("your-product-key", "your-device-secret", "esp32-water-sensor");
    _tuya->setEnabled(true);
    
    // Endpoint para reportar a Tuya manualmente
    _server->on("/tuya-report", HTTP_POST, [this](AsyncWebServerRequest *request) {
        String sensorJson = _sensorManager->getSensorJson("WaterLevel");
        DynamicJsonDocument doc(512);
        deserializeJson(doc, sensorJson);
        
        float litros = doc["litros"];
        float distancia = doc["distancia_cm"];
        
        _tuya->reportWaterLevel(litros, distancia);
        request->send(200, "application/json", "{\"status\":\"reported\"}");
    });
}

void WebManager::initWebSocket() {
    _ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){ onWebSocketEvent(server, client, type, arg, data, len); });
    _server->addHandler(&_ws);
}

void WebManager::onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            client->text(_systemStatus->isDisplayEnabled() ? "1" : "0");
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA: {
            AwsFrameInfo *info = (AwsFrameInfo*)arg;
            if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
                data[len] = 0;
                String command = (char*)data;
                if (command == "toggle") {
                    bool newState = !_systemStatus->isDisplayEnabled();
                    _systemStatus->setDisplayEnabled(newState);
                    if (newState) {
                        _displayManager->displayOn();
                        _systemStatus->resetOledTimeout();
                    } else {
                        _displayManager->displayOff();
                    }
                    _configManager->save();
                    _ws.textAll(newState ? "1" : "0");
                }
                else if (command == "reset") {
                    _ws.textAll("restarting");
                    delay(100);
                    ESP.restart();
                }
            }
            break;
        }
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void WebManager::initWebSerial() {
    WebSerial.begin(_server);
    WebSerial.onMessage([this](uint8_t *data, size_t len){ onWebSerialMessage(data, len); });
    WebSerial.println("\nWebSerial iniaciado. Escribe 'ayuda' para ver los comandos disponibles.");
}

void WebManager::onWebSerialMessage(uint8_t *data, size_t len) {
    String commandStr = String((char*)data).substring(0, len);
    commandStr.toLowerCase();
    commandStr.trim();

    const WebSerialCommand commands[] = {
        {"ayuda", []() {
            WebSerial.println("Comandos disponibles:");
            WebSerial.println("  estado - Muestra el estado del sistema.");
            WebSerial.println("  monitor - Activa/desactiva el monitor serial.");
            WebSerial.println("  reset - Reinicia el dispositivo.");
            delay(10);
        }},
        {"estado", [this]() {
            WebSerial.println("Estado del sistema:");
            WebSerial.println(String("  OLED: ") + (_systemStatus->isDisplayEnabled() ? "encendida" : "apagada"));
            WebSerial.println(String("  Sensor: ") + getSensorValue("WaterLevel", "estado"));
            WebSerial.println(String("  Litros: ") + getSensorValue("WaterLevel", "litros"));
            WebSerial.println(String("  Distancia: ") + getSensorValue("WaterLevel", "distancia_cm"));
            delay(10);
        }},
        {"monitor", [this]() {
            bool newState = !_systemStatus->isMonitorEnabled();
            _systemStatus->setMonitorEnabled(newState);
            WebSerial.println(String("Monitor serial ") + (newState ? "activado" : "desactivado"));
            delay(10);
        }},
        {"reset", []() {
            WebSerial.println("Reiniciando...");
            delay(100);
            ESP.restart();
        }}
    };

    for (const auto& cmd : commands) {
        if (commandStr == cmd.name) {
            cmd.handler();
            delay(10); // Pequeña pausa para asegurar que se envíe la respuesta
            return;
        }
    }
    WebSerial.println("Comando no reconocido. Escribe 'ayuda' para ver la lista de comandos.");
    delay(10); // Pequeña pausa para asegurar que se envíe la respuesta
}

String WebManager::processor(const String& var) {
    if (var == "ESTADO_OLED") {
        return _systemStatus->isDisplayEnabled() ? "encendida" : "apagada";
    }
    if (var == "ESTADO_SENSOR") {
        return getSensorValue("WaterLevel", "estado");
    }
    if (var == "LITROS") {
        return getSensorValue("WaterLevel", "litros");
    }
    if (var == "DISTANCIA") {
        return getSensorValue("WaterLevel", "distancia_cm");
    }
    return String();
}

String WebManager::getSensorValue(const String& sensorName, const String& key) {
    String json = _sensorManager->getSensorJson(sensorName);
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return "Error";
    }
    return doc[key].as<String>();
}

String WebManager::getImageForLevel() {
    String litrosStr = getSensorValue("WaterLevel", "litros");
    String estadoStr = getSensorValue("WaterLevel", "estado");
    
    if (estadoStr != "OK") {
        return "imagen_error.jpg";
    }
    
    float litros = litrosStr.toFloat();
    float capacidad = _configManager->getCapacity();
    
    if (capacidad <= 0) {
        return "imagen_error.jpg";
    }
    
    float porcentaje = (litros / capacidad) * 100.0;
    
    if (porcentaje >= 90) {
        return "imagen_lleno.jpg";
    } else if (porcentaje >= 75) {
        return "imagen_4.jpg";
    } else if (porcentaje >= 50) {
        return "imagen_3.jpg";
    } else if (porcentaje >= 25) {
        return "imagen_2.jpg";
    } else if (porcentaje >= 10) {
        return "imagen_1.jpg";
    } else {
        return "imagen_vacio.jpg";
    }
}

void WebManager::setWiFiResetCallback(void (*callback)()) {
    _wifiResetCallback = callback;
}

// === ESP-NOW ENDPOINTS ===
void WebManager::setupESPNowEndpoints() {
    // Endpoint para obtener datos de todos los sensores ESP-NOW
    _server->on("/multi-sensor-data", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String jsonData = _espNowManager->getSensorsJSON();
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonData);
        response->addHeader("Cache-Control", "no-cache");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });
    
    // Endpoint para el estado de la red ESP-NOW
    _server->on("/espnow-status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String statusData = _espNowManager->getNetworkStatusJSON();
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", statusData);
        response->addHeader("Cache-Control", "no-cache");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });
    
    // Endpoint para enviar comando a sensor específico
    _server->on("/espnow-command", HTTP_POST, [this](AsyncWebServerRequest *request) {
        // Obtener parámetros
        String sensorId = request->getParam("sensorId", true)->value();
        String commandType = request->getParam("commandType", true)->value();
        String commandValue = request->getParam("commandValue", true)->value();
        
        // Crear comando
        SensorCommand cmd;
        cmd.targetSensorId = sensorId.toInt();
        cmd.commandType = commandType.toInt();
        cmd.commandValue = commandValue.toInt();
        cmd.timestamp = millis();
        
        // Enviar comando broadcast
        bool success = _espNowManager->sendCommandToAll(cmd);
        
        String response = success ? "{\"status\":\"sent\"}" : "{\"status\":\"failed\"}";
        request->send(200, "application/json", response);
    });
    
    // Endpoint para datos JSON consolidados (sensor local + ESP-NOW)
    _server->on("/data", HTTP_GET, [this](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(1024);
        
        // Datos del sensor local
        doc["litros"] = getSensorValue("WaterLevel", "litros");
        doc["distancia"] = getSensorValue("WaterLevel", "distancia_cm");
        doc["estadoSensor"] = "Conectado";
        doc["sensorId"] = 0; // Sensor principal
        doc["timestamp"] = millis();
        
        // Información ESP-NOW
        doc["espnowEnabled"] = _espNowManager->isInit();
        doc["connectedSensors"] = _espNowManager->getConnectedSensorCount();
        
        String jsonString;
        serializeJson(doc, jsonString);
        
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonString);
        response->addHeader("Cache-Control", "no-cache");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });
    
    // === NUEVOS ENDPOINTS PARA PORTAL CAUTIVO ESP-NOW ===
    
    // Obtener información automática del dispositivo
    _server->on("/api/esp-now/device-info", HTTP_GET, [this](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(512);
        
        // Obtener configuración ESP-NOW desde ConfigManager
        if (_configManager) {
            const auto& espnowConfig = _configManager->getESPNowConfig();
            
            doc["deviceMAC"] = espnowConfig.getDeviceMAC();
            doc["deviceChipID"] = espnowConfig.getDeviceChipID();
            doc["autoSensorID"] = espnowConfig.getAutoSensorID();
            doc["autoSensorName"] = espnowConfig.getAutoSensorName();
            doc["currentSensorID"] = espnowConfig.getSensorID();
            doc["currentSensorName"] = espnowConfig.getSensorName();
            doc["isMaster"] = espnowConfig.isMaster();
            doc["isSlave"] = espnowConfig.isSlave();
            doc["useAutoID"] = (espnowConfig.getSensorID() == espnowConfig.getAutoSensorID());
            doc["useAutoName"] = (espnowConfig.getSensorName() == espnowConfig.getAutoSensorName());
            doc["status"] = _espNowManager->isInit() ? "configured" : "not_configured";
        } else {
            // Fallback si no hay ConfigManager
            doc["deviceMAC"] = WiFi.macAddress();
            doc["deviceChipID"] = String(ESP.getEfuseMac(), HEX);
            doc["autoSensorID"] = 1;
            doc["autoSensorName"] = "Sensor-" + String((uint32_t)ESP.getEfuseMac(), HEX).substring(0, 6);
            doc["status"] = "not_configured";
        }
        
        String jsonString;
        serializeJson(doc, jsonString);
        
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonString);
        response->addHeader("Cache-Control", "no-cache");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });
    
    // Obtener configuración ESP-NOW actual
    _server->on("/api/esp-now/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(512);
        
        if (_configManager) {
            const auto& espnowConfig = _configManager->getESPNowConfig();
            espnowConfig.toJson(doc.to<JsonObject>());
        } else {
            doc["error"] = "ConfigManager no disponible";
        }
        
        String jsonString;
        serializeJson(doc, jsonString);
        
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", jsonString);
        response->addHeader("Cache-Control", "no-cache");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });
    
    // Obtener información ESP-NOW del dispositivo (endpoint original)
    _server->on("/api/esp-now/info", HTTP_GET, [this](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(512);
        doc["macAddress"] = WiFi.macAddress();
        doc["ipAddress"] = WiFi.localIP().toString();
        doc["status"] = _espNowManager->isInit() ? "initialized" : "not_configured";
        doc["role"] = _espNowManager->isMasterMode() ? "master" : "slave";
        doc["connectedDevices"] = _espNowManager->getConnectedSensorCount();
        doc["packetsSent"] = _espNowManager->getPacketsSent();
        doc["packetsReceived"] = _espNowManager->getPacketsReceived();
        doc["wifiConnected"] = WiFi.status() == WL_CONNECTED;
        doc["wifiSSID"] = WiFi.SSID();
        doc["wifiRSSI"] = WiFi.RSSI();
        doc["deviceName"] = "ESP32-WaterSensor";
        doc["firmwareVersion"] = "1.0.0";
        doc["uptime"] = millis();
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // Escanear dispositivos ESP-NOW cercanos
    _server->on("/api/esp-now/scan", HTTP_POST, [this](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(1024);
        JsonArray devices = doc.createNestedArray("devices");
        
        // Simular dispositivos encontrados (en implementación real, usar escáner)
        // Por ahora, devolver lista mock para pruebas
        for (int i = 1; i <= 3; i++) {
            JsonObject device = devices.createNestedObject();
            device["name"] = "ESP32_Sensor_" + String(i);
            device["mac"] = "24:0A:C4:XX:XX:" + String(10 + i, HEX);
            device["rssi"] = -30 - (i * 10);
            device["channel"] = 6;
        }
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // Conectar a dispositivo ESP-NOW
    _server->on("/api/esp-now/connect", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("plain", true)) {
            String body = request->getParam("plain", true)->value();
            DynamicJsonDocument requestDoc(256);
            deserializeJson(requestDoc, body);
            
            String macAddress = requestDoc["mac"];
            
            // Convertir MAC string a array de bytes
            uint8_t macBytes[6];
            sscanf(macAddress.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", 
                   &macBytes[0], &macBytes[1], &macBytes[2], 
                   &macBytes[3], &macBytes[4], &macBytes[5]);
            
            bool success = _espNowManager->addPeer(macBytes);
            
            DynamicJsonDocument responseDoc(128);
            responseDoc["success"] = success;
            responseDoc["message"] = success ? "Dispositivo conectado" : "Error al conectar";
            
            String response;
            serializeJson(responseDoc, response);
            request->send(200, "application/json", response);
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"No MAC provided\"}");
        }
    });
    
    // Ping a dispositivo ESP-NOW
    _server->on("/api/esp-now/ping", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("plain", true)) {
            String body = request->getParam("plain", true)->value();
            DynamicJsonDocument requestDoc(256);
            deserializeJson(requestDoc, body);
            
            String macAddress = requestDoc["mac"];
            
            // Simular ping (en implementación real, enviar comando ping)
            DynamicJsonDocument responseDoc(128);
            responseDoc["success"] = true;
            responseDoc["responseTime"] = random(10, 50);
            responseDoc["message"] = "Ping exitoso";
            
            String response;
            serializeJson(responseDoc, response);
            request->send(200, "application/json", response);
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"No MAC provided\"}");
        }
    });
    
    // Configurar ESP-NOW (Nuevo sistema dinámico)
    _server->on("/api/esp-now/configure", HTTP_POST, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("plain", true)) {
            String body = request->getParam("plain", true)->value();
            DynamicJsonDocument requestDoc(512);
            deserializeJson(requestDoc, body);
            
            bool success = false;
            String errorMsg = "";
            
            if (_configManager) {
                // Actualizar configuración ESP-NOW
                auto& espnowConfig = _configManager->getESPNowConfig();
                
                // Aplicar configuración desde JSON
                espnowConfig.fromJson(requestDoc.as<JsonObject>());
                
                // Validar configuración
                if (espnowConfig.isValid()) {
                    // Guardar configuración
                    success = _configManager->save();
                    
                    if (success) {
                        // Aplicar configuración a ESP-NOW
                        _espNowManager->setMasterMode(espnowConfig.isMaster());
                        
                        // Reinicializar ESP-NOW con nueva configuración
                        _espNowManager->deinitESPNow();
                        success = _espNowManager->initESPNow();
                        
                        if (!success) {
                            errorMsg = "Error inicializando ESP-NOW";
                        }
                    } else {
                        errorMsg = "Error guardando configuración";
                    }
                } else {
                    errorMsg = espnowConfig.getValidationError();
                }
            } else {
                errorMsg = "ConfigManager no disponible";
            }
            
            DynamicJsonDocument responseDoc(512);
            responseDoc["success"] = success;
            
            if (success) {
                responseDoc["message"] = "ESP-NOW configurado correctamente";
                responseDoc["sensorID"] = _configManager->getESPNowConfig().getSensorID();
                responseDoc["sensorName"] = _configManager->getESPNowConfig().getSensorName();
                responseDoc["role"] = _configManager->getESPNowConfig().isMaster() ? "master" : "slave";
                responseDoc["useAutoID"] = (_configManager->getESPNowConfig().getSensorID() == _configManager->getESPNowConfig().getAutoSensorID());
            } else {
                responseDoc["error"] = errorMsg;
            }
            
            String response;
            serializeJson(responseDoc, response);
            request->send(success ? 200 : 400, "application/json", response);
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"No config provided\"}");
        }
    });
    
    // Probar conexión ESP-NOW
    _server->on("/api/esp-now/test", HTTP_POST, [this](AsyncWebServerRequest *request) {
        bool isInitialized = _espNowManager->isInit();
        int connectedDevices = _espNowManager->getConnectedSensorCount();
        
        DynamicJsonDocument doc(256);
        doc["success"] = isInitialized;
        doc["devicesFound"] = connectedDevices;
        doc["status"] = isInitialized ? "active" : "inactive";
        doc["message"] = isInitialized ? "ESP-NOW funcionando correctamente" : "ESP-NOW no inicializado";
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
    
    // Obtener dispositivos conectados
    _server->on("/api/esp-now/connected", HTTP_GET, [this](AsyncWebServerRequest *request) {
        DynamicJsonDocument doc(128);
        doc["count"] = _espNowManager->getConnectedSensorCount();
        doc["timestamp"] = millis();
        
        String response;
        serializeJson(doc, response);
        request->send(200, "application/json", response);
    });
}
