# Documentación CSS - Captive Portal
## Professional WiFi Configuration Interface Styles

### 📋 Información General
- **Archivo**: `data/web/captive_portal/css/style.css`
- **Versión**: 1.2.0
- **Líneas de código**: 930
- **Último modificado**: [Fecha actual]
- **Propósito**: Estilos profesionales para interfaz de configuración WiFi

### 🎨 Arquitectura de Diseño

#### **1. SISTEMA DE VARIABLES CSS (Líneas 1-18)**
```css
:root {
    --primary-color: #03a9f4;     /* Material Design Blue */
    --success-color: #4caf50;     /* Éxito - Verde */
    --error-color: #f44336;       /* Error - Rojo */
    --warning-color: #ff9800;     /* Advertencia - Naranja */
    --gold-color: #ffd700;        /* Acentos - Dorado */
}
```
**Propósito**: Sistema centralizado de colores siguiendo Material Design Guidelines
**Beneficios**: Consistencia visual, fácil mantenimiento, cambios globales rápidos

#### **2. GESTIÓN DE TEMAS (Líneas 19-42)**
- **Tema Oscuro (Por defecto)**: Fondo gradient oscuro, texto claro
- **Tema Claro**: Gradient pastel, texto oscuro
- **Transiciones suaves**: 0.5s ease para cambios de tema
- **Variables dinámicas**: Automática aplicación en toda la interfaz

### 🏗️ Componentes Arquitecturales

#### **3. LAYOUT ESTRUCTURA (Líneas 70-100)**
- **Sistema Flexbox**: Layout principal con sidebar y contenido
- **Responsive Grid**: Adaptable a diferentes resoluciones
- **Spacing System**: Gaps consistentes de 20px
- **Z-index Management**: Layers organizados para overlays

#### **4. NAVEGACIÓN SIDEBAR (Líneas 75-150)**
```css
#side-nav {
    position: sticky;           /* Mantiene visibilidad */
    backdrop-filter: blur(12px); /* Efecto vidrio esmerilado */
    border-radius: 15px;        /* Bordes redondeados modernos */
    transition: all 0.5s ease;  /* Animaciones fluidas */
}
```
**Características**:
- **Efecto Glassmorphism**: Transparencia con blur
- **Posición Sticky**: Se mantiene visible al hacer scroll
- **Hover Effects**: Cambio de color de borde en interacción
- **Responsive**: Se convierte en barra inferior en móviles

#### **5. COMPONENTES DE TARJETAS (Líneas 190-220)**
```css
.card {
    background: var(--card-bg);
    backdrop-filter: blur(12px);
    border: 1px solid var(--gold-color);
    border-radius: 15px;
    scroll-margin-top: 90px;
}
```
**Funcionalidades**:
- **Glassmorphism Effect**: Fondo translúcido con blur
- **Smooth Scrolling**: Margen superior para navegación
- **Interactive Borders**: Cambio de color en hover
- **Content Organization**: Estructura clara para información

### 🔧 Elementos Interactivos

#### **6. SISTEMA DE FORMULARIOS (Líneas 225-290)**
**Componentes incluidos**:
- **Form Groups**: Estructura organizativa
- **Input Fields**: Campos de texto con focus effects
- **Select Dropdowns**: Menús desplegables estilizados
- **Checkbox Styling**: Casillas de verificación personalizadas
- **Help Text**: Texto explicativo en cursiva

**Características técnicas**:
```css
.form-group input:focus {
    border-color: var(--primary-color);
    background: rgba(0,0,0,0.2);
    outline: none;
}
```

#### **7. SISTEMA DE BOTONES (Líneas 275-320)**
**Variantes de botones**:
- **Primary**: Color principal con hover effect
- **Secondary**: Color gris neutro
- **Danger**: Color rojo para acciones destructivas
- **Icon Buttons**: Botones con iconos integrados

**Efectos de interacción**:
- **Hover Transform**: Elevación de 2px
- **Box Shadow**: Sombra dinámica en hover
- **Transition**: Animaciones suaves de 0.3s

#### **8. SELECTOR DE IDIOMAS (Líneas 330-400)**
```css
.language-dropdown {
    position: relative;
    display: inline-block;
}
```
**Funcionalidades**:
- **Dropdown Menu**: Menú desplegable animado
- **Flag Icons**: Iconos de banderas circulares
- **Hover States**: Estados interactivos
- **Positioning**: Posicionamiento absoluto responsivo

