#include "DisplayManager.h"

// Pin definitions for Heltec WiFi Kit 32
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define Vext 21

DisplayManager::DisplayManager() : 
    _ledFadeActive(false), _lastFadeUpdate(0), _fadeDirection(1), _fadeValue(0), 
    _autoSleepTime(30), _lastActivity(0), _displaySleeping(false), _autoSleepEnabled(false),
    _countdownActive(false), _countdownSeconds(0), _countdownStart(0), 
    _lastButtonState(HIGH), _lastButtonPress(0) {}

void DisplayManager::begin() {
    // Configurar LED blanco
    ledcSetup(LED_CHANNEL, LED_FREQ, LED_RESOLUTION);
    ledcAttachPin(LED_PIN, LED_CHANNEL);
    setLedState(false); // Empezar apagado
    
    // Configurar botón PRG con pull-up interno
    pinMode(PRG_BUTTON_PIN, INPUT_PULLUP);
    
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
    
    // Mostrar splash screen por 3 segundos
    showSplashScreen();
    delay(3000);
    
    showInitialMessage();
    
    // LED encendido con la pantalla
    setLedState(true);
    
    // Inicializar timer de auto-sleep
    _lastActivity = millis();
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
        // Mostrar confirmación de encendido brevemente
        Heltec.display->clear();
        Heltec.display->setFont(ArialMT_Plain_16);
        Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
        Heltec.display->drawString(64, 20, "Pantalla ON");
        Heltec.display->display();
        delay(1000);
    } else {
        for (int i = 3; i > 0; i--) {
            showCountdown("Apagando pantalla", i);
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

// Auto-sleep control methods
void DisplayManager::setAutoSleepTime(uint16_t seconds) {
    _autoSleepTime = seconds;
    Serial.printf("⏰ Auto-sleep configurado: %d segundos\n", seconds);
}

uint16_t DisplayManager::getAutoSleepTime() const {
    return _autoSleepTime;
}

void DisplayManager::enableAutoSleep(bool enable) {
    _autoSleepEnabled = enable;
    if (enable) {
        _lastActivity = millis();
        Serial.println("⏰ Auto-sleep HABILITADO (modo sensor)");
    } else {
        _countdownActive = false;
        Serial.println("⏰ Auto-sleep DESHABILITADO (modo portal)");
    }
}

void DisplayManager::checkAutoSleep() {
    if (!_autoSleepEnabled || _displaySleeping) return;
    
    unsigned long currentTime = millis();
    unsigned long timeSinceActivity = currentTime - _lastActivity;
    
    // Si estamos en countdown activo
    if (_countdownActive) {
        unsigned long countdownElapsed = (currentTime - _countdownStart) / 1000;
        int remainingSeconds = 3 - countdownElapsed;
        
        if (remainingSeconds != _countdownSeconds) {
            _countdownSeconds = remainingSeconds;
            
            if (_countdownSeconds > 0) {
                // Mostrar countdown
                Heltec.display->clear();
                Heltec.display->setFont(ArialMT_Plain_16);
                Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
                Heltec.display->drawString(64, 15, F("Apagando pantalla"));
                Heltec.display->drawString(64, 35, "en " + String(_countdownSeconds) + "...");
                Heltec.display->display();
            } else {
                // Apagar pantalla
                Serial.println("� Pantalla apagada por auto-sleep");
                displayOff();
                setLedState(false);
                _displaySleeping = true;
                _countdownActive = false;
            }
        }
    } else {
        // Verificar si es tiempo de iniciar countdown
        if (timeSinceActivity >= (_autoSleepTime - 3) * 1000) {
            _countdownActive = true;
            _countdownStart = currentTime;
            _countdownSeconds = 3;
            Serial.println("⏰ Iniciando countdown de auto-sleep");
        }
    }
}

void DisplayManager::wakeUpDisplay() {
    if (_displaySleeping) {
        displayOn();
        setLedState(true);
        _displaySleeping = false;
        _countdownActive = false;
        _countdownSeconds = 0;
        _lastActivity = millis(); // Reiniciar timer
        Serial.println("🔆 Pantalla despertada manualmente");
        
        // Limpiar pantalla para evitar mostrar mensaje de "Apagando..."
        Heltec.display->clear();
        Heltec.display->display();
    }
}

void DisplayManager::forceDisplaySleep() {
    if (!_displaySleeping) {
        // Mostrar countdown rápido
        for (int i = 3; i > 0; i--) {
            Heltec.display->clear();
            Heltec.display->setFont(ArialMT_Plain_16);
            Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
            Heltec.display->drawString(64, 15, F("Apagando pantalla"));
            Heltec.display->drawString(64, 35, "en " + String(i) + "...");
            Heltec.display->display();
            delay(1000);
        }
        
        displayOff();
        setLedState(false);
        _displaySleeping = true;
        _countdownActive = false;
        Serial.println("😴 Pantalla apagada manualmente");
    }
}

bool DisplayManager::isDisplaySleeping() const {
    return _displaySleeping;
}

void DisplayManager::handlePRGButton() {
    bool currentButtonState = digitalRead(PRG_BUTTON_PIN);
    unsigned long currentTime = millis();
    
    // Detectar flanco descendente (botón presionado) con debounce
    if (currentButtonState == LOW && _lastButtonState == HIGH && 
        (currentTime - _lastButtonPress > 200)) {
        
        _lastButtonPress = currentTime;
        
        if (_displaySleeping) {
            wakeUpDisplay();
        }
    }
    
    _lastButtonState = currentButtonState;
}

void DisplayManager::showSplashScreen() {
    Heltec.display->clear();
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->drawString(64, 10, F("SENSOR NIVEL"));
    Heltec.display->drawString(64, 26, F("DE AGUA"));
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(64, 45, F("v2.0 - DataTech"));
    Heltec.display->display();
}