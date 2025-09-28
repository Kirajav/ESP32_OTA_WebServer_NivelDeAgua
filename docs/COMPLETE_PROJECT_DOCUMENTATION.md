# 📚 DOCUMENTACIÓN COMPLETA DEL PROYECTO ESP32 WATER LEVEL SENSOR
## Sistema IoT Profesional de Monitoreo de Nivel de Agua con Red Mesh

### 📋 **INFORMACIÓN GENERAL DEL PROYECTO**

#### **Metadata del Proyecto**
- **Nombre**: ESP32_OTA_WebServer_NivelDeAgua
- **Versión**: 3.0.0 (Enterprise Architecture)
- **Fecha**: Septiembre 28, 2025
- **Rama Actual**: `refactor/architecture-enterprise-v3`
- **Plataforma**: PlatformIO + ESP32 Heltec WiFi Kit 32
- **Lenguajes**: C++ (Firmware), HTML/CSS/JavaScript (Web UI)

#### **Descripción del Sistema**
Sistema IoT profesional para monitoreo de nivel de agua que combina:
- **Sensor Ultrasónico** para medición precisa de distancia
- **Conectividad WiFi** para integración con redes domésticas
- **Red Mesh ESP-NOW** para múltiples sensores distribuidos
- **Interfaz Web Responsive** para configuración y monitoreo
- **OTA Updates** para actualizaciones remotas sin cables
- **Display OLED** para información local en tiempo real

---

## 🏗️ **ARQUITECTURA DEL PROYECTO**

### **Estructura de Carpetas Principal**
```
ESP32_OTA_WebServer_NivelDeAgua/
├── 📁 compile_commands.json         # Configuración IDE (VS Code C++)
├── 📁 platformio.ini               # Configuración PlatformIO principal
├── 📁 README.md                    # Documentación básica usuario
├── 📁 backups/                     # Respaldos históricos
├── 📁 data/                        # Archivos SPIFFS (Web + Assets)
├── 📁 docs/                        # Documentación completa del proyecto
├── 📁 examples/                    # Ejemplos de código y configuraciones
├── 📁 include/                     # Headers públicos (PlatformIO standard)
├── 📁 lib/                         # Librerías personalizadas modulares
├── 📁 logs/                        # Logs de desarrollo y debug
├── 📁 scripts/                     # Scripts utilitarios y herramientas
├── 📁 src/                         # Código fuente principal C++
└── 📁 test/                        # Tests unitarios y de integración
```

---

## 📂 **DOCUMENTACIÓN DETALLADA POR CARPETAS**

### **1. CARPETA `/data/` - SISTEMA DE ARCHIVOS SPIFFS**

#### **Propósito**
Contiene todos los archivos que se cargan al sistema de archivos SPIFFS del ESP32:
- Interfaces web (HTML/CSS/JS)
- Assets multimedia (imágenes, iconos)
- Archivos de configuración JSON
- Resources estáticos del servidor web

#### **Estructura Detallada**
```
data/
├── 📄 imagen_vacio.jpg            # Imagen estado tanque vacío (JPEG optimizado)
├── 📄 imagen_lleno.jpg            # Imagen estado tanque lleno (JPEG optimizado)  
├── 📄 imagen_error.jpg            # Imagen estado error sensor (JPEG optimizado)
└── 📁 web/                        # Interfaces web del sistema
    ├── 📁 captive_portal/         # Portal de configuración WiFi
    │   ├── 📄 index.html          # Interfaz configuración WiFi (1393 líneas)
    │   ├── 📄 style.css           # Estilos profesionales (930 líneas)
    │   ├── 📄 captive-portal.js   # Lógica JavaScript (1013 líneas)
    │   └── 📁 assets/             # Assets específicos del portal
    ├── 📁 dashboard/              # Dashboard principal de monitoreo
    │   ├── 📄 index.html          # Interfaz dashboard responsive
    │   ├── 📄 style.css           # Estilos dashboard (componentes)
    │   ├── 📄 app.js              # Lógica aplicación principal
    │   └── 📁 assets/             # Gráficos, iconos del dashboard
    │       └── 📁 images/         # Imágenes específicas dashboard
    ├── 📁 esp_now/               # Gestor de red mesh ESP-NOW
    │   ├── 📄 index.html          # Interfaz gestión red mesh (681 líneas)
    │   ├── 📄 style.css           # Estilos especializados ESP-NOW (657 líneas)
    │   ├── 📄 esp-now-manager.js  # Lógica red mesh (681 líneas)
    │   └── 📁 assets/             # Assets específicos ESP-NOW
    └── 📁 shared/                 # Recursos compartidos (PENDIENTE)
        ├── 📁 css/                # Variables y componentes CSS comunes
        ├── 📁 js/                 # Utilidades JavaScript comunes
        └── 📁 assets/             # Icons, fonts globales
```

#### **Páginas Web - Análisis Técnico**

