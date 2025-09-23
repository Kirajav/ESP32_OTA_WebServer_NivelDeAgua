#include "ConfigManager.h"

const double DEFAULT_ALTURA_MAX_AGUA_TINACO = 115.0;
const double DEFAULT_CAPACIDAD_LITROS_TINACO = 1200.0;
const double DEFAULT_DISTANCIA_MINIMA_SENSOR = 19.0;
const char* DEFAULT_HOSTNAME = "ESP32_Sensor";
const bool DEFAULT_CHECK_UPDATES = true;
const uint8_t DEFAULT_TIPO_CONTENEDOR = 0; // 0=Tinaco, 1=Cisterna, 2=Contenedor
const char* DEFAULT_AP_SSID = "ESP32_Sensor";
const char* DEFAULT_AP_PASSWORD = "12345678";

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
    doc["ap_ssid"] = _config.ap_ssid;
    doc["ap_password"] = _config.ap_password;
    
    if (serializeJson(doc, configFile)) {
        Serial.println(F("Configuración guardada exitosamente"));
    } else {
        Serial.println(F("Error al escribir la configuración"));
    }
    configFile.close();
}

bool ConfigManager::loadConfig() {
    if (!SPIFFS.begin(true)) {
        Serial.println("Error al inicializar SPIFFS");
        setDefaultConfig();
        return false;
    }

    if (!SPIFFS.exists(CONFIG_FILE)) {
        Serial.println("Archivo de configuración no existe");
        setDefaultConfig();
        saveConfig(); // Guardar configuración por defecto
        return false;
    }

    File file = SPIFFS.open(CONFIG_FILE, "r");
    if (!file) {
        Serial.println("Error al abrir archivo de configuración");
        setDefaultConfig();
        saveConfig(); // Guardar configuración por defecto
        return false;
    }

    String json = file.readString();
    file.close();
    
    Serial.print("JSON leído del archivo: ");
    Serial.println(json);

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        Serial.print("Error al parsear JSON: ");
        Serial.println(error.c_str());
        setDefaultConfig();
        saveConfig(); // Guardar configuración por defecto
        return false;
    }

    // Cargar valores con validación
    _config.altura_max = doc["altura_max"] | DEFAULT_ALTURA_MAX_AGUA_TINACO;
    _config.capacidad = doc["capacidad"] | DEFAULT_CAPACIDAD_LITROS_TINACO;
    _config.distancia_min = doc["distancia_min"] | DEFAULT_DISTANCIA_MINIMA_SENSOR;
    
    // Validar que los valores no sean 0
    if (_config.altura_max <= 0) _config.altura_max = DEFAULT_ALTURA_MAX_AGUA_TINACO;
    if (_config.capacidad <= 0) _config.capacidad = DEFAULT_CAPACIDAD_LITROS_TINACO;
    if (_config.distancia_min <= 0) _config.distancia_min = DEFAULT_DISTANCIA_MINIMA_SENSOR;

    // Cargar configuraciones del host
    String loadedHostname = doc["hostname"] | DEFAULT_HOSTNAME;
    if (loadedHostname.length() > 0) {
        _config.hostname = loadedHostname;
    } else {
        _config.hostname = DEFAULT_HOSTNAME;
    }
    
    _config.display_on = doc["display_on"] | true;
    _config.check_updates = doc["check_updates"] | DEFAULT_CHECK_UPDATES;
    _config.tipo_contenedor = doc["tipo_contenedor"] | DEFAULT_TIPO_CONTENEDOR;

    // Cargar configuraciones del SoftAP
    String loadedApSSID = doc["ap_ssid"] | DEFAULT_AP_SSID;
    if (loadedApSSID.length() > 0) {
        _config.ap_ssid = loadedApSSID;
    } else {
        _config.ap_ssid = DEFAULT_AP_SSID;
    }
    
    String loadedApPassword = doc["ap_password"] | DEFAULT_AP_PASSWORD;
    if (loadedApPassword.length() >= 8) {  // Validar longitud mínima WiFi
        _config.ap_password = loadedApPassword;
    } else {
        _config.ap_password = DEFAULT_AP_PASSWORD;
    }

    Serial.println("=== CONFIGURACIÓN CARGADA ===");
    Serial.printf("Altura máxima: %.2f\n", _config.altura_max);
    Serial.printf("Capacidad: %.2f\n", _config.capacidad);
    Serial.printf("Distancia mínima: %.2f\n", _config.distancia_min);
    Serial.printf("Hostname: %s\n", _config.hostname.c_str());
    Serial.printf("Display: %s\n", _config.display_on ? "ON" : "OFF");
    Serial.printf("Tipo contenedor: %s\n", TIPOS_CONTENEDOR[_config.tipo_contenedor]);
    Serial.printf("SoftAP SSID: %s\n", _config.ap_ssid.c_str());
    Serial.printf("SoftAP Password: %s\n", _config.ap_password.c_str());
    Serial.println("=========================");
    
    // Solo guardar configuración si se cargaron valores por defecto
    if (!SPIFFS.exists("/config.json")) {
        saveConfig();
    }
    
    return true;
}

void ConfigManager::setDefaultConfig() {
    _config.altura_max = DEFAULT_ALTURA_MAX_AGUA_TINACO;
    _config.capacidad = DEFAULT_CAPACIDAD_LITROS_TINACO;
    _config.distancia_min = DEFAULT_DISTANCIA_MINIMA_SENSOR;
    _config.hostname = DEFAULT_HOSTNAME;
    _config.display_on = true;
    _config.check_updates = DEFAULT_CHECK_UPDATES;
    _config.tipo_contenedor = DEFAULT_TIPO_CONTENEDOR;
    _config.ap_ssid = DEFAULT_AP_SSID;
    _config.ap_password = DEFAULT_AP_PASSWORD;
    
    Serial.println("=== CONFIGURACIÓN POR DEFECTO ESTABLECIDA ===");
    Serial.printf("Altura máxima: %.2f\n", _config.altura_max);
    Serial.printf("Capacidad: %.2f\n", _config.capacidad);
    Serial.printf("Distancia mínima: %.2f\n", _config.distancia_min);
    Serial.printf("Hostname: %s\n", _config.hostname.c_str());
    Serial.printf("Display: %s\n", _config.display_on ? "ON" : "OFF");
    Serial.printf("Tipo contenedor: %s\n", TIPOS_CONTENEDOR[_config.tipo_contenedor]);
    Serial.println("========================================");
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

// Métodos para SoftAP SSID
void ConfigManager::setApSSID(const String& ap_ssid) {
    _config.ap_ssid = ap_ssid;
}

String ConfigManager::getApSSID() {
    return _config.ap_ssid;
}

// Métodos para SoftAP Password
void ConfigManager::setApPassword(const String& ap_password) {
    if (ap_password.length() >= 8) {  // Validar longitud mínima WiFi
        _config.ap_password = ap_password;
    }
}

String ConfigManager::getApPassword() {
    return _config.ap_password;
}
