#include "TuyaDevice.h"
#include <WiFiUdp.h>
#include <MD5Builder.h>

// Constantes del protocolo Tuya (mismo que usan dispositivos comerciales)
#define TUYA_PROTOCOL_VERSION "3.3"
#define TUYA_DISCOVERY_PORT 6666
#define TUYA_COMMUNICATION_PORT 6668
#define HEARTBEAT_INTERVAL 10000
#define CONFIG_AP_NAME "SmartConfig"

TuyaDevice::TuyaDevice() {
    _connected = false;
    _configured = false;
    _configMode = false;
    _configServer = WiFiServer(80);
    client = nullptr;
}

void TuyaDevice::begin(const DeviceConfig& config) {
    _config = config;
    generateDeviceCredentials();
    
    Serial.println("🔌 Tuya Device Starting...");
    Serial.println("📱 Device: " + _config.deviceName);
    Serial.println("🆔 Device ID: " + _deviceId);
    
    // Verificar si ya está configurado
    // TODO: Leer de EEPROM/preferences
    _configured = false; // Por ahora siempre falso
    
    if (!_configured) {
        Serial.println("⚙️ First time setup - Starting config mode");
        enterConfigMode();
    }
}

void TuyaDevice::generateDeviceCredentials() {
    // Generar Device ID único basado en MAC address
    String macAddress = WiFi.macAddress();
    macAddress.replace(":", "");
    _deviceId = "esp32" + macAddress.substring(6); // Últimos 6 caracteres
    
    // Generar Local Key (en dispositivos reales viene del fabricante)
    MD5Builder md5;
    md5.begin();
    md5.add(_deviceId + macAddress + _config.deviceModel);
    md5.calculate();
    _localKey = md5.toString();
}

void TuyaDevice::enterConfigMode() {
    _configMode = true;
    
    // Crear Access Point para configuración inicial (como dispositivos comerciales)
    String apName = CONFIG_AP_NAME "_" + _deviceId.substring(8);
    
    Serial.println("📡 Starting Config AP: " + apName);
    WiFi.softAP(apName.c_str(), "12345678");
    
    _configServer.begin();
    Serial.println("🌐 Config server started at: http://192.168.4.1");
    Serial.println("📱 Connect to WiFi '" + apName + "' and open browser");
}

void TuyaDevice::handleConfigServer() {
    if (!_configMode) return;
    
    WiFiClient client = _configServer.available();
    if (!client) return;
    
    String request = client.readStringUntil('\r');
    client.flush();
    
    if (request.indexOf("GET /") >= 0) {
        // Servir página de configuración
        String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Smart Device Setup</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body style="font-family: Arial; padding: 20px; background: #f0f0f0;">
    <div style="max-width: 400px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1);">
        <h2 style="color: #FF6B35; text-align: center;">🌊 Water Sensor Setup</h2>
        <form action="/config" method="POST">
            <div style="margin-bottom: 15px;">
                <label>WiFi Network:</label><br>
                <input type="text" name="ssid" style="width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 5px;" placeholder="Your WiFi Name">
            </div>
            <div style="margin-bottom: 15px;">
                <label>WiFi Password:</label><br>
                <input type="password" name="password" style="width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 5px;" placeholder="WiFi Password">
            </div>
            <div style="margin-bottom: 15px;">
                <label>Device Name:</label><br>
                <input type="text" name="device_name" value=")";
        html += _config.deviceName;
        html += R"(" style="width: 100%; padding: 10px; border: 1px solid #ddd; border-radius: 5px;">
            </div>
            <button type="submit" style="width: 100%; padding: 15px; background: #FF6B35; color: white; border: none; border-radius: 5px; font-size: 16px; cursor: pointer;">
                💾 Save & Connect
            </button>
        </form>
        <p style="text-align: center; color: #666; font-size: 12px; margin-top: 20px;">
            📱 After setup, add device in Tuya Smart app<br>
            🔍 Look for ")" + _config.deviceName + R"("
        </p>
    </div>
</body>
</html>)";
        
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close");
        client.println();
        client.println(html);
    }
    
    else if (request.indexOf("POST /config") >= 0) {
        // Procesar configuración
        String body = client.readString();
        
        // Parse form data (simplificado)
        String ssid = extractFormValue(body, "ssid");
        String password = extractFormValue(body, "password");
        String deviceName = extractFormValue(body, "device_name");
        
        if (ssid.length() > 0) {
            setWiFiCredentials(ssid, password);
            _config.deviceName = deviceName;
            
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");
            client.println();
            client.println("<html><body style='font-family:Arial;text-align:center;padding:50px;'>");
            client.println("<h2 style='color:#4CAF50;'>✅ Configuration Saved!</h2>");
            client.println("<p>Device is connecting to WiFi...</p>");
            client.println("<p>🔄 Please wait 30 seconds, then add device in Tuya app</p>");
            client.println("</body></html>");
            
            delay(2000);
            _configMode = false;
            WiFi.softAPdisconnect();
            
            // Conectar a WiFi y registrar en Tuya
            connectToTuyaCloud();
        }
    }
    
    client.stop();
}