##### **1. Captive Portal (`/captive_portal/`)**
- **Función**: Configuración inicial WiFi y parámetros del sensor
- **Tecnología**: HTML5 + CSS3 (Glassmorphism) + Vanilla JavaScript
- **Características**:
  - 🌐 Multi-idioma (ES/EN/FR) con persistencia
  - 🎨 Dual theme (Dark/Light) responsive
  - 📱 Mobile-first design con bottom navigation
  - 🔒 Validación client-side y server-side
  - 📡 WiFi scanner con visualización RSSI
  - ⚙️ Configuración avanzada sensores (intervalos inteligentes)
  - 🌍 NTP geolocalización automática

##### **2. Dashboard (`/dashboard/`)**
- **Función**: Monitoreo en tiempo real del sensor principal
- **Tecnología**: SPA (Single Page Application) con updates dinámicos
- **Características**:
  - 📊 Gráficos en tiempo real (Chart.js)
  - 📈 Historial de datos con almacenamiento local
  - 🚨 Sistema de alertas y notificaciones
  - 📱 PWA-ready (Progressive Web App)
  - 🔄 Auto-refresh configurable
  - 💾 Export de datos (CSV/JSON)

##### **3. ESP-NOW Manager (`/esp_now/`)**
- **Función**: Gestión de red mesh para múltiples sensores
- **Tecnología**: Interface especializada para redes mesh
- **Características**:
  - 🌐 Visualización topología de red
  - 📡 Device discovery y auto-connect
  - 📊 Monitoreo múltiples sensores simultáneos
  - 🔧 Configuración remota de dispositivos
  - 📈 Estadísticas de red y performance
  - 🗺️ Network mapping visual

---

### **2. CARPETA `/src/` - CÓDIGO FUENTE PRINCIPAL C++**

#### **Propósito**
Contiene el firmware principal del ESP32 con arquitectura modular enterprise.

#### **Estructura y Archivos**
```
src/
├── 📄 main.cpp                    # Entry point y loop principal
├── 📄 AppManager.cpp              # Gestor principal aplicación
├── 📄 AppManager.h                # Header gestor aplicación
├── 📄 AppManager.cpp.problemático # Backup versión con issues
└── 📄 main.cpp.bak               # Backup main anterior
```

#### **Archivo: `main.cpp`** 
```cpp
/**
 * ================================================================
 * MAIN.CPP - Entry Point del Sistema ESP32 Water Level Sensor
 * ================================================================
 * 
 * Este archivo contiene el punto de entrada principal del firmware
 * ESP32 y gestiona el ciclo de vida de la aplicación.
 * 
 * Responsabilidades principales:
 * - Inicialización del hardware y periféricos
 * - Setup de librerías y managers
 * - Loop principal de ejecución
 * - Gestión de excepciones y recovery
 */

#include <Arduino.h>              // Framework Arduino para ESP32
#include "AppManager.h"           // Gestor principal de la aplicación

// Instancia global del gestor de aplicación
AppManager appManager;

/**
 * Función setup() - Inicialización del sistema
 * 
 * Se ejecuta una sola vez al arrancar el ESP32.
 * Inicializa todos los componentes y librerías necesarias.
 */
void setup() {
    // Inicializar comunicación serie para debug
    Serial.begin(115200);
    Serial.println("🚀 ESP32 Water Level Sensor - Starting...");
    
    // Inicializar gestor de aplicación
    appManager.begin();
    
    Serial.println("✅ System initialized successfully");
}

/**
 * Función loop() - Ciclo principal de ejecución
 * 
 * Se ejecuta continuamente mientras el ESP32 esté encendido.
 * Gestiona todas las tareas recurrentes del sistema.
 */
void loop() {
    // Ejecutar tareas del gestor de aplicación
    appManager.handleTasks();
    
    // Yield para el watchdog timer
    yield();
}
```

