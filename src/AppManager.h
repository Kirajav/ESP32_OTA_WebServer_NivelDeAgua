#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ESP_DoubleResetDetector.h>
#include <WebSerial.h>
#include <ElegantOTA.h>
#include "DisplayManager.h"
#include "SensorManager.h"
#include "../lib/Config/ApplicationConfig.h"
#include "ESPNowManager.h"
#include "GoogleHomeIntegration.h"
#include "AlexaIntegration.h"
#include "TuyaIntegration.h"
#include "TuyaDevice.h"
#include "NTPTimeSync.h"
#include "WaterLevelSensor.h"

class AppManager {
public:
    AppManager();
    void initialize();
    void loop();
    bool isWiFiConnected();
    String getDeviceIP();
    void restart();

private:
    DisplayManager display_manager;
    SensorManager sensor_manager;
    ApplicationConfig config_manager;
    bool wifi_connected;
    bool portal_active;
    bool forcePortalMode;  // Para forzar portal cautivo con doble reset
    
    // Variables para chequeo de conexión WiFi
    unsigned long connectionCheckTimer;
    bool checkingConnection;
    
    // Variables para auto-sleep de pantalla
    unsigned long lastDisplayActivity;
    bool displaySleeping;
    
    // IoT Integrations
    ESPNowManager* espNowManager;
    GoogleHomeIntegration* googleHome;
    AlexaIntegration* alexa;
    TuyaIntegration* tuya;
    TuyaDevice* tuyaDevice;
    NTPTimeSync* ntpSync;
    
    void initializeNetwork();
    void startPortalMode();       // Iniciar portal cautivo
    void startNormalMode();       // Iniciar modo sensor normal
    bool tryConnectWiFi();        // Intentar conectar WiFi predeterminado
    void setupWebServer();        // Web server unificado inteligente
    void setupStaticFiles();      // Archivos estáticos compartidos
    void setupSmartRoutes();      // Rutas principales inteligentes
    void setupConditionalRoutes(); // Rutas específicas por modo
    void setupCaptiveDetectionRoutes(); // Detección portal cautivo
    void initializeServices();    // Servicios según modo
    void handleScanWiFi(AsyncWebServerRequest *request);
    void handleWiFiResults(AsyncWebServerRequest *request);
    void handleSensorData(AsyncWebServerRequest *request);
    void handleToggleDisplay(AsyncWebServerRequest *request);
    void updateSensorDisplay();       // Actualizar pantalla con datos del sensor
    void updateTuyaDeviceData();      // Actualizar datos Tuya Device
    bool detectFilling(float readings[], uint8_t threshold); // Detectar si el contenedor se está llenando
    void apCallback();
    void wakeUpDisplay();      // Despertar pantalla por actividad
    void checkAutoSleep();     // Verificar si debe apagar pantalla
};

#endif
