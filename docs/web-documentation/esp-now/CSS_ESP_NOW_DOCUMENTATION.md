# Documentación CSS - ESP-NOW Network Manager
## Professional Mesh Network Management Interface Styles

### 📋 Información General
- **Archivo**: `data/web/esp_now/css/style.css`
- **Versión**: 1.0.0
- **Líneas de código**: 657
- **Último modificado**: [Fecha actual]
- **Propósito**: Estilos profesionales para gestión de redes mesh ESP-NOW

### 🎨 Arquitectura de Diseño

#### **1. SISTEMA DE VARIABLES CSS (Líneas 7-25)**
```css
:root {
    --primary-color: #03a9f4;     /* Material Design Blue */
    --success-color: #4caf50;     /* Success - Verde */
    --error-color: #f44336;       /* Error - Rojo */
    --warning-color: #ff9800;     /* Warning - Naranja */
    --gold-color: #ffd700;        /* Accent - Dorado */
}
```
**Arquitectura**: Utiliza el mismo sistema de variables que captive portal para consistencia
**Beneficios**: Cohesión visual entre módulos, mantenimiento centralizado

#### **2. TIPOGRAFÍA ESPECIALIZADA (Líneas 26-42)**
- **Font Stack**: 'Inter', 'Segoe UI', 'Roboto' - Fuentes modernas para interfaces técnicas
- **Line Height**: 1.6 para óptima legibilidad
- **Transiciones**: 0.5s ease para cambios de tema suaves
- **Responsive Typography**: Escalado automático según dispositivo

### 🏗️ Componentes Especializados ESP-NOW

#### **3. CONTENEDOR PRINCIPAL (Líneas 47-54)**
```css
.espnow-container {
    max-width: 1400px;    /* Container más amplio para datos técnicos */
    margin: 0 auto;       /* Centrado automático */
    padding: 20px;        /* Espaciado consistente */
    min-height: 100vh;    /* Altura completa de viewport */
}
```
**Propósito**: Container especializado para interfaces de administración de red

#### **4. HEADER ESPECIALIZADO (Líneas 55-85)**
```css
.espnow-header {
    background: var(--card-bg);
    backdrop-filter: blur(20px);      /* Blur más intenso que otros módulos */
    border-radius: 20px;              /* Bordes más suaves */
    padding: 30px;                    /* Padding generoso */
    text-align: center;
}
```
**Características**:
- **Gradient Text**: Título con gradient profesional
- **Enhanced Blur**: 20px blur para efecto premium
- **Typography Hierarchy**: Escalado tipográfico especializado

### 🌐 Visualización de Topología de Red

#### **5. COMPONENTE DE TOPOLOGÍA (Líneas 86-170)**
```css
.network-topology {
    background: var(--card-bg);
    border-radius: 20px;
    backdrop-filter: blur(16px);
}
```

**Elementos principales**:

**5.1 Visualización de Nodos**:
```css
.master-node, .slave-node {
    position: relative;
    background: var(--card-bg);
    border-radius: 16px;
    backdrop-filter: blur(8px);
    min-width: 180px;
}
```

**5.2 Diferenciación de Roles**:
- **Master Node**: Border verde, fondo con alpha de success color
- **Slave Node**: Border azul, fondo con alpha de primary color
- **Hover Effects**: Scale 1.05 con sombras dinámicas por color

**5.3 Líneas de Conexión Animadas**:
```css
.connection-line {
    background: linear-gradient(90deg, var(--success-color), var(--primary-color));
    animation: pulse-line 2s ease-in-out infinite;
}
```
**Animación**: Efecto de pulso para indicar actividad de red

### 📊 Sistema de Estadísticas Avanzado

#### **6. GRID DE ESTADÍSTICAS (Líneas 171-220)**
```css
.stats-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
    gap: 20px;
}
```

