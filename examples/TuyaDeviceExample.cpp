// EJEMPLO DE USO: Cómo hacer que tu ESP32 aparezca automáticamente en la App Tuya
// Igual que cualquier dispositivo comercial que compras

#include "TuyaDevice.h"
#include "SensorManager.h"

TuyaDevice tuyaDevice;
SensorManager sensorManager;

void setup() {
    Serial.begin(115200);
    
    // 1. Configurar el dispositivo (igual que un producto comercial)
    TuyaDevice::DeviceConfig config;
    config.deviceName = "Sensor de Agua Inteligente";
    config.deviceModel = "ESP32-WLS-V2.0";
    config.firmwareVersion = "2.0.1";
    config.type = TuyaDevice::SENSOR;
    config.category = "cz"; // Categoría de sensores de agua en Tuya
    
    // 2. Inicializar dispositivo Tuya
    tuyaDevice.begin(config);
    
    // 3. Primera vez: ESP32 creará WiFi "SmartConfig_XXXXXX"
    //    Usuario se conecta y configura desde celular
    //    ¡Igual que cualquier switch inteligente que compres!
    
    Serial.println("🚀 Device starting...");
    Serial.println("📱 If first time, connect to config WiFi and setup");
}

void loop() {
    // 1. Loop del dispositivo Tuya (maneja todo automáticamente)
    tuyaDevice.loop();
    
    // 2. Si está conectado, enviar datos del sensor
    if (tuyaDevice.isConnected()) {
        static unsigned long lastSensorUpdate = 0;
        
        if (millis() - lastSensorUpdate > 30000) { // Cada 30 segundos
            // Leer datos del sensor
            String sensorJson = sensorManager.getSensorJson("WaterLevel");
            
            // Parsear datos
            DynamicJsonDocument doc(512);
            deserializeJson(doc, sensorJson);
            
            float litros = doc["litros"];
            float porcentaje = doc["porcentaje"];
            float distancia = doc["distancia_cm"];
            bool alerta = (porcentaje < 20.0);
            
            // Enviar a Tuya (aparecerá automáticamente en la app)
            tuyaDevice.updateSensorValue("1", litros);      // DP1: Litros
            tuyaDevice.updateSensorValue("2", porcentaje);  // DP2: Porcentaje  
            tuyaDevice.updateSensorValue("3", distancia);   // DP3: Distancia
            tuyaDevice.updateStatus("4", alerta);           // DP4: Alerta agua baja
            
            Serial.println("📊 Data sent to Tuya App:");
            Serial.println("   💧 Water: " + String(litros) + "L (" + String(porcentaje) + "%)");
            Serial.println("   📏 Distance: " + String(distancia) + "cm");
            Serial.println("   🚨 Alert: " + String(alerta ? "SI" : "NO"));
            
            lastSensorUpdate = millis();
        }
    }
    
    delay(100);
}

/*
📱 CÓMO FUNCIONA PARA EL USUARIO FINAL:

1. 🔌 Primera vez - Configuración:
   - ESP32 crea WiFi "SmartConfig_A1B2C3"
   - Usuario se conecta desde celular
   - Abre navegador → http://192.168.4.1
   - Configura WiFi de casa y nombre del dispositivo
   - ✅ Listo!

2. 📱 Agregar a App Tuya:
   - Abrir App "Tuya Smart" (gratuita)
   - Tocar "+" → "Agregar Dispositivo"
   - Seleccionar "Sensores" → "Sensor de Agua"
   - ¡Aparece automáticamente!
   - Tocar y agregar

3. 🏠 Uso diario:
   - Ver nivel de agua en tiempo real
   - Recibir notificaciones cuando esté bajo
   - Crear automatizaciones con otros dispositivos
   - ¡Todo desde la misma app Tuya!

🎯 RESULTADO: Tu ESP32 se comporta EXACTAMENTE igual que un
   dispositivo comercial. El usuario NO necesita:
   - Cuentas de desarrollador
   - Configuraciones técnicas
   - Webhooks o APIs
   - ¡Nada técnico!
*/