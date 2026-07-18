#pragma once
#include <string>
#include <vector>

namespace zeta {

enum class TipoToken {
    SIGIL, IDENTIFICADOR, NUMERO, CADENA,
    TRUE_LITERAL, FALSE_LITERAL, NULL_LITERAL,
    VECTOR_ABRE, VECTOR_CIERRA, MATRIZ_ABRE, MATRIZ_CIERRA,
    DICCIONARIO_ABRE, DICCIONARIO_CIERRA, COMA,
    ASIGNAR, SUMA, RESTA, MULTIPLICACION, DIVISION, MODULO,
    IGUAL, DIFERENTE, MAYOR_QUE, MENOR_QUE, MAYOR_IGUAL, MENOR_IGUAL,
    AND, OR, NOT,
    EXTRACTOR, TERNARIO_PREG, TERNARIO_SINO,
    FILTRO_ABRE, FILTRO_CIERRA, CORCHETE_ABRE, CORCHETE_CIERRA,
    PARENTESIS_ABRE, PARENTESIS_CIERRA,
    FN, IF, ELSE, FOR, WHILE, IN, RETURN, PRINT, BREAK, CONTINUE,
    INCLUDE, AS, EXPORT, PUNTO_DOBLE,
    LOAD_CSV, PLOT, SERVE, METRIC, DASHBOARD, ROUTE,
    IS_NULL, IS_ERROR, MEAN, COUNT, SUM_FN, MIN_FN, MAX_FN, STDDEV,
    ABS, ROUND, FLOOR, CEIL, POW, SQRT,
    LEN, UPPER, LOWER, SUBSTR, REVERSE, SORT_FN, UNIQUE, PUSH,
    KEYS, VALUES, TYPE_FN, RANGE, TRANSPOSE, DOT,
    HEAD, SELECT, SPLIT, JOIN, REPLACE, FIND,
    MAP_FN, FILTER_FN, REDUCE,
    PUNTO,
    CLASS, NEW, THIS, EXTENDS,
    EOF_TOKEN
};

struct Token {
    TipoToken tipo;
    std::string lexema;
    int linea;
    int columna;
    double valor_numero;
    std::string valor_cadena;

    Token(TipoToken t, const std::string& lex, int lin, int col)
        : tipo(t), lexema(lex), linea(lin), columna(col),
          valor_numero(0.0), valor_cadena("") {}
};

class Lexer {
public:
    explicit Lexer(std::string fuente);
    std::vector<Token> tokenizar();

private:
    std::string fuente_;
    size_t posicion_;
    int linea_;
    int columna_;
    bool esperando_valor_;
    int profundidad_coleccion_;

    char avanzar();
    char peek();
    bool esta_al_final();
    bool es_alpha(char c);
    bool es_digito(char c);
    void omitir_espacio_blanco();
    Token siguiente_token();
    Token tokenizar_identificador();
    Token tokenizar_numero();
    Token tokenizar_cadena();
};

} // namespace zeta
