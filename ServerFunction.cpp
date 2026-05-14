//1. EL CEREBRO C++
//Es la fábrica de lógica. Aquí es donde ocurre el procesamiento pesado: se calculan las coordenadas de los nodos
//(parseNodes), se crean las conexiones (parseEdges) y se empaqueta todo en JSON para mandarlo a la web.

// #include "headers/PhdFunctions.h"
#include "ServerFunction.h"
#include "Simulator.h"
#include "../headers/MathParser.h"
#include <cmath>      // sin, cos, tan, M_PI
#include <sstream>    // ostringstream
#include <iomanip>    // setprecision
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
// Registra las funciones disponibles.
//Función: Es el diccionario del servidor. Le dice qué método ejecutar cuando recibe un comando de texto.
//1. CONSTRUCTOR
ServerFunction::ServerFunction()
{
    // Vincula el texto (ej. "contact") con la dirección de memoria de la función real (&ServerFunction::contact).
    usrFunction.insert({"contact", &ServerFunction::contact});
    usrFunction.insert({"finiteAutomataFunction", &ServerFunction::finiteAutomataFunction});
    usrFunction.insert({"mathAnalysis", &ServerFunction::mathAnalyzerFunction});
//Si se quitara un insert, el servidor recibirá el comando del frontend,
//pero no sabrá qué hacer con él. Devolverá un error porque la acción no estará registrada.
}
//2. ENRUTADOR processAction(json jsonDoc)
//Función: Es el "cerebro" que distribuye el trabajo.
string ServerFunction::processAction(json jsonDoc)
{
    string status = "Error message";

    try
    {
        bool ok = false;
        // Extrae el valor de la clave "Action" enviada por el frontend
        auto action = jsonDoc["Action"].get<std::string>();     //Si se cambia jsonDoc["Action"] por otra palabra:
        //El servidor buscará una clave incorrecta en el JSON del cliente y fallará en cada petición.

        // Busca en el diccionario si la acción solicitada existe.
        auto fnc = getFunction(action, &ok);
        if (fnc != 0)
        {
            // Si la función existe, la ejecuta pasándole el JSON completo
            // (std::move optimiza la memoria) y guarda la respuesta.
            status = (this->*fnc)(std::move(jsonDoc));
        }
    }
    // Toma errores (ej. un JSON mal formado)
    // para evitar que el servidor crashee y se cierre repentinamente.
//Si se quitan los bloques catch: Cualquier error menor (como que el frontend envíe un dato incompleto)
//provocará una excepción no manejada y el ejecutable del servidor se cerrará por completo.
    catch (const json::exception& e){
        const string es = e.what();
        status = "Error. " + es;

    }
    catch (std::exception const &ex)
    {
        status = "Can't init settings. "; //+  ex.what();
        cout << status << endl;
    }
    catch (...)
    {
        status += "Error. Critical  Error";

        cout << status << endl;
    }

    return status;
}
//3. AUXILIAR DE BUSQUEDA
//Función: Realiza la búsqueda segura en memoria.
ServerFunction::srvrFunction ServerFunction::getFunction(string name, bool *ok)
{
    string nme = "none";
    ServerFunction::srvrFunction ptr = nullptr;

    *ok = false;
    // Busca el nombre de la acción en el mapa usrFunction.
    auto p = usrFunction.find(name);
    if (p != usrFunction.end())
    {
        // Si encuentra la acción (no llega al final del mapa),
        // guarda el puntero a la función y confirma el éxito (ok = true).
        ptr = p->second;
        *ok = true; //Si se cambiara a false Podría causar fallos si alguna parte del código depende estrictamente
        //de esa bandera booleana para continuar, aunque actualmente processAction
        //se fía más de si el puntero fnc es diferente de 0.
    }
    return ptr;
}
//4. EMPAQUETADOR
//Función: Estandariza todas las respuestas del servidor para que el frontend las entienda.
string ServerFunction::notifyMessage(string name, json message)
//Es la función de encapsulamiento que añade las cabeceras necesarias
//para que el cliente WebSocket sepa procesar la información de forma asíncrona."
{
    boost::posix_time::ptime now = boost::posix_time::microsec_clock::local_time();
    // Obtiene la fecha y hora actual del sistema usando la librería Boost.
    string dtime = boost::posix_time::to_simple_string(now);
    std::vector<std::string> tokens;

    // Split the string by the dot delimiter
    boost::algorithm::split(tokens, dtime, boost::is_any_of("."));

    message["_date"] = tokens[0];
    json msg;
    // Construye el JSON final de salida asegurando la estructura estándar que espera el JavaScript.
    msg["Action"] = name;
    msg["Data"] = message;
    //Si se cambiaran estas claves, El frontend (JavaScript) está programado para leer esas claves exactas.
    //Si las cambias, los datos llegarán al navegador, pero la interfaz no se actualizará
    //porque no encontrará las variables que busca.

    return  msg.dump();

}
//5. SALUDO INICIAL
//Función: Maneja el "saludo" cuando se conecta desde la web.
string ServerFunction::contact(json jsonV)
{
    string ft = jsonV.dump(4);
    json m ;
    m["Status"]= "OK -- Connected.";
    m["Source"]= "Humberto Hernandez";   //Se reemplazo "cppServer" Por mi nombre
    m["Matricula"] = "241653"; //Mi matricula
    m["From"] = jsonV["Data"]["name"].get<string>();
    // Llama al empaquetador para enviar los datos de vuelta al cliente con la etiqueta "notifyMsg".
    return notifyMessage("notifyMsg", m);

//Si se quita esta función: Al darle al botón "Connect" en la página web,
//no aparecerán los datos en el panel derecho del cliente WebSocket,
//dificultando saber si la conexión realmente fue exitosa.

}
//6.PUNTO DE ENTRADA
//Función: Es la primera barrera que toca el mensaje al llegar al backend.
string ServerFunction::rxMessage( string msg) {
    // Convierte el texto plano (string) que llega del WebSocket a un objeto JSON estructurado.
    json document = json::parse(msg);       //i se quita json::parse(msg): Sería imposible extraer la información
    //de manera limpia; se tendria que buscar el texto a mano, lo cual es ineficiente y propenso a errores.

    msg = "";
    // Manda el JSON ya parseado al enrutador principal para que decida qué hacer.
    string result =  processAction(std::move(document));

    return result;

}

