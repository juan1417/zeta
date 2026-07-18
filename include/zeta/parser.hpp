#pragma once
#include "zeta/lexer.hpp"
#include <vector>
#include <memory>
#include <string>

namespace zeta {

enum class TipoNodoAST {
    LITERAL_NUMERO, LITERAL_CADENA, LITERAL_BOOL, LITERAL_NULL,
    VARIABLE, ASIGNACION, BINARIA, UNARIA, TERNARIA,
    VECTOR, MATRIZ, DICCIONARIO,
    LLAMADA_FUNCION, LLAMADA_METODO, LLAMADA_NAMESPACE,
    ACCESO_COLUMNAS, ACCESO_INDICE, ACCESO_MATRIZ, ACCESO_METODO, ACCESO_NAMESPACE,
    ASIGNACION_INDICE, FILTRO_FILAS,
    BLOQUE, IF_ELSE, FOR_IN, WHILE, RETURN, BREAK, CONTINUE,
    PRINT, DECLARACION_FN, FUNCION_ANONIMA, DECLARACION_CLASE, NEW, THIS,
    INCLUSION, INCLUSION_SELECTIVA, INCLUSION_ALIAS, EXPORTACION,
    PROPAGACION, REGISTRAR_RUTA,
    IS_NULL, IS_ERROR, MEAN, COUNT, PLOT, METRIC, DASHBOARD, LOAD_CSV, SERVE,
    SUM_FN, MIN_FN, MAX_FN, STDDEV,
    ABS, ROUND, FLOOR, CEIL, POW, SQRT,
    LEN, UPPER, LOWER, SUBSTR, REVERSE, SORT_FN, UNIQUE, PUSH,
    KEYS, VALUES, TYPE_FN, RANGE, TRANSPOSE, DOT,
    HEAD, SELECT, SPLIT, JOIN, REPLACE, FIND,
    MAP_FN, FILTER_FN, REDUCE
};

struct NodoAST {
    TipoNodoAST tipo;
    std::string valor_texto;
    double valor_numerico = 0.0;
    bool valor_bool = false;
    std::vector<std::unique_ptr<NodoAST>> hijos;
    std::string nombre_funcion;
    std::vector<std::string> parametros;
    int linea = 0;
    int columna = 0;
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::unique_ptr<NodoAST> parsear();

private:
    std::vector<Token> tokens_;
    size_t posicion_;

    bool comprobar(TipoToken t);
    const Token& actual();
    Token consumir(TipoToken esperado = TipoToken::EOF_TOKEN);
    bool esta_al_final();

    std::unique_ptr<NodoAST> declaracion();
    std::unique_ptr<NodoAST> declaracion_asignacion();
    std::unique_ptr<NodoAST> declaracion_fn();
    std::unique_ptr<NodoAST> declaracion_print();
    std::unique_ptr<NodoAST> declaracion_if();
    std::unique_ptr<NodoAST> declaracion_for();
    std::unique_ptr<NodoAST> declaracion_while();
    std::unique_ptr<NodoAST> declaracion_return();
    std::unique_ptr<NodoAST> declaracion_inclusion();
    std::unique_ptr<NodoAST> declaracion_exportacion();
    std::unique_ptr<NodoAST> declaracion_clase();
    std::unique_ptr<NodoAST> bloque();

    std::unique_ptr<NodoAST> expresion();
    std::unique_ptr<NodoAST> expresion_ternaria();
    std::unique_ptr<NodoAST> expresion_or();
    std::unique_ptr<NodoAST> expresion_and();
    std::unique_ptr<NodoAST> expresion_igualdad();
    std::unique_ptr<NodoAST> expresion_comparacion();
    std::unique_ptr<NodoAST> expresion_termino();
    std::unique_ptr<NodoAST> expresion_factor();
    std::unique_ptr<NodoAST> expresion_unaria();
    std::unique_ptr<NodoAST> expresion_primaria();

    std::unique_ptr<NodoAST> parsear_vector();
    std::unique_ptr<NodoAST> parsear_matriz();
    std::unique_ptr<NodoAST> parsear_diccionario();
    std::unique_ptr<NodoAST> parsear_variable_o_llamada();
    std::unique_ptr<NodoAST> parsear_llamada_funcion(const std::string& nombre);
    std::unique_ptr<NodoAST> parsear_llamada_nativa(TipoToken tipo);
    std::unique_ptr<NodoAST> parsear_lambda_anonima();
    bool peek_siguiente_es_lambada();
};

} // namespace zeta
