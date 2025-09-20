#include <Arduino.h>
#include <WiFi.h>
#include <heltec.h>
#include "SPIFFS.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncDNSServer.h>
#define _ESPASYNC_WIFIMGR_LOGLEVEL_    4
#include <ESPAsync_WiFiManager.h>       
#include <WebSerial.h>
#include <ElegantOTA.h>
#include <HCSR04.h>
#include <ArduinoJson.h>
#include <ESP_DoubleResetDetector.h>

// Configuración del detector de doble reset
#define DRD_TIMEOUT 10
#define DRD_ADDRESS 0
DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

// Variables para el control del LED dimmer
#define LED_CHANNEL 0          // Canal del LED para PWM
#define LED_FREQ 5000         // Frecuencia PWM en Hz
#define LED_RESOLUTION 8      // Resolución de 8 bits (0-255)
#define FADE_STEP 5          // Paso de atenuación
#define FADE_INTERVAL 30     // Intervalo de atenuación en ms
uint8_t ledValue = 0;
bool fadeUp = true;
unsigned long lastFadeTime = 0;

// --- Definiciones de Hardware y Pines ---
#define LED_PIN 25          // LED integrado en la placa Heltec
#define PRG_BUTTON_PIN 0    // Botón "PRG" en la placa Heltec
#define TRIGGER_PIN 12      // Pin trigger del sensor HC-SR04
#define ECHO_PIN_1 13       // Pin echo del sensor HC-SR04
#define Vext 21             // Pin para alimentación del OLED en Heltec V1
#define OLED_RST 16         // Pin de reset del OLED en Heltec V1
#define OLED_SDA 4          // Pin SDA del OLED en Heltec V1
#define OLED_SCL 15         // Pin SCL del OLED en Heltec V1

// --- Parámetros de Configuración (valores por defecto) ---
double ALTURA_MAX_AGUA_TINACO = 115.0;
double CAPACIDAD_LITROS_TINACO = 1200.0;
double DISTANCIA_MINIMA_SENSOR = 19.0;

// --- Variables Globales de Estado ---
bool ledState = false;
bool displayEnabled = true;
double distanciaCm = 0;
String mensaje_error = "";
bool monitorEnabled = false;
bool isUpdating = false;
bool wifiConnected = false;
unsigned long lastWifiRetryMillis = 0;
const long wifiRetryInterval = 30000;  // 30 segundos entre intentos de reconexión

// --- Constantes WiFi ---
const char* AP_SSID = "ESP32: Sensor de nivel de agua";  // Nombre del punto de acceso WiFi
const char* AP_PASS = "12345678";      // Contraseña del punto de acceso WiFi
const char* HOST_NAME = "TinacoESP";   // Nombre de host para mDNS

// --- Temporizadores y Variables de Control ---
unsigned long previousMillis = 0;
const long interval = 5000;        // Intervalo de lectura del sensor (5 segundos)
unsigned long oledStartTime = 0;
const long oledTimeout = 300000;   // 5 minutos para timeout del OLED

// --- Estructura de Configuración ---
struct Config {
    double altura_max;
    double capacidad;
    double distancia_min;
    String hostname;
    bool check_updates;
};
Config config;

// --- Objetos Globales ---
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
AsyncDNSServer dnsServer;
UltraSonicDistanceSensor distanceSensor(TRIGGER_PIN, ECHO_PIN_1);
ESPAsync_WiFiManager wifiManager(&server, &dnsServer, "TinacoESP");

// --- Parámetros de WiFiManager ---
ESPAsync_WMParameter* custom_altura_max = nullptr;
ESPAsync_WMParameter* custom_capacidad = nullptr;
ESPAsync_WMParameter* custom_distancia_min = nullptr;
ESPAsync_WMParameter* custom_tipo_contenedor = nullptr;
ESPAsync_WMParameter* custom_hostname = nullptr;
ESPAsync_WMParameter* custom_check_updates = nullptr;

// --- Tipo de Contenedor ---
const char* TIPOS_CONTENEDOR[] = {"Tinaco", "Cisterna", "Contenedor"};
uint8_t tipo_contenedor_actual = 0;  // 0=Tinaco, 1=Cisterna, 2=Contenedor

