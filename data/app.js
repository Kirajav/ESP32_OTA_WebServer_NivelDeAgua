// === SMART WATER SENSOR DASHBOARD - MODERN VERSION ===
// Sistema de monitoreo inteligente con animaciones SVG

// --- Variables Globales ---
let websocket;
let isConnected = false;
let currentData = {};
let animationInterval;

// --- Inicialización ---
window.addEventListener("load", () => {
    console.log("🚀 Iniciando Smart Water Sensor Dashboard...");
    initWebSocket();
    initButtons();
    initAnimations();
    startPolling();
    loadDisplayStatus();
    setupMultiSensorButton();
    setupSensorConfig(); // ⚙️ Configuración del sensor
});

// --- WebSocket Avanzado ---
function initWebSocket() {
    const gateway = `ws://${window.location.hostname}/ws`;
    console.log("🔌 Conectando WebSocket...", gateway);
    
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
    websocket.onerror = onError;
}

function onOpen(event) {
    console.log("✅ WebSocket conectado");
    isConnected = true;
    updateConnectionStatus(true);
    showToast("Conectado al sensor", "success");
}

function onClose(event) {
    console.log("❌ WebSocket desconectado. Reintentando...");
    isConnected = false;
    updateConnectionStatus(false);
    setTimeout(initWebSocket, 2000);
}

function onError(event) {
    console.error("🚨 Error WebSocket:", event);
    showToast("Error de conexión", "error");
}

function onMessage(event) {
    console.log("📨 Mensaje WebSocket:", event.data);
    
    try {
        const data = JSON.parse(event.data);
        if (data.type === "sensorData") {
            updateSensorData(data);
        } else if (data.type === "displayStatus") {
            updateDisplayStatus(data.status);
        }
    } catch (e) {
        // Mensaje simple de display status
        if (event.data === "1") {
            updateDisplayStatus("Encendido");
        } else if (event.data === "0") {
            updateDisplayStatus("Apagado");
        }
    }
}

// --- Estado de Conexión Visual ---
function updateConnectionStatus(connected) {
    const statusDot = document.getElementById('sensor-status-dot');
    const statusText = document.getElementById('valor-estado-sensor');
    
    if (connected) {
        statusDot.style.background = '#4CAF50';
        statusText.textContent = 'Conectado';
        statusText.style.color = '#4CAF50';
    } else {
        statusDot.style.background = '#F44336';
        statusText.textContent = 'Desconectado';
        statusText.style.color = '#F44336';
    }
}

// --- Funciones de Display Status ---
function loadDisplayStatus() {
    fetch('/displayStatus')
        .then(response => response.json())
        .then(data => {
            console.log("📱 Estado display cargado:", data);
            updateDisplayStatus(data.status);
        })
        .catch(error => {
            console.error("❌ Error cargando estado display:", error);
            updateDisplayStatus("Desconocido");
        });
}

function updateDisplayStatus(status) {
    const statusElement = document.getElementById('valor-estado-oled');
    if (statusElement) {
        statusElement.textContent = status;
        console.log("📱 Display actualizado:", status);
    }
}

// --- Polling de Datos Mejorado ---
function startPolling() {
    console.log("🔄 Iniciando polling de datos...");
    
    const poll = async () => {
        try {
            const response = await fetch('/data');
            const data = await response.json();
            
            // Actualizar datos globales
            currentData = data;
            
            // Actualizar interfaz
            updateSensorData(data);
            updateTankAnimation(data);
            
        } catch (error) {
            console.error("❌ Error en polling:", error);
            showToast("Error obteniendo datos", "warning");
        }
    };
    
    // Poll inicial
    poll();
    
    // Poll cada 2 segundos
    setInterval(poll, 2000);
}

// --- Actualización de Datos del Sensor ---
function updateSensorData(data) {
    console.log("📊 Actualizando datos:", data);
    
    // Actualizar elementos de datos
    updateElement('Litros', data.litros);
    updateElement('Distancia', data.distancia);
    
    // Calcular y mostrar porcentaje
    const porcentaje = calculatePercentage(data.distancia);
    updateElement('Porcentaje', porcentaje);
    
    // Actualizar estado del sensor
    updateElement('valor-estado-sensor', data.estadoSensor || 'Conectado');
}

function updateElement(id, value) {
    const element = document.getElementById(id);
    if (element) {
        element.textContent = value || '--';
    }
}

