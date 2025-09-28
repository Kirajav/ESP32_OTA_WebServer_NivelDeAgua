# Documentación JavaScript - Captive Portal
## Professional WiFi Configuration Interface Logic

### 📋 Información General
- **Archivo**: `data/web/captive_portal/js/captive-portal.js`
- **Versión**: 1.3.0
- **Líneas de código**: 1013
- **Último modificado**: [Fecha actual]
- **Propósito**: Lógica completa para interfaz de configuración WiFi profesional

### 🌐 Arquitectura del Código

#### **1. SISTEMA DE INTERNACIONALIZACIÓN (Líneas 8-212)**
```javascript
const translations = {
    es: { /* 70+ traducciones */ },
    en: { /* 70+ traducciones */ },
    fr: { /* 70+ traducciones */ }
};
```

**Características del sistema i18n**:
- **Multi-idioma**: Español, Inglés, Francés
- **Cobertura completa**: Toda la interfaz traducida
- **Persistencia**: LocalStorage para mantener selección
- **Contexto técnico**: Terminología IoT especializada
- **Ayudas contextuales**: Explicaciones técnicas traducidas

**Funciones de internacionalización**:
```javascript
function selectLanguage(lang, flag) {
    currentLanguage = lang;
    document.getElementById('current-flag').innerHTML = flag;
    applyTranslations(lang);
    localStorage.setItem('selectedLanguage', lang);
}
```

#### **2. GESTIÓN DE EVENTOS (Líneas 235-300)**
```javascript
function initializeEventListeners() {
    // Language dropdown management
    // Theme toggle functionality
    // Navigation link handlers
    // Form submission handlers
}
```

**Event listeners implementados**:
- **Language Dropdown**: Toggle y selección de idiomas
- **Theme Toggle**: Cambio entre tema claro/oscuro
- **Navigation**: Smooth scrolling entre secciones
- **Form Validation**: Validación en tiempo real
- **Mobile Interactions**: Gestión de overlays móviles

### 📡 Gestión Avanzada de WiFi

#### **3. ESCANEO DE REDES WIFI (Líneas 345-450)**
```javascript
function scanWiFi() {
    // Step 1: Initiate scan
    fetch('/scan-wifi')
    // Step 2: Check results periodically
    setTimeout(() => checkScanResults(), 2000);
}
```

**Proceso de escaneo**:
1. **Initiate Scan**: POST a `/scan-wifi` endpoint
2. **Status Polling**: Verificación periódica de resultados
3. **Result Processing**: Ordenamiento por intensidad de señal
4. **UI Updates**: Actualización visual progresiva
5. **Error Handling**: Manejo robusto de errores de red

**Características avanzadas**:
- **Async Processing**: Escaneo no-bloqueante
- **Signal Strength Visualization**: Iconos SVG dinámicos
- **Smart Sorting**: Ordenamiento por RSSI
- **Retry Logic**: Reintento automático en fallos

#### **4. VISUALIZACIÓN DE SEÑAL WIFI (Líneas 450-500)**
```javascript
function getWiFiSignalIcon(rssi) {
    // Signal strength calculation
    // Color coding by strength
    // SVG generation with bars
    // Accessibility tooltip
}
```

**Sistema de visualización**:
- **RSSI Ranges**: -50dBm (Excelente) a -80dBm (Débil)
- **Color Coding**: Verde → Amarillo → Rojo por intensidad
- **SVG Icons**: Iconos escalables con barras dinámicas
- **Tooltips**: Información técnica en hover
- **Accessibility**: Alt text y ARIA labels

#### **5. GESTIÓN DE REDES GUARDADAS (Líneas 550-650)**
```javascript
function loadSavedNetworks() {
    fetch('/api/wifi/saved')
        .then(response => response.json())
        .then(data => renderSavedNetworks(data));
}
```

**Funcionalidades**:
- **Load Saved**: Carga redes WiFi almacenadas
- **Set Default**: Establecer red predeterminada
- **Remove Network**: Eliminación con confirmación
- **Status Display**: Indicadores visuales de estado
- **Batch Operations**: Operaciones múltiples eficientes

### ⚙️ Sistema de Configuración

