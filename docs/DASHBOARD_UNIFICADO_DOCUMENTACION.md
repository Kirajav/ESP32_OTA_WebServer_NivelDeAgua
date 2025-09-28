# 📊 **DOCUMENTACIÓN DASHBOARD UNIFICADO - VERSIÓN ENTERPRISE**

## 🎯 **DESCRIPCIÓN GENERAL**

El **Dashboard Unificado** es una interfaz web avanzada que centraliza el monitoreo de múltiples sensores de nivel de agua en una sola vista profesional. Combina el sensor principal del ESP32 con sensores adicionales conectados via red mesh ESP-NOW.

---

## 🏗️ **ARQUITECTURA DEL DASHBOARD**

### **Componentes Principales:**

1. **🏠 Sensor Principal**: Muestra datos del sensor conectado directamente al ESP32
2. **📡 Red ESP-NOW**: Visualiza sensores adicionales en la red mesh
3. **🎨 Contenedores Animados**: 3 tipos de visualizaciones (tanque, contenedor, cisterna)
4. **📊 Estado de Red**: Indicador del estado de la red mesh
5. **🔄 Actualizaciones en Tiempo Real**: WebSocket + polling para datos frescos

---

## 🎨 **TIPOS DE CONTENEDORES ANIMADOS**

### **1. 🏠 TINACO (Predeterminado)**
```javascript
// Forma: Cilíndrico con tapa elíptica
// Uso: Tanques domésticos elevados
// Animación: Agua con ondas, medidor lateral
// SVG: 300x400 viewBox con gradientes y patrones
```

**Características:**
- Tapa elíptica superior
- Sensor ubicado en la parte superior
- Medidor lateral con marcas de porcentaje
- Animación de ondas en la superficie del agua
- Gradientes de color para simular profundidad

### **2. 🗂️ CONTENEDOR**
```javascript
// Forma: Rectangular con asas laterales
// Uso: Contenedores industriales/comerciales
// Animación: Agua plana, estructura robusta
// SVG: Diseño más angular y funcional
```

**Características:**
- Forma rectangular más industrial
- Asas laterales para transporte
- Estructura más robusta visualmente
- Ideal para aplicaciones comerciales

### **3. 🏗️ CISTERNA**
```javascript
// Forma: Cilíndrica grande con base elíptica
// Uso: Almacenamiento subterráneo/industrial
// Animación: Agua con forma elíptica en la base
// SVG: Incluye escalera de acceso
```

**Características:**
- Base elíptica para drenaje eficiente
- Escalera de acceso lateral
- Diseño para grandes volúmenes
- Colores terrosos (marrón/beige)
- Animación especial para forma elíptica

---

## 📡 **INTEGRACIÓN ESP-NOW**

### **Detección Automática de Sensores**
```javascript
function loadESPNowSensors() {
    fetch('/multi-sensor-data')
        .then(response => response.json())
        .then(data => {
            espNowSensors = data.sensors;
            renderESPNowSensors();
            updateNetworkStatus();
        })
        .catch(error => {
            console.error("Error cargando sensores ESP-NOW:", error);
            networkStatus.connected = false;
        });
}
```

### **Características de Sensores ESP-NOW:**
- **🆔 ID Único**: Cada sensor tiene identificador único
- **📶 Señal RSSI**: Indicador visual de intensidad de señal (4 barras)
- **📊 Datos en Tiempo Real**: Litros, porcentaje, distancia
- **🕐 Timestamp**: Última actualización de datos
- **🔴 Estado Offline**: Indicación visual cuando sensor está desconectado

---

## 🎛️ **CONTROLES INTERACTIVOS**

### **Selector de Tipo de Contenedor**
```html
<select id="container-type" class="modern-select">
    <option value="tank">🏠 Tinaco</option>
    <option value="container">🗂️ Contenedor</option>
    <option value="cistern">🏗️ Cisterna</option>
</select>
```

**Funcionalidad:**
- Cambio dinámico de visualización
- Persistencia en localStorage
- Re-animación automática con datos actuales
- Actualización del título del sensor

### **Indicador de Estado de Red**
```html
<div class="network-status">
    <div class="network-status-icon">🌐</div>
    <div class="network-status-text">
        <div class="network-status-title">Red ESP-NOW Activa</div>
        <div class="network-status-detail">2 sensores conectados</div>
    </div>
</div>
```

---

## 🔄 **SISTEMA DE ACTUALIZACIONES**

### **Múltiples Fuentes de Datos:**

1. **WebSocket Principal**: Para sensor principal en tiempo real
2. **Polling ESP-NOW**: Cada 15 segundos para sensores mesh
3. **API REST**: Para configuración y estados

### **Gestión de Estados:**
```javascript
let networkStatus = {
    connected: false,      // Estado de conexión ESP-NOW
    deviceCount: 0,        // Número de sensores detectados
    lastUpdate: null       // Última actualización exitosa
};
```

---

## 🎨 **SISTEMA DE ANIMACIONES**

### **Animaciones CSS:**
```css
.water-animation {
    animation: waterFlow 3s ease-in-out infinite;
}

.water-waves {
    animation: waves 2s ease-in-out infinite;
}

.filling-animation {
    animation: filling 1.5s ease-in-out infinite;
}
```

### **Animaciones por Tipo de Contenedor:**

#### **Tanque:**
- `updateTankAnimation()`: Animación estándar con ondas
- Agua vertical con medidor lateral
- Ondas en la superficie

