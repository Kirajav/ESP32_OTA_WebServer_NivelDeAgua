#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <heltec.h>

class DisplayManager {
public:
    DisplayManager();
    void begin();
    void showCountdown(const char* message, int seconds);
    void handleDisplayStateChange(bool turnOn);
    void updateDisplay(const String& ip, const String& dist, const String& litros);
    void clear();
    void display();
    void setTextAlignment(int alignment);
    void setFont(const uint8_t* font);
    void drawString(int x, int y, const String& text);
    void displayOn();
    void displayOff();
    void drawProgressBar(int x, int y, int width, int height, int progress);
    
    // Control del LED blanco
    void setLedState(bool state);
    void startFadeEffect();
    void stopFadeEffect();
    void updateFadeEffect();

private:
    void showInitialMessage();
    
    // Variables para el LED
    bool _ledFadeActive;
    unsigned long _lastFadeUpdate;
    int _fadeDirection;
    int _fadeValue;
    static const int LED_PIN = 25;
    static const int LED_CHANNEL = 0;
    static const int LED_FREQ = 5000;
    static const int LED_RESOLUTION = 8;
};

#endif // DISPLAY_MANAGER_H