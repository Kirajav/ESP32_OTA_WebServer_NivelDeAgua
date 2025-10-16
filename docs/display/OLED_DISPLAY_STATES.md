# 🔄 Máquina de Estados del Display OLED

## 📋 Contenido
1. [Diagrama General de Estados](#diagrama-general-de-estados)
2. [Estados Detallados](#estados-detallados)
3. [Transiciones](#transiciones)
4. [Casos Especiales](#casos-especiales)

---

## Diagrama General de Estados

```mermaid
stateDiagram-v2
    [*] --> POWER_ON: ESP32 Enciende
    
    POWER_ON --> SPLASH: begin()
    SPLASH --> INIT_WIFI: 3 segundos
    
    INIT_WIFI --> PORTAL_MODE: WiFi no configurado<br/>o Double Reset
    INIT_WIFI --> SENSOR_MODE: WiFi configurado OK
    
    PORTAL_MODE --> PORTAL_DISPLAY: showPortalInfo()
    PORTAL_DISPLAY --> PORTAL_DISPLAY: Loop infinito<br/>(sin auto-sleep)
    PORTAL_DISPLAY --> SENSOR_MODE: WiFi configurado<br/>exitosamente
    
    SENSOR_MODE --> SENSOR_ACTIVE: enableAutoSleep(true)
    
    SENSOR_ACTIVE --> SENSOR_READING: Lecturas OK
    SENSOR_ACTIVE --> SENSOR_ERROR: Error de lectura
    
    SENSOR_READING --> AUTO_SLEEP_COUNTDOWN: 30s sin actividad
    SENSOR_ERROR --> AUTO_SLEEP_COUNTDOWN: 30s sin actividad
    
    AUTO_SLEEP_COUNTDOWN --> DISPLAY_SLEEPING: Countdown completo
    
    DISPLAY_SLEEPING --> SENSOR_ACTIVE: Botón PRG<br/>o Web ON
    
    SENSOR_ACTIVE --> DISPLAY_SLEEPING: Web OFF
    
    note right of PORTAL_DISPLAY
        Display SIEMPRE ON
        LED FIJO o FADE
        Auto-sleep OFF
    end note
    
    note right of SENSOR_READING
        Auto-sleep ON
        Actualiza datos
        LED ON fijo
    end note
    
    note right of DISPLAY_SLEEPING
        Display OFF
        LED OFF
        Ahorro energía
    end note
```

---

## Estados Detallados

### 1. POWER_ON (Encendido)
**Duración**: ~500ms  
**Hardware**:
- Vext (GPIO 21) → LOW (encender OLED)
- OLED_RST (GPIO 16) → Toggle (reset display)
- I2C inicializado (SDA=4, SCL=15)

**Secuencia**:
```mermaid
sequenceDiagram
    participant ESP32
    participant Display
    participant LED
    
    ESP32->>Display: pinMode(Vext, OUTPUT)
    ESP32->>Display: digitalWrite(Vext, LOW)
    Note over Display: Esperar 100ms
    ESP32->>Display: pinMode(OLED_RST, OUTPUT)
    ESP32->>Display: digitalWrite(OLED_RST, LOW)
    Note over Display: Esperar 50ms
    ESP32->>Display: digitalWrite(OLED_RST, HIGH)
    Note over Display: Esperar 50ms
    ESP32->>Display: Wire.begin(SDA=4, SCL=15)
    ESP32->>Display: Heltec.display->init()
    ESP32->>Display: flipScreenVertically()
    ESP32->>LED: ledcSetup() + ledcAttachPin()
```

### 2. SPLASH (Pantalla de Bienvenida)
**Duración**: 3 segundos exactos  
**Contenido**:
```
┌────────────────────┐
│   SENSOR NIVEL    │  ← Arial 16
│     DE AGUA       │  ← Arial 16
│                   │
│ v2.0 - DataTech   │  ← Arial 10
└────────────────────┘
```

**Código relevante**:
```cpp
void showSplashScreen() {
    display->clear();
    display->setFont(ArialMT_Plain_16);
    display->setTextAlignment(TEXT_ALIGN_CENTER);
    display->drawString(64, 10, "SENSOR NIVEL");
    display->drawString(64, 26, "DE AGUA");
    display->setFont(ArialMT_Plain_10);
    display->drawString(64, 45, "v2.0 - DataTech");
    display->display();
}
// En begin():
showSplashScreen();
delay(3000);  // ← Bloquea 3 segundos
```

### 3. INIT_WIFI (Inicializando WiFi)
**Duración**: Variable (depende de conexión)  
**Contenido**:
```
┌────────────────────┐
│  SENSOR DE        │  ← Arial 16
│  NIVEL DE AGUA    │  ← Arial 16
│                   │
│ Iniciando WiFi... │  ← Arial 10
└────────────────────┘
```

**Decisión de modo**:
```mermaid
graph TD
    A[INIT_WIFI] --> B{Double Reset<br/>detectado?}
    B -->|SI| C[PORTAL_MODE]
    B -->|NO| D{WiFi<br/>configurado?}
    D -->|NO| C
    D -->|SI| E{Conexión<br/>exitosa?}
    E -->|SI| F[SENSOR_MODE]
    E -->|NO| C
    
    style C fill:#f99
    style F fill:#9f9
```

### 4. PORTAL_MODE (Modo Portal Cautivo)
**Estado**: Persistente hasta configuración exitosa  
**Auto-sleep**: ❌ DESHABILITADO  
**LED**: Encendido fijo o fade (según preferencia)

**Contenido del display**:
```
┌──────────────────────────────────────────────┐
│ PORTAL CAUTIVO                              │  ← Arial 10
│ Red: ESP32: Sensor de nivel de agua        │
│ Contraseña: 12345678                        │
│ IP: 192.168.1.1                             │
│                                             │
│ MAC: XX:XX:XX:XX:XX:XX                      │
│ Clientes: 0                                 │  ← Dinámico
└──────────────────────────────────────────────┘
```

**Notas importantes**:
- **SSID**: `"ESP32: Sensor de nivel de agua"` (configurable en JSON)
- **IP**: `192.168.1.1` (configurada con `WiFi.softAPConfig()`)
- **Password**: Configurable (default: "12345678")
- **Portal Cautivo**: DNS server redirige todas las peticiones a 192.168.1.1
- **Double Reset**: Si se detecta, carga último WiFi del JSON y reintenta conexión
- **Clientes conectados**: Muestra número de clientes actualmente conectados al AP

**Diagrama de flujo**:
```mermaid
flowchart TD
    A[Entrar PORTAL_MODE] --> B["WiFi.softAP()<br/>IP: 192.168.1.1"]
    B --> C[Mostrar info AP]
    C --> D[enableAutoSleep false]
    D --> E[LED ON o Fade]
    E --> F{Usuario conecta<br/>y configura WiFi?}
    F -->|NO| F
    F -->|SI| G[Guardar credenciales<br/>en config.json]
    G --> H[Reintentar conexión WiFi]
    H --> I{Conexión<br/>exitosa?}
    I -->|NO| F
    I -->|SI| J[SENSOR_MODE]
    
    style F fill:#ff9
    style J fill:#9f9
```

### 5. SENSOR_MODE (Modo Sensor Activo)
**Auto-sleep**: ✅ HABILITADO (30 segundos default)  
**LED**: Encendido fijo (255 PWM)

**Sub-estados**:

#### 5.1 SENSOR_READING (Lectura Normal)
**Contenido**:
```
┌──────────────────────────┐
│ SENSOR DE NIVEL         │  ← Arial 10 (título)
│ Dist: 45.2 cm           │
│ Agua: 125.8 L           │
│ Lleno: 75%              │
│                         │
│ WiFi: MiSSID            │  ← (o "Sin WiFi")
└──────────────────────────┘
```

**Condiciones**:
- ✅ Sensor respondiendo
- ✅ Distancia < 400cm
- ✅ JSON válido

#### 5.2 SENSOR_ERROR (Error de Lectura)
**Contenido**:
```
┌──────────────────────────┐
│ SENSOR DE NIVEL         │
│ ⚠️ SENSOR ERROR         │
│ Verificar conexión      │
│ Trig/Echo pins          │
│                         │
│ WiFi: MiSSID            │
└──────────────────────────┘
```

**Condiciones**:
- ❌ Sensor no responde
- ❌ Distancia >= 400cm
- ❌ Timeout de lectura

### 6. AUTO_SLEEP_COUNTDOWN (Countdown de Apagado)
**Duración**: 3 segundos  
**Trigger**: 30 segundos sin actividad (configurable)

**Animación**:
```mermaid
sequenceDiagram
    participant Timer
    participant Display
    participant User
    
    Note over Timer: 30 segundos transcurridos
    Timer->>Display: Iniciar countdown
    Display->>Display: _countdownActive = true
    Display->>Display: _countdownStart = millis()
    
    loop Cada segundo
        Display->>User: "Apagando pantalla"
        Display->>User: "en 3..." (segundo 1)
        Note over Display: Esperar 1s
        Display->>User: "en 2..." (segundo 2)
        Note over Display: Esperar 1s
        Display->>User: "en 1..." (segundo 3)
        Note over Display: Esperar 1s
    end
    
    Display->>Display: displayOff()
    Display->>Timer: _displaySleeping = true
```

**Pantalla durante countdown**:
```
┌──────────────────────────┐
│                         │
│  Apagando pantalla      │  ← Arial 16
│                         │
│    en 2...              │  ← Arial 16
│                         │
└──────────────────────────┘
```

**Código**:
```cpp
void checkAutoSleep() {
    if (!_autoSleepEnabled || _displaySleeping) return;
    
    unsigned long timeSinceActivity = millis() - _lastActivity;
    
    if (_countdownActive) {
        unsigned long elapsed = (millis() - _countdownStart) / 1000;
        int remaining = 3 - elapsed;
        
        if (remaining != _countdownSeconds) {
            _countdownSeconds = remaining;
            
            if (_countdownSeconds > 0) {
                // Mostrar countdown
                display->clear();
                display->setFont(ArialMT_Plain_16);
                display->setTextAlignment(TEXT_ALIGN_CENTER);
                display->drawString(64, 15, "Apagando pantalla");
                display->drawString(64, 35, "en " + String(_countdownSeconds) + "...");
                display->display();
            } else {
                // Apagar
                displayOff();
                _displaySleeping = true;
            }
        }
    } else {
        // Verificar si es momento de countdown
        if (timeSinceActivity >= (_autoSleepTime - 3) * 1000) {
            _countdownActive = true;
            _countdownStart = millis();
            _countdownSeconds = 3;
        }
    }
}
```

### 7. DISPLAY_SLEEPING (Pantalla Dormida)
**Estado**: Bajo consumo  
**Display**: OFF (completamente negro)  
**LED**: OFF (0 PWM)  
**Consumo**: ~5mA (vs ~50mA encendido)

**Salida del estado**:
```mermaid
flowchart LR
    A[DISPLAY_SLEEPING] --> B{Evento de<br/>despertar}
    B -->|Botón PRG| C[wakeUpDisplay]
    B -->|Web ON| C
    B -->|Ninguno| A
    
    C --> D[displayOn]
    D --> E[LED ON]
    E --> F[_displaySleeping = false]
    F --> G[Reset _lastActivity]
    G --> H[SENSOR_ACTIVE]
    
    style A fill:#666,color:#fff
    style H fill:#9f9
```

**Botón PRG - Detección**:
```cpp
void handlePRGButton() {
    bool current = digitalRead(PRG_BUTTON_PIN);
    unsigned long now = millis();
    
    // Flanco descendente con debounce
    if (current == LOW && _lastButtonState == HIGH && 
        (now - _lastButtonPress > 200)) {
        
        _lastButtonPress = now;
        
        if (_displaySleeping) {
            wakeUpDisplay();  // Solo si está dormido
        }
    }
    
    _lastButtonState = current;
}
```

---

## Transiciones

### Tabla de Transiciones

| Estado Origen | Evento | Estado Destino | Tiempo |
|---------------|--------|----------------|--------|
| POWER_ON | `begin()` | SPLASH | 500ms |
| SPLASH | Timer 3s | INIT_WIFI | 3s |
| INIT_WIFI | Double reset | PORTAL_MODE | inmediato |
| INIT_WIFI | WiFi OK | SENSOR_MODE | ~2-5s |
| PORTAL_MODE | Config exitosa | SENSOR_MODE | inmediato |
| SENSOR_ACTIVE | 27s inactividad | AUTO_SLEEP_COUNTDOWN | 27s |
| AUTO_SLEEP_COUNTDOWN | Countdown termina | DISPLAY_SLEEPING | 3s |
| DISPLAY_SLEEPING | Botón PRG | SENSOR_ACTIVE | inmediato |
| DISPLAY_SLEEPING | Web ON | SENSOR_ACTIVE | inmediato |
| SENSOR_ACTIVE | Web OFF | DISPLAY_SLEEPING | 3s (countdown) |

### Diagrama de Transiciones con Tiempos

```mermaid
graph TD
    A[POWER_ON] -->|500ms| B[SPLASH]
    B -->|3s| C[INIT_WIFI]
    C -->|inmediato| D[PORTAL_MODE]
    C -->|2-5s| E[SENSOR_MODE]
    D -->|config OK| E
    E -->|27s| F[AUTO_SLEEP_COUNTDOWN]
    F -->|3s| G[DISPLAY_SLEEPING]
    G -->|PRG/Web| E
    E -->|Web OFF| H[Forced Sleep Countdown]
    H -->|3s| G
    
    style A fill:#4af
    style B fill:#4af
    style C fill:#fa4
    style D fill:#f44
    style E fill:#4f4
    style F fill:#ff4
    style G fill:#666,color:#fff
    style H fill:#f94
```

---

## Casos Especiales

### Caso 1: Despertar Accidental del Display
**Problema**: Usuario presiona PRG varias veces rápidamente  
**Solución**: Debounce de 200ms

```cpp
// Solo detecta UN evento cada 200ms mínimo
if (current == LOW && _lastButtonState == HIGH && 
    (now - _lastButtonPress > 200)) {
    // Procesar evento
    _lastButtonPress = now;
}
```

### Caso 2: Cambio de Modo Durante Countdown
**Escenario**: Se inicia countdown, pero llega evento web/PRG  
**Comportamiento**:
- Reset `_countdownActive = false`
- Reset `_lastActivity = millis()`
- Display se mantiene ON

```cpp
void wakeUpDisplay() {
    if (_displaySleeping) {
        displayOn();
        _displaySleeping = false;
        _countdownActive = false;  // ← Cancelar countdown
        _countdownSeconds = 0;
        _lastActivity = millis();  // ← Reset timer
    }
}
```

### Caso 3: Modo Portal → Modo Sensor
**Problema**: Auto-sleep estaba OFF, debe activarse  
**Solución**:
```cpp
// En transición a SENSOR_MODE
if (tryConnectWiFi()) {
    Serial.println("✅ WiFi conectado exitosamente");
    display_manager.enableAutoSleep(true);  // ← Activar
    startNormalMode();
}
```

### Caso 4: Error de Sensor Durante Sleep
**Comportamiento**: 
- Display permanece dormido (no despierta por error)
- Al despertar, muestra mensaje de error
- Auto-sleep sigue funcionando normalmente

### Caso 5: Múltiples Comandos Web Simultáneos
**Escenario**: Se envía ON y OFF casi al mismo tiempo  
**Comportamiento**: Último comando gana
```cpp
// Endpoint /api/display
if (action == "on") {
    if (display_manager.isDisplaySleeping()) {
        display_manager.wakeUpDisplay();
    }
} else if (action == "off") {
    display_manager.forceDisplaySleep();  // ← Ejecuta inmediatamente
}
```

---

## Resumen de Estados

```mermaid
pie title Distribución Típica de Estados (24h)
    "SENSOR_READING" : 60
    "DISPLAY_SLEEPING" : 35
    "AUTO_SLEEP_COUNTDOWN" : 3
    "PORTAL_MODE" : 1
    "SENSOR_ERROR" : 1
```

---

**Documentos relacionados:**
- [OLED_DISPLAY_ARCHITECTURE.md](./OLED_DISPLAY_ARCHITECTURE.md) - Arquitectura general
- [OLED_DISPLAY_MESSAGES.md](./OLED_DISPLAY_MESSAGES.md) - Catálogo de mensajes
- [OLED_DISPLAY_TIMING.md](./OLED_DISPLAY_TIMING.md) - Diagramas de tiempo
