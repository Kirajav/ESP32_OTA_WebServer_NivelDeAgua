# 📊 DIAGRAMA COMPLETO: FLUJO PANTALLA OLED ESP32 SENSOR DE AGUA

## 🔋 **INICIO DEL SISTEMA** (Después de Flash + Upload FS)

```
⚡ ESP32 BOOT SEQUENCE
├── 0.0s: Power On / Reset
├── 0.1s: Serial.begin(115200) 
├── 0.2s: app.initialize() llamado
└── 0.3s: === INICIALIZANDO SISTEMA ===
```

---

## 🖥️ **SECUENCIA COMPLETA PANTALLA OLED**

### **FASE 1: INICIALIZACIÓN DISPLAY (0.3s - 3.3s)**

```
📺 PANTALLA OLED - INICIO
├── 0.3s: display_manager.begin()
│   ├── ✅ LED blanco: ENCENDIDO
│   ├── ✅ Vext LOW (OLED encendido)
│   ├── ✅ OLED_RST reset sequence (100ms)
│   ├── ✅ Wire.begin(SDA=4, SCL=15)
│   ├── ✅ Heltec.begin(display=true)
│   └── ✅ Auto-sleep: 30 segundos default
│
├── 0.4s: 🎨 SPLASH SCREEN (3 segundos fijos)
│   ┌─────────────────────────┐
│   │    SENSOR NIVEL        │ ← ArialMT_Plain_16, centrado
│   │      DE AGUA           │
│   │                        │
│   │   v2.0 - DataTech      │ ← ArialMT_Plain_10
│   └─────────────────────────┘
│   
└── 3.4s: showInitialMessage()
    ┌─────────────────────────┐
    │   SENSOR DE             │ ← ArialMT_Plain_16, centrado
    │   NIVEL DE AGUA         │
    │                         │
    │ Iniciando WiFi...       │ ← ArialMT_Plain_10, izq
    └─────────────────────────┘
```

### **FASE 2: DETECCIÓN DOBLE RESET + DECISIÓN MODO (3.4s - 4.0s)**

```
🔄 DETECCIÓN DOBLE RESET
├── 3.4s: drd.detectDoubleReset()
│
├── CASO A: ✅ Reset Normal
│   ├── forcePortalMode = false
│   ├── tryConnectWiFi() llamado
│   └── → CONTINÚA A FASE 3A o 3B
│
└── CASO B: 🔄 Doble Reset Detectado  
    ├── forcePortalMode = true
    ├── 📢 Serial: "DOBLE RESET DETECTADO"
    └── → CONTINÚA A FASE 3B (Portal Forzado)
```

### **FASE 3A: MODO SENSOR NORMAL (4.0s - ∞)**

```
✅ CONEXIÓN WIFI EXITOSA
├── 4.0s: WiFi.begin(ssid, password)
├── 4.5s: Serial: "Conectando..."
├── 6.0s: WiFi.status() == WL_CONNECTED
├── 6.1s: startNormalMode() llamado
│
├── 6.2s: 📺 PANTALLA - INFO SENSOR
│   ┌─────────────────────────┐
│   │ SENSOR ACTIVO           │ ← ArialMT_Plain_10
│   │ WiFi: MiRedWiFi         │ 
│   │ IP: 192.168.1.45        │
│   │ Leyendo sensor...       │
│   └─────────────────────────┘
│
├── 6.3s: ⏰ AUTO-SLEEP HABILITADO
├── 6.4s: 🌐 Web Server iniciado (puerto 80)
├── 6.5s: 🔗 ESP-NOW inicializado
├── 6.6s: 🕐 NTP sincronización iniciada
├── 6.7s: 🏠 IoT integrations (Google/Alexa/Tuya)
│
└── 11.0s: PRIMER CICLO SENSOR (cada 5 segundos)
    ├── ❌ PROBLEMA: sensor_manager.update() 
    │   └── 🚨 NO HAY SENSORES REGISTRADOS
    ├── updateSensorDisplay() llamado
    └── 📺 PANTALLA - DATOS HARDCODEADOS:
        ┌─────────────────────────┐
        │ SENSOR NIVEL AGUA       │
        │ WiFi: MiRedWiFi         │
        │ Nivel: 75.5 cm          │ ← ⚠️ HARDCODED!
        │ Lleno: 65%              │ ← ⚠️ HARDCODED!
        │ Estado: Normal          │ ← ⚠️ HARDCODED!
        └─────────────────────────┘
```