function calculatePercentage(distancia) {
    if (!distancia || distancia === '--') return '--';
    
    const dist = parseFloat(distancia);
    if (isNaN(dist)) return '--';
    
    // Lógica de cálculo de porcentaje basada en el sensor
    // Distancia mínima: 19cm (100%), máxima: 200cm (0%)
    const minDist = 19;
    const maxDist = 200;
    
    let percentage;
    if (dist <= minDist) {
        percentage = 100;
    } else if (dist >= maxDist) {
        percentage = 0;
    } else {
        percentage = Math.round(((maxDist - dist) / (maxDist - minDist)) * 100);
    }
    
    return Math.max(0, Math.min(100, percentage));
}

// --- ANIMACIONES SVG DEL TANQUE ---
function initAnimations() {
    console.log("🎨 Inicializando animaciones SVG...");
    
    // Animación de ondas
    startWaveAnimation();
}

function updateTankAnimation(data) {
    if (!data.distancia || data.distancia === '--') return;
    
    const percentage = calculatePercentage(data.distancia);
    if (percentage === '--') return;
    
    animateWaterLevel(percentage);
    animateLevelBar(percentage);
    updateWaterColor(percentage);
}

function animateWaterLevel(percentage) {
    const waterLevel = document.getElementById('water-level');
    const waterWaves = document.getElementById('water-waves');
    
    if (waterLevel && waterWaves) {
        // Altura máxima del tanque (270px desde y=80 hasta y=350)
        const maxHeight = 270;
        const height = (percentage / 100) * maxHeight;
        const yPosition = 350 - height;
        
        // Animar nivel de agua
        waterLevel.style.transition = 'all 1.5s ease-in-out';
        waterLevel.setAttribute('y', yPosition);
        waterLevel.setAttribute('height', height);
        
        // Animar ondas
        waterWaves.style.transition = 'all 1.5s ease-in-out';
        waterWaves.setAttribute('y', yPosition);
        waterWaves.setAttribute('height', height);
        
        console.log(`🌊 Nivel animado: ${percentage}% (altura: ${height}px)`);
    }
}

function animateLevelBar(percentage) {
    const levelBar = document.getElementById('level-bar');
    
    if (levelBar) {
        const maxHeight = 270;
        const height = (percentage / 100) * maxHeight;
        const yPosition = 348 - height;
        
        levelBar.style.transition = 'all 1.5s ease-in-out';
        levelBar.setAttribute('y', yPosition);
        levelBar.setAttribute('height', height);
    }
}

function updateWaterColor(percentage) {
    const waterLevel = document.getElementById('water-level');
    
    if (waterLevel) {
        let color1, color2, color3;
        
        if (percentage >= 75) {
            // Verde para nivel alto
            color1 = '#4CAF50'; color2 = '#66BB6A'; color3 = '#2E7D32';
        } else if (percentage >= 50) {
            // Azul para nivel medio
            color1 = '#4FC3F7'; color2 = '#29B6F6'; color3 = '#0288D1';
        } else if (percentage >= 25) {
            // Naranja para nivel bajo
            color1 = '#FFB74D'; color2 = '#FF9800'; color3 = '#F57C00';
        } else {
            // Rojo para nivel crítico
            color1 = '#EF5350'; color2 = '#F44336'; color3 = '#C62828';
        }
        
        // Actualizar gradiente
        const gradient = document.getElementById('waterGradient');
        if (gradient) {
            const stops = gradient.querySelectorAll('stop');
            if (stops.length >= 3) {
                stops[0].setAttribute('style', `stop-color:${color1};stop-opacity:0.9`);
                stops[1].setAttribute('style', `stop-color:${color2};stop-opacity:0.8`);
                stops[2].setAttribute('style', `stop-color:${color3};stop-opacity:1`);
            }
        }
    }
}

function startWaveAnimation() {
    const wavePattern = document.getElementById('wavePattern');
    if (wavePattern) {
        let offset = 0;
        setInterval(() => {
            offset += 2;
            if (offset >= 100) offset = 0;
            wavePattern.setAttribute('x', offset);
        }, 100);
    }
}

// --- Botones Modernos ---
function initButtons() {
    console.log("🔘 Inicializando botones...");
    
    const toggleButton = document.getElementById('button-toggle');
    const resetButton = document.getElementById('button-reset');
    const wifiButton = document.getElementById('wifi-config-button');
    
    if (toggleButton) {
        toggleButton.addEventListener('click', toggleDisplay);
    }
    
    if (resetButton) {
        resetButton.addEventListener('click', resetESP32);
    }
    
    if (wifiButton) {
        wifiButton.addEventListener('click', resetWiFiConfig);
    }
}

