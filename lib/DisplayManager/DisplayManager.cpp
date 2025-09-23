#include "DisplayManager.h"

// Pin definitions for Heltec WiFi Kit 32
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define Vext 21

DisplayManager::DisplayManager() : _ledFadeActive(false), _lastFadeUpdate(0), _fadeDirection(1), _fadeValue(0) {}

void DisplayManager::begin() {
    // Configurar LED blanco
    ledcSetup(LED_CHANNEL, LED_FREQ, LED_RESOLUTION);
    ledcAttachPin(LED_PIN, LED_CHANNEL);
    setLedState(false); // Empezar apagado
    
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW); // Encender OLED
    delay(100);

    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(50);
    digitalWrite(OLED_RST, HIGH);
    delay(50);

    Wire.begin(OLED_SDA, OLED_SCL);

    Heltec.begin(true /*DisplayEnable*/, false /*LoRa*/, true /*Serial*/);
    Heltec.display->init();
    Heltec.display->flipScreenVertically();
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->clear();
    showInitialMessage();
    
    // LED encendido con la pantalla
    setLedState(true);
}

void DisplayManager::showInitialMessage() {
    Heltec.display->clear();
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->drawString(64, 0, F("SENSOR DE"));
    Heltec.display->drawString(64, 16, F("NIVEL DE AGUA"));
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->drawString(0, 35, F("Iniciando WiFi..."));
    Heltec.display->display();
}

void DisplayManager::showCountdown(const char* message, int seconds) {
    Heltec.display->clear();
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->drawString(64, 10, message);
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(64, 30, String(seconds) + " segundos...");
    Heltec.display->display();
}

void DisplayManager::handleDisplayStateChange(bool turnOn) {
    if (turnOn) {
        Heltec.display->displayOn();
        setLedState(true);
    } else {
        for (int i = 3; i > 0; i--) {
            showCountdown("Apagando display", i);
            delay(1000);
        }
        Heltec.display->displayOff();
        setLedState(false);
    }
}

void DisplayManager::updateDisplay(const String& ip, const String& dist, const String& litros) {
    Heltec.display->clear();
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->drawString(64, 0, "Sensor de Nivel");
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->drawString(0, 18, "IP: " + ip);
    Heltec.display->drawString(0, 30, "Dist: " + dist + " cm");
    Heltec.display->drawString(0, 42, "Litros: " + litros + " L");
    Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
    Heltec.display->drawString(128, 54, "by DataTech");
    Heltec.display->display();
}

void DisplayManager::clear() {
    Heltec.display->clear();
}

void DisplayManager::display() {
    Heltec.display->display();
}

void DisplayManager::setTextAlignment(int alignment) {
    Heltec.display->setTextAlignment((DISPLAY_TEXT_ALIGNMENT)alignment);
}

void DisplayManager::setFont(const uint8_t* font) {
    Heltec.display->setFont(font);
}

void DisplayManager::drawString(int x, int y, const String& text) {
    Heltec.display->drawString(x, y, text);
}

void DisplayManager::displayOn() {
    Heltec.display->displayOn();
    setLedState(true);
}

void DisplayManager::displayOff() {
    Heltec.display->displayOff();
    setLedState(false);
}

void DisplayManager::drawProgressBar(int x, int y, int width, int height, int progress) {
    Heltec.display->drawProgressBar(x, y, width, height, progress);
}

// Control del LED blanco
void DisplayManager::setLedState(bool state) {
    stopFadeEffect(); // Detener fade si está activo
    if (state) {
        ledcWrite(LED_CHANNEL, 255); // Encender al máximo
    } else {
        ledcWrite(LED_CHANNEL, 0); // Apagar completamente
    }
}

void DisplayManager::startFadeEffect() {
    _ledFadeActive = true;
    _fadeValue = 0;
    _fadeDirection = 1;
    _lastFadeUpdate = millis();
    Serial.println("=== INICIANDO EFECTO FADE LED ===");
}

void DisplayManager::stopFadeEffect() {
    if (_ledFadeActive) {
        _ledFadeActive = false;
        Serial.println("=== DETENIENDO EFECTO FADE LED ===");
    }
}

void DisplayManager::updateFadeEffect() {
    if (!_ledFadeActive) return;
    
    unsigned long currentTime = millis();
    if (currentTime - _lastFadeUpdate >= 10) { // Actualizar cada 10ms
        _lastFadeUpdate = currentTime;
        
        _fadeValue += _fadeDirection * 3; // Velocidad del fade
        
        if (_fadeValue <= 0) {
            _fadeValue = 0;
            _fadeDirection = 1;
        } else if (_fadeValue >= 255) {
            _fadeValue = 255;
            _fadeDirection = -1;
        }
        
        ledcWrite(LED_CHANNEL, _fadeValue);
    }
}