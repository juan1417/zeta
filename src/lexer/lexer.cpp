#include "zeta/lexer.hpp"
#include <stdexcept>
#include <map>
#include <cctype>

namespace zeta {

Lexer::Lexer(std::string fuente)
    : fuente_(std::move(fuente)), posicion_(0), linea_(1), columna_(1), esperando_valor_(false), profundidad_coleccion_(0) {}

char Lexer::avanzar() {
    if (posicion_ >= fuente_.size()) return '\0';
    char c = fuente_[posicion_++];
    if (c == '\n') { linea_++; columna_ = 1; }
    else { columna_++; }
    return c;
}

char Lexer::peek() {
    if (posicion_ >= fuente_.size()) return '\0';
    return fuente_[posicion_];
}

bool Lexer::esta_al_final() { return posicion_ >= fuente_.size(); }
bool Lexer::es_alpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
bool Lexer::es_digito(char c) { return c >= '0' && c <= '9'; }

void Lexer::omitir_espacio_blanco() {
    while (!esta_al_final()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') { avanzar(); }
        else if (c == '#') { while (!esta_al_final() && peek() != '\n') avanzar(); }
        else break;
    }
}

Token Lexer::tokenizar_identificador() {
    size_t inicio = posicion_ - 1;
    while (!esta_al_final() && (es_alpha(peek()) || es_digito(peek()) || peek() == '_')) avanzar();
    std::string texto = fuente_.substr(inicio, posicion_ - inicio);
    static const std::map<std::string, TipoToken> reservadas = {
        {"fn", TipoToken::FN}, {"if", TipoToken::IF}, {"else", TipoToken::ELSE},
        {"for", TipoToken::FOR}, {"while", TipoToken::WHILE}, {"in", TipoToken::IN},
        {"return", TipoToken::RETURN}, {"print", TipoToken::PRINT},
        {"break", TipoToken::BREAK}, {"continue", TipoToken::CONTINUE},
        {"include", TipoToken::INCLUDE}, {"as", TipoToken::AS}, {"export", TipoToken::EXPORT},
        {"true", TipoToken::TRUE_LITERAL}, {"false", TipoToken::FALSE_LITERAL},
        {"null", TipoToken::NULL_LITERAL},
        {"load_csv", TipoToken::LOAD_CSV},
        {"plot", TipoToken::PLOT}, {"serve", TipoToken::SERVE}, {"metric", TipoToken::METRIC},
        {"dashboard", TipoToken::DASHBOARD},
        {"is_null", TipoToken::IS_NULL}, {"is_error", TipoToken::IS_ERROR},
        {"mean", TipoToken::MEAN}, {"count", TipoToken::COUNT},
        {"sum", TipoToken::SUM_FN}, {"min", TipoToken::MIN_FN},
        {"max", TipoToken::MAX_FN}, {"stddev", TipoToken::STDDEV},
        {"abs", TipoToken::ABS}, {"round", TipoToken::ROUND},
        {"floor", TipoToken::FLOOR}, {"ceil", TipoToken::CEIL},
        {"pow", TipoToken::POW}, {"sqrt", TipoToken::SQRT},
        {"len", TipoToken::LEN}, {"upper", TipoToken::UPPER},
        {"lower", TipoToken::LOWER}, {"substr", TipoToken::SUBSTR},
        {"reverse", TipoToken::REVERSE}, {"sort", TipoToken::SORT_FN},
        {"unique", TipoToken::UNIQUE}, {"push", TipoToken::PUSH},
        {"keys", TipoToken::KEYS}, {"values", TipoToken::VALUES},
        {"type", TipoToken::TYPE_FN}, {"range", TipoToken::RANGE},
        {"transpose", TipoToken::TRANSPOSE}, {"dot", TipoToken::DOT},
        {"head", TipoToken::HEAD}, {"select", TipoToken::SELECT},
        {"split", TipoToken::SPLIT}, {"join", TipoToken::JOIN},
        {"replace", TipoToken::REPLACE}, {"find", TipoToken::FIND},
        {"map", TipoToken::MAP_FN}, {"filter", TipoToken::FILTER_FN},
        {"reduce", TipoToken::REDUCE},
        {"route", TipoToken::ROUTE},
        {"class", TipoToken::CLASS}, {"new", TipoToken::NEW},
        {"this", TipoToken::THIS}, {"extends", TipoToken::EXTENDS},
    };
    auto it = reservadas.find(texto);
    esperando_valor_ = false;
    if (it != reservadas.end()) return Token(it->second, texto, linea_, columna_ - static_cast<int>(texto.length()));
    return Token(TipoToken::IDENTIFICADOR, texto, linea_, columna_ - static_cast<int>(texto.length()));
}

Token Lexer::tokenizar_numero() {
    size_t inicio = posicion_ - 1;
    bool punto_encontrado = false;
    while (!esta_al_final() && (es_digito(peek()) || peek() == '.')) {
        if (peek() == '.') {
            if (punto_encontrado) break;
            punto_encontrado = true;
        }
        avanzar();
    }
    std::string texto = fuente_.substr(inicio, posicion_ - inicio);
    Token token(TipoToken::NUMERO, texto, linea_, columna_ - static_cast<int>(texto.length()));
    try {
        token.valor_numero = std::stod(texto);
    } catch (...) {
        throw std::runtime_error("Numero invalido: '" + texto + "' en linea " + std::to_string(linea_));
    }
    esperando_valor_ = false;
    return token;
}

Token Lexer::tokenizar_cadena() {
    size_t inicio = posicion_;
    std::string texto;
    while (!esta_al_final() && peek() != '"') {
        char c = avanzar();
        if (c == '\\') {
            char sig = peek();
            if (sig == 'n') { avanzar(); texto += '\n'; }
            else if (sig == 't') { avanzar(); texto += '\t'; }
            else if (sig == '"') { avanzar(); texto += '"'; }
            else if (sig == '\\') { avanzar(); texto += '\\'; }
            else if (sig == 'r') { avanzar(); texto += '\r'; }
            else { texto += c; texto += sig; avanzar(); }
        } else {
            texto += c;
        }
    }
    if (esta_al_final()) throw std::runtime_error("Cadena sin cerrar en linea " + std::to_string(linea_));
    avanzar();
    Token token(TipoToken::CADENA, texto, linea_, columna_ - static_cast<int>(texto.length()) - 2);
    token.valor_cadena = texto;
    esperando_valor_ = false;
    return token;
}

Token Lexer::siguiente_token() {
    omitir_espacio_blanco();
    if (esta_al_final()) return Token(TipoToken::EOF_TOKEN, "", linea_, columna_);
    char c = avanzar();
    if (c == '$') return Token(TipoToken::SIGIL, "$", linea_, columna_ - 1);
    if (es_alpha(c) || c == '_') return tokenizar_identificador();
    if (es_digito(c)) return tokenizar_numero();
    if (c == '"') return tokenizar_cadena();
    switch (c) {
        case '+': return Token(TipoToken::SUMA, "+", linea_, columna_ - 1);
        case '-': return Token(TipoToken::RESTA, "-", linea_, columna_ - 1);
        case '*': return Token(TipoToken::MULTIPLICACION, "*", linea_, columna_ - 1);
        case '/': return Token(TipoToken::DIVISION, "/", linea_, columna_ - 1);
        case '(': esperando_valor_ = true; return Token(TipoToken::PARENTESIS_ABRE, "(", linea_, columna_ - 1);
        case ')': esperando_valor_ = false; return Token(TipoToken::PARENTESIS_CIERRA, ")", linea_, columna_ - 1);
        case '{': esperando_valor_ = true; return Token(TipoToken::DICCIONARIO_ABRE, "{", linea_, columna_ - 1);
        case '}': esperando_valor_ = false; return Token(TipoToken::DICCIONARIO_CIERRA, "}", linea_, columna_ - 1);
        case ',': esperando_valor_ = true; return Token(TipoToken::COMA, ",", linea_, columna_ - 1);
        case '=':
            if (peek() == '=') { avanzar(); esperando_valor_ = false; return Token(TipoToken::IGUAL, "==", linea_, columna_ - 2); }
            esperando_valor_ = true; return Token(TipoToken::ASIGNAR, "=", linea_, columna_ - 1);
        case '%': esperando_valor_ = false; return Token(TipoToken::MODULO, "%", linea_, columna_ - 1);
        case '?': esperando_valor_ = true; return Token(TipoToken::TERNARIO_PREG, "?", linea_, columna_ - 1);
    }
    if (c == ':') {
        if (peek() == ':') {
            avanzar();
            esperando_valor_ = true;
            return Token(TipoToken::PUNTO_DOBLE, "::", linea_, columna_ - 2);
        }
        char sig = peek();
        if (es_alpha(sig) || sig == '_') { esperando_valor_ = true; return Token(TipoToken::EXTRACTOR, ":", linea_, columna_ - 1); }
        esperando_valor_ = true;
        return Token(TipoToken::TERNARIO_SINO, ":", linea_, columna_ - 1);
    }
    if (c == '>') {
        if (peek() == '>') { avanzar(); profundidad_coleccion_--; esperando_valor_ = false; return Token(TipoToken::MATRIZ_CIERRA, ">>", linea_, columna_ - 2); }
        if (peek() == '=') { avanzar(); esperando_valor_ = true; return Token(TipoToken::MAYOR_IGUAL, ">=", linea_, columna_ - 2); }
        if (profundidad_coleccion_ > 0) { profundidad_coleccion_--; esperando_valor_ = false; return Token(TipoToken::VECTOR_CIERRA, ">", linea_, columna_ - 1); }
        if (esperando_valor_) { esperando_valor_ = false; return Token(TipoToken::VECTOR_CIERRA, ">", linea_, columna_ - 1); }
        esperando_valor_ = false;
        return Token(TipoToken::MAYOR_QUE, ">", linea_, columna_ - 1);
    }
    if (c == '<') {
        if (peek() == '<') { avanzar(); profundidad_coleccion_++; esperando_valor_ = true; return Token(TipoToken::MATRIZ_ABRE, "<<", linea_, columna_ - 2); }
        if (peek() == '=') { avanzar(); esperando_valor_ = true; return Token(TipoToken::MENOR_IGUAL, "<=", linea_, columna_ - 2); }
        if (profundidad_coleccion_ > 0 || esperando_valor_) { profundidad_coleccion_++; esperando_valor_ = true; return Token(TipoToken::VECTOR_ABRE, "<", linea_, columna_ - 1); }
        esperando_valor_ = false;
        return Token(TipoToken::MENOR_QUE, "<", linea_, columna_ - 1);
    }
    if (c == '[') {
        if (peek() == '[') { avanzar(); esperando_valor_ = true; return Token(TipoToken::FILTRO_ABRE, "[[", linea_, columna_ - 2); }
        esperando_valor_ = true;
        return Token(TipoToken::CORCHETE_ABRE, "[", linea_, columna_ - 1);
    }
    if (c == ']') {
        if (peek() == ']') { avanzar(); esperando_valor_ = false; return Token(TipoToken::FILTRO_CIERRA, "]]", linea_, columna_ - 2); }
        esperando_valor_ = false;
        return Token(TipoToken::CORCHETE_CIERRA, "]", linea_, columna_ - 1);
    }
    if (c == '&') { if (peek() == '&') { avanzar(); esperando_valor_ = true; return Token(TipoToken::AND, "&&", linea_, columna_ - 2); } }
    if (c == '|') { if (peek() == '|') { avanzar(); esperando_valor_ = true; return Token(TipoToken::OR, "||", linea_, columna_ - 2); } }
    if (c == '!') {
        if (peek() == '=') { avanzar(); esperando_valor_ = true; return Token(TipoToken::DIFERENTE, "!=", linea_, columna_ - 2); }
        esperando_valor_ = true;
        return Token(TipoToken::NOT, "!", linea_, columna_ - 1);
    }
    if (c == '.') {
        if (es_digito(peek())) {
            // Leading-dot number like .5
            size_t inicio = posicion_ - 1;
            while (!esta_al_final() && es_digito(peek())) avanzar();
            std::string texto = fuente_.substr(inicio, posicion_ - inicio);
            Token token(TipoToken::NUMERO, texto, linea_, columna_ - static_cast<int>(texto.length()));
            try { token.valor_numero = std::stod(texto); } catch (...) {
                throw std::runtime_error("Numero invalido: '" + texto + "' en linea " + std::to_string(linea_));
            }
            esperando_valor_ = false;
            return token;
        }
        esperando_valor_ = true;
        return Token(TipoToken::PUNTO, ".", linea_, columna_ - 1);
    }
    throw std::runtime_error("Caracter inesperado '" + std::string(1, c) + "' en linea " + std::to_string(linea_));
}

std::vector<Token> Lexer::tokenizar() {
    std::vector<Token> tokens;
    while (!esta_al_final()) tokens.push_back(siguiente_token());
    tokens.emplace_back(TipoToken::EOF_TOKEN, "", linea_, columna_);
    return tokens;
}

}
