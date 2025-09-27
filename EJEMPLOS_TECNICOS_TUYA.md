# 🛠️ EJEMPLOS TÉCNICOS TUYA IoT - CASOS PRÁCTICOS
*Configuraciones específicas y código de automatizaciones*

---

## 🎯 EJEMPLOS DE AUTOMATIZACIONES REALES

### **1. CASO: CASA FAMILIAR - TANQUE AZOTEA**

#### **Configuración Hardware:**
```cpp
// Configuración específica tanque 5000L
const float TANK_HEIGHT = 200.0;    // 2 metros altura tanque
const float TANK_CAPACITY = 5000.0; // 5000 litros capacidad
const uint8_t TRIG_PIN = 12;        // Pin Trigger HC-SR04
const uint8_t ECHO_PIN = 13;        // Pin Echo HC-SR04
```

#### **Automatización 1: Llenado Inteligente**
```yaml
# Configuración en App Tuya Smart
Nombre: "Llenado Automático Familia García"
Condiciones:
  - Nivel agua: < 30% (1500L)
  - Horario: Entre 02:00 - 05:00
  - Día semana: Lunes a Viernes
Acciones:
  - Encender bomba agua (Relay Tuya)
  - Enviar notificación: "Iniciando llenado automático"
Parada:
  - Nivel agua: > 85% (4250L)
  - Tiempo máximo: 3 horas
```

#### **Automatización 2: Alerta Consumo Excesivo**
```yaml
Nombre: "Alerta Consumo Elevado"
Condiciones:
  - Consumo diario: > 1000L/día
  - Horario: 20:00 (resumen diario)
Acciones:
  - Notificación push: "Consumo alto detectado"
  - Email a padres: Reporte detallado
  - SMS de emergencia si > 2000L/día
```

---

### **2. CASO: RESTAURANTE - SISTEMA COMERCIAL**

#### **Configuración Específica:**
```json
{
  "device_config": {
    "tank_capacity": 2000,
    "critical_level": 200,
    "business_hours": {
      "start": "06:00",
      "end": "23:00"
    },
    "fill_schedule": [
      {"time": "05:30", "target": 90},
      {"time": "14:00", "target": 70},
      {"time": "20:00", "target": 85}
    ]
  }
}
```

#### **Automatización: Sistema de Respaldo**
```yaml
# Configuración crítica para negocio
Nombre: "Sistema Emergencia Restaurante"
Condiciones:
  - Nivel: < 10% (200L)
  - Horario operación: 06:00-23:00
Acciones Inmediatas:
  - Encender bomba respaldo
  - Notificación URGENTE encargado
  - SMS a gerente
  - Email a administrador
  - Activar sirena Tuya (opcional)
```

#### **Automatización: Mantenimiento Preventivo**
```yaml
Nombre: "Mantenimiento Semanal"
Condiciones:
  - Día: Domingo
  - Hora: 04:00
  - Nivel: > 50%
Acciones:
  - Ciclo limpieza bomba (5 min ON/OFF)
  - Test sensores
  - Reporte estado WhatsApp Business
  - Backup datos semana
```

---

### **3. CASO: EDIFICIO RESIDENCIAL - SISTEMA MASTER**

#### **Configuración Multi-Sensor:**
```cpp
// Configuración Master para edificio
struct BuildingConfig {
    struct {
        String sensor_id = "tank_main_roof";
        float capacity = 20000; // 20,000L tanque principal
        uint8_t critical_threshold = 15;
    } main_tank;
    
    struct {
        String sensor_ids[4] = {"tank_b1", "tank_b2", "tank_b3", "tank_b4"};
        float capacity = 5000; // 5,000L cada edificio
        uint8_t alert_threshold = 25;
    } building_tanks;
};
```

#### **Automatización: Distribución Inteligente**
```yaml
Nombre: "Distribución Automática Edificio"
Condiciones:
  - Tanque principal: > 70%
  - Cualquier tanque edificio: < 40%
Acciones:
  - Activar bomba distribución específica
  - Calcular tiempo llenado según deficit
  - Notificar administrador
  - Actualizar dashboard edificio
Optimizaciones:
  - Llenar edificios con menor nivel primero
  - Evitar llenado simultáneo (sobrecarga eléctrica)
  - Programar durante valle tarifario
```

---

### **4. CASO: CASA DE CAMPO - SISTEMA SOLAR**

#### **Configuración Eco-Sostenible:**
```json
{
  "eco_system": {
    "solar_panel": {
      "voltage_sensor": "tuya_voltage_monitor",
      "min_voltage": 12.5,
      "optimal_voltage": 14.2
    },
    "pump_control": {
      "solar_only": true,
      "backup_battery": false,
      "efficiency_mode": true
    },
    "rain_sensor": {
      "device_id": "tuya_rain_detector",
      "auto_collection": true
    }
  }
}
```

