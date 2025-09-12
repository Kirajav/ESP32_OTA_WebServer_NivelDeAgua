// Primero los includes básicos
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

// Defines y constantes
#define LED_PIN 25  // LED de la placa Heltec
#define TRIGGER_PIN 12 // Pin trigger del sensor HC-SR04
#define ECHO_PIN_1 13  // Pin echo del sensor HC-SR04

// Variables globales
// Distancia en cm desde el sensor hasta el fondo del tinaco (cuando está vacío). Este valor lo configura el usuario.
double ALTURA_MAX_AGUA_TINACO = 115.0;
// Capacidad total en litros del tinaco. Este valor lo configura el usuario.
double CAPACIDAD_LITROS_TINACO = 1200.0;
// Distancia mínima de operación del sensor ultrasónico en cm. Lecturas por debajo de este valor no son fiables y se consideran "lleno".
double DISTANCIA_MINIMA_SENSOR = 19.0;

bool ledState = false;
bool OLEDapagado = false;
bool displayEnabled = true;  // Estado del display, aunque no se usa directamente en el loop original
double distanciaCm = 0;
String mensaje_error = "";
bool monitorEnabled = false; // Flag para el modo monitor de WebSerial

// Variables para el temporizador de lectura del sensor
unsigned long previousMillis = 0;
const long interval = 5000; // Intervalo de 5 segundos para la lectura

// Estructura para la configuración
struct Config {
    double altura_max;
    double capacidad;
    double distancia_min;
};

Config config;

// Declaraciones adelantadas de funciones
String processor(const String& var);
String getImagen();
String getLitros();
void initWebSocket();
void initWebSerial();
void initSPIFFS();
void onWebSerialMessage(uint8_t *data, size_t len);
void updateGlobalsFromConfig();

// Objetos globales
AsyncWebServer server(80);
DNSServer dns;
AsyncWiFiManager wifiManager(&server, &dns);
AsyncWebSocket ws("/ws");
UltraSonicDistanceSensor distanceSensor(TRIGGER_PIN, ECHO_PIN_1);

// Variables para los parámetros WiFi
AsyncWiFiManagerParameter* custom_altura_max = nullptr;
AsyncWiFiManagerParameter* custom_capacidad = nullptr;
AsyncWiFiManagerParameter* custom_distancia_min = nullptr;

// Implementación de funciones
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
    // Si la distancia medida es menor que la distancia mínima de operación del sensor,
    // se asume que el tinaco está lleno para evitar lecturas no fiables.
    if (distanciaCm <= DISTANCIA_MINIMA_SENSOR) {
        return "/imagen_lleno.jpg";
    }

    // Se calcula la altura del agua basándose en la distancia medida desde el sensor al fondo del tinaco.
    double alturaActualAgua = ALTURA_MAX_AGUA_TINACO - distanciaCm;
    
    // El porcentaje se calcula en relación a la altura total del tinaco.
    // Este cálculo no usa DISTANCIA_MINIMA_SENSOR en la fórmula, según solicitado.
    // Esto puede causar que el tinaco nunca marque 100% en este cálculo, por eso el caso de "lleno" se trata por separado.
    double porcentaje = (alturaActualAgua / ALTURA_MAX_AGUA_TINACO) * 100.0;

    // Asegurarse que el porcentaje esté dentro del rango 0-100.
    if (porcentaje > 100.0) porcentaje = 100.0;
    if (porcentaje < 0.0) porcentaje = 0.0;

    // Devuelve la imagen correspondiente al porcentaje.
    if (porcentaje > 80.0) {
        return "/imagen_5.jpg";
    } else if (porcentaje > 60.0) {
        return "/imagen_4.jpg";
    } else if (porcentaje > 40.0) {
        return "/imagen_3.jpg";
    } else if (porcentaje > 20.0) {
        return "/imagen_2.jpg";
    } else if (porcentaje > 1.0) { // Un poco más que vacío
        return "/imagen_1.jpg";
    } else { // Prácticamente vacío
        return "/imagen_vacio.jpg";
    }
}

