#include "SystemStatus.h"

SystemStatus::SystemStatus(DisplayManager* displayManager, ConfigManager* configManager) : 
_displayManager(displayManager), _configManager(configManager) {
}

void SystemStatus::begin() {
    pinMode(PRG_BUTTON_PIN, INPUT_PULLUP);  // Configurar botón PRG
    pinMode(LED_PIN, OUTPUT);               // Configurar LED
    ledcSetup(LED_CHANNEL, LED_FREQ, LED_RESOLUTION);
    ledcAttachPin(LED_PIN, LED_CHANNEL);
}

void SystemStatus::loop() {
    handlePrgButton();
    updateLedDimmer();
}

bool SystemStatus::isMonitorEnabled() const {
    return _monitorEnabled;
}

void SystemStatus::setMonitorEnabled(bool enabled) {
    _monitorEnabled = enabled;
}

unsigned long SystemStatus::getOledStartTime() const {
    return _oledStartTime;
}

void SystemStatus::setOledStartTime(unsigned long time) {
    _oledStartTime = time;
}

unsigned long SystemStatus::getOledTimeout() const {
    return _oledTimeout;
}

void SystemStatus::setDisplayEnabled(bool enabled) {
    _configManager->setDisplayOn(enabled);
    if (enabled) {
        _oledStartTime = millis();
    }
}

void SystemStatus::setUpdating(bool updating) {
    _isUpdating = updating;
}

bool SystemStatus::isUpdating() const {
    return _isUpdating;
}

void SystemStatus::resetOledTimeout() {
    _oledStartTime = millis();
}

bool SystemStatus::isDisplayEnabled() const {
    return _configManager->isDisplayOn();
}

void SystemStatus::handlePrgButton() {
    static unsigned long lastDebounceTime = 0;
    static bool lastButtonState = HIGH;
    bool buttonState = digitalRead(PRG_BUTTON_PIN);

    if (buttonState != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > 50) {
        if (buttonState == LOW) {
            bool currentDisplayState = _configManager->isDisplayOn();
            setDisplayEnabled(!currentDisplayState);
            _displayManager->handleDisplayStateChange(!currentDisplayState);
            _configManager->saveConfig();
        }
    }
    lastButtonState = buttonState;
}

void SystemStatus::updateLedDimmer() {
    if (_isUpdating && millis() - lastFadeTime > FADE_INTERVAL) {
        lastFadeTime = millis();
        
        if (fadeUp) {
            ledValue += FADE_STEP;
            if (ledValue >= 255) {
                ledValue = 255;
                fadeUp = false;
            }
        } else {
            ledValue -= FADE_STEP;
            if (ledValue <= 0) {
                ledValue = 0;
                fadeUp = true;
            }
        }
        ledcWrite(LED_CHANNEL, ledValue);
    }
}