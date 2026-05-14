#include "../headers/MathParser.h"
#include <iostream>
#include <cmath>
#include <cctype>

// ─────────────────────────────────────────────────────────────────────────────
// CONSTRUCTOR
// Recibe la cadena de entrada (ej: "5+2") e inicia el proceso de análisis.
// Primero tokeniza (divide en piezas), luego resetea el puntero para el parseo.
// ─────────────────────────────────────────────────────────────────────────────
MathParser::MathParser(std::string input) : input(input), pos(0) {
    tokenize(); // FASE 1: Dividir la cadena en tokens
    pos = 0;    // Resetea el puntero para leer tokens desde el inicio en el parseo
}

/* ═══════════════════════════════════════════════════════════════════════════
 * FASE 1: ANALIZADOR LÉXICO (TOKENIZER)
 * Recorre la cadena caracter por caracter y agrupa los símbolos en categorías:
 *   NUMBER   → secuencia numérica válida (un solo punto decimal permitido)
 *   FUNCTION → palabra reservada (sin, cos, tan)
 *   OPERATOR → símbolo aritmético o paréntesis
 *
 * REGLAS APLICADAS:
 *   ① Números con múltiples puntos (5..2, 5...3) → INVALID_NUMBER
 *   ② Números que comienzan con punto (.2)        → INVALID_NUMBER
 *      NOTA: ".2" NO es válido en este DFA porque el alfabeto requiere
 *            al menos un dígito antes del punto decimal.
 *   ③ Números que terminan en punto (5.)          → INVALID_NUMBER
 *   ④ Operador seguido de punto (5./3)            → detectado en parseo
 * ═══════════════════════════════════════════════════════════════════════════ */
void MathParser::tokenize() {
    for (size_t i = 0; i < input.length(); i++) {

        // ── Ignorar espacios en blanco ──────────────────────────────────────
        if (isspace(input[i])) continue;

        // ── DETECCIÓN DE NÚMEROS ────────────────────────────────────────────
        // Condición de inicio: dígito.
        // Un punto solitario al inicio ya NO inicia un número.
        //   ".2" generará un token OPERATOR para "." y luego NUMBER "2",
        //   lo que el parser rechazará como secuencia inválida.
        if (isdigit(input[i])) {
            std::string num;
            int dotCount = 0;   // Contador de puntos decimales dentro del número

            while (i < input.length() && (isdigit(input[i]) || input[i] == '.')) {
                if (input[i] == '.') {
                    dotCount++;
                    // Si aparece un segundo punto, el número es inválido.
                    // Acumula el resto para reportar el token completo con error.
                    if (dotCount > 1) {
                        num += input[i++];
                        // Acumula dígitos/puntos extras para el mensaje de error
                        while (i < input.length() && (isdigit(input[i]) || input[i] == '.')) {
                            num += input[i++];
                        }
                        // Guarda el token como INVALID_NUMBER; el parser lanzará excepción
                        tokens.push_back({"INVALID_NUMBER", num});
                        i--; // Ajuste del índice por el bucle exterior
                        goto next_char; // Saltar al siguiente caracter
                    }
                }
                num += input[i++]; // Acumula el caracter y avanza
            }
            i--; // Ajusta índice por el incremento extra del while

            // Un número que termina en punto (ej. "5.") es inválido.
            // El punto al final no representa un decimal válido.
            if (!num.empty() && num.back() == '.') {
                tokens.push_back({"INVALID_NUMBER", num});
            } else {
                tokens.push_back({"NUMBER", num}); // Número válido
            }
        }

        // ── DETECCIÓN DE PALABRAS (FUNCIONES TRIGONOMÉTRICAS) ───────────────
        else if (isalpha(input[i])) {
            std::string func;
            while (i < input.length() && isalpha(input[i])) {
                func += input[i++]; // Acumula letras consecutivas
            }
            i--; // Ajuste del índice por el while

            // Solo se aceptan las funciones del alfabeto definido.
            // Cualquier otra palabra será UNKNOWN_FUNCTION y el parser la rechazará.
            if (func == "sin" || func == "cos" || func == "tan") {
                tokens.push_back({"FUNCTION", func});
            } else {
                tokens.push_back({"UNKNOWN_FUNCTION", func});
            }
        }

        // ── DETECCIÓN DE PUNTO SOLITARIO ────────────────────────────────────
        // Si un punto aparece sin un dígito previo (ej. ".2" o "5./3"),
        // se tokeniza como INVALID_DECIMAL para que el parser genere un mensaje claro.
        else if (input[i] == '.') {
            tokens.push_back({"INVALID_DECIMAL", "."});
        }

        // ── DETECCIÓN DE OPERADORES (+, -, *, /, (, )) ───────────────────────
        else {
            std::string op(1, input[i]);
            tokens.push_back({"OPERATOR", op});
        }

        next_char:; // Etiqueta para el salto seguro del tokenizer tras número inválido
    }
}

