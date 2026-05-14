// El Esqueleto y el Rostro (JavaScript Frontend)
//Es el decorador e interiorista. Aquí se crean los botones reales ("DFAutomata"),
// se define qué campos de texto aparecen debajo de ellos y, lo más importante,
// le avisa a Vis.js cómo dibujar los círculos y las flechas en la pantalla.

//INTERACCION DE LOS BOTONES LATERALES CON LAS ACCIONES REALES

//1. CONSTRUCCION DEL MENU LATERAL
//Función: Inicializa las herramientas del usuario en el menú lateral izquierdo.
function buildMenuButtons(){
    // Limpia el panel izquierdo para asegurar
    // que no se dupliquen los botones si la función se llama más de una vez.
    var lP = dojo.byId('leftPanelDiv');
    dojo.empty(lP);
    // Instancia el primer botón interactivo ("Server Properties") usando la plantilla creada en gui.js.
    var fntB = createAwesomeButton('Server Properties', 'midFont','Server System Properties',
        'fas fa-chalkboard-teacher', getHostProperties,48, 'blue','aboutBtn', '');

    dojo.place(fntB,lP,'last');
    //Si se omite dojo.place(dfa,lP,'last');: El botón del autómata se creará en la memoria,
    // pero nunca se insertará visualmente en el panel izquierdo (lP).

    //Define los valores por defecto que llenarán los campos de texto debajo del botón del autómata.
    var data= {
        Pattern_b: 'test',
        String_b: 'this is a test again',
        imgname: 'nodesImg.png',
        http:'https://fontawesome.com/icons/circle-nodes?f=classic&s=thin'
    } ;

    //Si se modifican las llaves de data (ej. cambiar Pattern_b por Pattern):
    // La lógica analizada en buildDataTable no detectará el sufijo _b.
    // En lugar de crear un campo de texto (TextBox) donde se pueda teclear otra palabra,
    // renderizará la palabra "test" como texto estático y no se podrá cambiar en la pantalla.

    // Crea el botón "DFAutomata", inyectándole el objeto 'data'
    // para que construya los inputs de 'Pattern' y 'String'.
    var dfa = createAwesomeButton('DFAutomata', 'midFont','DfaNodes',
        'fa-solid fa-circle-nodes', finiteAutomataFunction,60, 'blue','dfaBtn', data);
    dojo.place(dfa,lP,'last');

    // BOTON 3: MATH ANALYZER
    // Solo Pattern_b: la expresion a tokenizar y evaluar.
    // String_b se omite para que el campo no aparezca en el panel lateral.
    // El lienzo inferior mostrara los pasos de reduccion generados por el backend.
    var mathData = {
        Pattern_b: 'sin(90)+1*2'
    };

    // Se usa el nombre "Math Analyzer" como título del botón lateral
    // y 'Math Analysis' como acción que el WebSocket enviará al servidor C++.
    var mathBtn = createAwesomeButton('Math Analyzer', 'midFont', 'Math Analysis',
        'fa-solid fa-calculator', mathAnalysis, 60, 'orange', 'mathBtn', mathData);

    dojo.place(mathBtn, lP, 'last');
}



//2. PANEL DE DEPURACION (LOD DEL WEBSOCKET)
//Función: Actualizan el cuadro de texto del cliente WebSocket
// donde se imprimían los mensajes de conexión.
function notifyMsg(message){
    // Convierte el objeto JSON del servidor C++
    // en una cadena de texto formateada con 4 espacios de indentación para que sea legible.
    debug(JSON.stringify(message,null,4));
}

