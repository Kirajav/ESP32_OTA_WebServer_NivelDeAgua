#include "AppManager.h"
#include <Arduino.h>
#include <WiFi.h>
#include <heltec.h>
#include "SPIFFS.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncDNSServer.h>
#include <ESPAsync_WiFiManager.h>
#include <ArduinoJson.h>
#include <ESP_DoubleResetDetector.h>
#include "ConfigManager.h"
#include "SensorManager.h"
#include "WaterLevelSensor.h"
#include "DisplayManager.h"
#include "WebManager.h"
#include "SystemStatus.h"
#include "OTAUpdater.h"

AppManager::AppManager() :
    drd(DRD_TIMEOUT, DRD_ADDRESS),
    wifiConnected(false),
    lastWifiRetryMillis(0),
    previousMillis(0),
    server(80),
    dnsServer(),
    configManager(),
    displayManager(),
    sensorManager(),
    waterLevelSensor(TRIGGER_PIN, ECHO_PIN_1, &configManager),
    systemStatus(&displayManager, &configManager),
    otaUpdater(&displayManager, &systemStatus)
{
    _instance = this; // Set the static instance
    wifiManager = new ESPAsync_WiFiManager(&server, &dnsServer, "TinacoESP");
    webManager = new WebManager(&server, &sensorManager, &configManager, &displayManager, &systemStatus);
}

AppManager* AppManager::_instance = nullptr; // Initialize static member

void AppManager::saveConfigCallback() {
    if (_instance) {
        _instance->saveWiFiManagerParams();
    }
}