// Devuelve la lista de tokens para que el servidor pueda dibujar los nodos
std::vector<Token> MathParser::getTokens() { return tokens; }


/* ═══════════════════════════════════════════════════════════════════════════
 * FASE 2: ANALIZADOR SINTÁCTICO (PARSER — DESCENSO RECURSIVO)
 *
 * Técnica: "Recursive Descent" — cada función maneja un nivel de precedencia.
 * Jerarquía de precedencia (de menor a mayor):
 *   expression() → sumas/restas        (prioridad baja)
 *   term()        → mult/división      (prioridad media)
 *   factor()      → paréntesis/unario  (prioridad alta)
 *   function()    → trig               (prioridad más alta)
 * ═══════════════════════════════════════════════════════════════════════════ */


// ── Regla 1: EXPRESIÓN — maneja sumas (+) y restas (-) ─────────────────────
double MathParser::expression() {

    // Operador al inicio de la expresión (ej. "+1-2")
    // Si el primer token es un operador binario (no el unario "-"), lanza el error descriptivo.
    if (pos < tokens.size() &&
        (tokens[pos].value == "+" || tokens[pos].value == "*" || tokens[pos].value == "/")) {
        throw std::runtime_error(
            "Expresion incompleta: se encontro el operador '" + tokens[pos].value +
            "' al inicio sin un valor previo para operar. "
            "Ejemplo correcto: 5" + tokens[pos].value + "2"
        );
    }

    double result = term(); // Primero resuelve multiplicaciones/divisiones

    while (pos < tokens.size() && (tokens[pos].value == "+" || tokens[pos].value == "-")) {
        std::string op = tokens[pos++].value; // Consume el operador

        //  Operador al final de la expresión (ej. "5+")
        // Si tras consumir el operador no hay más tokens, es una expresión incompleta.
        if (pos >= tokens.size()) {
            throw std::runtime_error(
                "Expresion incompleta: se esperaba un valor despues del operador '" + op +
                "'. La expresion no puede terminar con un operador."
            );
        }

        if (op == "+") result += term();
        else           result -= term();
    }
    return result;
}

// ── Regla 2: TÉRMINO — maneja multiplicaciones (*) y divisiones (/) ─────────
double MathParser::term() {
    double result = factor(); // Primero resuelve paréntesis, funciones o números

    while (pos < tokens.size() && (tokens[pos].value == "*" || tokens[pos].value == "/")) {
        std::string op = tokens[pos++].value; // Consume el operador

        // (en multiplicación/división): Operador al final
        if (pos >= tokens.size()) {
            throw std::runtime_error(
                "Expresion incompleta: se esperaba un valor despues del operador '" + op +
                "'. La expresion no puede terminar con un operador."
            );
        }

        if (op == "*") result *= factor();
        else           result /= factor();
    }
    return result;
}

/* ── Regla 3: FACTOR — prioridad más alta ──────────────────────────────────
 * Resuelve:
 *   - Paréntesis  → reinicia el ciclo de prioridad con expression()
 *   - Funciones   → delega a function()
 *   - Signo '-'   → negación unaria
 *   - Números     → extrae el valor directamente
 *
 * Funcion:
 *   ① Tokens INVALID_NUMBER  → excepción con mensaje claro
 *   ② Tokens INVALID_DECIMAL → excepción indicando punto solitario
 *   ③ Funciones sin '('       → excepción indicando paréntesis faltante
 *   ④ UNKNOWN_FUNCTION        → excepción indicando función no reconocida
 * ────────────────────────────────────────────────────────────────────────── */
