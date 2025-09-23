// --- Global Variables ---
let websocket;

// --- Initialization ---
window.addEventListener("load", () => {
    initWebSocket();
    initButtons();
    startPolling();
    loadDisplayStatus(); // Cargar estado inicial del display
});

// --- WebSocket Functions ---
function initWebSocket() {
    const gateway = `ws://${window.location.hostname}/ws`;
    console.log("Attempting to open WebSocket connection...");
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log("WebSocket connection opened.");
}

function onClose(event) {
    console.log("WebSocket connection closed. Retrying in 2 seconds...");
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    // WebSocket is now only used for pushing display state from server
    console.log("WebSocket message received:", event.data);
    if (event.data === "1") {
        updateDisplayStatus("Encendido");
    } else if (event.data === "0") {
        updateDisplayStatus("Apagado");
    }
}

// --- Display Status Functions ---
function loadDisplayStatus() {
    fetch('/displayStatus')
        .then(response => response.json())
        .then(data => {
            console.log("Display status loaded:", data);
            updateDisplayStatus(data.status);
        })
        .catch(error => {
            console.error("Error loading display status:", error);
            updateDisplayStatus("Error");
        });
}

function updateDisplayStatus(status) {
    const statusElement = document.getElementById("valor-estado-oled");
    if (statusElement) {
        statusElement.innerHTML = status;
        console.log("Display status updated to:", status);
    }
}

// --- Button Initializers and Handlers ---
function initButtons() {
    document.getElementById("button-toggle").addEventListener("click", toggleDisplayHandler);
    document.getElementById("button-reset").addEventListener("click", resetDeviceHandler);
    document.getElementById("wifi-config-button").addEventListener("click", resetWifiHandler);
}

function toggleDisplayHandler() {
    console.log("Toggling display...");
    fetch('/toggleDisplay')
        .then(response => response.text())
        .then(state => {
            console.log("New display state:", state);
            showToast(state, 'info');
            // El estado se actualizará automáticamente vía WebSocket
        })
        .catch(error => {
            console.error("Error toggling display:", error);
            showToast("Error de conexión con el dispositivo", 'error');
            // Recargar estado en caso de error
            setTimeout(loadDisplayStatus, 1000);
        });
}

let isResetConfirmation = false;
let resetTimeout;
function resetDeviceHandler() {
    const resetButton = document.getElementById("button-reset");
    if (!isResetConfirmation) {
        isResetConfirmation = true;
        resetButton.textContent = "¿Seguro?";
        resetTimeout = setTimeout(() => {
            resetButton.textContent = "Forzar reinicio";
            isResetConfirmation = false;
        }, 3000);
    } else {
        clearTimeout(resetTimeout);
        fetch('/reset').catch(e => console.log("Network error expected during reset."));
        showToast("Reiniciando dispositivo...", 'warning');
        resetButton.disabled = true;
        resetButton.textContent = "Reiniciando";
        // Start checking for server to come back online
        setTimeout(checkServerStatus, 3000);
    }
}

function resetWifiHandler() {
    if (confirm("¿Estás seguro? Esto borrará la configuración de WiFi guardada y reiniciará el dispositivo en modo de configuración (Portal Cautivo).")) {
        fetch('/reset-wifi').catch(e => console.log("Network error expected during reset."));
        showToast("Borrando WiFi y reiniciando...", 'warning');
        // Start checking for server to come back online in AP mode
        setTimeout(checkServerStatus, 3000);
    }
}

function checkServerStatus() {
    fetch(`http://${window.location.hostname}`)
        .then(response => {
            if (response.ok) location.reload();
            else setTimeout(checkServerStatus, 3000);
        })
        .catch(() => setTimeout(checkServerStatus, 3000));
}

// --- UI Functions ---
function showToast(message, type = 'info') {
    const toast = document.getElementById("toast");
    const toastMessage = document.getElementById("toast-message");
    const toastIcon = document.getElementById("toast-icon");

    toastMessage.textContent = message;
    toast.className = 'toast show ' + type;
    toastIcon.className = 'material-symbols-outlined toast-icon ' + type;
    toastIcon.textContent = type; // sets icon to 'info', 'warning', or 'error'

    setTimeout(() => { toast.classList.remove("show"); }, 4000);
}

// --- Data Polling ---
function startPolling() {
    const updateData = () => {
        // Fetch sensor data and parse JSON
        fetch("/Sensor")
            .then(response => response.ok ? response.text() : Promise.reject('Response not OK'))
            .then(text => {
                try {
                    const sensorData = JSON.parse(text);
                    
                    // Update sensor status
                    const statusEl = document.getElementById("valor-estado-sensor");
                    if (statusEl) {
                        statusEl.textContent = sensorData.estado || "Error";
                        statusEl.classList.toggle("ok-conexion", sensorData.estado === "OK");
                        statusEl.classList.toggle("error-conexion", sensorData.estado !== "OK");
                    }
                    
                    // Update liters
                    const litrosEl = document.getElementById("Litros");
                    if (litrosEl && sensorData.litros !== undefined) {
                        litrosEl.textContent = sensorData.litros.toFixed(1);
                    }
                    
                    // Update distance
                    const distanciaEl = document.getElementById("Distancia");
                    if (distanciaEl && sensorData.distancia_cm !== undefined) {
                        distanciaEl.textContent = sensorData.distancia_cm.toFixed(1);
                    }
                    
                } catch (error) {
                    console.error("Error parsing sensor JSON:", error);
                    console.error("Received text:", text);
                    
                    // Fallback: show descriptive error
                    const statusEl = document.getElementById("valor-estado-sensor");
                    if (statusEl) {
                        if (text.includes("Error") || text.includes("error")) {
                            statusEl.textContent = "Error de sensor";
                        } else if (text.length > 50) {
                            statusEl.textContent = "Datos inválidos";
                        } else {
                            statusEl.textContent = "Sin comunicación";
                        }
                        statusEl.classList.remove("ok-conexion");
                        statusEl.classList.add("error-conexion");
                    }
                }
            })
            .catch(error => {
                console.error("Failed to fetch sensor data:", error);
                
                // Show connectivity error
                const statusEl = document.getElementById("valor-estado-sensor");
                if (statusEl) {
                    statusEl.textContent = "Sin conexión";
                    statusEl.classList.remove("ok-conexion");
                    statusEl.classList.add("error-conexion");
                }
            });
        
        // Update image
        document.getElementById("Imagen").src = '/imagen?t=' + new Date().getTime();
    };
    
    setInterval(updateData, 5000);
    updateData(); // Initial call
}

function fetchAndUpdate(url, elementId, callback) {
    fetch(url)
        .then(response => response.ok ? response.text() : Promise.reject('Response not OK'))
        .then(text => {
            const el = document.getElementById(elementId);
            if (el) {
                el.textContent = text;
                if (callback) callback(el);
            }
        })
        .catch(error => console.error(`Failed to fetch ${url}:`, error));
}
