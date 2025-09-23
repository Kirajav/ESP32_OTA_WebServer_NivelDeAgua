#ifndef SYSTEM_STATUS_H
#define SYSTEM_STATUS_H

#include <Arduino.h>
#include "DisplayManager.h"
#include <ConfigManager.h>

// Variables para el control del LED dimmer
#define LED_CHANNEL 0          // Canal del LED para PWM
#define LED_FREQ 5000         // Frecuencia PWM en Hz
#define LED_RESOLUTION 8      // Resolución de 8 bits (0-255)
#define FADE_STEP 5          // Paso de atenuación
#define FADE_INTERVAL 30     // Intervalo de atenuación en ms

// Pins - these should be defined in a central place
#define PRG_BUTTON_PIN 0
#define LED_PIN 2

class SystemStatus {
public:
    SystemStatus(DisplayManager* displayManager, ConfigManager* configManager);
    void begin();
    void loop();
    bool isMonitorEnabled() const;
    void setMonitorEnabled(bool enabled);
    unsigned long getOledStartTime() const;
    void setOledStartTime(unsigned long time);
    unsigned long getOledTimeout() const;
    void setDisplayEnabled(bool enabled);
    void setUpdating(bool updating);
    bool isUpdating() const;
    void resetOledTimeout();
    bool isDisplayEnabled() const;

private:
    DisplayManager* _displayManager;
    ConfigManager* _configManager;
    void handlePrgButton();
    void updateLedDimmer();

    uint8_t ledValue = 0;
    bool fadeUp = true;
    unsigned long lastFadeTime = 0;
    bool _monitorEnabled = false;
    unsigned long _oledStartTime = 0;
    const long _oledTimeout = 300000;   // 5 minutos para timeout del OLED
    bool _isUpdating = false;
};

#endif // SYSTEM_STATUS_H