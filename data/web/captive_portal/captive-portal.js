/**
 * ========================================
 * CAPTIVE PORTAL JAVASCRIPT
 * Professional WiFi Configuration Interface
 * ========================================
 */

// ========================================
// TRANSLATIONS & INTERNATIONALIZATION
// ========================================
const translations = {
    es: {
        page_title: "Portal de Configuración",
        header_title: "Sensor de Nivel",
        header_subtitle: "Portal de Configuración",
        wifi_config: "WiFi",
        sensor_config: "Sensor",
        ap_config: "Punto de Acceso",
        system_config: "Sistema",
        actions: "Acciones",
        saved_networks: "Redes Guardadas",
        no_saved_networks: "No hay redes guardadas",
        add_new_network: "Agregar Nueva Red",
        scan_networks: "Escanear Redes",
        network_name: "Nombre de Red (SSID)",
        password: "Contraseña",
        wifi_password_help: "Mínimo 8 caracteres para mayor seguridad.",
        make_default: "Establecer como predeterminada",
        add_network: "Agregar Red",
        tank_height: "Altura del Tanque (cm)",
        tank_height_help: "Distancia total desde el fondo del tanque hasta la parte superior.",
        tank_capacity: "Capacidad del Tanque (litros)",
        tank_capacity_help: "Volumen máximo de agua cuando el tanque está lleno.",
        min_distance: "Distancia Mínima del Sensor (cm)",
        min_distance_help: "Distancia mínima que el sensor puede medir con precisión.",
        container_type: "Tipo de Contenedor",
        container_type_help: "Contenedor sobre el cual se va a sensar su volumen.",
        tank: "Tinaco",
        cistern: "Cisterna",
        generic: "Contenedor Genérico",
        
        // Intervalos inteligentes
        normal_interval: "⏱️ Intervalo Normal de Sensado (segundos)",
        normal_interval_help: "Frecuencia de lectura cuando el tanque está estable. Recomendado: 15-60 segundos para ahorrar energía.",
        filling_interval: "🚰 Intervalo de Llenado (segundos)",
        filling_interval_help: "Frecuencia de lectura cuando se detecta que se está llenando. Recomendado: 3-10 segundos para monitoreo preciso.",
        filling_threshold: "🔢 Umbral de Detección de Llenado",
        filling_threshold_help: "Número de lecturas consecutivas incrementales para considerar que se está llenando. Mínimo 2, máximo 5 lecturas.",
        
        // Geolocalización NTP
        auto_geo_location: "🌐 Detectar Ubicación Automáticamente",
        auto_geo_help: "Detecta automáticamente la zona horaria basada en tu ubicación IP para mostrar fecha/hora correcta.",
        timezone: "🕐 Zona Horaria Manual (opcional)",
        timezone_help: "Solo si desactivas la detección automática. Formato: America/Mexico_City",
        show_datetime: "📅 Mostrar Fecha/Hora en Pantalla",
        datetime_help: "Muestra la fecha y hora actual en la pantalla OLED del dispositivo.",
        
        save_sensor: "Guardar",
        ap_name: "Nombre del AP",
        ap_name_help: "Nombre que aparecerá en la lista de redes WiFi.",
        ap_password: "Contraseña del AP",
        ap_password_help: "De 8 a 10 caracteres. Simple y fácil de recordar.",
        device_name: "Nombre del Dispositivo (Hostname)",
        device_name_help: "Nombre para identificar el dispositivo en la red local (sin espacios).",
        save_ap: "Guardar",
        auto_sleep_time: "Apagado Automático de Pantalla",
        auto_sleep_help: "Tiempo para que la pantalla se apague y ahorrar energía.",
        update_interval: "Intervalo de Lectura del Sensor (segundos)",
        update_interval_help: "Frecuencia con la que el sensor mide el nivel del agua.",
        save_system: "Guardar",
        save_all_title: "Guardar Todo",
        save_all_desc: "Guarda todos los cambios de todas las secciones sin reiniciar.",
        save_all_desc_short: "Guarda todos los ajustes.",
        save_and_restart_title: "Aplicar y Conectar",
        save_and_restart_desc: "Guarda la configuración y reinicia el dispositivo para conectar al WiFi.",
        save_and_restart_desc_short: "Guarda y reinicia con WiFi.",
        restart_title: "Reiniciar",
        restart_desc: "Reinicia el dispositivo. Los cambios no guardados se perderán.",
        restart_desc_short: "Reinicia el dispositivo.",
        factory_reset_title: "Reset de Fábrica",
        factory_reset_desc: "Borra toda la configuración y restaura los valores de fábrica.",
        factory_reset_desc_short: "Borra la configuración.",
        default_network: "Predeterminada",
        saved_network: "Guardada",
        set_as_default: "Hacer Predeterminada",
        remove_network: "Eliminar"
    },
    en: {
        page_title: "Configuration Portal",
        header_title: "Water Level Sensor",
        header_subtitle: "Configuration Portal",
        wifi_config: "WiFi",
        sensor_config: "Sensor",
        ap_config: "Access Point",
        system_config: "System",
        actions: "Actions",
        saved_networks: "Saved Networks",
        no_saved_networks: "No saved networks",
        add_new_network: "Add New Network",
        scan_networks: "Scan Networks",
        network_name: "Network Name (SSID)",
        password: "Password",
        wifi_password_help: "Minimum 8 characters for better security.",
        make_default: "Set as default",
        add_network: "Add Network",
        tank_height: "Tank Height (cm)",
        tank_height_help: "Total distance from tank bottom to top.",
        tank_capacity: "Tank Capacity (liters)",
        tank_capacity_help: "Maximum water volume when tank is full.",
        min_distance: "Minimum Sensor Distance (cm)",
        min_distance_help: "Minimum distance the sensor can measure accurately.",
        container_type: "Container Type",
        container_type_help: "Container type to measure volume.",
        tank: "Tank",
        cistern: "Cistern",
        generic: "Generic Container",
        
        // Smart intervals
        normal_interval: "⏱️ Normal Sensing Interval (seconds)",
        normal_interval_help: "Reading frequency when tank is stable. Recommended: 15-60 seconds to save energy.",
        filling_interval: "🚰 Filling Interval (seconds)",
        filling_interval_help: "Reading frequency when filling is detected. Recommended: 3-10 seconds for precise monitoring.",
        filling_threshold: "🔢 Filling Detection Threshold",
        filling_threshold_help: "Number of consecutive incremental readings to consider filling is happening. Minimum 2, maximum 5 readings.",
        
        // NTP Geolocation
        auto_geo_location: "🌐 Auto-Detect Location",
        auto_geo_help: "Automatically detects timezone based on your IP location to show correct date/time.",
        timezone: "🕐 Manual Timezone (optional)",
        timezone_help: "Only if you disable auto-detection. Format: America/New_York",
        show_datetime: "📅 Show Date/Time on Display",
        datetime_help: "Shows current date and time on the device's OLED display.",
        
        save_sensor: "Save",
        ap_name: "AP Name",
        ap_name_help: "Name that will appear in WiFi networks list.",
        ap_password: "AP Password",
        ap_password_help: "8 to 10 characters. Simple and easy to remember.",
        device_name: "Device Name (Hostname)",
        device_name_help: "Name to identify device on local network (no spaces).",
        save_ap: "Save",
        auto_sleep_time: "Automatic Screen Sleep",
        auto_sleep_help: "Time for screen to turn off and save energy.",
        update_interval: "Sensor Reading Interval (seconds)",
        update_interval_help: "Frequency with which sensor measures water level.",
        save_system: "Save",
        save_all_title: "Save All",
        save_all_desc: "Saves all changes from all sections without restarting.",
        save_all_desc_short: "Saves all settings.",
        save_and_restart_title: "Apply and Connect",
        save_and_restart_desc: "Saves configuration and restarts device to connect to WiFi.",
        save_and_restart_desc_short: "Save and restart with WiFi.",
        restart_title: "Restart",
        restart_desc: "Restarts device. Unsaved changes will be lost.",
        restart_desc_short: "Restart device.",
        factory_reset_title: "Factory Reset",
        factory_reset_desc: "Erases all configuration and restores factory values.",
        factory_reset_desc_short: "Erase configuration.",
        default_network: "Default",
        saved_network: "Saved",
        set_as_default: "Set as Default",
        remove_network: "Remove"
    },
    fr: {
        page_title: "Portail de Configuration",
        header_title: "Capteur de Niveau d'Eau",
        header_subtitle: "Portail de Configuration",
        wifi_config: "WiFi",
        sensor_config: "Capteur",
        ap_config: "Point d'Accès",
        system_config: "Système",
        actions: "Actions",
        saved_networks: "Réseaux Sauvegardés",
        no_saved_networks: "Aucun réseau sauvegardé",
        add_new_network: "Ajouter Nouveau Réseau",
        scan_networks: "Scanner Réseaux",
        network_name: "Nom du Réseau (SSID)",
        password: "Mot de Passe",
        wifi_password_help: "Minimum 8 caractères pour plus de sécurité.",
        make_default: "Définir par défaut",
        add_network: "Ajouter Réseau",
        tank_height: "Hauteur du Réservoir (cm)",
        tank_height_help: "Distance totale du fond du réservoir au sommet.",
        tank_capacity: "Capacité du Réservoir (litres)",
        tank_capacity_help: "Volume maximum d'eau quand le réservoir est plein.",
        min_distance: "Distance Minimale du Capteur (cm)",
        min_distance_help: "Distance minimale que le capteur peut mesurer avec précision.",
        container_type: "Type de Conteneur",
        container_type_help: "Type de conteneur pour mesurer le volume.",
        tank: "Réservoir",
        cistern: "Citerne",
        generic: "Conteneur Générique",
        save_sensor: "Sauvegarder",
        ap_name: "Nom du PA",
        ap_name_help: "Nom qui apparaîtra dans la liste des réseaux WiFi.",
        ap_password: "Mot de Passe PA",
        ap_password_help: "8 à 10 caractères. Simple et facile à retenir.",
        device_name: "Nom du Dispositif (Hostname)",
        device_name_help: "Nom pour identifier l'appareil sur le réseau local (sans espaces).",
        save_ap: "Sauvegarder",
        auto_sleep_time: "Mise en Veille Automatique",
        auto_sleep_help: "Temps pour que l'écran s'éteigne et économise l'énergie.",
        update_interval: "Intervalle de Lecture du Capteur (secondes)",
        update_interval_help: "Fréquence à laquelle le capteur mesure le nivel d'eau.",
        save_system: "Sauvegarder",
        save_all_title: "Sauvegarder Tout",
        save_all_desc: "Sauvegarde tous les changements de toutes les sections sans redémarrer.",
        save_all_desc_short: "Sauvegarde tous les paramètres.",
        save_and_restart_title: "Appliquer et Connecter",
        save_and_restart_desc: "Sauvegarde la configuration et redémarre l'appareil pour se connecter au WiFi.",
        save_and_restart_desc_short: "Sauvegarder et redémarrer avec WiFi.",
        restart_title: "Redémarrer",
        restart_desc: "Redémarre l'appareil. Les changements non sauvegardés seront perdus.",
        restart_desc_short: "Redémarrer l'appareil.",
        factory_reset_title: "Remise à Zéro",
        factory_reset_desc: "Efface toute la configuration et restaure les valeurs d'usine.",
        factory_reset_desc_short: "Effacer la configuration.",
        default_network: "Par Défaut",
        saved_network: "Sauvegardé",
        set_as_default: "Définir par Défaut",
        remove_network: "Supprimer"
    }
};