String getLitros() {
    if (distanciaCm < 0) { // Error de lectura del sensor
        return "Error";
    }
    // Si la distancia es menor o igual a la mínima del sensor, se considera lleno.
    if (distanciaCm <= DISTANCIA_MINIMA_SENSOR) {
        return String(CAPACIDAD_LITROS_TINACO, 1);
    }

    // La altura actual del agua es la distancia desde el fondo hasta la superficie.
    double alturaActualAgua = ALTURA_MAX_AGUA_TINACO - distanciaCm;
    if (alturaActualAgua < 0) alturaActualAgua = 0;

    // Según tu solicitud, la altura total del agua se considera como ALTURA_MAX_AGUA_TINACO.
    double alturaTotalAgua = ALTURA_MAX_AGUA_TINACO;
    if (alturaTotalAgua <= 0) return "0.0"; // Evitar división por cero

    // Se calcula el porcentaje de llenado.
    double porcentajeLlenado = (alturaActualAgua / alturaTotalAgua) * 100.0;
    if (porcentajeLlenado > 100.0) porcentajeLlenado = 100.0;
    if (porcentajeLlenado < 0.0) porcentajeLlenado = 0.0;

    // Se calculan los litros.
    double litros = (CAPACIDAD_LITROS_TINACO * porcentajeLlenado) / 100.0;

    return String(litros, 1); // Devolver con 1 decimal
}

void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
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

void initWebSocket() {
    ws.onEvent(onWebSocketEvent);
    server.addHandler(&ws);
}

void onWebSerialMessage(uint8_t *data, size_t len) {
  String command = "";
  for(size_t i=0; i < len; i++){
    command += (char)data[i];
  }
  command.trim();
  command.toLowerCase();
  
  if (command == "distancia") {
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
  } else if (command == "allinfo") {
    WebSerial.println(F("\n+------------------+----------------------------------------+"));
    WebSerial.println(F("| CATEGORIA        | VALOR                                  |"));
    WebSerial.println(F("+------------------+----------------------------------------+"));
    WebSerial.println(F("| -- Conectividad --                                       |"));
    WebSerial.println("| Direccion IP     | " + WiFi.localIP().toString());
    WebSerial.println("| Mascara Subred   | " + WiFi.subnetMask().toString());
    WebSerial.println("| Gateway IP       | " + WiFi.gatewayIP().toString());
    WebSerial.println("| DNS 1            | " + WiFi.dnsIP(0).toString());
    WebSerial.println("| DNS 2            | " + WiFi.dnsIP(1).toString());
    WebSerial.println("| Direccion MAC    | " + WiFi.macAddress());
    WebSerial.println(F("|                                                        |"));
    WebSerial.println(F("| -- Sensor --                                            |"));
    WebSerial.println("| Estado           | " + mensaje_error);
    WebSerial.println("| Distancia        | " + String(distanciaCm) + " cm");
    WebSerial.println("| Litros           | " + getLitros() + " L");
    WebSerial.println(F("|                                                        |"));
    WebSerial.println(F("| -- Dispositivo --                                       |"));
    WebSerial.println(String("| Estado Display   | ") + (displayEnabled ? "Encendida" : "Apagada"));
    WebSerial.println(F("+------------------+----------------------------------------+"));
  } else if (command == "?" || command == "help" || command == "ayuda") {
    WebSerial.println("--- Lista de Comandos Disponibles ---");
    WebSerial.println("distancia      - Muestra la distancia actual del sensor al agua.");
    WebSerial.println("litros         - Muestra la cantidad de litros actual en el tinaco.");
    WebSerial.println("ip             - Muestra la dirección IP del dispositivo.");
    WebSerial.println("submask        - Muestra la mascara de subred.");
    WebSerial.println("gateway ip     - Muestra la IP del gateway.");
    WebSerial.println("dns            - Muestra las direcciones de los servidores DNS.");
    WebSerial.println("mac            - Muestra la dirección MAC del dispositivo.");
    WebSerial.println("display        - Muestra el estado actual de la pantalla OLED.");
    WebSerial.println("sensor         - Muestra el estado de la conexión con el sensor.");
    WebSerial.println("allinfo        - Muestra toda la información disponible en una tabla.");
    WebSerial.println("monitor        - Inicia/detiene la impresión continua de datos del sensor.");
    WebSerial.println("ayuda, help, ? - Muestra este mensaje de ayuda.");
    WebSerial.println("-------------------------------------");
  } else {
    WebSerial.println("Comando no reconocido. Escribe 'ayuda' para ver la lista de comandos.");
  }
}

void initWebSerial() {
    WebSerial.begin(&server);
    WebSerial.onMessage(onWebSerialMessage);
}

void initSPIFFS() {
    if(!SPIFFS.begin(true)) {
        Serial.println("Error montando SPIFFS");
        return;
    }
}

// Función para guardar la configuración
void saveParamCallback() {
    Serial.println("Guardando parámetros...");
    if(custom_altura_max && custom_capacidad && custom_distancia_min) {
        config.altura_max = atof(custom_altura_max->getValue());
        config.capacidad = atof(custom_capacidad->getValue());
        config.distancia_min = atof(custom_distancia_min->getValue());
        
        StaticJsonDocument<512> doc;
        doc["altura_max"] = config.altura_max;
        doc["capacidad"] = config.capacidad;
        doc["distancia_min"] = config.distancia_min;
        doc["display_on"] = displayEnabled;

        File configFile = SPIFFS.open("/config.json", "w");
        if (configFile) {
            serializeJson(doc, configFile);
            configFile.close();
        }
        updateGlobalsFromConfig(); // Aplicar la nueva configuración inmediatamente
    }
}

