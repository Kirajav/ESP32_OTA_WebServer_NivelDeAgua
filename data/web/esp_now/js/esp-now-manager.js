/**
 * ========================================
 * ESP-NOW NETWORK MANAGER JAVASCRIPT
 * Professional Mesh Network Management Interface
 * ========================================
 */

// ========================================
// GLOBAL VARIABLES
// ========================================

let networkData = {
    connectedDevices: [],
    availableDevices: [],
    networkStats: {
        status: 'active',
        connectedCount: 0,
        dataRate: 0,
        signalStrength: -30
    }
};

// ========================================
// INITIALIZATION
// ========================================

/**
 * Initialize when DOM is loaded
 */
document.addEventListener('DOMContentLoaded', function() {
    console.log('🚀 ESP-NOW Manager iniciado');
    initializeESPNowManager();
    loadNetworkData();
    startDataPolling();
});

/**
 * Initialize ESP-NOW manager
 */
function initializeESPNowManager() {
    updateNetworkStats();
    renderNetworkTopology();
    renderDeviceLists();
    setupEventListeners();
}

/**
 * Setup event listeners
 */
function setupEventListeners() {
    // Add any global event listeners here
    console.log('Event listeners configured');
}

// ========================================
// DATA MANAGEMENT
// ========================================

/**
 * Load network data from server
 */
function loadNetworkData() {
    // Load network info
    fetch('/api/esp-now/info')
        .then(response => response.json())
        .then(data => {
            networkData.networkStats.status = data.status;
            networkData.networkStats.connectedCount = data.connectedDevices || 0;
            updateNetworkStats();
        })
        .catch(error => {
            console.error('Error cargando datos de red:', error);
            showToast('Error cargando datos de red', 'error');
        });

    // Load connected devices
    fetch('/multi-sensor-data')
        .then(response => response.json())
        .then(data => {
            networkData.connectedDevices = data.sensors || [];
            renderDeviceLists();
            renderNetworkTopology();
        })
        .catch(error => {
            console.error('Error cargando dispositivos:', error);
        });
}

/**
 * Update network statistics display
 */
function updateNetworkStats() {
    const statusElement = document.getElementById('network-status');
    const countElement = document.getElementById('connected-count');
    const rateElement = document.getElementById('data-rate');
    const strengthElement = document.getElementById('signal-strength');

    if (statusElement) {
        statusElement.textContent = networkData.networkStats.status === 'active' ? 'Activo' : 'Inactivo';
    }
    if (countElement) {
        countElement.textContent = networkData.networkStats.connectedCount;
    }
    if (rateElement) {
        rateElement.textContent = networkData.networkStats.dataRate;
    }
    if (strengthElement) {
        strengthElement.textContent = networkData.networkStats.signalStrength + ' dBm';
    }
}

// ========================================
// NETWORK VISUALIZATION
// ========================================

/**
 * Render network topology visualization
 */
function renderNetworkTopology() {
    const topology = document.getElementById('network-topology');
    if (!topology) return;

    topology.innerHTML = '';

    // Master node (this device)
    const masterNode = document.createElement('div');
    masterNode.className = 'master-node';
    masterNode.innerHTML = `
        <span class="node-icon">👑</span>
        <div class="node-name">ESP32 Maestro</div>
        <div class="node-details">
            IP: ${getLocalIP()}<br>
            MAC: ${getMacAddress()}<br>
            Rol: Coordinador
        </div>
    `;
    topology.appendChild(masterNode);

    // Slave nodes
    networkData.connectedDevices.forEach((device, index) => {
        const slaveNode = document.createElement('div');
        slaveNode.className = 'slave-node';
        slaveNode.innerHTML = `
            <span class="node-icon">📡</span>
            <div class="node-name">${device.sensorName || 'ESP32 Sensor'}</div>
            <div class="node-details">
                ID: ${device.sensorId}<br>
                Nivel: ${device.waterLevel}L<br>
                Batería: ${device.batteryPercent}%<br>
                Señal: ${device.signalStrength}%
            </div>
        `;
        topology.appendChild(slaveNode);
    });

    // Empty state if no devices
    if (networkData.connectedDevices.length === 0) {
        const noDevices = document.createElement('div');
        noDevices.className = 'empty-state';
        noDevices.innerHTML = `
            <div class="empty-state-icon">🔍</div>
            <div class="empty-state-message">No hay dispositivos conectados</div>
            <div class="empty-state-hint">Busca dispositivos cercanos para expandir tu red</div>
        `;
        topology.appendChild(noDevices);
    }
}

/**
 * Render device lists
 */
