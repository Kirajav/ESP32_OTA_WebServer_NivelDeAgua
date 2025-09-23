#include "ConfigManager.h"

const double DEFAULT_ALTURA_MAX_AGUA_TINACO = 115.0;
const double DEFAULT_CAPACIDAD_LITROS_TINACO = 1200.0;
const double DEFAULT_DISTANCIA_MINIMA_SENSOR = 19.0;
const char* DEFAULT_HOSTNAME = "ESP32_Sensor_1";
const bool DEFAULT_CHECK_UPDATES = true;
const uint8_t DEFAULT_TIPO_CONTENEDOR = 0; // 0=Tinaco, 1=Cisterna, 2=Contenedor

const char* TIPOS_CONTENEDOR[] = {"Tinaco", "Cisterna", "Contenedor"};

ConfigManager::ConfigManager() {
}

void ConfigManager::begin() {
    if (!SPIFFS.begin(true)) {
        Serial.println(F("Error montando SPIFFS"));
    }
    loadConfig();
}

void ConfigManager::saveConfig() {
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
        Serial.println(F("Error al abrir config.json para escritura"));
        return;
    }
    
    StaticJsonDocument<1024> doc;
    doc["altura_max"] = _config.altura_max;
    doc["capacidad"] = _config.capacidad;
    doc["distancia_min"] = _config.distancia_min;
    doc["display_on"] = _config.display_on;
    doc["tipo_contenedor"] = _config.tipo_contenedor;
    doc["tipo_nombre"] = TIPOS_CONTENEDOR[_config.tipo_contenedor];
    doc["hostname"] = _config.hostname;
    doc["check_updates"] = _config.check_updates;
    
    if (serializeJson(doc, configFile)) {
        Serial.println(F("Configuración guardada exitosamente"));
    } else {
        Serial.println(F("Error al escribir la configuración"));
    }
    configFile.close();
}

bool ConfigManager::loadConfig() {
    File configFile = SPIFFS.open("/config.json", "r");
    if (!configFile) {
        Serial.println(F("No se encontró archivo de configuración, usando valores por defecto"));
        // Inicializar con valores por defecto si no hay archivo
        _config.altura_max = DEFAULT_ALTURA_MAX_AGUA_TINACO;
        _config.capacidad = DEFAULT_CAPACIDAD_LITROS_TINACO;
        _config.distancia_min = DEFAULT_DISTANCIA_MINIMA_SENSOR;
        _config.display_on = true; // Por defecto encendido
        _config.tipo_contenedor = DEFAULT_TIPO_CONTENEDOR;
        _config.hostname = DEFAULT_HOSTNAME;
        _config.check_updates = DEFAULT_CHECK_UPDATES;
        
        Serial.println("Configuración por defecto:");
        Serial.println("Altura máxima: " + String(_config.altura_max));
        Serial.println("Capacidad: " + String(_config.capacidad));
        Serial.println("Distancia mínima: " + String(_config.distancia_min));
        
        saveConfig(); // Guardar la configuración por defecto
        return false;
    }

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, configFile);
    configFile.close();

    if (error) {
        Serial.println(F("Error al parsear el archivo de configuración"));
        return false;
    }

    _config.altura_max = doc["altura_max"] | DEFAULT_ALTURA_MAX_AGUA_TINACO;
    _config.capacidad = doc["capacidad"] | DEFAULT_CAPACIDAD_LITROS_TINACO;
    _config.distancia_min = doc["distancia_min"] | DEFAULT_DISTANCIA_MINIMA_SENSOR;
    _config.display_on = doc["display_on"] | true;
    _config.tipo_contenedor = doc["tipo_contenedor"] | DEFAULT_TIPO_CONTENEDOR;
    _config.hostname = doc["hostname"] | DEFAULT_HOSTNAME;
    _config.check_updates = doc["check_updates"] | DEFAULT_CHECK_UPDATES;

    Serial.println("Configuración cargada desde archivo:");
    Serial.println("Altura máxima: " + String(_config.altura_max));
    Serial.println("Capacidad: " + String(_config.capacidad));
    Serial.println("Distancia mínima: " + String(_config.distancia_min));

    return true;
}

Config& ConfigManager::getConfig() {
    return _config;
}

void ConfigManager::setAlturaMax(double altura_max) {
    _config.altura_max = altura_max;
}

double ConfigManager::getAlturaMax() {
    return _config.altura_max;
}

void ConfigManager::setCapacidad(double capacidad) {
    _config.capacidad = capacidad;
}

double ConfigManager::getCapacidad() {
    return _config.capacidad;
}

void ConfigManager::setDistanciaMin(double distancia_min) {
    _config.distancia_min = distancia_min;
}

double ConfigManager::getDistanciaMin() {
    return _config.distancia_min;
}

void ConfigManager::setHostname(const String& hostname) {
    _config.hostname = hostname;
}

String ConfigManager::getHostname() {
    return _config.hostname;
}

void ConfigManager::setCheckUpdates(bool check_updates) {
    _config.check_updates = check_updates;
}

bool ConfigManager::getCheckUpdates() {
    return _config.check_updates;
}

void ConfigManager::setDisplayOn(bool display_on) {
    _config.display_on = display_on;
}

bool ConfigManager::isDisplayOn() {
    return _config.display_on;
}

void ConfigManager::setTipoContenedor(uint8_t tipo_contenedor) {
    _config.tipo_contenedor = tipo_contenedor;
}

uint8_t ConfigManager::getTipoContenedor() {
    return _config.tipo_contenedor;
}

const char* ConfigManager::getTipoContenedorStr() {
    return TIPOS_CONTENEDOR[_config.tipo_contenedor];
}
