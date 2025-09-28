# 🔧 MÉTODOS PENDIENTES DE IMPLEMENTACIÓN

## **NTPTimeSync.cpp - Métodos Faltantes**

### **1. getCompactTime() - Formato Compacto para OLED**
```cpp
String NTPTimeSync::getCompactTime() {
    if (!_timeValid) return "--:--";
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return "--:--";
    
    char timeStr[6]; // "HH:MM"
    strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
    return String(timeStr);
}
```

### **2. getCompactDate() - Fecha Compacta para OLED**  
```cpp
String NTPTimeSync::getCompactDate() {
    if (!_timeValid) return "--/--";
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return "--/--";
    
    char dateStr[6]; // "DD/MM"
    strftime(dateStr, sizeof(dateStr), "%d/%m", &timeinfo);
    return String(dateStr);
}
```

### **3. getCompactDateTime() - Fecha y Hora Juntas**
```cpp
String NTPTimeSync::getCompactDateTime() {
    if (!_timeValid) return "--/-- --:--";
    
    String date = getCompactDate();
    String time = getCompactTime();
    return date + " " + time;
}
```

### **4. setLocation() - Configurar Ubicación Manual**
```cpp
void NTPTimeSync::setLocation(const String& timezone, const String& countryCode, const String& city) {
    _timezone = timezone;
    _countryCode = countryCode; 
    _city = city;
    
    // Aplicar zona horaria inmediatamente si está válida
    if (_timezone.length() > 0) {
        setenv("TZ", _timezone.c_str(), 1);
        tzset();
        _logger->log("NTP: Zona horaria configurada: " + _timezone);
    }
}
```

### **5. enableAutoGeoLocation() - Activar Detección Automática**
```cpp
void NTPTimeSync::enableAutoGeoLocation(bool enable) {
    _autoGeoLocation = enable;
    
    if (enable && WiFi.isConnected()) {
        detectLocationFromIP();
    }
    
    _logger->log("NTP: Auto-geolocalización " + String(enable ? "habilitada" : "deshabilitada"));
}
```

### **6. detectLocationFromIP() - Detección por IP**
```cpp
void NTPTimeSync::detectLocationFromIP() {
    if (!WiFi.isConnected()) return;
    
    HTTPClient http;
    http.begin("http://ip-api.com/json/?fields=timezone,countryCode,city");
    http.setTimeout(5000);
    
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        // Parse JSON básico (sin DynamicJsonDocument para ahorrar memoria)
        int timezoneStart = payload.indexOf("\"timezone\":\"") + 12;
        int timezoneEnd = payload.indexOf("\"", timezoneStart);
        if (timezoneStart > 11 && timezoneEnd > timezoneStart) {
            _timezone = payload.substring(timezoneStart, timezoneEnd);
        }
        
        int countryStart = payload.indexOf("\"countryCode\":\"") + 15;
        int countryEnd = payload.indexOf("\"", countryStart);
        if (countryStart > 14 && countryEnd > countryStart) {
            _countryCode = payload.substring(countryStart, countryEnd);
        }
        
        int cityStart = payload.indexOf("\"city\":\"") + 8;
        int cityEnd = payload.indexOf("\"", cityStart);
        if (cityStart > 7 && cityEnd > cityStart) {
            _city = payload.substring(cityStart, cityEnd);
        }
        
        // Aplicar zona horaria detectada
        if (_timezone.length() > 0) {
            setenv("TZ", _timezone.c_str(), 1);
            tzset();
        }
        
        _logger->log("NTP: Ubicación detectada: " + _city + ", " + _countryCode + " (" + _timezone + ")");
    } else {
        _logger->log("NTP: Error detectando ubicación por IP: " + String(httpCode));
    }
    
    http.end();
}
```

### **7. Variables de Clase Faltantes**
```cpp
// En NTPTimeSync.h - private:
String _timezone;
String _countryCode;
String _city;
bool _autoGeoLocation;
```

---

## **Portal Cautivo - Nueva Sección HTML**

