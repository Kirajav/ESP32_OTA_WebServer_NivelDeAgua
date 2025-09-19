// --- INICIO DEL ARCHIVO ---
#include <Arduino.h>
#include <WiFi.h>
#include <heltec.h>
#include "SPIFFS.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <WebSerial.h>
#include <ElegantOTA.h>
#include <HCSR04.h>
#include <ArduinoJson.h>


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

unsigned long oledStartTime = 0;
const long oledTimeout = 300000;

// --- Variables para Debounce del Botón ---
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
int lastButtonState = HIGH;

// Declaraciones adelantadas de funciones
String processor(const String& var);
String processor(const String& var);
String getImagen();
String getLitros();
void initWebSocket();
void initWebSerial();
void initSPIFFS();
void onWebSerialMessage(uint8_t *data, size_t len);
void updateGlobalsFromConfig();

// --- Estructura de Configuración ---
struct Config { double altura_max; double capacidad; double distancia_min; };
Config config;

// --- Objetos Globales ---
AsyncWebServer server(80);
DNSServer dns;
AsyncWiFiManager wifiManager(&server, &dns);
AsyncWebSocket ws("/ws");
UltraSonicDistanceSensor distanceSensor(TRIGGER_PIN, ECHO_PIN_1);
AsyncWiFiManagerParameter* custom_altura_max = nullptr;
AsyncWiFiManagerParameter* custom_capacidad = nullptr;
AsyncWiFiManagerParameter* custom_distancia_min = nullptr;

//======================================================================================//
//                          FUNCIONES AUXILIARES Y DE CONFIGURACIÓN                     //
//======================================================================================//

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            // Enviar el estado actual del display al cliente recién conectado
            client->text(displayEnabled ? "1" : "0");
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA: {
            AwsFrameInfo *info = (AwsFrameInfo*)arg;
            if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
                data[len] = 0;
                if (strcmp((char*)data, "toggle") == 0) {
                    ledState = !ledState;
                    ws.textAll(String(ledState));
                }
            }
            break;
        }
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

String processor(const String& var) {
    if (var == "ESTADO_OLED") {
        return ledState ? "encendida" : "apagada";
    }
    if (var == "ESTADO_SENSOR") {
        return mensaje_error;
    }
    if (var == "LITROS") {
        return getLitros();
    }
    if (var == "DISTANCIA") {
        if (distanciaCm < 0) {
            return "Error";
        }
        return String(distanciaCm);
    }
    return String();
}

String getImagen() {
    if (distanciaCm <= DISTANCIA_MINIMA_SENSOR) return "/imagen_lleno.jpg";
    if (ALTURA_MAX_AGUA_TINACO <= 0) return "/imagen_vacio.jpg"; // Evitar división por cero
    double alturaActualAgua = ALTURA_MAX_AGUA_TINACO - distanciaCm;
    double porcentaje = (alturaActualAgua / ALTURA_MAX_AGUA_TINACO) * 100.0;
    if (porcentaje > 100.0) porcentaje = 100.0;
    if (porcentaje < 0.0) porcentaje = 0.0;
    if (porcentaje > 80.0) return "/imagen_4.jpg";
    else if (porcentaje > 60.0) return "/imagen_4.jpg";
    else if (porcentaje > 40.0) return "/imagen_3.jpg";
    else if (porcentaje > 20.0) return "/imagen_2.jpg";
    else if (porcentaje > 1.0) return "/imagen_1.jpg";
    else return "/imagen_vacio.jpg";
}

