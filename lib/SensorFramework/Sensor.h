#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include <ArduinoJson.h>

// Interfaz para cualquier tipo de sensor
class Sensor {
public:
    virtual ~Sensor() {}

    // Inicializa el sensor (ej. configurar pines)
    virtual void begin() = 0;

    // Lee los datos del sensor y actualiza los valores internos
    virtual void update() = 0;

    // Devuelve el nombre del sensor
    virtual String getName() const = 0;

    // Devuelve un JSON con todas las lecturas del sensor
    virtual String getJson() const = 0;
};

#endif // SENSOR_H