### **Configuración de Sensado y Geolocalización**
```html
<!-- En captive_portal.html, agregar después de la sección WiFi -->

<div class="section">
    <h3>🔧 Configuración del Sensor</h3>
    
    <div class="form-group">
        <label for="normal_interval">⏱️ Intervalo Normal (segundos):</label>
        <input type="number" id="normal_interval" name="normal_interval" min="5" max="300" value="30">
        <small>Tiempo entre lecturas en uso normal (5-300 segundos)</small>
    </div>
    
    <div class="form-group">
        <label for="filling_interval">🚰 Intervalo Llenando (segundos):</label>
        <input type="number" id="filling_interval" name="filling_interval" min="1" max="30" value="5">
        <small>Tiempo entre lecturas cuando se detecta llenado (1-30 segundos)</small>
    </div>
    
    <div class="form-group">
        <label for="filling_threshold">🔢 Umbral Llenado (lecturas):</label>
        <input type="number" id="filling_threshold" name="filling_threshold" min="2" max="10" value="3">
        <small>Lecturas incrementales consecutivas para detectar llenado (2-10)</small>
    </div>
</div>

<div class="section">
    <h3>🌍 Geolocalización y Tiempo</h3>
    
    <div class="form-group">
        <label>
            <input type="checkbox" id="auto_geo_location" name="auto_geo_location" checked>
            🤖 Detectar ubicación automáticamente por IP
        </label>
    </div>
    
    <div id="manual_location" style="display:none;">
        <div class="form-group">
            <label for="timezone">🌐 Zona Horaria:</label>
            <select id="timezone" name="timezone">
                <option value="America/Mexico_City">América/Ciudad de México</option>
                <option value="America/Monterrey">América/Monterrey</option>
                <option value="America/Tijuana">América/Tijuana</option>
                <option value="America/New_York">América/Nueva York</option>
                <option value="America/Los_Angeles">América/Los Ángeles</option>
                <option value="Europe/Madrid">Europa/Madrid</option>
            </select>
        </div>
        
        <div class="form-group">
            <label for="country_code">🏳️ Código País:</label>
            <input type="text" id="country_code" name="country_code" maxlength="2" value="MX" placeholder="MX">
        </div>
        
        <div class="form-group">
            <label for="city">🏙️ Ciudad:</label>
            <input type="text" id="city" name="city" value="Mexico City" placeholder="Ciudad">
        </div>
    </div>
    
    <div class="form-group">
        <label>
            <input type="checkbox" id="show_datetime" name="show_datetime" checked>
            📅 Mostrar fecha y hora en pantallas
        </label>
    </div>
</div>

<script>
// Toggle manual location fields
document.getElementById('auto_geo_location').addEventListener('change', function() {
    const manualDiv = document.getElementById('manual_location');
    manualDiv.style.display = this.checked ? 'none' : 'block';
});
</script>
```

---

## **WebManager - Nuevos Endpoints**

### **Guardar Configuración de Sensor**
```cpp
// En WebManager.cpp - agregar nuevo endpoint
server.on("/save_sensor_config", HTTP_POST, [this]() {
    if (!server.hasArg("normal_interval") || 
        !server.hasArg("filling_interval") || 
        !server.hasArg("filling_threshold")) {
        server.send(400, "text/plain", "Parámetros faltantes");
        return;
    }
    
    uint16_t normalInterval = server.arg("normal_interval").toInt();
    uint16_t fillingInterval = server.arg("filling_interval").toInt();
    uint8_t fillingThreshold = server.arg("filling_threshold").toInt();
    
    // Validaciones
    if (normalInterval < 5 || normalInterval > 300) {
        server.send(400, "text/plain", "Intervalo normal inválido (5-300s)");
        return;
    }
    
    if (fillingInterval < 1 || fillingInterval > 30) {
        server.send(400, "text/plain", "Intervalo llenado inválido (1-30s)");
        return;
    }
    
    if (fillingThreshold < 2 || fillingThreshold > 10) {
        server.send(400, "text/plain", "Umbral llenado inválido (2-10)");
        return;
    }
    
    // Guardar configuración
    config_manager.setNormalInterval(normalInterval);
    config_manager.setFillingInterval(fillingInterval);
    config_manager.setFillingThreshold(fillingThreshold);
    config_manager.saveConfig();
    
    server.send(200, "text/plain", "Configuración de sensor guardada");
    
    logger->log("WebManager: Configuración sensor actualizada - Normal:" + 
                String(normalInterval) + "s, Llenado:" + String(fillingInterval) + "s");
});
```