String getLitros() {
    if (distanciaCm < 0) return "Error";
    if (distanciaCm <= DISTANCIA_MINIMA_SENSOR) return String(CAPACIDAD_LITROS_TINACO, 1);
    if (ALTURA_MAX_AGUA_TINACO <= 0) return "0.0"; // Evitar división por cero
    double alturaActualAgua = ALTURA_MAX_AGUA_TINACO - distanciaCm;
    if (alturaActualAgua < 0) alturaActualAgua = 0;
    double porcentajeLlenado = (alturaActualAgua / ALTURA_MAX_AGUA_TINACO) * 100.0;
    if (porcentajeLlenado > 100.0) porcentajeLlenado = 100.0;
    if (porcentajeLlenado < 0.0) porcentajeLlenado = 0.0;
    return String((CAPACIDAD_LITROS_TINACO * porcentajeLlenado) / 100.0, 1);
}

void initWebSocket() { ws.onEvent(onWebSocketEvent); server.addHandler(&ws); }
void initWebSerial() { 
    WebSerial.begin(&server); 
    WebSerial.onMessage(onWebSerialMessage);
}

void onWebSerialMessage(uint8_t *data, size_t len) {
  // La forma más robusta de procesar el comando, inmune a problemas de búfer.
  String command = String((char*)data).substring(0, len);
  command.toLowerCase();
  
  if (command == "distancia") { // Comandos de una sola línea
    WebSerial.println("Distancia actual: " + String(distanciaCm) + " cm");
  } else if (command == "litros") {
    WebSerial.println("Litros actuales: " + getLitros() + " L");
  } else if (command == "ip") {
    WebSerial.println("Dirección IP: " + WiFi.localIP().toString());
  } else if (command == "submask") {
    WebSerial.println("Mascara de Subred: " + WiFi.subnetMask().toString());
  } else if (command == "gateway ip") {
    WebSerial.println("Gateway IP: " + WiFi.gatewayIP().toString());
  } else if (command == "dns") {
    WebSerial.println("DNS 1: " + WiFi.dnsIP(0).toString());
    WebSerial.println("DNS 2: " + WiFi.dnsIP(1).toString());
  } else if (command == "mac") {
    WebSerial.println("Dirección MAC: " + WiFi.macAddress());
  } else if (command == "display") {
    WebSerial.println(String("Estado del display: ") + (displayEnabled ? "Encendida" : "Apagada"));
  } else if (command == "sensor") {
    WebSerial.println("Estado del sensor: " + mensaje_error);
  } else if (command == "monitor") {
    monitorEnabled = !monitorEnabled;
    if (monitorEnabled) {
        WebSerial.println("Modo monitor iniciado. Enviando datos cada 5 segundos...");
    } else {
        WebSerial.println("Modo monitor detenido.");
    }
  } else if (command == "allinfo") { // Comandos de múltiples líneas
    String lines[] = {
        "\n+------------------+----------------------------------------+",
        "| CATEGORIA        | VALOR                                  |",
        "+------------------+----------------------------------------+",
        "| -- Conectividad --                                       |",
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
        "" // Usamos una cadena vacía para marcar el final
    };
    for (int i = 0; !lines[i].isEmpty(); ++i) { 
        WebSerial.println(lines[i]);
        vTaskDelay(10 / portTICK_PERIOD_MS); // Pequeña pausa para evitar saturar el buffer
    }
  } else if (command == "?" || command == "help" || command == "ayuda") {
    String lines[] = {
        "--- Lista de Comandos Disponibles ---",
        "distancia      - Muestra la distancia actual del sensor al agua.",
        "litros         - Muestra la cantidad de litros actual en el tinaco.",
        "ip             - Muestra la dirección IP del dispositivo.",
        "submask        - Muestra la mascara de subred.",
        "gateway ip     - Muestra la IP del gateway.",
        "dns            - Muestra las direcciones de los servidores DNS.",
        "mac            - Muestra la dirección MAC del dispositivo.",
        "display        - Muestra el estado actual de la pantalla OLED.",
        "sensor         - Muestra el estado de la conexión con el sensor.",
        "allinfo        - Muestra toda la información disponible en una tabla.",
        "monitor        - Inicia/detiene la impresión continua de datos del sensor.",
        "ayuda, help, ? - Muestra este mensaje de ayuda.",
        "-------------------------------------",
        "" // Usamos una cadena vacía para marcar el final
    };
    for (int i = 0; !lines[i].isEmpty(); ++i) { 
        WebSerial.println(lines[i]);
        vTaskDelay(10 / portTICK_PERIOD_MS); // Pequeña pausa para evitar saturar el buffer
    }
  }

}

