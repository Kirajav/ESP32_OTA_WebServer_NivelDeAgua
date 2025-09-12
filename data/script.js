var gateway = `ws://${window.location.hostname}/ws`;

var websocket;

window.addEventListener("load", onLoad);
function initWebSocket() {
    console.log("Intentando abrir conexión WebSocket...");
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage; // <-- add this line
}
function onOpen(event) {
    console.log("Conexión abierta");
}
function onClose(event) {
    console.log("Conexión cerrada");
    setTimeout(initWebSocket, 2000);
}
function onMessage(event) {
    var estado;
    var texto_boton;
  
    if (event.data == "1") {
        estado = "encendidas";
        texto_boton = "Apagar";
    } else {
        estado = "apagadas";
        texto_boton = "Encender";
    }
        
    document.getElementById("valor-estado-oled").innerHTML = estado;
    document.getElementById("button").innerHTML = texto_boton;
    
}
function onLoad(event) {
    initWebSocket();
    initButton();
}
function initButton() {
    document.getElementById("button").addEventListener("click", toggle);
}
function toggle() {
    //console.log(element.id /*+ " " + element.event*/);
    websocket.send("toggle");
}
function formato_estado_sensor(){
    var tag_sensor_valor = document.getElementById("valor-estado-sensor");
    var tag_sensor_valor_text = tag_sensor_valor.innerText;
    (tag_sensor_valor_text.includes("OK")) ? tag_sensor_valor.className = "ok-conexion" : tag_sensor_valor.className = "error-conexion" 
}

setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("valor-estado-sensor").innerHTML = this.responseText;
        }
    };
    xhttp.open("GET", "/Sensor", true);
    xhttp.send();
}, 5000);

setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("Litros").innerHTML = this.responseText;
        }
    };
    xhttp.open("GET", "/Litros", true);
    xhttp.send();
}, 5000);

setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("Distancia").innerHTML = this.responseText;
        }
    };
    xhttp.open("GET", "/Distancia", true);
    xhttp.send();
}, 5000);

setInterval(function () {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function () {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById("Imagen").src = this.responseURL;
        }
    };
    xhttp.open("GET", "/imagen", true);
    xhttp.send();
}, 5000);