#### **6. CONFIGURACIÓN DEL SENSOR (Líneas 670-720)**
```javascript
function saveSensorConfig() {
    const config = {
        tank_height: document.getElementById('tank-height').value,
        // ... configuraciones físicas
        // Smart intervals configuration
        normal_interval: document.getElementById('normal-interval').value,
        filling_interval: document.getElementById('filling-interval').value,
        // Geographic NTP configuration
        auto_geo_location: document.getElementById('auto-geo-location').checked,
        timezone: document.getElementById('timezone').value,
        show_datetime: document.getElementById('show-datetime').checked
    };
}
```

**Parámetros de configuración avanzados**:
- **Dimensiones físicas**: Altura, capacidad, distancia mínima
- **Intervalos inteligentes**: Sensado normal vs. llenado
- **Geolocalización NTP**: Timezone automático por IP
- **Display settings**: Mostrar fecha/hora en OLED
- **Container types**: Tinaco, cisterna, genérico

#### **7. CONFIGURACIÓN DE SISTEMA (Líneas 720-770)**
```javascript
function saveSystemConfig() {
    // Auto-sleep configuration
    // Update interval settings
    // Power management options
}
```

**Gestión de energía**:
- **Auto Sleep**: Apagado automático de pantalla
- **Update Intervals**: Frecuencia de lecturas
- **Power Optimization**: Configuraciones de bajo consumo

### 🔧 Funciones de Control del Sistema

#### **8. OPERACIONES DE REINICIO (Líneas 780-850)**
```javascript
function saveAndRestartConfirm() {
    if(confirm('¿Guardar configuración y reiniciar el dispositivo?')) {
        fetch('/api/save-and-restart', { method: 'POST' });
    }
}
```

**Tipos de reinicio**:
- **Save & Restart**: Guarda y reinicia para aplicar WiFi
- **Simple Restart**: Reinicio sin guardar cambios
- **Factory Reset**: Reset completo con confirmación doble

#### **9. SISTEMA DE NOTIFICACIONES (Líneas 310-340)**
```javascript
function showToast(message, type = 'info') {
    // Create toast element
    // Apply styling by type
    // Animate in/out
    // Auto-remove after timeout
}
```

**Tipos de notificaciones**:
- **Info**: Información general (azul)
- **Success**: Operaciones exitosas (verde)
- **Warning**: Advertencias (naranja)
- **Error**: Errores y fallos (rojo)

### 🚀 Funcionalidades Avanzadas

#### **10. MODO ESP-NOW (Líneas 870-920)**
```javascript
function selectMode(mode) {
    selectedMode = mode;
    // Update visual interface
    // Enable/disable sections
    // Initialize mode-specific features
}
```

**Gestión de modos**:
- **WiFi Mode**: Configuración WiFi tradicional
- **ESP-NOW Mode**: Red mesh peer-to-peer
- **Dynamic UI**: Interfaz adaptable por modo
- **Future-ready**: Preparado para expansión

#### **11. RESPONSIVE MOBILE ACTIONS (Líneas 980-1000)**
```javascript
// Mobile actions panel logic
const mobileActionsSheet = document.getElementById('mobile-actions-sheet');
actionsNavLink.addEventListener('click', function(e) {
    if (window.innerWidth <= 900) {
        mobileActionsOverlay.classList.add('open');
    }
});
```

**Características móviles**:
- **Bottom Sheet**: Panel deslizante desde abajo
- **Touch Gestures**: Gestos táctiles para cerrar
- **Responsive Breakpoints**: Activación automática en móviles
- **Action Cloning**: Duplicación de acciones para móviles

### 🎯 Patrones de Diseño Implementados

#### **12. MODULAR ARCHITECTURE**
- **Separation of Concerns**: Funciones especializadas
- **Event-Driven**: Sistema basado en eventos
- **Promise-Based**: Async/await para operaciones de red
- **Error Handling**: Manejo centralizado de errores

#### **13. USER EXPERIENCE PATTERNS**
- **Progressive Enhancement**: Funcionalidad base + mejoras
- **Graceful Degradation**: Funciona sin JavaScript
- **Loading States**: Indicadores de progreso
- **Confirmation Dialogs**: Prevención de acciones accidentales

#### **14. PERFORMANCE OPTIMIZATIONS**
- **Lazy Loading**: Carga bajo demanda
- **Event Delegation**: Event listeners eficientes
- **LocalStorage Caching**: Cache de configuraciones
- **Debounced Actions**: Prevención de spam de requests

### 📱 Características Responsive

#### **15. MOBILE-FIRST DESIGN**
```javascript
if (window.innerWidth <= 900) {
    // Mobile-specific behavior
    mobileActionsOverlay.classList.add('open');
}
```

