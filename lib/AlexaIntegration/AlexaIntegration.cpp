#include "AlexaIntegration.h"

AlexaIntegration::AlexaIntegration(SensorManager* sensor) {
    sensorManager = sensor;
    enabled = false;
}

void AlexaIntegration::init(const String& skillId) {
    this->skillId = skillId;
}

String AlexaIntegration::processAlexaRequest(const String& requestBody) {
    if (!enabled) {
        return createAlexaResponse("El servicio no está habilitado");
    }
    
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, requestBody);
    
    String intentName = doc["request"]["intent"]["name"];
    
    if (intentName == "WaterLevelIntent") {
        return handleWaterLevelIntent();
    } else if (intentName == "DeviceStatusIntent") {
        return handleDeviceStatusIntent();
    } else if (intentName == "TankCapacityIntent") {
        return handleTankCapacityIntent();
    }
    
    return createAlexaResponse("No entendí tu consulta sobre el sensor de agua");
}

String AlexaIntegration::handleWaterLevelIntent() {
    String sensorJson = sensorManager->getSensorJson("WaterLevel");
    DynamicJsonDocument doc(512);
    deserializeJson(doc, sensorJson);
    
    float litros = doc["litros"];
    
    String response;
    if (litros > 100) {
        response = "El tanque está bien lleno con " + String(litros, 0) + " litros de agua";
    } else if (litros > 50) {
        response = "El nivel de agua es bueno, tienes " + String(litros, 0) + " litros";
    } else {
        response = "Alerta, el agua está baja, solo " + String(litros, 0) + " litros";
    }
    
    return createAlexaResponse(response);
}

String AlexaIntegration::handleDeviceStatusIntent() {
    return createAlexaResponse("El sensor de agua está funcionando correctamente");
}

String AlexaIntegration::handleTankCapacityIntent() {
    String sensorJson = sensorManager->getSensorJson("WaterLevel");
    DynamicJsonDocument doc(512);
    deserializeJson(doc, sensorJson);
    
    float litros = doc["litros"];
    float porcentaje = (litros / 200.0) * 100;
    
    String response = "El tanque está al " + String(porcentaje, 0) + " por ciento de su capacidad";
    return createAlexaResponse(response);
}

String AlexaIntegration::createAlexaResponse(const String& outputSpeech, bool shouldEndSession) {
    DynamicJsonDocument doc(1024);
    
    doc["version"] = "1.0";
    
    JsonObject response = doc.createNestedObject("response");
    JsonObject outputSpeechObj = response.createNestedObject("outputSpeech");
    outputSpeechObj["type"] = "PlainText";
    outputSpeechObj["text"] = outputSpeech;
    
    response["shouldEndSession"] = shouldEndSession;
    
    String jsonString;
    serializeJson(doc, jsonString);
    return jsonString;
}