// ========================================
// GLOBAL VARIABLES
// ========================================
let currentLanguage = 'es';

// ========================================
// THEME & LANGUAGE MANAGEMENT
// ========================================

/**
 * Change application language
 */
function selectLanguage(lang, flag) {
    currentLanguage = lang;
    document.getElementById('current-flag').innerHTML = flag;
    document.getElementById('language-dropdown').classList.remove('open');
    applyTranslations(lang);
    localStorage.setItem('selectedLanguage', lang);
    localStorage.setItem('selectedFlag', flag);
}

/**
 * Apply translations to page elements
 */
function applyTranslations(lang) {
    const elements = document.querySelectorAll('[data-translate]');
    elements.forEach(element => {
        const key = element.getAttribute('data-translate');
        if (translations[lang] && translations[lang][key]) {
            if (element.tagName === 'INPUT' && element.type === 'text') {
                element.placeholder = translations[lang][key];
            } else {
                element.textContent = translations[lang][key];
            }
        }
    });
    document.title = translations[lang].page_title;
}

// ========================================
// EVENT LISTENERS SETUP
// ========================================

/**
 * Initialize event listeners
 */
function initializeEventListeners() {
    // Language dropdown toggle
    document.getElementById('language-dropdown').addEventListener('click', function(e) {
        e.stopPropagation();
        this.classList.toggle('open');
    });

    // Close language menu when clicking outside
    document.addEventListener('click', function() {
        document.getElementById('language-dropdown').classList.remove('open');
    });

    // Theme toggle
    document.getElementById('theme-toggle').addEventListener('click', function() {
        document.body.classList.toggle('light-theme');
        this.textContent = document.body.classList.contains('light-theme') ? '🌙' : '☀️';
        localStorage.setItem('theme', document.body.classList.contains('light-theme') ? 'light' : 'dark');
    });

    // Navigation links
    document.querySelectorAll('.nav-link').forEach(link => {
        if (link.id !== 'actions-nav-link') {
            link.addEventListener('click', function(e) {
                e.preventDefault();
                document.querySelectorAll('.nav-link').forEach(l => l.classList.remove('active'));
                this.classList.add('active');
                const target = this.getAttribute('href');
                document.querySelector(target).scrollIntoView({ behavior: 'smooth' });
            });
        }
    });
}

