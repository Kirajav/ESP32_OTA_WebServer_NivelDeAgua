#ifndef SENSOR_CONFIG_H
#define SENSOR_CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>

enum ContainerType {
    CONTAINER_TANK = 0,     // Tinaco
    CONTAINER_CISTERN = 1,  // Cisterna
    CONTAINER_GENERIC = 2   // Contenedor genérico
};

struct SensorSettings {
    double max_height;      // altura_max -> max_height (cm)
    double capacity;        // capacidad -> capacity (liters)
    double min_distance;    // distancia_min -> min_distance (cm)
    ContainerType container_type; // tipo_contenedor -> container_type
    
    // Configuraciones de tiempo de sensado
    uint16_t normal_interval;     // Intervalo normal en segundos (15-60s recomendado)
    uint16_t filling_interval;    // Intervalo al llenar en segundos (3-5s recomendado)
    uint8_t filling_threshold;    // Número de lecturas incrementales para detectar llenado (3 por defecto)
    
    // Configuración geográfica para NTP
    String timezone;             // Zona horaria (ej: "America/Mexico_City")
    String country_code;         // Código país (ej: "MX")
    String city;                 // Ciudad (ej: "Mexico City")
    bool auto_geo_location;      // Detectar ubicación automáticamente
    bool show_datetime;          // Mostrar fecha/hora en pantallas
};

class SensorConfig {
public:
    SensorConfig();
    
    // Core functionality
    bool loadFromJson(const JsonObject& json);
    void saveToJson(JsonObject& json) const;
    void setDefaults();
    
    // Height management
    void setMaxHeight(double height);
    double getMaxHeight() const { return settings.max_height; }
    
    // Capacity management
    void setCapacity(double capacity);
    double getCapacity() const { return settings.capacity; }
    
    // Distance management
    void setMinDistance(double distance);
    double getMinDistance() const { return settings.min_distance; }
    
    // Container type management
    void setContainerType(ContainerType type);
    void setContainerType(uint8_t type);
    ContainerType getContainerType() const { return settings.container_type; }
    uint8_t getContainerTypeAsInt() const { return static_cast<uint8_t>(settings.container_type); }
    const char* getContainerTypeString() const;
    
    // Sensor timing management
    void setNormalInterval(uint16_t seconds);
    uint16_t getNormalInterval() const { return settings.normal_interval; }
    void setFillingInterval(uint16_t seconds);
    uint16_t getFillingInterval() const { return settings.filling_interval; }
    void setFillingThreshold(uint8_t threshold);
    uint8_t getFillingThreshold() const { return settings.filling_threshold; }
    
    // Geographic/NTP management
    void setTimezone(const String& timezone);
    String getTimezone() const { return settings.timezone; }
    void setCountryCode(const String& code);
    String getCountryCode() const { return settings.country_code; }
    void setCity(const String& city);
    String getCity() const { return settings.city; }
    void setAutoGeoLocation(bool enabled);
    bool getAutoGeoLocation() const { return settings.auto_geo_location; }
    void setShowDateTime(bool show);
    bool getShowDateTime() const { return settings.show_datetime; }
    
    // Access to settings
    const SensorSettings& getSettings() const { return settings; }
    
    // Validation
    bool isValid() const;
    
private:
    SensorSettings settings;
    
    // Default values
    static const double DEFAULT_MAX_HEIGHT;
    static const double DEFAULT_CAPACITY;
    static const double DEFAULT_MIN_DISTANCE;
    static const ContainerType DEFAULT_CONTAINER_TYPE = CONTAINER_TANK;
    
    // Timing defaults
    static const uint16_t DEFAULT_NORMAL_INTERVAL = 30;    // 30 segundos normal
    static const uint16_t DEFAULT_FILLING_INTERVAL = 5;    // 5 segundos llenando
    static const uint8_t DEFAULT_FILLING_THRESHOLD = 3;    // 3 lecturas incrementales
    
    // Geographic defaults
    static const String DEFAULT_TIMEZONE;
    static const String DEFAULT_COUNTRY_CODE;
    static const String DEFAULT_CITY;
    static const bool DEFAULT_AUTO_GEO = true;
    static const bool DEFAULT_SHOW_DATETIME = true;
};

#endif // SENSOR_CONFIG_H