// Función para calcular litros de agua
String getLitros() {
    if (distanciaCm < 0) { // Error de lectura del sensor
        return "Error";
    }
    // Si la distancia es menor o igual a la mínima del sensor, se considera lleno
    if (distanciaCm <= DISTANCIA_MINIMA_SENSOR) {
        return String(CAPACIDAD_LITROS_TINACO, 1);
    }

    double alturaActualAgua = ALTURA_MAX_AGUA_TINACO - distanciaCm;
    if (alturaActualAgua < 0) alturaActualAgua = 0;
    double alturaTotalAgua = ALTURA_MAX_AGUA_TINACO;
    if (alturaTotalAgua <= 0) return "0.0";
    
    double porcentajeLlenado = (alturaActualAgua / alturaTotalAgua) * 100.0;
    if (porcentajeLlenado > 100.0) porcentajeLlenado = 100.0;
    if (porcentajeLlenado < 0.0) porcentajeLlenado = 0.0;

    double litros = (CAPACIDAD_LITROS_TINACO * porcentajeLlenado) / 100.0;
    return String(litros, 1);
}

// --- Declaraciones de funciones ---
void saveConfig();
void updateGlobalsFromConfig();
void initWebSocket();
void handleDisplayStateChange(bool turnOn);
void showCountdown(const char* message, int seconds);
// --- Estructura para el manejo de comandos de WebSerial ---
typedef void (*CommandHandler)();

struct Command {
    const char* name;
    CommandHandler handler;
};

void handleDistancia() {
    WebSerial.println("Distancia actual: " + String(distanciaCm) + " cm");
}

void handleLitros() {
    WebSerial.println("Litros actuales: " + getLitros() + " L");
}

void handleIp() {
    WebSerial.println("Dirección IP: " + WiFi.localIP().toString());
}

void handleSubmask() {
    WebSerial.println("Mascara de Subred: " + WiFi.subnetMask().toString());
}

void handleGateway() {
    WebSerial.println("Gateway IP: " + WiFi.gatewayIP().toString());
}

void handleDns() {
    WebSerial.println("DNS 1: " + WiFi.dnsIP(0).toString());
    WebSerial.println("DNS 2: " + WiFi.dnsIP(1).toString());
}

void handleMac() {
    WebSerial.println("Dirección MAC: " + WiFi.macAddress());
}

void handleDisplay() {
    WebSerial.println(String("Estado del display: ") + (displayEnabled ? "Encendida" : "Apagada"));
}

void handleSensor() {
    WebSerial.println("Estado del sensor: " + mensaje_error);
}

void handleMonitor() {
    monitorEnabled = !monitorEnabled;
    if (monitorEnabled) {
        WebSerial.println("Modo monitor iniciado. Enviando datos cada 5 segundos...");
    } else {
        WebSerial.println("Modo monitor detenido.");
    }
}

