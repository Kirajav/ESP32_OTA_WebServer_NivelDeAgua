#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <heltec.h>
#include "SPIFFS.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncDNSServer.h>
#define _ESPASYNC_WIFIMGR_LOGLEVEL_    4
class ESPAsync_WiFiManager;
class ESPAsync_WMParameter;

#include <ArduinoJson.h>
#include <ESP_DoubleResetDetector.h>
#include <ConfigManager.h>
#include "SensorManager.h"
#include "WaterLevelSensor.h"
#include "DisplayManager.h"
#include "WebManager.h"
#include "SystemStatus.h"
#include "OTAUpdater.h"

class AppManager {
public:
    AppManager();
    void begin();
    void loop();
    void resetWiFiFromWeb();  // Nuevo método para reset desde web

private:
    // --- Hardware y Pines ---
    static const int TRIGGER_PIN = 12;
    static const int ECHO_PIN_1 = 13;

    // --- Detector de Doble Reset ---
    static const int DRD_TIMEOUT = 5000;
    static const int DRD_ADDRESS = 0;
    DoubleResetDetector drd;

    // --- Variables de Estado ---
    bool wifiConnected;
    unsigned long lastWifiRetryMillis;
    const long wifiRetryInterval = 30000;

    // --- Temporizadores ---
    unsigned long previousMillis;
    const long interval = 10000;  // Aumentado a 10 segundos para mejorar estabilidad

    // --- Objetos Globales ---
    AsyncWebServer server;
    AsyncDNSServer dnsServer;
    ConfigManager configManager;
    ESPAsync_WiFiManager* wifiManager;
    DisplayManager displayManager;
    SensorManager sensorManager;
    WebManager* webManager;
    SystemStatus systemStatus;
    OTAUpdater otaUpdater;
    WaterLevelSensor waterLevelSensor;

    // Custom WiFiManager parameters
    ESPAsync_WMParameter* custom_tipo_contenedor;
    ESPAsync_WMParameter* custom_altura_max;
    ESPAsync_WMParameter* custom_capacidad;
    ESPAsync_WMParameter* custom_distancia_min;
    ESPAsync_WMParameter* custom_hostname;
    ESPAsync_WMParameter* custom_check_updates;

    void saveWiFiManagerParams();

    static AppManager* _instance;
    static void saveConfigCallback();
    static void configModeCallback(ESPAsync_WiFiManager* myWiFiManager);
    static void resetWiFiCallback();  // Nuevo callback estático para reset WiFi
};

#endif // APP_MANAGER_H