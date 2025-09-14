// --- Global Variables ---
let websocket;

// --- Initialization ---
window.addEventListener("load", () => {
    initWebSocket();
    initButtons();
    startPolling();
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
    if (event.data === "1") {
        document.getElementById("valor-estado-oled").innerHTML = "Encendido";
    } else if (event.data === "0") {
        document.getElementById("valor-estado-oled").innerHTML = "Apagado";
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
        })
        .catch(error => {
            console.error("Error toggling display:", error);
            showToast("Error de conexión con el dispositivo", 'error');
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
        fetchAndUpdate("/Sensor", "valor-estado-sensor", el => {
            el.classList.toggle("ok-conexion", el.textContent.includes("OK"));
            el.classList.toggle("error-conexion", !el.textContent.includes("OK"));
        });
        fetchAndUpdate("/Litros", "Litros");
        fetchAndUpdate("/Distancia", "Distancia");
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
