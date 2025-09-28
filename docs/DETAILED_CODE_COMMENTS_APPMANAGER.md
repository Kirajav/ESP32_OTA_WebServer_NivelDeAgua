/**
 * ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
 * APPMANAGER.CPP - GESTOR PRINCIPAL DEL SISTEMA ESP32 WATER LEVEL SENSOR v3.0 ENTERPRISE
 * ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
 * 
 * 📋 DESCRIPCIÓN GENERAL:
 * ----------------------
 * El AppManager es el corazón del sistema ESP32 Water Level Sensor. Implementa un patrón arquitectural
 * Enterprise-grade que coordina todos los componentes del sistema IoT de forma robusta, escalable y
 * mantenible. Este gestor centraliza toda la lógica de negocio y orquesta la interacción entre:
 * 
 * • 🌊 Sistema de sensores (ultrasónico, temperatura, humedad)
 * • 🌐 Conectividad (WiFi, ESP-NOW mesh, Bluetooth)
 * • 💻 Servidor web (HTTP/HTTPS, WebSockets, API REST)
 * • 📱 Interfaces de usuario (Web responsive, OLED display)
 * • ☁️ Integraciones cloud (Alexa, Google Home, Tuya IoT)
 * • 🔧 Mantenimiento (OTA updates, configuración, logging)
 * • 🔒 Seguridad (autenticación, encriptación, rate limiting)
 * 
 * 🏗️ ARQUITECTURA DEL SISTEMA:
 * ---------------------------
 * Implementa los siguientes patrones de diseño enterprise:
 * 
 * 1. 🎯 SINGLETON PATTERN:
 *    Una sola instancia del AppManager coordina todo el sistema
 * 
 * 2. 🏭 FACTORY PATTERN:
 *    Creación dinámica de sensores y servicios según configuración
 * 
 * 3. 👁️ OBSERVER PATTERN:
 *    Sistema de eventos para comunicación entre componentes
 * 
 * 4. 📋 STRATEGY PATTERN:
 *    Múltiples estrategias de conectividad y algoritmos de sensor
 * 
 * 5. 🔧 DEPENDENCY INJECTION:
 *    Inyección de dependencias para testabilidad y flexibilidad
 * 
 * 6. 🛡️ FACADE PATTERN:
 *    Interfaz simplificada para sistemas complejos
 * 
 * ⚡ CARACTERÍSTICAS ENTERPRISE:
 * ----------------------------
 * • 🚀 Alto rendimiento con optimizaciones de memoria y CPU
 * • 🔄 Alta disponibilidad con recuperación automática de fallos
 * • 📊 Monitoreo y métricas en tiempo real
 * • 🔒 Seguridad robusta con mejores prácticas
 * • 📈 Escalabilidad horizontal con red mesh
 * • 🛠️ Mantenibilidad con código modular y documentado
 * • 🧪 Testabilidad con interfaces y mocks
 * • 📱 Multi-plataforma (web, móvil, desktop)
 * 
 * 🎯 CASOS DE USO PRINCIPALES:
 * --------------------------
 * 1. 🏠 HOGAR INTELIGENTE: Monitor individual de tinaco/cisterna
 * 2. 🏢 COMERCIAL: Red de sensores para edificios/industrias
 * 3. 🌐 IOT MESH: Parte de ecosistema IoT más amplio
 * 4. 📊 ANALYTICS: Recolección de datos para ML y predicciones
 * 5. 🚨 ALERTAS: Sistema de notificaciones críticas
 * 6. 🤖 AUTOMATIZACIÓN: Integración con sistemas de control
 * 
 * 🔧 CONFIGURACIÓN Y PERSONALIZACIÓN:
 * ---------------------------------
 * El sistema es altamente configurable via:
 * • Archivo JSON de configuración (config.json)
 * • Variables de entorno en tiempo de compilación
 * • Portal web de configuración (captive portal)
 * • API REST para configuración remota
 * • OTA updates para cambios de firmware
 * 
 * 📊 MÉTRICAS Y MONITOREO:
 * ----------------------
 * Sistema completo de métricas en tiempo real:
 * • Performance (CPU, memoria, red)
 * • Calidad de datos (precisión, latencia)
 * • Conectividad (WiFi strength, mesh topology)
 * • Hardware (temperatura, voltaje, uptime)
 * • Aplicación (requests/sec, errors, features usage)
 * 
 * @author: ESP32 IoT Development Team
 * @version: 3.0.0 Enterprise Architecture
 * @date: Septiembre 2025
 * @license: MIT License
 * @hardware: ESP32 Heltec WiFi Kit 32 (32MB Flash, 4MB PSRAM, OLED 128x64)
 * @framework: Arduino Core for ESP32 v2.0+, AsyncWebServer, ArduinoJson
 * @dependencies: Ver platformio.ini para lista completa de librerías
 */