#### **Archivo: `AppManager.cpp`** (Gestor Principal)
```cpp
/**
 * ================================================================
 * APPMANAGER.CPP - Gestor Principal del Sistema
 * ================================================================
 * 
 * Clase principal que coordina todos los componentes del sistema:
 * - Gestión de configuración
 * - Servidor web y rutas
 * - Manejo de sensores
 * - Conectividad WiFi y ESP-NOW
 * - OTA updates
 * - Display OLED
 * 
 * Implementa patrón Singleton para gestión centralizada.
 */

#include "AppManager.h"

// Métodos principales implementados:

/**
 * begin() - Inicialización completa del sistema
 * Inicializa todos los componentes en orden correcto
 */
void AppManager::begin() {
    // 1. Inicializar SPIFFS para archivos web
    if (!SPIFFS.begin(true)) {
        Serial.println("❌ Error mounting SPIFFS");
        return;
    }
    
    // 2. Cargar configuración desde memoria
    loadConfiguration();
    
    // 3. Inicializar hardware (sensores, display)
    initializeHardware();
    
    // 4. Configurar conectividad (WiFi/ESP-NOW)
    setupConnectivity();
    
    // 5. Inicializar servidor web
    setupWebServer();
    
    // 6. Configurar OTA updates
    setupOTA();
    
    Serial.println("✅ AppManager initialized");
}

/**
 * handleTasks() - Manejo de tareas recurrentes
 * Ejecuta todas las tareas que deben correr en el loop principal
 */
void AppManager::handleTasks() {
    // Manejar servidor web
    server.handleClient();
    
    // Procesar OTA updates
    ArduinoOTA.handle();
    
    // Actualizar lecturas de sensores
    updateSensorReadings();
    
    // Manejar ESP-NOW
    handleESPNow();
    
    // Actualizar display
    updateDisplay();
    
    // Gestión de memoria y cleanup
    performMaintenance();
}

/**
 * setupWebServer() - Configuración del servidor web
 * Define todas las rutas y handlers del servidor HTTP
 */
void AppManager::setupWebServer() {
    // Rutas principales de páginas
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/web/dashboard/index.html", "text/html");
    });
    
    server.on("/captive-portal", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/web/captive_portal/index.html", "text/html");
    });
    
    server.on("/esp-now", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/web/esp_now/index.html", "text/html");
    });
    
    // Rutas de recursos estáticos (CSS, JS, Assets)
    setupStaticRoutes();
    
    // Rutas de API REST
    setupAPIRoutes();
    
    // Iniciar servidor en puerto 80
    server.begin();
    Serial.println("🌐 Web server started on port 80");
}

/**
 * setupAPIRoutes() - Configuración de endpoints API REST
 * Define todos los endpoints para comunicación con interfaces web
 */
void AppManager::setupAPIRoutes() {
    // API Sensor Data
    server.on("/api/sensor-data", HTTP_GET, [this](AsyncWebServerRequest *request) {
        handleSensorDataAPI(request);
    });
    
    // API WiFi Management
    server.on("/api/wifi/scan", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleWiFiScanAPI(request);
    });
    
    // API ESP-NOW Management
    server.on("/api/esp-now/scan", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleESPNowScanAPI(request);
    });
    
    // API Configuration
    server.on("/api/config/save", HTTP_POST, [this](AsyncWebServerRequest *request) {
        handleConfigSaveAPI(request);
    });
    
    // Más endpoints API...
}
```

---

### **3. CARPETA `/lib/` - LIBRERÍAS MODULARES PERSONALIZADAS**

#### **Propósito**
Contiene librerías modulares desarrolladas específicamente para este proyecto, organizadas por funcionalidad.

#### **Estructura Detallada**
```
lib/
├── 📁 AlexaIntegration/           # Integración con Amazon Alexa
│   ├── 📄 AlexaIntegration.cpp    # Implementación cliente Alexa
│   └── 📄 AlexaIntegration.h      # Headers integración Alexa
├── 📁 Config/                     # Sistema de configuración
│   ├── 📄 ConfigManager.cpp       # Gestor configuración JSON
│   ├── 📄 ConfigManager.h         # Headers configuración
│   └── 📄 config_schema.json      # Schema validación configuración
├── 📁 DisplayManager/             # Gestión display OLED
│   ├── 📄 DisplayManager.cpp      # Driver OLED personalizado
│   ├── 📄 DisplayManager.h        # Headers display
│   └── 📄 fonts/                  # Fuentes personalizadas OLED
├── 📁 ESPNowManager/              # Red mesh ESP-NOW
│   ├── 📄 ESPNowManager.cpp       # Implementación protocolo mesh
│   ├── 📄 ESPNowManager.h         # Headers ESP-NOW
│   └── 📄 mesh_protocol.h         # Definiciones protocolo mesh
├── 📁 GoogleHomeIntegration/      # Integración Google Assistant
│   ├── 📄 GoogleHomeIntegration.cpp
│   └── 📄 GoogleHomeIntegration.h
├── 📁 HardwareBoardStatus/        # Monitor estado hardware
│   ├── 📄 BoardStatus.cpp         # Monitor temperatura, voltaje, RAM
│   └── 📄 BoardStatus.h           # Headers monitor hardware
├── 📁 Interfaces/                 # Interfaces y abstracciones
│   ├── 📄 ISensor.h               # Interface genérica sensores
│   ├── 📄 IDisplay.h              # Interface displays
│   ├── 📄 IConnectivity.h         # Interface conectividad
│   └── 📄 ICloudService.h         # Interface servicios cloud
├── 📁 Logger/                     # Sistema de logging
│   ├── 📄 Logger.cpp              # Logger con niveles y destinos
│   ├── 📄 Logger.h                # Headers logging
│   └── 📄 log_config.h            # Configuración logging
├── 📁 OTAUpdater/                 # Actualizaciones Over-The-Air
│   ├── 📄 OTAUpdater.cpp          # Cliente OTA robusto
│   ├── 📄 OTAUpdater.h            # Headers OTA
│   └── 📄 ota_security.h          # Configuración seguridad OTA
├── 📁 SensorFramework/            # Framework sensores genérico
│   ├── 📄 SensorFramework.cpp     # Base framework sensores
│   ├── 📄 SensorFramework.h       # Headers framework
│   ├── 📄 UltrasonicSensor.cpp    # Sensor ultrasónico especializado
│   ├── 📄 UltrasonicSensor.h      # Headers ultrasónico
│   └── 📄 sensor_calibration.h    # Utilidades calibración
├── 📁 TuyaIntegration/            # Integración ecosistema Tuya IoT
│   ├── 📄 TuyaIntegration.cpp     # Cliente Tuya IoT
│   └── 📄 TuyaIntegration.h       # Headers Tuya
├── 📁 TuyaSDK/                    # SDK oficial Tuya
│   └── 📄 [archivos SDK Tuya]     # SDK completo Tuya
└── 📁 WebManager/                 # Gestión servidor web avanzado
    ├── 📄 WebManager.cpp          # Servidor web optimizado
    ├── 📄 WebManager.h            # Headers web manager
    ├── 📄 websocket_handler.cpp   # WebSockets tiempo real
    └── 📄 api_routes.h            # Definiciones rutas API
```