//FUNCION PRINCIPAL
std::string ServerFunction::finiteAutomataFunction(json doc) {
    // Extrae qué acción se pidió, la palabra que define el autómata y la cadena a evaluar.
    string action = doc["Action"].get<std::string>();
    string pattern = doc["Data"]["Pattern_b"].get<std::string>();
    string stringWord = doc["Data"]["String_b"].get<std::string>();

    //Si se cambiaran estas llaves "Pattern_b" o "String_b": El backend crasheara al intentar usar el metodo
    //.get<std::string>() sobre un valor nulo, porque el JavaScrip de la interfaz web
    //envia los datos exactamente con esos nombres

    // Crea un nodo inicial invisible (root) para simular la flecha de entrada al autómata.
    // ==========================================
    // SMART ROUTER: ¿Es una operación matemática?
    // ==========================================
    // Si la cadena tiene operadores o la palabra "sin", se desvia el tráfico


    //BLOQUE EN PRUEBA DE CANCELACION
    // if (pattern.find_first_of("+-*/()") != std::string::npos || pattern.find("sin") != std::string::npos) {
        // Ejecutamos el analizador matemático pero le devolvemos al frontend
        // la "action" original para que Vis.js lo dibuje sin quejarse.
    //    return mathAnalyzerFunction(doc);
    //}


    // ==========================================
    auto startNode = R"({
        "position":"root",
        "label":"",
        "id":"root",
        "shape":"circle",
        "borderWidth": 0,
        "color": {
            "background": "transparent",
            "highlight": {
                "background": "transparent"
            }
        },
        "edgeLabel":"",
        "x":-700,
        "y": 0
    })"_json;

    // Crea el estado de aceptación final (nodo verde etiquetado "Meet").
    auto endNode = R"({
        "position":"end",
        "label":"Meet",
        "id":"end",
        "shape":"circle",
        "borderWidth": 6,
        "borderWidthSelected":6,
        "color": {
            "background": "lightgreen",
            "highlight": {
                "background": "lightgreen",
                "border": "darkseagreen"
            }
        },
        "edgeLabel":"Meet",
        "y":0
    })"_json;

    // Genera los estados intermedios (q0, q1...) basándose en la palabra 'pattern'.
    auto  nodes = parseNodes(pattern);

    // Calcula la posición X del último nodo generado para colocar el nodo final (Meet) justo a la derecha.
    int endX = nodes.back()["x"].get<int>();
    endNode["x"] = endX+150;

    // Inserta el nodo invisible al principio y el nodo de aceptación al final de la lista.
    nodes.insert(nodes.begin(), startNode);
    nodes.push_back(endNode);
    //Si se quitaran estas lineas, el automata en pantalla perderia la flecha inicila transparente
    //que apunta a q0, o el nodo verde de aceptacion final. EL automata quedaria visualmente incompleto.

    // Genera las flechas (edges) que conectarán toda esta secuencia principal.
    auto  edges = parseEdges(nodes);

    // Genera los nodos y flechas de la segunda red (la cadena de prueba en la parte inferior de la pantalla).
    // Se pasa 'true' para indicar que es la cadena de entrada, y '100' como ID inicial para evitar colisiones.
    auto strNodes = parseNodes(stringWord,  true, 100); //si se modifica
    //y se le pasa un 0 en lugar de 100: Los IDs de los nodos de la cadena (strNodes empezaran en 0,
    //colisionando con los IDs de los nodos del automata (nodes). Vis.js se confundiria y mezclaria
    //o sobreescribiria los nodos de arriba con los de abajo, rompiendo la interfaz.


    auto strEdges = parseEdges(strNodes, true);

    // Empaqueta los arreglos de nodos y flechas en estructuras JSON.
    json networks;
    json data;
    data["nodes"] = nodes;
    data["edges"] = edges;

    json dataS;
    dataS["nodes"] = strNodes;
    dataS["edges"] = strEdges;

    // Asigna la red del autómata a "pattern" y la red de la cadena a "input".
    networks["pattern"] = data;
    networks["input"] = dataS;
