#include "SensorConfig.h"

// Default values
const double SensorConfig::DEFAULT_MAX_HEIGHT = 200.0;  // 200 cm
const double SensorConfig::DEFAULT_CAPACITY = 1000.0;   // 1000 liters
const double SensorConfig::DEFAULT_MIN_DISTANCE = 5.0;  // 5 cm

// Timing and geo defaults
const String SensorConfig::DEFAULT_TIMEZONE = "America/Mexico_City";
const String SensorConfig::DEFAULT_COUNTRY_CODE = "MX";
const String SensorConfig::DEFAULT_CITY = "Mexico City";

SensorConfig::SensorConfig() {
    setDefaults();
}

bool SensorConfig::loadFromJson(const JsonObject& json) {
    if (json.containsKey("altura_max") || json.containsKey("max_height")) {
        // Support both old and new key names for backward compatibility
        settings.max_height = json.containsKey("max_height") ? 
            json["max_height"].as<double>() : json["altura_max"].as<double>();
    }
    
    if (json.containsKey("capacidad") || json.containsKey("capacity")) {
        settings.capacity = json.containsKey("capacity") ? 
            json["capacity"].as<double>() : json["capacidad"].as<double>();
    }
    
    if (json.containsKey("distancia_min") || json.containsKey("min_distance")) {
        settings.min_distance = json.containsKey("min_distance") ? 
            json["min_distance"].as<double>() : json["distancia_min"].as<double>();
    }
    
    if (json.containsKey("tipo_contenedor") || json.containsKey("container_type")) {
        uint8_t type = json.containsKey("container_type") ? 
            json["container_type"].as<uint8_t>() : json["tipo_contenedor"].as<uint8_t>();
        setContainerType(type);
    }
    
    // Cargar configuraciones de tiempo
    if (json.containsKey("normal_interval")) {
        setNormalInterval(json["normal_interval"].as<uint16_t>());
    }
    if (json.containsKey("filling_interval")) {
        setFillingInterval(json["filling_interval"].as<uint16_t>());
    }
    if (json.containsKey("filling_threshold")) {
        setFillingThreshold(json["filling_threshold"].as<uint8_t>());
    }
    
    // Cargar configuraciones geográficas
    if (json.containsKey("timezone")) {
        setTimezone(json["timezone"].as<String>());
    }
    if (json.containsKey("country_code")) {
        setCountryCode(json["country_code"].as<String>());
    }
    if (json.containsKey("city")) {
        setCity(json["city"].as<String>());
    }
    if (json.containsKey("auto_geo_location")) {
        setAutoGeoLocation(json["auto_geo_location"].as<bool>());
    }
    if (json.containsKey("show_datetime")) {
        setShowDateTime(json["show_datetime"].as<bool>());
    }
    
    return isValid();
}

void SensorConfig::saveToJson(JsonObject& json) const {
    // Use new English key names
    json["max_height"] = settings.max_height;
    json["capacity"] = settings.capacity;
    json["min_distance"] = settings.min_distance;
    json["container_type"] = static_cast<uint8_t>(settings.container_type);
    
    // Keep old Spanish keys for backward compatibility
    json["altura_max"] = settings.max_height;
    json["capacidad"] = settings.capacity;
    json["distancia_min"] = settings.min_distance;
    json["tipo_contenedor"] = static_cast<uint8_t>(settings.container_type);
    
    // Guardar configuraciones de tiempo
    json["normal_interval"] = settings.normal_interval;
    json["filling_interval"] = settings.filling_interval;
    json["filling_threshold"] = settings.filling_threshold;
    
    // Guardar configuraciones geográficas
    json["timezone"] = settings.timezone;
    json["country_code"] = settings.country_code;
    json["city"] = settings.city;
    json["auto_geo_location"] = settings.auto_geo_location;
    json["show_datetime"] = settings.show_datetime;
}