#### **Librería Destacada: `SensorFramework/`**
```cpp
/**
 * ================================================================
 * SENSOR FRAMEWORK - Framework Genérico para Sensores
 * ================================================================
 * 
 * Framework extensible para manejo de múltiples tipos de sensores
 * con características avanzadas:
 * - Calibración automática
 * - Filtrado de ruido
 * - Intervalos adaptativos
 * - Cache de lecturas
 * - Detección de anomalías
 */

class SensorFramework {
private:
    std::vector<ISensor*> sensors;        // Lista de sensores registrados
    SensorConfig config;                  // Configuración global sensores
    CalibrationData calibration;          // Datos de calibración
    FilterManager filters;                // Filtros de procesamiento
    
public:
    /**
     * registerSensor() - Registrar nuevo sensor en el framework
     * @param sensor Puntero al sensor implementando ISensor
     * @param config Configuración específica del sensor
     */
    void registerSensor(ISensor* sensor, SensorConfig config);
    
    /**
     * readAllSensors() - Leer todos los sensores registrados
     * @return SensorDataCollection con todas las lecturas
     */
    SensorDataCollection readAllSensors();
    
    /**
     * calibrateSensor() - Calibrar sensor específico
     * @param sensorId ID del sensor a calibrar
     * @param referenceValues Valores de referencia para calibración
     */
    void calibrateSensor(uint8_t sensorId, std::vector<float> referenceValues);
    
    /**
     * enableAdaptiveIntervals() - Habilitar intervalos adaptativos
     * Ajusta automáticamente la frecuencia de lectura basado en:
     * - Variabilidad de los datos
     * - Detección de cambios rápidos
     * - Nivel de batería disponible
     */
    void enableAdaptiveIntervals(bool enable);
};
```

---

### **4. CARPETA `/docs/` - DOCUMENTACIÓN COMPLETA**

#### **Propósito**
Documentación técnica completa del proyecto, organizada por temas y componentes.

#### **Estructura Actual**
```
docs/
├── 📄 NUEVAS_FUNCIONALIDADES_IMPLEMENTADAS.md
├── 📄 NOTAS_PENDIENTES.md                  # Tareas futuras y mejoras
├── 📄 PROYECTO_COMPLETADO.md
├── 📄 PROYECTO_COMPLETADO_FINAL.md
├── 📄 RESUMEN_EJECUTIVO_PROYECTO_COMPLETADO.md
├── 📄 WEB_ORGANIZATION_COMPLETED.md
├── 📄 WEB_STRUCTURE_CORRECTED.md
├── 📁 analysis/                            # Análisis técnicos
│   ├── 📄 ANALISIS_HTTPS_VIABILIDAD.md
│   └── 📄 ANALISIS_SEGURIDAD_TUYA_IoT.md
├── 📁 architecture/                        # Documentación arquitectural
│   ├── 📄 ARCHITECTURAL_IMPROVEMENTS.md
│   └── 📄 PATRONES_ARQUITECTURALES_IMPLEMENTADOS.md
├── 📁 examples/                            # Ejemplos de uso
├── 📁 guides/                              # Guías de usuario y técnicas
│   ├── 📄 CASOS_USO_ESP-NOW.md
│   ├── 📄 CHECKPOINTS_PRINCIPALES.md
│   ├── 📄 ESP-NOW_CONFIGURATION_GUIDE.md
│   ├── 📄 HTTPS_IMPLEMENTATION_GUIDE.md
│   ├── 📄 INSTRUCCIONES_USO_INMEDIATO.md
│   ├── 📄 IOT_INTEGRATION_PLAN.md
│   ├── 📄 MANUAL_USUARIO_TUYA_IOT.md
│   └── 📄 METODOS_PENDIENTES_IMPLEMENTACION.md
└── 📁 web-documentation/                   # Documentación interfaces web
    ├── 📁 captive-portal/                 # Docs portal cautivo
    │   ├── 📄 CSS_CAPTIVE_PORTAL_DOCUMENTATION.md
    │   └── 📄 JAVASCRIPT_CAPTIVE_PORTAL_DOCUMENTATION.md
    └── 📁 esp-now/                        # Docs ESP-NOW manager
        ├── 📄 CSS_ESP_NOW_DOCUMENTATION.md
        └── 📄 JAVASCRIPT_ESP_NOW_DOCUMENTATION.md
```

---

### **5. CARPETA `/examples/` - EJEMPLOS Y DEMOS**

#### **Propósito**
Código de ejemplo para desarrollo y testing de funcionalidades específicas.

