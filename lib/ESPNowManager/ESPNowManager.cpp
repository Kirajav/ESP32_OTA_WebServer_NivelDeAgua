#include "ESPNowManager.h"
#include "UnifiedLogger.h"
#include <esp_wifi.h>

// Definición de variables estáticas
ESPNowManager* ESPNowManager::instance = nullptr;
std::vector<SensorData> ESPNowManager::connectedSensors;
std::vector<std::array<uint8_t, 6>> ESPNowManager::knownPeers;
std::vector<DiscoveredDevice> ESPNowManager::discoveredDevices;

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
    
    if (WiFi.getMode() != WIFI_AP_STA) {
        WiFi.mode(WIFI_AP_STA);
    }
    
    if (esp_now_init() != ESP_OK) {
        UnifiedLogger::getInstance().error("Error inicializando ESP-NOW", "ESP-NOW");
        return false;
    }
    
    esp_now_register_recv_cb(onDataReceived);
    esp_now_register_send_cb(onDataSent);
    
    isInitialized = true;
    packetsSent = 0;
    packetsReceived = 0;
    
    UnifiedLogger::getInstance().info("ESP-NOW inicializado correctamente", "ESP-NOW");
    UnifiedLogger::getInstance().info("MAC Address: " + WiFi.macAddress(), "ESP-NOW");
    
    return true;
}

void ESPNowManager::deinitESPNow() {
    if (!isInitialized) return;
    esp_now_deinit();
    isInitialized = false;
    UnifiedLogger::getInstance().info("ESP-NOW desinicializado", "ESP-NOW");
}

// Callbacks
void ESPNowManager::onDataReceived(const uint8_t *mac, const uint8_t *data, int len) {
    ESPNowManager* mgr = getInstance();
    mgr->packetsReceived++;
    
    // Si recibimos datos, consideramos al peer "descubierto"
    bool isNewDiscovery = true;
    for (const auto& device : discoveredDevices) {
        if (memcmp(device.mac, mac, 6) == 0) {
            isNewDiscovery = false;
            break;
        }
    }
    if (isNewDiscovery) {
        DiscoveredDevice newDevice;
        memcpy(newDevice.mac, mac, 6);
        // Obtenemos el RSSI del paquete WiFi subyacente
        wifi_promiscuous_pkt_t* promiscuous_pkt = (wifi_promiscuous_pkt_t*)data;
        wifi_pkt_rx_ctrl_t& ctrl = promiscuous_pkt->rx_ctrl;
        newDevice.rssi = ctrl.rssi;
        newDevice.channel = ctrl.channel;
        discoveredDevices.push_back(newDevice);
        UnifiedLogger::getInstance().info("Nuevo dispositivo descubierto en escaneo: " + mgr->macToString(mac), "ESP-NOW");
    }

    if (len == sizeof(SensorData)) {
        SensorData sensorData;
        memcpy(&sensorData, data, sizeof(SensorData));
        mgr->processSensorData(sensorData, mac);
    } else if (len == sizeof(SensorCommand)) {
        SensorCommand command;
        memcpy(&command, data, sizeof(SensorCommand));
        mgr->processCommand(command, mac);
    }
}

void ESPNowManager::onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    ESPNowManager* mgr = getInstance();
    if (mgr->onSendCallback) {
        mgr->onSendCallback(mac_addr, status);
    }
}

// Procesamiento de datos
void ESPNowManager::processSensorData(const SensorData& data, const uint8_t* mac) {
    bool found = false;
    for (auto& sensor : connectedSensors) {
        if (sensor.sensorId == data.sensorId) {
            sensor = data;
            found = true;
            break;
        }
    }
    if (!found) {
        connectedSensors.push_back(data);
    }
    if (!isPeerKnown(mac)) {
        addKnownPeer(mac);
        addPeer(mac);
    }
    if (onSensorDataCallback) {
        onSensorDataCallback(data, mac);
    }
}

