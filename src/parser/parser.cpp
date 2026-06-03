#include "zeta/parser.hpp"
#include <stdexcept>

namespace zeta {

Parser::Parser(std::vector<Token> tokens)
    : tokens_(std::move(tokens)), posicion_(0) {}

static std::unique_ptr<NodoAST> mk_num_literal(double val) {
    auto nodo = std::make_unique<NodoAST>();
    nodo->tipo = TipoNodoAST::LITERAL_NUMERO;
    nodo->valor_numerico = val;
    return nodo;
}

bool Parser::comprobar(TipoToken t) {
    return !esta_al_final() && tokens_[posicion_].tipo == t;
}

const Token& Parser::actual() {
    return tokens_[posicion_];
}

Token Parser::consumir(TipoToken esperado) {
    if (comprobar(esperado) || esperado == TipoToken::EOF_TOKEN) {
        return tokens_[posicion_++];
    }
    throw std::runtime_error("Token inesperado: esperaba tipo " + std::to_string(static_cast<int>(esperado))
        + " pero obtuvo '" + actual().lexema + "' en linea " + std::to_string(actual().linea));
}

bool Parser::esta_al_final() {
    return posicion_ >= tokens_.size() || tokens_[posicion_].tipo == TipoToken::EOF_TOKEN;
}

std::unique_ptr<NodoAST> Parser::parsear() {
    std::vector<std::unique_ptr<NodoAST>> declaraciones;
    while (!esta_al_final()) {
        declaraciones.push_back(declaracion());
    }
    auto bloque = std::make_unique<NodoAST>();
    bloque->tipo = TipoNodoAST::BLOQUE;
    bloque->hijos = std::move(declaraciones);
    return bloque;
}

std::unique_ptr<NodoAST> Parser::declaracion() {
    if (comprobar(TipoToken::CLASS)) return declaracion_clase();
    if (comprobar(TipoToken::FN)) return declaracion_fn();
    if (comprobar(TipoToken::PRINT)) return declaracion_print();
    if (comprobar(TipoToken::IF)) return declaracion_if();
    if (comprobar(TipoToken::FOR)) return declaracion_for();
    if (comprobar(TipoToken::WHILE)) return declaracion_while();
    if (comprobar(TipoToken::RETURN)) return declaracion_return();
    if (comprobar(TipoToken::INCLUDE)) return declaracion_inclusion();
    if (comprobar(TipoToken::EXPORT)) return declaracion_exportacion();
    if (comprobar(TipoToken::BREAK)) { consumir(TipoToken::BREAK); auto n = std::make_unique<NodoAST>(); n->tipo = TipoNodoAST::BREAK; return n; }
    if (comprobar(TipoToken::CONTINUE)) { consumir(TipoToken::CONTINUE); auto n = std::make_unique<NodoAST>(); n->tipo = TipoNodoAST::CONTINUE; return n; }
    return declaracion_asignacion();
}

std::unique_ptr<NodoAST> Parser::declaracion_asignacion() {
    if (!comprobar(TipoToken::SIGIL)) {
        return expresion();
    }

    size_t saved_pos = posicion_;

    consumir(TipoToken::SIGIL);
    auto nombre = consumir(TipoToken::IDENTIFICADOR);

    auto nodo = std::make_unique<NodoAST>();
    nodo->tipo = TipoNodoAST::VARIABLE;
    nodo->valor_texto = nombre.lexema;

    if (comprobar(TipoToken::CORCHETE_ABRE)) {
        consumir(TipoToken::CORCHETE_ABRE);
        auto primer_expr = expresion();

        if (comprobar(TipoToken::COMA)) {
            consumir(TipoToken::COMA);
            auto segunda_expr = expresion();
            consumir(TipoToken::CORCHETE_CIERRA);

            auto acceso = std::make_unique<NodoAST>();
            acceso->tipo = TipoNodoAST::ACCESO_MATRIZ;
            acceso->hijos.push_back(std::move(nodo));
            acceso->hijos.push_back(std::move(primer_expr));
            acceso->hijos.push_back(std::move(segunda_expr));
            nodo = std::move(acceso);
        } else {
            consumir(TipoToken::CORCHETE_CIERRA);

            auto acceso = std::make_unique<NodoAST>();
            acceso->tipo = TipoNodoAST::ACCESO_INDICE;
            acceso->hijos.push_back(std::move(nodo));
            acceso->hijos.push_back(std::move(primer_expr));
            nodo = std::move(acceso);
        }

        if (comprobar(TipoToken::ASIGNAR)) {
            consumir(TipoToken::ASIGNAR);
            auto valor = expresion();

            auto asign = std::make_unique<NodoAST>();
            asign->tipo = TipoNodoAST::ASIGNACION_INDICE;
            asign->hijos.push_back(std::move(nodo));
            asign->hijos.push_back(std::move(valor));
            asign->linea = nombre.linea;
            return asign;
        }

        posicion_ = saved_pos;
        return expresion();
    }

    while (comprobar(TipoToken::EXTRACTOR)) {
        consumir(TipoToken::EXTRACTOR);
        auto col = consumir(TipoToken::IDENTIFICADOR);

        auto acceso = std::make_unique<NodoAST>();
        acceso->tipo = TipoNodoAST::ACCESO_COLUMNAS;
        acceso->valor_texto = col.lexema;
        acceso->hijos.push_back(std::move(nodo));
        nodo = std::move(acceso);
    }

    // Field access on a variable: $obj.campo (read or assign)
    while (comprobar(TipoToken::PUNTO)) {
        consumir(TipoToken::PUNTO);
        if (!(comprobar(TipoToken::IDENTIFICADOR) ||
              comprobar(TipoToken::COUNT) || comprobar(TipoToken::MEAN) ||
              comprobar(TipoToken::SUM_FN) || comprobar(TipoToken::MAX_FN) ||
              comprobar(TipoToken::MIN_FN) || comprobar(TipoToken::STDDEV) ||
              comprobar(TipoToken::ABS) || comprobar(TipoToken::ROUND) ||
              comprobar(TipoToken::FLOOR) || comprobar(TipoToken::CEIL) ||
              comprobar(TipoToken::POW) || comprobar(TipoToken::SQRT) ||
              comprobar(TipoToken::LEN) || comprobar(TipoToken::UPPER) ||
              comprobar(TipoToken::LOWER) || comprobar(TipoToken::SUBSTR) ||
              comprobar(TipoToken::REVERSE) || comprobar(TipoToken::SORT_FN) ||
              comprobar(TipoToken::UNIQUE) || comprobar(TipoToken::PUSH) ||
              comprobar(TipoToken::KEYS) || comprobar(TipoToken::VALUES) ||
              comprobar(TipoToken::TYPE_FN) || comprobar(TipoToken::RANGE) ||
              comprobar(TipoToken::TRANSPOSE) || comprobar(TipoToken::DOT) ||
              comprobar(TipoToken::HEAD) || comprobar(TipoToken::SELECT) ||
              comprobar(TipoToken::SPLIT) || comprobar(TipoToken::JOIN) ||
              comprobar(TipoToken::REPLACE) || comprobar(TipoToken::FIND) ||
              comprobar(TipoToken::MAP_FN) || comprobar(TipoToken::FILTER_FN) ||
              comprobar(TipoToken::REDUCE) || comprobar(TipoToken::IS_NULL) ||
              comprobar(TipoToken::IS_ERROR) || comprobar(TipoToken::PRINT) ||
              comprobar(TipoToken::LOAD_CSV) || comprobar(TipoToken::PLOT) ||
              comprobar(TipoToken::SERVE) || comprobar(TipoToken::METRIC) ||
              comprobar(TipoToken::DASHBOARD) || comprobar(TipoToken::ROUTE))) {
            throw std::runtime_error("Se esperaba nombre de campo despues de '.'");
        }
        std::string fname = consumir().lexema;
        auto acc = std::make_unique<NodoAST>();
        acc->tipo = TipoNodoAST::ACCESO_METODO;
        acc->valor_texto = fname;
        acc->hijos.push_back(std::move(nodo));
        nodo = std::move(acc);
        if (comprobar(TipoToken::ASIGNAR)) {
            consumir(TipoToken::ASIGNAR);
            auto valor = expresion();
            auto asign = std::make_unique<NodoAST>();
            asign->tipo = TipoNodoAST::ASIGNACION;
            asign->valor_texto = "FIELD:" + fname;  // marker: target is the field-access on stack
            asign->hijos.push_back(std::move(nodo));
            asign->hijos.push_back(std::move(valor));
            return asign;
        }
    }

    if (comprobar(TipoToken::PARENTESIS_ABRE)) {
        consumir(TipoToken::PARENTESIS_ABRE);
        auto indice = expresion();
        consumir(TipoToken::PARENTESIS_CIERRA);

        auto acceso = std::make_unique<NodoAST>();
        acceso->tipo = TipoNodoAST::ACCESO_INDICE;
        acceso->hijos.push_back(std::move(nodo));
        acceso->hijos.push_back(std::move(indice));
        nodo = std::move(acceso);

        if (comprobar(TipoToken::ASIGNAR)) {
            consumir(TipoToken::ASIGNAR);
            auto valor = expresion();

            if (comprobar(TipoToken::TERNARIO_PREG)) {
                consumir(TipoToken::TERNARIO_PREG);
                auto prop = std::make_unique<NodoAST>();
                prop->tipo = TipoNodoAST::PROPAGACION;
                prop->hijos.push_back(std::move(valor));
                valor = std::move(prop);
            }

            auto asign = std::make_unique<NodoAST>();
            asign->tipo = TipoNodoAST::ASIGNACION_INDICE;
            asign->hijos.push_back(std::move(nodo));
            asign->hijos.push_back(std::move(valor));
            asign->linea = nombre.linea;
            asign->columna = nombre.columna;
            return asign;
        }
    }

    if (comprobar(TipoToken::FILTRO_ABRE)) {
        consumir(TipoToken::FILTRO_ABRE);
        auto condicion = expresion();
        consumir(TipoToken::FILTRO_CIERRA);

        auto filtro = std::make_unique<NodoAST>();
        filtro->tipo = TipoNodoAST::FILTRO_FILAS;
        filtro->hijos.push_back(std::move(nodo));
        filtro->hijos.push_back(std::move(condicion));
        nodo = std::move(filtro);
    }

    if (comprobar(TipoToken::ASIGNAR)) {
        consumir(TipoToken::ASIGNAR);
        auto valor = expresion();

        if (comprobar(TipoToken::TERNARIO_PREG)) {
            consumir(TipoToken::TERNARIO_PREG);
            auto prop = std::make_unique<NodoAST>();
            prop->tipo = TipoNodoAST::PROPAGACION;
            prop->hijos.push_back(std::move(valor));
            valor = std::move(prop);
        }

        auto asign = std::make_unique<NodoAST>();
        asign->tipo = TipoNodoAST::ASIGNACION;
        asign->valor_texto = nombre.lexema;
        asign->hijos.push_back(std::move(valor));
        asign->linea = nombre.linea;
        asign->columna = nombre.columna;
        return asign;
    }

    posicion_ = saved_pos;
    return expresion();
}

std::unique_ptr<NodoAST> Parser::declaracion_fn() {
    consumir(TipoToken::FN);
    std::string nombre_fn;
    if (comprobar(TipoToken::IDENTIFICADOR)) {
        nombre_fn = consumir(TipoToken::IDENTIFICADOR).lexema;
    } else {
        nombre_fn = consumir().lexema;
    }
    consumir(TipoToken::PARENTESIS_ABRE);

    std::vector<std::string> params;
    if (!comprobar(TipoToken::PARENTESIS_CIERRA)) {
        do {
            consumir(TipoToken::SIGIL);
            params.push_back(consumir(TipoToken::IDENTIFICADOR).lexema);
        } while (comprobar(TipoToken::COMA) && (consumir(TipoToken::COMA), true));
    }
    consumir(TipoToken::PARENTESIS_CIERRA);

    auto cuerpo = bloque();

    auto nodo = std::make_unique<NodoAST>();
    nodo->tipo = TipoNodoAST::DECLARACION_FN;
    nodo->valor_texto = nombre_fn;
    nodo->parametros = params;
    nodo->hijos.push_back(std::move(cuerpo));
    nodo->linea = 0;
    return nodo;
}

std::unique_ptr<NodoAST> Parser::declaracion_print() {
    consumir(TipoToken::PRINT);
    consumir(TipoToken::PARENTESIS_ABRE);

    auto nodo = std::make_unique<NodoAST>();
    nodo->tipo = TipoNodoAST::PRINT;

    if (!comprobar(TipoToken::PARENTESIS_CIERRA)) {
        do {
            nodo->hijos.push_back(expresion());
        } while (comprobar(TipoToken::COMA) && (consumir(TipoToken::COMA), true));
    }
    consumir(TipoToken::PARENTESIS_CIERRA);
    return nodo;
}

std::unique_ptr<NodoAST> Parser::declaracion_if() {
    consumir(TipoToken::IF);
    consumir(TipoToken::PARENTESIS_ABRE);
    auto condicion = expresion();
    consumir(TipoToken::PARENTESIS_CIERRA);

    auto bloque_si = bloque();

    std::unique_ptr<NodoAST> bloque_sino;
    if (comprobar(TipoToken::ELSE)) {
        consumir(TipoToken::ELSE);
        if (comprobar(TipoToken::IF)) {
            bloque_sino = declaracion_if();
        } else {
            bloque_sino = bloque();
        }
    }

    auto nodo = std::make_unique<NodoAST>();
    nodo->tipo = TipoNodoAST::IF_ELSE;
    nodo->hijos.push_back(std::move(condicion));
    nodo->hijos.push_back(std::move(bloque_si));
    if (bloque_sino) nodo->hijos.push_back(std::move(bloque_sino));
    return nodo;
}

std::unique_ptr<NodoAST> Parser::declaracion_for() {
    consumir(TipoToken::FOR);
    consumir(TipoToken::PARENTESIS_ABRE);
    consumir(TipoToken::SIGIL);
    auto var = consumir(TipoToken::IDENTIFICADOR);
    consumir(TipoToken::IN);
    auto coleccion = expresion();
    consumir(TipoToken::PARENTESIS_CIERRA);

    auto cuerpo = bloque();

    auto nodo = std::make_unique<NodoAST>();
    nodo->tipo = TipoNodoAST::FOR_IN;
    nodo->valor_texto = var.lexema;
    nodo->hijos.push_back(std::move(coleccion));
    nodo->hijos.push_back(std::move(cuerpo));
    return nodo;
}

std::unique_ptr<NodoAST> Parser::declaracion_while() {
    consumir(TipoToken::WHILE);
    consumir(TipoToken::PARENTESIS_ABRE);
    auto condicion = expresion();
    consumir(TipoToken::PARENTESIS_CIERRA);

    auto cuerpo = bloque();

    auto nodo = std::make_unique<NodoAST>();
    nodo->tipo = TipoNodoAST::WHILE;
    nodo->hijos.push_back(std::move(condicion));
    nodo->hijos.push_back(std::move(cuerpo));
    return nodo;
}

std::unique_ptr<NodoAST> Parser::declaracion_return() {
    consumir(TipoToken::RETURN);
    auto valor = expresion();

    auto nodo = std::make_unique<NodoAST>();
    nodo->tipo = TipoNodoAST::RETURN;
    nodo->hijos.push_back(std::move(valor));
    return nodo;
}

std::unique_ptr<NodoAST> Parser::declaracion_inclusion() {
    consumir(TipoToken::INCLUDE);
    auto ruta = consumir(TipoToken::CADENA);
    auto nodo = std::make_unique<NodoAST>();
    nodo->nombre_funcion = ruta.valor_cadena;
    nodo->linea = ruta.linea;

    if (comprobar(TipoToken::PUNTO_DOBLE)) {
        consumir(TipoToken::PUNTO_DOBLE);
        consumir(TipoToken::DICCIONARIO_ABRE);
        nodo->tipo = TipoNodoAST::INCLUSION_SELECTIVA;
        if (!comprobar(TipoToken::DICCIONARIO_CIERRA)) {
            do {
                if (comprobar(TipoToken::IDENTIFICADOR)) {
                    nodo->parametros.push_back(consumir(TipoToken::IDENTIFICADOR).lexema);
                } else {
                    nodo->parametros.push_back(consumir().lexema);
                }
            } while (comprobar(TipoToken::COMA) && (consumir(TipoToken::COMA), true));
        }
        consumir(TipoToken::DICCIONARIO_CIERRA);
        return nodo;
    }

    if (comprobar(TipoToken::AS)) {
        consumir(TipoToken::AS);
        std::string alias_nombre;
        if (comprobar(TipoToken::IDENTIFICADOR)) {
            alias_nombre = consumir(TipoToken::IDENTIFICADOR).lexema;
        } else {
            alias_nombre = consumir().lexema;
        }
        nodo->tipo = TipoNodoAST::INCLUSION_ALIAS;
        nodo->valor_texto = alias_nombre;
        return nodo;
    }

    nodo->tipo = TipoNodoAST::INCLUSION;
    return nodo;
}

std::unique_ptr<NodoAST> Parser::declaracion_exportacion() {
    consumir(TipoToken::EXPORT);
    consumir(TipoToken::DICCIONARIO_ABRE);
    auto nodo = std::make_unique<NodoAST>();
    nodo->tipo = TipoNodoAST::EXPORTACION;
    if (!comprobar(TipoToken::DICCIONARIO_CIERRA)) {
        do {
            if (comprobar(TipoToken::IDENTIFICADOR)) {
                nodo->parametros.push_back(consumir(TipoToken::IDENTIFICADOR).lexema);
            } else {
                nodo->parametros.push_back(consumir().lexema);
            }
        } while (comprobar(TipoToken::COMA) && (consumir(TipoToken::COMA), true));
    }
    consumir(TipoToken::DICCIONARIO_CIERRA);
    return nodo;
}

std::unique_ptr<NodoAST> Parser::declaracion_clase() {
    consumir(TipoToken::CLASS);

    auto nodo = std::make_unique<NodoAST>();
    nodo->tipo = TipoNodoAST::DECLARACION_CLASE;

    if (!comprobar(TipoToken::IDENTIFICADOR)) {
        throw std::runtime_error("Se esperaba nombre de clase despues de 'class'");
    }
    nodo->valor_texto = consumir(TipoToken::IDENTIFICADOR).lexema;

    if (comprobar(TipoToken::EXTENDS)) {
        consumir(TipoToken::EXTENDS);
        if (!comprobar(TipoToken::IDENTIFICADOR)) {
            throw std::runtime_error("Se esperaba nombre de clase padre despues de 'extends'");
        }
        nodo->parametros.push_back(consumir(TipoToken::IDENTIFICADOR).lexema);
    }

    consumir(TipoToken::DICCIONARIO_ABRE);

    // Members: $campo = expr; or fn nombre(...) { body }
    while (!comprobar(TipoToken::DICCIONARIO_CIERRA) && !esta_al_final()) {
        if (comprobar(TipoToken::SIGIL)) {
            // Field declaration: $name = expr
            consumir(TipoToken::SIGIL);
            // Accept any identifier-or-keyword as field name
            std::string nombre_campo = "$";
            if (comprobar(TipoToken::IDENTIFICADOR) ||
                comprobar(TipoToken::COUNT) || comprobar(TipoToken::MEAN) ||
                comprobar(TipoToken::SUM_FN) || comprobar(TipoToken::MAX_FN) ||
                comprobar(TipoToken::MIN_FN) || comprobar(TipoToken::STDDEV) ||
                comprobar(TipoToken::ABS) || comprobar(TipoToken::ROUND) ||
                comprobar(TipoToken::FLOOR) || comprobar(TipoToken::CEIL) ||
                comprobar(TipoToken::POW) || comprobar(TipoToken::SQRT) ||
                comprobar(TipoToken::LEN) || comprobar(TipoToken::UPPER) ||
                comprobar(TipoToken::LOWER) || comprobar(TipoToken::SUBSTR) ||
                comprobar(TipoToken::REVERSE) || comprobar(TipoToken::SORT_FN) ||
                comprobar(TipoToken::UNIQUE) || comprobar(TipoToken::PUSH) ||
                comprobar(TipoToken::KEYS) || comprobar(TipoToken::VALUES) ||
                comprobar(TipoToken::TYPE_FN) || comprobar(TipoToken::RANGE) ||
                comprobar(TipoToken::TRANSPOSE) || comprobar(TipoToken::DOT) ||
                comprobar(TipoToken::HEAD) || comprobar(TipoToken::SELECT) ||
                comprobar(TipoToken::SPLIT) || comprobar(TipoToken::JOIN) ||
                comprobar(TipoToken::REPLACE) || comprobar(TipoToken::FIND) ||
                comprobar(TipoToken::MAP_FN) || comprobar(TipoToken::FILTER_FN) ||
                comprobar(TipoToken::REDUCE) || comprobar(TipoToken::IS_NULL) ||
                comprobar(TipoToken::IS_ERROR) || comprobar(TipoToken::PRINT) ||
                comprobar(TipoToken::LOAD_CSV) || comprobar(TipoToken::PLOT) ||
                comprobar(TipoToken::SERVE) || comprobar(TipoToken::METRIC) ||
                comprobar(TipoToken::DASHBOARD) || comprobar(TipoToken::ROUTE) ||
                comprobar(TipoToken::CLASS) || comprobar(TipoToken::NEW) ||
                comprobar(TipoToken::THIS) || comprobar(TipoToken::EXTENDS)) {
                nombre_campo += consumir().lexema;
            } else {
                throw std::runtime_error("Se esperaba nombre de campo despues de '$'");
            }
            consumir(TipoToken::ASIGNAR);
            auto valor = expresion();
            // Wrap the value in an ASIGNACION node so the interpreter can call
            // evaluar_asignacion() on it, which expects hijos[0] = value expr.
            // Convention: field names stored WITHOUT the $ prefix to match
            // the variable-lookup convention used elsewhere.
            auto asign = std::make_unique<NodoAST>();
            asign->tipo = TipoNodoAST::ASIGNACION;
            std::string bare_name = nombre_campo;
            if (!bare_name.empty() && bare_name[0] == '$') bare_name = bare_name.substr(1);
            asign->valor_texto = bare_name;
            asign->hijos.push_back(std::move(valor));
            nodo->hijos.push_back(std::move(asign));
        } else if (comprobar(TipoToken::FN)) {
            // Method declaration
            consumir(TipoToken::FN);
            if (!comprobar(TipoToken::IDENTIFICADOR)) {
                throw std::runtime_error("Se esperaba nombre de metodo despues de 'fn'");
            }
            std::string nombre_metodo = consumir(TipoToken::IDENTIFICADOR).lexema;
            consumir(TipoToken::PARENTESIS_ABRE);
            std::vector<std::string> params;
            if (!comprobar(TipoToken::PARENTESIS_CIERRA)) {
                do {
                    if (comprobar(TipoToken::SIGIL)) {
                        consumir(TipoToken::SIGIL);
                        if (!comprobar(TipoToken::IDENTIFICADOR)) {
                            throw std::runtime_error("Se esperaba nombre de parametro");
                        }
                        // Convention: param names stored WITHOUT the $ prefix.
                        params.push_back(consumir(TipoToken::IDENTIFICADOR).lexema);
                    } else {
                        params.push_back(consumir().lexema);
                    }
                } while (comprobar(TipoToken::COMA) && (consumir(TipoToken::COMA), true));
            }
            consumir(TipoToken::PARENTESIS_CIERRA);
            auto cuerpo = bloque();
            auto metodo_nodo = std::make_unique<NodoAST>();
            metodo_nodo->tipo = TipoNodoAST::DECLARACION_FN;
            metodo_nodo->valor_texto = nombre_metodo;
            metodo_nodo->parametros = params;
            metodo_nodo->hijos.push_back(std::move(cuerpo));
            nodo->hijos.push_back(std::move(metodo_nodo));
        } else {
            throw std::runtime_error("Miembro de clase invalido (esperaba $campo o fn metodo)");
        }
    }
    consumir(TipoToken::DICCIONARIO_CIERRA);

    return nodo;
}

std::unique_ptr<NodoAST> Parser::bloque() {
    consumir(TipoToken::DICCIONARIO_ABRE);

    auto nodo = std::make_unique<NodoAST>();
    nodo->tipo = TipoNodoAST::BLOQUE;

    while (!comprobar(TipoToken::DICCIONARIO_CIERRA) && !esta_al_final()) {
        nodo->hijos.push_back(declaracion());
    }

    consumir(TipoToken::DICCIONARIO_CIERRA);
    return nodo;
}

std::unique_ptr<NodoAST> Parser::expresion() {
    return expresion_ternaria();
}

std::unique_ptr<NodoAST> Parser::expresion_ternaria() {
    auto cond = expresion_or();
    if (comprobar(TipoToken::TERNARIO_PREG)) {
        consumir(TipoToken::TERNARIO_PREG);
        auto v = expresion();
        consumir(TipoToken::TERNARIO_SINO);
        auto f = expresion();

        auto nodo = std::make_unique<NodoAST>();
        nodo->tipo = TipoNodoAST::TERNARIA;
        nodo->hijos.push_back(std::move(cond));
        nodo->hijos.push_back(std::move(v));
        nodo->hijos.push_back(std::move(f));
        return nodo;
    }
    return cond;
}

std::unique_ptr<NodoAST> Parser::expresion_or() {
    auto izq = expresion_and();
    while (comprobar(TipoToken::OR)) {
        consumir(TipoToken::OR);
        auto der = expresion_and();
        auto nodo = std::make_unique<NodoAST>();
        nodo->tipo = TipoNodoAST::BINARIA;
        nodo->valor_texto = "||";
        nodo->hijos.push_back(std::move(izq));
        nodo->hijos.push_back(std::move(der));
        izq = std::move(nodo);
    }
    return izq;
}

std::unique_ptr<NodoAST> Parser::expresion_and() {
    auto izq = expresion_igualdad();
    while (comprobar(TipoToken::AND)) {
        consumir(TipoToken::AND);
        auto der = expresion_igualdad();
        auto nodo = std::make_unique<NodoAST>();
        nodo->tipo = TipoNodoAST::BINARIA;
        nodo->valor_texto = "&&";
        nodo->hijos.push_back(std::move(izq));
        nodo->hijos.push_back(std::move(der));
        izq = std::move(nodo);
    }
    return izq;
}

std::unique_ptr<NodoAST> Parser::expresion_igualdad() {
    auto izq = expresion_comparacion();
    while (comprobar(TipoToken::IGUAL) || comprobar(TipoToken::DIFERENTE)) {
        auto op = consumir();
        auto der = expresion_comparacion();
        auto nodo = std::make_unique<NodoAST>();
        nodo->tipo = TipoNodoAST::BINARIA;
        nodo->valor_texto = op.lexema;
        nodo->hijos.push_back(std::move(izq));
        nodo->hijos.push_back(std::move(der));
        izq = std::move(nodo);
    }
    return izq;
}

std::unique_ptr<NodoAST> Parser::expresion_comparacion() {
    auto izq = expresion_termino();
    while (comprobar(TipoToken::MAYOR_QUE) || comprobar(TipoToken::MENOR_QUE) ||
           comprobar(TipoToken::MAYOR_IGUAL) || comprobar(TipoToken::MENOR_IGUAL)) {
        auto op = consumir();
        auto der = expresion_termino();
        auto nodo = std::make_unique<NodoAST>();
        nodo->tipo = TipoNodoAST::BINARIA;
        nodo->valor_texto = op.lexema;
        nodo->hijos.push_back(std::move(izq));
        nodo->hijos.push_back(std::move(der));
        izq = std::move(nodo);
    }
    return izq;
}

std::unique_ptr<NodoAST> Parser::expresion_termino() {
    auto izq = expresion_factor();
    while (comprobar(TipoToken::SUMA) || comprobar(TipoToken::RESTA)) {
        auto op = consumir();
        auto der = expresion_factor();
        auto nodo = std::make_unique<NodoAST>();
        nodo->tipo = TipoNodoAST::BINARIA;
        nodo->valor_texto = op.lexema;
        nodo->hijos.push_back(std::move(izq));
        nodo->hijos.push_back(std::move(der));
        izq = std::move(nodo);
    }
    return izq;
}

std::unique_ptr<NodoAST> Parser::expresion_factor() {
    auto izq = expresion_unaria();
    while (comprobar(TipoToken::MULTIPLICACION) || comprobar(TipoToken::DIVISION) || comprobar(TipoToken::MODULO)) {
        auto op = consumir();
        auto der = expresion_unaria();
        auto nodo = std::make_unique<NodoAST>();
        nodo->tipo = TipoNodoAST::BINARIA;
        nodo->valor_texto = op.lexema;
        nodo->hijos.push_back(std::move(izq));
        nodo->hijos.push_back(std::move(der));
        izq = std::move(nodo);
    }
    return izq;
}

std::unique_ptr<NodoAST> Parser::expresion_unaria() {
    if (comprobar(TipoToken::NOT) || comprobar(TipoToken::RESTA)) {
        auto op = consumir();
        auto operando = expresion_unaria();
        auto nodo = std::make_unique<NodoAST>();
        nodo->tipo = TipoNodoAST::UNARIA;
        nodo->valor_texto = op.lexema;
        nodo->hijos.push_back(std::move(operando));
        return nodo;
    }
    auto izq = expresion_primaria();
    // Method call postfix: $obj.metodo(args) or field access: $obj.campo
    while (comprobar(TipoToken::PUNTO)) {
        consumir(TipoToken::PUNTO);
        // Accept any identifier-or-keyword after dot
        if (!(comprobar(TipoToken::IDENTIFICADOR) ||
              comprobar(TipoToken::COUNT) || comprobar(TipoToken::MEAN) ||
              comprobar(TipoToken::SUM_FN) || comprobar(TipoToken::MAX_FN) ||
              comprobar(TipoToken::MIN_FN) || comprobar(TipoToken::STDDEV) ||
              comprobar(TipoToken::ABS) || comprobar(TipoToken::ROUND) ||
              comprobar(TipoToken::FLOOR) || comprobar(TipoToken::CEIL) ||
              comprobar(TipoToken::POW) || comprobar(TipoToken::SQRT) ||
              comprobar(TipoToken::LEN) || comprobar(TipoToken::UPPER) ||
              comprobar(TipoToken::LOWER) || comprobar(TipoToken::SUBSTR) ||
              comprobar(TipoToken::REVERSE) || comprobar(TipoToken::SORT_FN) ||
              comprobar(TipoToken::UNIQUE) || comprobar(TipoToken::PUSH) ||
              comprobar(TipoToken::KEYS) || comprobar(TipoToken::VALUES) ||
              comprobar(TipoToken::TYPE_FN) || comprobar(TipoToken::RANGE) ||
              comprobar(TipoToken::TRANSPOSE) || comprobar(TipoToken::DOT) ||
              comprobar(TipoToken::HEAD) || comprobar(TipoToken::SELECT) ||
              comprobar(TipoToken::SPLIT) || comprobar(TipoToken::JOIN) ||
              comprobar(TipoToken::REPLACE) || comprobar(TipoToken::FIND) ||
              comprobar(TipoToken::MAP_FN) || comprobar(TipoToken::FILTER_FN) ||
              comprobar(TipoToken::REDUCE) || comprobar(TipoToken::IS_NULL) ||
              comprobar(TipoToken::IS_ERROR) || comprobar(TipoToken::PRINT) ||
              comprobar(TipoToken::LOAD_CSV) || comprobar(TipoToken::PLOT) ||
              comprobar(TipoToken::SERVE) || comprobar(TipoToken::METRIC) ||
              comprobar(TipoToken::DASHBOARD) || comprobar(TipoToken::ROUTE))) {
            throw std::runtime_error("Se esperaba nombre de metodo o campo despues de '.'");
        }
        std::string nombre = consumir().lexema;
        // If followed by '(', it's a method call; otherwise field access
        if (comprobar(TipoToken::PARENTESIS_ABRE)) {
            auto nodo = std::make_unique<NodoAST>();
            nodo->tipo = TipoNodoAST::LLAMADA_METODO;
            nodo->valor_texto = nombre;
            nodo->hijos.push_back(std::move(izq));
            consumir(TipoToken::PARENTESIS_ABRE);
            if (!comprobar(TipoToken::PARENTESIS_CIERRA)) {
                do {
                    nodo->hijos.push_back(expresion());
                } while (comprobar(TipoToken::COMA) && (consumir(TipoToken::COMA), true));
            }
            consumir(TipoToken::PARENTESIS_CIERRA);
            izq = std::move(nodo);
        } else {
            auto nodo = std::make_unique<NodoAST>();
            nodo->tipo = TipoNodoAST::ACCESO_METODO;
            nodo->valor_texto = nombre;
            nodo->hijos.push_back(std::move(izq));
            izq = std::move(nodo);
        }
    }
    return izq;
}

std::unique_ptr<NodoAST> Parser::expresion_primaria() {
    if (comprobar(TipoToken::NUMERO)) {
        auto t = consumir(TipoToken::NUMERO);
        auto nodo = std::make_unique<NodoAST>();
        nodo->tipo = TipoNodoAST::LITERAL_NUMERO;
        nodo->valor_numerico = t.valor_numero;
        nodo->linea = t.linea;
        return nodo;
    }

    if (comprobar(TipoToken::CADENA)) {
        auto t = consumir(TipoToken::CADENA);
        auto nodo = std::make_unique<NodoAST>();
        nodo->tipo = TipoNodoAST::LITERAL_CADENA;
        nodo->valor_texto = t.valor_cadena;
        nodo->linea = t.linea;
        return nodo;
    }

    if (comprobar(TipoToken::TRUE_LITERAL)) {
        auto t = consumir(TipoToken::TRUE_LITERAL);
        auto nodo = std::make_unique<NodoAST>();
        nodo->tipo = TipoNodoAST::LITERAL_BOOL;
        nodo->valor_bool = true;
        nodo->linea = t.linea;
        return nodo;
    }

    if (comprobar(TipoToken::FALSE_LITERAL)) {
        auto t = consumir(TipoToken::FALSE_LITERAL);
        auto nodo = std::make_unique<NodoAST>();
        nodo->tipo = TipoNodoAST::LITERAL_BOOL;
        nodo->valor_bool = false;
        nodo->linea = t.linea;
        return nodo;
    }

    if (comprobar(TipoToken::NULL_LITERAL)) {
        auto t = consumir(TipoToken::NULL_LITERAL);
        auto nodo = std::make_unique<NodoAST>();
        nodo->tipo = TipoNodoAST::LITERAL_NULL;
        nodo->linea = t.linea;
        return nodo;
    }

    if (comprobar(TipoToken::VECTOR_ABRE)) return parsear_vector();
    if (comprobar(TipoToken::MATRIZ_ABRE)) return parsear_matriz();
    if (comprobar(TipoToken::DICCIONARIO_ABRE)) return parsear_diccionario();

    if (comprobar(TipoToken::FN) && peek_siguiente_es_lambada()) {
        return parsear_lambda_anonima();
    }

    if (comprobar(TipoToken::SIGIL)) return parsear_variable_o_llamada();

    if (comprobar(TipoToken::NEW)) {
        consumir(TipoToken::NEW);
        auto nodo = std::make_unique<NodoAST>();
        nodo->tipo = TipoNodoAST::NEW;
        if (!comprobar(TipoToken::IDENTIFICADOR)) {
            throw std::runtime_error("Se esperaba nombre de clase despues de 'new'");
        }
        nodo->valor_texto = consumir(TipoToken::IDENTIFICADOR).lexema;
        consumir(TipoToken::PARENTESIS_ABRE);
        if (!comprobar(TipoToken::PARENTESIS_CIERRA)) {
            do {
                nodo->hijos.push_back(expresion());
            } while (comprobar(TipoToken::COMA) && (consumir(TipoToken::COMA), true));
        }
        consumir(TipoToken::PARENTESIS_CIERRA);
        return nodo;
    }

    if (comprobar(TipoToken::THIS)) {
        consumir(TipoToken::THIS);
        auto nodo = std::make_unique<NodoAST>();
        nodo->tipo = TipoNodoAST::THIS;
        return nodo;
    }

    if (comprobar(TipoToken::IS_NULL) || comprobar(TipoToken::IS_ERROR) ||
        comprobar(TipoToken::MEAN) || comprobar(TipoToken::COUNT) ||
        comprobar(TipoToken::SUM_FN) || comprobar(TipoToken::MIN_FN) ||
        comprobar(TipoToken::MAX_FN) || comprobar(TipoToken::STDDEV) ||
        comprobar(TipoToken::PLOT) || comprobar(TipoToken::METRIC) ||
        comprobar(TipoToken::LOAD_CSV) ||
        comprobar(TipoToken::SERVE) || comprobar(TipoToken::DASHBOARD) ||
        comprobar(TipoToken::ABS) || comprobar(TipoToken::ROUND) ||
        comprobar(TipoToken::FLOOR) || comprobar(TipoToken::CEIL) ||
        comprobar(TipoToken::POW) || comprobar(TipoToken::SQRT) ||
        comprobar(TipoToken::LEN) || comprobar(TipoToken::UPPER) ||
        comprobar(TipoToken::LOWER) || comprobar(TipoToken::SUBSTR) ||
        comprobar(TipoToken::REVERSE) || comprobar(TipoToken::SORT_FN) ||
        comprobar(TipoToken::UNIQUE) || comprobar(TipoToken::PUSH) ||
        comprobar(TipoToken::KEYS) || comprobar(TipoToken::VALUES) ||
        comprobar(TipoToken::TYPE_FN) || comprobar(TipoToken::RANGE) ||
        comprobar(TipoToken::TRANSPOSE) || comprobar(TipoToken::DOT) ||
        comprobar(TipoToken::HEAD) || comprobar(TipoToken::SELECT) ||
        comprobar(TipoToken::SPLIT) || comprobar(TipoToken::JOIN) ||
        comprobar(TipoToken::REPLACE) || comprobar(TipoToken::FIND) ||
        comprobar(TipoToken::MAP_FN) || comprobar(TipoToken::FILTER_FN) ||
        comprobar(TipoToken::REDUCE) || comprobar(TipoToken::ROUTE)) {
        return parsear_llamada_nativa(actual().tipo);
    }

    if (comprobar(TipoToken::IDENTIFICADOR)) {
        auto t = consumir(TipoToken::IDENTIFICADOR);
        if (comprobar(TipoToken::PUNTO_DOBLE)) {
            consumir(TipoToken::PUNTO_DOBLE);
            std::string miembro_nombre;
            if (comprobar(TipoToken::IDENTIFICADOR)) {
                miembro_nombre = consumir(TipoToken::IDENTIFICADOR).lexema;
            } else {
                auto tk = consumir();
                miembro_nombre = tk.lexema;
            }
            auto acc = std::make_unique<NodoAST>();
            acc->tipo = TipoNodoAST::ACCESO_NAMESPACE;
            acc->valor_texto = miembro_nombre;
            auto ns = std::make_unique<NodoAST>();
            ns->tipo = TipoNodoAST::VARIABLE;
            ns->valor_texto = t.lexema;
            acc->hijos.push_back(std::move(ns));
            if (comprobar(TipoToken::PARENTESIS_ABRE)) {
                consumir(TipoToken::PARENTESIS_ABRE);
                auto nodo = std::make_unique<NodoAST>();
                nodo->tipo = TipoNodoAST::LLAMADA_NAMESPACE;
                nodo->nombre_funcion = miembro_nombre;
                nodo->hijos.push_back(std::move(acc));
                if (!comprobar(TipoToken::PARENTESIS_CIERRA)) {
                    do {
                        nodo->hijos.push_back(expresion());
                    } while (comprobar(TipoToken::COMA) && (consumir(TipoToken::COMA), true));
                }
                consumir(TipoToken::PARENTESIS_CIERRA);
                return nodo;
            }
            return acc;
        }
        if (comprobar(TipoToken::PARENTESIS_ABRE)) {
            return parsear_llamada_funcion(t.lexema);
        }
        auto nodo = std::make_unique<NodoAST>();
        nodo->tipo = TipoNodoAST::VARIABLE;
        nodo->valor_texto = t.lexema;
        return nodo;
    }

    if (comprobar(TipoToken::PARENTESIS_ABRE)) {
        consumir(TipoToken::PARENTESIS_ABRE);
        auto expr = expresion();
        consumir(TipoToken::PARENTESIS_CIERRA);
        return expr;
    }

    throw std::runtime_error("Expresion inesperada en linea " + std::to_string(actual().linea));
}

std::unique_ptr<NodoAST> Parser::parsear_vector() {
    consumir(TipoToken::VECTOR_ABRE);

    auto nodo = std::make_unique<NodoAST>();
    nodo->tipo = TipoNodoAST::VECTOR;

    if (!comprobar(TipoToken::VECTOR_CIERRA)) {
        do {
            nodo->hijos.push_back(expresion());
        } while (comprobar(TipoToken::COMA) && (consumir(TipoToken::COMA), true));
    }
    consumir(TipoToken::VECTOR_CIERRA);
    return nodo;
}

std::unique_ptr<NodoAST> Parser::parsear_matriz() {
    consumir(TipoToken::MATRIZ_ABRE);

    auto nodo = std::make_unique<NodoAST>();
    nodo->tipo = TipoNodoAST::MATRIZ;

    if (!comprobar(TipoToken::MATRIZ_CIERRA)) {
        do {
            // Cada fila es un vector - el primer < puede estar implicito despues de <<
            bool necesita_abre = !comprobar(TipoToken::VECTOR_ABRE);
            if (comprobar(TipoToken::VECTOR_ABRE)) {
                consumir(TipoToken::VECTOR_ABRE);
            }

            auto fila = std::make_unique<NodoAST>();
            fila->tipo = TipoNodoAST::VECTOR;
            if (!comprobar(TipoToken::VECTOR_CIERRA)) {
                do {
                    fila->hijos.push_back(expresion());
                } while (comprobar(TipoToken::COMA) && (consumir(TipoToken::COMA), true));
            }
            if (comprobar(TipoToken::VECTOR_CIERRA)) {
                consumir(TipoToken::VECTOR_CIERRA);
            }
            nodo->hijos.push_back(std::move(fila));

            // Si consumimos implicitamente el primer <, no esperamos VECTOR_ABRE para siguientes filas
            if (necesita_abre && comprobar(TipoToken::COMA)) {
                consumir(TipoToken::COMA);
                // Las filas siguientes SI necesitan <
                if (comprobar(TipoToken::VECTOR_ABRE)) {
                    consumir(TipoToken::VECTOR_ABRE);
                }
            }
        } while (!comprobar(TipoToken::MATRIZ_CIERRA) && !esta_al_final());
    }
    consumir(TipoToken::MATRIZ_CIERRA);
    return nodo;
}

std::unique_ptr<NodoAST> Parser::parsear_diccionario() {
    consumir(TipoToken::DICCIONARIO_ABRE);

    auto nodo = std::make_unique<NodoAST>();
    nodo->tipo = TipoNodoAST::DICCIONARIO;

    if (!comprobar(TipoToken::DICCIONARIO_CIERRA)) {
        do {
            auto clave = consumir(TipoToken::CADENA);
            consumir(TipoToken::TERNARIO_SINO);
            auto valor = expresion();

            auto par = std::make_unique<NodoAST>();
            par->tipo = TipoNodoAST::ASIGNACION;
            par->valor_texto = clave.valor_cadena;
            par->hijos.push_back(std::move(valor));
            nodo->hijos.push_back(std::move(par));
        } while (comprobar(TipoToken::COMA) && (consumir(TipoToken::COMA), true));
    }
    consumir(TipoToken::DICCIONARIO_CIERRA);
    return nodo;
}

std::unique_ptr<NodoAST> Parser::parsear_variable_o_llamada() {
    consumir(TipoToken::SIGIL);
    auto nombre = consumir(TipoToken::IDENTIFICADOR);

    auto nodo = std::make_unique<NodoAST>();
    nodo->tipo = TipoNodoAST::VARIABLE;
    nodo->valor_texto = nombre.lexema;

    while (comprobar(TipoToken::EXTRACTOR)) {
        consumir(TipoToken::EXTRACTOR);
        auto col = consumir(TipoToken::IDENTIFICADOR);

        auto acceso = std::make_unique<NodoAST>();
        acceso->tipo = TipoNodoAST::ACCESO_COLUMNAS;
        acceso->valor_texto = col.lexema;
        acceso->hijos.push_back(std::move(nodo));
        nodo = std::move(acceso);
    }

    if (comprobar(TipoToken::PARENTESIS_ABRE)) {
        consumir(TipoToken::PARENTESIS_ABRE);
        auto indice = expresion();
        consumir(TipoToken::PARENTESIS_CIERRA);

        auto acceso = std::make_unique<NodoAST>();
        acceso->tipo = TipoNodoAST::ACCESO_INDICE;
        acceso->hijos.push_back(std::move(nodo));
        acceso->hijos.push_back(std::move(indice));
        nodo = std::move(acceso);
    }

    if (comprobar(TipoToken::CORCHETE_ABRE)) {
        consumir(TipoToken::CORCHETE_ABRE);
        auto fila = expresion();
        consumir(TipoToken::COMA);
        auto columna = expresion();
        consumir(TipoToken::CORCHETE_CIERRA);

        auto acceso = std::make_unique<NodoAST>();
        acceso->tipo = TipoNodoAST::ACCESO_MATRIZ;
        acceso->hijos.push_back(std::move(nodo));
        acceso->hijos.push_back(std::move(fila));
        acceso->hijos.push_back(std::move(columna));
        nodo = std::move(acceso);
    }

    if (comprobar(TipoToken::FILTRO_ABRE)) {
        consumir(TipoToken::FILTRO_ABRE);
        auto condicion = expresion();
        consumir(TipoToken::FILTRO_CIERRA);

        auto filtro = std::make_unique<NodoAST>();
        filtro->tipo = TipoNodoAST::FILTRO_FILAS;
        filtro->hijos.push_back(std::move(nodo));
        filtro->hijos.push_back(std::move(condicion));
        nodo = std::move(filtro);
    }

    return nodo;
}

std::unique_ptr<NodoAST> Parser::parsear_llamada_funcion(const std::string& nombre) {
    consumir(TipoToken::PARENTESIS_ABRE);

    auto nodo = std::make_unique<NodoAST>();
    nodo->tipo = TipoNodoAST::LLAMADA_FUNCION;
    nodo->nombre_funcion = nombre;

    if (!comprobar(TipoToken::PARENTESIS_CIERRA)) {
        do {
            nodo->hijos.push_back(expresion());
        } while (comprobar(TipoToken::COMA) && (consumir(TipoToken::COMA), true));
    }
    consumir(TipoToken::PARENTESIS_CIERRA);
    return nodo;
}

std::unique_ptr<NodoAST> Parser::parsear_llamada_nativa(TipoToken tipo) {
    auto t = consumir();

    auto nodo = std::make_unique<NodoAST>();
    nodo->nombre_funcion = t.lexema;
    nodo->linea = t.linea;

    switch (tipo) {
        case TipoToken::IS_NULL: nodo->tipo = TipoNodoAST::IS_NULL; break;
        case TipoToken::IS_ERROR: nodo->tipo = TipoNodoAST::IS_ERROR; break;
        case TipoToken::MEAN: nodo->tipo = TipoNodoAST::MEAN; break;
        case TipoToken::COUNT: nodo->tipo = TipoNodoAST::COUNT; break;
        case TipoToken::PLOT: nodo->tipo = TipoNodoAST::PLOT; break;
        case TipoToken::METRIC: nodo->tipo = TipoNodoAST::METRIC; break;
        case TipoToken::LOAD_CSV: nodo->tipo = TipoNodoAST::LOAD_CSV; break;
        case TipoToken::SERVE: nodo->tipo = TipoNodoAST::SERVE; break;
        case TipoToken::ROUTE: nodo->tipo = TipoNodoAST::REGISTRAR_RUTA; break;
        case TipoToken::DASHBOARD: nodo->tipo = TipoNodoAST::DASHBOARD; break;
        case TipoToken::SUM_FN: nodo->tipo = TipoNodoAST::SUM_FN; break;
        case TipoToken::MIN_FN: nodo->tipo = TipoNodoAST::MIN_FN; break;
        case TipoToken::MAX_FN: nodo->tipo = TipoNodoAST::MAX_FN; break;
        case TipoToken::STDDEV: nodo->tipo = TipoNodoAST::STDDEV; break;
        case TipoToken::ABS: nodo->tipo = TipoNodoAST::ABS; break;
        case TipoToken::ROUND: nodo->tipo = TipoNodoAST::ROUND; break;
        case TipoToken::FLOOR: nodo->tipo = TipoNodoAST::FLOOR; break;
        case TipoToken::CEIL: nodo->tipo = TipoNodoAST::CEIL; break;
        case TipoToken::POW: nodo->tipo = TipoNodoAST::POW; break;
        case TipoToken::SQRT: nodo->tipo = TipoNodoAST::SQRT; break;
        case TipoToken::LEN: nodo->tipo = TipoNodoAST::LEN; break;
        case TipoToken::UPPER: nodo->tipo = TipoNodoAST::UPPER; break;
        case TipoToken::LOWER: nodo->tipo = TipoNodoAST::LOWER; break;
        case TipoToken::SUBSTR: nodo->tipo = TipoNodoAST::SUBSTR; break;
        case TipoToken::REVERSE: nodo->tipo = TipoNodoAST::REVERSE; break;
        case TipoToken::SORT_FN: nodo->tipo = TipoNodoAST::SORT_FN; break;
        case TipoToken::UNIQUE: nodo->tipo = TipoNodoAST::UNIQUE; break;
        case TipoToken::PUSH: nodo->tipo = TipoNodoAST::PUSH; break;
        case TipoToken::KEYS: nodo->tipo = TipoNodoAST::KEYS; break;
        case TipoToken::VALUES: nodo->tipo = TipoNodoAST::VALUES; break;
        case TipoToken::TYPE_FN: nodo->tipo = TipoNodoAST::TYPE_FN; break;
        case TipoToken::RANGE: nodo->tipo = TipoNodoAST::RANGE; break;
        case TipoToken::TRANSPOSE: nodo->tipo = TipoNodoAST::TRANSPOSE; break;
        case TipoToken::DOT: nodo->tipo = TipoNodoAST::DOT; break;
        case TipoToken::HEAD: nodo->tipo = TipoNodoAST::HEAD; break;
        case TipoToken::SELECT: nodo->tipo = TipoNodoAST::SELECT; break;
        case TipoToken::SPLIT: nodo->tipo = TipoNodoAST::SPLIT; break;
        case TipoToken::JOIN: nodo->tipo = TipoNodoAST::JOIN; break;
        case TipoToken::REPLACE: nodo->tipo = TipoNodoAST::REPLACE; break;
        case TipoToken::FIND: nodo->tipo = TipoNodoAST::FIND; break;
        case TipoToken::MAP_FN: nodo->tipo = TipoNodoAST::MAP_FN; break;
        case TipoToken::FILTER_FN: nodo->tipo = TipoNodoAST::FILTER_FN; break;
        case TipoToken::REDUCE: nodo->tipo = TipoNodoAST::REDUCE; break;
        default: break;
    }

    consumir(TipoToken::PARENTESIS_ABRE);
    if (!comprobar(TipoToken::PARENTESIS_CIERRA)) {
        do {
            nodo->hijos.push_back(expresion());
        } while (comprobar(TipoToken::COMA) && (consumir(TipoToken::COMA), true));
    }
    consumir(TipoToken::PARENTESIS_CIERRA);
    return nodo;
}

bool Parser::peek_siguiente_es_lambada() {
    if (posicion_ + 1 >= tokens_.size()) return false;
    return tokens_[posicion_ + 1].tipo == TipoToken::PARENTESIS_ABRE;
}

std::unique_ptr<NodoAST> Parser::parsear_lambda_anonima() {
    consumir(TipoToken::FN);
    consumir(TipoToken::PARENTESIS_ABRE);

    std::vector<std::string> params;
    if (!comprobar(TipoToken::PARENTESIS_CIERRA)) {
        do {
            consumir(TipoToken::SIGIL);
            params.push_back(consumir(TipoToken::IDENTIFICADOR).lexema);
        } while (comprobar(TipoToken::COMA) && (consumir(TipoToken::COMA), true));
    }
    consumir(TipoToken::PARENTESIS_CIERRA);

    auto cuerpo = bloque();

    auto nodo = std::make_unique<NodoAST>();
    nodo->tipo = TipoNodoAST::FUNCION_ANONIMA;
    nodo->valor_texto = "<lambda>";
    nodo->parametros = params;
    nodo->hijos.push_back(std::move(cuerpo));
    nodo->linea = 0;
    return nodo;
}

}