function debug(message) {
    // Concatena el nuevo mensaje al final del área de texto del "WebSocket Client" (panel derecho).
    debugTextArea.value += message + "\n";
    // Auto-scroll: Fuerza a la barra de desplazamiento a ir hasta
    // abajo automáticamente cada vez que llega un mensaje nuevo.
    debugTextArea.scrollTop = debugTextArea.scrollHeight;
    //Si se quita la línea de scrollTop: A medida que lleguen nuevos mensajes de los nodos o del servidor,
    // el texto seguirá creciendo hacia abajo, pero tú te quedarás viendo la parte superior del cuadro de texto.
    // Seria necesario usar el mouse para bajar y ver qué acaba de llegar.
}
//3. ACTUALIZACION DE FECHA Y HORA
//Función: Actualiza el reloj/estado en la parte inferior de la pantalla.
function dateTime(data) {
    var item = dojo.byId('bottomDiv');
// Inyecta la cadena de tiempo recibida del servidor
// directamente en el pie de página de la aplicación, aplicándole color y tamaño.
    // Quitamos 'blueColor' y aplicamos un estilo directo de color blanco/gris claro
    item.style.color = "#00d4ff";
    item.innerHTML = '<strong style="font-size: 1.1em; letter-spacing: 1px;">' + data + '</strong>';
    //Si se cambia 'bottomDiv' por otro ID: El JavaScript no sabrá dónde poner la hora,
    // y la barra inferior azul de la interfaz se quedará pasmada.

}
//4. PUENTE HACIA EL AUTOMATA
//Función: Recibe el gran JSON que el C++ armó con las funciones
// parseNodes y parseEdges, lo desempaqueta, y dispara el dibujado.
function finiteAutomataFunction(obj){
    // Extrae estrictamente la carga útil ('arguments')
    // de la respuesta del servidor C++ para dársela a Vis.js.
    var data = obj.arguments;
    // Pasa el contexto 'dfa' para que setTrackButton genere los botones correctos del Autómata.
    // Llama a la función encargada de dibujar los círculos y flechas en el lienzo central.
    buildNodeSample(data, 'dfa');

    //Si se cambia obj.arguments por obj.data: La variable data quedará undefined.
    // Cuando buildNodeSample intente leer los nodos, fallará
    // porque la llave que el servidor C++ envía se llama literalmente arguments.
}
//5. PETICION AL SERVIDOR WEB: getHostProperties
//Función: Se comunica de manera silenciosa (sin recargar la página)
// con el backend de Node.js para pedir la información de la RAM, CPU, y sistema operativo.
function getHostProperties(fromObj){
    // Carga asíncronamente el módulo de Dojo para hacer peticiones HTTP (AJAX).
    //AJAX (Asynchronous JavaScript and XML) es una
    // técnica de desarrollo web para crear aplicaciones interactivas y rápidas,
    // permitiendo actualizar partes de una página en segundo plano sin recargarla por completo.
    require(["dojo/request"], function(request){
        // Dispara una petición POST a la ruta '/about' del servidor Node.js (script.js externo).
        request.post('/about', {
            //Si se cambia request.post por request.get: Esto es una conexión directa
            // con lo visto en el archivo de Express. Si aquí se cambia a get,
            // el servidor Node.js lo rechazará con un error 404, porque en el backend
            // esta configurado para escuchar exclusivamente peticiones POST en esa ruta.

            data: {
                uacj: "iit",
                cu: 'Edificio B'

                //El objeto data: { uacj: "iit"... }: Actualmente, se envía este dato al servidor,
                // pero con respecto al código de Express analizado,
                // el servidor de Node.js no está haciendo nada con él,
                // solo devuelve el objeto pcProperties.
            },
            headers: {
                "ajax": "with dojo"
            }
        }).then(function(text){

            var json = JSON.parse(text);
            var center = dojo.byId('centerInnerPanelDiv');
            // Llama a un formateador visual para convertir el JSON crudo
            // del hardware en elementos HTML presentables y los coloca en el centro.
            var parse =  parseJsonObject(json);
            dojo.place(parse,center);
        });
    });
}

//1. INICIALIZACION Y LIMPIEZA DE VARIABLES
//Función: Prepara el "lienzo en blanco" antes de pintar.

// Variable global para distinguir qué analizador generó la red actual.
// 'dfa' = Autómata Finito Determinista | 'math' = Analizador Matemático
var currentAnalyzerContext = 'dfa';