function toggleDisplay() {
    console.log("🔄 Alternando display...");
    showToast("Alternando display OLED...", "info");
    
    fetch('/toggle', { method: 'POST' })
        .then(response => response.text())
        .then(data => {
            console.log("✅ Display alternado:", data);
            showToast("Display alternado correctamente", "success");
        })
        .catch(error => {
            console.error("❌ Error alternando display:", error);
            showToast("Error alternando display", "error");
        });
}

function resetESP32() {
    if (confirm("⚠️ ¿Estás seguro de que quieres reiniciar el ESP32?")) {
        console.log("🔄 Reiniciando ESP32...");
        showToast("Reiniciando ESP32...", "warning");
        
        fetch('/reset', { method: 'POST' })
            .then(() => {
                showToast("ESP32 reiniciado. Reconectando...", "info");
                setTimeout(() => location.reload(), 3000);
            })
            .catch(error => {
                console.error("❌ Error reiniciando:", error);
                showToast("Error reiniciando ESP32", "error");
            });
    }
}

function resetWiFiConfig() {
    if (confirm("⚠️ ¿Reiniciar configuración WiFi? El dispositivo volverá al modo configuración.")) {
        console.log("📶 Reiniciando configuración WiFi...");
        showToast("Reiniciando configuración WiFi...", "warning");
        
        fetch('/wifi-reset', { method: 'POST' })
            .then(() => {
                showToast("Configuración WiFi reiniciada", "info");
                setTimeout(() => location.reload(), 2000);
            })
            .catch(error => {
                console.error("❌ Error reiniciando WiFi:", error);
                showToast("Error reiniciando WiFi", "error");
            });
    }
}

// --- Multi-Sensor ESP-NOW ---
function setupMultiSensorButton() {
    const multiSensorBtn = document.getElementById('multi-sensor-btn');
    if (multiSensorBtn) {
        multiSensorBtn.addEventListener('click', openMultiSensorDashboard);
    }
}

function openMultiSensorDashboard() {
    console.log("🔗 Abriendo ESP-NOW Manager...");
    showToast("Abriendo gestión ESP-NOW", "info");
    
    // Abrir página completa de gestión ESP-NOW
    window.open('/espnow-manager', '_blank');
}

// --- Sistema de Toast Moderno ---
function showToast(message, type = 'info') {
    const toast = document.getElementById('toast');
    const toastIcon = document.getElementById('toast-icon');
    const toastMessage = document.getElementById('toast-message');
    
    if (!toast || !toastIcon || !toastMessage) return;
    
    // Configurar icono según tipo
    const icons = {
        success: '✅',
        error: '❌',
        warning: '⚠️',
        info: 'ℹ️'
    };
    
    toastIcon.textContent = icons[type] || icons.info;
    toastMessage.textContent = message;
    
    // Mostrar toast
    toast.classList.add('show');
    console.log(`📢 Toast: ${message} (${type})`);
    
    // Ocultar después de 4 segundos
    setTimeout(() => {
        toast.classList.remove('show');
    }, 4000);
}

// --- Funciones de Utilidad ---
function formatValue(value) {
    if (value === null || value === undefined || value === '') {
        return '--';
    }
    return value.toString();
}

// --- 🎛️ CONFIGURACIÓN DEL SENSOR ---
function setupSensorConfig() {
    const configButton = document.getElementById('sensor-config-button');
    if (configButton) {
        configButton.addEventListener('click', (e) => {
            e.preventDefault();
            showSensorConfigModal();
        });
    }
}

async function showSensorConfigModal() {
    try {
        const response = await fetch('/api/sensor/config');
        const config = await response.json();
        
        if (config.success) {
            createSensorConfigModal(config);
        } else {
            showToast('Error al cargar configuración', 'error');
        }
    } catch (error) {
        console.error('Error:', error);
        showToast('Error de conexión', 'error');
    }
}