**Componentes de tarjetas estadísticas**:
- **Auto-fit Grid**: Adaptación automática según contenido
- **Hover Animations**: Elevación y cambio de borde
- **Icon System**: Iconos temáticos de 2.5rem
- **Typography Hierarchy**: Valores prominentes, labels secundarios

**Estados visuales**:
```css
.stat-card:hover {
    transform: translateY(-5px);
    border-color: var(--gold-color);
}
```

### 🔧 Panel de Gestión de Dispositivos

#### **7. SISTEMA DE DISPOSITIVOS (Líneas 221-350)**
```css
.devices-panel {
    display: grid;
    grid-template-columns: 1fr 1fr;    /* Split 50/50 para connected/available */
    gap: 30px;
}
```

**7.1 Lista de Dispositivos**:
```css
.device-list {
    max-height: 400px;
    overflow-y: auto;
}
```
**Características**:
- **Custom Scrollbar**: Estilos WebKit personalizados
- **Interactive Items**: Hover effects con transform translateX
- **Status Indicators**: Dots animados con colores por estado

**7.2 Estados de Dispositivos**:
```css
.status-online { 
    background: var(--success-color);
    box-shadow: 0 0 8px rgba(76, 175, 80, 0.6);
}
.status-connecting { 
    animation: pulse 1.5s ease-in-out infinite;
}
```
**Estados**: Online (verde con glow), Offline (rojo con glow), Connecting (naranja pulsante)

**7.3 Acciones por Dispositivo**:
```css
.action-btn-small {
    padding: 6px 12px;
    font-size: 0.8rem;
    border-radius: 8px;
    transition: all 0.3s ease;
}
```
**Funcionalidades**: Botones compactos con hover scale effects

### ⚙️ Panel de Controles de Red

#### **8. SISTEMA DE CONTROLES (Líneas 380-450)**
```css
.controls-panel {
    background: var(--card-bg);
    border-radius: 20px;
    backdrop-filter: blur(16px);
}
```

**8.1 Grid de Controles**:
```css
.control-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
    gap: 20px;
}
```

**8.2 Botones de Control Especializados**:
```css
.control-btn {
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 10px;
    transition: all 0.3s ease;
}
```

**Variantes por función**:
- **Default**: Hover azul con sombra primary
- **Danger**: Hover rojo para acciones destructivas
- **Success**: Hover verde para acciones positivas

**Estados interactivos**:
```css
.control-btn:hover {
    transform: translateY(-3px);
    box-shadow: 0 8px 25px rgba(33, 150, 243, 0.2);
}
```

### 🎭 Estados de Interfaz

#### **9. ESTADOS VACÍOS (Líneas 451-480)**
```css
.empty-state {
    text-align: center;
    padding: 40px 20px;
    color: var(--text-secondary);
}
```
**Elementos**:
- **Empty State Icon**: 3rem icon con opacity reducida
- **Message**: Mensaje explicativo de estado
- **Hint**: Sugerencia de acción

#### **10. ESTADOS DE CARGA (Líneas 481-510)**
```css
.loading-spinner {
    border: 2px solid var(--border-color);
    border-top-color: var(--primary-color);
    animation: spin 1s ease-in-out infinite;
}
```
**Animaciones**:
- **Spinner**: Rotación continua suave
- **Scanning Indicator**: Estado de búsqueda de dispositivos

### 📱 Responsive Design Especializado

#### **11. BREAKPOINT TABLET (768px) (Líneas 515-590)**
**Cambios principales**:
- **Devices Panel**: De 2 columnas a 1 columna
- **Topology**: De horizontal a vertical
- **Stats Grid**: Reducción de minWidth
- **Control Grid**: 2 columnas en lugar de auto-fit

#### **12. BREAKPOINT MÓVIL (480px) (Líneas 591-625)**
**Optimizaciones móviles**:
- **Single Column**: Todo en una columna
- **Reduced Padding**: Espaciado optimizado
- **Smaller Icons**: Iconos reducidos pero legibles
- **Simplified Actions**: Acciones reorganizadas