#include "AppManager.h"                    // ← Header principal del gestor
#include <DNSServer.h>                     // ← DNS server para portal cautivo
#include <esp_task_wdt.h>                  // ← Watchdog timer del ESP32
#include <esp_system.h>                    // ← Funciones de sistema ESP32
#include <ArduinoJson.h>                   // ← Parsing y generación JSON
#include <esp_wifi.h>                      // ← WiFi low-level APIs
#include <esp_now.h>                       // ← ESP-NOW protocol APIs
#include <time.h>                          // ← Time functions para NTP
#include <sys/time.h>                      // ← System time para timestamps

/**
 * ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
 * 🌐 INSTANCIAS GLOBALES DE SERVICIOS CORE
 * ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
 * 
 * Estas instancias globales gestionan los servicios fundamentales del sistema.
 * Se inicializan una sola vez y se comparten entre todos los componentes.
 */

/**
 * 🌐 SERVIDOR WEB ASÍNCRONO
 * Puerto 80 estándar HTTP. Maneja múltiples conexiones concurrentes sin bloquear
 * el loop principal. Optimizado para ESP32 con buffer management inteligente.
 */
AsyncWebServer server(80);

/**
 * 🔍 SERVIDOR DNS PARA PORTAL CAUTIVO
 * Redirige todas las peticiones DNS al ESP32 cuando está en modo portal cautivo.
 * Permite que dispositivos se conecten automáticamente sin configuración manual.
 */
DNSServer dnsServer;

/**
 * 🔄 DETECTOR DE DOBLE RESET
 * Detecta cuando el usuario hace reset doble para forzar portal de configuración.
 * Parámetros: timeout=10s, address=0 (EEPROM virtual en flash)
 */
DoubleResetDetector drd(10, 0);

/**
 * ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
 * 🛡️ VARIABLES DE ESTABILIDAD Y PROTECCIÓN DEL SISTEMA
 * ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
 * 
 * Estas variables implementan protecciones contra operaciones que podrían
 * desestabilizar el sistema o consumir excesivos recursos.
 */

/**
 * ⏰ CONTROL DE FRECUENCIA DE ESCANEO WIFI
 * Previene escaneos WiFi muy frecuentes que pueden causar:
 * • Interferencia con conexiones activas
 * • Alto consumo de CPU y memoria
 * • Degradación de performance general
 * • Posibles resets por watchdog
 */
static unsigned long lastWiFiScan = 0;                    // ← Timestamp del último escaneo
static const unsigned long WIFI_SCAN_INTERVAL = 10000;    // ← 10 segundos mínimo entre escaneos

/**
 * 📊 MÉTRICAS DE PERFORMANCE EN TIEMPO REAL
 * Variables para monitoreo continuo del sistema
 */
static unsigned long systemStartTime = 0;                 // ← Tiempo de inicio del sistema
static unsigned long totalLoopCycles = 0;                 // ← Contador de ciclos del loop principal
static unsigned long totalAPIRequests = 0;                // ← Contador de peticiones API
static unsigned long totalSensorReadings = 0;             // ← Contador de lecturas de sensor
static float averageLoopTime = 0.0;                       // ← Tiempo promedio por ciclo de loop
static uint32_t peakMemoryUsage = 0;                      // ← Pico de uso de memoria
static uint32_t minFreeHeap = UINT32_MAX;                 // ← Mínima memoria libre registrada

/**
 * ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
 * 🏗️ CONSTRUCTOR DE APPMANAGER - INICIALIZACIÓN DE ESTADO
 * ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
 * 
 * El constructor inicializa el estado base del sistema sin realizar operaciones
 * pesadas. La inicialización real se hace en initialize() para mejor control.
 */