function createSensorConfigModal(config) {
    // Crear modal dinámicamente
    const modalHTML = `
        <div id="sensor-config-modal" class="modal-overlay">
            <div class="modal-content">
                <div class="modal-header">
                    <h2>⚙️ Configuración del Sensor</h2>
                    <button class="modal-close" onclick="closeSensorConfigModal()">&times;</button>
                </div>
                <div class="modal-body">
                    <div class="config-section">
                        <h3>📏 Configuración Básica</h3>
                        <div class="form-group">
                            <label>Altura del Tanque (cm)</label>
                            <input type="number" id="tank-height" value="${config.tank_height}" min="50" max="500">
                        </div>
                        <div class="form-group">
                            <label>Capacidad (litros)</label>
                            <input type="number" id="tank-capacity" value="${config.tank_capacity}" min="100" max="10000">
                        </div>
                        <div class="form-group">
                            <label>Distancia Mínima (cm)</label>
                            <input type="number" id="min-distance" value="${config.min_distance}" min="5" max="50">
                        </div>
                    </div>
                    
                    <div class="config-section">
                        <h3>⏱️ Intervalos Inteligentes</h3>
                        <div class="form-group">
                            <label>🐌 Intervalo Normal (segundos)</label>
                            <input type="number" id="normal-interval" value="${config.normal_interval}" min="15" max="300">
                            <small>Frecuencia cuando el tanque está estable</small>
                        </div>
                        <div class="form-group">
                            <label>🚰 Intervalo de Llenado (segundos)</label>
                            <input type="number" id="filling-interval" value="${config.filling_interval}" min="3" max="10">
                            <small>Frecuencia cuando se detecta llenado</small>
                        </div>
                        <div class="form-group">
                            <label>🔢 Umbral de Detección</label>
                            <input type="number" id="filling-threshold" value="${config.filling_threshold}" min="2" max="5">
                            <small>Lecturas incrementales para detectar llenado</small>
                        </div>
                    </div>
                    
                    <div class="config-section">
                        <h3>🌍 Configuración Geográfica</h3>
                        <div class="form-group">
                            <label>
                                <input type="checkbox" id="auto-geo" ${config.auto_geo_location ? 'checked' : ''}>
                                🌐 Detectar ubicación automáticamente
                            </label>
                        </div>
                        <div class="form-group">
                            <label>🕐 Zona Horaria</label>
                            <input type="text" id="timezone" value="${config.timezone}" placeholder="America/Mexico_City">
                        </div>
                        <div class="form-group">
                            <label>
                                <input type="checkbox" id="show-datetime" ${config.show_datetime ? 'checked' : ''}>
                                📅 Mostrar fecha/hora en pantalla
                            </label>
                        </div>
                    </div>
                </div>
                <div class="modal-footer">
                    <button class="modern-btn secondary" onclick="closeSensorConfigModal()">Cancelar</button>
                    <button class="modern-btn primary" onclick="saveSensorConfig()">💾 Guardar</button>
                </div>
            </div>
        </div>
    `;
    
    document.body.insertAdjacentHTML('beforeend', modalHTML);
}

async function saveSensorConfig() {
    const config = {
        tank_height: parseFloat(document.getElementById('tank-height').value),
        tank_capacity: parseFloat(document.getElementById('tank-capacity').value),
        min_distance: parseFloat(document.getElementById('min-distance').value),
        normal_interval: parseInt(document.getElementById('normal-interval').value),
        filling_interval: parseInt(document.getElementById('filling-interval').value),
        filling_threshold: parseInt(document.getElementById('filling-threshold').value),
        auto_geo_location: document.getElementById('auto-geo').checked,
        timezone: document.getElementById('timezone').value,
        show_datetime: document.getElementById('show-datetime').checked,
        container_type: 0 // Default to tank
    };
    
    try {
        const response = await fetch('/api/sensor/config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(config)
        });
        
        const result = await response.json();
        
        if (result.success) {
            showToast('⚙️ Configuración guardada exitosamente', 'success');
            closeSensorConfigModal();
        } else {
            showToast('❌ Error: ' + result.message, 'error');
        }
    } catch (error) {
        console.error('Error:', error);
        showToast('❌ Error de conexión', 'error');
    }
}

function closeSensorConfigModal() {
    const modal = document.getElementById('sensor-config-modal');
    if (modal) {
        modal.remove();
    }
}

// --- Debug y Logging ---
function logSensorData() {
    console.table(currentData);
}

// Exponer funciones globales para debug
window.smartSensor = {
    data: currentData,
    isConnected,
    logData: logSensorData,
    showToast,
    updateTankAnimation: () => updateTankAnimation(currentData),
    showSensorConfig: showSensorConfigModal
};

console.log("💧 Smart Water Sensor Dashboard cargado completamente!");