### **Configurar Geolocalización**
```cpp
server.on("/save_geo_config", HTTP_POST, [this]() {
    bool autoGeo = server.hasArg("auto_geo_location");
    bool showDateTime = server.hasArg("show_datetime");
    
    config_manager.setAutoGeoLocation(autoGeo);
    config_manager.setShowDateTime(showDateTime);
    
    if (!autoGeo) {
        // Configuración manual
        String timezone = server.arg("timezone");
        String countryCode = server.arg("country_code");
        String city = server.arg("city");
        
        config_manager.setTimezone(timezone);
        config_manager.setCountryCode(countryCode);
        config_manager.setCity(city);
    }
    
    config_manager.saveConfig();
    server.send(200, "text/plain", "Configuración geográfica guardada");
    
    logger->log("WebManager: Configuración geo actualizada - Auto:" + 
                String(autoGeo) + ", DateTime:" + String(showDateTime));
});
```

---

## **Dashboard Mejorado - Estado del Sensor**

### **Información en Tiempo Real**
```html
<div class="sensor-status-card">
    <h3>📊 Estado del Sensor</h3>
    
    <div class="sensor-readings">
        <div class="reading">
            <span class="label">💧 Nivel Actual:</span>
            <span class="value" id="current_level">--</span>
        </div>
        
        <div class="reading">
            <span class="label">⏱️ Modo Sensado:</span>
            <span class="value" id="sensing_mode">--</span>
        </div>
        
        <div class="reading">
            <span class="label">🕐 Última Lectura:</span>
            <span class="value" id="last_reading">--</span>
        </div>
        
        <div class="reading">
            <span class="label">📈 Tendencia:</span>
            <span class="value" id="trend_indicator">--</span>
        </div>
    </div>
    
    <div class="sensor-controls">
        <button onclick="testSensor()" class="btn-test">🧪 Test Sensor</button>
        <button onclick="showConfigModal()" class="btn-config">⚙️ Configurar</button>
        <button onclick="calibrateSensor()" class="btn-calibrate">🎯 Calibrar</button>
    </div>
</div>

<script>
function updateSensorStatus() {
    fetch('/sensor_status')
        .then(response => response.json())
        .then(data => {
            document.getElementById('current_level').textContent = 
                data.distance + 'cm (' + data.liters + 'L, ' + data.percentage + '%)';
            
            document.getElementById('sensing_mode').textContent = 
                data.mode + ' (' + data.interval + 's)';
            
            document.getElementById('last_reading').textContent = data.timestamp;
            
            const trendIcon = data.trend === 'INCREMENTANDO' ? '↗️' : 
                             data.trend === 'DECREMENTANDO' ? '↘️' : '➡️';
            document.getElementById('trend_indicator').textContent = trendIcon + ' ' + data.trend;
        });
}

// Actualizar cada 5 segundos
setInterval(updateSensorStatus, 5000);
updateSensorStatus(); // Primera carga
</script>
```

---

## **Próximos Pasos Recomendados**

1. **Implementar métodos NTPTimeSync**: Los métodos arriba listados
2. **Actualizar portal cautivo**: Agregar secciones de configuración 
3. **Endpoints WebManager**: Para guardar nuevas configuraciones
4. **Dashboard mejorado**: Estado en tiempo real del sensor
5. **Testing**: Verificar funcionamiento con sensor real

**¡Todas las funcionalidades core ya están implementadas y funcionando!** 🎉