#include "ESPNowManager.h"
#include "UnifiedLogger.h"

// Variables estáticas
ESPNowManager* ESPNowManager::instance = nullptr;
std::vector<SensorData> ESPNowManager::connectedSensors;
std::vector<std::array<uint8_t, 6>> ESPNowManager::knownPeers;

// Singleton
ESPNowManager* ESPNowManager::getInstance() {
    if (instance == nullptr) {
        instance = new ESPNowManager();
    }
    return instance;
}

// Inicialización
bool ESPNowManager::initESPNow() {
    if (isInitialized) {
        UnifiedLogger::getInstance().warning("ESP-NOW ya inicializado", "ESP-NOW");
        return true;
    }
    
    UnifiedLogger::getInstance().info("Inicializando ESP-NOW...", "ESP-NOW");
    
    // MANTENER modo AP_STA (no sobrescribir si ya está configurado)
    if (WiFi.getMode() != WIFI_AP_STA) {
        WiFi.mode(WIFI_AP_STA);
    }
    
    // Si WiFi está conectado, usar el mismo canal para ESP-NOW
    if (WiFi.status() == WL_CONNECTED) {
        int wifiChannel = WiFi.channel();
        UnifiedLogger::getInstance().info("Sincronizando ESP-NOW con canal WiFi: " + String(wifiChannel), "ESP-NOW");
        // El ESP-NOW automáticamente usa el mismo canal que WiFi cuando están en modo STA
    }
    
    // Inicializar ESP-NOW
    if (esp_now_init() != ESP_OK) {
        UnifiedLogger::getInstance().error("Error inicializando ESP-NOW", "ESP-NOW");
        return false;
    }
    
    // Registrar callbacks
    esp_now_register_recv_cb(onDataReceived);
    esp_now_register_send_cb(onDataSent);
    
    isInitialized = true;
    packetsSent = 0;
    packetsReceived = 0;
    
    UnifiedLogger::getInstance().info("ESP-NOW inicializado correctamente", "ESP-NOW");
    UnifiedLogger::getInstance().info("MAC Address: " + WiFi.macAddress(), "ESP-NOW");
    
    // Mostrar estado de coexistencia WiFi + ESP-NOW
    if (WiFi.status() == WL_CONNECTED) {
        UnifiedLogger::getInstance().info("✅ Coexistencia WiFi + ESP-NOW activa", "ESP-NOW");
        UnifiedLogger::getInstance().info("📶 WiFi IP: " + WiFi.localIP().toString(), "ESP-NOW");
        UnifiedLogger::getInstance().info("🔗 ESP-NOW MAC: " + WiFi.macAddress(), "ESP-NOW");
        UnifiedLogger::getInstance().info("📡 Canal sincronizado: " + String(WiFi.channel()), "ESP-NOW");
    } else {
        UnifiedLogger::getInstance().info("📡 ESP-NOW modo standalone (sin WiFi)", "ESP-NOW");
    }
    
    return true;
}

void ESPNowManager::deinitESPNow() {
    if (!isInitialized) return;
    
    esp_now_deinit();
    connectedSensors.clear();
    knownPeers.clear();
    isInitialized = false;
    
    UnifiedLogger::getInstance().info("ESP-NOW desinicializado", "ESP-NOW");
}

// Callback de datos recibidos
void ESPNowManager::onDataReceived(const uint8_t *mac, const uint8_t *data, int len) {
    ESPNowManager* mgr = getInstance();
    mgr->packetsReceived++;
    
    UnifiedLogger::getInstance().debug("Datos ESP-NOW recibidos de: " + mgr->macToString(mac), "ESP-NOW");
    
    // Verificar tamaño
    if (len == sizeof(SensorData)) {
        // Es un dato de sensor
        SensorData sensorData;
        memcpy(&sensorData, data, sizeof(SensorData));
        mgr->processSensorData(sensorData, mac);
        
    } else if (len == sizeof(SensorCommand)) {
        // Es un comando
        SensorCommand command;
        memcpy(&command, data, sizeof(SensorCommand));
        mgr->processCommand(command, mac);
        
    } else {
        UnifiedLogger::getInstance().warning("Datos ESP-NOW con tamaño inválido: " + String(len), "ESP-NOW");
    }
}