function buildNodeSample(data, analyzerContext){
    // Selecciona el panel central derecho
    // y vacía cualquier dibujo anterior para evitar que se amontonen las gráficas.
    var host = dojo.byId('rightInnerDiv');
    dojo.empty(host);
//Resetea las variables globales de estado
// a nulo o a su valor inicial ('q0') cada vez que se genera un nuevo autómata.

    //Que pasa si se quita (dojo.empty(host)); Si se le da al boton de actualizar automata
    //tres veces, se vera tres automatas dibujados uno debajo del otro, saturando la memoria del navegador

    patternNetwork = null;
    inputNetwork = null;
    selectedNode = null;
    firstNodeId = 'q0';
    // Elimina los botones de control de la ejecución anterior para crear unos nuevos y limpios.

    //Que pasa si no se resetea (firstNodeId = 'q0';) En caos de que se quiera probar un automata
    //y luego otro, el programa podria intentar iniciar la validacion desde un estado del automata
    //anterior que ya no existe, causando un error.

    // Guarda el contexto del analizador activo para que setTrackButton
    // sepa qué etiquetas de botón mostrar (DFA o Math Analyzer).
    // Si no se recibe el parámetro, asume contexto DFA por compatibilidad.
    currentAnalyzerContext = analyzerContext || 'dfa';

    // Destruye los botones de ambos contextos posibles para evitar duplicados al cambiar de analizador.
    removeButtonById('dfaStartTracker');
    removeButtonById('clearDFABtn');
    removeButtonById('mathEvalTracker');
    removeButtonById('clearMathBtn');

    //2. Estilos del AUtomata (optionsP)
    //Función: Le dice a Vis.js cómo deben verse los nodos y las líneas azules.
// Configuración visual (opciones) exclusivas para la red del Patrón (el Autómata de arriba).

    var optionsP = {
        autoResize: true,
        height: '100%',
        width: '100%',
        nodes: {
            margin: 20,
            // Retirado de 'shape: circle' de aquí para que Vis.js use
            // el shape (box o circle) que mandamos desde C++
            font: {
                size: 24,
                face: 'Monospace',
                color: 'black',
                align: 'center'
            },
            borderWidth: 1,
            color: {
                background: 'white',
                highlight: {
                    border: 'tomato', // Color del borde deseado cuando el nodo está seleccionado
                    background: 'tomato' // Color de fondo deseado cuando el nodo está seleccionado
                }
            }
        },
        edges: {
            width: 1,
            font: {
                size: 40,
                color: 'blue',
                align: 'top'
            },
            smooth: {
                enabled: true, // Esto asegura compatibilidad
                type: 'curvedCW', // curvas en sentido horario (o 'curvedCCW', 'cubicBezier')
                //*******forceDirection: 'none', // Permite que las curvas se doblen libremente******
                roundness: 0.5 // Ajusta la intensidad de la curva (de 0.0 a 1.0)
            }
        },
        layout: {
            hierarchical: {
                enabled: false
            }
        },
        physics: {
            enabled: false   // Por lo general, las redes jerárquicas funcionan mejor con la física desactivada
            //Que pasa si se cambia enabled: false a true en physics?: Los nodos del automata empezaran a flotar
            //y rebotar por toda la pantalla como si tuvieran resortes (la fisica de la libreria), arruinando el orden
            //que se calculo en el codigo de C++.
        }
    };

//3. DIBUJO DE LA CADENA DE PRUEBA( optionsI)
    //Función: Dibuja la fila inferior amarilla con la palabra que se quiera probar.
// Si el servidor C++ mandó los datos de la palabra (input), configura y dibuja la gráfica inferior.
    if('input' in data) {
        // CONTEXTO MATH: Fondo transparente en la red inferior también para ver la imagen de fondo.
        // CONTEXTO DFA: Fondo blanco para mantener la legibilidad de los nodos de la cadena.
        var inputBg = (currentAnalyzerContext === 'math')
            ? 'background: transparent;'
            : 'background: white;';
        var container = dojo.create('div', { class: 'network', style: inputBg }, host,'first');
        var dataI = data.input;
        var optionsI = {
            autoResize: true,
            height: '100%',
            width: '100%',
            nodes: {
                margin: 20,
                shape: "circle",
                font: {
                    size: 24,
                    face: 'Monospace',
                    color: 'black',
                    align: 'center'
                },
                borderWidth: 1,
                borderWidthSelected: 0,
                color: {
                    background: 'white',
                    highlight: {
                        border: 'tomato',
                        background: 'tomato'
                    }
                }
            },
            edges: {
                width: 1,
                font: {
                    size: 40,
                    color: 'blue',
                    align: 'top'
                },
                smooth: {
                    type: 'curvedCW', // Curva en sentido horario
                    forceDirection: 'none',
                    roundness: 0.2   // Un grado específico de redondez para que la línea no sea tan alta
                }
            },
            layout: {
                hierarchical: {
                    //Que pasa si se quita layout: {hierachical: ...}?: Las letras de la palabra
                    //dejaran de estar en linea recta de izquierda a derecha y se dibujaran como un circulo
                    //amontonado (comportamiento por defecto de Vis.js)
                    direction: 'LR',    // De izquierda a derecha
                    sortMethod: 'hubsize'   // Se adhiere a los datos de las flechas (to/from)
                    // === SE ELIMINÓ SMOOTH DE AQUÍ ===
                }
            },
            physics: {
                enabled: false  //// Por lo general, la jerarquía (hierarchical) funciona mejor con la física desactivada
            }
        };

        //4. MOTOR DE INTERACCION (EVENTO CLICK)
        //Función: Es el cerebro del automata. Decide si el clic fue correcto o hubo equivocacion de letra.
    // Escucha el evento de clic específicamente en las letras de la cadena de entrada (inputNetwork).
        inputNetwork = new vis.Network(container, dataI, optionsI);
        inputNetwork.on("click", function (params) {

            var btn = dijit.byId('dfaStartTracker');
            // Solo permite interactuar si el usuario ya presionó el botón de Iniciar seguimiento (Start).
            if (btn.get('label') === 'Started'){
                //Si se omite esta validacion, se podria empezar a dar clics a las letras y avanzar en el automata
                //sin haber activado formalmente la herramienta de "start tracker",
                //rompiendo la secuencia planeada de la interfaz

                if (params.nodes.length > 0) {
                    var nodeId = params.nodes[0];
                    var nodesDataSet = inputNetwork.body.data.nodes;
                    nodesDataSet.update({
                        //Parte del codigo que cambia el color y estilo visual
                        id: nodeId,
                        color: {
                            background: 'yellow', // Cambia al color deseado
                            //Si se cambia de 'yellow' a 'red', al hacer clic en las letras de abajo, se pintaran de rojo
                            //en lugar de amarillo, independientemente si se acierte o no
                            highlight: {
                                background: 'yellow' // Opcional: mantener el color al pasar el cursor por encima
                            }
                        }
                    });
                }

                var clickedNodeLabel = nodesDataSet.get(nodeId).label;
                var selectedStr = selectedNode.edgeLabel;
                // VALIDACIÓN PRINCIPAL: Compara la letra que el usuario clickeó
                // con la letra que el autómata está esperando en ese momento.
                if( clickedNodeLabel === selectedStr){
                    advanceNetwork(selectedNode);
                }
                else{
                    // Si el usuario se equivoca, revisa si la letra tocada coincide
                    // con el nodo inicial (por si quiere reiniciar). Dibuja una línea roja de error.
                    var patternDataset = patternNetwork.body.data.nodes;
                    var ids = patternDataset.getIds();
                    var currentIndex = ids.indexOf(selectedNode.id);
                    var idN = ids[currentIndex];

                    var firstNde = patternDataset.get(firstNodeId);
                    var firstLabel = firstNde.edgeLabel;
                    clearNodes(patternNetwork);
                    selectSpecificNode(firstNodeId);
                    var targetNode = firstNodeId;
                    if( clickedNodeLabel === firstLabel) {

                        var nextId = firstNde.nextId;
                        selectedNode = firstNde;
                        var toNode = advanceNetwork(selectedNode);
                        targetNode = toNode;
                        //addEdge(patternNetwork, idN, toNode,clickedNodeLabel);
                    }
                    //else{
                        addEdge(patternNetwork, idN, targetNode,clickedNodeLabel);
                   // }
                }
            }
        });
    }

    //************************************************

    // -------------------------------------------------------------
    // LIENZO 1: ÁRBOL LIBRE DEL DFA LÉXICO
    // -------------------------------------------------------------
    if('dfa_trace' in data) {
        var networkBg = 'background: transparent; border-bottom: 2px solid rgba(255,255,255,0.2); margin-bottom: 10px; min-height: 250px;';

        var containerTrace = dojo.create('div', { class: 'network', style: networkBg }, host,'first');

        dojo.create('div', {
            innerHTML: "Árbol Libre de Transiciones (DFA)",
            style: "color: #00d4ff; font-family: Monospace; font-size: 16px; font-weight: bold; position: absolute; z-index: 10; padding: 10px;"
        }, containerTrace, 'first');

        var dataT = data.dfa_trace;

        var optionsT = {
            autoResize: true,
            height: '100%',
            width: '100%',
            // ¡LA MAGIA SUCEDE AQUÍ!
            physics: {
                enabled: true, // Nodos flotantes y orgánicos
                barnesHut: {
                    gravitationalConstant: -2000, // Fuerza de repulsión para que no se peguen
                    centralGravity: 0.1,
                    springLength: 150 // Largo de las flechas
                }
            },
            interaction: {
                dragNodes: true, // El usuario PUEDE arrastrar y reacomodar los círculos
                dragView: true,  // Puede arrastrar el lienzo completo
                zoomView: true   // Puede hacer scroll para alejar o acercar
            }
        };
        new vis.Network(containerTrace, dataT, optionsT);
    }

    //*****************************************************

    //5. FINALIZACION Y DIBUJO DEL AUTOMATA / GRAFO DE TOKENS
    // Si el servidor mandó el patrón (DFA o grafo de tokens del Math Analyzer),
    // dibuja la red en la parte superior del panel usando las 'optionsP'.
    if('pattern' in data) {
        dojo.place('<hr>',host,'first');
        //(dojo.place('<hr>',host,'first'))Dibuja una línea divisoria horizontal
        // (etiqueta <hr>) para separar visualmente tu autómata de la palabra que se esta evaluando.
        // Si se quitas, ambas gráficas se verán muy pegadas.

        // CONTEXTO MATH: Se aplica fondo transparente al contenedor del grafo de tokens
        // para que la imagen de fondo del panel (Aztlan) sea visible a través del lienzo.
        // CONTEXTO DFA: Se mantiene el fondo blanco para preservar la legibilidad
        // de las flechas y estados del autómata sobre la imagen de fondo.
        var networkBg = (currentAnalyzerContext === 'math')
            ? 'background: transparent;'
            : 'background: white;';
        var container1 = dojo.create('div', { class: 'network', style: networkBg }, host,'first');
        var dataP = data.pattern;
        patternNetwork = new vis.Network(container1, dataP, optionsP);

        // >>> INICIO NUEVO: Llamar al dibujante del Árbol DFA <<<
        // Si el servidor nos mandó la ruta del DFA, mandamos a dibujarla
        // pasándole el historial de saltos y el lienzo (host)
        if ('dfa_path' in data) {
            drawDFATree(data.dfa_path, host);
        }
        // >>> FIN NUEVO <<<
    }
    // Finalmente, inyecta los botones dinámicos de control (Start/Clear) debajo de las gráficas.
    setTrackButton(host);
}
var patternNetwork;
var inputNetwork;
var selectedNode;
var firstNodeId = 'q0';