```css
@media (max-width: 480px) {
    .control-grid {
        grid-template-columns: 1fr;
    }
    .master-node, .slave-node {
        min-width: 120px;
        padding: 15px;
    }
}
```

### 🔄 Funcionalidades Avanzadas

#### **13. THEME TOGGLE (Líneas 626-642)**
```css
.theme-toggle {
    position: fixed;
    top: 20px;
    right: 20px;
    z-index: 1000;
}
```
**Características**:
- **Fixed Position**: Siempre visible y accesible
- **Circular Design**: Botón circular elegante
- **Hover States**: Cambio a primary color con scale

#### **14. UTILITY CLASSES (Líneas 643-657)**
**Clases de utilidad**:
- **Text Colors**: success, error, warning, muted
- **Spacing**: Margin bottom classes (mb-0 a mb-3)
- **Visibility**: hidden/visible toggles
- **Alignment**: text-center

### 🚀 Características Técnicas Destacadas

#### **15. PERFORMANCE OPTIMIZATIONS**
- **GPU Acceleration**: transform3d para animations
- **Efficient Selectors**: Especificidad optimizada
- **CSS Grid**: Layout moderno y eficiente
- **Custom Properties**: Variables para theming dinámico

#### **16. ACCESSIBILITY FEATURES**
- **Color Contrast**: Ratios WCAG AA compliant
- **Focus Management**: Estados de foco visibles
- **Animation Preferences**: Respeta reduce-motion
- **Semantic Structure**: Layout semánticamente correcto

#### **17. BROWSER COMPATIBILITY**
- **Modern Features**: CSS Grid, Custom Properties, Backdrop Filter
- **Progressive Enhancement**: Fallbacks para características avanzadas
- **Webkit Optimizations**: Scrollbar styling, backdrop-filter
- **Cross-browser**: Testing en Chromium, WebKit, Gecko

### 🎯 Casos de Uso Especializados

1. **Network Topology Management**: Visualización de redes mesh complejas
2. **Device Discovery**: Interfaz para encontrar y conectar dispositivos
3. **Real-time Monitoring**: Monitoreo en tiempo real de estado de red
4. **Network Administration**: Herramientas de control y configuración
5. **Status Visualization**: Indicadores visuales de conectividad
6. **Mobile Network Management**: Gestión móvil de redes IoT

### 🔍 Diferencias con Captive Portal

| Aspecto | Captive Portal | ESP-NOW Manager |
|---------|----------------|-----------------|
| **Layout** | Sidebar + Main | Container centralizado |
| **Focus** | Configuración WiFi | Gestión de red mesh |
| **Animations** | Sutiles | Más dinámicas (pulsos, glows) |
| **Grid System** | Form-based | Data-visualization focused |
| **Color Usage** | Consistent theming | Status-based colors |
| **Mobile Strategy** | Bottom navigation | Simplified stacking |

### 📊 Métricas de Código
- **Selectores**: ~95 selectores únicos
- **Propiedades**: ~320 declaraciones CSS
- **Media Queries**: 2 breakpoints especializados
- **Animations**: 3 keyframe animations
- **Grid Layouts**: 4 diferentes sistemas de grid
- **Custom Properties**: Herencia del sistema captive portal

### 🔧 Notas de Implementación

- **Especialización**: Diseñado específicamente para administración de redes
- **Escalabilidad**: Fácil extensión para nuevos tipos de dispositivos
- **Modularity**: Compatible con sistema de themes del proyecto
- **Performance**: Optimizado para actualizaciones en tiempo real
- **Accessibility**: Consideraciones para usuarios con discapacidades

Este sistema CSS proporciona una interfaz profesional especializada para la gestión de redes mesh ESP-NOW, optimizada para visualización de datos técnicos y administración de dispositivos IoT.