**Adaptaciones móviles**:
- **Touch-friendly**: Botones y áreas táctiles optimizadas
- **Swipe Gestures**: Navegación por gestos
- **Bottom Navigation**: Navegación accesible con pulgares
- **Modal Overlays**: Paneles modales para acciones

### 🔍 API Integration

#### **16. REST API CONSUMPTION**
```javascript
// Consistent API pattern
fetch('/api/endpoint', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(data)
})
.then(response => response.json())
.then(data => handleResponse(data))
.catch(error => handleError(error));
```

**Endpoints utilizados**:
- **WiFi Management**: `/api/wifi/*` - Gestión de redes
- **Configuration**: `/api/*/config` - Configuraciones por módulo
- **System Control**: `/api/save-and-restart` - Control del sistema
- **Factory Reset**: `/api/factory-reset` - Reset de fábrica

### 🛡️ Validación y Seguridad

#### **17. INPUT VALIDATION**
```javascript
if (!ssid) {
    showToast('Por favor ingrese el nombre de la red', 'error');
    return;
}
if (password && password.length < 8) {
    showToast('La contraseña debe tener al menos 8 caracteres', 'error');
    return;
}
```

**Validaciones implementadas**:
- **SSID Validation**: Longitud máxima 32 caracteres
- **Password Strength**: Mínimo 8 caracteres
- **Hostname Validation**: Sin espacios ni caracteres especiales
- **Numeric Ranges**: Validación de rangos para sensores

#### **18. ERROR HANDLING**
```javascript
.catch(error => {
    console.error('Error:', error);
    showToast(`Error: ${error.message}`, 'error');
});
```

**Estrategias de error**:
- **Network Errors**: Timeout y retry automático
- **Validation Errors**: Feedback inmediato al usuario
- **Server Errors**: Mensajes explicativos
- **Graceful Fallbacks**: Funcionalidad básica siempre disponible

### 🎨 Características de UI/UX

#### **19. THEME MANAGEMENT**
```javascript
function toggleTheme() {
    document.body.classList.toggle('light-theme');
    localStorage.setItem('theme', isDark ? 'light' : 'dark');
}
```

**Sistema de temas**:
- **Dark Mode**: Tema oscuro por defecto
- **Light Mode**: Tema claro opcional
- **Persistence**: Recordar preferencia del usuario
- **System Integration**: Futura integración con OS theme

#### **20. ANIMATION SYSTEM**
```javascript
setTimeout(() => toast.classList.add('show'), 100);
setTimeout(() => {
    toast.classList.remove('show');
    setTimeout(() => container.removeChild(toast), 400);
}, 3000);
```

**Animaciones implementadas**:
- **Toast Animations**: Slide-in desde la derecha
- **Smooth Scrolling**: Navegación suave entre secciones
- **Button Feedback**: Estados hover y active
- **Loading States**: Spinners y indicadores de progreso

### 📊 Métricas de Código

- **Funciones**: 25+ funciones especializadas
- **Event Listeners**: 15+ listeners configurados
- **API Endpoints**: 12+ endpoints integrados
- **Validations**: 20+ validaciones implementadas
- **Translations**: 210+ strings traducidos
- **Lines of Code**: 1013 líneas totales
- **Components**: 8 componentes principales (WiFi, Sensor, AP, System, etc.)

### 🚀 Casos de Uso Principales

1. **Configuración WiFi**: Escaneo, selección y gestión de redes
2. **Configuración de Sensor**: Parámetros físicos y de medición
3. **Gestión de Sistema**: Control de energía y reinicio
4. **Multi-idioma**: Soporte completo para 3 idiomas
5. **Responsive Mobile**: Experiencia móvil optimizada
6. **ESP-NOW Ready**: Preparado para redes mesh

### 🔧 Notas de Implementación

- **Browser Compatibility**: ES6+ (Chrome 60+, Firefox 55+, Safari 12+)
- **Progressive Enhancement**: Funciona sin JavaScript (formularios básicos)
- **Accessibility**: ARIA labels, keyboard navigation
- **Performance**: Lazy loading, debounced events
- **Security**: XSS prevention, input sanitization
- **Maintainability**: Código modular y bien documentado

Este sistema JavaScript proporciona una experiencia completa y profesional para la configuración de dispositivos IoT ESP32, con énfasis en usabilidad, internacionalización y responsive design.