void handleAllInfo() {
    String lines[] = {
        "\n+------------------+----------------------------------------+",
        "| CATEGORIA        | VALOR                                  |",
        "+------------------+----------------------------------------+",
        "| -- Conectividad --                                      |",
        "| Direccion IP     | " + WiFi.localIP().toString(),
        "| Mascara Subred   | " + WiFi.subnetMask().toString(),
        "| Gateway IP       | " + WiFi.gatewayIP().toString(),
        "| DNS 1            | " + WiFi.dnsIP(0).toString(),
        "| DNS 2            | " + WiFi.dnsIP(1).toString(),
        "| Direccion MAC    | " + WiFi.macAddress(),
        "|                                                        |",
        "| -- Sensor --                                            |",
        "| Estado           | " + mensaje_error,
        "| Distancia        | " + String(distanciaCm) + " cm",
        "| Litros           | " + getLitros() + " L",
        "|                                                        |",
        "| -- Dispositivo --                                       |",
        "| Estado Display   | " + String(displayEnabled ? "Encendida" : "Apagada"),
        "+------------------+----------------------------------------+",
        ""
    };
    
    for (int i = 0; !lines[i].isEmpty(); ++i) {
        WebSerial.println(lines[i]);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void handleHelp() {
    String lines[] = {
        "--- Lista de Comandos Disponibles ---",
        "distancia      - Muestra la distancia actual del sensor al agua.",
        "litros         - Muestra la cantidad de litros actual en el tinaco.",
        "ip             - Muestra la dirección IP del dispositivo.",
        "submask        - Muestra la mascara de subred.",
        "gateway        - Muestra la IP del gateway.",
        "dns            - Muestra las direcciones de los servidores DNS.",
        "mac            - Muestra la dirección MAC del dispositivo.",
        "display        - Muestra el estado actual de la pantalla OLED.",
        "display on     - Enciende la pantalla OLED y el LED blanco.",
        "display off    - Apaga la pantalla OLED y el LED blanco.",
        "sensor         - Muestra el estado de la conexión con el sensor.",
        "allinfo        - Muestra toda la información disponible en una tabla.",
        "monitor        - Inicia/detiene la impresión continua de datos del sensor.",
        "ayuda, help, ? - Muestra este mensaje de ayuda.",
        "-------------------------------------",
        ""
    };
    
    for (int i = 0; !lines[i].isEmpty(); ++i) {
        WebSerial.println(lines[i]);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void handleDisplayOn() {
    displayEnabled = true;
    handleDisplayStateChange(true);
    WebSerial.println("Display y LED encendidos");
    saveConfig();
    ws.textAll("1");
}

void handleDisplayOff() {
    displayEnabled = false;
    handleDisplayStateChange(false);
    WebSerial.println("Display y LED apagados");
    saveConfig();
    ws.textAll("0");
}

void handleReset() {
    WebSerial.println("Reiniciando dispositivo...");
    delay(1000);
    ESP.restart();
}

Command commands[] = {
    {"distancia", &handleDistancia},
    {"litros", &handleLitros},
    {"ip", &handleIp},
    {"submask", &handleSubmask},
    {"gateway", &handleGateway},
    {"dns", &handleDns},
    {"mac", &handleMac},
    {"display", &handleDisplay},
    {"sensor", &handleSensor},
    {"monitor", &handleMonitor},
    {"allinfo", &handleAllInfo},
    {"display on", &handleDisplayOn},
    {"display off", &handleDisplayOff},
    {"reset", &handleReset},
    {"?", &handleHelp},
    {"help", &handleHelp},
    {"ayuda", &handleHelp}
};

void onWebSerialMessage(uint8_t *data, size_t len) {
    String commandStr = String((char*)data).substring(0, len);
    commandStr.toLowerCase();
    commandStr.trim();

    for (const auto& cmd : commands) {
        if (commandStr == cmd.name) {
            cmd.handler();
            return;
        }
    }
    WebSerial.println("Comando no reconocido. Escribe 'ayuda' para ver la lista de comandos.");
}

void initWebSerial() {
    WebSerial.begin(&server);
    WebSerial.onMessage(onWebSerialMessage);
    WebSerial.println("\nWebSerial iniciado. Escribe 'ayuda' para ver los comandos disponibles.");
}

// --- Función para procesar variables en el HTML ---
String processor(const String& var) {
    if (var == "ESTADO_OLED") {
        return displayEnabled ? "encendida" : "apagada";
    }
    if (var == "ESTADO_SENSOR") {
        return mensaje_error;
    }
    if (var == "LITROS") {
        return getLitros();
    }
    if (var == "DISTANCIA") {
        return distanciaCm < 0 ? "Error" : String(distanciaCm);
    }
    return String();
}

// --- Manejador de eventos WebSocket ---
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            client->text(displayEnabled ? "1" : "0");
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA: {
            AwsFrameInfo *info = (AwsFrameInfo*)arg;
            if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
                data[len] = 0;
                String command = (char*)data;
                if (command == "toggle") {
                    displayEnabled = !displayEnabled;
                    if (displayEnabled) {
                        Heltec.display->displayOn();
                        oledStartTime = millis();
                    } else {
                        Heltec.display->displayOff();
                    }
                    saveConfig();
                    ws.textAll(displayEnabled ? "1" : "0");
                }
                else if (command == "reset") {
                    ws.textAll("restarting");
                    delay(100);
                    ESP.restart();
                }
            }
            break;
        }
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void initWebSocket() {
    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);
}

// --- Funciones de Configuración ---
void saveConfig() {
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
        Serial.println(F("Error al abrir config.json para escritura"));
        return;
    }
    
    StaticJsonDocument<1024> doc;
    doc["altura_max"] = config.altura_max;
    doc["capacidad"] = config.capacidad;
    doc["distancia_min"] = config.distancia_min;
    doc["display_on"] = displayEnabled;
    doc["tipo_contenedor"] = tipo_contenedor_actual;
    doc["tipo_nombre"] = TIPOS_CONTENEDOR[tipo_contenedor_actual];
    doc["hostname"] = config.hostname;
    doc["check_updates"] = config.check_updates;
    
    if (serializeJson(doc, configFile)) {
        Serial.println(F("Configuración guardada exitosamente"));
    } else {
        Serial.println(F("Error al escribir la configuración"));
    }
    configFile.close();
}

void updateGlobalsFromConfig() {
    ALTURA_MAX_AGUA_TINACO = config.altura_max;
    CAPACIDAD_LITROS_TINACO = config.capacidad;
    DISTANCIA_MINIMA_SENSOR = config.distancia_min;
}

bool loadConfig() {
    if (!SPIFFS.begin(true)) {
        Serial.println(F("Error montando SPIFFS"));
        return false;
    }

    File configFile = SPIFFS.open("/config.json", "r");
    if (!configFile) {
        Serial.println(F("No se encontró archivo de configuración, usando valores por defecto"));
        return false;
    }

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();

    if (error) {
        Serial.println(F("Error al parsear el archivo de configuración"));
        return false;
    }

    config.altura_max = doc["altura_max"] | ALTURA_MAX_AGUA_TINACO;
    config.capacidad = doc["capacidad"] | CAPACIDAD_LITROS_TINACO;
    config.distancia_min = doc["distancia_min"] | DISTANCIA_MINIMA_SENSOR;
    displayEnabled = doc["display_on"] | true;
    tipo_contenedor_actual = doc["tipo_contenedor"] | 0;
    config.hostname = doc["hostname"] | "ESP32_Sensor_1";
    config.check_updates = doc["check_updates"] | true;

    updateGlobalsFromConfig();
    return true;
}

// --- Callback para guardar parámetros ---
void saveParamCallback() {
    Serial.println(F("Callback de guardado de parámetros"));
    
    if (custom_altura_max && custom_capacidad && custom_distancia_min && custom_tipo_contenedor &&
        custom_hostname && custom_check_updates) {
        
        config.altura_max = atof(custom_altura_max->getValue());
        config.capacidad = atof(custom_capacidad->getValue());
        config.distancia_min = atof(custom_distancia_min->getValue());
        
        // Obtener y procesar el tipo de contenedor
        String tipoStr = String(custom_tipo_contenedor->getValue());
        tipoStr.toLowerCase();  // Convertir a minúsculas
        tipoStr.trim();        // Eliminar espacios
        
        if (tipoStr == "tinaco") tipo_contenedor_actual = 0;
        else if (tipoStr == "cisterna") tipo_contenedor_actual = 1;
        else if (tipoStr == "otro contenedor") tipo_contenedor_actual = 2;
        else tipo_contenedor_actual = 0;  // Por defecto
        
        // Guardar parámetros de red
        config.hostname = custom_hostname->getValue();
        config.check_updates = String(custom_check_updates->getValue()) == "Sí";
        
        saveConfig();
        updateGlobalsFromConfig();
        
        Serial.println(F("Parámetros guardados:"));
        Serial.println("Tipo de contenedor: " + tipoStr);
        Serial.println("Altura máxima: " + String(config.altura_max));
        Serial.println("Capacidad: " + String(config.capacidad));
        Serial.println("Distancia mínima: " + String(config.distancia_min));
        Serial.println("Hostname: " + config.hostname);
        Serial.println("Buscar actualizaciones: " + String(config.check_updates ? "Sí" : "No"));
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println(F("\nIniciando Sensor de Nivel de Agua..."));
    
    // Inicializar WiFi con la última configuración guardada
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

    // Inicialización del hardware
    pinMode(Vext, OUTPUT);
    pinMode(PRG_BUTTON_PIN, INPUT_PULLUP);  // Configurar botón PRG
    pinMode(LED_PIN, OUTPUT);               // Configurar LED
    digitalWrite(Vext, LOW);    // Encender OLED
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
    
    // Inicializar SPIFFS y cargar configuración
    if(!SPIFFS.begin(true)) {
        Serial.println(F("Error montando SPIFFS"));
    }
    loadConfig();
    
    // Configurar WiFiManager
    wifiManager.setDebugOutput(true);
    wifiManager.setMinimumSignalQuality(-1);
    wifiManager.setBreakAfterConfig(false);  // No romper después de guardar configuración
    wifiManager.setSaveConfigCallback([]() {
        Serial.println(F("Configuración guardada, reiniciando..."));
        delay(1000);
        ESP.restart();
    });
    wifiManager.setConfigPortalTimeout(180);  // 3 minutos de timeout
    wifiManager.setConnectTimeout(10000);     // 10 segundos timeout de conexión
    wifiManager.setCustomHeadElement("<style>"
        ".networks-list .wifi-name:after {content: ' (Guardada)'; color: green; font-weight: bold;} "
        ".panel-title { margin-bottom: 10px; text-align: center; color: #069; } "
        ".saved-wifi { background-color: #e8f5e9; border-left: 3px solid #4caf50; } "
        ".networks-list { margin-top: 10px; } "
        "</style>");
    
    // Detectar doble reset
    if (drd.detectDoubleReset()) {
        Serial.println("Doble reset detectado");
        Heltec.display->clear();
        Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
        Heltec.display->drawString(64, 10, "Modo Config WiFi");
        Heltec.display->drawString(64, 20, "Borrando config...");
        Heltec.display->display();
        wifiManager.resetSettings();
        delay(1000);
        ESP.restart();
    }
    wifiManager.addParameter(new ESPAsync_WMParameter("<hr><h3 style='text-align:center'>Configuración de red IP</h3>"));
    
    // Títulos de secciones
    wifiManager.addParameter(new ESPAsync_WMParameter("<hr><h3 style='text-align:center'>Configuración del sensor</h3>"));
    
    const char* tipo_actual = tipo_contenedor_actual == 0 ? "tinaco" : 
                               tipo_contenedor_actual == 1 ? "cisterna" : 
                               "otro contenedor";
    custom_tipo_contenedor = new ESPAsync_WMParameter(
        "tipo_contenedor", "Tipo de contenedor", tipo_actual, 20,
        "required onchange='this.form.tipo_contenedor.value=this.value' list='tipos_contenedor'");
    wifiManager.addParameter(new ESPAsync_WMParameter(
        "<datalist id='tipos_contenedor'>"
        "<option value='tinaco'>Tinaco</option>"
        "<option value='cisterna'>Cisterna</option>"
        "<option value='otro contenedor'>Otro contenedor</option>"
        "</datalist>"));
        
    custom_altura_max = new ESPAsync_WMParameter(
        "altura_max", "Altura máxima del nivel del agua en el contenedor (cm)", 
        String(config.altura_max).c_str(), 10,
        "required type='number' step='0.1' min='0'");
        
    custom_capacidad = new ESPAsync_WMParameter(
        "capacidad", "Capacidad del contenedor aprox. (L)", 
        String(config.capacidad).c_str(), 10,
        "required type='number' step='0.1' min='0'");
        
    custom_distancia_min = new ESPAsync_WMParameter(
        "distancia_min", "Distancia mínima que puede leer el sensor (cm)", 
        String(config.distancia_min).c_str(), 10,
        "required type='number' step='0.1' min='0'");
        
    // Agregar sección de configuración de red
    wifiManager.addParameter(new ESPAsync_WMParameter("<hr><h3 style='text-align:center'>Configuración de red</h3>"));
    
    custom_hostname = new ESPAsync_WMParameter(
        "hostname", "Nombre del dispositivo en la red", 
        config.hostname.c_str(), 40,
        "placeholder='ESP32_Sensor_1' pattern='^[^-\\.]{2,32}$' title='Entre 2 y 32 caracteres sin puntos ni guiones'");
        
    custom_check_updates = new ESPAsync_WMParameter(
        "check_updates", "¿Buscar actualizaciones al iniciar?",
        config.check_updates ? "Sí" : "No", 10,
        "required");
        
    // Agregar parámetros al WiFiManager
    wifiManager.addParameter(custom_tipo_contenedor);
    wifiManager.addParameter(custom_altura_max);
    wifiManager.addParameter(custom_capacidad);
    wifiManager.addParameter(custom_distancia_min);
    
    // Configurar callbacks
    wifiManager.setSaveConfigCallback(saveParamCallback);
    
    // Configurar portal cautivo
    wifiManager.setConfigPortalTimeout(180);
    
    // Mostrar mensaje inicial
    Heltec.display->clear();
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->drawString(64, 0, F("SENSOR DE"));
    Heltec.display->drawString(64, 16, F("NIVEL DE AGUA"));
    
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->drawString(0, 35, F("Iniciando WiFi..."));
    Heltec.display->drawString(0, 45, "SSID: " + String(AP_SSID));
    Heltec.display->display();
    
    // Intentar conectar o iniciar portal cautivo
    if (wifiManager.autoConnect(AP_SSID, AP_PASS)) {
        Serial.println(F("Conectado a WiFi!"));
        Serial.print(F("IP: "));
        Serial.println(WiFi.localIP());
        
        // Asegurar que la configuración se guarde
        if (WiFi.SSID().length() > 0) {
            WiFi.setHostname(config.hostname.c_str());  // Usar WiFi.setHostname en lugar de wifiManager.setHostname
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
    
    // Actualizar display con información de conexión
    Heltec.display->clear();
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->drawString(64, 0, F("WiFi Conectado"));
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->drawString(0, 15, "Red: " + WiFi.SSID());
    Heltec.display->drawString(0, 25, "IP: " + WiFi.localIP().toString());
    Heltec.display->drawString(0, 35, "MAC: " + WiFi.macAddress());
    Heltec.display->display();
    delay(3000);
    
    // Configurar eventos WiFi
    WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
        switch(event) {
            case SYSTEM_EVENT_STA_CONNECTED:
                Serial.println(F("WiFi conectado"));
                wifiConnected = true;
                break;
            case SYSTEM_EVENT_STA_DISCONNECTED:
                Serial.println(F("WiFi desconectado"));
                wifiConnected = false;
                // No intentamos reconectar aquí, lo hacemos en el loop
                break;
            case SYSTEM_EVENT_STA_GOT_IP:
                Serial.println("IP obtenida: " + WiFi.localIP().toString());
                break;
            default:
                break;
        }
    });
    
    // Inicializar servicios web
    initWebSocket();
    initWebSerial();
    
    // Configurar rutas del servidor web
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html", false, processor);
    });
    
    server.on("/Sensor", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", mensaje_error);
    });
    
    server.on("/Litros", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(getLitros()));
    });
    
    server.on("/Distancia", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", String(distanciaCm));
    });
    
    server.on("/toggleDisplay", HTTP_GET, [](AsyncWebServerRequest *request) {
        displayEnabled = !displayEnabled;
        if (displayEnabled) {
            Heltec.display->displayOn();
            oledStartTime = millis();
        } else {
            Heltec.display->displayOff();
        }
        saveConfig();
        request->send(200, "text/plain", displayEnabled ? "Display ON" : "Display OFF");
    });
    
    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Reiniciando...");
        delay(100);
        ESP.restart();
    });
    
    // Servir archivos estáticos
    server.serveStatic("/", SPIFFS, "/");
    
    // Inicializar OTA
    ElegantOTA.begin(&server);
    
    // Iniciar servidor
    server.begin();
    
    // Inicializar temporizadores
    oledStartTime = millis();
    previousMillis = millis();
}

