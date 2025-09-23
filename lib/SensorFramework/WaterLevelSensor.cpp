#include "WaterLevelSensor.h"

WaterLevelSensor::WaterLevelSensor(uint8_t triggerPin, uint8_t echoPin, ConfigManager* configManager)
    : m_distanceSensor(triggerPin, echoPin), m_configManager(configManager), m_distanciaCm(-1.0), m_mensajeError("No_init") {}

void WaterLevelSensor::begin() {
    // La librería HCSR04 no requiere un método begin(),
    // la inicialización se hace en el constructor.
}

void WaterLevelSensor::update() {
    float medida = m_distanceSensor.measureDistanceCm();
    // Un rango de 4m es el máximo para este sensor
    if (medida >= 0 && medida <= 400) { 
        m_distanciaCm = medida;
        m_mensajeError = "OK";
    } else {
        m_distanciaCm = -1.0; // Valor inválido
        m_mensajeError = "Error de lectura";
    }
}

String WaterLevelSensor::getName() const {
    return "WaterLevel";
}

String WaterLevelSensor::getJson() const {
    StaticJsonDocument<256> doc;
    doc["nombre"] = getName();
    doc["estado"] = m_mensajeError;
    if (m_distanciaCm >= 0) {
        doc["distancia_cm"] = serialized(String(m_distanciaCm, 1));
        doc["litros"] = serialized(calculateLitros());
    } else {
        doc["distancia_cm"] = "Error";
        doc["litros"] = "Error";
    }
    
    String output;
    serializeJson(doc, output);
    return output;
}

String WaterLevelSensor::calculateLitros() const {
    if (m_distanciaCm < 0) { // Error de lectura del sensor
        return "Error";
    }

    // Debug solo cada 10 segundos para reducir spam
    static unsigned long lastDebugTime = 0;
    unsigned long currentTime = millis();
    bool shouldDebug = (currentTime - lastDebugTime) > 10000;
    
    if (shouldDebug) {
        Serial.println("=== CÁLCULO DE LITROS ===");
        Serial.print("Distancia: ");
        Serial.print(m_distanciaCm);
        Serial.print("cm, Litros: ");
        lastDebugTime = currentTime;
    }

    // Si la distancia es menor o igual a la mínima del sensor, se considera lleno
    if (m_distanciaCm <= m_configManager->getDistanciaMin()) {
        if (shouldDebug) {
            Serial.println(String(m_configManager->getCapacidad(), 1) + "L (LLENO)");
        }
        return String(m_configManager->getCapacidad(), 1);
    }

    double alturaActualAgua = m_configManager->getAlturaMax() - m_distanciaCm;

    if (alturaActualAgua < 0) {
        alturaActualAgua = 0;
    }

    double alturaTotalAgua = m_configManager->getAlturaMax();
    
    if (alturaTotalAgua <= 0) {
        if (shouldDebug) {
            Serial.println("0.0L (ERROR: Altura total <= 0)");
        }
        return "0.0";
    }
    
    double porcentajeLlenado = (alturaActualAgua / alturaTotalAgua) * 100.0;
    
    if (porcentajeLlenado > 100.0) porcentajeLlenado = 100.0;
    if (porcentajeLlenado < 0.0) porcentajeLlenado = 0.0;

    double litros = (m_configManager->getCapacidad() * porcentajeLlenado) / 100.0;
    
    if (shouldDebug) {
        Serial.println(String(litros, 1) + "L (" + String(porcentajeLlenado, 1) + "%)");
    }
    
    return String(litros, 1);
}