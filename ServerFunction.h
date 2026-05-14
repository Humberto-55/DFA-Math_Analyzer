//1. CEREBRO (C++)
//Es el plano arquitectónico. Aquí se declara qué funciones existen y se define el "diccionario" (map)
//para que el servidor sepa a dónde dirigir cada mensaje que llega de la web.

//1. GUARDAS DE INCLUSION
// Evita que este archivo se incluya múltiples veces durante la compilación,
// previniendo errores de redefinición de la clase.
#ifndef PHDFUNCTIONS_H
#define PHDFUNCTIONS_H
//Si otro archivo .h y un .cpp incluyen ServerFunction.h al mismo tiempo, el compilador
//arrojara un erro de "Class redefinition" (Clase redefinida) y no compilara

//En caso de cambiar el nombre mientras #ifndef y #define tengan exactamente la misma palabra
//inventada (ej. #define MIO_H), funcionara. Si son diferentes, las guardas fallaran
#include "globalHeaders.h"

class ServerFunction
{
public:
    ServerFunction();
    //2. PUNTEROS A FUNCIONES (TYPEDEF)
    // Crea un "alias" llamado srvrFunction para cualquier función de esta clase
    // que reciba un JSON y devuelva un string. Es la base del enrutador dinámico.
    typedef std::string (ServerFunction::*srvrFunction)(json doc);  //Si se quitara esta linea
    //la declaracion del mapa usrFUnction en la parte privada fallara porque no sabra que tipo de datos es srvrFunction
    //En caso de modificar (json doc) por (string doc), el servidor ya no podra procesar directamente los objetos JSON
    //estructurados, rompiendo toda la comunicacion con el frontend
    //Web-Server Functions
    std::string contact(json doc);
    std::string finiteAutomataFunction(json doc) ;
    std::string mathAnalyzerFunction(json doc);

    //FUNCIONES CON VALORES POR DEFECTO (Default Arguments)
    vector<string> jsonKeys(json obj);
    std::string processAction(json doc);
    // Los valores '= false' y '= 0' son parámetros por defecto.
    // Permiten llamar a la función enviando solo el 'pattern' sin obligar a enviar los otros dos datos.
    vector<json> parseNodes(string pattern, bool stringNetwork = false, int startId = 0) ;  //Estos valores son clave,
    //porque en caso de que se quitaran, en el .h, cada vez que se llame a parseNodes(pattern) en el .cpp
    //el compilador marcara error exigiendo que se le pase exactamente 3 argumentos
    //Los valores por defect le dan flexibilidad

    vector<json> parseEdges(vector<json> nodes,bool stringNetwork = false) ;


    ServerFunction::srvrFunction getFunction(std::string name, bool *ok);

    string notifyMessage(std::string name, json message);
    std::string rxMessage( string msg);

    //4. DICCIONARIO DE RUTAS (EL MAPA PRIVADO)
private:
    // Diccionario en memoria que asocia el nombre de la acción en texto
    // (ej. "contact") con la dirección de memoria de la función a ejecutar.

    map<std::string, srvrFunction> usrFunction; //si se quitara, se destruye el mecanismo de enrutamiento del servidor
    //La funcion processAction no tendria donde buscar que codigo ejecutar cuando llega un mensaje de la web.
    //Si se moviera a public: Seguiria funcionando, pero rompe el principio de encapsulamiento de POO.
    //El diccionario solo debe ser manipulado internamente por la clase
};



#endif //PHDFUNCTIONS_H