function renderDeviceLists() {
    renderConnectedDevices();
    renderAvailableDevices();
}

/**
 * Render connected devices list
 */
function renderConnectedDevices() {
    const container = document.getElementById('connected-devices');
    if (!container) return;

    container.innerHTML = '';

    if (networkData.connectedDevices.length === 0) {
        container.innerHTML = `
            <div class="empty-state">
                <div class="empty-state-icon">📱</div>
                <div class="empty-state-message">No hay dispositivos conectados</div>
            </div>
        `;
        return;
    }

    networkData.connectedDevices.forEach(device => {
        const deviceElement = document.createElement('div');
        deviceElement.className = 'device-item';
        deviceElement.innerHTML = `
            <div class="device-info">
                <div class="device-name">${device.sensorName || 'ESP32 Sensor'}</div>
                <div class="device-details">
                    ID: ${device.sensorId} | 
                    Nivel: ${device.waterLevel}L | 
                    Batería: ${device.batteryPercent}%
                </div>
                <div class="device-status">
                    <span class="status-dot status-online"></span>
                    <span>Online - Señal: ${device.signalStrength}%</span>
                </div>
            </div>
            <div class="device-actions">
                <button class="action-btn-small" onclick="pingDevice('${device.sensorId}')">Ping</button>
                <button class="action-btn-small" onclick="configureDevice('${device.sensorId}')">Config</button>
                <button class="action-btn-small" onclick="disconnectDevice('${device.sensorId}')">Desconectar</button>
            </div>
        `;
        container.appendChild(deviceElement);
    });
}

/**
 * Render available devices list
 */
function renderAvailableDevices() {
    const container = document.getElementById('available-devices');
    if (!container) return;

    container.innerHTML = '';

    if (networkData.availableDevices.length === 0) {
        container.innerHTML = `
            <div class="empty-state">
                <div class="empty-state-icon">🔍</div>
                <div class="empty-state-message">No hay dispositivos disponibles</div>
                <div class="empty-state-hint">Usa "Buscar Dispositivos" para escanear</div>
            </div>
        `;
        return;
    }

    networkData.availableDevices.forEach(device => {
        const deviceElement = document.createElement('div');
        deviceElement.className = 'device-item';
        deviceElement.innerHTML = `
            <div class="device-info">
                <div class="device-name">${device.name || 'ESP32 Detectado'}</div>
                <div class="device-details">
                    MAC: ${device.mac}<br>
                    Señal: ${device.rssi} dBm | Canal: ${device.channel}
                </div>
                <div class="device-status">
                    <span class="status-dot status-offline"></span>
                    <span>Disponible para conexión</span>
                </div>
            </div>
            <div class="device-actions">
                <button class="action-btn-small" onclick="connectDevice('${device.mac}')">Conectar</button>
                <button class="action-btn-small" onclick="pingDevice('${device.mac}')">Ping</button>
            </div>
        `;
        container.appendChild(deviceElement);
    });
}

// ========================================
// NETWORK CONTROL FUNCTIONS
// ========================================

/**
 * Scan for available ESP-NOW devices
 */
function scanForDevices() {
    showToast('🔍 Buscando dispositivos ESP-NOW...', 'info');
    
    // Show scanning indicator
    const container = document.getElementById('available-devices');
    if (container) {
        container.innerHTML = `
            <div class="scanning-indicator">
                <div class="loading-spinner"></div>
                <span>Escaneando dispositivos...</span>
            </div>
        `;
    }
    
    fetch('/api/esp-now/scan', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            networkData.availableDevices = data.devices || [];
            renderAvailableDevices();
            showToast(`✅ Encontrados ${networkData.availableDevices.length} dispositivos`, 'success');
        })
        .catch(error => {
            console.error('Error en escaneo:', error);
            showToast('❌ Error buscando dispositivos', 'error');
            renderAvailableDevices(); // Reset to empty state
        });
}

/**
 * Connect to an ESP-NOW device
 */
function connectDevice(mac) {
    showToast('🔗 Conectando dispositivo...', 'info');
    
    fetch('/api/esp-now/connect', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ mac: mac })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast('✅ Dispositivo conectado exitosamente', 'success');
            loadNetworkData(); // Reload data
        } else {
            showToast('❌ Error conectando dispositivo: ' + (data.error || 'Unknown'), 'error');
        }
    })
    .catch(error => {
        console.error('Error conectando:', error);
        showToast('❌ Error de conexión', 'error');
    });
}

/**
 * Ping a specific device
 */
