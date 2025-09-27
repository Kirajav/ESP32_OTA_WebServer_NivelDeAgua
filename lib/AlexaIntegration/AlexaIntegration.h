#ifndef ALEXA_INTEGRATION_H
#define ALEXA_INTEGRATION_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "SensorManager.h"

class AlexaIntegration {
private:
    SensorManager* sensorManager;
    String skillId;
    bool enabled;
    
    String createAlexaResponse(const String& outputSpeech, bool shouldEndSession = true);
    String handleWaterLevelIntent();
    String handleDeviceStatusIntent();
    String handleTankCapacityIntent();
    
public:
    AlexaIntegration(SensorManager* sensor);
    
    void init(const String& skillId);
    void setEnabled(bool enable) { enabled = enable; }
    bool isEnabled() { return enabled; }
    
    String processAlexaRequest(const String& requestBody);
};

#endif