# ESP32-OTA-WebServer-NivelDeAgua

Este proyecto para ESP32 mide el nivel de agua en un tanque utilizando un sensor ultrasónico HC-SR04 y muestra los datos en una página web. Ofrece configuración a través de un portal cautivo y actualizaciones de firmware Over-the-Air (OTA).

## 🏗️ REFACTORIZACIÓN MAYOR - Arquitectura POO

### **Migración de Monolítico a Modular**
Este proyecto ha sido completamente refactorizado de una arquitectura monolítica (main.cpp único de 700+ líneas) a una **arquitectura modular orientada a objetos (POO)** con librerías especializadas.

### **Librerías Creadas**

#### 📋 **AppManager** - Coordinador Principal
- Gestiona la inicialización y coordinación de todos los componentes
- Maneja el ciclo de vida de la aplicación
- Coordina la comunicación entre módulos

#### ⚙️ **ConfigManager** - Gestión de Configuración
- Persistencia de configuración en JSON (SPIFFS)
- Valores por defecto y validación
- Gestión de parámetros del sensor y red

#### 🔬 **SensorFramework** - Framework de Sensores
- **Framework extensible** para múltiples tipos de sensores
- **WaterLevelSensor**: Implementación específica para HC-SR04
- Interfaz unificada para lectura y procesamiento de datos

#### 📱 **DisplayManager** - Control de Pantalla
- Gestión completa del OLED Heltec
- Efectos visuales y transiciones
- Control de encendido/apagado con cuenta regresiva

#### 🌐 **WebManager** - Servidor Web y APIs
- Servidor web asíncrono
- APIs REST para datos del sensor
- Gestión de rutas y endpoints

#### 📊 **SystemStatus** - Monitoreo del Sistema
- Estados del sistema y componentes
- Monitoreo de conectividad
- Gestión de timeouts y eventos

#### 🔄 **OTAUpdater** - Actualizaciones Remotas
- Actualizaciones Over-the-Air
- Efectos visuales durante actualización
- Gestión de progreso y estados

### **Beneficios de la Refactorización**

✅ **Mantenibilidad**: Código organizado en módulos específicos  
✅ **Escalabilidad**: Fácil agregar nuevos sensores o funcionalidades  
✅ **Debugging**: Logging granular por componente  
✅ **Reutilización**: Librerías reutilizables en otros proyectos  
✅ **Testing**: Cada componente se puede probar independientemente  
✅ **Separación de responsabilidades**: Cada clase tiene un propósito específico  

### **Estadísticas del Proyecto**
- **Compilación**: ✅ Exitosa
- **Flash**: 98.3% utilizado (1,288,445 bytes)
- **RAM**: 16.4% utilizado (53,776 bytes)
- **Archivos**: 8 librerías modulares
- **Líneas de código**: Distribuidas en arquitectura POO

## Características

-   **Medición de Nivel de Agua:** Utiliza un sensor ultrasónico HC-SR04 para medir la distancia al agua y calcular el volumen restante.
-   **Servidor Web Asíncrono:** Implementa un servidor web en el ESP32 para mostrar la información del nivel de agua.
-   **Portal Cautivo para WiFi:** Usa WiFiManager para facilitar la configuración de las credenciales de red (SSID y contraseña) sin necesidad de hardcodearlas.
-   **Actualizaciones OTA:** Integrado con ElegantOTA para permitir la actualización del firmware de forma remota a través de la interfaz web.
-   **Configuración Personalizada:** A través del portal cautivo, se pueden modificar los parámetros de funcionamiento del sensor:
    -   `ALTURA_MAX_AGUA_TINACO`: Altura máxima del tanque (en cm).
    -   `CAPACIDAD_LITROS_TINACO`: Capacidad total del tanque (en litros).
    -   `DISTANCIA_MINIMA_SENSOR`: Distancia mínima de operación del sensor.
-   **Interfaz Web Dinámica:** La página web muestra:
    -   Una imagen que representa visualmente el nivel del agua.
    -   La cantidad de litros restantes.
    -   La distancia medida en centímetros.
-   **Consola WebSerial:** Permite la depuración y el envío de comandos al ESP32 a través de una consola en el navegador.
-   **Display OLED:** Muestra información relevante como la dirección IP, la distancia y los litros en la pantalla OLED de la placa Heltec.
-   **Persistencia de Datos:** Guarda la configuración en el sistema de archivos SPIFFS para mantenerla entre reinicios.

## Hardware Requerido

-   Placa de desarrollo ESP32 (el código está adaptado para una Heltec WiFi Kit 32).
-   Sensor de distancia ultrasónico HC-SR04.

## Pines Utilizados

-   **LED:** `25` (LED integrado en la placa Heltec).
-   **Sensor HC-SR04:**
    -   `TRIGGER_PIN`: `12`
    -   `ECHO_PIN`: `13`

## Librerías

-   `Arduino`
-   `WiFi`
-   `heltec.h`
-   `SPIFFS`
-   `AsyncTCP`
-   `ESPAsyncWebServer`
-   `ESPAsyncWiFiManager`
-   `WebSerial`
-   `ElegantOTA`
-   `HCSR04`
-   `ArduinoJson`

