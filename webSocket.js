/*
FRONTEND
Este archivo implementa la interfaz de cliente para comunicarse con el servidor
mediante WebSockets. Forma parte de un sistema que interactúa con el backend
*/

// Variable global para el área de depuración (donde se imprimen mensajes)
var debugTextArea;

// Variable global que almacena la instancia del WebSocket
var websocket;


// Función que construye la interfaz gráfica del cliente WebSocket
function webSocketInterface(){

    // Se crea un contenedor principal tipo <div>
    var webHost = dojo.create('div', {});

    // Se agrega un título a la interfaz
    dojo.create('div',{id:'dateTimeId', innerHTML:'<h3>WebSocket Client</h3>'}, webHost);

    // Contenedor para los botones
    var btnHost = dojo.create('div', {},webHost);

    // Botón para iniciar conexión con el WebSocket
    var init = createDojoButton('Connect','dijitEditorIcon dijitEditorIconCreateLink', initWebSocket,true);

    // Se coloca el botón en el contenedor
    dojo.place(init,btnHost);

    // Botón para cerrar la conexión
    var stop = createDojoButton('Disconnect','dijitEditorIcon dijitEditorIconUnlink', stopWebSocket,true);

    // Se coloca el botón en el contenedor
    dojo.place(stop,btnHost);

    // Botón para consultar el estado del WebSocket
    var status = createDojoButton('Status','dijitIconFunction', checkSocket,true);

    // Se coloca el botón en el contenedor
    dojo.place(status,btnHost);

    // Se crea un área de texto para mostrar mensajes de depuración
    debugTextArea = dojo.create('textarea', {
        id:'debugTextArea',
        style: {width: '306px',height:'400px'}
    }, webHost);

    // Se retorna el contenedor completo
    return webHost;
}

// Función para inicializar la conexión WebSocket
function initWebSocket() {

    // Obtiene la IP del host actual (aunque no se usa directamente después)
    var ip = location.host.split(':')[0];

    // Construye la URI del WebSocket usando la IP del servidor
    wsUri = 'ws://' + serverIp + ':9055';

    try {
        // Compatibilidad con navegadores antiguos (Firefox)
        if (typeof MozWebSocket == 'function')
            WebSocket = MozWebSocket;

        // Si ya existe una conexión abierta, se cierra antes de crear otra
        if ( websocket && websocket.readyState == 1 ){
            websocket.close();
        }

        // Se crea una nueva conexión WebSocket
        websocket = new WebSocket( wsUri );

        // Evento cuando la conexión se abre correctamente
        websocket.onopen = function (evt) {
            debug("CONNECTED");
            // Se envía un mensaje inicial al servidor
            sendMessage("contact",{name:'Hello from NODEJS'});
        };

        // Evento cuando la conexión se cierra
        websocket.onclose = function (evt) {
            debug("DISCONNECTED");
        };

        // Evento cuando se recibe un mensaje del servidor
        websocket.onmessage = function (evt) {
            socketService(evt.data);
        };

        // Evento en caso de error
        websocket.onerror = function (evt) {
            debug('ERROR: ' + evt.data);
        };

    } catch (exception) {
        // Manejo de errores en la creación del WebSocket
        debug('ERROR: ' + exception);
    }
}

// Función para cerrar la conexión WebSocket
function stopWebSocket() {
    // Si existe una conexión, se cierra
    if (websocket)
        websocket.close();
}

// Función para verificar el estado actual del WebSocket
function checkSocket() {

    // Si el WebSocket existe
    if (websocket != null) {

        // Variable para almacenar el estado en texto
        var stateStr;

        // Se evalúa el estado numérico del WebSocket
        switch (websocket.readyState) {

            case 0: {
                stateStr = "CONNECTING"; // Conectando
                break;
            }

            case 1: {
                stateStr = "OPEN"; // Abierto
                break;
            }

            case 2: {
                stateStr = "CLOSING"; // Cerrándose
                break;
            }

            case 3: {
                stateStr = "CLOSED"; // Cerrado
                break;
            }

            default: {
                stateStr = "UNKNOW"; // Estado desconocido
                break;
            }
        }

        // Se imprime el estado actual
        debug("WebSocket state = " + websocket.readyState + " ( " + stateStr + " )");

    } else {
        // Si el WebSocket no existe
        debug("WebSocket is null");
    }
}

