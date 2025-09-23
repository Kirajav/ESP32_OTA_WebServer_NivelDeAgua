#ifndef WATER_LEVEL_SENSOR_H
#define WATER_LEVEL_SENSOR_H

#include "Sensor.h"
#include <HCSR04.h>
#include <ConfigManager.h>

class WaterLevelSensor : public Sensor {
public:
    WaterLevelSensor(uint8_t triggerPin, uint8_t echoPin, ConfigManager* configManager);

    void begin() override;
    void update() override;
    String getName() const override;
    String getJson() const override;

private:
    UltraSonicDistanceSensor m_distanceSensor;
    ConfigManager* m_configManager;
    float m_distanciaCm;
    String m_mensajeError;

    String calculateLitros() const;
};

#endif // WATER_LEVEL_SENSOR_H