// Callback de datos enviados
void ESPNowManager::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    ESPNowManager* mgr = getInstance();
    
    String statusStr = (status == ESP_NOW_SEND_SUCCESS) ? "SUCCESS" : "FAIL";
    UnifiedLogger::getInstance().debug("ESP-NOW enviado a " + mgr->macToString(mac_addr) + ": " + statusStr, "ESP-NOW");
    
    if (mgr->onSendCallback) {
        mgr->onSendCallback(mac_addr, status);
    }
}

// Procesar datos de sensor
void ESPNowManager::processSensorData(const SensorData& data, const uint8_t* mac) {
    UnifiedLogger::getInstance().debug("Procesando datos del sensor ID: " + String(data.sensorId), "ESP-NOW");
    
    // Buscar sensor existente
    bool found = false;
    for (auto& sensor : connectedSensors) {
        if (sensor.sensorId == data.sensorId) {
            sensor = data; // Actualizar datos
            found = true;
            break;
        }
    }
    
    // Si no existe, agregarlo
    if (!found) {
        connectedSensors.push_back(data);
        UnifiedLogger::getInstance().info("Nuevo sensor agregado: " + String(data.sensorName), "ESP-NOW");
    }
    
    // Agregar peer si no es conocido
    if (!isPeerKnown(mac)) {
        addKnownPeer(mac);
        addPeer(mac);
    }
    
    // Callback del usuario
    if (onSensorDataCallback) {
        onSensorDataCallback(data, mac);
    }
}

// Procesar comando
void ESPNowManager::processCommand(const SensorCommand& cmd, const uint8_t* mac) {
    UnifiedLogger::getInstance().debug("Comando recibido tipo: " + String(cmd.commandType), "ESP-NOW");
    
    // Si somos slave, procesar comandos del master
    if (!isMaster) {
        switch (cmd.commandType) {
            case CMD_PING:
                // Responder con nuestros datos
                {
                    SensorData response;
                    response.sensorId = 1; // ID de este sensor
                    response.timestamp = millis();
                    strcpy(response.sensorName, "ESP32-Main");
                    sendDataToPeer(mac, response);
                }
                break;
                
            case CMD_REQUEST_DATA:
                // Enviar datos actuales
                // Implementar según necesidades
                break;
                
            case CMD_RESET:
                UnifiedLogger::getInstance().warning("Comando RESET recibido", "ESP-NOW");
                ESP.restart();
                break;
        }
    }
}

// Gestión de peers
bool ESPNowManager::addPeer(const uint8_t* macAddress) {
    if (!isInitialized) return false;
    
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, macAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    
    esp_err_t result = esp_now_add_peer(&peerInfo);
    if (result == ESP_OK) {
        UnifiedLogger::getInstance().info("Peer agregado: " + macToString(macAddress), "ESP-NOW");
        return true;
    } else {
        UnifiedLogger::getInstance().error("Error agregando peer: " + String(result), "ESP-NOW");
        return false;
    }
}

bool ESPNowManager::removePeer(const uint8_t* macAddress) {
    if (!isInitialized) return false;
    
    esp_err_t result = esp_now_del_peer(macAddress);
    return (result == ESP_OK);
}

void ESPNowManager::removeAllPeers() {
    // Obtener lista de peers y eliminarlos
    esp_now_peer_info_t peer;
    esp_now_peer_num_t peerNum;
    esp_now_get_peer_num(&peerNum);
    
    for (int i = 0; i < peerNum.total_num; i++) {
        if (esp_now_fetch_peer(true, &peer) == ESP_OK) {
            esp_now_del_peer(peer.peer_addr);
        }
    }
    
    knownPeers.clear();
}

// Envío de datos
bool ESPNowManager::broadcastData(const SensorData& data) {
    if (!isInitialized) return false;
    
    uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&data, sizeof(SensorData));
    
    if (result == ESP_OK) {
        packetsSent++;
        lastBroadcast = millis();
        return true;
    }
    
    return false;
}

bool ESPNowManager::sendDataToPeer(const uint8_t* macAddress, const SensorData& data) {
    if (!isInitialized) return false;
    
    esp_err_t result = esp_now_send(macAddress, (uint8_t*)&data, sizeof(SensorData));
    
    if (result == ESP_OK) {
        packetsSent++;
        return true;
    }
    
    return false;
}