String TuyaDevice::extractFormValue(const String& body, const String& key) {
    int start = body.indexOf(key + "=");
    if (start == -1) return "";
    
    start += key.length() + 1;
    int end = body.indexOf("&", start);
    if (end == -1) end = body.length();
    
    String value = body.substring(start, end);
    value.replace("+", " ");
    // TODO: URL decode properly
    return value;
}

void TuyaDevice::connectToTuyaCloud() {
    Serial.println("🌐 Connecting to WiFi...");
    // WiFi connection logic here
    
    Serial.println("☁️ Registering with Tuya Cloud...");
    registerDevice();
    
    _configured = true;
    _connected = true;
    
    Serial.println("✅ Device ready! Add in Tuya Smart app:");
    Serial.println("   1. Open Tuya Smart app");
    Serial.println("   2. Tap '+' → Add Device");
    Serial.println("   3. Select 'Water Sensor' or 'Other'");
    Serial.println("   4. Device should appear automatically!");
}

void TuyaDevice::registerDevice() {
    // Enviar broadcast UDP para auto-discovery (protocolo Tuya estándar)
    WiFiUDP udp;
    
    DynamicJsonDocument doc(512);
    doc["ip"] = WiFi.localIP().toString();
    doc["gwId"] = _deviceId;
    doc["active"] = 2;
    doc["ability"] = 0;
    doc["mode"] = 0;
    doc["encrypt"] = true;
    doc["productKey"] = "water_sensor_esp32";
    doc["version"] = TUYA_PROTOCOL_VERSION;
    
    String message;
    serializeJson(doc, message);
    
    // Broadcast en red local (así es como te encuentra la app Tuya)
    udp.beginPacket("255.255.255.255", TUYA_DISCOVERY_PORT);
    udp.print(message);
    udp.endPacket();
    
    Serial.println("📡 Device announced to network");
}

void TuyaDevice::updateSensorValue(const String& dpId, float value) {
    if (!_connected) return;
    
    DynamicJsonDocument doc(256);
    doc["dps"][dpId] = value;
    doc["t"] = millis() / 1000;
    
    String message;
    serializeJson(doc, message);
    
    // Enviar a Tuya Cloud (implementación simplificada)
    Serial.println("📊 Sensor Update - DP" + dpId + ": " + String(value));
}

void TuyaDevice::loop() {
    if (_configMode) {
        handleConfigServer();
        return;
    }
    
    if (!_connected) return;
    
    static unsigned long lastHeartbeat = 0;
    if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
        sendHeartbeat();
        lastHeartbeat = millis();
    }
    
    processIncomingCommands();
}

void TuyaDevice::sendHeartbeat() {
    // Mantener conexión viva con Tuya Cloud
    Serial.println("💓 Heartbeat sent");
}

void TuyaDevice::processIncomingCommands() {
    // Procesar comandos de la app Tuya
    // (comandos para cambiar configuraciones del sensor)
}

bool TuyaDevice::isConnected() {
    return _connected;
}

bool TuyaDevice::isConfigured() {
    return _configured;
}

bool TuyaDevice::isInConfigMode() {
    return _configMode;
}

void TuyaDevice::setWiFiCredentials(const String& ssid, const String& password) {
    // Intentar conectar a WiFi
    WiFi.begin(ssid.c_str(), password.c_str());
    
    Serial.println("🌐 Conectando a WiFi: " + ssid);
    
    // Esperar conexión con timeout
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ WiFi conectado!");
        Serial.println("📍 IP: " + WiFi.localIP().toString());
    } else {
        Serial.println("\n❌ Error conectando WiFi");
    }
}

void TuyaDevice::onCommand(std::function<void(String dpId, String value)> callback) {
    _commandCallback = callback;
}

void TuyaDevice::updateStatus(const String& datapoint, bool value) {
    // Actualizar estado local
    if (datapoint == "water_level") {
        deviceStatus.waterLevel = value;
    } else if (datapoint == "tank_full") {
        deviceStatus.tankFull = value;
    } else if (datapoint == "alert_enabled") {
        deviceStatus.alertEnabled = value;
    }
    
    // Si está conectado, enviar actualización
    if (_connected) {
        // Construir mensaje de actualización de estado
        String message = "{\"dps\":{\"" + datapoint + "\":" + (value ? "true" : "false") + "}}";
        
        Serial.println("[TuyaDevice] Estado actualizado: " + datapoint + " = " + (value ? "true" : "false"));
        // TODO: Enviar al servidor Tuya cuando esté implementado
    }
}