void ESPNowManager::processCommand(const SensorCommand& cmd, const uint8_t* mac) {
    // Lógica para procesar comandos recibidos
}

// Gestión de Peers
bool ESPNowManager::addPeer(const uint8_t* macAddress) {
    if (!isInitialized || esp_now_is_peer_exist(macAddress)) return false;
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, macAddress, 6);
    peerInfo.channel = 0; // 0 para usar el canal actual
    peerInfo.encrypt = false;
    esp_err_t result = esp_now_add_peer(&peerInfo);
    if (result == ESP_OK) {
        addKnownPeer(macAddress);
        UnifiedLogger::getInstance().info("Peer agregado: " + macToString(macAddress), "ESP-NOW");
        return true;
    }
    return false;
}

bool ESPNowManager::removePeer(const uint8_t* macAddress) {
    if (!isInitialized || !esp_now_is_peer_exist(macAddress)) return false;
    esp_err_t result = esp_now_del_peer(macAddress);
    if (result == ESP_OK) {
        // Eliminar de la lista de peers conocidos y sensores conectados
        knownPeers.erase(std::remove_if(knownPeers.begin(), knownPeers.end(), [&](const std::array<uint8_t, 6>& p) { return memcmp(p.data(), macAddress, 6) == 0; }), knownPeers.end());
        // Aquí se podría añadir lógica para eliminar el sensor de `connectedSensors` si se identifica por MAC
        UnifiedLogger::getInstance().info("Peer eliminado: " + macToString(macAddress), "ESP-NOW");
        return true;
    }
    return false;
}

// --- Implementación de Nuevas Funciones ---

void ESPNowManager::scanForPeers(uint32_t scanDuration) {
    UnifiedLogger::getInstance().info("Iniciando escaneo pasivo de ESP-NOW...", "ESP-NOW");
    discoveredDevices.clear();
    // Habilitar modo promiscuo para capturar todos los paquetes
    esp_wifi_set_promiscuous(true);
    // La lógica de descubrimiento está en onDataReceived. Aquí solo esperamos.
    // El handler de la web se encargará de la espera y de apagar el modo promiscuo.
}

String ESPNowManager::getDiscoveredDevicesJSON() {
    DynamicJsonDocument doc(1024);
    JsonArray devices = doc.createNestedArray("devices");
    for (const auto& device : discoveredDevices) {
        JsonObject deviceObj = devices.createNestedObject();
        deviceObj["mac"] = macToString(device.mac);
        deviceObj["rssi"] = device.rssi;
        deviceObj["channel"] = device.channel;
    }
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

bool ESPNowManager::pingPeer(const uint8_t* macAddress) {
    if (!isInitialized) return false;
    // Implementación simplificada: verifica si el peer está en la lista de ESP-NOW.
    // Una implementación real enviaría un paquete PING y esperaría un PONG.
    return esp_now_is_peer_exist(macAddress);
}

// --- Funciones existentes (sin cambios mayores) ---

String ESPNowManager::getSensorsJSON() {
    DynamicJsonDocument doc(2048);
    JsonArray sensorsArray = doc.createNestedArray("sensors");
    for (const auto& sensor : connectedSensors) {
        JsonObject sensorObj = sensorsArray.createNestedObject();
        sensorObj["id"] = sensor.sensorId;
        sensorObj["name"] = sensor.sensorName;
        sensorObj["waterLevel"] = sensor.waterLevel;
        sensorObj["distance"] = sensor.distance;
        sensorObj["liters"] = sensor.liters;
        sensorObj["signal"] = sensor.signalStrength;
        sensorObj["timestamp"] = sensor.timestamp;
    }
    doc["count"] = connectedSensors.size();
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
    doc["macAddress"] = WiFi.macAddress();
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

// ... resto de funciones de utilidad como macToString, etc. ...
String ESPNowManager::macToString(const uint8_t* mac) {
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(macStr);
}

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

uint8_t ESPNowManager::getConnectedSensorCount() {
    return connectedSensors.size();
}