// Botón PRG
// Función para mostrar la cuenta regresiva
void showCountdown(const char* message, int seconds) {
    Heltec.display->clear();
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->drawString(64, 10, message);
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(64, 30, String(seconds) + " segundos...");
    Heltec.display->display();
}

void handleDisplayStateChange(bool turnOn) {
    if (turnOn) {
        Heltec.display->displayOn();
        ledcWrite(LED_CHANNEL, 255);  // Encender LED al máximo
        oledStartTime = millis();
    } else {
        // Mostrar cuenta regresiva
        for (int i = 3; i > 0; i--) {
            showCountdown("Apagando display", i);
            delay(1000);
        }
        Heltec.display->displayOff();
        ledcWrite(LED_CHANNEL, 0);  // Apagar LED
    }
}

void handlePrgButton() {
    static unsigned long lastDebounceTime = 0;
    static bool lastButtonState = HIGH;
    bool buttonState = digitalRead(PRG_BUTTON_PIN);

    if (buttonState != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > 50) {
        if (buttonState == LOW) {
            displayEnabled = !displayEnabled;
            handleDisplayStateChange(displayEnabled);
            saveConfig();
            ws.textAll(displayEnabled ? "1" : "0");
        }
    }
    lastButtonState = buttonState;
}

// Funciones de manejo OTA
void otaStart() {
    isUpdating = true;
    // Mostrar mensaje de actualización
    Heltec.display->clear();
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->drawString(64, 10, "Actualizando...");
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(64, 35, "No desconecte");
    Heltec.display->display();
}