void initSPIFFS() { if(!SPIFFS.begin(true)) { Serial.println("Error montando SPIFFS"); } }

void updateGlobalsFromConfig() {
    ALTURA_MAX_AGUA_TINACO = config.altura_max;
    CAPACIDAD_LITROS_TINACO = config.capacidad;
    DISTANCIA_MINIMA_SENSOR = config.distancia_min;
}

void saveConfig() {
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
        Serial.println("Error al abrir config.json para escritura");
        return;
    }
    StaticJsonDocument<512> doc;
    doc["altura_max"] = config.altura_max;
    doc["capacidad"] = config.capacidad;
    doc["distancia_min"] = config.distancia_min;
    doc["display_on"] = displayEnabled;
    serializeJson(doc, configFile);
    configFile.close();
    Serial.println("Configuración guardada en config.json");
}

void saveParamCallback() {
    config.altura_max = atof(custom_altura_max->getValue());
    config.capacidad = atof(custom_capacidad->getValue());
    config.distancia_min = atof(custom_distancia_min->getValue());
    File configFile = SPIFFS.open("/config.json", "w");
    StaticJsonDocument<512> doc;
    doc["altura_max"] = config.altura_max;
    doc["capacidad"] = config.capacidad;
    doc["distancia_min"] = config.distancia_min;
    doc["display_on"] = displayEnabled;
    serializeJson(doc, configFile); // Reutilizamos la lógica de guardado
    configFile.close();
    updateGlobalsFromConfig();
}

bool loadConfig() {
    if (SPIFFS.begin(true)) {
        if (SPIFFS.exists("/config.json")) {
            File configFile = SPIFFS.open("/config.json", "r");
            StaticJsonDocument<512> doc;
            if (deserializeJson(doc, configFile) == DeserializationError::Ok) {
                // Forma correcta de asignar valores por defecto con ArduinoJson
                config.altura_max = doc["altura_max"] | ALTURA_MAX_AGUA_TINACO; // El operador OR es correcto aquí, ArduinoJson lo sobrecarga.
                config.capacidad = doc["capacidad"] | CAPACIDAD_LITROS_TINACO;
                config.distancia_min = doc["distancia_min"] | DISTANCIA_MINIMA_SENSOR;
                displayEnabled = doc["display_on"] | true; // Por defecto, el display está encendido
                configFile.close();
                Serial.println("Configuración cargada desde config.json");
                return true;
            }
        } else {
            // Si el archivo no existe, lo creamos con los valores por defecto
            Serial.println("No se encontró config.json, creando archivo con valores por defecto.");
            config.altura_max = ALTURA_MAX_AGUA_TINACO;
            config.capacidad = CAPACIDAD_LITROS_TINACO;
            config.distancia_min = DISTANCIA_MINIMA_SENSOR;
            displayEnabled = true;
            saveConfig(); // Llama a la nueva función de guardado segura
        }
    }
    displayEnabled = true; // Valor por defecto si no hay config
    return false;
}

void toggleDisplayAndLed() {
    displayEnabled = !displayEnabled;
    digitalWrite(LED_PIN, displayEnabled ? HIGH : LOW); // Simple HIGH/LOW
    if (displayEnabled) {
        Heltec.display->displayOn();
        oledStartTime = millis();
        Serial.println("Display y LED encendidos.");
    } else {
        Heltec.display->displayOff();
        Serial.println("Display y LED apagados.");
    }
    ws.textAll(displayEnabled ? "1" : "0");
}

