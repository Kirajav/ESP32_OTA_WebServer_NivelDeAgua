#ifndef GOOGLE_HOME_INTEGRATION_H
#define GOOGLE_HOME_INTEGRATION_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include "SensorManager.h"

class GoogleHomeIntegration {
private:
    SensorManager* sensorManager;
    String projectId;
    String deviceId;
    bool enabled;
    unsigned long lastUpdate;
    
    // Google Actions fulfillment
    String handleGoogleRequest(const String& requestBody);
    String createGoogleResponse(const String& intentName, const String& responseText);
    void reportDeviceState();
    
public:
    GoogleHomeIntegration(SensorManager* sensor);
    
    // Configuración
    void init(const String& projectId, const String& deviceId);
    void setEnabled(bool enable) { enabled = enable; }
    bool isEnabled() { return enabled; }
    
    // Manejo de requests de Google Assistant
    String processGoogleRequest(const String& requestBody);
    
    // Reportar estado del dispositivo
    void updateDeviceState();
    void reportWaterLevel(float level, float distance);
    
    // Comandos soportados
    String getWaterLevelResponse();
    String getDeviceStatusResponse();
    String getTankCapacityResponse();
};

#endif