**🔄 CICLO CONTINUO MODO SENSOR:**
```
⏰ CADA 5 SEGUNDOS (11s, 16s, 21s, ...):
├── 🌊 sensor_manager.update() (❌ VACÍO)
├── 📺 updateSensorDisplay() (datos fake)
├── 📡 Envío datos Tuya (si conectado)
└── ⏰ Auto-sleep check

📱 AUTO-SLEEP SEQUENCE (después de 30s inactividad):
├── 27s: Sin actividad desde última interacción
├── 28s: Countdown 3 segundos iniciado
├── 28s: 📺 "Apagando pantalla en 3..."
├── 29s: 📺 "Apagando pantalla en 2..."  
├── 30s: 📺 "Apagando pantalla en 1..."
├── 31s: 📺 displayOff() + LED APAGADO
└── ∞: Pantalla dormida hasta botón PRG o web

🔘 DESPERTAR PANTALLA:
├── Botón PRG presionado → wakeUpDisplay()
├── Web toggle → handleToggleDisplay()
└── 📺 Pantalla encendida + LED + datos sensor
```

### **FASE 3B: MODO PORTAL CAUTIVO (4.0s - ∞)**

```
🌐 PORTAL CAUTIVO ACTIVADO
├── 4.0s: startPortalMode() llamado
├── 4.1s: WiFi.mode(WIFI_AP)
├── 4.2s: WiFi.softAP("SmartConfig_A1B2C3", "12345678")
├── 4.3s: apCallback() ejecutado
│
├── 4.4s: 📺 PANTALLA - PORTAL INFO
│   ┌─────────────────────────┐
│   │ PORTAL CAUTIVO          │ ← ArialMT_Plain_10
│   │ Red: SmartConfig_A1B2C3 │
│   │ Contraseña: 12345678    │
│   │ IP: 192.168.1.1         │
│   │ MAC: 24:6F:28:B2:A5:C8  │
│   └─────────────────────────┘
│
├── 4.5s: 🌐 DNS Server iniciado (puerto 53)
├── 4.6s: 🌐 Web Server iniciado (puerto 80)
├── 4.7s: ⏰ AUTO-SLEEP DESHABILITADO
└── 4.8s: 📢 "Portal disponible en: http://192.168.1.1"
```

**🔄 CICLO CONTINUO MODO PORTAL:**
```
♾️ LOOP INFINITO:
├── 📡 dnsServer.processNextRequest()
├── 🌐 Web requests handling
├── 📱 Portal cautivo responses
├── ⚠️ NO sensor reading
├── ⚠️ NO auto-sleep
└── 📺 Pantalla SIEMPRE encendida
```

---

## 🚨 **PROBLEMAS DETECTADOS:**

### **1. 🔴 CRÍTICO: Sensor no inicializado**
```cpp
// ❌ PROBLEMA en AppManager::initialize()
// sensor_manager está declarado pero VACÍO
sensor_manager.update(); // No hace nada - no hay sensores

// ❌ FALTA esta inicialización:
WaterLevelSensor* waterSensor = new WaterLevelSensor(trigPin, echoPin, &config_manager);
sensor_manager.addSensor(waterSensor);
sensor_manager.begin();
```

### **2. 🟡 MEDIO: Display initialization confusa**
```cpp
// ❌ PROBLEMA: Display se inicializa ANTES de saber el modo
display_manager.begin();  // ← Siempre igual
// ↓ 3 segundos después decide modo
if (tryConnectWiFi()) { /* sensor */ } else { /* portal */ }

// ✅ MEJOR: Inicializar display específico por modo
```

### **3. 🟡 MEDIO: Auto-sleep inconsistente**
```cpp
// ❌ CONFUSO: Auto-sleep se habilita/deshabilita múltiples veces
display_manager.enableAutoSleep(true);   // Al conectar WiFi
display_manager.enableAutoSleep(false);  // En portal mode
// Sin indicación visual del estado actual
```