void SensorConfig::setDefaults() {
    settings.max_height = DEFAULT_MAX_HEIGHT;
    settings.capacity = DEFAULT_CAPACITY;
    settings.min_distance = DEFAULT_MIN_DISTANCE;
    settings.container_type = DEFAULT_CONTAINER_TYPE;
    
    // Valores por defecto de tiempo
    settings.normal_interval = DEFAULT_NORMAL_INTERVAL;
    settings.filling_interval = DEFAULT_FILLING_INTERVAL;
    settings.filling_threshold = DEFAULT_FILLING_THRESHOLD;
    
    // Valores por defecto geográficos
    settings.timezone = DEFAULT_TIMEZONE;
    settings.country_code = DEFAULT_COUNTRY_CODE;
    settings.city = DEFAULT_CITY;
    settings.auto_geo_location = DEFAULT_AUTO_GEO;
    settings.show_datetime = DEFAULT_SHOW_DATETIME;
}

void SensorConfig::setMaxHeight(double height) {
    if (height > 0 && height <= 1000) { // Max 10 meters
        settings.max_height = height;
    }
}

void SensorConfig::setCapacity(double capacity) {
    if (capacity > 0 && capacity <= 100000) { // Max 100,000 liters
        settings.capacity = capacity;
    }
}

void SensorConfig::setMinDistance(double distance) {
    if (distance >= 0 && distance <= 100) { // Max 1 meter
        settings.min_distance = distance;
    }
}

void SensorConfig::setContainerType(ContainerType type) {
    if (type >= CONTAINER_TANK && type <= CONTAINER_GENERIC) {
        settings.container_type = type;
    }
}

void SensorConfig::setContainerType(uint8_t type) {
    setContainerType(static_cast<ContainerType>(type));
}

const char* SensorConfig::getContainerTypeString() const {
    switch (settings.container_type) {
        case CONTAINER_TANK:
            return "Tinaco";
        case CONTAINER_CISTERN:
            return "Cisterna";
        case CONTAINER_GENERIC:
            return "Contenedor";
        default:
            return "Desconocido";
    }
}

// Métodos para configuraciones de tiempo
void SensorConfig::setNormalInterval(uint16_t seconds) {
    if (seconds >= 5 && seconds <= 300) { // Entre 5 segundos y 5 minutos
        settings.normal_interval = seconds;
    }
}

void SensorConfig::setFillingInterval(uint16_t seconds) {
    if (seconds >= 1 && seconds <= 30) { // Entre 1 y 30 segundos
        settings.filling_interval = seconds;
    }
}

void SensorConfig::setFillingThreshold(uint8_t threshold) {
    if (threshold >= 2 && threshold <= 10) { // Entre 2 y 10 lecturas
        settings.filling_threshold = threshold;
    }
}

// Métodos para configuraciones geográficas
void SensorConfig::setTimezone(const String& timezone) {
    if (timezone.length() > 0 && timezone.length() <= 64) {
        settings.timezone = timezone;
    }
}

void SensorConfig::setCountryCode(const String& code) {
    if (code.length() == 2) { // Código ISO de 2 letras
        settings.country_code = code;
        settings.country_code.toUpperCase();
    }
}

void SensorConfig::setCity(const String& city) {
    if (city.length() > 0 && city.length() <= 64) {
        settings.city = city;
    }
}

void SensorConfig::setAutoGeoLocation(bool enabled) {
    settings.auto_geo_location = enabled;
}

void SensorConfig::setShowDateTime(bool show) {
    settings.show_datetime = show;
}

bool SensorConfig::isValid() const {
    return settings.max_height > 0 &&
           settings.max_height <= 1000 &&
           settings.capacity > 0 &&
           settings.capacity <= 100000 &&
           settings.min_distance >= 0 &&
           settings.min_distance <= 100 &&
           settings.min_distance < settings.max_height &&
           settings.container_type >= CONTAINER_TANK &&
           settings.container_type <= CONTAINER_GENERIC &&
           settings.normal_interval >= 5 &&
           settings.normal_interval <= 300 &&
           settings.filling_interval >= 1 &&
           settings.filling_interval <= 30 &&
           settings.filling_threshold >= 2 &&
           settings.filling_threshold <= 10;
}