void otaStart() {
    isUpdating = true;
    Heltec.display->clear(); // Limpia el búfer
    Heltec.display->setColor(BLACK); // Establece el color de dibujo a NEGRO
    Heltec.display->fillRect(0, 0, 128, 64); // Dibuja el rectángulo para borrar la pantalla
    Heltec.display->setColor(WHITE); // Restablece el color de dibujo a BLANCO para el texto
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->drawString(64, 10, "Actualizando...");
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(64, 35, "No desconecte.");
    Heltec.display->display();
    delay(100); // Pequeña pausa para asegurar que el display se actualice antes de que OTA tome el control.
}

void otaEnd(bool success) {
    isUpdating = false;
}
void otaProgress(size_t, size_t) { Serial.print("."); }

void handleButtonPress() {
    if ((millis() - lastDebounceTime) > debounceDelay) {
        int reading = digitalRead(PRG_BUTTON_PIN);
        if (reading == LOW && lastButtonState == HIGH) {
            Serial.println("Botón PRG presionado.");
            toggleDisplayAndLed();
        }
        lastButtonState = reading;
    }
}

void apCallback(AsyncWiFiManager *myWiFiManager) {
    Heltec.display->clear();
    Heltec.display->display(); // Forzar actualización
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
    Heltec.display->drawString(64, 0, "Sensor de Nivel de Agua");
    Heltec.display->setFont(ArialMT_Plain_16);
    Heltec.display->drawString(64, 12, "¡Conéctate a la red!");
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(0, 30, "Red: " + myWiFiManager->getConfigPortalSSID());
    Heltec.display->drawString(0, 40, "IP: 192.168.4.1");
    Heltec.display->drawString(0, 50, "Contraseña: 12345");
    Heltec.display->display();
}

//======================================================================================//
//                                 CONFIGURACIÓN (SETUP)                                //
//======================================================================================//

void setup() {
    Serial.begin(115200);
    Serial.println("\nIniciando...");

    // --- Secuencia de inicio de OLED (Original del usuario) ---
    pinMode(Vext, OUTPUT);
    digitalWrite(Vext, LOW);
    delay(100);
    pinMode(OLED_RST, OUTPUT);
    digitalWrite(OLED_RST, LOW);
    delay(50);
    digitalWrite(OLED_RST, HIGH);
    delay(50);
    Wire.begin(OLED_SDA, OLED_SCL);
    // --- Fin de secuencia ---

    pinMode(PRG_BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT); // Simple pinMode para LED

    // La solución definitiva:
    // 1. Llamamos a Heltec.begin() con el segundo parámetro en 'false' para que NO inicialice el display.
    Heltec.begin(true /*Serial*/, false /*Display*/, true /*LoRa*/);
    // 2. Ahora, inicializamos el display manualmente, sin los mensajes de depuración de la librería.
    Heltec.display->init();
    Heltec.display->flipScreenVertically();
    Heltec.display->setContrast(255);
    Heltec.display->clear();
    loadConfig();
    updateGlobalsFromConfig();

    digitalWrite(LED_PIN, displayEnabled ? HIGH : LOW); // Control simple del LED
    if (displayEnabled) { oledStartTime = millis(); } else { Heltec.display->displayOff(); }

    wifiManager.setAPCallback(apCallback);
    custom_altura_max = new AsyncWiFiManagerParameter("altura_max", "Altura máx. (cm)", String(config.altura_max).c_str(), 10);
    custom_capacidad = new AsyncWiFiManagerParameter("capacidad", "Capacidad (L)", String(config.capacidad).c_str(), 10);
    custom_distancia_min = new AsyncWiFiManagerParameter("distancia_min", "Distancia mín. (cm)", String(config.distancia_min).c_str(), 10);
    wifiManager.addParameter(custom_altura_max);
    wifiManager.addParameter(custom_capacidad);
    wifiManager.addParameter(custom_distancia_min);
    wifiManager.setSaveConfigCallback(saveParamCallback);
    wifiManager.setConfigPortalTimeout(180);

    if (!wifiManager.autoConnect("ESP32: Sensor de nivel de agua", "12345")) {
        Serial.println("Fallo de conexión, reiniciando...");
        delay(3000);
        ESP.restart();
    }

    Serial.println("Conectado a WiFi!");
    initWebSerial();
    initWebSocket();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, "/index.html", "text/html", false, processor); });
    server.on("/Sensor", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(200, "text/plain", mensaje_error); });
    server.on("/Litros", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(200, "text/plain", getLitros()); });
    server.on("/Distancia", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(200, "text/plain", String(distanciaCm)); });
    server.on("/imagen", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(SPIFFS, getImagen(), "image/jpg"); });
    server.on("/toggleDisplay", HTTP_GET, [](AsyncWebServerRequest *request) {
        toggleDisplayAndLed();
        request->send(200, "text/plain", displayEnabled ? "Display Encendido" : "Display Apagado");
    });
    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) { request->send(200, "text/plain", "OK"); ESP.restart(); });
    server.on("/reset-wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "OK");
        wifiManager.resetSettings();
        delay(500);
        ESP.restart();
    });

    server.serveStatic("/", SPIFFS, "/");
    
    ElegantOTA.onStart(otaStart);
    ElegantOTA.onEnd(otaEnd);
    ElegantOTA.onProgress(otaProgress);
    ElegantOTA.begin(&server);
    
    server.begin();
    Serial.println("Servidor HTTP iniciado.");

    // Crear una tarea separada para la lectura del sensor
    xTaskCreate(
      [](void* pvParameters) {
        for (;;) {
          float medida = distanceSensor.measureDistanceCm();
          if (medida >= 0 && medida <= 400) { distanciaCm = medida; mensaje_error = "OK"; } 
          else { distanciaCm = -1; mensaje_error = "Error"; }
          vTaskDelay(5000 / portTICK_PERIOD_MS); // Esperar 5 segundos
        }
      },
      "SensorReadTask", 2048, NULL, 1, NULL
    );
}