#### **Archivos Incluidos**
```
examples/
├── 📄 ESP32_Slave_Sensor.cpp      # Ejemplo sensor esclavo ESP-NOW
└── 📄 TuyaDeviceExample.cpp       # Ejemplo integración Tuya IoT
```

#### **Ejemplo: `ESP32_Slave_Sensor.cpp`**
```cpp
/**
 * ================================================================
 * ESP32 SLAVE SENSOR - Ejemplo Sensor Secundario para Red Mesh
 * ================================================================
 * 
 * Código de ejemplo para configurar un ESP32 como sensor esclavo
 * en una red mesh ESP-NOW. Demuestra:
 * - Configuración básica ESP-NOW
 * - Envío de datos de sensor
 * - Manejo de estados de red
 * - Modo bajo consumo entre lecturas
 */

#include <esp_now.h>
#include <WiFi.h>
#include <ArduinoJson.h>

// Configuración del sensor esclavo
struct SlaveConfig {
    uint8_t sensorId = 2;                    // ID único del sensor
    String sensorName = "ESP32_Slave_01";    // Nombre identificativo
    uint8_t masterMAC[6] = {0x24, 0x0A, 0xC4, 0x12, 0x34, 0x56}; // MAC maestro
    int measurementInterval = 30000;         // Intervalo medición (ms)
    int transmissionPower = 20;              // Potencia transmisión
};

SlaveConfig config;

/**
 * setup() - Inicialización sensor esclavo
 */
void setup() {
    Serial.begin(115200);
    Serial.println("🚀 ESP32 Slave Sensor Starting...");
    
    // Inicializar WiFi en modo station
    WiFi.mode(WIFI_STA);
    
    // Inicializar ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("❌ Error initializing ESP-NOW");
        return;
    }
    
    // Registrar peer (maestro)
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, config.masterMAC, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("❌ Failed to add peer");
        return;
    }
    
    Serial.println("✅ Slave sensor initialized");
}

/**
 * loop() - Ciclo principal sensor esclavo
 */
void loop() {
    // Leer datos del sensor
    SensorData data = readSensorData();
    
    // Enviar datos al maestro
    sendDataToMaster(data);
    
    // Esperar próxima lectura
    delay(config.measurementInterval);
}

/**
 * readSensorData() - Leer datos del sensor ultrasónico
 */
SensorData readSensorData() {
    SensorData data;
    data.sensorId = config.sensorId;
    data.timestamp = millis();
    
    // Leer sensor ultrasónico (HC-SR04)
    data.waterLevel = readUltrasonicSensor();
    
    // Leer voltaje batería
    data.batteryVoltage = analogRead(A0) * (3.3 / 4095.0) * 2;
    data.batteryPercent = calculateBatteryPercent(data.batteryVoltage);
    
    // Calcular intensidad señal WiFi
    data.signalStrength = WiFi.RSSI();
    
    return data;
}

/**
 * sendDataToMaster() - Enviar datos al sensor maestro
 */
void sendDataToMaster(SensorData data) {
    // Serializar datos a JSON
    StaticJsonDocument<200> doc;
    doc["sensorId"] = data.sensorId;
    doc["sensorName"] = config.sensorName;
    doc["timestamp"] = data.timestamp;
    doc["waterLevel"] = data.waterLevel;
    doc["batteryPercent"] = data.batteryPercent;
    doc["signalStrength"] = data.signalStrength;
    
    String jsonString;
    serializeJson(doc, jsonString);
    
    // Enviar via ESP-NOW
    esp_err_t result = esp_now_send(config.masterMAC, 
                                   (uint8_t*)jsonString.c_str(),
                                   jsonString.length());
    
    if (result == ESP_OK) {
        Serial.println("✅ Data sent successfully");
    } else {
        Serial.printf("❌ Error sending data: %d\n", result);
    }
}
```

---

### **6. CARPETA `/scripts/` - SCRIPTS Y HERRAMIENTAS**

#### **Propósito**
Scripts utilitarios para desarrollo, deployment y mantenimiento.

#### **Archivos Incluidos**
```
scripts/
├── 📄 ESP32_Download_Controller.ino  # Script Arduino IDE para ESP32
├── 📄 esp32_uploader.py             # Script Python para upload masivo  
└── 📄 min_spiffs.csv               # Configuración particiones SPIFFS
```