void otaProgress(size_t current, size_t total) {
    static unsigned long lastUpdate = 0;
    unsigned long now = millis();
    
    // Actualizar solo cada 100ms para no sobrecargar
    if (now - lastUpdate > 100) {
        lastUpdate = now;
        
        // Calcular porcentaje
        int progress = (current * 100) / total;
        
        // Mostrar barra de progreso
        Heltec.display->drawProgressBar(10, 50, 110, 10, progress);
        Heltec.display->display();
    }
}

void otaEnd(bool success) {
    isUpdating = false;
    if (success) {
        Heltec.display->clear();
        Heltec.display->setFont(ArialMT_Plain_16);
        Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
        Heltec.display->drawString(64, 10, "Actualización");
        Heltec.display->drawString(64, 30, "Completada!");
        Heltec.display->display();
        delay(2000);
    }
}

void updateLedDimmer() {
    if (isUpdating && millis() - lastFadeTime > FADE_INTERVAL) {
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

void loop() {
    ws.cleanupClients();
    ElegantOTA.loop();
    WebSerial.loop();
    handlePrgButton();
    unsigned long currentMillis = millis();
    
    // Actualizar efecto dimmer del LED durante OTA
    updateLedDimmer();
    
    // Manejar reconexión WiFi
    if (!wifiConnected && (currentMillis - lastWifiRetryMillis >= wifiRetryInterval)) {
        Serial.println(F("Intentando reconexión WiFi..."));
        lastWifiRetryMillis = currentMillis;
        
        if (WiFi.status() != WL_CONNECTED) {
            WiFi.disconnect();
            delay(1000);
            WiFi.reconnect();
        }
        
        // Actualizar mensaje en el display
        if (displayEnabled) {
            Heltec.display->clear();
            Heltec.display->setFont(ArialMT_Plain_10);
            Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
            Heltec.display->drawString(64, 10, F("Reconectando"));
            Heltec.display->drawString(64, 25, F("WiFi..."));
            Heltec.display->display();
        }
    }
    
    // Lectura periódica del sensor
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        
        float medida = distanceSensor.measureDistanceCm();
        if (medida >= 0 && medida <= 400) {
            distanciaCm = medida;
            mensaje_error = "OK";
            Serial.printf("Distancia: %.1f cm\n", distanciaCm);
        } else {
            mensaje_error = "Error de lectura";
            Serial.println(F("Error en la lectura del sensor"));
        }
        
        // Actualizar display si está activo
        if (displayEnabled) {
            Heltec.display->clear();
            
            // Título
            Heltec.display->setFont(ArialMT_Plain_16);
            Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
            Heltec.display->drawString(64, 0, "Sensor de Nivel");
            
            // Información del sensor
            Heltec.display->setFont(ArialMT_Plain_10);
            Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
            
            // Mostrar IP con formato adecuado
            String ip = WiFi.localIP().toString();
            Heltec.display->drawString(0, 18, "IP: " + ip);
            
            // Mostrar distancia
            String distStr = (distanciaCm < 0) ? "Error" : String(distanciaCm, 1) + " cm";
            Heltec.display->drawString(0, 30, "Dist: " + distStr);
            
            // Mostrar litros
            String litrosStr = getLitros() + " L";
            Heltec.display->drawString(0, 42, "Litros: " + litrosStr);
            
            // Firma
            Heltec.display->setTextAlignment(TEXT_ALIGN_RIGHT);
            Heltec.display->drawString(128, 54, "by DataTech");
            
            Heltec.display->display();
        }
    }
    
    // Control de timeout del display
    if (displayEnabled && (currentMillis - oledStartTime >= oledTimeout)) {
        Serial.println(F("Apagando display por timeout"));
        displayEnabled = false;
        Heltec.display->displayOff();
        saveConfig();
        ws.textAll("0");
    }
    
    // Otras tareas periódicas
    ElegantOTA.loop();
}