#### **Contenedor:**
- `updateContainerAnimationRect()`: Animación rectangular
- Llenado vertical simple
- Sin ondas complejas

#### **Cisterna:**
- `updateCisternAnimation()`: Animación elíptica especial
- Base elíptica + cuerpo rectangular
- Manejo especial de altura de agua

---

## 📱 **DISEÑO RESPONSIVE**

### **Breakpoints:**
- **Desktop**: > 768px - Grid completo con múltiples columnas
- **Tablet**: 768px - Una columna para sensores ESP-NOW
- **Mobile**: < 480px - Stack vertical completo

### **Adaptaciones Móviles:**
```css
@media (max-width: 768px) {
    .esp-now-sensors-grid {
        grid-template-columns: 1fr;
    }
    
    .container-svg {
        height: 280px;
    }
}
```

---

## 🔧 **APIs UTILIZADAS**

### **Endpoints Principales:**

1. **`/multi-sensor-data`**: Datos de todos los sensores ESP-NOW
2. **`/api/esp-now/status`**: Estado de la red mesh
3. **`/api/sensor-data`**: Datos del sensor principal
4. **WebSocket `/ws`**: Datos en tiempo real

### **Formato de Respuesta ESP-NOW:**
```json
{
    "timestamp": 1234567890,
    "network": {
        "status": "active",
        "master_device": "AA:BB:CC:DD:EE:FF",
        "connected_count": 2
    },
    "sensors": [
        {
            "id": "ESP32_02",
            "name": "Sensor Cochera",
            "online": true,
            "waterLevel": 45.3,
            "percentage": 78,
            "distance": 23.4,
            "rssi": -45,
            "lastUpdate": "2025-09-28T14:30:00Z"
        }
    ]
}
```

---

## 🛠️ **FUNCIONES PRINCIPALES**

### **Inicialización:**
```javascript
initContainerSVGs()           // Genera SVGs para cada tipo
initContainerTypeSelector()   // Configura selector de tipo
loadESPNowSensors()          // Carga sensores mesh
startESPNowPolling()         // Inicia polling periódico
```

### **Renderizado:**
```javascript
renderContainerSVG()         // Dibuja contenedor seleccionado
renderESPNowSensors()        // Crea elementos de sensores mesh
updateNetworkStatus()        // Actualiza estado de red
```

### **Animaciones:**
```javascript
updateContainerAnimation()   // Animación unificada
updateTankAnimation()        // Específica para tanque
updateContainerAnimationRect() // Específica para contenedor
updateCisternAnimation()     // Específica para cisterna
```

---

## 🎯 **CASOS DE USO**

### **1. Hogar con Múltiples Tanques**
- Tinaco principal (ESP32 maestro)
- Tanque auxiliar (ESP32 esclavo ESP-NOW)
- Visualización unificada en un solo dashboard

### **2. Aplicación Comercial**
- Múltiples contenedores industriales
- Monitoreo centralizado
- Alertas de niveles bajos

### **3. Sistema de Cisternas**
- Cisternas subterráneas
- Red mesh para comunicación
- Monitoreo de múltiples ubicaciones

---

## 🚀 **FUNCIONALIDADES AVANZADAS**

### **Detección Automática de Llenado**
- El sensor principal detecta cuando se está llenando
- Aumenta automáticamente la frecuencia de medición
- Animaciones especiales durante el llenado

### **Persistencia de Configuración**
- Tipo de contenedor guardado en localStorage
- Configuraciones de usuario persistentes
- Restauración automática al cargar

### **Estados de Carga**
- Loading states para sensores
- Animación shimmer mientras cargan datos
- Indicadores de sensores offline

---

## 🔍 **DEBUG Y MONITOREO**

### **Consola de Debug:**
```javascript
// Acceder a datos en consola del navegador
window.smartSensor.logData();        // Ver todos los datos
window.smartSensor.switchContainer('cistern'); // Cambiar tipo
window.smartSensor.loadESPNow();     // Recargar ESP-NOW
```

### **Logging Detallado:**
- Console.log para cada operación importante
- Errores específicos para troubleshooting
- Métricas de rendimiento

---

## 📊 **MÉTRICAS DE RENDIMIENTO**

### **Optimizaciones Implementadas:**
- Polling inteligente (15s para ESP-NOW vs 1s para principal)
- Lazy loading de sensores
- Animaciones CSS optimizadas
- Debouncing de eventos de UI

### **Consumo de Recursos:**
- **JavaScript**: ~15KB comprimido
- **CSS**: ~8KB para estilos del dashboard
- **Memoria**: <2MB para DOM completo
- **Network**: <1KB por actualización ESP-NOW

---

## 🔮 **FUTURAS MEJORAS**

### **Próximas Características:**
1. **📊 Gráficos Históricos**: Charts.js para tendencias
2. **🔔 Alertas Push**: Notificaciones web nativas
3. **📱 PWA**: Progressive Web App con offline support
4. **🤖 IA Predictiva**: ML para predecir patrones de consumo
5. **🌐 Multi-idioma**: i18n completo
6. **📈 Analytics**: Dashboard de métricas avanzadas

### **Integraciones Planificadas:**
- **MQTT**: Para IoT platforms
- **InfluxDB**: Base de datos de series temporales
- **Grafana**: Dashboards profesionales
- **Home Assistant**: Integración domótica

---

*Documentación actualizada: Septiembre 28, 2025*  
*Versión Dashboard: 3.0.0 Enterprise Unified*