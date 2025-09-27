#include "GoogleHomeIntegration.h"

GoogleHomeIntegration::GoogleHomeIntegration(SensorManager* sensor) {
    sensorManager = sensor;
    enabled = false;
    lastUpdate = 0;
}

void GoogleHomeIntegration::init(const String& projectId, const String& deviceId) {
    this->projectId = projectId;
    this->deviceId = deviceId;
}

String GoogleHomeIntegration::processGoogleRequest(const String& requestBody) {
    if (!enabled) {
        return createGoogleResponse("error", "Servicio no habilitado");
    }
    
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, requestBody);
    
    String intentName = doc["queryResult"]["intent"]["displayName"];
    
    if (intentName == "water.level.check") {
        return getWaterLevelResponse();
    } else if (intentName == "device.status") {
        return getDeviceStatusResponse();
    } else if (intentName == "tank.capacity") {
        return getTankCapacityResponse();
    }
    
    return createGoogleResponse("default", "No entendí tu consulta sobre el sensor de agua");
}

String GoogleHomeIntegration::getWaterLevelResponse() {
    String sensorJson = sensorManager->getSensorJson("WaterLevel");
    DynamicJsonDocument doc(512);
    deserializeJson(doc, sensorJson);
    
    float litros = doc["litros"];
    float distancia = doc["distancia_cm"];
    
    String response;
    if (litros > 100) {
        response = "El tinaco está bien lleno con " + String(litros, 0) + " litros de agua";
    } else if (litros > 50) {
        response = "El nivel de agua es bueno, tienes " + String(litros, 0) + " litros";
    } else if (litros > 20) {
        response = "El agua está un poco baja, solo " + String(litros, 0) + " litros";
    } else {
        response = "¡Alerta! El agua está muy baja, solo " + String(litros, 0) + " litros";
    }
    
    return createGoogleResponse("water.level.check", response);
}

String GoogleHomeIntegration::getDeviceStatusResponse() {
    String response = "El sensor de agua está funcionando correctamente y conectado";
    return createGoogleResponse("device.status", response);
}

String GoogleHomeIntegration::getTankCapacityResponse() {
    String sensorJson = sensorManager->getSensorJson("WaterLevel");
    DynamicJsonDocument doc(512);
    deserializeJson(doc, sensorJson);
    
    float litros = doc["litros"];
    float porcentaje = (litros / 200.0) * 100; // Asumiendo tanque de 200L
    
    String response = "El tanque está al " + String(porcentaje, 0) + " por ciento de su capacidad";
    return createGoogleResponse("tank.capacity", response);
}

String GoogleHomeIntegration::createGoogleResponse(const String& intentName, const String& responseText) {
    DynamicJsonDocument doc(1024);
    
    doc["fulfillmentText"] = responseText;
    
    JsonObject payload = doc.createNestedObject("payload");
    JsonObject google = payload.createNestedObject("google");
    google["expectUserResponse"] = false;
    
    JsonObject richResponse = google.createNestedObject("richResponse");
    JsonArray items = richResponse.createNestedArray("items");
    JsonObject simpleResponse = items.createNestedObject();
    simpleResponse.createNestedObject("simpleResponse")["textToSpeech"] = responseText;
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}

void GoogleHomeIntegration::updateDeviceState() {
    if (!enabled || millis() - lastUpdate < 30000) return; // Update cada 30s
    
    reportDeviceState();
    lastUpdate = millis();
}

void GoogleHomeIntegration::reportWaterLevel(float level, float distance) {
    if (!enabled) return;
    
    // Implementar reporte a Google Home Graph API si es necesario
    Serial.println("Reportando a Google Home: " + String(level) + "L, " + String(distance) + "cm");
}

void GoogleHomeIntegration::reportDeviceState() {
    // Implementar Home Graph API para reportar estado del dispositivo
    Serial.println("Reportando estado del dispositivo a Google Home");
}