// ========================================
// UTILITY FUNCTIONS
// ========================================

/**
 * Show toast notification
 */
function showToast(message, type = 'info') {
    const container = document.getElementById('toast-container');
    const toast = document.createElement('div');
    toast.className = `toast ${type}`;
    toast.innerHTML = `<div class="toast-message">${message}</div>`;
    container.appendChild(toast);
    
    setTimeout(() => toast.classList.add('show'), 100);
    setTimeout(() => {
        toast.classList.remove('show');
        setTimeout(() => container.removeChild(toast), 400);
    }, 3000);
}

/**
 * Toggle password visibility
 */
function togglePassword(inputId) {
    const input = document.getElementById(inputId);
    const button = input.nextElementSibling;
    if (input.type === 'password') {
        input.type = 'text';
        button.innerHTML = '<svg class="icon"><use xlink:href="#eye-closed-icon"></use></svg>';
    } else {
        input.type = 'password';
        button.innerHTML = '<svg class="icon"><use xlink:href="#eye-open-icon"></use></svg>';
    }
}

// ========================================
// WIFI MANAGEMENT FUNCTIONS
// ========================================

/**
 * Scan for available WiFi networks
 */
function scanWiFi() {
    const button = document.getElementById('scan-button');
    const wifiList = document.getElementById('wifi-list');
    
    // Change button to scanning state
    button.disabled = true;
    button.innerHTML = '⏳ Escaneando...';
    showToast('Iniciando escaneo de redes WiFi...', 'info');
    
    // Clear previous list
    wifiList.innerHTML = '<div class="wifi-item">🔍 Iniciando escaneo...</div>';
    
    // Step 1: Start scan
    fetch('/scan-wifi')
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}`);
            }
            return response.json();
        })
        .then(data => {
            if (data[0] && data[0].error) {
                throw new Error(data[0].message || 'Error en el escaneo');
            }
            
            if (data[0] && (data[0].scanning || data[0].message)) {
                showToast(data[0].message || 'Escaneo en progreso...', 'info');
                wifiList.innerHTML = '<div class="wifi-item">🔍 Escaneando redes...</div>';
                
                // Wait and check results
                setTimeout(() => checkScanResults(), 2000);
            }
        })
        .catch(error => {
            console.error('Error starting WiFi scan:', error);
            showToast(`Error al iniciar escaneo: ${error.message}`, 'error');
            wifiList.innerHTML = '<div class="wifi-item">❌ Error al iniciar escaneo</div>';
            restoreButton();
        });
}

/**
 * Check scan results
 */
function checkScanResults() {
    const wifiList = document.getElementById('wifi-list');
    
    fetch('/wifi-results')
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP ${response.status}`);
            }
            return response.json();
        })
        .then(data => {
            if (data[0] && data[0].error) {
                throw new Error(data[0].message || 'Error obteniendo resultados');
            }
            
            if (data[0] && data[0].scanning) {
                // Still scanning, wait more
                showToast('Escaneo en progreso...', 'info');
                setTimeout(() => checkScanResults(), 1500);
                return;
            }
            
            // Results ready
            renderWiFiNetworks(data);
            showToast(`${data.length} redes encontradas`, 'success');
            restoreButton();
        })
        .catch(error => {
            console.error('Error getting WiFi results:', error);
            showToast(`Error obteniendo resultados: ${error.message}`, 'error');
            wifiList.innerHTML = '<div class="wifi-item">❌ Error obteniendo resultados</div>';
            restoreButton();
        });
}