### **4. 🟠 MENOR: Tiempos hardcodeados**
```cpp
delay(3000);        // Splash screen - no configurable
delay(500);         // WiFi connection - puede ser lento
// No hay timeouts configurables por usuario
```

### **5. 🟠 MENOR: Datos hardcodeados**
```cpp
// ❌ En updateSensorDisplay():
float waterLevel = 75.5;  // ← Siempre lo mismo
int percentage = 65;      // ← Nunca cambia
String status = "Normal"; // ← Fake data
```

---

## 🎯 **EXPERIENCIA DE USUARIO ESPERADA vs REAL:**

### **✅ FUNCIONAMIENTO ESPERADO:**
```
1. Flash firmware → 
2. Splash 3s → 
3. "Conectando WiFi..." → 
4. SI conecta: Datos reales del sensor cada 5s
5. SI no conecta: Portal cautivo para configurar
6. Auto-sleep en 30s (modo sensor)
7. Despertar con botón o web
```

### **❌ FUNCIONAMIENTO REAL:**
```
1. Flash firmware → ✅
2. Splash 3s → ✅  
3. "Conectando WiFi..." → ✅
4. SI conecta: DATOS FALSOS cada 5s ❌
5. SI no conecta: Portal cautivo OK ✅
6. Auto-sleep funcional ✅
7. Despertar funcional ✅
```

---

## 🛠️ **FIXES NECESARIOS URGENTES:**

### **1. 🚨 CRÍTICO - Inicializar Sensor Real:**
```cpp
// En AppManager::initialize(), después de config_manager.begin():
Serial.println("🌊 Inicializando sensores...");

// Configurar pines desde configuración
uint8_t trigPin = config_manager.getHardwareBoardConfig().getTriggerPin();
uint8_t echoPin = config_manager.getHardwareBoardConfig().getEchoPin();

// Crear y registrar sensor
WaterLevelSensor* waterSensor = new WaterLevelSensor(trigPin, echoPin, &config_manager);
sensor_manager.addSensor(waterSensor);
sensor_manager.begin();

Serial.println("✅ Sensor de agua inicializado");
```

### **2. 🟡 MEDIO - Mejorar updateSensorDisplay():**
```cpp
void AppManager::updateSensorDisplay() {
    if (portal_active || display_manager.isDisplaySleeping()) return;
    
    // Obtener datos REALES del sensor
    String sensorJson = sensor_manager.getSensorJson("WaterLevel");
    if (sensorJson.contains("error")) {
        // Mostrar error en pantalla
        display_manager.clear();
        display_manager.drawString(0, 0, "SENSOR ERROR");
        display_manager.drawString(0, 12, "Verificar conexión");
        display_manager.display();
        return;
    }
    
    // Parsear datos reales
    DynamicJsonDocument doc(256);
    deserializeJson(doc, sensorJson);
    
    float distance = doc["distancia"] | -1.0;
    float litros = doc["litros"] | 0.0;
    int percentage = doc["porcentaje"] | 0;
    
    // Mostrar datos reales
    display_manager.clear();
    display_manager.drawString(0, 0, "SENSOR NIVEL AGUA");
    display_manager.drawString(0, 12, "Dist: " + String(distance, 1) + " cm");
    display_manager.drawString(0, 24, "Agua: " + String(litros, 1) + " L");
    display_manager.drawString(0, 36, "Lleno: " + String(percentage) + "%");
    
    // Indicador auto-sleep
    if (display_manager.getAutoSleepTime() > 0) {
        display_manager.drawString(0, 48, "Sleep: " + String(display_manager.getAutoSleepTime()) + "s");
    }
    
    display_manager.display();
}
```

### **3. 🟠 MENOR - Indicador visual modo/estado:**
```cpp
// En las pantallas, agregar indicadores:
"[M]" para Master ESP-NOW
"[S]" para Slave ESP-NOW  
"[WiFi]" para conectado
"[AP]" para modo portal
"💤" para auto-sleep habilitado
```

**¿Quieres que implemente estos fixes críticos ahora?** El más importante es inicializar el sensor real, porque actualmente el sistema muestra datos falsos.