//1. CREACION EL CONTENEDOR DE CONTROLES
// La función setTrackButton genera los botones de control en la parte superior del lienzo.
// Su comportamiento varía según el contexto activo (DFA o Math Analyzer),
// usando la variable global 'currentAnalyzerContext' para decidir qué botones renderizar.
function  setTrackButton(host){
// Crea un contenedor div en la parte superior ('first')
// del panel derecho para alojar los botones de control del autómata o del analizador.
    var div = dojo.create('div',{innerHTML:'Press:'},host, 'first');
//Si se cambara 'first' por 'last' los botones de "start" y "clear" apareceran hasta abajo de la pantalla
    //debajo de la cadena de prueba en lugar de estar en la parte superior

    // BIFURCACION DE CONTEXTO:
    // Si el contexto es 'math', se generan botones específicos del Analizador Matemático.
    // Si el contexto es 'dfa' (o cualquier otro valor), se generan los botones del Autómata.
    if (currentAnalyzerContext === 'math') {

        // ----------------------------------------------------------
        // CONTEXTO MATH: Sin botones de seguimiento manual.
        // El grafo se genera y muestra automáticamente al presionar Update.
        // Los botones "Evaluar Expresión" y "Limpiar Análisis" fueron eliminados
        // porque el análisis ocurre al instante desde el servidor.
        // ----------------------------------------------------------

    } else {

        // ----------------------------------------------------------
        // BOTONES EXCLUSIVOS DEL AUTÓMATA FINITO DETERMINISTA (DFA)
        // ----------------------------------------------------------

        //2B. BOTON "Start"
        // Este botón reinicia el estado visual y habilita la validación de clics en la red.
        var start = new dijit.form.Button({
            id: 'dfaStartTracker',
            label: 'Start',
            title: 'Start',
            style: { 'marginLeft': '10px', 'marginRight': '6px' },
            onClick: function (evt) {
                // Elimina cualquier línea roja de error (dinámica) que haya quedado de un intento anterior.
                removeAddedEdges(patternNetwork,'dynamics');
                // Despinta todos los nodos de la cadena (amarillos)
                // y del autómata, regresándolos a su color blanco original.
                clearNodes(inputNetwork);
                clearNodes(patternNetwork);
                // Resalta visualmente el estado inicial (q0) para indicar dónde comienza el autómata.
                selectSpecificNode(firstNodeId);
                //Cambia su propio texto a 'Started'.
                // La función de validación anterior exige este texto para permitir interactuar con el DFA.

                //Si se quita selectSpecificNode, el usuario no sabra visualmente
                // en que estado se encuentra parado al iniciar.

                this.set('label', 'Started');

                //si se omite this.set('label', 'Started'), el usuario podra darle clic, pero al intentar tocar las letras
                //de la palabra a evaluar, el codigo analizado previamente (if (btn.get('label') === 'Started'))
                //fallara silenciosamente y no pasara nada.
            }
        });

        dojo.place(start.domNode,div);

        //3B. BOTON "Clear"
        //Si se quita el reinicio de la etiqueta del boton star, el usuario le dara "clear" para limpiar la pantalla
        // pero el sistema seguira en modo "activo", lo que causaria comportamentos erraticos o errores
        //de indice si se toca una letra sin haber reiniciado el rastreo del estado q0.

        // Crea el botón Clear. Su función es abortar el intento actual
        // y limpiar la pantalla, desactivando la interactividad.
        var clear = new dijit.form.Button({
            id:'clearDFABtn',
            label: 'Clear',
            title: 'Clear',
            style: { 'margin-right': '6px' },
            onClick: function (evt) {
                removeAddedEdges(patternNetwork,'dynamics');

                clearNodes(inputNetwork);
                clearNodes(patternNetwork);
    // Busca el botón Start y le regresa su texto original,
    // lo cual bloquea ('apaga') la validación de clics en la gráfica inferior.
                var btn = dijit.byId('dfaStartTracker');
                btn.set('label','Start');
            }
        });

        dojo.place(clear.domNode,div);

    } // Fin de bifurcación de contexto DFA / Math Analyzer
}
//1. VALORES POR DEFECTO
function clearNodes(network, backGroundColor){
    // Configura colores por defecto. Si no se manda un color de fondo,
    // asume que es un reseteo total a nodos blancos con texto negro.
    var fontColor = 'white';
    if( backGroundColor === undefined) {
        backGroundColor = 'white';
        fontColor = 'black';

        //Si se modifica fontcolor = 'black' por 'white' dentro del if, al limpiar la pantalla,
        //las letras dentro de los circulos blancos desapareceran
        // porque el texto blanco sobre el fondo blanco es invisible.
    }
    var nodesDataSet = network.body.data.nodes;

    //2. ITERACION Y ACTUALIZACION DE ESTADOS
// Recorre todos los nodos existentes en la memoria
// de la gráfica de Vis.js para modificar sus propiedades visuales.
    var updatedNodes = nodesDataSet.map(function (node, index) {
//Protege el nodo inicial transparente
// (la flecha de entrada al autómata). Si es el 'root', lo mantiene invisible.
        if( index === 'root')
            //El nodo root es la flecha invisible que se crea en C++ que apunta a q0.
            //Si se quita la validacion if(index === 'root'), al darle al boton "Clear", la flecha transparente
            //de entrada se convertira en un circulo blanco solido con un borde, arruinando la representacion formal
            //del diagrama de transicion

            // Construye un nuevo objeto con las propiedades visuales actualizadas
            // (color de fondo y fuente) para el nodo actual.
            return {
                id: node.id,
                color: {
                    background: 'transparent',
                },
                font: {
                    color:'transparent'
                }
            };
        else
        return {
            id: node.id,
            color: {
                background: backGroundColor,
            },
            font: {
                color:fontColor
            }
            //Tamanio: 20      // Ejemplo de cambio: cambiar tamanio
        };
    });

    //3. APLICACION DE CAMBIOS
    // Inyecta el arreglo de nodos modificados de vuelta al motor de Vis.js
    // para que redibuje la pantalla de un solo golpe.
    nodesDataSet.update(updatedNodes);

    //Si se quitara nodesDataSet.update(updatedNodes), todo el ciclo map
    //anterior habra sido inutil porque los cambios solo se quedaron en una variable temporal
    //de JavaScript y nunca se le ordeno a la libreria grafica que los pintara

    // Quita el halo de selección azul/gris que Vis.js
    // le pone por defecto al último nodo que el usuario clickeó.
    network.unselectAll();
}
//Función: Es el marcador visual y lógico del estado actual del autómata.
function selectSpecificNode(nodeId) {
// Accede al conjunto de datos (dataset) de Vis.js que contiene todos los estados del autómata (el patrón).
    var nodesDataSet = patternNetwork.body.data.nodes;
//Actualiza la variable global 'selectedNode' con el estado actual.
// Esto permite que el sistema sepa dónde está parado el usuario en el DFA.
    selectedNode = nodesDataSet.get(nodeId);

    //So se omite selectedNode = ... La interfaz se pintara de azul, pero logicamente le programa no avanzara
    // Al siguiente clic, el codigo intentara leer selectedNode.edgeLabel y fallara porque la variable global
    //esta vacia o desactualizada

    // Actualiza el color de fondo del nodo
    // Cambia visualmente el nodo actual a color azul (#4169E1)
    // y fuente blanca para resaltar que es el estado activo.
    nodesDataSet.update({
        id: nodeId,
        font: {
            color: 'white'
        },
        color: {
            background: '#4169E1', // Cambiar al color deseado
            //Si se cambia #4169E1 por white: El nodo activo se vera exactamente igual que los nodos inactivos,
            //por o que el usuario no sabra en que estado del DFA se encuentra
            highlight: {
                background: '#4169E1' // Opcional: mantener el color al pasar el ratón por encima
            }
        }
    });
}
//Función: Limpieza de la interfaz gráfica. Se usa (como en buildNodeSample)
// para borrar los botones "Start" y "Clear" viejos antes de crear unos nuevos.
function removeButtonById(buttonId) {
    // Busca en el registro interno de Dojo Toolkit si existe un widget (botón) con el ID proporcionado.
    var button = dijit.byId(buttonId);
    //Si se cambia dijit.byId por dojo.byId: El codigo fallaria. dojo.byId solo devuelve un elemento HTML crudo,
    //el cual no tiene la funcion .destroyRecursive(), que es exclusiva de los widgets avanzados de la libreria Dijit.

    if (button) {
        // Destruye el widget y sus nodos DOM asociados.
        button.destroyRecursive(false); // Pasa 'false' para destruir también los nodos HTML del navegador DOM
        // (el valor predeterminado es false) para evitar fugas de memoria.
    }
}