/**
 * Restore scan button
 */
function restoreButton() {
    const button = document.getElementById('scan-button');
    button.disabled = false;
    button.innerHTML = '🔄 <span data-translate="scan_networks">Escanear Redes</span>';
}

/**
 * Render WiFi networks list
 */
function renderWiFiNetworks(networks) {
    const wifiList = document.getElementById('wifi-list');
    
    if (!networks || networks.length === 0) {
        wifiList.innerHTML = '<div class="wifi-item">📡 No se encontraron redes</div>';
        return;
    }

    // Sort by signal strength (higher RSSI = better signal)
    networks.sort((a, b) => b.rssi - a.rssi);

    wifiList.innerHTML = networks.map(network => `
        <div class="wifi-item" onclick="selectWiFiNetwork('${network.ssid}', ${network.encryption})">
            <div class="wifi-signal">
                ${getWiFiSignalIcon(network.rssi)}
            </div>
            <div class="wifi-details">
                <div class="wifi-name">${network.ssid}</div>
                <div class="wifi-info">
                    <span class="wifi-rssi">${network.rssi} dBm</span>
                    <span class="wifi-security">${network.encryption ? '🔒' : '🔓'}</span>
                </div>
            </div>
        </div>
    `).join('');
}

/**
 * Get WiFi signal strength icon
 */
function getWiFiSignalIcon(rssi) {
    // Convert RSSI to signal percentage and assign color
    let signalBars, signalColor, signalPercentage;
    
    if (rssi >= -50) {
        signalBars = 4; signalColor = '#4caf50'; signalPercentage = 100; // Excellent
    } else if (rssi >= -60) {
        signalBars = 3; signalColor = '#8bc34a'; signalPercentage = 75;  // Good
    } else if (rssi >= -70) {
        signalBars = 2; signalColor = '#ff9800'; signalPercentage = 50;  // Fair
    } else if (rssi >= -80) {
        signalBars = 1; signalColor = '#f44336'; signalPercentage = 25;  // Weak
    } else {
        signalBars = 0; signalColor = '#9e9e9e'; signalPercentage = 5;   // Very weak
    }

    return `
        <svg class="wifi-signal-icon" viewBox="0 0 24 24" style="color: ${signalColor};" title="${signalPercentage}% - ${rssi} dBm">
            <path d="M1 9l2 2c4.97-4.97 13.03-4.97 18 0l2-2C16.93 2.93 7.08 2.93 1 9z" 
                  fill="${signalBars >= 1 ? signalColor : '#333'}" opacity="${signalBars >= 1 ? 1 : 0.3}"/>
            <path d="M5 13l2 2c2.76-2.76 7.24-2.76 10 0l2-2C15.14 9.14 8.87 9.14 5 13z" 
                  fill="${signalBars >= 2 ? signalColor : '#333'}" opacity="${signalBars >= 2 ? 1 : 0.3}"/>
            <path d="M9 17l2 2c.87-.87 2.13-.87 3 0l2-2C14.63 15.63 11.37 15.63 9 17z" 
                  fill="${signalBars >= 3 ? signalColor : '#333'}" opacity="${signalBars >= 3 ? 1 : 0.3}"/>
            <circle cx="12" cy="21" r="1" 
                   fill="${signalBars >= 4 ? signalColor : '#333'}" opacity="${signalBars >= 4 ? 1 : 0.3}"/>
        </svg>
    `;
}

/**
 * Select WiFi network from scan results
 */
function selectWiFiNetwork(ssid, requiresPassword) {
    document.getElementById('wifi-ssid').value = ssid;
    if (requiresPassword) {
        document.getElementById('wifi-password').focus();
        showToast(`Red "${ssid}" seleccionada. Ingresa la contraseña.`, 'info');
    } else {
        showToast(`Red abierta "${ssid}" seleccionada.`, 'success');
    }
}