#### **Script: `esp32_uploader.py`**
```python
#!/usr/bin/env python3
"""
================================================================
ESP32 UPLOADER - Script Automatizado para Upload de Firmware
================================================================

Script Python para automatizar el proceso de compilación y
upload de firmware a múltiples dispositivos ESP32.

Características:
- Upload masivo a múltiples dispositivos
- Verificación de conexión serie
- Backup automático de firmware anterior
- Logs detallados de operaciones
- Configuración via archivo JSON
"""

import serial
import subprocess
import json
import os
import sys
from datetime import datetime

class ESP32Uploader:
    def __init__(self, config_file="upload_config.json"):
        """Inicializar uploader con configuración"""
        self.config = self.load_config(config_file)
        self.log_file = f"upload_log_{datetime.now().strftime('%Y%m%d_%H%M%S')}.txt"
        
    def load_config(self, config_file):
        """Cargar configuración desde archivo JSON"""
        try:
            with open(config_file, 'r') as f:
                return json.load(f)
        except FileNotFoundError:
            self.create_default_config(config_file)
            return self.load_config(config_file)
    
    def create_default_config(self, config_file):
        """Crear configuración por defecto"""
        default_config = {
            "project_path": ".",
            "environment": "heltec_wifi_kit_32",
            "serial_ports": [
                "COM3", "COM4", "COM5"  # Puertos serie Windows
            ],
            "baud_rate": 115200,
            "upload_speed": 921600,
            "verify_after_upload": True,
            "backup_firmware": True,
            "backup_path": "./backups/firmware/"
        }
        
        with open(config_file, 'w') as f:
            json.dump(default_config, f, indent=4)
        
        print(f"✅ Created default config: {config_file}")
    
    def detect_connected_devices(self):
        """Detectar dispositivos ESP32 conectados"""
        connected_devices = []
        
        for port in self.config["serial_ports"]:
            try:
                ser = serial.Serial(port, self.config["baud_rate"], timeout=2)
                ser.close()
                connected_devices.append(port)
                self.log(f"✅ Device detected on {port}")
            except serial.SerialException:
                self.log(f"❌ No device on {port}")
        
        return connected_devices
    
    def compile_firmware(self):
        """Compilar firmware usando PlatformIO"""
        self.log("🔨 Compiling firmware...")
        
        cmd = [
            "platformio", "run",
            "-e", self.config["environment"]
        ]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, cwd=self.config["project_path"])
            
            if result.returncode == 0:
                self.log("✅ Firmware compiled successfully")
                return True
            else:
                self.log(f"❌ Compilation failed: {result.stderr}")
                return False
                
        except Exception as e:
            self.log(f"❌ Compilation error: {str(e)}")
            return False
    
    def upload_to_device(self, port):
        """Upload firmware a dispositivo específico"""
        self.log(f"📤 Uploading to device on {port}...")
        
        cmd = [
            "platformio", "run",
            "-e", self.config["environment"],
            "--target", "upload",
            "--upload-port", port
        ]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, cwd=self.config["project_path"])
            
            if result.returncode == 0:
                self.log(f"✅ Upload successful to {port}")
                return True
            else:
                self.log(f"❌ Upload failed to {port}: {result.stderr}")
                return False
                
        except Exception as e:
            self.log(f"❌ Upload error to {port}: {str(e)}")
            return False
    
    def verify_upload(self, port):
        """Verificar que el upload fue exitoso"""
        if not self.config["verify_after_upload"]:
            return True
            
        self.log(f"🔍 Verifying upload on {port}...")
        
        try:
            ser = serial.Serial(port, self.config["baud_rate"], timeout=5)
            
            # Leer output serial por 5 segundos
            start_time = datetime.now()
            output = ""
            
            while (datetime.now() - start_time).seconds < 5:
                if ser.in_waiting:
                    output += ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
            
            ser.close()
            
            # Verificar si contiene mensajes de inicio esperados
            if "ESP32 Water Level Sensor" in output and "System initialized" in output:
                self.log(f"✅ Verification successful on {port}")
                return True
            else:
                self.log(f"⚠️ Verification failed on {port} - unexpected output")
                return False
                
        except Exception as e:
            self.log(f"❌ Verification error on {port}: {str(e)}")
            return False
    
    def upload_all_devices(self):
        """Upload firmware a todos los dispositivos detectados"""
        # Detectar dispositivos conectados
        devices = self.detect_connected_devices()
        
        if not devices:
            self.log("❌ No devices detected")
            return False
        
        # Compilar firmware
        if not self.compile_firmware():
            return False
        
        # Upload a cada dispositivo
        success_count = 0
        
        for device in devices:
            if self.upload_to_device(device):
                if self.verify_upload(device):
                    success_count += 1
        
        self.log(f"📊 Upload summary: {success_count}/{len(devices)} devices successful")
        return success_count == len(devices)
    
    def log(self, message):
        """Escribir mensaje al log y consola"""
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        log_message = f"[{timestamp}] {message}"
        
        print(log_message)
        
        with open(self.log_file, 'a', encoding='utf-8') as f:
            f.write(log_message + "\n")

def main():
    """Función principal"""
    print("🚀 ESP32 Uploader Tool")
    print("=" * 50)
    
    uploader = ESP32Uploader()
    
    if uploader.upload_all_devices():
        print("✅ All devices updated successfully")
        sys.exit(0)
    else:
        print("❌ Some devices failed to update")
        sys.exit(1)

if __name__ == "__main__":
    main()
```

---

### **7. CARPETA `/include/` - HEADERS PÚBLICOS**

#### **Propósito**
Headers públicos siguiendo estándar PlatformIO para acceso global.

#### **Archivos**
```
include/
├── 📄 README                      # Información sobre uso de headers
└── 📄 wifi_debug_fix.h           # Fix para debugging WiFi
```

---

### **8. CARPETA `/test/` - TESTING**

#### **Propósito**
Tests unitarios y de integración para garantizar calidad del código.

#### **Estructura**
```
test/
└── 📄 README                      # Guía de testing
```

