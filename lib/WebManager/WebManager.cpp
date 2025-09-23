#include "WebManager.h"
#include "SPIFFS.h"

// Estructura para definir comandos de WebSerial
struct WebSerialCommand {
    const char* name;
    std::function<void()> handler;
};

WebManager::WebManager(AsyncWebServer* server, SensorManager* sensorManager, ConfigManager* configManager, DisplayManager* displayManager, SystemStatus* systemStatus) :
_server(server), _sensorManager(sensorManager), _configManager(configManager), _displayManager(displayManager), _systemStatus(systemStatus), _ws("/ws"), _wifiResetCallback(nullptr) {
}

void WebManager::begin() {
    initWebSocket();
    initWebSerial();

    _server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html", false, [this](const String& var){ return processor(var); });
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
        _configManager->saveConfig();
        
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
                    _configManager->saveConfig();
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
    float capacidad = _configManager->getCapacidad();
    
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
