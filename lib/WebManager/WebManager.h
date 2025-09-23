#ifndef WEB_MANAGER_H
#define WEB_MANAGER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>
#include <ArduinoJson.h>
#include "SensorManager.h"
#include "ConfigManager.h"
#include "DisplayManager.h"
#include "SystemStatus.h"

class WebManager {
public:
    WebManager(AsyncWebServer* server, SensorManager* sensorManager, ConfigManager* configManager, DisplayManager* displayManager, SystemStatus* systemStatus);
    void begin();
    void setWiFiResetCallback(void (*callback)());  // Nuevo callback para reset WiFi

private:
    AsyncWebServer* _server;
    SensorManager* _sensorManager;
    ConfigManager* _configManager;
    DisplayManager* _displayManager;
    SystemStatus* _systemStatus;
    AsyncWebSocket _ws;
    void (*_wifiResetCallback)();  // Callback para reset WiFi

    void initWebSocket();
    void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
    void initWebSerial();
    void onWebSerialMessage(uint8_t *data, size_t len);
    String processor(const String& var);
    String getSensorValue(const String& sensorName, const String& key);
    String getImageForLevel();
};

#endif // WEB_MANAGER_H
