#ifndef TUYA_DEVICE_H
#define TUYA_DEVICE_H

#include <WiFi.h>
#include <ArduinoJson.h>

/**
 * @brief Tuya Device Integration (Comercial Style)
 * 
 * Esta clase permite que tu ESP32 aparezca automáticamente en la App Tuya
 * como cualquier dispositivo comercial, SIN necesidad de configurar Tuya Cloud
 * manualmente. Usa el protocolo estándar que usan los dispositivos comerciales.
 */
class TuyaDevice {
public:
    enum DeviceType {
        SENSOR = 0,
        SWITCH = 1,
        DIMMER = 2
    };

    struct DeviceConfig {
        String deviceName = "Water Level Sensor";
        String deviceModel = "ESP32-WLS-V1";
        String firmwareVersion = "1.0.0";
        DeviceType type = SENSOR;
        String category = "cz"; // Water sensor category in Tuya
    };

    TuyaDevice();
    
    // Configuración inicial
    void begin(const DeviceConfig& config);
    void setWiFiCredentials(const String& ssid, const String& password);
    
    // Estados del dispositivo
    void updateSensorValue(const String& dpId, float value);
    void updateStatus(const String& dpId, bool status);
    void updateString(const String& dpId, const String& value);
    
    // Callbacks para comandos recibidos
    void onCommand(std::function<void(String dpId, String value)> callback);
    
    // Loop principal
    void loop();
    
    // Estado de conexión
    bool isConnected();
    bool isConfigured();
    
    // Modo configuración (como dispositivos comerciales)
    void enterConfigMode(); // Crear AP para configuración inicial
    bool isInConfigMode();

private:
    DeviceConfig _config;
    String _deviceId;
    String _localKey;
    bool _connected;
    bool _configured;
    bool _configMode;
    
    WiFiServer _configServer;
    WiFiClient* client;
    std::function<void(String, String)> _commandCallback;
    
    // Estado del dispositivo
    struct DeviceStatus {
        bool waterLevel = false;
        bool tankFull = false;
        bool alertEnabled = true;
    } deviceStatus;
    
    void generateDeviceCredentials();
    void handleConfigServer();
    void connectToTuyaCloud();
    void processIncomingCommands();
    void sendHeartbeat();
    void registerDevice();
    
    // Protocol helpers
    String encryptMessage(const String& message);
    String decryptMessage(const String& message);
    String createDataPoint(const String& dpId, const String& value, const String& type);
    String extractFormValue(const String& body, const String& key);
};

#endif