/**
 * Add new WiFi network
 */
function addWiFiNetwork() {
    const ssid = document.getElementById('wifi-ssid').value.trim();
    const password = document.getElementById('wifi-password').value;
    const makeDefault = document.getElementById('make-default').checked;
    
    if (!ssid) {
        showToast('Por favor ingrese el nombre de la red', 'error');
        return;
    }
    
    if (ssid.length > 32) {
        showToast('El nombre de la red no puede tener más de 32 caracteres', 'error');
        return;
    }
    
    if (password && password.length < 8) {
        showToast('La contraseña debe tener al menos 8 caracteres', 'error');
        return;
    }

    const networkData = {
        ssid: ssid,
        password: password,
        make_default: makeDefault
    };

    fetch('/api/wifi/add', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(networkData)
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast(`Red "${ssid}" agregada correctamente`, 'success');
            document.getElementById('wifi-ssid').value = '';
            document.getElementById('wifi-password').value = '';
            loadSavedNetworks();
        } else {
            showToast(data.message || 'Error al agregar la red', 'error');
        }
    })
    .catch(error => {
        console.error('Error adding network:', error);
        showToast('Error al agregar la red', 'error');
    });
}

/**
 * Load saved WiFi networks
 */
function loadSavedNetworks() {
    fetch('/api/wifi/saved')
        .then(response => response.json())
        .then(data => {
            const savedList = document.getElementById('saved-wifi-list');
            if (!data.networks || data.networks.length === 0) {
                savedList.innerHTML = '<div class="wifi-saved-item">📡 <span data-translate="no_saved_networks">No hay redes guardadas</span></div>';
                return;
            }
            
            savedList.innerHTML = data.networks.map(network => `
                <div class="wifi-saved-item ${network.is_default ? 'wifi-saved-default' : ''}">
                    <div class="wifi-details">
                        <div class="wifi-name">${network.ssid}</div>
                        <div class="wifi-saved-status">
                            ${network.is_default ? '<span data-translate="default_network">Predeterminada</span>' : '<span data-translate="saved_network">Guardada</span>'}
                        </div>
                    </div>
                    <div class="wifi-saved-actions">
                        ${!network.is_default ? `<button class="button button-secondary" onclick="setDefaultNetwork('${network.ssid}')">⭐</button>` : ''}
                        <button class="button button-danger" onclick="removeNetwork('${network.ssid}')">🗑️</button>
                    </div>
                </div>
            `).join('');
        })
        .catch(error => {
            console.error('Error loading saved networks:', error);
            document.getElementById('saved-wifi-list').innerHTML = '<div class="wifi-saved-item">❌ Error al cargar redes guardadas</div>';
        });
}

/**
 * Set default network
 */
function setDefaultNetwork(ssid) {
    fetch('/api/wifi/default', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ ssid: ssid })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast(`"${ssid}" establecida como predeterminada`, 'success');
            loadSavedNetworks();
        } else {
            showToast(data.message || 'Error al establecer red predeterminada', 'error');
        }
    })
    .catch(error => {
        console.error('Error setting default network:', error);
        showToast('Error al establecer red predeterminada', 'error');
    });
}

/**
 * Remove network
 */
function removeNetwork(ssid) {
    if (!confirm(`¿Eliminar la red "${ssid}"?`)) return;
    
    fetch('/api/wifi/remove', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ ssid: ssid })
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast(`Red "${ssid}" eliminada`, 'success');
            loadSavedNetworks();
        } else {
            showToast(data.message || 'Error al eliminar la red', 'error');
        }
    })
    .catch(error => {
        console.error('Error removing network:', error);
        showToast('Error al eliminar la red', 'error');
    });
}

// ========================================
// CONFIGURATION SAVE FUNCTIONS
// ========================================

/**
 * Save sensor configuration
 */
function saveSensorConfig() {
    const config = {
        tank_height: document.getElementById('tank-height').value,
        tank_capacity: document.getElementById('tank-capacity').value,
        min_distance: document.getElementById('min-distance').value,
        container_type: document.getElementById('container-type').value,
        
        // Smart intervals
        normal_interval: document.getElementById('normal-interval').value,
        filling_interval: document.getElementById('filling-interval').value,
        filling_threshold: document.getElementById('filling-threshold').value,
        
        // Geographic NTP configuration
        auto_geo_location: document.getElementById('auto-geo-location').checked,
        timezone: document.getElementById('timezone').value,
        show_datetime: document.getElementById('show-datetime').checked
    };
    
    fetch('/api/sensor/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(config)
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast('Configuración del sensor guardada', 'success');
        } else {
            showToast(data.message || 'Error al guardar configuración del sensor', 'error');
        }
    })
    .catch(error => {
        console.error('Error saving sensor config:', error);
        showToast('Error al guardar configuración del sensor', 'error');
    });
}

/**
 * Save Access Point configuration
 */
function saveAPConfig() {
    const config = {
        ap_ssid: document.getElementById('ap-ssid').value,
        ap_password: document.getElementById('ap-password').value,
        hostname: document.getElementById('hostname').value
    };
    
    fetch('/api/ap/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(config)
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast('Configuración del AP guardada', 'success');
        } else {
            showToast(data.message || 'Error al guardar configuración del AP', 'error');
        }
    })
    .catch(error => {
        console.error('Error saving AP config:', error);
        showToast('Error al guardar configuración del AP', 'error');
    });
}

