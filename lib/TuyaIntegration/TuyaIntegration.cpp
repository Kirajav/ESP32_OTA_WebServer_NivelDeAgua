#include "TuyaIntegration.h"

TuyaIntegration::TuyaIntegration(SensorManager* sensor) {
    sensorManager = sensor;
    enabled = false;
    lastReport = 0;
}

void TuyaIntegration::init(const String& productKey, const String& deviceSecret, const String& deviceId) {
    this->productKey = productKey;
    this->deviceSecret = deviceSecret;
    this->deviceId = deviceId;
}

void TuyaIntegration::reportWaterLevel(float level, float distance) {
    if (!enabled) return;
    
    String payload = createTuyaPayload(level, distance);
    reportToTuya(payload);
}

void TuyaIntegration::reportDeviceStatus(const String& status) {
    if (!enabled) return;
    
    DynamicJsonDocument doc(512);
    doc["deviceStatus"] = status;
    doc["timestamp"] = millis();
    doc["online"] = (status == "Conectado");
    
    String payload;
    serializeJson(doc, payload);
    reportToTuya(payload);
}

void TuyaIntegration::updateTuyaCloud() {
    if (!enabled || millis() - lastReport < 60000) return; // Update cada 60s
    
    String sensorJson = sensorManager->getSensorJson("WaterLevel");
    DynamicJsonDocument doc(512);
    deserializeJson(doc, sensorJson);
    
    float litros = doc["litros"];
    float distancia = doc["distancia_cm"];
    
    reportWaterLevel(litros, distancia);
    lastReport = millis();
}

String TuyaIntegration::createTuyaPayload(float waterLevel, float distance) {
    DynamicJsonDocument doc(1024);
    
    // Tuya data points format
    JsonObject dataPoints = doc.createNestedObject("dataPoints");
    
    // DP 1: Water level in liters
    dataPoints["1"] = String(waterLevel, 1);
    
    // DP 2: Distance in cm  
    dataPoints["2"] = String(distance, 1);
    
    // DP 3: Percentage
    float percentage = (waterLevel / 200.0) * 100;
    dataPoints["3"] = String(percentage, 0);
    
    // DP 4: Status
    String status = (waterLevel > 20) ? "normal" : "low";
    dataPoints["4"] = status;
    
    doc["deviceId"] = deviceId;
    doc["timestamp"] = millis();
    
    String payload;
    serializeJson(doc, payload);
    return payload;
}

void TuyaIntegration::reportToTuya(const String& dataPoints) {
    if (!WiFi.isConnected()) return;
    
    HTTPClient http;
    http.begin("https://openapi.tuyacn.com/v1.0/devices/" + deviceId + "/properties/report");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("client_id", productKey);
    // En producción agregar firma HMAC-SHA256
    
    int httpResponseCode = http.POST(dataPoints);
    
    if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Tuya response: " + response);
    } else {
        Serial.println("Error en request Tuya: " + String(httpResponseCode));
    }
    
    http.end();
}