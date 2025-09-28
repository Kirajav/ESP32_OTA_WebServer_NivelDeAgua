#ifndef WATER_LEVEL_SENSOR_H
#define WATER_LEVEL_SENSOR_H

#include "Sensor.h"
#include <HCSR04.h>
#include <ConfigManagerV2.h>

class WaterLevelSensor : public Sensor {
public:
    WaterLevelSensor(uint8_t triggerPin, uint8_t echoPin, ConfigManagerV2* configManager);

    // Sensor interface implementation
    bool begin() override;
    void update() override;
    String getName() const override;
    String getJson() const override;
    
    // ISensor interface overrides for better control
    bool hasValidReading() const override { return m_distanciaCm >= 0; }
    float getDistance() const override { return m_distanciaCm; }
    float getPercentage() const override;
    String getLastError() const override { return m_mensajeError; }
    bool isConnected() const override { return m_distanciaCm >= 0; }

private:
    UltraSonicDistanceSensor m_distanceSensor;
    ConfigManagerV2* m_configManager;
    float m_distanciaCm;
    String m_mensajeError;

    String calculateLitros() const;
};

#endif // WATER_LEVEL_SENSOR_H