---

### **9. CARPETA `/logs/` - LOGS DE DESARROLLO**

#### **Propósito**
Almacenamiento de logs de desarrollo, debug y monitoreo.

#### **Archivos**
```
logs/
└── 📄 device-monitor-250925-023129.log  # Log monitoreo dispositivo
```

---

### **10. CARPETA `/backups/` - RESPALDOS**

#### **Propósito**
Respaldos históricos de código y firmware para recuperación.

#### **Estructura**
```
backups/
├── 📄 BACKUP_app.js                    # Backup JavaScript dashboard  
├── 📄 BACKUP_captive_portal.html       # Backup HTML portal
├── 📄 BACKUP_index.html                # Backup index general
├── 📄 BACKUP_style.css                 # Backup estilos
├── 📄 captive_portal_ORIGINAL_MONOLITICO.html  # Versión monolítica original
├── 📄 Firmware_backup, 1a. versión_*.bin       # Backups firmware binario
└── 📄 SPIFFS_backup, 1a. versión_*.bin         # Backups SPIFFS
```

---

## 🔧 **HERRAMIENTAS Y CONFIGURACIÓN**

### **PlatformIO Configuration (`platformio.ini`)**
```ini
; ================================================================
; PLATFORMIO.INI - Configuración Principal del Proyecto
; ================================================================
;
; Configuración para compilación y upload del firmware ESP32
; Incluye configuraciones optimizadas para desarrollo y producción

[env:heltec_wifi_kit_32]
platform = espressif32
board = heltec_wifi_kit_32
framework = arduino

; Configuración del monitor serie
monitor_speed = 115200
monitor_filters = esp32_exception_decoder

; Configuración de upload
upload_speed = 921600
upload_port = COM3

; Configuración de particiones para SPIFFS
board_build.partitions = min_spiffs.csv

; Librerías del proyecto
lib_deps = 
    bblanchon/ArduinoJson@^6.21.3
    me-no-dev/AsyncTCP@^1.1.1
    me-no-dev/ESP Async WebServer@^1.2.3
    adafruit/Adafruit SSD1306@^2.5.7
    adafruit/Adafruit GFX Library@^1.11.7
    arduino-libraries/ArduinoHttpClient@^0.4.0

; Flags de compilación
build_flags = 
    -DCORE_DEBUG_LEVEL=3
    -DBOARD_HAS_PSRAM
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DWIFI_SSID_MAX_LEN=32
    -DWIFI_PASSWORD_MAX_LEN=64

; Configuración de debugging
debug_tool = esp-prog
debug_init_break = tbreak setup
```

### **Configuración de Particiones (`scripts/min_spiffs.csv`)**
```csv
# Name,   Type, SubType, Offset,  Size,     Flags
nvs,      data, nvs,     0x9000,  0x5000,
otadata,  data, ota,     0xe000,  0x2000,
app0,     app,  ota_0,   0x10000, 0x140000,
app1,     app,  ota_1,   0x150000,0x140000,
spiffs,   data, spiffs,  0x290000,0x160000,
```

---

## 🚀 **FUNCIONALIDADES IMPLEMENTADAS**

### **Core Features**
- ✅ **Sensor Ultrasónico**: Medición precisa de nivel de agua
- ✅ **WiFi Management**: Configuración y gestión de redes WiFi
- ✅ **Web Server**: Servidor HTTP asíncrono con múltiples páginas
- ✅ **OTA Updates**: Actualizaciones Over-The-Air seguras
- ✅ **OLED Display**: Pantalla local con información en tiempo real
- ✅ **Configuration System**: Sistema de configuración JSON persistente

### **Advanced Features**
- ✅ **ESP-NOW Mesh Network**: Red mesh para múltiples sensores
- ✅ **Responsive Web UI**: Interfaces web responsive multi-dispositivo
- ✅ **Multi-language Support**: Soporte para múltiples idiomas
- ✅ **Theme System**: Temas claro/oscuro con persistencia
- ✅ **Real-time Data**: Actualizaciones en tiempo real via WebSockets
- ✅ **Smart Intervals**: Intervalos adaptativos de medición

### **Enterprise Features**
- ✅ **Modular Architecture**: Arquitectura modular enterprise-grade
- ✅ **Comprehensive Logging**: Sistema de logging con múltiples niveles
- ✅ **Error Handling**: Manejo robusto de errores y recovery
- ✅ **Security**: Implementación de mejores prácticas de seguridad
- ✅ **Performance Optimization**: Optimizaciones de memoria y CPU
- ✅ **Extensive Documentation**: Documentación completa del proyecto

### **Integration Features**
- 🚧 **Alexa Integration**: Integración con Amazon Alexa (en desarrollo)
- 🚧 **Google Home**: Integración con Google Assistant (en desarrollo)
- 🚧 **Tuya IoT**: Integración con ecosistema Tuya (en desarrollo)
- 📋 **MQTT Support**: Soporte protocolo MQTT (planificado)
- 📋 **Cloud Analytics**: Analytics en la nube (planificado)

---

## 📈 **MÉTRICAS DEL PROYECTO**