/**
 * Save system configuration
 */
function saveSystemConfig() {
    const config = {
        auto_sleep_time: document.getElementById('auto-sleep-time').value,
        update_interval: document.getElementById('update-interval').value
    };
    
    fetch('/api/system/config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(config)
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast('Configuración del sistema guardada', 'success');
        } else {
            showToast(data.message || 'Error al guardar configuración del sistema', 'error');
        }
    })
    .catch(error => {
        console.error('Error saving system config:', error);
        showToast('Error al guardar configuración del sistema', 'error');
    });
}

/**
 * Save all configuration
 */
function saveAllConfig() {
    showToast('Guardando toda la configuración...', 'info');
    
    fetch('/api/config/save-all', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' }
    })
    .then(response => response.json())
    .then(data => {
        if (data.success) {
            showToast('Toda la configuración guardada', 'success');
        } else {
            showToast(data.message || 'Error al guardar la configuración', 'error');
        }
    })
    .catch(error => {
        console.error('Error saving all config:', error);
        showToast('Error al guardar la configuración', 'error');
    });
}

// ========================================
// SYSTEM CONTROL FUNCTIONS
// ========================================

/**
 * Save configuration and restart device
 */
function saveAndRestartConfirm() {
    if(confirm('¿Guardar configuración y reiniciar el dispositivo?')) {
        showToast('Guardando y reiniciando...', 'info');
        
        fetch('/api/save-and-restart', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' }
        })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                showToast('Configuración guardada. Reiniciando dispositivo...', 'success');
            } else {
                showToast(data.message || 'Error al guardar y reiniciar', 'error');
            }
        })
        .catch(error => {
            console.error('Error saving and restarting:', error);
            showToast('Error al guardar y reiniciar', 'error');
        });
    }
}

/**
 * Restart device
 */
function restartDevice() {
    if(confirm('¿Reiniciar el dispositivo? Los cambios no guardados se perderán.')) {
        showToast('Reiniciando dispositivo...', 'info');
        
        fetch('/api/restart', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' }
        })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                showToast('Dispositivo reiniciando...', 'success');
            } else {
                showToast(data.message || 'Error al reiniciar', 'error');
            }
        })
        .catch(error => {
            console.error('Error restarting device:', error);
            showToast('Error al reiniciar dispositivo', 'error');
        });
    }
}

/**
 * Factory reset confirmation
 */
function factoryResetConfirm() {
    if(confirm('¿Realizar reset de fábrica? Esto borrará TODA la configuración y no se puede deshacer.')) {
        showToast('Realizando reset de fábrica...', 'warning');
        
        fetch('/api/factory-reset', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' }
        })
        .then(response => response.json())
        .then(data => {
            if (data.success) {
                showToast('Reset de fábrica completado. Reiniciando...', 'success');
            } else {
                showToast(data.message || 'Error al realizar reset de fábrica', 'error');
            }
        })
        .catch(error => {
            console.error('Error during factory reset:', error);
            showToast('Error al realizar reset de fábrica', 'error');
        });
    }
}

// ========================================
// ESP-NOW MANAGEMENT (Future Enhancement)
// ========================================

// Selected connection mode
let selectedMode = '';

/**
 * Select connection mode (WiFi or ESP-NOW)
 */
function selectMode(mode) {
    selectedMode = mode;
    
    // Update visual interface
    document.querySelectorAll('.mode-card').forEach(card => {
        card.classList.remove('selected');
    });
    
    const selectedCard = mode === 'wifi' ? 
        document.getElementById('wifi-mode-card') : 
        document.getElementById('espnow-mode-card');
    
    selectedCard.classList.add('selected');
    
    // Update hidden field and enable button
    document.getElementById('selected-mode').value = mode;
    document.getElementById('continue-button').disabled = false;
    
    console.log('🔄 Mode selected:', mode);
}

/**
 * Continue with selected mode
 */
function continueWithMode() {
    if (!selectedMode) return;
    
    // Hide mode selection
    document.getElementById('mode-selection').style.display = 'none';
    
    if (selectedMode === 'wifi') {
        // Show traditional WiFi section
        document.getElementById('wifi-section').style.display = 'block';
        setActiveNav('wifi-section');
        showToast('Configuración WiFi tradicional', 'info');
    } else if (selectedMode === 'espnow') {
        // Show ESP-NOW section
        document.getElementById('espnow-section').style.display = 'block';
        setActiveNav('espnow-section');
        showToast('Configuración ESP-NOW activada', 'info');
        initializeESPNowInterface();
    }
}

/**
 * Helper to set active navigation
 */
function setActiveNav(sectionId) {
    document.querySelectorAll('.nav-link').forEach(link => {
        link.classList.remove('active');
    });
    
    const targetLink = document.querySelector(`a[href="#${sectionId}"]`);
    if (targetLink) {
        targetLink.classList.add('active');
    }
}

// ========================================
// INITIALIZATION
// ========================================

/**
 * Initialize application when DOM is loaded
 */
