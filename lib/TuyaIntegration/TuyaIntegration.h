#ifndef TUYA_INTEGRATION_H
#define TUYA_INTEGRATION_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include "SensorManager.h"

class TuyaIntegration {
private:
    SensorManager* sensorManager;
    String productKey;
    String deviceSecret;
    String deviceId;
    bool enabled;
    unsigned long lastReport;
    
    void reportToTuya(const String& dataPoints);
    String createTuyaPayload(float waterLevel, float distance);
    
public:
    TuyaIntegration(SensorManager* sensor);
    
    void init(const String& productKey, const String& deviceSecret, const String& deviceId);
    void setEnabled(bool enable) { enabled = enable; }
    bool isEnabled() { return enabled; }
    
    void reportWaterLevel(float level, float distance);
    void reportDeviceStatus(const String& status);
    void updateTuyaCloud();
};

#endif