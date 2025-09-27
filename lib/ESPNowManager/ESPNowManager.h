#ifndef ESPNOW_MANAGER_H
#define ESPNOW_MANAGER_H

#include <esp_now.h>
#include <WiFi.h>
#include <vector>
#include <array>
#include <ArduinoJson.h>

// Estructura de datos del sensor
struct SensorData {
    uint8_t sensorId;           // ID único del sensor (1-255)
    float waterLevel;           // Nivel de agua en litros
    float distance;             // Distancia del sensor en cm
    uint8_t batteryPercent;     // Porcentaje de batería (0-100)
    uint32_t timestamp;         // Timestamp del dato
    char sensorName[32];        // Nombre del sensor
    uint8_t signalStrength;     // Fuerza de señal ESP-NOW (0-100)
};

// Estructura de comando
struct SensorCommand {
    uint8_t targetSensorId;     // ID del sensor objetivo
    uint8_t commandType;        // Tipo de comando
    uint8_t commandValue;       // Valor del comando
    uint32_t timestamp;         // Timestamp del comando
};

// Estructura para sincronización de tiempo NTP
struct TimeSyncData {
    uint32_t masterTimestamp;   // Timestamp del master (NTP)
    uint32_t localTimestamp;    // Timestamp local del master
    int8_t timezoneOffset;      // Offset de zona horaria
    bool isNTPSynced;           // Si el master está sincronizado con NTP
    uint8_t signalQuality;      // Calidad de la sincronización (0-100)
};

// Tipos de comando
enum CommandType {
    CMD_PING = 1,               // Ping al sensor
    CMD_REQUEST_DATA = 2,       // Solicitar datos
    CMD_SET_INTERVAL = 3,       // Configurar intervalo de envío
    CMD_CALIBRATE = 4,          // Calibrar sensor
    CMD_SLEEP = 5,              // Modo sleep
    CMD_WAKE = 6,               // Despertar sensor
    CMD_RESET = 7,              // Reset sensor
    CMD_TIME_SYNC = 8           // Sincronización de tiempo NTP
};

class ESPNowManager {
private:
    static ESPNowManager* instance;
    static std::vector<SensorData> connectedSensors;
    static std::vector<std::array<uint8_t, 6>> knownPeers;
    
    // Callbacks estáticos
    static void onDataReceived(const uint8_t *mac, const uint8_t *data, int len);
    static void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
    
    // Funciones privadas
    void processSensorData(const SensorData& data, const uint8_t* mac);
    void processCommand(const SensorCommand& cmd, const uint8_t* mac);
    bool isPeerKnown(const uint8_t* mac);
    void addKnownPeer(const uint8_t* mac);
    
    // Estado
    bool isInitialized = false;
    bool isMaster = true;  // Este ESP32 es el master
    uint32_t lastBroadcast = 0;
    uint16_t packetsSent = 0;
    uint16_t packetsReceived = 0;
    
public:
    // Singleton
    static ESPNowManager* getInstance();
    
    // Inicialización
    bool initESPNow();
    void deinitESPNow();
    bool isInit() { return isInitialized; }
    
    // Gestión de peers
    bool addPeer(const uint8_t* macAddress);
    bool removePeer(const uint8_t* macAddress);
    void removeAllPeers();
    
    // Envío de datos
    bool broadcastData(const SensorData& data);
    bool sendDataToPeer(const uint8_t* macAddress, const SensorData& data);
    bool sendCommand(const uint8_t* macAddress, const SensorCommand& cmd);
    bool sendCommandToAll(const SensorCommand& cmd);
    
    // Sincronización de tiempo NTP
    bool broadcastTimeSync(const TimeSyncData& timeData);
    bool sendTimeSyncToPeer(const uint8_t* macAddress, const TimeSyncData& timeData);
    void requestTimeSync();  // Para slaves que solicitan sincronización
    
    // Recepción y gestión de datos
    std::vector<SensorData> getConnectedSensors();
    SensorData* getSensorById(uint8_t sensorId);
    void clearOldSensors(uint32_t maxAge = 60000); // 60 segundos por defecto
    
    // Estadísticas y diagnóstico
    uint16_t getPacketsSent() { return packetsSent; }
    uint16_t getPacketsReceived() { return packetsReceived; }
    uint8_t getConnectedSensorCount();
    String getNetworkStatus();
    
    // Utilidades
    String macToString(const uint8_t* mac);
    void printMac(const uint8_t* mac);
    bool stringToMac(const String& macStr, uint8_t* mac);
    
    // JSON para web interface
    String getSensorsJSON();
    String getNetworkStatusJSON();
    
    // Configuración
    void setMasterMode(bool master) { isMaster = master; }
    bool isMasterMode() { return isMaster; }
    
    // Callbacks para el usuario
    std::function<void(const SensorData&, const uint8_t*)> onSensorDataCallback;
    std::function<void(const uint8_t*, esp_now_send_status_t)> onSendCallback;
};

#endif // ESPNOW_MANAGER_H