### 📶 Componentes WiFi Especializados

#### **9. LISTAS DE REDES WIFI (Líneas 625-720)**
```css
.wifi-list {
    max-height: 250px;
    overflow-y: auto;
    border-radius: 8px;
}
```
**Elementos incluidos**:
- **WiFi Items**: Elementos de red individuales
- **Signal Icons**: Iconos de intensidad de señal
- **Security Indicators**: Indicadores de seguridad
- **Hover Animations**: Efectos de deslizamiento en hover
- **RSSI Display**: Mostrar potencia de señal

**Estados interactivos**:
- **Hover Effect**: Highlight azul con transform
- **Selection State**: Estado seleccionado visual
- **Scrollable Container**: Lista desplazable con estilo

#### **10. PANEL DE INFORMACIÓN DISPOSITIVO (Líneas 750-800)**
```css
.device-info-panel {
    background: var(--card-bg);
    border-radius: 12px;
    padding: 15px;
}
```
**Componentes**:
- **Info Rows**: Filas de información key-value
- **Monospace Values**: Valores técnicos en fuente monoespaciada
- **Color Coding**: Codificación por colores para valores

### 🔔 Sistema de Notificaciones

#### **11. TOAST NOTIFICATIONS (Líneas 720-750)**
```css
.toast {
    transform: translateX(100%);
    transition: all 0.4s cubic-bezier(0.25, 0.8, 0.25, 1);
}
```
**Características**:
- **Slide Animation**: Animación de entrada desde la derecha
- **Auto-positioning**: Posicionamiento fijo inferior derecho
- **Color Variants**: Diferentes colores según tipo de mensaje
- **Cubic Bezier**: Easing suave profesional

### 📱 Responsive Design

#### **12. ADAPTACIÓN MÓVIL (Líneas 850-930)**
**Breakpoint**: `@media (max-width: 900px)`

**Cambios principales**:
- **Sidebar → Bottom Bar**: Navegación se mueve abajo
- **Horizontal Layout**: Navegación horizontal en móviles
- **Fixed Controls**: Controles flotantes fijos
- **Optimized Spacing**: Espaciado reducido para pantallas pequeñas

```css
#side-nav {
    position: fixed;
    bottom: 10px;
    flex-direction: row;
    height: 60px;
}
```

### 🚀 Características Avanzadas

#### **13. EFECTOS VISUALES**
- **Glassmorphism**: `backdrop-filter: blur(12px)`
- **Smooth Animations**: Transiciones de 0.3s-0.5s
- **Hover States**: Estados interactivos en todos los elementos
- **Z-index Management**: Gestión de capas ordenada

#### **14. ACCESIBILIDAD**
- **Color Contrast**: Contraste adecuado en ambos temas
- **Focus Styles**: Estados de foco visibles
- **Readable Fonts**: Fuentes legibles (Segoe UI, Roboto)
- **Responsive Text**: Tamaños de texto adaptables

#### **15. PERFORMANCE OPTIMIZATIONS**
- **CSS Variables**: Reutilización eficiente de valores
- **Selective Transitions**: Solo elementos necesarios animados
- **Optimized Selectors**: Selectores específicos y eficientes
- **Mobile-first**: Aproximación responsive optimizada

### 🎯 Casos de Uso Principales

1. **Configuración WiFi**: Interfaz intuitiva para conexión de redes
2. **Gestión de Identidad**: Opciones de configuración de dispositivo
3. **Información del Sistema**: Visualización de datos técnicos
4. **Navegación Multi-sección**: Interfaz de configuración completa
5. **Soporte Multi-idioma**: Interfaz internacionalizada
6. **Responsive Experience**: Experiencia óptima en todos los dispositivos

### 🔍 Notas de Implementación

- **Compatibilidad**: Webkit/Blink (Chrome, Safari, Edge)
- **Fallbacks**: Degradación elegante sin JavaScript
- **Modularity**: Estructura modular para fácil mantenimiento
- **Scalability**: Fácil extensión para nuevas funcionalidades

### 📊 Métricas de Código
- **Selectores**: ~120 selectores únicos
- **Propiedades**: ~400 declaraciones CSS
- **Media Queries**: 1 breakpoint principal
- **Variables CSS**: 15 custom properties
- **Animations**: 25+ transiciones/animaciones

Este sistema CSS proporciona una experiencia de usuario profesional y moderna para la configuración de dispositivos IoT ESP32.