//Si se quitara esta funcion cada vez que el usuario genere un nuevo automata, se crearan nuevos botones
//"Start" y "clear" que se iran apilando uno debajo del otro en el panel derecho, saturando la pantalla

//Función: Es el motor de transición del autómata. Dicta el paso del estado qn al estado qn+1.
function advanceNetwork(node){
    // Lee la propiedad 'nextId' del nodo actual.
    // Recordatorio: Esta propiedad fue inyectada por el servidor C++ al generar el autómata.
    var nextId = node.nextId;

    //Si se altera node.nextId: Toda la conexion entre el frontend y el backend se rompe.
    //El JavaScript no sabe como esta estructurado el automata; depende enteramente de que el C++
    //le haya dicho "despues de q0 sigue q1" a traves de esta variable.

    // Condición de completado: Si el nodo actual no tiene un 'nextId',
    // significa que se llego al final del autómata con éxito.
    if( nextId === undefined){
        // Pinta todos los nodos del autómata de color verde
        // para indicar visualmente que la cadena fue aceptada completamente.
        clearNodes(patternNetwork, 'green');
   }
    else
        // Si aún hay camino por recorrer, mueve el marcador azul al siguiente estado del DFA.
        selectSpecificNode(nextId);

    return nextId;
    //Si se quita return nextId; Funciones externas que dependan de saber a que estado salto el automata
    //(como la validacion de errores que dibuja la linea roja) recibiran un valor nulo
    //y podria fallar en su logica de ruteo
}
//Función: Dibuja las líneas rojas cuando se comete un error de letra al validar la palabra en el DFA.
function addEdge(network, fromNodeId, toNodeId, edgeLabel) {

    var edges = network.body.data.edges;
    //var connectedEdgeIds = network.getConnectedEdges(fromNodeId);
    //Esta linea comentada es una limitacion de la libreria Vis.js a mitad del desarrollo
    //Ese metodo (getConnectedEdges) es muy rapido, pero solo devuelve un arreglo con los IDs numericos
    //o de texto de las flechas (ej. ["edge1", "edge5"]. No devuelve el objeto completo con sus propiedades.
    //Para saber de que color era la flecha (eType === 'dynamics') y hacia donde apuntaba (to --- toNodeId).
    //esos IDs no servian de nada. Por lo tanto se descargaron todas las flechas completas con (edges.get())
    //para poder filtrarlas por sus propiedades internas

    // Obtiene la lista completa de flechas (aristas) dibujadas actualmente en el autómata.
    var allEdges = edges.get();
    // Filtra la lista para buscar si ya existe una flecha roja de error
    // ('dynamics') conectando exactamente estos dos nodos.
    var matchingEdges = allEdges.filter(function(edge) {
        return (edge.from === fromNodeId && edge.to === toNodeId &&
         edge.eType === 'dynamics');
    });
    // Si ya existe una flecha de error, no dibuja otra encima.
    // Solo le concatena la nueva letra (ej. de "a" pasa a "a, b").
    if( matchingEdges.length > 0 ){

        //si se omite esta seccion, si se equivoca varias veces en el mismo estado, el programa dibujara
        //multiples flechas rojas exactamente en la mism aposicion, una encima de otra,
        //sobrecargando la memoria grafica y haciendo que el texto sea ilegible

        var currentLabel = matchingEdges[0].label;
        var newLabel = currentLabel +', ' + edgeLabel;
        matchingEdges[0].label =newLabel;
        edges.update(matchingEdges);
        return;
    }
    // Si no existe, construye una flecha roja y curva desde cero.
    // Usa Date.now() para asegurar que su ID sea único e irrepetible.
    var newEdgeData = {
        id: 'edge' + Date.now(), // Genera un ID unico
        from: fromNodeId,
        to: toNodeId,
        arrows: 'to',
        eType:'dynamics',
        //Si se cambiara eType:'dynamics' a eType:'normal': Se romperia la logica de la limpieza.
        //Cuando se de clic al boton de "Clear", la siguiente funcion que se analice no encontrara
        //esa flecha para borrarla, y el error rojo se quedara permanentemente en la pantalla
        label: edgeLabel,
        smooth: {
            forceDirection: 'none',
            type: 'curvedCW',
            roundness: 0.3
        },
        color:'red',
        font:{color:'red', size:20, align: 'horizontal' }
    };
    // Agrega el nuevo borde al conjunto de datos.
// Inyecta la nueva flecha de error en el motor gráfico para que aparezca inmediatamente en pantalla.
    edges.update([newEdgeData]);
}
//Función: Se encarga de limpiar los errores. Se invoca cuando se presionan los botones "Start" o "Clear"
// (como se observa en la función setTrackButton) pasándole el target 'dynamics'
function removeAddedEdges(network, target) {
    var edgesDataSet = network.body.data.edges;
    // Descarga todas las flechas del autómata a un arreglo temporal.
    var allEdges = edgesDataSet.get();

    var edgeIDs = [];
    // Filtra el array para obtener los bordes con la etiqueta correspondiente.
    // Escanea todas las flechas buscando aquellas cuyo tipo
    // (eType) coincida con el objetivo (ej. 'dynamics').
    var matchingEdges = allEdges.filter(function(edge) {
        var mtch = edge.eType === target
        // Si la flecha coincide con el criterio,
        // guarda su ID en la "lista negra" (edgeIDs) para su futura eliminación.
        if( mtch )
            edgeIDs.push(edge.id);

        //Si se cambia edgeIDs.push(edge.id); por edgeIDs.push(edge);: La función fallará rapidamente.
        // El comando edgesDataSet.remove() de Vis.js exige estrictamente un arreglo de IDs (identificadores),
        // no los objetos completos. Si se le pasa objetos completos, no borrará nada.

        return mtch;//edge.label === targetLabel;

        //Si quitas el return mtch; dentro del filter
        // La función filter de JavaScript devolverá un arreglo vacío,
        // por lo que la "lista negra" nunca se llenará y las líneas rojas de error jamás se borrarán de la pantalla.

    });
// Borra del lienzo visual todas las flechas recolectadas en la lista negra de un solo golpe.
    edgesDataSet.remove(edgeIDs);
}