//Si se cambiara "pattern" o "input" en este objeto, el codigo en buildMenuButtons.js
    //en los bloques if('input' in data) y if('pattern' in data). Al renombrar estas llaves, el JS simplemente
    //las ignorara y la pantalla aparecera en blanco

    // Prepara la respuesta final encapsulando todo bajo "arguments".
    json done;
    done["arguments"] = networks;
    std::string result = notifyMessage(action,done);

    return result;
    //Esta funcion es el coordinador principal del proyecto. TOma las dos palabras que se ingresan en la web,
    //construye dos graficas separadas (el patron de validacion y la cadena que el usuario quiere probar)
    //y las envia listas para que Vis.js las dibuje
}

//FUNCION AUXILIAR
//Es una herramienta de utilidad. Si se le pasa un JSON como {"nombre": "A", "edad": 20},
//esta función devolverá una lista que solo contiene ["nombre", "edad"].

//En caso de querer quitarla, en el flujo específico de finiteAutomataFunction, esta función no se está llamando.
//Sin embargo, en arquitecturas de backend completas, suele usarse para validar si el cliente envió todos los campos
//obligatorios antes de procesar una petición. Si se quita y otro módulo del servidor la necesita para validar datos,
//ese módulo fallará al compilar.

vector<string> ServerFunction::jsonKeys(json obj)
{
    // Crea un vector (arreglo dinámico) para almacenar cadenas de texto.
    vector<string> vector;

    // Recorre cada elemento dentro del objeto JSON recibido.
    for(auto &item : obj.items()){
        // Extrae solamente el nombre de la llave (key) y lo añade a la lista.
        auto key = item.key();
        vector.push_back(key);
    }

    // Devuelve la lista con todas las llaves encontradas.
    return vector;
}
    //FUNCION parseNodes
//Convierte la palabra ingresada *ya sea el patron del automata o la cadena a evaluar) en una lista de nodos
//(circulos) en formato JSON para que Vis.js los dibuje. Separa la logica usando la bandera "stringNetwork": si es falsa,
//crea los estados del automata (q0, q1, ...), si es verdadera, crea la cadena inferior interactiva
vector<json> ServerFunction::parseNodes(string pattern, bool stringNetwork, int startId) {
    std::vector<char> patternNodes(pattern.begin(), pattern.end());

    json nodes = {} ;
    //json edges = {};
    int id = startId;
    // Define el espaciado horizontal (en píxeles) entre cada círculo del autómata.
    int nodeGap = 150;      //si se cambiara por 0 o un valor muy pequenio todos los circulos del automata
    //se dibujarian amontonados uno encima del otro. Es la separacion horizontal en pixeles.

    int startPos = -600;    //En caso de cambiar el valor, el automata se movera de lugar en la pantalla
    //Si se pone un numero positivo, el automata aparecera "fuera" del area visible o muy a la derecha
    for (char c : patternNodes) {
        json nde;
        string label;
        string q = "q";     //en caso de cambia rel valor del string por "s": Los estados en la pantalla
        //dejaran de llamarse q0, q1 y se llamaran s0, s1
        label.push_back(c);
        // nde["id"] = id;
        nde["label"] =  q.append(to_string(id));
        nde["id"] = nde["label"];       //Los nodos no tendrian un identificador unico si se quita esta linea
        //El motor grafico Vis.js no podria dibujar las flechas porque no sabria de donde a donde conectar
        nde["edgeLabel"] = label;
        nde["network"] = "pattern";
        if ( !stringNetwork)
        {
            if (id > 0)
                // Nombra los estados como q0, q1, q2... para que se vean así en la interfaz web.
                nodes[id-1]["nextId"] = nde["id"];      //Seria fatal para la logica si se quita esta linea
            //Basicamente le dice al nodo actual cual es su nodo siguiente. Si se borra, cuando se haga clic en las letras
            //en la pagina web, el JavaScript no sabra a que estado avanzar y el automata dejaria de funcionar.
            nde["x"] = startPos;
            nde["y"] = 0;
            startPos += nodeGap;
        }
        // Si la bandera es verdadera, configura los nodos
        // como parte de la palabra de prueba (la fila inferior amarilla).
        if ( stringNetwork )
        {
            nde["label"] = label;
            nde["network"] = "input";
            nde.erase("edgeLabel");     //Si se eliminara esta linea, los nodos amarillos de la parte inferior
            //intentarian guardar datos de transiciones que no necesitan, lo cual es ineficiente pero no rompe el programa.
        }
        nodes.push_back(nde);
        id++;
    }

    return nodes;
}
    //FUNCION parseEdges