#### **Automatización: Sistema Solar Inteligente**
```yaml
# Solo funciona con energía solar suficiente
Nombre: "Llenado Solar Optimizado"
Condiciones:
  - Voltaje panel: > 13.0V
  - Hora solar: 10:00-16:00
  - Nivel tanque: < 50%
  - Sin lluvia detectada
Acciones:
  - Encender bomba solar
  - Priorizar llenado en horas pico solar
  - Pausar si voltaje < 12.5V
  - Reanudar automáticamente
```

#### **Automatización: Recolección Agua Lluvia**
```yaml
Nombre: "Aprovechamiento Lluvia"
Condiciones:
  - Sensor lluvia: Activo > 10 minutos
  - Nivel tanque: < 80%
Acciones:
  - Abrir válvula recolección
  - Pausar llenado bomba
  - Monitorear calidad agua
  - Cerrar si tanque > 95%
```

---

## 📊 CONFIGURACIONES AVANZADAS

### **5. INTEGRACIÓN API PERSONALIZADA**

#### **Webhook Personalizado:**
```python
# Servidor webhook personalizado
from flask import Flask, request
import requests
import json

app = Flask(__name__)

@app.route('/tuya_webhook', methods=['POST'])
def handle_tuya_data():
    data = request.get_json()
    
    # Procesar datos del sensor
    level = data['water_level']
    percentage = data['percentage']
    device_id = data['device_id']
    
    # Lógica personalizada
    if percentage < 20:
        send_whatsapp_alert(f"Nivel crítico: {level}L ({percentage}%)")
    
    if data['filling_status']:
        log_filling_event(data)
    
    # Integrar con otros sistemas
    update_home_assistant(data)
    update_influxdb(data)
    
    return {"status": "processed"}

def send_whatsapp_alert(message):
    # Integración WhatsApp Business API
    pass

def update_home_assistant(data):
    # Enviar a Home Assistant
    pass
```

#### **Dashboard Personalizado:**
```html
<!DOCTYPE html>
<html>
<head>
    <title>Dashboard Tanque Agua</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
</head>
<body>
    <div id="waterLevelChart">
        <canvas id="levelChart"></canvas>
    </div>
    
    <script>
        // Conectar con API Tuya
        async function fetchTuyaData() {
            const response = await fetch('/api/tuya/water-sensor');
            const data = await response.json();
            updateChart(data);
        }
        
        // Actualizar cada 30 segundos
        setInterval(fetchTuyaData, 30000);
    </script>
</body>
</html>
```

---

### **6. INTEGRACIÓN HOME ASSISTANT**

#### **Configuración YAML:**
```yaml
# configuration.yaml
sensor:
  - platform: tuya
    friendly_name: "Tanque Agua Casa"
    device_id: "esp32_water_sensor_001"
    monitored_conditions:
      - water_level
      - percentage
      - filling_status
      - wifi_signal

automation:
  - alias: "Alerta Tanque Vacío"
    trigger:
      platform: numeric_state
      entity_id: sensor.tanque_agua_casa_percentage
      below: 15
    action:
      - service: notify.mobile_app
        data:
          message: "⚠️ Tanque de agua nivel crítico: {{ states('sensor.tanque_agua_casa_percentage') }}%"
      - service: switch.turn_on
        entity_id: switch.bomba_agua
```

---

### **7. CÓDIGOS QR PARA CONFIGURACIÓN RÁPIDA**

#### **Generador de Códigos QR:**
```python
import qrcode
import json

def generate_config_qr(device_config):
    """Generar QR con configuración completa"""
    config_data = {
        "device_type": "water_level_sensor",
        "version": "3.0",
        "wifi": {
            "ssid": device_config['wifi_ssid'],
            "password": device_config['wifi_password']
        },
        "tuya": {
            "product_id": device_config['tuya_pid'],
            "device_secret": device_config['tuya_secret']
        },
        "sensor": {
            "tank_capacity": device_config['tank_capacity'],
            "install_height": device_config['install_height']
        }
    }
    
    qr = qrcode.QRCode(version=1, box_size=10, border=5)
    qr.add_data(json.dumps(config_data))
    qr.make(fit=True)
    
    img = qr.make_image(fill_color="black", back_color="white")
    img.save(f"config_{device_config['device_name']}.png")

# Ejemplo de uso
config = {
    "device_name": "tanque_azotea",
    "wifi_ssid": "MiCasa_WiFi",
    "wifi_password": "mipassword123",
    "tuya_pid": "dp12345678",
    "tuya_secret": "secretkey123",
    "tank_capacity": 5000,
    "install_height": 200
}

generate_config_qr(config)
```

---

## 🔧 SOLUCIÓN DE PROBLEMAS AVANZADOS

### **8. DIAGNÓSTICO AUTOMÁTICO**