function pingDevice(deviceId) {
    showToast('🏓 Enviando ping...', 'info');
    
    fetch('/api/esp-now/ping', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ mac: deviceId })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast(`✅ Ping exitoso: ${data.responseTime || '< 10'}ms`, 'success');
        } else {
            showToast('❌ Ping falló - dispositivo no responde', 'error');
        }
    })
    .catch(error => {
        console.error('Error en ping:', error);
        showToast('❌ Error en ping', 'error');
    });
}

/**
 * Configure this device as master
 */
function configureAsMaster() {
    showToast('👑 Configurando como maestro...', 'info');
    
    fetch('/api/esp-now/configure', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
            role: 'master',
            deviceName: 'ESP32_Master',
            sensorId: 1,
            channel: 6,
            encryption: 'wep'
        })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast('✅ Configurado como maestro exitosamente', 'success');
            loadNetworkData();
        } else {
            showToast('❌ Error en configuración: ' + (data.error || 'Unknown'), 'error');
        }
    })
    .catch(error => {
        console.error('Error configurando:', error);
        showToast('❌ Error en configuración', 'error');
    });
}

/**
 * Broadcast ping to all devices
 */
function broadcastPing() {
    showToast('📡 Enviando ping a todos los dispositivos...', 'info');
    
    fetch('/api/esp-now/test', { method: 'POST' })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                const deviceCount = data.devicesFound || networkData.connectedDevices.length;
                showToast(`✅ Red activa: ${deviceCount} dispositivos respondieron`, 'success');
            } else {
                showToast('⚠️ No hay dispositivos activos en la red', 'warning');
            }
        })
        .catch(error => {
            console.error('Error en ping general:', error);
            showToast('❌ Error en ping general', 'error');
        });
}

/**
 * Scan and auto-connect to first device found
 */
function scanAndConnect() {
    showToast('🔍 Buscando y conectando automáticamente...', 'info');
    
    scanForDevices();
    
    setTimeout(() => {
        // Auto-connect to first device found
        if (networkData.availableDevices.length > 0) {
            const firstDevice = networkData.availableDevices[0];
            showToast(`🔗 Conectando a ${firstDevice.name || 'dispositivo encontrado'}...`, 'info');
            connectDevice(firstDevice.mac);
        } else {
            showToast('❌ No se encontraron dispositivos para conectar', 'warning');
        }
    }, 3000);
}

/**
 * Reset ESP-NOW network
 */
function resetNetwork() {
    if (confirm('¿Estás seguro de reiniciar la red ESP-NOW? Se perderán todas las conexiones.')) {
        showToast('🔄 Reiniciando red ESP-NOW...', 'warning');
        
        fetch('/api/esp-now/reset', { method: 'POST' })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    showToast('✅ Red reiniciada exitosamente', 'success');
                    // Clear local data
                    networkData.connectedDevices = [];
                    networkData.availableDevices = [];
                    renderDeviceLists();
                    renderNetworkTopology();
                } else {
                    showToast('❌ Error reiniciando red', 'error');
                }
            })
            .catch(error => {
                console.error('Error reiniciando red:', error);
                showToast('❌ Error reiniciando red', 'error');
            });
    }
}

/**
 * Export network configuration
 */
function exportConfig() {
    const config = {
        networkData: networkData,
        timestamp: new Date().toISOString(),
        deviceInfo: {
            mac: getMacAddress(),
            ip: getLocalIP(),
            role: 'master'
        },
        metadata: {
            version: '1.0',
            generatedBy: 'ESP-NOW Manager'
        }
    };
    
    const blob = new Blob([JSON.stringify(config, null, 2)], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = `espnow-config-${new Date().toISOString().split('T')[0]}.json`;
    a.click();
    URL.revokeObjectURL(url);
    
    showToast('📁 Configuración exportada exitosamente', 'success');
}

/**
 * Show detailed network map (future enhancement)
 */
function showNetworkMap() {
    showToast('🗺️ Vista de mapa detallado en desarrollo', 'info');
    // TODO: Implement detailed network topology map
}

// ========================================
// DEVICE MANAGEMENT
// ========================================

/**
 * Configure a specific device
 */
function configureDevice(deviceId) {
    showToast('⚙️ Abriendo configuración de dispositivo...', 'info');
    
    // TODO: Implement device configuration interface
    setTimeout(() => {
        showToast('🚧 Función de configuración en desarrollo', 'warning');
    }, 1000);
}

/**
 * Disconnect a device from the network
 */
function disconnectDevice(deviceId) {
    const device = networkData.connectedDevices.find(d => d.sensorId === deviceId);
    const deviceName = device ? device.sensorName : `Dispositivo ${deviceId}`;
    
    if (confirm(`¿Desconectar "${deviceName}" de la red ESP-NOW?`)) {
        showToast('🔌 Desconectando dispositivo...', 'warning');
        
        fetch('/api/esp-now/disconnect', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ deviceId: deviceId })
        })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                showToast(`✅ "${deviceName}" desconectado exitosamente`, 'success');
                loadNetworkData(); // Reload data
            } else {
                showToast('❌ Error desconectando dispositivo', 'error');
            }
        })
        .catch(error => {
            console.error('Error desconectando:', error);
            showToast('❌ Error desconectando dispositivo', 'error');
        });
    }
}