//Toma la lista de nodos generada para crear las conexiones (flechas) entre ellos. Itera sobre los nodos
//saltandose el primero (usando la bandera init) para ir conectando el nodo anterior (from) con el actual (to)
vector<json> ServerFunction::parseEdges(vector<json> nodes, bool stringNetwork) {
    json edges = {};
    json start = nodes.front();
    string lbl = "";
    if (!stringNetwork )
        lbl = start["edgeLabel"].get<std::string>();

    string from = start["id"].get<std::string>();
    bool init = true;
    for (json n : nodes) {
        // Salta la primera iteración porque el primer nodo (q0)
        // no tiene una flecha que venga de un nodo anterior en este ciclo.
        if ( init ) {
            init = false;
            continue;
            //Si se quitara esta validacion el ciclo intentara crear una flecha en la primera pasada donde no hay un nodo
            //valido o creara un auto-bucle en el estado q0 apuntando a si mismo de manera erronea
        }
        string to = n["id"].get<std::string>();
        json edge;
        edge["from"] = from;
        edge["to"] = to;
        // Define que la flecha sea unidireccional (apuntando hacia el siguiente estado).
        edge["arrows"] = "to";  //Si se cambiara esta linea por "" (vacio: las lineas que conectan los estados
        //ya no tendran la punta de flecha, por lo uqe visualmente no se sabra en que direccion influye el automata
        if ( !stringNetwork )
        {
            // Coloca el carácter de la transición (ej. 'a') encima de la flecha azul entre estados.
            edge["label"] = lbl;        //Al quitar esta linea, las flechas que unen los estados del automata apareceran en blanco,
            //sin la letra de transicion (por ejemplo, ya no dira "a", "b" o "c" sobre la linea
            lbl = n["edgeLabel"].get<std::string>();        //Si se modificara el orden
            //la transicion en la flecha mostrara el caracter equivocado. La logica actual guarda la letra
            //del nodo anterior para ponerla en la flecha que va hacia el nodo siguiente
        }

        edges.push_back(edge);
        from = to;
    }

    return edges;
}
/* 1. Instancia el Parser con la expresión recibida por el socket.
 * 2. Genera una estructura de datos tipo Grafo (Nodos y Aristas).
 * 3. Inyecta metadatos visuales (colores, formas, coordenadas) para Vis.js.
 */
