//Este archivo es el plano del analizador.
//Aquí es donde se define la estructura de datos y las reglas que el compilador debe seguir.
#ifndef MATHPARSER_H
#define MATHPARSER_H

#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/*Unidad mínima de significado dentro de la gramática matemática.
 * Representa el componente básico (átomo) después del análisis léxico.
 */
struct Token {
    std::string type;  // Categoría: "NUMBER", "FUNCTION", "OPERATOR"
    std::string value; // Valor literal: "sin", "+", "3.14"
};

/*Motor de Análisis Léxico y Sintáctico (Lexer/Parser).
 * Implementa una gramática libre de contexto mediante el método de
 * Descenso Recurrente para evaluar expresiones aritméticas y trigonométricas.
 */
class MathParser {
public:
    // Constructor: Inicia el proceso de tokenización al instanciar la clase
    MathParser(std::string input);

    // Método principal que dispara la evaluación sintáctica y retorna el valor numérico
    double parse();

    // Interfaz para recuperar la lista de tokens generada (usada para el dibujo de nodos)
    std::vector<Token> getTokens();

private:
    std::string input;         // Cadena de caracteres original (input del usuario)
    size_t pos;                // Cursor/Puntero de lectura para el recorrido de la lista de tokens
    std::vector<Token> tokens; // Tabla de símbolos generada por el Lexer

    /* Escáner Léxico (Scanner/Lexer).
     * Transforma la cadena de texto en una secuencia de objetos Token.
     */
    void tokenize();

    /*Regla de Producción: Expresión.
     * Maneja las operaciones de menor precedencia (Adición y Sustracción).
     */
    double expression();

    /* Regla de Producción: Término.
     * Maneja operaciones de precedencia media (Multiplicación y División).
     */
    double term();

    /* Regla de Producción: Factor.
     * Maneja la precedencia más alta: paréntesis, signos negativos y llamadas a funciones.
     */
    double factor();

    /*Analizador de Funciones Especiales.
     * Gestiona la lógica trigonométrica (sin, cos, tan) y su encapsulamiento matemático.
     */
    double function();

    // Métodos auxiliares de lectura de flujo (Stream handling)
    char peek(); // Observa el siguiente carácter sin avanzar el puntero
    char get();  // Recupera el carácter actual y avanza el puntero de lectura
};

#endif