## Instalación y Uso

1.  **Flashear el Firmware:** Compila y sube el proyecto a tu placa ESP32 usando PlatformIO.
2.  **Subir los Datos a SPIFFS:** Sube el contenido de la carpeta `data` al sistema de archivos SPIFFS del ESP32. Esto se puede hacer con la tarea "Upload File System image" de PlatformIO.
3.  **Configuración WiFi:**
    -   La primera vez que se inicie, el ESP32 creará un punto de acceso WiFi con el SSID **"WiFi ESP32: Sensor Tinaco"** y la contraseña **"1234"**.
    -   La primera vez que se inicie, el ESP32 creará un punto de acceso WiFi con el SSID **"ESP32: Sensor de nivel de agua"** y la contraseña **"12345"**.
    -   Conéctate a esta red desde un teléfono o una computadora. Se abrirá automáticamente un portal cautivo en tu navegador (o navega a `192.168.4.1`).
    -   En el portal, selecciona tu red WiFi local, introduce la contraseña y ajusta los parámetros del tanque.
    -   Guarda la configuración. El ESP32 se reiniciará y se conectará a tu red WiFi.
4.  **Acceder a la Interfaz Web:**
    -   La dirección IP del dispositivo se mostrará en el monitor serie y en la pantalla OLED.
    -   Abre un navegador web y navega a esa dirección IP para ver el nivel del agua.
5.  **Actualizaciones OTA:**
    -   Navega a `http://<IP_DEL_ESP32>/update`.
    -   Selecciona el nuevo archivo de firmware (`.bin`) y súbelo.

## Comandos WebSerial

Puedes interactuar con el dispositivo a través de la consola WebSerial disponible en la interfaz web. Algunos comandos útiles son:

-   `distancia`: Muestra la distancia actual medida.
-   `litros`: Muestra los litros calculados.
-   `ip`: Muestra la dirección IP.
-   `allinfo`: Muestra un resumen de toda la información del dispositivo.
-   `monitor`: Activa/desactiva el envío periódico de datos del sensor a la consola.
-   `ayuda`: Muestra la lista completa de comandos.

## Changelog de esta Versión

### 🏗️ **REFACTORIZACIÓN MAYOR - Arquitectura POO**

#### **Migración Arquitectónica Completa**
- **Antes**: Código monolítico en un solo `main.cpp` de 700+ líneas
- **Ahora**: Arquitectura modular POO con 7 librerías especializadas

#### **Correcciones Críticas Resueltas**
- ✅ **Bucle infinito de reinicios** - Restaurado `ESP.restart()` después del doble reset
- ✅ **Error de sintaxis** - Corregido regex del hostname en WiFiManager  
- ✅ **Reconexión WiFi incorrecta** - Solo reconecta cuando realmente se pierde conexión
- ✅ **Debugging mejorado** - Logging detallado para identificar problema de "0.0 L"

#### **Beneficios Técnicos**
- **Mantenibilidad**: Código organizado en módulos específicos
- **Escalabilidad**: Fácil agregar nuevos sensores o funcionalidades
- **Debugging**: Logging granular por componente
- **Reutilización**: Librerías reutilizables en otros proyectos
- **Testing**: Cada componente se puede probar independientemente

#### **Resultados de Compilación**
- **Flash**: 98.3% utilizado (1,288,445 bytes)
- **RAM**: 16.4% utilizado (53,776 bytes)
- **Estado**: ✅ Sin errores de compilación
- **Arquitectura**: Lista para expansión y mantenimiento

### **Versiones Anteriores**

#### **Robustez en la Configuración:**
- Se eliminó un bucle de reinicios que ocurría en el primer arranque si el archivo `config.json` no existía.
- El sistema ahora crea un `config.json` con valores por defecto de forma segura en el primer arranque.

#### **Corrección de Cálculo de Litros:**
- Se solucionó un error que causaba que el cálculo de litros mostrara "nan" (Not a Number) debido a una posible división por cero.

#### **Mejoras en la Interfaz de Usuario (Frontend):**
- El estado del display OLED (Encendido/Apagado) ahora se muestra correctamente en la página web tan pronto como se carga.
- Se mejoró el diseño de las tarjetas añadiendo un fondo estilizado a los títulos para mejorar la legibilidad y la jerarquía visual.
- Se añadió una barra de navegación superior para un acceso rápido a las secciones de Inicio, Actualización OTA y WebSerial.
- Se implementó un botón de reinicio del dispositivo con un diálogo de confirmación para evitar reinicios accidentales.
- Se integraron notificaciones "Toast" para proporcionar feedback visual al usuario sobre las acciones realizadas (ej. reinicio, cambio de estado del display).

#### **Correcciones del Display OLED:**
- Se eliminó el mensaje "OLED init..." que aparecía al iniciar el dispositivo.
- Se solucionó un problema que causaba que la pantalla no se limpiara correctamente al iniciar una actualización OTA, mostrando el mensaje de "Actualizando..." sobre los datos anteriores.