void updateGlobalsFromConfig() {
    ALTURA_MAX_AGUA_TINACO = config.altura_max;
    CAPACIDAD_LITROS_TINACO = config.capacidad;
    DISTANCIA_MINIMA_SENSOR = config.distancia_min;
}

bool loadConfig() {
    if (SPIFFS.begin(true)) {
        File configFile = SPIFFS.open("/config.json", "r");
        if (configFile) {
            StaticJsonDocument<512> doc;
            DeserializationError error = deserializeJson(doc, configFile);
            
            if (!error) {
                config.altura_max = doc["altura_max"] | ALTURA_MAX_AGUA_TINACO;
                config.capacidad = doc["capacidad"] | CAPACIDAD_LITROS_TINACO;
                config.distancia_min = doc["distancia_min"] | DISTANCIA_MINIMA_SENSOR;
                displayEnabled = doc["display_on"] | true;
                OLEDapagado = !displayEnabled;
                
                configFile.close();
                return true;
            }
        }
    }
    // Valores por defecto si el archivo no existe o hay un error
    config.altura_max = ALTURA_MAX_AGUA_TINACO;
    config.capacidad = CAPACIDAD_LITROS_TINACO;
    config.distancia_min = DISTANCIA_MINIMA_SENSOR;
    displayEnabled = true;
    OLEDapagado = false;
    return false;
}

void setup() {
    Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);
    
    // Serial
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    // LED y OLED
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Inicializar SPIFFS y cargar configuración
    if(!SPIFFS.begin(true)) {
        Serial.println("Error montando SPIFFS");
    }
    loadConfig();
    updateGlobalsFromConfig(); // Aplicar la configuración cargada

    if (!displayEnabled) {
        Heltec.display->displayOff();
    } else {
        Heltec.display->displayOn();
    }

    // Crear parámetros personalizados
    custom_altura_max = new AsyncWiFiManagerParameter("altura_max", "Altura máxima (cm)", 
        String(config.altura_max).c_str(), 10, 
        "placeholder='115.0' title='Distancia en cm desde el sensor hasta el fondo cuando está vacío'");
    
    custom_capacidad = new AsyncWiFiManagerParameter("capacidad", "Capacidad (L)", 
        String(config.capacidad).c_str(), 10,
        "placeholder='1200.0' title='Capacidad máxima en litros del tinaco'");
    
    custom_distancia_min = new AsyncWiFiManagerParameter("distancia_min", "Distancia mínima (cm)", 
        String(config.distancia_min).c_str(), 10,
        "placeholder='19.0' title='Distancia mínima de lectura. Lecturas menores se consideran lleno'");

    // Agregar control del display y reinicio
    const char* custom_html = "<br/><label>Display OLED</label><input type='checkbox' name='display' value='1' checked><br/><br/>"
                            "<button onclick='location.href=\"/reset\"'>Reiniciar ESP32</button>";
    AsyncWiFiManagerParameter* custom_controls = new AsyncWiFiManagerParameter(custom_html);

    // Configurar WiFiManager
    wifiManager.addParameter(custom_altura_max);
    wifiManager.addParameter(custom_capacidad);
    wifiManager.addParameter(custom_distancia_min);
    wifiManager.addParameter(custom_controls);
    wifiManager.setSaveConfigCallback(saveParamCallback);
    
    // Configurar portal cautivo con IP personalizada
    IPAddress apIP(192, 168, 1, 1);
    IPAddress netMsk(255, 255, 255, 0);
    wifiManager.setAPStaticIPConfig(apIP, apIP, netMsk);
    wifiManager.setConfigPortalTimeout(180); // 3 minutos timeout

    // Callback cuando inicia el modo AP
    wifiManager.setAPCallback([](AsyncWiFiManager* myWiFiManager) {
        Heltec.display->clear();
        Heltec.display->setFont(ArialMT_Plain_10);
        Heltec.display->drawString(0, 0, "Modo Configuración WiFi");
        Heltec.display->drawString(0, 12, "Red: " + myWiFiManager->getConfigPortalSSID());
        Heltec.display->drawString(0, 24, "IP: 192.168.1.1");
        Heltec.display->drawString(0, 36, "MAC: " + WiFi.macAddress());
        Heltec.display->drawString(0, 48, "Pass: 1234");
        Heltec.display->display();
    });

    // Mostrar info en el display
    Heltec.display->clear();
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(0, 0, "Modo Configuración WiFi");
    Heltec.display->drawString(0, 12, "Red: WiFi ESP32: Sensor Tinaco");
    Heltec.display->drawString(0, 24, "IP: 192.168.1.1");
    Heltec.display->drawString(0, 36, "MAC: " + WiFi.macAddress());
    Heltec.display->drawString(0, 48, "Pass: 1234");
    Heltec.display->display();

    // Intentar conectar o iniciar portal cautivo
    if (!wifiManager.autoConnect("WiFi ESP32: Sensor Tinaco", "1234")) {
        Serial.println("Falló la conexión y el timeout expiró");
        delay(3000);
        ESP.restart();
    }

    // Continuar con el resto de la configuración
    // Print ESP Local IP Address
    Serial.println(WiFi.localIP());

    initWebSerial();
    
    // Initialize SPIFFS and WebSocket
    initSPIFFS();
    initWebSocket();

    // Ruta para la página raíz / web
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", "text/html", false, processor); });
              
    // Litros entregados al sitio web /Sensor
    server.on("/Sensor", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", mensaje_error.c_str());
    });
    
    // Litros entregados al sitio web /Litros
    server.on("/Litros", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", getLitros());
    });

    // Distancia entregados al sitio web /Distancia
    server.on("/Distancia", HTTP_GET, [](AsyncWebServerRequest *request) { 
        request->send(200, "text/plain", String(distanciaCm).c_str());
    });

    // Agregue la imagen al sitio web /imagen
    server.on("/imagen", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, getImagen().c_str(), "image/jpg"); });

    // Ruta para controlar el display
    server.on("/toggleDisplay", HTTP_GET, [](AsyncWebServerRequest *request) {
        displayEnabled = !displayEnabled;
        OLEDapagado = !displayEnabled;
        if(!displayEnabled) {
            Heltec.display->displayOff();
        } else {
            Heltec.display->displayOn();
        }

        // Guardar el estado del display
        File configFile = SPIFFS.open("/config.json", "w");
        if (configFile) {
            StaticJsonDocument<512> doc;
            doc["altura_max"] = config.altura_max;
            doc["capacidad"] = config.capacidad;
            doc["distancia_min"] = config.distancia_min;
            doc["display_on"] = displayEnabled;
            serializeJson(doc, configFile);
            configFile.close();
        }
        
        request->send(200, "text/plain", displayEnabled ? "Display ON" : "Display OFF");
    });

    // Ruta para reiniciar el ESP32
    server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", 
            "Reiniciando ESP32...<script>setTimeout(function(){ window.location.href = '/'; }, 5000);</script>");
        delay(500);
        ESP.restart();
    });

    server.serveStatic("/", SPIFFS, "/");

    // Initialize ElegantOTA with async support
    ElegantOTA.begin(&server);

    // Start server
    server.begin();
}