//======================================================================================//
//                                  BUCLE PRINCIPAL (LOOP)                              //
//======================================================================================//

void loop() {
    ElegantOTA.loop();
    WebSerial.loop();
    handleButtonPress();

    if (isUpdating) {
        // Parpadeo simple del LED durante la actualización
        unsigned long currentMillis = millis();
        if (currentMillis % 1000 < 500) { // 500ms ON, 500ms OFF
            digitalWrite(LED_PIN, HIGH);
        } else {
            digitalWrite(LED_PIN, LOW);
        }
        return;
    }

    unsigned long currentMillis = millis();

    if (monitorEnabled) {
        static unsigned long lastMonitorTime = 0;
        if (currentMillis - lastMonitorTime >= 5000) {
            lastMonitorTime = currentMillis;
            WebSerial.println("Distancia: " + String(distanciaCm) + " cm, Litros: " + getLitros() + " L");
        }
    }

    if (displayEnabled && (currentMillis - oledStartTime >= oledTimeout)) {
        Serial.println("Apagando display por timeout.");
        toggleDisplayAndLed();
    }

    if (displayEnabled) {
        static unsigned long lastDisplayUpdate = 0;
        if (currentMillis - lastDisplayUpdate >= 1000) {
            lastDisplayUpdate = currentMillis;
            Heltec.display->clear();
            Heltec.display->setFont(ArialMT_Plain_16);
            Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
            Heltec.display->drawString(64, 0, "Sensor de Nivel");
            Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
            Heltec.display->setFont(ArialMT_Plain_10);
            Heltec.display->drawString(0, 18, "IP: " + WiFi.localIP().toString());
            Heltec.display->drawString(0, 30, "Dist: " + (distanciaCm < 0 ? "Error" : String(distanciaCm, 1) + " cm"));
            Heltec.display->drawString(0, 42, "Litros: " + getLitros() + " L");
            Heltec.display->drawString(0, 54, "by DataTech");
            Heltec.display->display();
        }
    }
}