double MathParser::factor() {

    // Verificación de límite de tokens antes de operar
    if (pos >= tokens.size()) {
        throw std::runtime_error(
            "Expresion incompleta: se esperaba un numero, funcion o '(' pero la expresion termino abruptamente."
        );
    }

    // ── NÚMERO INVÁLIDO (múltiples puntos, termina en punto) ─────────────────
    //  "5..2", "5...3", "5." → token INVALID_NUMBER
    if (tokens[pos].type == "INVALID_NUMBER") {
        throw std::runtime_error(
            "Numero invalido: '" + tokens[pos].value + "'. "
            "Un numero decimal solo puede tener un punto (ej. 3.14). "
            "Expresiones como '5..2' o '5.' no son validas en el alfabeto definido."
        );
    }

    // ── PUNTO DECIMAL SOLITARIO ───────────────────────────────────────────────
    // Si hay algun numero de esta manera ".2" o "5./" → token INVALID_DECIMAL o punto tras operador
    if (tokens[pos].type == "INVALID_DECIMAL") {
        throw std::runtime_error(
            "Punto decimal invalido: '.' encontrado sin un digito previo. "
            "El alfabeto requiere al menos un digito antes del punto (ej. 0.2 en lugar de .2). "
            "Si aparece entre operadores (ej. '5./3'), elimine el punto innecesario."
        );
    }

    // ── FUNCIÓN DESCONOCIDA ───────────────────────────────────────────────────
    if (tokens[pos].type == "UNKNOWN_FUNCTION") {
        throw std::runtime_error(
            "Funcion no reconocida: '" + tokens[pos].value + "'. "
            "Las funciones permitidas son: sin, cos, tan."
        );
    }

    // ── PARÉNTESIS DE APERTURA → evaluación recursiva ────────────────────────
    if (tokens[pos].value == "(") {
        pos++; // Consume el '('
        double result = expression(); // Evalúa recursivamente el contenido

        // Validación: debe existir el ')' de cierre
        if (pos >= tokens.size() || tokens[pos].value != ")") {
            throw std::runtime_error(
                "Parentesis no cerrado: se encontro '(' sin su ')' correspondiente. "
                "Revise el balanceo de parentesis."
            );
        }
        pos++; // Consume el ')'
        return result;
    }

    // ── FUNCIÓN TRIGONOMÉTRICA ────────────────────────────────────────────────
    // Si la función existe pero el siguiente token NO es '(',
    //   lanza un error descriptivo en lugar de abortar el programa.
    if (tokens[pos].type == "FUNCTION") {
        // Verifica que el token siguiente sea '(' antes de llamar a function()
        if (pos + 1 >= tokens.size() || tokens[pos + 1].value != "(") {
            throw std::runtime_error(
                "Llamada invalida a funcion '" + tokens[pos].value + "': "
                "se esperaba '(' despues del nombre de la funcion. "
                "Uso correcto: " + tokens[pos].value + "(angulo). "
                "Ejemplo: sin(90) en lugar de sin90."
            );
        }
        return function(); // Delega a la regla de funciones
    }

    // ── SIGNO NEGATIVO UNARIO ─────────────────────────────────────────────────
    if (tokens[pos].value == "-") {
        pos++; // Consume el '-'
        return -factor(); // Aplica negación al siguiente factor
    }

    // ── NÚMERO VÁLIDO ─────────────────────────────────────────────────────────
    // Se usa stod para convertir la cadena numérica a double.
    // Si el token no es un número válido aquí, stod lanzará una excepción.
    if (tokens[pos].type != "NUMBER") {
        throw std::runtime_error(
            "Se esperaba un numero o expresion valida, pero se encontro: '" +
            tokens[pos].value + "' (tipo: " + tokens[pos].type + ")."
        );
    }
    return std::stod(tokens[pos++].value); // Extrae el valor y avanza el puntero
}


/* ═══════════════════════════════════════════════════════════════════════════
 * EVALUADOR DE FUNCIONES TRIGONOMÉTRICAS
 *
 * IMPORTANTE: C++ usa radianes. Multiplicamos por (PI/180) para que el
 * usuario pueda ingresar ángulos en Grados Sexagesimales.
 *
 * Permite anidamiento: sin(cos(0)) funciona por recursividad de expression().
 * ═══════════════════════════════════════════════════════════════════════════ */
double MathParser::function() {
    const double PI = 3.14159265358979323846; // Definición local de PI

    std::string name = tokens[pos++].value; // Guarda el nombre (ej. "sin") y avanza
    pos++; // Consume el '(' que sigue a la función (validado previamente en factor())

    // EVALUACIÓN RECURSIVA del argumento (permite funciones anidadas)
    double val = expression();

    // Valida que exista el ')' de cierre de la función
    if (pos >= tokens.size() || tokens[pos].value != ")") {
        throw std::runtime_error(
            "Parentesis de cierre faltante en la funcion '" + name + "'. "
            "Uso correcto: " + name + "(angulo)."
        );
    }
    pos++; // Consume el ')'

    // Conversión Grados → Radianes antes de calcular
    if (name == "sin") return std::sin(val * PI / 180.0);
    if (name == "cos") return std::cos(val * PI / 180.0);
    if (name == "tan") return std::tan(val * PI / 180.0);

    // Nunca debería llegar aquí porque factor() valida el nombre antes
    throw std::runtime_error("Funcion interna desconocida: '" + name + "'.");
}


/* ═══════════════════════════════════════════════════════════════════════════
 * PUNTO DE ENTRADA — parse()
 *
 * Inicia el descenso recursivo y verifica que se consumieron TODOS los tokens.
 * Si quedan tokens sin procesar, la expresión tiene caracteres extra inválidos.
 * ═══════════════════════════════════════════════════════════════════════════ */
double MathParser::parse() {
    // 1. Iniciar la evaluación desde la regla de menor precedencia
    double res = expression();

    // 2. REVISIÓN FINAL: Si 'pos' no llegó al final del vector de tokens,
    //    hay símbolos extra que no pertenecen a ninguna operación válida.
    if (pos < tokens.size()) {
        throw std::runtime_error(
            "Caracteres extra al final de la expresion: '" +
            tokens[pos].value + "'. "
            "Verifique que la expresion este completa y bien formada."
        );
    }

    return res;
}