//--------------------------------------------------------------------------------------//
//                                         LOOP                                         //
//--------------------------------------------------------------------------------------//


void loop()
{
  // Leer la distancia del sensor cada 'interval' milisegundos
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    distanciaCm = distanceSensor.measureDistanceCm();
    if (distanciaCm < 0) {
      mensaje_error = "Error Sensor";
    } else {
      mensaje_error = "OK";
    }

    if (monitorEnabled) {
        String litros = getLitros();
        WebSerial.println("Distancia medida: " + String(distanciaCm, 1) + " [cm], " + litros + " [Litros] (Las medidas son aproximadas)");
    }
  }

  ws.cleanupClients();
  digitalWrite(LED_PIN, ledState);
  
  if (ledState && OLEDapagado)
  {
    Serial.println("Encendiendo pantalla y led.");
    WebSerial.println("Encendiendo pantalla y led.");
    Heltec.display->displayOn();
    OLEDapagado = false;
  }
  else if (!ledState && !OLEDapagado)
  {
    Serial.println("Apagando pantalla y led.");
    WebSerial.println("Apagando pantalla y led.");
    Heltec.display->displayOff();
    OLEDapagado = true;
  }
  if (!OLEDapagado)
  {
    Heltec.display->display();
    Heltec.display->clear();
    Heltec.display->setFont(ArialMT_Plain_16);
    // getLitros() actualiza la distancia y los litros
    String litrosStr = getLitros();
    Heltec.display->drawStringMaxWidth(0, 0, 128, "IP: " + WiFi.localIP().toString());
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawStringMaxWidth(0, 30, 128, "Distancia: " + String(distanciaCm / 100) + " [m]");
    Heltec.display->drawStringMaxWidth(0, 50, 128, "Litros: " + litrosStr + " [L]");
    delay(1000);
  }  
  
  // Manejar actualizaciones OTA
  ElegantOTA.loop();
}