std::string ServerFunction::mathAnalyzerFunction(json doc) {
    // Extracción de metadatos del JSON recibido por el WebSocket
    string action = doc["Action"].get<std::string>();
    string expression = doc["Data"]["Pattern_b"].get<std::string>(); // Expresión a evaluar

    // Instanciación del motor de análisis léxico y sintáctico
    MathParser parser(expression);
    double result = 0;
    string statusMsg = "OK"; // Por defecto es OK

    // ─────────────────────────────────────────────────────────────
    // FASE 0: VALIDACIONES PREVIAS AL PARSEO
    // Se aplican antes de llamar a MathParser para dar mensajes
    // de error precisos mediante un switch de categorías de fallo.
    // ─────────────────────────────────────────────────────────────

    // 0A. Detección del tipo de error mediante categorías
    enum class ErrorKind { None, EmptyExpr, InvalidChar, UnbalancedOpen, UnbalancedClose, ParseError };
    ErrorKind errKind = ErrorKind::None;
    string errDetail = "";

    // 0B. Expresión vacía
    if (expression.empty() || expression.find_first_not_of(" \t") == std::string::npos) {
        errKind = ErrorKind::EmptyExpr;
    }

    // 0C. Caracteres inválidos (alfabeto permitido: 0-9, ., *, +, -, /, (, ), sin, cos, tan, espacio)
    if (errKind == ErrorKind::None) {
        // Eliminar tokens válidos de palabras (sin, cos, tan) antes de revisar char a char
        string exprCheck = expression;
        // Sustituir funciones conocidas por marcador neutro para no bloquear sus letras
        auto replaceAll = [](string s, const string& from, const string& to) {
            size_t pos = 0;
            while ((pos = s.find(from, pos)) != string::npos) {
                s.replace(pos, from.size(), to);
                pos += to.size();
            }
            return s;
        };
        exprCheck = replaceAll(exprCheck, "sin", "000");
        exprCheck = replaceAll(exprCheck, "cos", "000");
        exprCheck = replaceAll(exprCheck, "tan", "000");

        string allowedChars = "0123456789.+-*/() \t";
        for (char ch : exprCheck) {
            if (allowedChars.find(ch) == string::npos) {
                errKind = ErrorKind::InvalidChar;
                errDetail = string(1, ch);
                break;
            }
        }
    }

    // 0D. Balanceo de paréntesis — lógica correcta con stack
    if (errKind == ErrorKind::None) {
        int depth = 0;
        int pos   = 0;
        for (char ch : expression) {
            if (ch == '(') {
                depth++;
            } else if (ch == ')') {
                if (depth == 0) {
                    // Cierre sin apertura correspondiente
                    errKind    = ErrorKind::UnbalancedClose;
                    errDetail  = to_string(pos);
                    break;
                }
                depth--;
            }
            pos++;
        }
        if (errKind == ErrorKind::None && depth > 0) {
            // Quedan aperturas sin cerrar
            errKind   = ErrorKind::UnbalancedOpen;
            errDetail = to_string(depth);
        }
    }

    // 0E. Si no hay error previo, intentar el parseo real
    if (errKind == ErrorKind::None) {
        try {
            result = parser.parse();
        } catch (const std::exception& e) {
            errKind   = ErrorKind::ParseError;
            errDetail = std::string(e.what());
        }
    }

    // 0F. Construcción del mensaje de status con switch de categorías
    switch (errKind) {
        case ErrorKind::None:
            statusMsg = "OK";
            break;

        case ErrorKind::EmptyExpr:
            statusMsg = "ERROR: La expresion esta vacia. Ingrese una expresion matematica valida.";
            break;

        case ErrorKind::InvalidChar:
            statusMsg = "ERROR: Caracter invalido detectado: '" + errDetail + "'. "
                        "El alfabeto permitido es: 0-9, '.', '+', '-', '*', '/', '(', ')', sin, cos, tan.";
            break;

        case ErrorKind::UnbalancedOpen:
            statusMsg = "ERROR: Parentesis sin cerrar. Faltan " + errDetail + " parentesis de cierre ')'. "
                        "Revise que cada '(' tenga su ')' correspondiente.";
            break;

        case ErrorKind::UnbalancedClose:
            statusMsg = "ERROR: Parentesis de cierre inesperado en la posicion " + errDetail + ". "
                        "Se encontro ')' sin un '(' previo que lo abra.";
            break;

        case ErrorKind::ParseError:
            statusMsg = "ERROR GRAMATICAL: " + errDetail;
            break;

        default:
            statusMsg = "ERROR: Error desconocido en la expresion.";
            break;
    }

    // Obtención de la lista de tokens clasificados por el Lexer
    auto tokens = parser.getTokens();
    json nodes = json::array(); // Contenedor para los vértices del grafo
    json edges = json::array(); // Contenedor para las aristas (flechas) del grafo

    // >>> INICIO NUEVO: Generar el historial del camino DFA <<<
    json dfa_path = json::array();       // Creamos un arreglo JSON vacío
    dfa_path.push_back("START");         // El autómata siempre inicia en el estado START

    // Iteramos sobre todos los tokens que encontró el analizador léxico
    for (size_t i = 0; i < tokens.size(); i++) {
        // Guardamos la categoría gramatical ("NUMBER", "FUNCTION" u "OPERATOR")
        // Esto representa el salto de estado que dio el DFA al leer el símbolo.
        dfa_path.push_back(tokens[i].type);
    }

    // Si la expresión fue válida (no hubo errores de sintaxis o alfabeto)
    if (errKind == ErrorKind::None) {
        dfa_path.push_back("END");       // El autómata alcanza el estado de aceptación final
    }
    // >>> FIN NUEVO <<<
    // Punto de origen en el eje X para el renderizado horizontal
    int xPos = -700;


    /*FASE 1: MAPEADO DE TOKENS A NODOS VISUALES
     * Se transformo la lógica abstracta del compilador en elementos gráficos.
     * Los nodos adoptan la forma CIRCULAR del DFA tradicional para mantener
     * coherencia visual entre el autómata finito y el analizador matemático.
     */
    for (size_t i = 0; i < tokens.size(); i++) {
        json node;
        // Identificador único qn (Compatible con la lógica de seguimiento del DFA)
        string id = "q" + to_string(i);
        node["id"] = id;

        // Etiqueta visual de dos líneas:
        //   Línea 1: nombre del estado (q0, q1...) — igual que en el DFA
        //   Línea 2: valor del token y su categoría gramatical (ej. "5 (NUMBER)")
        // Esto unifica la representación visual con el Autómata Finito.
        node["label"] = id + "\n" + tokens[i].value + "\n(" + tokens[i].type + ")";

        // FORMA CIRCULAR: Se usa "circle" en lugar de "box" para mantener la
        // apariencia tradicional de los estados de un autómata finito.
        node["shape"] = "circle";

        node["x"] = xPos;      // Asignación de coordenada fija para evitar solapamiento
        node["y"] = 0;
// Las coordenadas son fijas para anular el motor de física de Vis.js.
// Esto permite tener un control total sobre la topología del grafo,
// asegurando que la representación visual coincida con la lectura lineal humana."

        // CODIFICACIÓN POR COLORES: Diferenciación semántica de los tokens.
        // El grosor del borde (borderWidth) refuerza visualmente la categoría.

        // CODIFICACIÓN POR COLORES (Estilo Dark/Neon UI)
            // Representa operandos o datos numéricos puros
        if(tokens[i].type == "NUMBER") {
            node["color"] = {{"background", "#121212"}, {"border", "#00d4ff"}};
            node["font"] = {{"color", "white"}, {"bold", true}};
            node["borderWidth"] = 3;
            //Representa funciones trigonométricas (sin, cos, tan)
            // Se diferencia de los operadores aritméticos para mayor claridad semántica.
        } else if(tokens[i].type == "FUNCTION") {
            node["color"] = {{"background", "#121212"}, {"border", "#ffd700"}};
            node["font"] = {{"color", "white"}, {"bold", true}};
            node["borderWidth"] = 3;
        } else { // OPERATOR Representa operadores aritméticos (+, -, *, /, paréntesis)
            node["color"] = {{"background", "#121212"}, {"border", "#ff00ff"}};
            node["font"] = {{"color", "white"}, {"bold", true}};
            node["borderWidth"] = 3;
        }

        // ENLAZAMIENTO LÓGICO: Define la ruta de transición entre estados
        if (i < tokens.size() - 1) {
            // Indica al Frontend cuál es el siguiente nodo en la secuencia
            node["nextId"] = "q" + to_string(i + 1);
        } else {
            // Al ser el último token, apunta directamente al estado de aceptación (resultado)
            node["nextId"] = "resultado_final";
        }

        nodes.push_back(node);

        // Generación de aristas: Conecta el nodo anterior con el actual para formar la cadena
        if (i > 0) {
            edges.push_back({{"from", "q" + to_string(i-1)}, {"to", id}, {"arrows", "to"}});
        }

        // Incremento del desplazamiento horizontal para mantener una topología limpia.
        // Se aumenta a 200px (antes 180) para dar espacio a la etiqueta de dos líneas.
        xPos += 200;
    }

    /*FASE 2: GENERACIÓN DEL NODO DE ESTADO FINAL (ACCEPTANCE STATE)
     * Representa la conclusión exitosa de la evaluación aritmética.
     * Este nodo sigue la convención visual del DFA: círculo con doble borde (borderWidth grueso)
     * para indicar que es un estado de aceptación terminal.
     */
    json resNode;
    resNode["id"] = "resultado_final";

    // Normalización de la salida: Eliminación de decimales redundantes (ceros a la derecha)
    string resStr = to_string(result);
    resStr.erase(resStr.find_last_not_of('0') + 1, std::string::npos);
    if(resStr.back() == '.') resStr.pop_back();

    // La etiqueta incluye "= resultado" en la primera línea para reflejar la conclusión
    // del análisis, similar al nodo "Meet" del DFA que indica aceptación de la cadena.
    resNode["label"] = "= " + resStr;
    resNode["shape"] = "circle"; // Círculo: Símbolo estándar de estado terminal/final

    // Posicionamiento relativo dinámico: Se ubica siempre al final de la secuencia de tokens
    resNode["x"] = xPos + 120;
    resNode["y"] = 0;

    // Borde verde neón doble (borderWidth 6) para indicar estado de aceptación,
    // replicando la convención visual del nodo "Meet" en el Autómata Finito.
    resNode["borderWidth"] = 6;
    resNode["color"] = {{"background", "#121212"}, {"border", "#00ff41"}};
    resNode["font"] = {{"color", "#00ff41"}, {"size", 20}, {"bold", true}};
    nodes.push_back(resNode);

    // Arista final de asignación (Símbolo '=')
    edges.push_back({{"from", "q" + to_string(tokens.size()-1)}, {"to", "resultado_final"}, {"arrows", "to"}, {"label", "="}});

    /**
     * FASE 3: GRAFO DE PASOS DE REDUCCIÓN (LIENZO INFERIOR)
     * Muestra la evaluación paso a paso respetando la precedencia de operadores:
     *   Prioridad 1 (mayor): funciones trigonométricas  → sin, cos, tan
     *   Prioridad 2:         paréntesis (sub-expr)       → (...)
     *   Prioridad 3:         multiplicación y división   → * /
     *   Prioridad 4 (menor): suma y resta               → + -
     *
     * Cada nodo del grafo inferior representa un paso de reducción:
     *   Label: "Paso N\n<sub-expr> = <resultado parcial>"
     * Los nodos se conectan en cadena de izquierda a derecha.
     * El último nodo repite el resultado final como estado de aceptación.
     */
    // Estructura para registrar cada paso de reducción
    struct ReductionStep {
        string subExpr;   // Sub-expresión que se resolvió
        string partial;   // Resultado parcial
        int    priority;  // Prioridad de operación
    };

    // Uso de gestión de memoria dinámica manual
    ReductionStep* steps = new ReductionStep[100];
    int stepCount = 0;

    // Solo construir pasos si la expresión es válida
    if (errKind == ErrorKind::None) {

        // Función auxiliar: normalizar double a string y atrapar números gigantes (asíntotas)
        auto dblToStr = [](double v) -> string {
            if (std::abs(v) > 1e10) return "Indefinido"; // Correccion para que no se dibujen nodos grandes
            std::ostringstream oss;
            if (v == (long long)v) {
                oss << (long long)v;
            } else {
                oss << std::fixed << std::setprecision(4) << v;
                string s = oss.str();
                s.erase(s.find_last_not_of('0') + 1, std::string::npos);
                if (s.back() == '.') s.pop_back();
                return s;
            }
            return oss.str();
        };

        // Copia temporal de tokens con memoria dinámica manual para simular la reducción
        struct TempToken { string type; string value; };
        int currentSize = tokens.size();
        TempToken* currentTokens = new TempToken[currentSize];
        for (int i = 0; i < currentSize; ++i) {
            currentTokens[i].type = tokens[i].type;
            currentTokens[i].value = tokens[i].value;
        }

        // Simulación del autómata reductor (resuelve jerarquía paso a paso y colapsa la cadena)
        bool madeChanges = true;
        while (madeChanges) {
            madeChanges = false;
            for (int prio = 0; prio <= 3; prio++) {
                bool reduced = true;
                while (reduced) {
                    reduced = false;
                    for (int i = 0; i < currentSize; i++) {

                        // Prioridad 0: Limpiar parentesis (ej. "( 8 )" -> "8")
                        if (prio == 0 && currentTokens[i].value == "(") {
                            // REGLA DE ORO: Evitar colapsar si el parentesis le pertenece a una funcion
                            bool isFuncArg = (i > 0 && currentTokens[i-1].type == "FUNCTION");

                            // Buscamos si el patron exacto es ( NUMBER )
                            if (!isFuncArg && i + 2 < currentSize &&
                                currentTokens[i+1].type == "NUMBER" &&
                                currentTokens[i+2].value == ")") {

                                // Colapsar: quitar los parentesis y dejar solo el numero
                                currentTokens[i].type = "NUMBER";
                                currentTokens[i].value = currentTokens[i+1].value;

                                for (int j = i + 1; j < currentSize - 2; j++) {
                                    currentTokens[j] = currentTokens[j + 2];
                                }
                                currentSize -= 2;
                                reduced = true;
                                madeChanges = true; // Forzamos un reinicio maestro
                                break;
                            }
                        }

                        // Prioridad 1: Funciones trigonométricas
                        else if (prio == 1 && currentTokens[i].type == "FUNCTION" && i + 3 < currentSize) {
                            string funcName = currentTokens[i].value;
                            string argStr   = currentTokens[i+2].value;
                            double arg      = 0.0;
                            // Blindaje extra: try/catch local para evitar crasheos fantasma
                            try { arg = std::stod(argStr); } catch(...) { break; }

                            double res      = 0.0;

                            if      (funcName == "sin") res = std::sin(arg * M_PI / 180.0);
                            else if (funcName == "cos") res = std::cos(arg * M_PI / 180.0);
                            else if (funcName == "tan") res = std::tan(arg * M_PI / 180.0);

                            steps[stepCount++] = {funcName + "(" + argStr + ")", dblToStr(res), 1};

                            // Colapsar tokens
                            currentTokens[i].type = "NUMBER";
                            currentTokens[i].value = dblToStr(res);
                            for (int j = i + 1; j < currentSize - 3; j++) {
                                currentTokens[j] = currentTokens[j + 3];
                            }
                            currentSize -= 3;
                            reduced = true;
                            madeChanges = true;
                            break;
                        }
                        // Prioridad 2 (*, /) y Prioridad 3 (+, -)
                        else if ((prio == 2 && (currentTokens[i].value == "*" || currentTokens[i].value == "/")) ||
                                 (prio == 3 && (currentTokens[i].value == "+" || currentTokens[i].value == "-"))) {

                            if (i > 0 && i + 1 < currentSize &&
                                currentTokens[i-1].type == "NUMBER" && currentTokens[i+1].type == "NUMBER") {

                                if (currentTokens[i-1].value == "Indefinido" || currentTokens[i+1].value == "Indefinido") {
                                    steps[stepCount++] = {currentTokens[i-1].value + " " + currentTokens[i].value + " " + currentTokens[i+1].value, "Indefinido", prio};
                                    currentTokens[i-1].value = "Indefinido";
                                } else {
                                    double lv = 0.0, rv = 0.0;
                                    try {
                                        lv = std::stod(currentTokens[i-1].value);
                                        rv = std::stod(currentTokens[i+1].value);
                                    } catch(...) { break; }

                                    double res = 0;
                                    string op = currentTokens[i].value;

                                    if (op == "*") res = lv * rv;
                                    else if (op == "/") res = (rv != 0) ? lv / rv : 0;
                                    else if (op == "+") res = lv + rv;
                                    else if (op == "-") res = lv - rv;

                                    steps[stepCount++] = {currentTokens[i-1].value + " " + op + " " + currentTokens[i+1].value, dblToStr(res), prio};
                                    currentTokens[i-1].value = dblToStr(res);
                                }

                                for (int j = i; j < currentSize - 2; j++) {
                                    currentTokens[j] = currentTokens[j + 2];
                                }
                                currentSize -= 2;
                                reduced = true;
                                madeChanges = true;
                                break;
                            }
                        }
                    }
                }
            }
        }

        string resStrFinal = dblToStr(result);
        steps[stepCount++] = {"Resultado final", resStrFinal, 4};

        delete[] currentTokens; // Liberar memoria
    }

    // ── Construcción del grafo de pasos ──
    json stepNodes = json::array();
    json stepEdges = json::array();

    if (stepCount > 0) {
        int xStep = -600;
        const int stepGap = 220;

        for (int s = 0; s < stepCount; s++) {
            json sNode;
            string sid = "s" + to_string(s);
            sNode["id"]    = sid;
            sNode["shape"] = "circle";
            sNode["x"]     = xStep;
            sNode["y"]     = 0;
            xStep += stepGap;

            bool isFinal = (s == stepCount - 1);
            if (isFinal) {
                sNode["label"] = "= " + steps[s].partial;
                sNode["borderWidth"] = 6;
                sNode["color"] = {{"background", "#121212"}, {"border", "#00ff41"}};
                sNode["font"]  = {{"color", "#00ff41"}, {"size", 18}, {"bold", true}};
            } else {
                string stepLabel = "Paso " + to_string(s+1) + "\n"
                                 + steps[s].subExpr + "\n= " + steps[s].partial;
                sNode["label"] = stepLabel;
                sNode["borderWidth"] = 2;
                switch(steps[s].priority) {
                    case 1: sNode["color"] = {{"background","#121212"},{"border","#ffd700"}}; sNode["font"] = {{"color","white"},{"bold",true}}; break;
                    case 2: sNode["color"] = {{"background","#121212"},{"border","#ff8c00"}}; sNode["font"] = {{"color","white"},{"bold",true}}; break;
                    case 3: sNode["color"] = {{"background","#121212"},{"border","#00d4ff"}}; sNode["font"] = {{"color","white"},{"bold",true}}; break;
                    default: sNode["color"] = {{"background","#121212"},{"border","#888888"}}; sNode["font"] = {{"color","white"},{"bold",true}};
                }
            }
            if (s < stepCount - 1) sNode["nextId"] = "s" + to_string(s+1);
            stepNodes.push_back(sNode);

            if (s > 0) {
                json sEdge;
                sEdge["from"] = "s" + to_string(s-1);
                sEdge["to"] = sid;
                sEdge["arrows"] = "to";
                stepEdges.push_back(sEdge);
            }
        }
    } else {
        json errNode;
        errNode["id"] = "s0"; errNode["shape"] = "circle"; errNode["label"] = "Sin pasos\n(expresion invalida)";
        errNode["x"] = 0; errNode["y"] = 0; errNode["borderWidth"] = 3;
        errNode["color"] = {{"background","#1a0000"},{"border","#ff4444"}}; errNode["font"] = {{"color","#ff4444"},{"bold",true}};
        stepNodes.push_back(errNode);
    }

    delete[] steps; // Liberar memoria del arreglo principal


    // ─────────────────────────────────────────────────────────────
    // FASE 4: ÁRBOL LIBRE DE TRANSICIONES DFA (Grafo Real)
    // ─────────────────────────────────────────────────────────────
    // 1. Crear el historial de saltos (Path)
    json dfa_path_list = json::array();
    dfa_path_list.push_back("START");
    for (size_t i = 0; i < tokens.size(); i++) {
        dfa_path_list.push_back(tokens[i].type);
    }
    if (statusMsg == "OK" || statusMsg.find("ERROR") == string::npos) {
        dfa_path_list.push_back("END");
    }

    json traceNodes = json::array();
    json traceEdges = json::array();

    // 2. Crear SOLAMENTE los estados únicos (Máximo 5 círculos)
    vector<string> uniqueStates;
    for (size_t i = 0; i < dfa_path_list.size(); i++) {
        string stateName = dfa_path_list[i].get<string>();

        // Verificamos si ya dibujamos este círculo
        bool exists = false;
        for(string u : uniqueStates) { if(u == stateName) exists = true; }

        if (!exists) {
            uniqueStates.push_back(stateName);

            json tNode;
            tNode["id"] = stateName; // El ID es el nombre literal del estado
            tNode["label"] = stateName;
            tNode["shape"] = "circle";
            tNode["borderWidth"] = 3;
            tNode["font"] = {{"color", "white"}, {"face", "Monospace"}, {"size", 16}};

            // Colores estilo Dark/Neon
            if (stateName == "START") { tNode["color"] = {{"background", "#121212"}, {"border", "#888888"}}; }
            else if (stateName == "NUMBER") { tNode["color"] = {{"background", "#121212"}, {"border", "#00d4ff"}}; }
            else if (stateName == "FUNCTION") { tNode["color"] = {{"background", "#121212"}, {"border", "#ffd700"}}; }
            else if (stateName == "OPERATOR") { tNode["color"] = {{"background", "#121212"}, {"border", "#ff00ff"}}; }
            else if (stateName == "END") { tNode["color"] = {{"background", "#121212"}, {"border", "#00ff41"}}; tNode["borderWidth"] = 6; }

            traceNodes.push_back(tNode);
        }
    }

    // 3. Conectar los estados con flechas rebotando
    std::map<string, int> edgeCount; // Usamos un mapa para saber cuántas flechas hay entre dos mismos nodos

    for (size_t i = 1; i < dfa_path_list.size(); i++) {
        string fromState = dfa_path_list[i-1].get<string>();
        string toState = dfa_path_list[i].get<string>();

        json tEdge;
        tEdge["from"] = fromState;
        tEdge["to"] = toState;
        tEdge["arrows"] = "to";
        tEdge["label"] = "[" + to_string(i) + "]"; // Muestra el número de paso
        tEdge["font"] = {{"color", "white"}, {"size", 14}, {"align", "top"}};
        tEdge["color"] = {{"color", "#aaaaaa"}};

        // TRUCO MATEMÁTICO: Si hay múltiples flechas entre los mismos estados (Ej. NUMBER a OPERATOR),
        // incrementamos la curvatura para que se abran como un abanico y no se empalmen.
        string edgeKey = fromState + "-" + toState;
        edgeCount[edgeKey]++;
        double curve = 0.15 * edgeCount[edgeKey];

        tEdge["smooth"] = {{"type", "curvedCW"}, {"roundness", curve}};
        tEdge["dashes"] = true;

        traceEdges.push_back(tEdge);
    }

    // ── Empaquetado final ──
    json networks;
    networks["pattern"] = {{"nodes", nodes}, {"edges", edges}};
    networks["input"]   = {{"nodes", stepNodes}, {"edges", stepEdges}};
    // Aquí es donde se vincula lo que se crea con la llave que busca el JS
    networks["dfa_trace"] = {{"nodes", traceNodes}, {"edges", traceEdges}};
    // >>> INICIO NUEVO: Adjuntar el camino al JSON de respuesta <<<
    // Pasamos el arreglo de saltos al JavaScript bajo la llave "dfa_path"
    networks["dfa_path"] = dfa_path;
    // >>> FIN NUEVO <<<

    json done;
    done["arguments"] = networks;
    done["status"]    = statusMsg;
    if (errKind == ErrorKind::None) {
        done["result"] = result;
    }

    return notifyMessage("Math Analysis", done);
}