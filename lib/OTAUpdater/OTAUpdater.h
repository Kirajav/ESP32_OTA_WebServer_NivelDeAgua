#ifndef OTA_UPDATER_H
#define OTA_UPDATER_H

#include <Arduino.h>
#include <ElegantOTA.h>
#include "DisplayManager.h"
#include "SystemStatus.h"

class OTAUpdater {
public:
    OTAUpdater(DisplayManager* displayManager, SystemStatus* systemStatus);
    void begin(AsyncWebServer* server);
    void loop();

private:
    DisplayManager* _displayManager;
    SystemStatus* _systemStatus;
};

#endif // OTA_UPDATER_H
