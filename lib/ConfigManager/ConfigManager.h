#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"

// --- Parámetros de Configuración (valores por defecto) ---
extern const double DEFAULT_ALTURA_MAX_AGUA_TINACO;
extern const double DEFAULT_CAPACIDAD_LITROS_TINACO;
extern const double DEFAULT_DISTANCIA_MINIMA_SENSOR;
extern const char* DEFAULT_HOSTNAME;
extern const bool DEFAULT_CHECK_UPDATES;
extern const uint8_t DEFAULT_TIPO_CONTENEDOR; // 0=Tinaco, 1=Cisterna, 2=Contenedor

// --- Estructura de Configuración ---
struct Config {
    double altura_max;
    double capacidad;
    double distancia_min;
    String hostname;
    bool check_updates;
    bool display_on; // Estado del display, se guarda aquí
    uint8_t tipo_contenedor; // 0=Tinaco, 1=Cisterna, 2=Contenedor
};

class ConfigManager {
public:
    ConfigManager();
    void begin();
    void saveConfig();
    bool loadConfig();

    Config& getConfig();

    void setAlturaMax(double altura_max);
    double getAlturaMax();

    void setCapacidad(double capacidad);
    double getCapacidad();

    void setDistanciaMin(double distancia_min);
    double getDistanciaMin();

    void setHostname(const String& hostname);
    String getHostname();

    void setCheckUpdates(bool check_updates);
    bool getCheckUpdates();

    void setDisplayOn(bool display_on);
    bool isDisplayOn();

    void setTipoContenedor(uint8_t tipo_contenedor);
    uint8_t getTipoContenedor();
    const char* getTipoContenedorStr();

private:
    Config _config;
};

#endif // CONFIG_MANAGER_H