/*
// =========================================================================
// Función: Dibuja el Árbol DFA Dinámico de Transiciones Léxicas en SVG
// =========================================================================
function drawDFATree(pathSeq, host) {
    // 1. Configuración del espacio de trabajo SVG
    var svgNS = "http://www.w3.org/2000/svg"; // Espacio de nombres obligatorio para SVG
    var svg = document.createElementNS(svgNS, "svg");
    svg.setAttribute("width", "100%");     // Ocupa todo el ancho disponible
    svg.setAttribute("height", "280");     // Reserva 280 píxeles de altura
    svg.style.display = "block";           // Evita comportamientos extraños de margen
    svg.style.borderBottom = "2px solid rgba(255,255,255,0.2)"; // Línea sutil divisoria
    svg.style.marginBottom = "10px";

    // 2. Definición estática de los 5 estados universales del Lexer
    // Cada estado tiene sus coordenadas (x, y) y su color temático
    var states = {
        "START":    { x: 100, y: 140, color: "#888888" }, // Gris
        "NUMBER":   { x: 350, y: 60,  color: "#00d4ff" }, // Cian
        "FUNCTION": { x: 350, y: 220, color: "#ffd700" }, // Amarillo
        "OPERATOR": { x: 600, y: 140, color: "#ff00ff" }, // Magenta
        "END":      { x: 850, y: 140, color: "#00ff41" }  // Verde Neón
    };

    var transitionCounts = {}; // Diccionario para rastrear cuántas veces pasamos por el mismo camino

    // 3. Trazado de las rutas (flechas) usando el historial de saltos (pathSeq)
    // Se recorre el arreglo de "from" hacia "to" (ej. START -> NUMBER)
    for (var i = 0; i < pathSeq.length - 1; i++) {
        var from = pathSeq[i];
        var to = pathSeq[i+1];

        // Si el estado no existe en el diccionario, lo ignora (seguridad)
        if(!states[from] || !states[to]) continue;

        // Genera una clave única para este camino, ej. "NUMBER-OPERATOR"
        var key = from + "-" + to;
        transitionCounts[key] = (transitionCounts[key] || 0) + 1;

        // Multiplicador de curvatura. Si la línea pasa varias veces por el mismo sitio,
        // se hace más curva para que no se empalme con la línea anterior.
        var offset = transitionCounts[key] * 25;

        var p1 = states[from];
        var p2 = states[to];

        // Puntos de control para generar curvas de Bézier (Arcos)
        var dx = p2.x - p1.x;
        var dy = p2.y - p1.y;
        var cx = p1.x + dx/2;           // Punto medio en X
        var cy = p1.y + dy/2 - offset;  // Punto medio en Y (Arqueado hacia arriba)

        // Si el autómata retrocede (ej. va de derecha a izquierda: OPERATOR -> NUMBER)
        // Arqueamos la curva fuertemente hacia abajo para no chocar con las que van de ida.
        if (p1.x > p2.x) {
            cy = p1.y + dy/2 + offset + 60;
        }

        // Construye el objeto <path> (línea SVG)
        var path = document.createElementNS(svgNS, "path");
        // M = Mover lápiz a (p1), Q = Curva de Bezier a través del punto control (cx,cy) hasta (p2)
        var d = "M " + p1.x + " " + p1.y + " Q " + cx + " " + cy + " " + p2.x + " " + p2.y;
        path.setAttribute("d", d);
        path.setAttribute("fill", "transparent");           // Sin relleno interior
        path.setAttribute("stroke", "rgba(255, 255, 255, 0.8)"); // Línea blanca semi-transparente
        path.setAttribute("stroke-width", "2");             // Grosor de 2px
        path.setAttribute("stroke-dasharray", "5,5");       // Línea punteada (estilo tecnológico)
        svg.appendChild(path);

        // Dibuja el número de paso sobre la curva (Ej. [1], [2], [3])
        var text = document.createElementNS(svgNS, "text");
        text.setAttribute("x", cx);
        text.setAttribute("y", cy - 5);
        text.setAttribute("fill", "#ffffff");
        text.setAttribute("font-size", "14");
        text.setAttribute("font-family", "Monospace");
        text.setAttribute("text-anchor", "middle"); // Centra el texto en la coordenada
        text.textContent = "[" + (i + 1) + "]";     // Agrega los corchetes
        svg.appendChild(text);
    }

    // 4. Dibujo de los Nodos (Círculos) por encima de las líneas
    for (var s in states) {
        var state = states[s];
        var g = document.createElementNS(svgNS, "g"); // Grupo SVG para agrupar circulo + texto

        // Configura el circulo base
        var circle = document.createElementNS(svgNS, "circle");
        circle.setAttribute("cx", state.x);
        circle.setAttribute("cy", state.y);
        circle.setAttribute("r", "45");               // Radio de 45px
        circle.setAttribute("fill", "#121212");       // Color oscuro por defecto (apagado)
        circle.setAttribute("stroke", state.color);   // Color del borde según el estado
        circle.setAttribute("stroke-width", "4");

        // EFECTO DE LUZ: Si la expresión actual tocó este estado,
        // lo rellenamos para que parezca "Encendido/Activado"
        if (pathSeq.includes(s)) {
            circle.setAttribute("fill", state.color);
            circle.setAttribute("fill-opacity", "0.3");
        }

        // Dibuja el nombre del estado (ej. "NUMBER") en el centro del círculo
        var text = document.createElementNS(svgNS, "text");
        text.setAttribute("x", state.x);
        text.setAttribute("y", state.y + 5); // +5 para centrar verticalmente la fuente
        text.setAttribute("fill", "white");
        text.setAttribute("font-family", "Monospace");
        text.setAttribute("font-size", "14");
        text.setAttribute("font-weight", "bold");
        text.setAttribute("text-anchor", "middle");
        text.textContent = s;

        // Agrega el círculo y el texto al grupo, y el grupo al SVG
        g.appendChild(circle);
        g.appendChild(text);
        svg.appendChild(g);
    }

    // 5. Título Elegante en la esquina superior izquierda
    var title = document.createElementNS(svgNS, "text");
    title.setAttribute("x", "510");
    title.setAttribute("y", "10");
    title.setAttribute("fill", "#00d4ff");
    title.setAttribute("font-size", "18");
    title.setAttribute("font-family", "Monospace");
    title.setAttribute("font-weight", "bold");
    title.textContent = "Árbol de Transiciones del DFA Léxico";
    svg.appendChild(title);

    // 6. Inserción final: Lo coloca en la posición 'first' del panel,
    // asegurando que quede hasta arriba, antes de las redes de Vis.js
    dojo.place(svg, host, "first");
}*/