bool ESPNowManager::sendCommand(const uint8_t* macAddress, const SensorCommand& cmd) {
    if (!isInitialized) return false;
    
    esp_err_t result = esp_now_send(macAddress, (uint8_t*)&cmd, sizeof(SensorCommand));
    
    if (result == ESP_OK) {
        packetsSent++;
        return true;
    }
    
    return false;
}

bool ESPNowManager::sendCommandToAll(const SensorCommand& cmd) {
    if (!isInitialized) return false;
    
    uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*)&cmd, sizeof(SensorCommand));
    
    if (result == ESP_OK) {
        packetsSent++;
        return true;
    }
    
    return false;
}

// Gestión de datos
std::vector<SensorData> ESPNowManager::getConnectedSensors() {
    return connectedSensors;
}

SensorData* ESPNowManager::getSensorById(uint8_t sensorId) {
    for (auto& sensor : connectedSensors) {
        if (sensor.sensorId == sensorId) {
            return &sensor;
        }
    }
    return nullptr;
}

void ESPNowManager::clearOldSensors(uint32_t maxAge) {
    uint32_t currentTime = millis();
    
    connectedSensors.erase(
        std::remove_if(connectedSensors.begin(), connectedSensors.end(),
            [currentTime, maxAge](const SensorData& sensor) {
                return (currentTime - sensor.timestamp) > maxAge;
            }),
        connectedSensors.end()
    );
}

uint8_t ESPNowManager::getConnectedSensorCount() {
    return connectedSensors.size();
}

// Utilidades
String ESPNowManager::macToString(const uint8_t* mac) {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(macStr);
}

void ESPNowManager::printMac(const uint8_t* mac) {
    Serial.println(macToString(mac));
}

bool ESPNowManager::stringToMac(const String& macStr, uint8_t* mac) {
    int values[6];
    if (sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x", 
               &values[0], &values[1], &values[2], 
               &values[3], &values[4], &values[5]) == 6) {
        for (int i = 0; i < 6; i++) {
            mac[i] = (uint8_t)values[i];
        }
        return true;
    }
    return false;
}

// JSON para web interface
String ESPNowManager::getSensorsJSON() {
    DynamicJsonDocument doc(2048);
    JsonArray sensorsArray = doc.createNestedArray("sensors");
    
    for (const auto& sensor : connectedSensors) {
        JsonObject sensorObj = sensorsArray.createNestedObject();
        sensorObj["id"] = sensor.sensorId;
        sensorObj["name"] = sensor.sensorName;
        sensorObj["waterLevel"] = sensor.waterLevel;
        sensorObj["distance"] = sensor.distance;
        sensorObj["battery"] = sensor.batteryPercent;
        sensorObj["signal"] = sensor.signalStrength;
        sensorObj["timestamp"] = sensor.timestamp;
        sensorObj["age"] = millis() - sensor.timestamp;
    }
    
    doc["count"] = connectedSensors.size();
    doc["lastUpdate"] = millis();
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

String ESPNowManager::getNetworkStatusJSON() {
    DynamicJsonDocument doc(1024);
    
    doc["initialized"] = isInitialized;
    doc["isMaster"] = isMaster;
    doc["packetsSent"] = packetsSent;
    doc["packetsReceived"] = packetsReceived;
    doc["connectedSensors"] = connectedSensors.size();
    doc["lastBroadcast"] = lastBroadcast;
    doc["macAddress"] = WiFi.macAddress();
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

String ESPNowManager::getNetworkStatus() {
    String status = "ESP-NOW Status:\n";
    status += "Initialized: " + String(isInitialized ? "Yes" : "No") + "\n";
    status += "Mode: " + String(isMaster ? "Master" : "Slave") + "\n";
    status += "Sensors: " + String(connectedSensors.size()) + "\n";
    status += "Packets Sent: " + String(packetsSent) + "\n";
    status += "Packets Received: " + String(packetsReceived) + "\n";
    status += "MAC: " + WiFi.macAddress() + "\n";
    
    return status;
}

// Funciones auxiliares
bool ESPNowManager::isPeerKnown(const uint8_t* mac) {
    for (const auto& knownMac : knownPeers) {
        if (memcmp(knownMac.data(), mac, 6) == 0) {
            return true;
        }
    }
    return false;
}

void ESPNowManager::addKnownPeer(const uint8_t* mac) {
    if (!isPeerKnown(mac)) {
        std::array<uint8_t, 6> newMac;
        memcpy(newMac.data(), mac, 6);
        knownPeers.push_back(newMac);
    }
}