void AppManager::begin() {
    Serial.begin(115200);
    Serial.println(F("\nIniciando Sensor de Nivel de Agua..."));
    Serial.print("Detect Double Reset: ");
    Serial.println(drd.detectDoubleReset());

    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

    WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
        switch(event) {
            case SYSTEM_EVENT_STA_CONNECTED:
                Serial.println(F("WiFi Event: CONNECTED"));
                this->wifiConnected = true;
                break;
            case SYSTEM_EVENT_STA_DISCONNECTED:
                Serial.println(F("WiFi Event: DISCONNECTED"));
                this->wifiConnected = false;
                break;
            case SYSTEM_EVENT_STA_GOT_IP:
                Serial.println("WiFi Event: GOT IP: " + WiFi.localIP().toString());
                break;
            default:
                Serial.printf("WiFi Event: %d\n", event);
                break;
        }
    });
    
    displayManager.begin();
    
    configManager.begin();

    sensorManager.addSensor(&waterLevelSensor);
    sensorManager.begin();
    
    systemStatus.begin();
    systemStatus.setMonitorEnabled(true); // Habilitar el monitoreo serial del sensor
    configManager.setDisplayOn(true);     // Forzar el display a estar encendido
    
    if (drd.detectDoubleReset()) {
        Serial.println("Doble reset detectado");
        displayManager.clear();
        displayManager.setTextAlignment(TEXT_ALIGN_CENTER);
        displayManager.drawString(64, 10, "Modo Config WiFi");
        displayManager.drawString(64, 20, "Borrando config...");
        displayManager.display();
        wifiManager->resetSettings();
        delay(2000);
        ESP.restart(); // Restaurar el restart para evitar el bucle infinito
    }
    
    // WiFiManager setup
    wifiManager->setDebugOutput(true);
    wifiManager->setMinimumSignalQuality(-1);
    wifiManager->setBreakAfterConfig(false);  // No romper después de guardar configuración
    wifiManager->setSaveConfigCallback(AppManager::saveConfigCallback);
    wifiManager->setConfigPortalTimeout(180);  // 3 minutos de timeout
    wifiManager->setConnectTimeout(10000);     // 10 segundos timeout de conexión
    wifiManager->setCustomHeadElement("<style>\n        .networks-list .wifi-name:after {content: ' (Guardada)'; color: green; font-weight: bold;} \n        .panel-title { margin-bottom: 10px; text-align: center; color: #069; } \n        .saved-wifi { background-color: #e8f5e9; border-left: 3px solid #4caf50; } \n        .networks-list { margin-top: 10px; } \n        </style>");

    // Títulos de secciones
    wifiManager->addParameter(new ESPAsync_WMParameter("<hr><h3 style='text-align:center'>Configuración del sensor</h3>"));
    
    const char* tipo_actual = configManager.getTipoContenedorStr();
    custom_tipo_contenedor = new ESPAsync_WMParameter(
        "tipo_contenedor", "Tipo de contenedor", tipo_actual, 20,
        "required onchange='this.form.tipo_contenedor.value=this.value' list='tipos_contenedor'");
    wifiManager->addParameter(new ESPAsync_WMParameter(
        "<datalist id='tipos_contenedor'>\n        <option value='tinaco'>Tinaco</option>\n        <option value='cisterna'>Cisterna</option>\n        <option value='otro contenedor'>Otro contenedor</option>\n        </datalist>"));
        
    custom_altura_max = new ESPAsync_WMParameter(
        "altura_max", "Altura máxima del nivel del agua en el contenedor (cm)", 
        String(configManager.getAlturaMax()).c_str(), 10,
        "required type='number' step='0.1' min='0'");
        
    custom_capacidad = new ESPAsync_WMParameter(
        "capacidad", "Capacidad del contenedor aprox. (L)", 
        String(configManager.getCapacidad()).c_str(), 10,
        "required type='number' step='0.1' min='0'");
        
    custom_distancia_min = new ESPAsync_WMParameter(
        "distancia_min", "Distancia mínima que puede leer el sensor (cm)", 
        String(configManager.getDistanciaMin()).c_str(), 10,
        "required type='number' step='0.1' min='0'");
        
    // Agregar sección de configuración de red
    wifiManager->addParameter(new ESPAsync_WMParameter("<hr><h3 style='text-align:center'>Configuración de red</h3>"));
    
    custom_hostname = new ESPAsync_WMParameter(
        "hostname", "Nombre del dispositivo en la red", 
        configManager.getHostname().c_str(), 40,
        "placeholder='ESP32_Sensor_1' pattern='^[^-\\.]{2,32}$' title='Entre 2 y 32 caracteres sin puntos ni guiones'");
        
    custom_check_updates = new ESPAsync_WMParameter(
        "check_updates", "¿Buscar actualizaciones al iniciar?",
        configManager.getCheckUpdates() ? "Sí" : "No", 10,
        "required");
        
    // Agregar parámetros al WiFiManager
    wifiManager->addParameter(custom_tipo_contenedor);
    wifiManager->addParameter(custom_altura_max);
    wifiManager->addParameter(custom_capacidad);
    wifiManager->addParameter(custom_distancia_min);
    wifiManager->addParameter(custom_hostname);
    wifiManager->addParameter(custom_check_updates);
    
    // Configurar callbacks
    // The original setSaveConfigCallback was removed because it was causing a restart.
    // The saveWiFiManagerParams() method will be called when the parameters are saved.

    const char* AP_SSID = "ESP32: Sensor de nivel de agua";
    const char* AP_PASS = "12345678";

    displayManager.clear();
    displayManager.setFont(ArialMT_Plain_16);
    displayManager.setTextAlignment(TEXT_ALIGN_CENTER);
    displayManager.drawString(64, 0, F("SENSOR DE"));
    displayManager.drawString(64, 16, F("NIVEL DE AGUA"));
    
    displayManager.setFont(ArialMT_Plain_10);
    displayManager.setTextAlignment(TEXT_ALIGN_LEFT);
    displayManager.drawString(0, 35, F("Iniciando WiFi..."));
    displayManager.drawString(0, 45, "SSID: " + String(AP_SSID));
    displayManager.display();
    
    if (wifiManager->autoConnect(AP_SSID, AP_PASS)) {
        Serial.println(F("Conectado a WiFi!"));
        Serial.print(F("IP: "));
        Serial.println(WiFi.localIP());
        
        if (WiFi.SSID().length() > 0) {
            WiFi.setHostname(configManager.getHostname().c_str());
            Serial.println(F("Guardando configuración WiFi..."));
            WiFi.setAutoReconnect(true);
            WiFi.persistent(true);
            delay(1000);
        }
    } else {
        Serial.println(F("Fallo en la conexión, reiniciando..."));
        delay(1000);
        ESP.restart();
    }
    
    displayManager.clear();
    displayManager.setTextAlignment(TEXT_ALIGN_CENTER);
    displayManager.drawString(64, 0, F("WiFi Conectado"));
    displayManager.setTextAlignment(TEXT_ALIGN_LEFT);
    displayManager.drawString(0, 15, "Red: " + WiFi.SSID());
    displayManager.drawString(0, 25, "IP: " + WiFi.localIP().toString());
    displayManager.drawString(0, 35, "MAC: " + WiFi.macAddress());
    displayManager.display();
    delay(3000);
    
    webManager->begin();
    
    otaUpdater.begin(&server);
    
    server.begin();
    
    previousMillis = millis();
}