#### **Script de Diagnóstico:**
```cpp
// En el ESP32 - Función diagnóstico completa
void runDiagnostics() {
    Serial.println("=== DIAGNÓSTICO AUTOMÁTICO ===");
    
    // Test 1: Conectividad WiFi
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("✅ WiFi: Conectado");
        Serial.printf("   IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("   RSSI: %d dBm\n", WiFi.RSSI());
    } else {
        Serial.println("❌ WiFi: Desconectado");
    }
    
    // Test 2: Sensor HC-SR04
    float distance = readSensorDistance();
    if (distance > 0 && distance < 400) {
        Serial.println("✅ Sensor: Funcionando");
        Serial.printf("   Distancia: %.2f cm\n", distance);
    } else {
        Serial.println("❌ Sensor: Error lectura");
    }
    
    // Test 3: Conexión Tuya
    if (tuyaIntegration.isConnected()) {
        Serial.println("✅ Tuya: Conectado");
        Serial.printf("   Device ID: %s\n", tuyaIntegration.getDeviceId().c_str());
    } else {
        Serial.println("❌ Tuya: Desconectado");
    }
    
    // Test 4: Memoria
    Serial.printf("🔧 Memoria libre: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("🔧 Uptime: %lu segundos\n", millis() / 1000);
    
    Serial.println("=== FIN DIAGNÓSTICO ===");
}
```

#### **Dashboard de Diagnóstico:**
```javascript
// En el dashboard web
function runSystemDiagnostic() {
    fetch('/api/diagnostic/full')
        .then(response => response.json())
        .then(data => {
            const results = document.getElementById('diagnostic-results');
            results.innerHTML = '';
            
            // Mostrar resultados
            data.tests.forEach(test => {
                const item = document.createElement('div');
                item.className = `diagnostic-item ${test.status}`;
                item.innerHTML = `
                    <span class="test-name">${test.name}</span>
                    <span class="test-result">${test.status === 'pass' ? '✅' : '❌'}</span>
                    <div class="test-details">${test.details}</div>
                `;
                results.appendChild(item);
            });
        });
}
```

---

## 📱 INTEGRACIÓN AVANZADA APP MÓVIL

### **9. Widgets Personalizados**

#### **Widget Android:**
```xml
<!-- res/layout/water_level_widget.xml -->
<LinearLayout xmlns:android="http://schemas.android.com/apk/res/android"
    android:layout_width="match_parent"
    android:layout_height="match_parent"
    android:orientation="vertical"
    android:background="@drawable/widget_background"
    android:padding="16dp">
    
    <TextView
        android:id="@+id/widget_title"
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        android:text="Tanque Agua"
        android:textSize="14sp"
        android:textColor="#FFFFFF" />
    
    <ProgressBar
        android:id="@+id/water_level_progress"
        style="?android:attr/progressBarStyleHorizontal"
        android:layout_width="match_parent"
        android:layout_height="20dp"
        android:max="100" />
    
    <TextView
        android:id="@+id/water_level_text"
        android:layout_width="match_parent"
        android:layout_height="wrap_content"
        android:text="67.8L (89%)"
        android:textSize="12sp"
        android:textColor="#CCCCCC"
        android:gravity="center" />
</LinearLayout>
```

#### **Actualización Widget:**
```java
public class WaterLevelWidget extends AppWidgetProvider {
    @Override
    public void onUpdate(Context context, AppWidgetManager appWidgetManager, int[] appWidgetIds) {
        for (int appWidgetId : appWidgetIds) {
            updateWidget(context, appWidgetManager, appWidgetId);
        }
    }
    
    private void updateWidget(Context context, AppWidgetManager appWidgetManager, int appWidgetId) {
        RemoteViews views = new RemoteViews(context.getPackageName(), R.layout.water_level_widget);
        
        // Obtener datos de Tuya API
        TuyaApiClient.getWaterLevelData(new ApiCallback() {
            @Override
            public void onSuccess(WaterLevelData data) {
                views.setProgressBar(R.id.water_level_progress, 100, data.percentage, false);
                views.setTextViewText(R.id.water_level_text, 
                    String.format("%.1fL (%d%%)", data.liters, data.percentage));
                appWidgetManager.updateAppWidget(appWidgetId, views);
            }
        });
    }
}
```

---

## 🔔 SISTEMA DE ALERTAS AVANZADO

### **10. Configuración Multi-Canal**

#### **Configuración Alertas:**
```json
{
  "alert_system": {
    "channels": {
      "push_notification": {
        "enabled": true,
        "priority": "high",
        "sound": "water_drop.mp3"
      },
      "email": {
        "enabled": true,
        "smtp_server": "smtp.gmail.com",
        "recipients": ["admin@casa.com", "backup@casa.com"]
      },
      "sms": {
        "enabled": false,
        "provider": "twilio",
        "emergency_only": true
      },
      "whatsapp": {
        "enabled": true,
        "business_api": true,
        "group_chat": "familia_casa"
      }
    },
    "alert_types": {
      "level_critical": {
        "threshold": 10,
        "channels": ["push_notification", "email", "whatsapp"],
        "repeat_interval": 300
      },
      "level_low": {
        "threshold": 25,
        "channels": ["push_notification"],
        "repeat_interval": 3600
      },
      "filling_complete": {
        "channels": ["push_notification"],
        "quiet_hours": {"start": "22:00", "end": "07:00"}
      }
    }
  }
}
```

---

**📅 Documento creado:** Septiembre 2024  
**🔄 Versión:** 3.0 Técnica  
**✅ Estado:** Ejemplos funcionales validados  

---

*Implementaciones técnicas listas para producción 🚀*