document.addEventListener('DOMContentLoaded', function() {
    // Load saved theme
    const savedTheme = localStorage.getItem('theme') || 'dark';
    if (savedTheme === 'light') {
        document.body.classList.add('light-theme');
        document.getElementById('theme-toggle').textContent = '🌙';
    } else {
        document.body.classList.remove('light-theme');
        document.getElementById('theme-toggle').textContent = '☀️';
    }

    // Generate language options dynamically
    const languageMenu = document.getElementById('language-menu');
    const languageFlags = {
        es: '<svg class="flag-icon" viewBox="0 0 900 600"><defs><clipPath id="circleViewMX"><circle cx="450" cy="300" r="280" /></clipPath></defs><g clip-path="url(#circleViewMX)"><rect width="300" height="600" fill="#006847"/><rect x="300" width="300" height="600" fill="#FFFFFF"/><rect x="600" width="300" height="600" fill="#CE1126"/><g transform="translate(450,300)"><circle r="65" fill="#8B4513" stroke="#654321" stroke-width="2"/><ellipse cx="0" cy="-10" rx="45" ry="55" fill="#228B22"/><path d="M-35,-40 Q-25,-50 -15,-45 Q-5,-50 5,-45 Q15,-50 25,-45 Q35,-50 35,-40 Q30,-30 25,-35 Q15,-30 5,-35 Q-5,-30 -15,-35 Q-25,-30 -35,-40Z" fill="#32CD32"/><ellipse cx="0" cy="0" rx="25" ry="30" fill="#8B4513"/><path d="M-20,-25 Q-10,-35 0,-30 Q10,-35 20,-25 Q15,-15 10,-20 Q5,-15 0,-20 Q-5,-15 -10,-20 Q-15,-15 -20,-25Z" fill="#654321"/><circle cx="-8" cy="-5" r="3" fill="#000"/><circle cx="8" cy="-5" r="3" fill="#000"/><path d="M-5,5 Q0,10 5,5" stroke="#000" stroke-width="2" fill="none"/><path d="M-15,15 Q-5,25 5,20 Q15,25 25,15" stroke="#DAA520" stroke-width="3" fill="none"/><rect x="-2" y="20" width="4" height="15" fill="#8B4513"/><ellipse cx="0" cy="40" rx="8" ry="5" fill="#228B22"/><path d="M-25,-60 L-20,-50 L-30,-50 Z" fill="#FFD700"/><path d="M25,-60 L20,-50 L30,-50 Z" fill="#FFD700"/><path d="M-35,20 Q-40,30 -30,35 Q-25,30 -30,25" fill="#8B4513"/><path d="M35,20 Q40,30 30,35 Q25,30 30,25" fill="#8B4513"/><circle cx="0" cy="-50" r="8" fill="#FFD700"/><path d="M-8,-58 L0,-70 L8,-58 Z" fill="#FF6347"/></g></g></svg>',
        en: '<svg class="flag-icon" viewBox="0 0 500 500"><defs><clipPath id="circleView"><circle cx="250" cy="250" r="250" /></clipPath></defs><g clip-path="url(#circleView)"><rect width="500" height="500" fill="#FFFFFF"/><rect width="500" height="38.46" y="0" fill="#B22234"/><rect width="500" height="38.46" y="76.92" fill="#B22234"/><rect width="500" height="38.46" y="153.84" fill="#B22234"/><rect width="500" height="38.46" y="230.76" fill="#B22234"/><rect width="500" height="38.46" y="307.68" fill="#B22234"/><rect width="500" height="38.46" y="384.6" fill="#B22234"/><rect width="500" height="38.46" y="461.52" fill="#B22234"/><rect width="263.16" height="269.23" fill="#3C3B6E"/><defs><polygon id="star" points="0,-10 2.9389,-4.0451 9.5106,-3.0902 4.7553,1.5451 5.8779,8.0902 0,5 -5.8779,8.0902 -4.7553,1.5451 -9.5106,-3.0902 -2.9389,-4.0451" fill="#FFFFFF"/></defs><g><g><use xlink:href="#star" transform="translate(21.93, 26.92)"/><use xlink:href="#star" transform="translate(65.79, 26.92)"/><use xlink:href="#star" transform="translate(109.65, 26.92)"/><use xlink:href="#star" transform="translate(153.51, 26.92)"/><use xlink:href="#star" transform="translate(197.37, 26.92)"/><use xlink:href="#star" transform="translate(241.23, 26.92)"/></g><g transform="translate(21.93, 26.92)"><use xlink:href="#star" transform="translate(21.93, 26.92)"/><use xlink:href="#star" transform="translate(65.79, 26.92)"/><use xlink:href="#star" transform="translate(109.65, 26.92)"/><use xlink:href="#star" transform="translate(153.51, 26.92)"/><use xlink:href="#star" transform="translate(197.37, 26.92)"/></g><g transform="translate(0, 53.84)"><use xlink:href="#star" transform="translate(21.93, 26.92)"/><use xlink:href="#star" transform="translate(65.79, 26.92)"/><use xlink:href="#star" transform="translate(109.65, 26.92)"/><use xlink:href="#star" transform="translate(153.51, 26.92)"/><use xlink:href="#star" transform="translate(197.37, 26.92)"/><use xlink:href="#star" transform="translate(241.23, 26.92)"/></g><g transform="translate(21.93, 80.76)"><use xlink:href="#star" transform="translate(21.93, 26.92)"/><use xlink:href="#star" transform="translate(65.79, 26.92)"/><use xlink:href="#star" transform="translate(109.65, 26.92)"/><use xlink:href="#star" transform="translate(153.51, 26.92)"/><use xlink:href="#star" transform="translate(197.37, 26.92)"/></g><g transform="translate(0, 107.68)"><use xlink:href="#star" transform="translate(21.93, 26.92)"/><use xlink:href="#star" transform="translate(65.79, 26.92)"/><use xlink:href="#star" transform="translate(109.65, 26.92)"/><use xlink:href="#star" transform="translate(153.51, 26.92)"/><use xlink:href="#star" transform="translate(197.37, 26.92)"/><use xlink:href="#star" transform="translate(241.23, 26.92)"/></g><g transform="translate(21.93, 134.6)"><use xlink:href="#star" transform="translate(21.93, 26.92)"/><use xlink:href="#star" transform="translate(65.79, 26.92)"/><use xlink:href="#star" transform="translate(109.65, 26.92)"/><use xlink:href="#star" transform="translate(153.51, 26.92)"/><use xlink:href="#star" transform="translate(197.37, 26.92)"/></g><g transform="translate(0, 161.52)"><use xlink:href="#star" transform="translate(21.93, 26.92)"/><use xlink:href="#star" transform="translate(65.79, 26.92)"/><use xlink:href="#star" transform="translate(109.65, 26.92)"/><use xlink:href="#star" transform="translate(153.51, 26.92)"/><use xlink:href="#star" transform="translate(197.37, 26.92)"/><use xlink:href="#star" transform="translate(241.23, 26.92)"/></g><g transform="translate(21.93, 188.44)"><use xlink:href="#star" transform="translate(21.93, 26.92)"/><use xlink:href="#star" transform="translate(65.79, 26.92)"/><use xlink:href="#star" transform="translate(109.65, 26.92)"/><use xlink:href="#star" transform="translate(153.51, 26.92)"/><use xlink:href="#star" transform="translate(197.37, 26.92)"/></g><g transform="translate(0, 215.36)"><use xlink:href="#star" transform="translate(21.93, 26.92)"/><use xlink:href="#star" transform="translate(65.79, 26.92)"/><use xlink:href="#star" transform="translate(109.65, 26.92)"/><use xlink:href="#star" transform="translate(153.51, 26.92)"/><use xlink:href="#star" transform="translate(197.37, 26.92)"/><use xlink:href="#star" transform="translate(241.23, 26.92)"/></g></g></g></svg>',
        fr: '<svg class="flag-icon" viewBox="0 0 900 600"><rect width="300" height="600" fill="#0055a4"/><rect x="300" width="300" height="600" fill="#ffffff"/><rect x="600" width="300" height="600" fill="#ef4135"/></svg>'
    };

    Object.keys(translations).forEach(lang => {
        if (languageFlags[lang]) {
            const option = document.createElement('div');
            option.className = 'language-option';
            option.innerHTML = languageFlags[lang];
            option.onclick = () => selectLanguage(lang, languageFlags[lang]);
            languageMenu.appendChild(option);
        }
    });

    // Load saved language
    const savedLanguage = localStorage.getItem('selectedLanguage') || 'es';
    const savedFlag = localStorage.getItem('selectedFlag') || languageFlags.es;
    selectLanguage(savedLanguage, savedFlag);

    // Initialize event listeners
    initializeEventListeners();

    // Load saved WiFi networks
    loadSavedNetworks();

    // Mobile actions panel logic
    const actionsNavLink = document.getElementById('actions-nav-link');
    const mobileActionsOverlay = document.getElementById('mobile-actions-overlay');
    const mobileActionsSheet = document.getElementById('mobile-actions-sheet');
    const desktopActions = document.querySelector('.actions-menu');
    
    if (desktopActions) {
        const clonedActions = desktopActions.cloneNode(true);
        clonedActions.classList.remove('actions-menu');
        mobileActionsSheet.appendChild(clonedActions);
    }

    actionsNavLink.addEventListener('click', function(e) {
        if (window.innerWidth <= 900) {
            mobileActionsOverlay.classList.add('open');
            mobileActionsSheet.classList.add('open');
        }
    });

    mobileActionsOverlay.addEventListener('click', function() {
        mobileActionsOverlay.classList.remove('open');
        mobileActionsSheet.classList.remove('open');
    });
});

// ========================================
// EXPORT FOR GLOBAL ACCESS
// ========================================

// Make functions available globally for onclick handlers
window.scanWiFi = scanWiFi;
window.selectWiFiNetwork = selectWiFiNetwork;
window.addWiFiNetwork = addWiFiNetwork;
window.setDefaultNetwork = setDefaultNetwork;
window.removeNetwork = removeNetwork;
window.saveSensorConfig = saveSensorConfig;
window.saveAPConfig = saveAPConfig;
window.saveSystemConfig = saveSystemConfig;
window.saveAllConfig = saveAllConfig;
window.saveAndRestartConfirm = saveAndRestartConfirm;
window.restartDevice = restartDevice;
window.factoryResetConfirm = factoryResetConfirm;
window.togglePassword = togglePassword;
window.selectLanguage = selectLanguage;
window.selectMode = selectMode;
window.continueWithMode = continueWithMode;