// ========================================
// UTILITY FUNCTIONS
// ========================================

/**
 * Get local IP address (placeholder)
 */
function getLocalIP() {
    return '192.168.1.100'; // TODO: Get from server
}

/**
 * Get MAC address (placeholder)
 */
function getMacAddress() {
    return '24:0A:C4:XX:XX:XX'; // TODO: Get from server
}

/**
 * Start periodic data polling
 */
function startDataPolling() {
    setInterval(() => {
        loadNetworkData();
        // Simulate changing data rate
        networkData.networkStats.dataRate = Math.floor(Math.random() * 50) + 10;
        updateNetworkStats();
    }, 5000);
}

/**
 * Show toast notification
 */
function showToast(message, type = 'info') {
    // Remove existing toasts
    const existingToasts = document.querySelectorAll('.esp-toast');
    existingToasts.forEach(toast => toast.remove());
    
    const toast = document.createElement('div');
    toast.className = `esp-toast esp-toast-${type}`;
    toast.style.cssText = `
        position: fixed;
        top: 20px;
        right: 20px;
        background: var(--card-bg);
        color: var(--text-color);
        padding: 15px 20px;
        border-radius: 10px;
        border: 1px solid var(--border-color);
        backdrop-filter: blur(16px);
        box-shadow: 0 8px 30px rgba(0, 0, 0, 0.3);
        z-index: 10000;
        max-width: 350px;
        font-weight: 500;
        animation: espToastSlideIn 0.3s ease;
    `;
    
    // Add left border color based on type
    const borderColors = {
        success: 'var(--success-color)',
        error: 'var(--error-color)',
        warning: 'var(--warning-color)',
        info: 'var(--primary-color)'
    };
    toast.style.borderLeftColor = borderColors[type];
    toast.style.borderLeftWidth = '4px';
    
    toast.innerHTML = message;
    
    document.body.appendChild(toast);
    
    // Auto-remove after 4 seconds
    setTimeout(() => {
        toast.style.animation = 'espToastSlideOut 0.3s ease';
        setTimeout(() => {
            if (document.body.contains(toast)) {
                document.body.removeChild(toast);
            }
        }, 300);
    }, 4000);
}

// ========================================
// THEME SUPPORT
// ========================================

/**
 * Toggle theme (if theme toggle exists)
 */
function toggleTheme() {
    document.body.classList.toggle('light-theme');
    const isLight = document.body.classList.contains('light-theme');
    localStorage.setItem('espnow-theme', isLight ? 'light' : 'dark');
}

/**
 * Load saved theme
 */
function loadSavedTheme() {
    const savedTheme = localStorage.getItem('espnow-theme') || 'dark';
    if (savedTheme === 'light') {
        document.body.classList.add('light-theme');
    }
}

// ========================================
// EXPORT FUNCTIONS FOR GLOBAL ACCESS
// ========================================

// Make functions available globally for onclick handlers
window.scanForDevices = scanForDevices;
window.connectDevice = connectDevice;
window.pingDevice = pingDevice;
window.configureAsMaster = configureAsMaster;
window.broadcastPing = broadcastPing;
window.scanAndConnect = scanAndConnect;
window.resetNetwork = resetNetwork;
window.exportConfig = exportConfig;
window.showNetworkMap = showNetworkMap;
window.configureDevice = configureDevice;
window.disconnectDevice = disconnectDevice;
window.toggleTheme = toggleTheme;

// ========================================
// CSS ANIMATIONS (Injected)
// ========================================

// Inject CSS animations for toasts
const style = document.createElement('style');
style.textContent = `
    @keyframes espToastSlideIn {
        from { transform: translateX(100%); opacity: 0; }
        to { transform: translateX(0); opacity: 1; }
    }
    
    @keyframes espToastSlideOut {
        from { transform: translateX(0); opacity: 1; }
        to { transform: translateX(100%); opacity: 0; }
    }
`;
document.head.appendChild(style);

// Initialize theme on load
loadSavedTheme();