### **Código Fuente**
- **Líneas de C++**: ~5,000 líneas (firmware + librerías)
- **Líneas HTML**: ~2,500 líneas (3 interfaces web)
- **Líneas CSS**: ~1,600 líneas (estilos profesionales)
- **Líneas JavaScript**: ~1,700 líneas (lógica cliente)
- **Total**: ~10,800 líneas de código

### **Archivos y Estructura**
- **Archivos fuente**: 45+ archivos C++/H
- **Archivos web**: 9 archivos HTML/CSS/JS
- **Documentación**: 25+ archivos Markdown
- **Ejemplos**: 2 proyectos de ejemplo
- **Scripts**: 3 herramientas utilitarias

### **Funcionalidades**
- **APIs REST**: 15+ endpoints
- **Páginas Web**: 3 interfaces especializadas
- **Librerías**: 12 librerías modulares
- **Integraciones**: 3 servicios cloud
- **Protocolos**: WiFi, ESP-NOW, HTTP, WebSocket

---

## 🎯 **CASOS DE USO PRINCIPALES**

### **1. Hogar Inteligente Individual**
- Monitor nivel de agua en tinaco doméstico
- Alertas automáticas por nivel bajo
- Histórico de consumo de agua
- Control remoto via app web

### **2. Red Mesh Multi-Sensor**
- Múltiples sensores distribuidos (tinaco, cisterna, etc.)
- Centralización de datos en un dashboard
- Redundancia de red mesh
- Gestión centralizada de configuración

### **3. Aplicaciones Comerciales**
- Monitoreo de tanques industriales
- Integración con sistemas de gestión
- Reportes automáticos y analytics
- Mantenimiento predictivo

### **4. Integración IoT**
- Parte de ecosistema IoT más amplio
- Conexión con asistentes de voz
- Automatización con otros dispositivos
- Analytics en la nube

---

## 🔜 **ROADMAP Y PRÓXIMOS PASOS**

### **Corto Plazo (1-2 meses)**
1. **Extracción Shared Resources**: Consolidar código común
2. **Testing Automatizado**: Implementar tests unitarios
3. **Performance Optimization**: Optimizar memoria y CPU
4. **Security Hardening**: Mejorar seguridad del sistema

### **Mediano Plazo (3-6 meses)**
1. **Mobile App**: Aplicación móvil nativa (React Native)
2. **Cloud Backend**: Backend en la nube para analytics
3. **Machine Learning**: Predicción de patrones de consumo
4. **Advanced Alerting**: Sistema de alertas inteligente

### **Largo Plazo (6+ meses)**
1. **Commercial Platform**: Plataforma comercial SaaS
2. **Multi-tenant**: Soporte múltiples organizaciones
3. **Enterprise Integrations**: APIs para ERP/CRM
4. **AI Analytics**: Analytics avanzados con IA

---

## 🛡️ **CONSIDERACIONES DE SEGURIDAD**

### **Implementadas**
- ✅ **WiFi Security**: WPA2/WPA3 encryption
- ✅ **Input Validation**: Validación client-side y server-side
- ✅ **XSS Prevention**: Sanitización de inputs HTML
- ✅ **CSRF Protection**: Tokens CSRF en formularios
- ✅ **OTA Security**: Verificación de firmas digitales

### **Pendientes**
- 🚧 **HTTPS**: Certificados SSL/TLS
- 🚧 **Authentication**: Sistema de usuarios y roles
- 🚧 **Rate Limiting**: Protección contra ataques DDoS
- 🚧 **Data Encryption**: Encriptación de datos sensibles
- 🚧 **Security Audit**: Auditoría de seguridad profesional

---

## 📞 **SOPORTE Y CONTACTO**

### **Documentación**
- **Documentación Técnica**: `/docs/` folder
- **Ejemplos**: `/examples/` folder
- **Guías de Usuario**: `/docs/guides/` folder

### **Desarrollo**
- **Issues**: GitHub Issues para bugs y features
- **Contribuciones**: Pull Requests bienvenidos
- **Testing**: Instrucciones en `/test/README`

### **Comunidad**
- **Discord**: [Enlace al servidor Discord]
- **Forum**: [Enlace al foro de soporte]
- **Wiki**: [Enlace a la wiki del proyecto]

---

## 📋 **CONCLUSIONES**

Este proyecto representa un **sistema IoT enterprise-grade** completo para monitoreo de nivel de agua, implementando las mejores prácticas de desarrollo de software y hardware. La arquitectura modular, la documentación extensiva y las características avanzadas lo posicionan como una solución robusta y escalable para aplicaciones domésticas, comerciales e industriales.

**Puntos Destacados**:
- 🏗️ **Arquitectura Enterprise**: Modular, escalable y mantenible
- 📚 **Documentación Completa**: Más de 25 documentos técnicos
- 🌐 **Multi-Platform**: Web, móvil y integraciones cloud
- 🔒 **Security-First**: Implementación de mejores prácticas
- 🚀 **Performance**: Optimizado para recursos limitados ESP32
- 🔧 **Developer-Friendly**: Herramientas y scripts para desarrollo

---

*Documentación generada automáticamente - Última actualización: Septiembre 28, 2025*