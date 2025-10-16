# 📝 Catálogo de Mensajes del Display OLED

## 📋 Contenido
1. [Mensajes de Arranque](#mensajes-de-arranque)
2. [Mensajes de Modo Portal](#mensajes-de-modo-portal)
3. [Mensajes de Modo Sensor](#mensajes-de-modo-sensor)
4. [Mensajes de Estado](#mensajes-de-estado)
5. [Mensajes de Error](#mensajes-de-error)
6. [Índice de Búsqueda](#índice-de-búsqueda)

---

## Mensajes de Arranque

### MSG-001: Splash Screen
**Código**: `showSplashScreen()`  
**Duración**: 3 segundos  
**Frecuencia**: Solo al encender/reiniciar

```
┌──────────────────────────────┐
│                              │
│      SENSOR NIVEL            │  Arial 16, CENTER
│       DE AGUA                │  Arial 16, CENTER
│                              │
│   v2.0 - DataTech            │  Arial 10, CENTER
│                              │
└──────────────────────────────┘
```

**Código fuente**:
```cpp
void DisplayManager::showSplashScreen() {
    display->clear();
    display->setFont(ArialMT_Plain_16);
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->drawString(64, 10, "SENSOR NIVEL");
    display->drawString(64, 26, "DE AGUA");
    display->setFont(ArialMT_Plain_10);
    display->drawString(64, 45, "v2.0 - DataTech");
    display->display();
}
```

**LED**: Encendido fijo (255 PWM)

---

## Mensajes de Modo Portal

### MSG-100: Portal Cautivo - Información de Conexión
**Código**: `showPortalInfo()`  
**Condiciones**: WiFi no configurado o Double Reset detectado  
**Auto-sleep**: ❌ Deshabilitado

```
┌──────────────────────────────────────────────┐
│ PORTAL CAUTIVO                              │  Arial 10, LEFT
│ Red: ESP32: Sensor de nivel de agua        │  Arial 10, LEFT
│ Pass: 12345678                              │  Arial 10, LEFT
│ IP: 192.168.1.1                             │  Arial 10, LEFT
│                                             │
│ MAC: XX:XX:XX:XX:XX:XX                      │  Arial 10, LEFT
│ Clientes: 2                                 │  Arial 10, LEFT
└──────────────────────────────────────────────┘
```

**Variables dinámicas**:
- `SSID`: `"ESP32: Sensor de nivel de agua"` (configurable en `config.json` → `ap_ssid`)
- `Password`: Configurada en `config.json` → `ap_password` (default: "12345678")
- `IP`: Siempre `192.168.1.1` (configurada con `WiFi.softAPConfig()`)
- `MAC`: `WiFi.softAPmacAddress()`
- `Clientes`: Número de clientes conectados al AP (`WiFi.softAPgetStationNum()`)

**Nota**: La API de ESP32 no proporciona fácilmente las IPs de clientes conectados al AP, solo el conteo.

**Código fuente**:
```cpp
void DisplayManager::showPortalInfo() {
    display->clear();
    display->setFont(ArialMT_Plain_10);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    
    display->drawString(0, 0, "PORTAL CAUTIVO");
    display->drawString(0, 12, "Red: " + String(WIFI_SSID_AP));
    display->drawString(0, 24, "Pass: " + String(WIFI_PASSWORD_AP));
    display->drawString(0, 36, "IP: 192.168.1.1");
    
    String mac = WiFi.softAPmacAddress();
    display->drawString(0, 48, "MAC: " + mac);
    
    // Clientes conectados
    int numClients = WiFi.softAPgetStationNum();
    String clientsInfo = "Clientes: " + String(numClients);
    display->drawString(0, 60, clientsInfo);
    
    display->display();
}
```

**LED**: Encendido fijo o fade (según configuración)  
**Frecuencia**: Actualiza cada 2-3 segundos (para refrescar conteo de clientes)

---

## Mensajes de Modo Sensor

### MSG-200: Sensor - Lectura Normal
**Código**: `updateDisplay()` en AppManager  
**Condiciones**: Sensor OK, WiFi OK  
**Auto-sleep**: ✅ Activo (30s default)

```
┌──────────────────────────────┐
│ SENSOR DE NIVEL              │  Arial 10, LEFT (título)
│ Dist: 45.2 cm                │  Arial 10, LEFT
│ Agua: 125.8 L                │  Arial 10, LEFT
│ Lleno: 75%                   │  Arial 10, LEFT
│                              │
│ WiFi: MiSSID                 │  Arial 10, LEFT
└──────────────────────────────┘
```

**Variables dinámicas**:
- `Dist`: Distancia del sensor (cm) - `sensor_distance`
- `Agua`: Volumen calculado (L) - `current_volume`
- `Lleno`: Porcentaje de llenado - `fill_percentage`
- `WiFi`: SSID conectado - `WiFi.SSID()`

**Código fuente** (AppManager.cpp):
```cpp
void AppManager::updateSensorDisplay() {
    if (display_manager.isDisplaySleeping()) return;
    
    display_manager.display->clear();
    display_manager.display->setFont(ArialMT_Plain_10);
    display_manager.display->setTextAlignment(TEXT_ALIGN_LEFT);
    
    // Título
    display_manager.display->drawString(0, 0, "SENSOR DE NIVEL");
    
    // Datos del sensor
    display_manager.display->drawString(0, 15, "Dist: " + String(sensor_distance, 1) + " cm");
    display_manager.display->drawString(0, 27, "Agua: " + String(current_volume, 1) + " L");
    display_manager.display->drawString(0, 39, "Lleno: " + String(fill_percentage, 0) + "%");
    
    // WiFi
    if (WiFi.isConnected()) {
        display_manager.display->drawString(0, 54, "WiFi: " + WiFi.SSID());
    } else {
        display_manager.display->drawString(0, 54, "WiFi: Sin conexión");
    }
    
    display_manager.display->display();
}
```

**LED**: Encendido fijo (255 PWM)  
**Frecuencia**: Actualiza cada 1-2 segundos

### MSG-201: Sensor - Sin WiFi
**Condiciones**: Sensor OK, WiFi desconectado  
**Auto-sleep**: ✅ Activo

```
┌──────────────────────────────┐
│ SENSOR DE NIVEL              │
│ Dist: 45.2 cm                │
│ Agua: 125.8 L                │
│ Lleno: 75%                   │
│                              │
│ WiFi: Sin conexión           │  ← Mensaje diferente
└──────────────────────────────┘
```

**Código**:
```cpp
if (WiFi.isConnected()) {
    display_manager.display->drawString(0, 54, "WiFi: " + WiFi.SSID());
} else {
    display_manager.display->drawString(0, 54, "WiFi: Sin conexión");
}
```

### MSG-202: Sensor - Error de Lectura
**Condiciones**: Sensor no responde o lectura inválida  
**Auto-sleep**: ✅ Activo

```
┌──────────────────────────────┐
│ SENSOR DE NIVEL              │
│ ⚠️ SENSOR ERROR              │  Arial 10, LEFT
│ Verificar conexión           │  Arial 10, LEFT
│ Trig/Echo pins               │  Arial 10, LEFT
│                              │
│ WiFi: MiSSID                 │
└──────────────────────────────┘
```

**Código fuente**:
```cpp
void AppManager::showSensorError() {
    display_manager.display->clear();
    display_manager.display->setFont(ArialMT_Plain_10);
    display_manager.display->setTextAlignment(TEXT_ALIGN_LEFT);
    
    display_manager.display->drawString(0, 0, "SENSOR DE NIVEL");
    display_manager.display->drawString(0, 15, "⚠️ SENSOR ERROR");
    display_manager.display->drawString(0, 27, "Verificar conexión");
    display_manager.display->drawString(0, 39, "Trig/Echo pins");
    
    if (WiFi.isConnected()) {
        display_manager.display->drawString(0, 54, "WiFi: " + WiFi.SSID());
    } else {
        display_manager.display->drawString(0, 54, "WiFi: Sin conexión");
    }
    
    display_manager.display->display();
}
```

**LED**: Encendido fijo (255 PWM)  
**Frecuencia**: Actualiza cuando se detecta error

### MSG-203: Sensor - Datos ESP-NOW
**Condiciones**: Recibiendo datos de sensores remotos  
**Auto-sleep**: ✅ Activo

```
┌──────────────────────────────┐
│ ESP-NOW ACTIVO               │  Arial 10, LEFT
│ Sensores: 2                  │  Arial 10, LEFT
│ Sensor 1: 23.5°C             │  Arial 10, LEFT
│ Sensor 2: 45.2 cm            │  Arial 10, LEFT
│                              │
│ WiFi: MiSSID                 │
└──────────────────────────────┘
```

**Nota**: Implementación específica depende de datos recibidos.

---

## Mensajes de Estado

### MSG-300: Auto-Sleep Countdown
**Código**: `checkAutoSleep()` en DisplayManager  
**Condiciones**: 30s sin actividad (solo en modo sensor)  
**Duración**: 3 segundos

```
┌──────────────────────────────┐
│                              │
│                              │
│    Apagando pantalla         │  Arial 16, CENTER
│                              │
│         en 2...              │  Arial 16, CENTER
│                              │
└──────────────────────────────┘
```

**Secuencia**:
- Segundo 1: `"en 3..."`
- Segundo 2: `"en 2..."`
- Segundo 3: `"en 1..."`
- Después: Display OFF

**Código fuente**:
```cpp
void DisplayManager::checkAutoSleep() {
    // ... (verificaciones previas)
    
    if (_countdownActive) {
        unsigned long elapsed = (millis() - _countdownStart) / 1000;
        int remaining = 3 - elapsed;
        
        if (remaining != _countdownSeconds) {
            _countdownSeconds = remaining;
            
            if (_countdownSeconds > 0) {
                display->clear();
                display->setFont(ArialMT_Plain_16);
                display->setTextAlignment(TEXT_ALIGN_CENTER);
                display->drawString(64, 15, "Apagando pantalla");
                display->drawString(64, 35, "en " + String(_countdownSeconds) + "...");
                display->display();
            } else {
                displayOff();
                _displaySleeping = true;
            }
        }
    }
}
```

**LED**: Encendido fijo durante countdown, OFF al finalizar

### MSG-301: Display Sleeping
**Código**: `displayOff()`  
**Condiciones**: Countdown completado o comando web/manual  
**Duración**: Indefinida (hasta despertar)

```
┌──────────────────────────────┐
│                              │
│                              │  ← Pantalla completamente
│          (NEGRO)             │     negra (sin contenido)
│                              │
│                              │
│                              │
└──────────────────────────────┘
```

**Código fuente**:
```cpp
void DisplayManager::displayOff() {
    display->displayOff();
    Vext_OFF();  // Apagar alimentación OLED
    
    // Apagar LED
    ledcWrite(LEDCHANNEL, 0);
    _ledFadeActive = false;
}
```

**LED**: OFF (0 PWM)  
**Consumo**: ~5mA (vs ~50mA encendido)

### MSG-302: Display Waking Up
**Código**: `wakeUpDisplay()`  
**Trigger**: Botón PRG o comando web ON  
**Duración**: Instantáneo

```
┌──────────────────────────────┐
│                              │  ← Display se enciende
│  (Vuelve a mostrar           │     y muestra último
│   último estado conocido)    │     contenido
│                              │
└──────────────────────────────┘
```

**Código fuente**:
```cpp
void DisplayManager::wakeUpDisplay() {
    if (_displaySleeping) {
        displayOn();
        _displaySleeping = false;
        _countdownActive = false;
        _countdownSeconds = 0;
        _lastActivity = millis();
        
        // Encender LED
        ledcWrite(LEDCHANNEL, 255);
    }
}
```

**LED**: Encendido fijo (255 PWM)  
**Comportamiento**: Restaura último mensaje mostrado

---

## Mensajes de Error

### MSG-400: WiFi - Error de Conexión
**Código**: Dentro de `tryConnectWiFi()` en AppManager  
**Condiciones**: Timeout o error al conectar WiFi

```
┌──────────────────────────────┐
│ ERROR WIFI                   │  Arial 10, LEFT
│                              │
│ No se pudo conectar          │  Arial 10, CENTER
│ a la red configurada         │  Arial 10, CENTER
│                              │
│ Entrando a Portal...         │  Arial 10, CENTER
└──────────────────────────────┘
```

**Duración**: 2-3 segundos antes de cambiar a Portal Mode

### MSG-401: Sensor - Timeout
**Condiciones**: Sensor no responde después de múltiples intentos

```
┌──────────────────────────────┐
│ SENSOR DE NIVEL              │
│ ⚠️ TIMEOUT                   │  Arial 10, LEFT
│ Sensor no responde           │  Arial 10, LEFT
│ Verificar alimentación       │  Arial 10, LEFT
│                              │
│ WiFi: MiSSID                 │
└──────────────────────────────┘
```

### MSG-402: Configuración - Error JSON
**Condiciones**: Error al cargar/guardar config.json

```
┌──────────────────────────────┐
│ ERROR CONFIG                 │  Arial 10, LEFT
│                              │
│ No se pudo cargar            │  Arial 10, CENTER
│ configuración                │  Arial 10, CENTER
│ Usando valores default       │  Arial 10, CENTER
│                              │
└──────────────────────────────┘
```

**Duración**: 3 segundos, luego pasa a modo normal

### MSG-403: OTA - Actualización
**Condiciones**: Actualización OTA en progreso

```
┌──────────────────────────────┐
│ ACTUALIZACIÓN OTA            │  Arial 10, CENTER
│                              │
│    Progreso: 45%             │  Arial 16, CENTER
│                              │
│ ⚠️ NO APAGAR                 │  Arial 10, CENTER
│                              │
└──────────────────────────────┘
```

**Auto-sleep**: ❌ Deshabilitado durante OTA  
**LED**: Fade activo

---

## Índice de Búsqueda

### Por Código de Mensaje
- MSG-001: Splash Screen
- MSG-100: Portal Cautivo - Info (con clientes conectados)
- MSG-200: Sensor - Lectura Normal
- MSG-201: Sensor - Sin WiFi
- MSG-202: Sensor - Error de Lectura
- MSG-203: Sensor - Datos ESP-NOW
- MSG-300: Auto-Sleep Countdown
- MSG-301: Display Sleeping
- MSG-302: Display Waking Up
- MSG-400: WiFi - Error de Conexión
- MSG-401: Sensor - Timeout
- MSG-402: Configuración - Error JSON
- MSG-403: OTA - Actualización

### Por Función en Código
- `showSplashScreen()` → MSG-001
- `showPortalInfo()` → MSG-100
- `updateSensorDisplay()` → MSG-200, MSG-201
- `showSensorError()` → MSG-202
- `checkAutoSleep()` → MSG-300
- `displayOff()` → MSG-301
- `wakeUpDisplay()` → MSG-302

### Por Condición
- **Al encender**: MSG-001
- **Sin WiFi configurado**: MSG-100
- **WiFi OK + Sensor OK**: MSG-200
- **WiFi desconectado**: MSG-201
- **Sensor error**: MSG-202
- **30s sin actividad**: MSG-300
- **Dormido**: MSG-301
- **Botón PRG/Web ON**: MSG-302
- **Error WiFi**: MSG-400
- **Sensor timeout**: MSG-401

### Por Estado del LED
- **LED Encendido Fijo (255)**: MSG-001, MSG-200, MSG-201, MSG-202, MSG-300, MSG-302
- **LED OFF (0)**: MSG-301
- **LED Fade**: MSG-100 (opcional), MSG-403

### Por Auto-Sleep
- **Auto-Sleep OFF**: MSG-001, MSG-100, MSG-101, MSG-403
- **Auto-Sleep ON**: MSG-200, MSG-201, MSG-202, MSG-203

---

## Tabla Resumen

| Código | Mensaje | Fuente | Duración | Auto-Sleep | LED |
|--------|---------|--------|----------|------------|-----|
| MSG-001 | Splash Screen | `DisplayManager` | 3s | OFF | ON |
| MSG-100 | Portal Info + Clientes | `DisplayManager` | Actualiza 2-3s | OFF | ON/Fade |
| MSG-200 | Sensor Normal | `AppManager` | Continuo | ON | ON |
| MSG-201 | Sin WiFi | `AppManager` | Continuo | ON | ON |
| MSG-202 | Sensor Error | `AppManager` | Continuo | ON | ON |
| MSG-300 | Countdown | `DisplayManager` | 3s | ON | ON |
| MSG-301 | Sleeping | `DisplayManager` | Indefinido | ON | OFF |
| MSG-302 | Waking Up | `DisplayManager` | Instantáneo | ON | ON |
| MSG-400 | WiFi Error | `AppManager` | 2-3s | OFF | ON |

---

**Documentos relacionados:**
- [OLED_DISPLAY_ARCHITECTURE.md](./OLED_DISPLAY_ARCHITECTURE.md) - Arquitectura general
- [OLED_DISPLAY_STATES.md](./OLED_DISPLAY_STATES.md) - Máquina de estados
- [OLED_DISPLAY_TIMING.md](./OLED_DISPLAY_TIMING.md) - Diagramas de tiempo