AppManager::AppManager() {
    Serial.println("┌─────────────────────────────────────────────────────────┐");
    Serial.println("│             🚀 AppManager Constructor                   │");
    Serial.println("│          Inicializando estado base del sistema         │");
    Serial.println("└─────────────────────────────────────────────────────────┘");
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // 🔧 INICIALIZACIÓN DE VARIABLES DE ESTADO CORE
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    
    /**
     * 🌐 ESTADO DE CONECTIVIDAD
     * Variables que rastrean el estado de las conexiones de red
     */
    wifi_connected = false;                    // ← WiFi no conectado inicialmente
    portal_active = false;                     // ← Portal cautivo inactivo inicialmente
    meshNetworkActive = false;                 // ← Red mesh ESP-NOW inactiva inicialmente
    internetConnected = false;                 // ← Conexión a Internet no verificada
    lastConnectivityCheck = 0;                 // ← Timestamp de última verificación de conectividad
    
    /**
     * 📱 ESTADO DEL DISPLAY OLED
     * Variables para gestión inteligente de la pantalla
     */
    lastDisplayActivity = 0;                   // ← Timestamp de última actividad en display
    displaySleeping = false;                   // ← Display no está en modo sleep inicialmente
    displayBrightness = 255;                   // ← Brillo máximo inicialmente (0-255)
    displayAutoSleep = true;                   // ← Auto-sleep habilitado por defecto
    
    /**
     * ⚡ VARIABLES DE PERFORMANCE Y OPTIMIZACIÓN
     * Estado para optimizaciones dinámicas del sistema
     */
    systemLoadLevel = 0;                       // ← Nivel de carga del sistema (0-100%)
    adaptiveIntervals = true;                  // ← Intervalos adaptativos habilitados
    powerSaveMode = false;                     // ← Modo ahorro energía deshabilitado inicialmente
    cpuFrequency = 240;                        // ← Frecuencia CPU por defecto (240MHz)
    
    /**
     * 🔒 VARIABLES DE SEGURIDAD Y PROTECCIÓN
     * Estado para características de seguridad del sistema
     */
    securityLevel = SECURITY_MEDIUM;           // ← Nivel de seguridad medio por defecto
    rateLimitingActive = true;                 // ← Rate limiting habilitado
    encryptionEnabled = false;                 // ← Encriptación deshabilitada inicialmente (HTTPS pendiente)
    authenticationRequired = false;            // ← Autenticación no requerida inicialmente
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // 🧩 INICIALIZACIÓN DE PUNTEROS DE COMPONENTES IOT
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    
    /**
     * 🔌 INTEGRACIONES IOT - INICIALIZACIÓN COMO nullptr
     * Los componentes IoT se inicializan como nullptr y se crean dinámicamente
     * según la configuración y disponibilidad de servicios.
     */
    espNowManager = nullptr;                   // ← Gestor red mesh ESP-NOW
    googleHome = nullptr;                      // ← Integración Google Assistant
    alexa = nullptr;                           // ← Integración Amazon Alexa
    tuya = nullptr;                            // ← Integración Tuya IoT Platform
    tuyaDevice = nullptr;                      // ← Device específico Tuya
    ntpSync = nullptr;                         // ← Sincronización NTP para timestamps
    mqttClient = nullptr;                      // ← Cliente MQTT para brokers
    cloudAnalytics = nullptr;                  // ← Cliente analytics en la nube
    
    /**
     * 🎯 MANAGERS ESPECIALIZADOS
     * Punteros a managers de funcionalidades específicas
     */
    apiManager = nullptr;                      // ← Gestor de APIs REST
    securityManager = nullptr;                 // ← Gestor de seguridad
    cacheManager = nullptr;                    // ← Gestor de cache para performance
    logManager = nullptr;                      // ← Gestor de logging avanzado
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // 📊 INICIALIZACIÓN DE MÉTRICAS Y CONTADORES
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    
    systemStartTime = millis();                // ← Registrar tiempo de inicio
    totalLoopCycles = 0;                       // ← Resetear contador de ciclos
    totalAPIRequests = 0;                      // ← Resetear contador de requests
    totalSensorReadings = 0;                   // ← Resetear contador de lecturas
    peakMemoryUsage = ESP.getFreeHeap();       // ← Memoria inicial como referencia
    minFreeHeap = ESP.getFreeHeap();           // ← Memoria libre inicial
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // 📝 LOGGING DE INICIALIZACIÓN EXITOSA
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    
    Serial.printf("✅ AppManager constructor completado\n");
    Serial.printf("   💾 Memoria libre inicial: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("   🖥️ CPU Frequency: %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("   📶 WiFi Status: %s\n", wifi_connected ? "Connected" : "Disconnected");
    Serial.printf("   🏠 Portal Status: %s\n", portal_active ? "Active" : "Inactive");
    Serial.printf("   📱 Display Status: %s\n", displaySleeping ? "Sleeping" : "Active");
    Serial.println();
}

/**
 * ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
 * 🚀 MÉTODO INITIALIZE() - INICIALIZACIÓN COMPLETA DEL SISTEMA
 * ═══════════════════════════════════════════════════════════════════════════════════════════════════════════════
 * 
 * Este método es el corazón de la inicialización del sistema. Ejecuta una secuencia
 * cuidadosamente orquestada de pasos para levantar todos los componentes del sistema
 * de forma robusta y confiable.
 * 
 * 📋 FASES DE INICIALIZACIÓN:
 * -------------------------
 * 1. 🛡️ Configuración de protecciones del sistema (watchdog, memoria)
 * 2. 🔄 Detección de condiciones especiales (doble reset, recovery mode)
 * 3. 💾 Inicialización del file system (SPIFFS)
 * 4. ⚙️ Carga de configuración del sistema
 * 5. 📱 Inicialización del display OLED
 * 6. 🌊 Configuración de sensores
 * 7. 🌐 Inicialización de conectividad (WiFi/ESP-NOW)
 * 8. 💻 Setup del servidor web y APIs
 * 9. ☁️ Configuración de integraciones IoT
 * 10. 🔧 Inicialización de servicios auxiliares
 * 11. ✅ Verificación final y activación
 */
void AppManager::initialize() {
    Serial.println();
    Serial.println("████████████████████████████████████████████████████████████████████████");
    Serial.println("█                                                                      █");
    Serial.println("█           🚀 INICIANDO SISTEMA COMPLETO v3.0 ENTERPRISE             █");
    Serial.println("█                     Water Level Sensor System                       █");
    Serial.println("█                                                                      █");
    Serial.println("████████████████████████████████████████████████████████████████████████");
    Serial.println();
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // 🛡️ FASE 1: CONFIGURACIÓN DE PROTECCIONES DEL SISTEMA
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    Serial.println("┌─ FASE 1: Configurando protecciones del sistema");
    
    /**
     * ⏰ CONFIGURACIÓN DEL WATCHDOG TIMER
     * El watchdog timer protege contra colgadas del sistema reiniciando
     * automáticamente si no recibe señal de vida en 30 segundos.
     */
    esp_task_wdt_init(30, true);                           // ← 30s timeout, panic habilitado
    esp_task_wdt_add(NULL);                                // ← Añadir tarea actual al watchdog
    Serial.println("  ✅ Watchdog timer configurado (30s timeout)");
    
    /**
     * 📊 REGISTRO DE ESTADO INICIAL DEL SISTEMA
     * Captura métricas base para monitoreo y debugging
     */
    uint32_t initialFreeHeap = ESP.getFreeHeap();
    uint32_t heapSize = ESP.getHeapSize();
    uint32_t cpuFreq = ESP.getCpuFreqMHz();
    
    Serial.printf("  📊 Estado inicial del sistema:\n");
    Serial.printf("     💾 Heap total: %d bytes (%.1f KB)\n", heapSize, heapSize/1024.0);
    Serial.printf("     💾 Heap libre: %d bytes (%.1f KB, %.1f%%)\n", 
                  initialFreeHeap, initialFreeHeap/1024.0, (initialFreeHeap*100.0)/heapSize);
    Serial.printf("     🖥️ CPU Freq: %d MHz\n", cpuFreq);
    Serial.printf("     🔧 Chip Model: %s\n", ESP.getChipModel());
    Serial.printf("     📦 SDK Version: %s\n", ESP.getSdkVersion());
    Serial.printf("     🆔 Chip ID: %04X%08X\n", (uint16_t)(ESP.getEfuseMac()>>32), (uint32_t)ESP.getEfuseMac());
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // 🔄 FASE 2: DETECCIÓN DE CONDICIONES ESPECIALES
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    Serial.println("└─ FASE 1 completada");
    Serial.println("┌─ FASE 2: Detectando condiciones especiales de inicio");
    
    /**
     * 🔄 DETECCIÓN DE DOBLE RESET
     * Si el usuario hace doble reset en menos de 10 segundos,
     * fuerza el modo portal cautivo para reconfiguración
     */
    if (drd.detectDoubleReset()) {
        Serial.println("  🔄 ¡DOBLE RESET DETECTADO!");
        Serial.println("     → Forzando modo portal cautivo para reconfiguración");
        Serial.println("     → Usuario requiere reconfigurar WiFi o parámetros");
        forcePortalMode = true;
        
        // Mostrar en display para feedback inmediato al usuario
        display_manager.begin();
        display_manager.clearDisplay();
        display_manager.drawString(0, 0, "DOBLE RESET");
        display_manager.drawString(0, 16, "Portal Activo");
        display_manager.drawString(0, 32, "Configurar WiFi");
        display_manager.display();
        
    } else {
        Serial.println("  ✅ Reset normal detectado");
        Serial.println("     → Intentando conectar con configuración guardada");
        forcePortalMode = false;
    }
    
    /**
     * 🔍 DETECCIÓN DE MODO RECOVERY
     * Verificar si el sistema debe iniciar en modo de recuperación
     * basado en fallos previos o configuración específica
     */
    bool recoveryMode = checkRecoveryMode();
    if (recoveryMode) {
        Serial.println("  ⚠️ Modo de recuperación activado");
        Serial.println("     → Sistema iniciará con configuración mínima");
        Serial.println("     → Algunas funciones avanzadas estarán deshabilitadas");
    }
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // 💾 FASE 3: INICIALIZACIÓN DEL FILE SYSTEM
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    Serial.println("└─ FASE 2 completada");
    Serial.println("┌─ FASE 3: Inicializando sistema de archivos SPIFFS");
    
    /**
     * 📁 MONTAJE DEL SISTEMA DE ARCHIVOS SPIFFS
     * SPIFFS almacena archivos web, configuración y datos del sistema
     * formatOnFail=true permite recuperación automática si está corrupto
     */
    if (!SPIFFS.begin(true)) {
        Serial.println("  ❌ ERROR CRÍTICO: SPIFFS falló al montar");
        Serial.println("     → Sistema no puede continuar sin file system");
        Serial.println("     → Verificar particiones en platformio.ini");
        Serial.println("     → Intentar re-flash completo del firmware");
        
        // Mostrar error crítico en display
        display_manager.begin();
        display_manager.clearDisplay();
        display_manager.drawString(0, 0, "ERROR CRÍTICO");
        display_manager.drawString(0, 16, "SPIFFS Failed");
        display_manager.drawString(0, 32, "Re-flash needed");
        display_manager.display();
        
        // Entrar en loop infinito con parpadeo del LED para indicar error crítico
        while(true) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(500);
            digitalWrite(LED_BUILTIN, LOW);
            delay(500);
            esp_task_wdt_reset();  // Reset watchdog para evitar reinicio
        }
        return;  // Nunca se ejecuta, pero buena práctica
    }
    
    /**
     * 📊 INFORMACIÓN DEL FILE SYSTEM
     * Mostrar estadísticas de uso para monitoreo y debugging
     */
    size_t totalBytes = SPIFFS.totalBytes();
    size_t usedBytes = SPIFFS.usedBytes();
    size_t freeBytes = totalBytes - usedBytes;
    
    Serial.printf("  ✅ SPIFFS montado exitosamente\n");
    Serial.printf("     📊 Espacio total: %d bytes (%.1f KB)\n", totalBytes, totalBytes/1024.0);
    Serial.printf("     📊 Espacio usado: %d bytes (%.1f KB, %.1f%%)\n", 
                  usedBytes, usedBytes/1024.0, (usedBytes*100.0)/totalBytes);
    Serial.printf("     📊 Espacio libre: %d bytes (%.1f KB, %.1f%%)\n", 
                  freeBytes, freeBytes/1024.0, (freeBytes*100.0)/totalBytes);
    
    // Verificar si el espacio libre es crítico (menos del 10%)
    if (freeBytes < (totalBytes * 0.1)) {
        Serial.println("  ⚠️ ADVERTENCIA: Espacio libre en SPIFFS crítico (<10%)");
        Serial.println("     → Considerar limpiar archivos de log antiguos");
        Serial.println("     → Verificar tamaño de archivos web");
    }
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // ⚙️ FASE 4: CARGA DE CONFIGURACIÓN DEL SISTEMA
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    Serial.println("└─ FASE 3 completada");
    Serial.println("┌─ FASE 4: Cargando configuración del sistema");
    
    /**
     * 📋 INICIALIZACIÓN DEL CONFIG MANAGER
     * El ConfigManager gestiona toda la configuración del sistema
     * con valores por defecto, validación y persistencia
     */
    if (!config_manager.begin()) {
        Serial.println("  ⚠️ Warning: Error inicializando ConfigManager");
        Serial.println("     → Usando configuración por defecto");
        Serial.println("     → El sistema funcionará pero sin personalización");
        
        // Crear configuración por defecto mínima
        createDefaultConfiguration();
    } else {
        Serial.println("  ✅ ConfigManager inicializado correctamente");
        
        // Mostrar configuración cargada
        displayLoadedConfiguration();
    }
    
    /**
     * 🔧 APLICACIÓN DE CONFIGURACIÓN AL SISTEMA
     * Aplicar configuración cargada a todos los componentes
     */
    applySystemConfiguration();
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // 📱 FASE 5: INICIALIZACIÓN DEL DISPLAY OLED
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    Serial.println("└─ FASE 4 completada");
    Serial.println("┌─ FASE 5: Inicializando display OLED");
    
    /**
     * 📺 INICIALIZACIÓN DEL DISPLAY MANAGER
     * Gestiona la pantalla OLED integrada del Heltec WiFi Kit 32
     */
    if (!display_manager.begin()) {
        Serial.println("  ⚠️ Warning: Display OLED no pudo inicializarse");
        Serial.println("     → Sistema continuará sin display");
        Serial.println("     → Verificar conexiones I2C (SDA/SCL)");
        displayAvailable = false;
    } else {
        Serial.println("  ✅ Display OLED inicializado correctamente");
        displayAvailable = true;
        
        /**
         * 🎨 CONFIGURACIÓN INICIAL DEL DISPLAY
         * Aplicar configuración de brillo, rotación y auto-sleep
         */
        uint16_t autoSleepTime = config_manager.getAutoSleepTime();
        display_manager.setAutoSleepTime(autoSleepTime);
        Serial.printf("     ⏰ Auto-sleep configurado: %d segundos\n", autoSleepTime);
        
        uint8_t brightness = config_manager.getDisplayBrightness();
        display_manager.setBrightness(brightness);
        Serial.printf("     🔆 Brillo configurado: %d/255\n", brightness);
        
        /**
         * 📄 MOSTRAR PANTALLA DE INICIALIZACIÓN
         * Feedback visual inmediato para el usuario
         */
        display_manager.clearDisplay();
        display_manager.drawString(0, 0, "Sistema v3.0");
        display_manager.drawString(0, 16, "Inicializando...");
        
        // Mostrar progress bar animado
        for (int i = 0; i <= 100; i += 10) {
            display_manager.drawProgressBar(0, 40, 128, 8, i);
            display_manager.display();
            delay(100);
            esp_task_wdt_reset();  // Reset watchdog durante animación
        }
        
        display_manager.drawString(0, 54, "Display OK");
        display_manager.display();
    }
    
    // Inicializar timer de actividad del display
    lastDisplayActivity = millis();
    displaySleeping = false;
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // 🌊 FASE 6: CONFIGURACIÓN DE SENSORES
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    Serial.println("└─ FASE 5 completada");
    Serial.println("┌─ FASE 6: Configurando sistema de sensores");
    
    esp_task_wdt_reset(); // Reset watchdog antes de operaciones pesadas
    
    /**
     * 🌊 INICIALIZACIÓN DEL SENSOR PRINCIPAL DE AGUA
     * Configurar sensor ultrasónico HC-SR04 con pines optimizados
     * para el Heltec WiFi Kit 32
     */
    Serial.println("  🌊 Inicializando sensor ultrasónico de agua...");
    
    // Pines optimizados para Heltec WiFi Kit 32 (evitan conflictos con OLED y LoRa)
    const uint8_t trigPin = 12;  // ← Pin trigger del sensor HC-SR04
    const uint8_t echoPin = 13;  // ← Pin echo del sensor HC-SR04
    
    /**
     * 🏭 CREACIÓN DINÁMICA DEL SENSOR
     * Usar factory pattern para crear sensor según configuración
     */
    WaterLevelSensor* waterSensor = new WaterLevelSensor(trigPin, echoPin, &config_manager);
    
    // Configurar parámetros avanzados del sensor
    waterSensor->setMeasurementTimeout(30000);             // ← 30ms timeout para medición
    waterSensor->setNumberOfSamples(5);                    // ← 5 muestras para promedio
    waterSensor->setOutlierFilterThreshold(10.0);          // ← Filtro de valores atípicos
    waterSensor->enableTemperatureCompensation(true);      // ← Compensación por temperatura
    
    // Registrar sensor en el framework
    if (sensor_manager.addSensor(waterSensor)) {
        Serial.printf("  ✅ Sensor de agua registrado (ID: %d, Trig: %d, Echo: %d)\n", 
                      waterSensor->getSensorId(), trigPin, echoPin);
    } else {
        Serial.println("  ❌ Error registrando sensor de agua");
    }
    
    /**
     * 🌡️ INICIALIZACIÓN DE SENSORES AUXILIARES (OPCIONAL)
     * Configurar sensores adicionales si están disponibles
     */
    initializeAuxiliarySensors();
    
    /**
     * 🚀 INICIALIZACIÓN DEL SENSOR MANAGER
     * El SensorManager coordina todos los sensores del sistema
     */
    if (!sensor_manager.begin()) {
        Serial.println("  ❌ Error inicializando SensorManager");
        Serial.println("     → Algunas funciones de sensores no estarán disponibles");
    } else {
        Serial.printf("  ✅ SensorManager inicializado (%d sensores registrados)\n", 
                      sensor_manager.getSensorCount());
        
        // Mostrar información de sensores registrados
        displaySensorInformation();
    }
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // 🌐 FASE 7: INICIALIZACIÓN DE CONECTIVIDAD
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    Serial.println("└─ FASE 6 completada");
    Serial.println("┌─ FASE 7: Inicializando conectividad de red");
    
    /**
     * 📶 INICIALIZACIÓN DE CONECTIVIDAD
     * Configurar WiFi, ESP-NOW y otros protocolos de comunicación
     */
    initializeNetwork();
    
    // Actualizar display con estado de conectividad
    if (displayAvailable) {
        updateConnectivityDisplay();
    }
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // ☁️ FASE 8: CONFIGURACIÓN DE INTEGRACIONES IOT
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    Serial.println("└─ FASE 7 completada");
    Serial.println("┌─ FASE 8: Configurando integraciones IoT");
    
    /**
     * 🕐 SINCRONIZACIÓN NTP PARA TIMESTAMPS PRECISOS
     * Configurar sincronización de tiempo via NTP antes que otras integraciones
     * porque muchos servicios cloud requieren timestamps precisos
     */
    Serial.println("  🕐 Configurando sincronización NTP...");
    initializeNTPSync();
    
    /**
     * 📡 INICIALIZACIÓN DE ESP-NOW MESH NETWORK
     * Configurar red mesh para comunicación con otros sensores ESP32
     */
    Serial.println("  📡 Inicializando red mesh ESP-NOW...");
    initializeESPNowMesh();
    
    /**
     * 🤖 INTEGRACIÓN CON ALEXA
     * Configurar integración con Amazon Alexa si está habilitada
     */
    if (config_manager.isAlexaEnabled()) {
        Serial.println("  🤖 Inicializando integración Alexa...");
        initializeAlexaIntegration();
    }
    
    /**
     * 🏠 INTEGRACIÓN CON GOOGLE HOME
     * Configurar integración con Google Assistant si está habilitada
     */
    if (config_manager.isGoogleHomeEnabled()) {
        Serial.println("  🏠 Inicializando integración Google Home...");
        initializeGoogleHomeIntegration();
    }
    
    /**
     * 🏢 INTEGRACIÓN CON TUYA IOT
     * Configurar integración con plataforma Tuya IoT si está habilitada
     */
    if (config_manager.isTuyaEnabled()) {
        Serial.println("  🏢 Inicializando integración Tuya IoT...");
        initializeTuyaIntegration();
    }
    
    /**
     * 📊 CLIENTE MQTT PARA BROKERS
     * Configurar cliente MQTT si está habilitado
     */
    if (config_manager.isMQTTEnabled()) {
        Serial.println("  📊 Inicializando cliente MQTT...");
        initializeMQTTClient();
    }
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // 🔧 FASE 9: INICIALIZACIÓN DE SERVICIOS AUXILIARES
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    Serial.println("└─ FASE 8 completada");
    Serial.println("┌─ FASE 9: Inicializando servicios auxiliares");
    
    /**
     * 📝 SISTEMA DE LOGGING AVANZADO
     * Configurar logging con múltiples niveles y destinos
     */
    Serial.println("  📝 Inicializando sistema de logging...");
    initializeLoggingSystem();
    
    /**
     * 🔒 GESTOR DE SEGURIDAD
     * Configurar características de seguridad del sistema
     */
    Serial.println("  🔒 Inicializando gestor de seguridad...");
    initializeSecurityManager();
    
    /**
     * 🚀 GESTOR DE CACHE PARA PERFORMANCE
     * Configurar sistema de cache para optimizar performance
     */
    Serial.println("  🚀 Inicializando gestor de cache...");
    initializeCacheManager();
    
    /**
     * 📊 SISTEMA DE MÉTRICAS Y MONITOREO
     * Configurar recolección de métricas en tiempo real
     */
    Serial.println("  📊 Inicializando sistema de métricas...");
    initializeMetricsSystem();
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // ✅ FASE 10: VERIFICACIÓN FINAL Y ACTIVACIÓN
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    Serial.println("└─ FASE 9 completada");
    Serial.println("┌─ FASE 10: Verificación final y activación del sistema");
    
    /**
     * 🔍 VERIFICACIÓN DE COMPONENTES CRÍTICOS
     * Verificar que todos los componentes críticos estén funcionando
     */
    bool systemHealthy = performSystemHealthCheck();
    
    if (systemHealthy) {
        Serial.println("  ✅ Verificación de salud del sistema: EXITOSA");
        
        /**
         * 🚀 ACTIVACIÓN COMPLETA DEL SISTEMA
         * Habilitar todas las funcionalidades y servicios
         */
        activateAllServices();
        
        /**
         * 📊 REGISTRO DE MÉTRICAS DE INICIALIZACIÓN
         * Guardar métricas del proceso de inicialización
         */
        unsigned long initializationTime = millis() - systemStartTime;
        uint32_t finalFreeHeap = ESP.getFreeHeap();
        uint32_t memoryUsed = initialFreeHeap - finalFreeHeap;
        
        Serial.printf("  📊 Métricas de inicialización:\n");
        Serial.printf("     ⏱️ Tiempo total: %lu ms\n", initializationTime);
        Serial.printf("     💾 Memoria usada: %d bytes (%.1f KB)\n", memoryUsed, memoryUsed/1024.0);
        Serial.printf("     💾 Memoria libre final: %d bytes (%.1f KB)\n", finalFreeHeap, finalFreeHeap/1024.0);
        Serial.printf("     🎯 Eficiencia memoria: %.1f%%\n", (finalFreeHeap*100.0)/heapSize);
        
        /**
         * 📱 ACTUALIZACIÓN FINAL DEL DISPLAY
         * Mostrar estado final del sistema en la pantalla
         */
        if (displayAvailable) {
            display_manager.clearDisplay();
            display_manager.drawString(0, 0, "Sistema Activo");
            display_manager.drawString(0, 16, wifi_connected ? "WiFi: Conectado" : "WiFi: Portal");
            display_manager.drawString(0, 32, String("Sensores: " + String(sensor_manager.getSensorCount())));
            display_manager.drawString(0, 48, String("Heap: " + String(finalFreeHeap/1024) + " KB"));
            display_manager.display();
        }
        
        /**
         * 🎉 MENSAJE DE ÉXITO FINAL
         * Confirmar que el sistema está completamente operativo
         */
        Serial.println("└─ FASE 10 completada");
        Serial.println();
        Serial.println("🎉 ¡INICIALIZACIÓN COMPLETADA EXITOSAMENTE!");
        Serial.println();
        Serial.println("████████████████████████████████████████████████████████████████████████");
        Serial.println("█                                                                      █");
        Serial.println("█              ✅ SISTEMA COMPLETAMENTE OPERATIVO                      █");
        Serial.println("█                                                                      █");
        Serial.println("█  🌐 Servidor Web: http://[IP_ESP32]                                 █");
        Serial.println("█  🔧 Portal Config: http://[IP_ESP32]/captive-portal                 █");
        Serial.println("█  📡 ESP-NOW Mesh: http://[IP_ESP32]/esp-now                         █");
        Serial.println("█  📊 Dashboard: http://[IP_ESP32]/dashboard                           █");
        Serial.println("█                                                                      █");
        Serial.println("████████████████████████████████████████████████████████████████████████");
        Serial.println();
        
    } else {
        Serial.println("  ⚠️ Verificación de salud del sistema: FALLÓ");
        Serial.println("     → Sistema operando en modo degradado");
        Serial.println("     → Algunas funciones pueden no estar disponibles");
        
        // Activar solo servicios básicos en modo degradado
        activateBasicServices();
        
        if (displayAvailable) {
            display_manager.clearDisplay();
            display_manager.drawString(0, 0, "Modo Degradado");
            display_manager.drawString(0, 16, "Funciones Basicas");
            display_manager.drawString(0, 32, "Check Serial Log");
            display_manager.display();
        }
    }
    
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    // 🔄 INICIALIZACIÓN DE VARIABLES DE CONTROL
    // ═══════════════════════════════════════════════════════════════════════════════════════════════════════════
    
    connectionCheckTimer = 0;
    checkingConnection = false;
    lastConnectivityCheck = millis();
    
    // Reset final del watchdog
    esp_task_wdt_reset();
}