// Función para enviar mensajes al servidor
function sendMessage(action, message) {

    // Se construye el objeto a enviar
    var payload = {
        Action: action, // Tipo de acción
        Data: (message !== undefined) ? message : {} // Datos (o vacío si no hay)
    };

    // Se convierte el objeto a formato JSON
    var jsonStr = JSON.stringify(payload);

    // Se muestra en consola lo que se enviará
    console.log("Enviando al servidor:", jsonStr);

    // Si el WebSocket está abierto, se envía el mensaje
    if (websocket && websocket.readyState === 1) {
        websocket.send(jsonStr);
    } else {
        // Si no está abierto, se muestra error
        console.error("No se pudo enviar: WebSocket cerrado.");
    }
}

// Función que procesa los mensajes recibidos del servidor
function socketService(data) {

    // Si no hay datos válidos, se ignora
    if (data === 'NoData' || data === undefined) return;

    var message;

    try {
        // Se intenta parsear el JSON recibido
        message = JSON.parse(data);
    } catch (e) {
        // Si falla el parseo, se muestra el dato crudo
        debug(data);
        return;
    }

    // Se obtiene la acción del mensaje
    var action = message.Action;

    // Se obtienen los datos asociados
    var dataArg = message.Data;

    console.log("Acción recibida:", action);

    // Se evalúa la acción recibida
    switch (action) {

        case "notifyMsg":
            notifyMsg(dataArg); // Maneja notificaciones
            break;

        case "finiteAutomataFunction":
            finiteAutomataFunction(dataArg); // Maneja DFA general
            break;

        /*
        Math Analysis
        Aquí se recibe el resultado del backend
        */
        case "Math Analysis":
            mathAnalysis(dataArg);
            break;

        case "dateTime":
            dateTime(dataArg); // Manejo de fecha/hora
            break;

        default:
            // Si la acción no es reconocida
            console.warn("Acción no reconocida:", action);
            break;
    } // Fin del switch

} // Fin de socketService

// PUENTE PARA DIBUJAR LA CALCULADORA
function mathAnalysisResponse(obj) {

    // Se extraen los argumentos del objeto recibido
    var data = obj.arguments;

    /*
    Esta función recibe el resultado del análisis matemático ya procesado,
    y lo envía a una función de renderizado (buildNodeSample).
    */

    // Se construye la representación visual de los datos
    buildNodeSample(data);
}

// Esta función es la que recibe los datos procesados del backend
function mathAnalysis(obj) {
    // 1. Mensaje de estado descriptivo en el panel negro
    var status = obj.status;
    var isOk = (status === "OK");

    debug("Status: " + (isOk ? "OK" : "ERROR"));
    if (isOk) {
        debug("Expresion Matematica valida.");
        if (obj.result !== undefined) {
            debug("Resultado: " + obj.result);
        }
    } else {
        // El status viene con el mensaje de error detallado desde C++
        debug(status);
    }

    // 2. Dibujar grafo de tokens (superior) y pasos de reduccion (inferior)
    //    con contexto 'math' para que no se generen botones de seguimiento
    var data = obj.arguments;
    buildNodeSample(data, 'math');

    // 3. Titulo en el panel derecho
    var host = dojo.byId('rightInnerDiv');
    if (host) {
        dojo.create('h2', {
            innerHTML: "Math Analysis Results",
            style: "color: white; position: absolute; top: 10px; left: 20px; z-index: 10;"
        }, host, "first");
    }
}