void AppManager::saveWiFiManagerParams() {
    Serial.println(F("Callback de guardado de parámetros"));
    
    if (custom_altura_max && custom_capacidad && custom_distancia_min && custom_tipo_contenedor &&
        custom_hostname && custom_check_updates) {
        
        configManager.setAlturaMax(atof(custom_altura_max->getValue()));
        configManager.setCapacidad(atof(custom_capacidad->getValue()));
        Serial.print(F("Valor de custom_distancia_min->getValue(): "));
        Serial.println(custom_distancia_min->getValue());
        configManager.setDistanciaMin(atof(custom_distancia_min->getValue()));
        
        // Obtener y procesar el tipo de contenedor
        String tipoStr = String(custom_tipo_contenedor->getValue());
        tipoStr.toLowerCase();  // Convertir a minúsculas
        tipoStr.trim();        // Eliminar espacios
        
        if (tipoStr == "tinaco") configManager.setTipoContenedor(0);
        else if (tipoStr == "cisterna") configManager.setTipoContenedor(1);
        else if (tipoStr == "otro contenedor") configManager.setTipoContenedor(2);
        else configManager.setTipoContenedor(0);  // Por defecto
        
        // Guardar parámetros de red
        configManager.setHostname(custom_hostname->getValue());
        configManager.setCheckUpdates(String(custom_check_updates->getValue()) == "Sí");
        
        configManager.saveConfig();
        
        Serial.println(F("Parámetros guardados:"));
        Serial.println("Tipo de contenedor: " + tipoStr);
        Serial.println("Altura máxima: " + String(configManager.getAlturaMax()));
        Serial.println("Capacidad: " + String(configManager.getCapacidad()));
        Serial.println("Distancia mínima: " + String(configManager.getDistanciaMin()));
        Serial.println("Hostname: " + configManager.getHostname());
        Serial.println("Buscar actualizaciones: " + String(configManager.getCheckUpdates() ? "Sí" : "No"));
    }
}

void AppManager::loop() {
    //Serial.println("WiFi Status: " + String(WiFi.status()));
    //Serial.println("SystemStatus isMonitorEnabled: " + String(systemStatus.isMonitorEnabled()));
    //Serial.println("ConfigManager isDisplayOn: " + String(configManager.isDisplayOn()));
    otaUpdater.loop();
    systemStatus.loop();
    unsigned long currentMillis = millis();
    
    // Solo intentar reconexión si realmente se perdió la conexión
    if (WiFi.status() != WL_CONNECTED && (currentMillis - lastWifiRetryMillis >= wifiRetryInterval)) {
        Serial.println(F("Conexión WiFi perdida, intentando reconexión..."));
        lastWifiRetryMillis = currentMillis;
        
        WiFi.reconnect();
        
        if (configManager.isDisplayOn()) {
            displayManager.clear();
            displayManager.setFont(ArialMT_Plain_10);
            displayManager.setTextAlignment(TEXT_ALIGN_CENTER);
            displayManager.drawString(64, 10, F("Reconectando"));
            displayManager.drawString(64, 25, F("WiFi..."));
            displayManager.display();
        }
    }
    
    //Serial.println("AppManager::loop() - Checking update interval.");
    if (currentMillis - previousMillis >= interval) {
        Serial.println("AppManager::loop() - Actualizando sensores...");
        previousMillis = currentMillis;
        
        Serial.println("Llamando sensorManager.update()...");
        sensorManager.update();
        
        if (systemStatus.isMonitorEnabled()) {
            String sensorJson = sensorManager.getSensorJson("WaterLevel");
            Serial.println("JSON del sensor: " + sensorJson);
        }
        
        if (configManager.isDisplayOn()) {
            StaticJsonDocument<256> doc;
            String sensorJson = sensorManager.getSensorJson("WaterLevel");
            DeserializationError error = deserializeJson(doc, sensorJson);
            if (!error) {
                String distancia = doc["distancia_cm"].as<String>();
                String litros = doc["litros"].as<String>();
                Serial.println("Actualizando display - Distancia: " + distancia + ", Litros: " + litros);
                displayManager.updateDisplay(WiFi.localIP().toString(), distancia, litros);
            } else {
                Serial.println("Error deserializando JSON del sensor");
            }
        }
    }
    
    if (configManager.isDisplayOn() && (currentMillis - systemStatus.getOledStartTime() >= systemStatus.getOledTimeout())) {
        //Serial.println(F("Apagando display por timeout"));
        systemStatus.setDisplayEnabled(false);
        displayManager.displayOff();
        configManager.saveConfig();
    }
}
