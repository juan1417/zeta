#include "zeta/interpreter.hpp"
#include "zeta/estadisticas.hpp"
#include "zeta/distribuciones.hpp"
#include "zeta/window_functions.hpp"
#include "zeta/errores.hpp"
#include "zeta/lexer.hpp"
#include "zeta/dl_loader.hpp"
#include "zeta/zeta_abi.h"
#include "zeta/grafo_json.hpp"
#include "zeta/serializador.hpp"
#include "zeta/xlsx_reader.hpp"
#include "zeta/valor.hpp"
#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <cstdlib>
#include <set>
#include <map>
#include <numeric>
#include <random>

namespace zeta {

namespace fs = std::filesystem;

Interpreter::Interpreter() {
    ambito_global_ = std::make_shared<TablaSimbolos>();
    
    // Constantes numéricas
    ambito_global_->definir("PI", mk_num(M_PI));
    ambito_global_->definir("E", mk_num(M_E));
    ambito_global_->definir("INFINITY", mk_num(std::numeric_limits<double>::infinity()));
    ambito_global_->definir("MAX_NUM", mk_num(std::numeric_limits<double>::max()));
    ambito_global_->definir("MIN_NUM", mk_num(std::numeric_limits<double>::min()));
    
    const char* env_path = std::getenv("ZETA_PATH");
    if (env_path) {
        std::string p = env_path;
        size_t start = 0;
        while (start < p.size()) {
            size_t end = p.find(':', start);
            if (end == std::string::npos) end = p.size();
            std::string segment = p.substr(start, end - start);
            if (!segment.empty()) include_paths_.push_back(segment);
            start = end + 1;
        }
    }
}

ValorZeta Interpreter::ejecutar(std::unique_ptr<NodoAST> ast) {
    ast_actual_ = std::move(ast);
    if (!ast_actual_) return mk_null_val();
    return evaluar(*ast_actual_);
}

nlohmann::json Interpreter::obtener_datos_json() {
    nlohmann::json datos = nlohmann::json::object();
    for (const auto& [nombre, valor] : ambito_global_->variables) {
        datos[nombre] = valor_a_json(valor);
    }
    return datos;
}

nlohmann::json Interpreter::obtener_dashboard_json() {
    if (dashboard_) {
        return dashboard_a_json(*dashboard_);
    }
    return nlohmann::json::object();
}

std::vector<nlohmann::json> Interpreter::obtener_metricas_json() {
    std::vector<nlohmann::json> resultado;
    for (const auto& m : metricas_) {
        resultado.push_back(valor_a_json(mk_metrica(m)));
    }
    return resultado;
}

const std::vector<RutaRegistrada>& Interpreter::obtener_rutas() const {
    return rutas_registradas_;
}

ValorZeta Interpreter::llamar_usuario_directo(const ValorZeta& handler, const std::vector<ValorZeta>& args) {
    if (!handler || handler->tipo != ValorImpl::FUNC) {
        return mk_err("runtime", "No se pudo llamar al handler", 0);
    }
    return llamar_usuario(handler, args);
}

static nlohmann::json scene_node_a_json(const SceneNode& n) {
    nlohmann::json j;
    j["tipo"] = n.tipo;
    if (!n.id.empty()) j["id"] = n.id;
    j["titulo"] = n.titulo;
    if (!n.cols.empty()) j["cols"] = n.cols;
    if (!n.nums.empty()) j["nums"] = n.nums;
    if (!n.strs.empty()) j["strs"] = n.strs;
    if (!n.series.empty()) {
        nlohmann::json s = nlohmann::json::object();
        for (const auto& [k, v] : n.series) s[k] = v;
        j["data"] = s;
    }
    return j;
}

nlohmann::json Interpreter::obtener_grafo_json() {
    if (!grafo_actual_) return nlohmann::json::object();
    const auto& s = *grafo_actual_;
    nlohmann::json j;
    j["titulo"] = s.titulo;
    j["autor"] = s.autor;
    j["created_at"] = s.created_at;
    j["updated_at"] = s.updated_at;
    nlohmann::json layout = nlohmann::json::object();
    layout["tipo"] = s.layout.tipo;
    layout["cols"] = s.layout.cols;
    layout["gap"] = s.layout.gap;
    layout["background"] = s.layout.background;
    j["layout"] = layout;
    nlohmann::json nodes = nlohmann::json::array();
    for (const auto& n : s.nodes) nodes.push_back(scene_node_a_json(n));
    j["nodes"] = nodes;
    return j;
}

ValorZeta Interpreter::evaluar(const NodoAST& nodo) {
    switch (nodo.tipo) {
        case TipoNodoAST::LITERAL_NUMERO: return mk_num(nodo.valor_numerico);
        case TipoNodoAST::LITERAL_CADENA: return mk_str(nodo.valor_texto);
        case TipoNodoAST::LITERAL_BOOL: return mk_bool(nodo.valor_bool);
        case TipoNodoAST::LITERAL_NULL: return mk_null_val();
        case TipoNodoAST::VARIABLE: return evaluar_variable(nodo);
        case TipoNodoAST::ASIGNACION: return evaluar_asignacion(nodo);
        case TipoNodoAST::BINARIA: return evaluar_binaria(nodo);
        case TipoNodoAST::UNARIA: return evaluar_unaria(nodo);
        case TipoNodoAST::TERNARIA: return evaluar_ternaria(nodo);
        case TipoNodoAST::VECTOR: return evaluar_vector(nodo);
        case TipoNodoAST::MATRIZ: return evaluar_matriz(nodo);
        case TipoNodoAST::DICCIONARIO: return evaluar_diccionario(nodo);
        case TipoNodoAST::LLAMADA_FUNCION: return evaluar_llamada_funcion(nodo);
        case TipoNodoAST::ACCESO_COLUMNAS: return evaluar_acceso_columnas(nodo);
        case TipoNodoAST::FILTRO_FILAS: return evaluar_filtro_filas(nodo);
        case TipoNodoAST::ACCESO_INDICE: return evaluar_acceso_indice(nodo);
        case TipoNodoAST::ACCESO_MATRIZ: return evaluar_acceso_matriz(nodo);
        case TipoNodoAST::ASIGNACION_INDICE: return evaluar_asignacion_indice(nodo);
        case TipoNodoAST::BREAK: return mk_str("__ZETA_BREAK__");
        case TipoNodoAST::CONTINUE: return mk_str("__ZETA_CONTINUE__");
        case TipoNodoAST::PROPAGACION: {
            auto val = evaluar(*nodo.hijos[0]);
            if (is_error(val)) return val;
            return val;
        }
        case TipoNodoAST::BLOQUE: return evaluar_bloque(nodo);
        case TipoNodoAST::IF_ELSE: return evaluar_if(nodo);
        case TipoNodoAST::FOR_IN: return evaluar_for(nodo);
        case TipoNodoAST::WHILE: return evaluar_while(nodo);
        case TipoNodoAST::PRINT: {
            for (size_t i = 0; i < nodo.hijos.size(); ++i) {
                auto val = evaluar(*nodo.hijos[i]);
                std::cout << valor_a_string(val);
                if (i < nodo.hijos.size() - 1) std::cout << " ";
            }
            std::cout << std::endl;
            return mk_null_val();
        }
        case TipoNodoAST::RETURN: {
            auto val_retorno = evaluar(*nodo.hijos[0]);
            // Wrap return value in a special marker
            std::map<std::string, ValorZeta> wrapper;
            wrapper["__value__"] = val_retorno;
            auto result = mk_dict(wrapper);
            // Mark as return by setting str_val
            result->str_val = "__ZETA_RETURN_VALUE__";
            result->tipo = ValorImpl::DICT; // ensure it stays as dict
            return result;
        }
        case TipoNodoAST::DECLARACION_FN: {
            auto func_val = mk_func(nodo.valor_texto, nodo.parametros,
                          nodo.hijos.empty() ? nullptr : nodo.hijos[0].get(),
                          ambito_global_);
            ambito_global_->definir(nodo.valor_texto, func_val);
            return func_val;
        }
        case TipoNodoAST::FUNCION_ANONIMA: {
            return mk_func(nodo.valor_texto, nodo.parametros,
                          nodo.hijos.empty() ? nullptr : nodo.hijos[0].get(),
                          ambito_global_);
        }
        case TipoNodoAST::DECLARACION_CLASE: return evaluar_clase(nodo);
        case TipoNodoAST::NEW: return evaluar_new(nodo);
        case TipoNodoAST::THIS: return evaluar_this(nodo);
        case TipoNodoAST::LLAMADA_METODO: return evaluar_llamada_metodo(nodo);
        case TipoNodoAST::ACCESO_METODO: {
            auto obj_val = evaluar(*nodo.hijos[0]);
            if (!obj_val || obj_val->tipo != ValorImpl::OBJ) {
                return mk_err("runtime", "Acceso a campo requiere objeto", 0);
            }
            auto& campos = obj_val->obj_val->campos;
            auto it = campos.find(nodo.valor_texto);
            if (it == campos.end()) {
                return mk_err("runtime", "Campo '" + nodo.valor_texto + "' no existe en objeto de clase '" + obj_val->obj_val->clase + "'", 0);
            }
            return it->second;
        }
        case TipoNodoAST::IS_NULL:
        case TipoNodoAST::IS_ERROR:
        case TipoNodoAST::MEAN:
        case TipoNodoAST::COUNT:
        case TipoNodoAST::PLOT:
        case TipoNodoAST::METRIC:
        case TipoNodoAST::DASHBOARD:
        case TipoNodoAST::LOAD_CSV:
        case TipoNodoAST::SERVE:
        case TipoNodoAST::REGISTRAR_RUTA:
        case TipoNodoAST::SUM_FN:
        case TipoNodoAST::MIN_FN:
        case TipoNodoAST::MAX_FN:
        case TipoNodoAST::STDDEV:
        case TipoNodoAST::ABS:
        case TipoNodoAST::ROUND:
        case TipoNodoAST::FLOOR:
        case TipoNodoAST::CEIL:
        case TipoNodoAST::POW:
        case TipoNodoAST::SQRT:
        case TipoNodoAST::LEN:
        case TipoNodoAST::UPPER:
        case TipoNodoAST::LOWER:
        case TipoNodoAST::SUBSTR:
        case TipoNodoAST::REVERSE:
        case TipoNodoAST::SORT_FN:
        case TipoNodoAST::UNIQUE:
        case TipoNodoAST::PUSH:
        case TipoNodoAST::KEYS:
        case TipoNodoAST::VALUES:
        case TipoNodoAST::TYPE_FN:
        case TipoNodoAST::RANGE:
        case TipoNodoAST::TRANSPOSE:
        case TipoNodoAST::DOT:
        case TipoNodoAST::HEAD:
        case TipoNodoAST::SELECT:
        case TipoNodoAST::SPLIT:
        case TipoNodoAST::JOIN:
        case TipoNodoAST::REPLACE:
        case TipoNodoAST::FIND:
        case TipoNodoAST::MAP_FN:
        case TipoNodoAST::FILTER_FN:
        case TipoNodoAST::REDUCE:
            return evaluar_llamada_funcion(nodo);
        case TipoNodoAST::INCLUSION:
            return cargar_modulo(nodo.nombre_funcion);
        case TipoNodoAST::INCLUSION_SELECTIVA:
            return cargar_selectivo(nodo.nombre_funcion, nodo.parametros);
        case TipoNodoAST::INCLUSION_ALIAS:
            return cargar_con_alias(nodo.nombre_funcion, nodo.valor_texto);
        case TipoNodoAST::EXPORTACION: {
            if (cargando_modulo_) {
                for (const auto& nombre : nodo.parametros) {
                    exports_modulo_actual_.insert(nombre);
                }
            }
            return mk_null_val();
        }
        case TipoNodoAST::ACCESO_NAMESPACE: {
            auto ns_val = evaluar(*nodo.hijos[0]);
            if (is_error(ns_val)) return ns_val;
            if (!ns_val || ns_val->tipo != ValorImpl::DICT) {
                return mk_err("runtime", "Acceso namespace requiere un dict", nodo.linea);
            }
            auto it = ns_val->dict_val.find(nodo.valor_texto);
            if (it == ns_val->dict_val.end()) {
                return mk_err("runtime", "Namespace no contiene: " + nodo.valor_texto, nodo.linea);
            }
            return it->second;
        }
        case TipoNodoAST::LLAMADA_NAMESPACE: {
            auto fn_val = evaluar(*nodo.hijos[0]);
            if (is_error(fn_val)) return fn_val;
            std::vector<ValorZeta> args;
            for (size_t i = 1; i < nodo.hijos.size(); ++i) {
                auto arg = evaluar(*nodo.hijos[i]);
                if (is_error(arg)) return arg;
                args.push_back(arg);
            }
            if (!fn_val) {
                return mk_err("runtime", "Miembro namespace es null: " + nodo.nombre_funcion, nodo.linea);
            }
            if (fn_val->tipo == ValorImpl::FUNC) {
                return llamar_usuario(fn_val, args);
            }
            return mk_err("runtime", "Miembro namespace no es funcion: " + nodo.nombre_funcion, nodo.linea);
        }
        default:
            return mk_err("runtime", "Nodo no soportado", nodo.linea);
    }
}

std::string Interpreter::valor_a_string(const ValorZeta& val) {
    if (!val) return "null";
    switch (val->tipo) {
        case ValorImpl::NUM:
            if (es_null(val->num_val)) return "null";
            return std::to_string(val->num_val);
        case ValorImpl::BOOL: return val->bool_val ? "true" : "false";
        case ValorImpl::STR: return val->str_val;
        case ValorImpl::VEC: {
            std::string s = "[";
            for (size_t i = 0; i < val->vec_val.size(); ++i) {
                if (i > 0) s += ", ";
                if (es_null(val->vec_val[i])) s += "null";
                else s += std::to_string(val->vec_val[i]);
            }
            s += "]";
            return s;
        }
        case ValorImpl::BOOL_VEC: {
            std::string s = "[";
            for (size_t i = 0; i < val->bool_vec_val.size(); ++i) {
                if (i > 0) s += ", ";
                s += (val->bool_vec_val[i] ? "true" : "false");
            }
            s += "]";
            return s;
        }
        case ValorImpl::STR_VEC: {
            std::string s = "[";
            for (size_t i = 0; i < val->str_vec_val.size(); ++i) {
                if (i > 0) s += ", ";
                s += "\"" + val->str_vec_val[i] + "\"";
            }
            s += "]";
            return s;
        }
        case ValorImpl::MATRIZ: {
            std::string s = "[";
            for (size_t i = 0; i < val->matriz_val.size(); ++i) {
                if (i > 0) s += ", ";
                s += "[";
                for (size_t j = 0; j < val->matriz_val[i].size(); ++j) {
                    if (j > 0) s += ", ";
                    if (es_null(val->matriz_val[i][j])) s += "null";
                    else s += std::to_string(val->matriz_val[i][j]);
                }
                s += "]";
            }
            s += "]";
            return s;
        }
        case ValorImpl::DF: {
            std::string s = "{";
            bool first = true;
            for (const auto& [col_name, col] : val->df_val.columnas) {
                if (!first) s += ", ";
                s += col_name + ": [";
                for (size_t i = 0; i < col.size(); ++i) {
                    if (i > 0) s += ", ";
                    if (col.null_bitmap[i]) {
                        s += "null";
                    } else if (col.tipo == "num") {
                        s += std::to_string(col.nums[i]);
                    } else if (col.tipo == "str") {
                        s += col.strs[i];
                    } else if (col.tipo == "bool") {
                        s += col.bools[i] ? "true" : "false";
                    }
                }
                s += "]";
                first = false;
            }
            s += "}";
            return s;
        }
        case ValorImpl::DICT: {
            std::string s = "{";
            bool first = true;
            for (const auto& [k, v] : val->dict_val) {
                if (!first) s += ", ";
                s += k + ": " + valor_a_string(v);
                first = false;
            }
            s += "}";
            return s;
        }
        case ValorImpl::ERR: return "[error: " + val->err_val.mensaje + "]";
        case ValorImpl::FUNC: return "[fn " + val->func_nombre + "]";
        case ValorImpl::GRAFICO: return "[plot: " + val->grafico_val.tipo_grafico + "]";
        case ValorImpl::METRICA: return "[metric: " + val->metrica_val.nombre + " = " + std::to_string(val->metrica_val.valor) + "]";
        case ValorImpl::OBJ: {
            auto obj = val->obj_val;
            std::string cur = obj->clase;
            while (!cur.empty()) {
                auto cit = clases_definidas_.find(cur);
                if (cit == clases_definidas_.end()) break;
                auto mit = cit->second->metodos.find("to_string");
                if (mit != cit->second->metodos.end()) {
                    auto prev_this = this_actual_;
                    this_actual_ = val;
                    ValorZeta result = llamar_usuario(mit->second, {val});
                    this_actual_ = prev_this;
                    if (result && result->tipo == ValorImpl::STR) return result->str_val;
                    return result ? valor_a_string(result) : obj->clase + "{}";
                }
                cur = cit->second->padre;
            }
            return obj->clase + "{}";
        }
        default: return tipo_nombre(val);
    }
}

ValorZeta Interpreter::evaluar_variable(const NodoAST& nodo) {
    auto* valor = ambito_global_->buscar(nodo.valor_texto);
    if (!valor) {
        return mk_err("runtime", "Variable no definida: " + nodo.valor_texto, nodo.linea);
    }
    return *valor;
}

ValorZeta Interpreter::evaluar_asignacion(const NodoAST& nodo) {
    if (nodo.valor_texto.rfind("FIELD:", 0) == 0) {
        // Field assignment: $obj.campo = expr
        std::string fname = nodo.valor_texto.substr(6);
        // hijos[0] is the ACCESO_METODO node, hijos[1] is the value
        ValorZeta obj_val = evaluar(*nodo.hijos[0]->hijos[0]);
        if (!obj_val || obj_val->tipo != ValorImpl::OBJ) {
            return mk_err("runtime", "Asignacion a campo requiere objeto", 0);
        }
        ValorZeta val = evaluar(*nodo.hijos[1]);
        if (is_error(val)) return val;
        obj_val->obj_val->campos[fname] = val;
        return val;
    }
    auto valor = evaluar(*nodo.hijos[0]);
    ambito_global_->actualizar(nodo.valor_texto, valor);
    return valor;
}

ValorZeta Interpreter::evaluar_binaria(const NodoAST& nodo) {
    auto izq = evaluar(*nodo.hijos[0]);
    auto der = evaluar(*nodo.hijos[1]);
    auto op = nodo.valor_texto;

    // ── Fast path: NUM ⊕ NUM (most common case) ──────────────────
    // Uses inline double values — no allocation beyond the result.
    if (izq && der && izq->tipo == ValorImpl::NUM && der->tipo == ValorImpl::NUM) {
        double a = izq->num_val;
        double b = der->num_val;
        bool a_null = es_null(a);
        bool b_null = es_null(b);

        if (op == "+") {
            if (a_null || b_null) return mk_null_val();
            return mk_num(a + b);
        }
        if (op == "-") {
            if (a_null || b_null) return mk_null_val();
            return mk_num(a - b);
        }
        if (op == "*") {
            if (a_null || b_null) return mk_null_val();
            return mk_num(a * b);
        }
        if (op == "/") {
            if (a_null || b_null || b == 0.0) return mk_null_val();
            return mk_num(a / b);
        }
        if (op == "%") {
            if (a_null || b_null || b == 0.0) return mk_null_val();
            return mk_num(std::fmod(a, b));
        }
        if (op == "==") return mk_bool(a == b);
        if (op == "!=") return mk_bool(a != b);
        if (op == ">")  return mk_bool(a > b);
        if (op == "<")  return mk_bool(a < b);
        if (op == ">=") return mk_bool(a >= b);
        if (op == "<=") return mk_bool(a <= b);
    }

    // ── Fast path: BOOL ⊕ BOOL ───────────────────────────────────
    if (izq && der && izq->tipo == ValorImpl::BOOL && der->tipo == ValorImpl::BOOL) {
        bool a = izq->bool_val;
        bool b = der->bool_val;
        if (op == "==") return mk_bool(a == b);
        if (op == "!=") return mk_bool(a != b);
    }

    // ── Coercion: bool → num → str ────────────────────────────────
    if (izq && der && izq->tipo != der->tipo) {
        if (izq->tipo == ValorImpl::BOOL && der->tipo == ValorImpl::NUM) {
            izq = mk_num(izq->bool_val ? 1.0 : 0.0);
        } else if (izq->tipo == ValorImpl::NUM && der->tipo == ValorImpl::BOOL) {
            der = mk_num(der->bool_val ? 1.0 : 0.0);
        } else if (izq->tipo == ValorImpl::NUM && der->tipo == ValorImpl::STR) {
            std::string num_str;
            double v = izq->num_val;
            if (v == static_cast<long long>(v) && std::abs(v) < 1e15) {
                num_str = std::to_string(static_cast<long long>(v));
            } else {
                num_str = std::to_string(v);
                auto pos = num_str.find_last_not_of('0');
                if (pos != std::string::npos && num_str[pos] == '.') pos--;
                num_str.erase(pos + 1);
            }
            izq = mk_str(num_str);
        } else if (izq->tipo == ValorImpl::STR && der->tipo == ValorImpl::NUM) {
            std::string num_str;
            double v = der->num_val;
            if (v == static_cast<long long>(v) && std::abs(v) < 1e15) {
                num_str = std::to_string(static_cast<long long>(v));
            } else {
                num_str = std::to_string(v);
                auto pos = num_str.find_last_not_of('0');
                if (pos != std::string::npos && num_str[pos] == '.') pos--;
                num_str.erase(pos + 1);
            }
            der = mk_str(num_str);
        } else if (izq->tipo == ValorImpl::BOOL && der->tipo == ValorImpl::STR) {
            izq = mk_str(izq->bool_val ? "true" : "false");
        } else if (izq->tipo == ValorImpl::STR && der->tipo == ValorImpl::BOOL) {
            der = mk_str(der->bool_val ? "true" : "false");
        }
    }

    if (op == "+" && izq && der) {
        if (izq->tipo == ValorImpl::NUM && der->tipo == ValorImpl::NUM) {
            if (es_null(izq->num_val) || es_null(der->num_val)) return mk_null_val();
            return mk_num(izq->num_val + der->num_val);
        }
        if (izq->tipo == ValorImpl::STR && der->tipo == ValorImpl::STR) {
            return mk_str(izq->str_val + der->str_val);
        }
        if (izq->tipo == ValorImpl::VEC && der->tipo == ValorImpl::VEC) {
            std::vector<double> combined = izq->vec_val;
            combined.insert(combined.end(), der->vec_val.begin(), der->vec_val.end());
            return mk_vec(std::move(combined));
        }
    }

    if (op == "-" && izq && der && izq->tipo == ValorImpl::NUM && der->tipo == ValorImpl::NUM) {
        if (es_null(izq->num_val) || es_null(der->num_val)) return mk_null_val();
        return mk_num(izq->num_val - der->num_val);
    }

    if (op == "*" && izq && der && izq->tipo == ValorImpl::NUM && der->tipo == ValorImpl::NUM) {
        if (es_null(izq->num_val) || es_null(der->num_val)) return mk_null_val();
        return mk_num(izq->num_val * der->num_val);
    }

    if (op == "/" && izq && der && izq->tipo == ValorImpl::NUM && der->tipo == ValorImpl::NUM) {
        if (es_null(izq->num_val) || es_null(der->num_val) || der->num_val == 0.0) return mk_null_val();
        return mk_num(izq->num_val / der->num_val);
    }

    if (op == "%" && izq && der && izq->tipo == ValorImpl::NUM && der->tipo == ValorImpl::NUM) {
        if (es_null(izq->num_val) || es_null(der->num_val) || der->num_val == 0.0) return mk_null_val();
        return mk_num(std::fmod(izq->num_val, der->num_val));
    }

    if ((op == "==" || op == "!=") && izq && der) {
        // Vectorized == / !=
        if (izq->tipo == ValorImpl::VEC && der->tipo == ValorImpl::NUM) {
            std::vector<bool> result;
            for (double v : izq->vec_val) {
                if (es_null(v)) result.push_back(false);
                else result.push_back(op == "==" ? (v == der->num_val) : (v != der->num_val));
            }
            return mk_bool_vec(result);
        }
        if (izq->tipo == ValorImpl::VEC && der->tipo == ValorImpl::VEC) {
            size_t n = std::min(izq->vec_val.size(), der->vec_val.size());
            std::vector<bool> result;
            for (size_t i = 0; i < n; ++i) {
                if (es_null(izq->vec_val[i]) || es_null(der->vec_val[i])) result.push_back(false);
                else result.push_back(op == "==" ? (izq->vec_val[i] == der->vec_val[i]) : (izq->vec_val[i] != der->vec_val[i]));
            }
            return mk_bool_vec(result);
        }
        // Vectorized STR_VEC == STR / STR_VEC == STR_VEC
        if (izq->tipo == ValorImpl::STR_VEC && der->tipo == ValorImpl::STR) {
            std::vector<bool> result;
            for (const auto& s : izq->str_vec_val) {
                result.push_back(op == "==" ? (s == der->str_val) : (s != der->str_val));
            }
            return mk_bool_vec(result);
        }
        if (izq->tipo == ValorImpl::STR && der->tipo == ValorImpl::STR_VEC) {
            std::vector<bool> result;
            for (const auto& s : der->str_vec_val) {
                result.push_back(op == "==" ? (izq->str_val == s) : (izq->str_val != s));
            }
            return mk_bool_vec(result);
        }
        if (izq->tipo == ValorImpl::STR_VEC && der->tipo == ValorImpl::STR_VEC) {
            size_t n = std::min(izq->str_vec_val.size(), der->str_vec_val.size());
            std::vector<bool> result;
            for (size_t i = 0; i < n; ++i) {
                result.push_back(op == "==" ? (izq->str_vec_val[i] == der->str_vec_val[i]) : (izq->str_vec_val[i] != der->str_vec_val[i]));
            }
            return mk_bool_vec(result);
        }
        bool eq = false;
        if (izq->tipo == ValorImpl::NUM && der->tipo == ValorImpl::NUM) {
            if (es_null(izq->num_val) || es_null(der->num_val)) eq = false;
            else eq = izq->num_val == der->num_val;
        } else if (izq->tipo == ValorImpl::BOOL && der->tipo == ValorImpl::BOOL) {
            eq = izq->bool_val == der->bool_val;
        } else if (izq->tipo == ValorImpl::STR && der->tipo == ValorImpl::STR) {
            eq = izq->str_val == der->str_val;
        } else if (izq->tipo == ValorImpl::OBJ && der->tipo == ValorImpl::OBJ) {
            eq = (izq->obj_val.get() == der->obj_val.get());
        }
        return mk_bool(op == "==" ? eq : !eq);
    }

    if (izq && der && izq->tipo == ValorImpl::NUM && der->tipo == ValorImpl::NUM) {
        if (es_null(izq->num_val) || es_null(der->num_val)) return mk_bool(false);
        if (op == ">") return mk_bool(izq->num_val > der->num_val);
        if (op == "<") return mk_bool(izq->num_val < der->num_val);
        if (op == ">=") return mk_bool(izq->num_val >= der->num_val);
        if (op == "<=") return mk_bool(izq->num_val <= der->num_val);
    }

    if (izq && der && izq->tipo == ValorImpl::STR && der->tipo == ValorImpl::STR) {
        if (op == ">") return mk_bool(izq->str_val > der->str_val);
        if (op == "<") return mk_bool(izq->str_val < der->str_val);
        if (op == ">=") return mk_bool(izq->str_val >= der->str_val);
        if (op == "<=") return mk_bool(izq->str_val <= der->str_val);
    }

    // Vectorized comparison: VEC op NUM → bool_vec
    if (izq && der && izq->tipo == ValorImpl::VEC && der->tipo == ValorImpl::NUM) {
        std::vector<bool> result;
        for (double v : izq->vec_val) {
            if (es_null(v)) { result.push_back(false); continue; }
            if (op == ">") result.push_back(v > der->num_val);
            else if (op == "<") result.push_back(v < der->num_val);
            else if (op == ">=") result.push_back(v >= der->num_val);
            else if (op == "<=") result.push_back(v <= der->num_val);
            else if (op == "==") result.push_back(v == der->num_val);
            else if (op == "!=") result.push_back(v != der->num_val);
            else { result.push_back(false); }
        }
        return mk_bool_vec(result);
    }
    // Vectorized comparison: NUM op VEC → bool_vec
    if (izq && der && izq->tipo == ValorImpl::NUM && der->tipo == ValorImpl::VEC) {
        std::vector<bool> result;
        for (double v : der->vec_val) {
            if (es_null(v)) { result.push_back(false); continue; }
            if (op == ">") result.push_back(izq->num_val > v);
            else if (op == "<") result.push_back(izq->num_val < v);
            else if (op == ">=") result.push_back(izq->num_val >= v);
            else if (op == "<=") result.push_back(izq->num_val <= v);
            else if (op == "==") result.push_back(izq->num_val == v);
            else if (op == "!=") result.push_back(izq->num_val != v);
            else { result.push_back(false); }
        }
        return mk_bool_vec(result);
    }
    // Vectorized comparison: VEC op VEC → bool_vec
    if (izq && der && izq->tipo == ValorImpl::VEC && der->tipo == ValorImpl::VEC) {
        size_t n = std::min(izq->vec_val.size(), der->vec_val.size());
        std::vector<bool> result;
        for (size_t i = 0; i < n; ++i) {
            if (es_null(izq->vec_val[i]) || es_null(der->vec_val[i])) { result.push_back(false); continue; }
            if (op == ">") result.push_back(izq->vec_val[i] > der->vec_val[i]);
            else if (op == "<") result.push_back(izq->vec_val[i] < der->vec_val[i]);
            else if (op == ">=") result.push_back(izq->vec_val[i] >= der->vec_val[i]);
            else if (op == "<=") result.push_back(izq->vec_val[i] <= der->vec_val[i]);
            else if (op == "==") result.push_back(izq->vec_val[i] == der->vec_val[i]);
            else if (op == "!=") result.push_back(izq->vec_val[i] != der->vec_val[i]);
            else { result.push_back(false); }
        }
        return mk_bool_vec(result);
    }

    if (op == "&&" && izq && der && izq->tipo == ValorImpl::BOOL && der->tipo == ValorImpl::BOOL) {
        return mk_bool(izq->bool_val && der->bool_val);
    }

    if (op == "||" && izq && der && izq->tipo == ValorImpl::BOOL && der->tipo == ValorImpl::BOOL) {
        return mk_bool(izq->bool_val || der->bool_val);
    }

    return mk_err("runtime", "Operacion no soportada: " + op, nodo.linea);
}

ValorZeta Interpreter::evaluar_unaria(const NodoAST& nodo) {
    auto val = evaluar(*nodo.hijos[0]);

    if (nodo.valor_texto == "-" && val && val->tipo == ValorImpl::NUM) {
        if (es_null(val->num_val)) return mk_null_val();
        return mk_num(-val->num_val);
    }

    if (nodo.valor_texto == "!" && val && val->tipo == ValorImpl::BOOL) {
        return mk_bool(!val->bool_val);
    }

    return mk_err("runtime", "Operacion unaria no soportada", nodo.linea);
}

ValorZeta Interpreter::evaluar_ternaria(const NodoAST& nodo) {
    auto cond = evaluar(*nodo.hijos[0]);

    if (cond && cond->tipo == ValorImpl::BOOL) {
        if (cond->bool_val) return evaluar(*nodo.hijos[1]);
        else return evaluar(*nodo.hijos[2]);
    }

    if (cond && cond->tipo == ValorImpl::VEC) {
        auto v = evaluar(*nodo.hijos[1]);
        auto f = evaluar(*nodo.hijos[2]);

        std::vector<double> resultado;
        resultado.reserve(cond->vec_val.size());

        for (size_t i = 0; i < cond->vec_val.size(); ++i) {
            if (es_null(cond->vec_val[i])) {
                if (f && f->tipo == ValorImpl::VEC && i < f->vec_val.size()) {
                    resultado.push_back(f->vec_val[i]);
                } else if (f && f->tipo == ValorImpl::NUM) {
                    resultado.push_back(f->num_val);
                } else {
                    resultado.push_back(crear_null());
                }
            } else {
                if (v && v->tipo == ValorImpl::VEC && i < v->vec_val.size()) {
                    resultado.push_back(v->vec_val[i]);
                } else if (v && v->tipo == ValorImpl::NUM) {
                    resultado.push_back(v->num_val);
                } else {
                    resultado.push_back(crear_null());
                }
            }
        }
        return mk_vec(resultado);
    }

    // Ternario vectorizado con condicion vector<bool> (resultado de is_null)
    if (cond && cond->tipo == ValorImpl::BOOL_VEC) {
        auto v = evaluar(*nodo.hijos[1]);
        auto f = evaluar(*nodo.hijos[2]);

        std::vector<double> resultado;
        resultado.reserve(cond->bool_vec_val.size());

        for (size_t i = 0; i < cond->bool_vec_val.size(); ++i) {
            if (cond->bool_vec_val[i]) {
                // Condicion verdadera -> tomar valor de v
                if (v && v->tipo == ValorImpl::VEC && i < v->vec_val.size()) {
                    resultado.push_back(v->vec_val[i]);
                } else if (v && v->tipo == ValorImpl::NUM) {
                    resultado.push_back(v->num_val);
                } else {
                    resultado.push_back(crear_null());
                }
            } else {
                // Condicion falsa -> tomar valor de f
                if (f && f->tipo == ValorImpl::VEC && i < f->vec_val.size()) {
                    resultado.push_back(f->vec_val[i]);
                } else if (f && f->tipo == ValorImpl::NUM) {
                    resultado.push_back(f->num_val);
                } else {
                    resultado.push_back(crear_null());
                }
            }
        }
        return mk_vec(resultado);
    }

    return mk_err("runtime", "Condicion ternaria no es booleana", nodo.linea);
}

ValorZeta Interpreter::evaluar_vector(const NodoAST& nodo) {
    // Evaluate all children and detect types
    std::vector<ValorZeta> elems;
    bool has_num = false, has_str = false, has_bool = false, has_obj = false;
    for (const auto& hijo : nodo.hijos) {
        auto val = evaluar(*hijo);
        elems.push_back(val);
        if (!val) { has_num = true; continue; } // null is compatible with num
        switch (val->tipo) {
            case ValorImpl::NUM:  has_num = true; break;
            case ValorImpl::STR:  has_str = true; break;
            case ValorImpl::BOOL: has_bool = true; break;
            case ValorImpl::OBJ:  has_obj = true; break;
            default: break;
        }
    }

    int type_count = (has_num ? 1 : 0) + (has_str ? 1 : 0) + (has_bool ? 1 : 0) + (has_obj ? 1 : 0);

    if (type_count <= 1) {
        // Homogeneous: all same type
        if (has_str) {
            std::vector<std::string> strs;
            for (auto& e : elems) {
                if (!e) strs.push_back("");
                else if (e->tipo == ValorImpl::STR) strs.push_back(e->str_val);
                else strs.push_back(valor_a_string(e));
            }
            return mk_str_vec(std::move(strs));
        }
        if (has_bool && !has_num) {
            std::vector<bool> bools;
            for (auto& e : elems) {
                if (!e) bools.push_back(false);
                else if (e->tipo == ValorImpl::BOOL) bools.push_back(e->bool_val);
                else bools.push_back(false);
            }
            return mk_bool_vec(std::move(bools));
        }
        if (has_obj && !has_num && !has_str && !has_bool) {
            // All objects: return as dict with index keys (no OBJ_VEC type exists)
            std::map<std::string, ValorZeta> dict;
            for (size_t i = 0; i < elems.size(); ++i) {
                dict[std::to_string(i)] = elems[i];
            }
            return mk_dict(std::move(dict));
        }
        // Default: numeric (includes null-only and bool+null cases)
        std::vector<double> nums;
        for (auto& e : elems) {
            if (!e) nums.push_back(crear_null());
            else if (e->tipo == ValorImpl::NUM) nums.push_back(e->num_val);
            else if (e->tipo == ValorImpl::BOOL) nums.push_back(e->bool_val ? 1.0 : 0.0);
            else nums.push_back(crear_null());
        }
        return mk_vec(std::move(nums));
    }

    // Mixed types → dict with index keys (preserves types)
    std::map<std::string, ValorZeta> dict;
    for (size_t i = 0; i < elems.size(); ++i) {
        dict[std::to_string(i)] = elems[i];
    }
    return mk_dict(std::move(dict));
}

ValorZeta Interpreter::evaluar_matriz(const NodoAST& nodo) {
    std::vector<std::vector<double>> filas;
    for (const auto& hijo : nodo.hijos) {
        std::vector<double> fila;
        for (const auto& elem : hijo->hijos) {
            auto val = evaluar(*elem);
            if (val && val->tipo == ValorImpl::NUM) {
                fila.push_back(val->num_val);
            } else {
                fila.push_back(crear_null());
            }
        }
        filas.push_back(std::move(fila));
    }
    return mk_matriz(filas);
}

ValorZeta Interpreter::evaluar_diccionario(const NodoAST& nodo) {
    bool es_dataframe = true;
    size_t len_esperada = 0;
    std::map<std::string, Columna> df_columns;
    std::map<std::string, ValorZeta> dict_map;

    for (const auto& hijo : nodo.hijos) {
        if (hijo->tipo != TipoNodoAST::ASIGNACION) {
            es_dataframe = false;
            continue;
        }

        auto clave = hijo->valor_texto;
        auto valor = evaluar(*hijo->hijos[0]);
        dict_map[clave] = valor;

        if (valor && valor->tipo == ValorImpl::VEC) {
            Columna col("num");
            col.nums = valor->vec_val;
            col.null_bitmap.resize(col.nums.size(), false);
            for (size_t i = 0; i < col.nums.size(); ++i) {
                if (es_null(col.nums[i])) col.null_bitmap[i] = true;
            }
            df_columns[clave] = std::move(col);
            if (df_columns.size() == 1) len_esperada = valor->vec_val.size();
            else if (valor->vec_val.size() != len_esperada) es_dataframe = false;
        } else if (valor && valor->tipo == ValorImpl::STR_VEC) {
            Columna col("str");
            col.strs = valor->str_vec_val;
            col.null_bitmap.resize(col.strs.size(), false);
            for (size_t i = 0; i < col.strs.size(); ++i) {
                if (col.strs[i].empty()) col.null_bitmap[i] = true;
            }
            df_columns[clave] = std::move(col);
            if (df_columns.size() == 1) len_esperada = valor->str_vec_val.size();
            else if (valor->str_vec_val.size() != len_esperada) es_dataframe = false;
        } else if (valor && valor->tipo == ValorImpl::BOOL_VEC) {
            Columna col("bool");
            col.bools = valor->bool_vec_val;
            col.null_bitmap.resize(col.bools.size(), false);
            df_columns[clave] = std::move(col);
            if (df_columns.size() == 1) len_esperada = valor->bool_vec_val.size();
            else if (valor->bool_vec_val.size() != len_esperada) es_dataframe = false;
        } else {
            es_dataframe = false;
        }
    }

    if (es_dataframe && !df_columns.empty()) {
        DataFrame df;
        for (const auto& [k, v] : df_columns) {
            df.nombres_columnas.push_back(k);
        }
        df.columnas = std::move(df_columns);
        return mk_df(std::move(df));
    }

    return mk_dict(dict_map);
}

ValorZeta Interpreter::evaluar_llamada_funcion(const NodoAST& nodo) {
    // First check if it's a user-defined function
    auto* fn_val = ambito_global_->buscar(nodo.nombre_funcion);
    if (fn_val && *fn_val && (*fn_val)->tipo == ValorImpl::FUNC) {
        std::vector<ValorZeta> args;
        for (const auto& hijo : nodo.hijos) {
            args.push_back(evaluar(*hijo));
        }
        return llamar_usuario(*fn_val, args);
    }

    // Otherwise it's a native function
    std::vector<ValorZeta> args;
    for (const auto& hijo : nodo.hijos) {
        args.push_back(evaluar(*hijo));
    }
    return llamar_nativa(nodo.nombre_funcion, args);
}

ValorZeta Interpreter::llamar_usuario(const ValorZeta& func, const std::vector<ValorZeta>& args) {
    if (!func || func->tipo != ValorImpl::FUNC) {
        return mk_err("runtime", "Intento de llamar algo que no es funcion", 0);
    }

    if (func->func_nombre.rfind("::dl::", 0) == 0 && func->func_cuerpo) {
        auto fn = reinterpret_cast<zeta_fn_t>(func->func_cuerpo);
        std::vector<double> packed;
        packed.reserve(args.size());
        for (const auto& a : args) {
            if (!a) packed.push_back(crear_null());
            else if (a->tipo == ValorImpl::NUM) packed.push_back(a->num_val);
            else if (a->tipo == ValorImpl::VEC) {
                for (double v : a->vec_val) packed.push_back(v);
            } else if (a->tipo == ValorImpl::BOOL) packed.push_back(a->bool_val ? 1.0 : 0.0);
            else packed.push_back(crear_null());
        }
        double result = fn(static_cast<int>(packed.size()), packed.data());
        return mk_num(result);
    }

    // Create child scope with closure
    auto ambito_func = std::make_shared<TablaSimbolos>(func->func_cierre);

    if (args.size() > func->func_params.size()) {
        return mk_err("runtime", "Demasiados argumentos para funcion '" + func->func_nombre +
                       "': esperaba " + std::to_string(func->func_params.size()) +
                       " pero recibio " + std::to_string(args.size()), 0);
    }

    // Bind parameters
    for (size_t i = 0; i < func->func_params.size(); ++i) {
        if (i < args.size()) {
            ambito_func->definir(func->func_params[i], args[i]);
        } else {
            return mk_err("runtime", "Falta argumento '" + func->func_params[i] +
                           "' para funcion '" + func->func_nombre + "'", 0);
        }
    }

    // Execute body
    auto ambito_anterior = ambito_global_;
    ambito_global_ = ambito_func;

    ValorZeta resultado = mk_null_val();
    if (func->func_cuerpo) {
        auto* cuerpo = static_cast<const NodoAST*>(func->func_cuerpo);
        resultado = evaluar_bloque(*cuerpo);
        if (resultado && resultado->tipo == ValorImpl::DICT &&
            resultado->str_val == "__ZETA_RETURN_VALUE__" &&
            resultado->dict_val.count("__value__")) {
            resultado = resultado->dict_val.at("__value__");
        }
    }

    ambito_global_ = ambito_anterior;

    return resultado;
}

ValorZeta Interpreter::llamar_nativa(const std::string& nombre, const std::vector<ValorZeta>& args) {
    if (nombre == "is_null") {
        if (args.empty()) return mk_bool(false);
        if (!args[0]) return mk_bool(true);
        if (args[0]->tipo == ValorImpl::VEC) {
            return mk_bool_vec(fn_is_null_bool(args[0]->vec_val));
        }
        if (args[0]->tipo == ValorImpl::STR_VEC) {
            std::vector<bool> result;
            for (const auto& s : args[0]->str_vec_val) result.push_back(s.empty());
            return mk_bool_vec(result);
        }
        if (args[0]->tipo == ValorImpl::BOOL_VEC) {
            std::vector<bool> result(args[0]->bool_vec_val.size(), false);
            return mk_bool_vec(result);
        }
        return mk_bool(es_null_valor(args[0]) || (args[0]->tipo == ValorImpl::STR && args[0]->str_val.empty()));
    }

    if (nombre == "fill_null") {
        if (args.size() < 2) return mk_err("runtime", "fill_null requiere (valor, defecto)", 0);
        auto val = args[0];
        auto def = args[1];
        if (!val) return def;
        if (val->tipo == ValorImpl::VEC) {
            std::vector<double> result = val->vec_val;
            for (auto& v : result) {
                if (es_null(v)) v = def->num_val;
            }
            return mk_vec(result);
        }
        if (val->tipo == ValorImpl::STR_VEC) {
            std::vector<std::string> result = val->str_vec_val;
            for (auto& s : result) {
                if (s.empty()) s = def->str_val;
            }
            return mk_str_vec(result);
        }
        if (val->tipo == ValorImpl::NUM && es_null(val->num_val)) {
            return def;
        }
        return val;
    }

    if (nombre == "is_error") {
        if (args.empty()) return mk_bool(false);
        return mk_bool(is_error(args[0]));
    }

    if (nombre == "mean") {
        if (args.empty()) return mk_null_val();
        if (args[0] && args[0]->tipo == ValorImpl::VEC) {
            return mk_num(fn_mean(args[0]->vec_val));
        }
        if (args[0] && args[0]->tipo == ValorImpl::NUM) return args[0];
        return mk_null_val();
    }

    if (nombre == "count") {
        if (args.empty()) return mk_num(0);
        if (args[0] && args[0]->tipo == ValorImpl::VEC) {
            return mk_num(static_cast<double>(fn_count(args[0]->vec_val)));
        }
        return mk_num(1);
    }

    if (nombre == "sum") {
        if (args.empty()) return mk_null_val();
        if (args[0] && args[0]->tipo == ValorImpl::VEC) {
            return mk_num(fn_sum(args[0]->vec_val));
        }
        if (args[0] && args[0]->tipo == ValorImpl::BOOL_VEC) {
            double total = 0;
            for (bool b : args[0]->bool_vec_val) total += b ? 1.0 : 0.0;
            return mk_num(total);
        }
        return mk_null_val();
    }

    if (nombre == "min") {
        if (args.empty()) return mk_null_val();
        if (args[0] && args[0]->tipo == ValorImpl::VEC) {
            return mk_num(fn_min(args[0]->vec_val));
        }
        return mk_null_val();
    }

    if (nombre == "max") {
        if (args.empty()) return mk_null_val();
        if (args[0] && args[0]->tipo == ValorImpl::VEC) {
            return mk_num(fn_max(args[0]->vec_val));
        }
        return mk_null_val();
    }

    if (nombre == "stddev") {
        if (args.empty()) return mk_null_val();
        if (args[0] && args[0]->tipo == ValorImpl::VEC) {
            return mk_num(fn_stddev(args[0]->vec_val));
        }
        return mk_null_val();
    }

    if (nombre == "abs") {
        if (args.empty() || args[0]->tipo != ValorImpl::NUM) return mk_err("runtime", "abs requiere un numero", 0);
        if (es_null(args[0]->num_val)) return mk_null_val();
        return mk_num(std::abs(args[0]->num_val));
    }

    if (nombre == "round") {
        if (args.empty() || args[0]->tipo != ValorImpl::NUM) return mk_err("runtime", "round requiere un numero", 0);
        if (es_null(args[0]->num_val)) return mk_null_val();
        return mk_num(std::round(args[0]->num_val));
    }

    if (nombre == "floor") {
        if (args.empty() || args[0]->tipo != ValorImpl::NUM) return mk_err("runtime", "floor requiere un numero", 0);
        if (es_null(args[0]->num_val)) return mk_null_val();
        return mk_num(std::floor(args[0]->num_val));
    }

    if (nombre == "ceil") {
        if (args.empty() || args[0]->tipo != ValorImpl::NUM) return mk_err("runtime", "ceil requiere un numero", 0);
        if (es_null(args[0]->num_val)) return mk_null_val();
        return mk_num(std::ceil(args[0]->num_val));
    }

    if (nombre == "pow") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::NUM || args[1]->tipo != ValorImpl::NUM)
            return mk_err("runtime", "pow requiere base y exponente", 0);
        if (es_null(args[0]->num_val) || es_null(args[1]->num_val)) return mk_null_val();
        return mk_num(std::pow(args[0]->num_val, args[1]->num_val));
    }

    if (nombre == "sqrt") {
        if (args.empty() || args[0]->tipo != ValorImpl::NUM) return mk_err("runtime", "sqrt requiere un numero", 0);
        if (es_null(args[0]->num_val)) return mk_null_val();
        return mk_num(std::sqrt(args[0]->num_val));
    }

    if (nombre == "format") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::NUM || args[1]->tipo != ValorImpl::NUM)
            return mk_err("runtime", "format requiere (numero, decimales)", 0);
        if (es_null(args[0]->num_val)) return mk_null_val();
        int decimales = static_cast<int>(args[1]->num_val);
        if (decimales < 0) decimales = 0;
        if (decimales > 15) decimales = 15;
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(decimales) << args[0]->num_val;
        return mk_str(ss.str());
    }

    if (nombre == "time") {
        auto now = std::chrono::high_resolution_clock::now();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
        return mk_num(static_cast<double>(ns) / 1e9);
    }

    if (nombre == "len") {
        if (args.empty()) return mk_err("runtime", "len requiere un argumento", 0);
        if (args[0]->tipo == ValorImpl::STR) return mk_num(static_cast<double>(args[0]->str_val.size()));
        if (args[0]->tipo == ValorImpl::VEC) return mk_num(static_cast<double>(args[0]->vec_val.size()));
        if (args[0]->tipo == ValorImpl::STR_VEC) return mk_num(static_cast<double>(args[0]->str_vec_val.size()));
        if (args[0]->tipo == ValorImpl::BOOL_VEC) return mk_num(static_cast<double>(args[0]->bool_vec_val.size()));
        if (args[0]->tipo == ValorImpl::DICT) return mk_num(static_cast<double>(args[0]->dict_val.size()));
        if (args[0]->tipo == ValorImpl::DF) return mk_num(static_cast<double>(args[0]->df_val.filas()));
        if (args[0]->tipo == ValorImpl::MATRIZ) return mk_num(static_cast<double>(args[0]->matriz_val.size()));
        return mk_err("runtime", "len no soporta tipo: " + tipo_nombre(args[0]), 0);
    }

    if (nombre == "upper") {
        if (args.empty() || args[0]->tipo != ValorImpl::STR) return mk_err("runtime", "upper requiere un string", 0);
        std::string s = args[0]->str_val;
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        return mk_str(s);
    }

    if (nombre == "lower") {
        if (args.empty() || args[0]->tipo != ValorImpl::STR) return mk_err("runtime", "lower requiere un string", 0);
        std::string s = args[0]->str_val;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return mk_str(s);
    }

    if (nombre == "substr") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::STR || args[1]->tipo != ValorImpl::NUM)
            return mk_err("runtime", "substr requiere string, inicio, [longitud]", 0);
        int start = static_cast<int>(args[1]->num_val);
        if (start < 0) start += static_cast<int>(args[0]->str_val.size());
        if (start < 0 || start >= static_cast<int>(args[0]->str_val.size())) return mk_str("");
        if (args.size() >= 3 && args[2]->tipo == ValorImpl::NUM) {
            int len = static_cast<int>(args[2]->num_val);
            return mk_str(args[0]->str_val.substr(start, len));
        }
        return mk_str(args[0]->str_val.substr(start));
    }

    if (nombre == "reverse") {
        if (args.empty() || args[0]->tipo != ValorImpl::VEC) return mk_err("runtime", "reverse requiere un vector", 0);
        std::vector<double> v = args[0]->vec_val;
        std::reverse(v.begin(), v.end());
        return mk_vec(std::move(v));
    }

    if (nombre == "sort") {
        if (args.empty()) return mk_err("runtime", "sort requiere un argumento", 0);
        if (args[0]->tipo == ValorImpl::VEC) {
            std::vector<double> v = args[0]->vec_val;
            std::sort(v.begin(), v.end());
            return mk_vec(std::move(v));
        }
        if (args[0]->tipo == ValorImpl::STR_VEC) {
            std::vector<std::string> v = args[0]->str_vec_val;
            std::sort(v.begin(), v.end());
            return mk_str_vec(std::move(v));
        }
        return mk_err("runtime", "sort requiere un vector o str_vec", 0);
    }

    if (nombre == "unique") {
        if (args.empty() || args[0]->tipo != ValorImpl::VEC) return mk_err("runtime", "unique requiere un vector", 0);
        std::vector<double> v = args[0]->vec_val;
        std::sort(v.begin(), v.end());
        v.erase(std::unique(v.begin(), v.end()), v.end());
        return mk_vec(std::move(v));
    }

    if (nombre == "push") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::NUM)
            return mk_err("runtime", "push requiere vector y valor", 0);
        std::vector<double> v = args[0]->vec_val;
        v.push_back(args[1]->num_val);
        return mk_vec(std::move(v));
    }

    if (nombre == "keys") {
        if (args.empty() || args[0]->tipo != ValorImpl::DICT) return mk_err("runtime", "keys requiere un dict", 0);
        std::vector<double> indices;
        std::vector<std::string> keys;
        for (const auto& [k, _] : args[0]->dict_val) keys.push_back(k);
        return mk_str_vec(std::move(keys));
    }

    if (nombre == "values") {
        if (args.empty() || args[0]->tipo != ValorImpl::DICT) return mk_err("runtime", "values requiere un dict", 0);
        std::vector<double> vals;
        for (const auto& [_, v] : args[0]->dict_val) {
            if (v && v->tipo == ValorImpl::NUM) vals.push_back(v->num_val);
            else vals.push_back(crear_null());
        }
        return mk_vec(std::move(vals));
    }

    if (nombre == "type") {
        if (args.empty()) return mk_err("runtime", "type requiere un argumento", 0);
        return mk_str(tipo_nombre(args[0]));
    }

    if (nombre == "range") {
        if (args.empty() || args[0]->tipo != ValorImpl::NUM) return mk_err("runtime", "range requiere inicio numerico", 0);
        double start = args[0]->num_val;
        double end = 0.0, step = 1.0;
        if (args.size() >= 2 && args[1]->tipo == ValorImpl::NUM) {
            end = args[1]->num_val;
        } else {
            end = start; start = 0.0;
        }
        if (args.size() >= 3 && args[2]->tipo == ValorImpl::NUM) step = args[2]->num_val;
        std::vector<double> result;
        for (double i = start; i < end; i += step) result.push_back(i);
        return mk_vec(std::move(result));
    }

    if (nombre == "transpose") {
        if (args.empty() || args[0]->tipo != ValorImpl::MATRIZ) return mk_err("runtime", "transpose requiere una matriz", 0);
        const auto& m = args[0]->matriz_val;
        if (m.empty()) return mk_matriz({});
        size_t rows = m.size(), cols = m[0].size();
        std::vector<std::vector<double>> t(cols, std::vector<double>(rows));
        for (size_t i = 0; i < rows; ++i)
            for (size_t j = 0; j < cols; ++j)
                t[j][i] = m[i][j];
        return mk_matriz(t);
    }

    if (nombre == "dot") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "dot requiere dos vectores", 0);
        const auto& a = args[0]->vec_val, &b = args[1]->vec_val;
        if (a.size() != b.size()) return mk_err("runtime", "dot: vectores de diferente longitud", 0);
        double sum = 0.0;
        for (size_t i = 0; i < a.size(); ++i) {
            if (es_null(a[i]) || es_null(b[i])) return mk_null_val();
            sum += a[i] * b[i];
        }
        return mk_num(sum);
    }

    if (nombre == "head") {
        if (args.empty()) return mk_err("runtime", "head requiere un DataFrame o vector", 0);
        int n = 5;
        if (args.size() >= 2 && args[1]->tipo == ValorImpl::NUM) n = static_cast<int>(args[1]->num_val);
        if (args[0]->tipo == ValorImpl::DF) {
            const auto& df = args[0]->df_val;
            DataFrame result;
            result.nombres_columnas = df.nombres_columnas;
            for (const auto& [col_name, col] : df.columnas) {
                Columna sliced(col.tipo);
                for (int i = 0; i < n && i < static_cast<int>(col.size()); ++i) {
                    if (col.tipo == "num") {
                        sliced.nums.push_back(col.nums[i]);
                        sliced.null_bitmap.push_back(col.null_bitmap[i]);
                    } else if (col.tipo == "str") {
                        sliced.strs.push_back(col.strs[i]);
                        sliced.null_bitmap.push_back(col.null_bitmap[i]);
                    } else if (col.tipo == "bool") {
                        sliced.bools.push_back(col.bools[i]);
                        sliced.null_bitmap.push_back(col.null_bitmap[i]);
                    }
                }
                result.columnas[col_name] = std::move(sliced);
            }
            return mk_df(result);
        }
        if (args[0]->tipo == ValorImpl::VEC) {
            std::vector<double> sliced;
            for (int i = 0; i < n && i < static_cast<int>(args[0]->vec_val.size()); ++i) sliced.push_back(args[0]->vec_val[i]);
            return mk_vec(sliced);
        }
        if (args[0]->tipo == ValorImpl::STR_VEC) {
            std::vector<std::string> sliced;
            for (int i = 0; i < n && i < static_cast<int>(args[0]->str_vec_val.size()); ++i) sliced.push_back(args[0]->str_vec_val[i]);
            return mk_str_vec(sliced);
        }
        if (args[0]->tipo == ValorImpl::BOOL_VEC) {
            std::vector<bool> sliced;
            for (int i = 0; i < n && i < static_cast<int>(args[0]->bool_vec_val.size()); ++i) sliced.push_back(args[0]->bool_vec_val[i]);
            return mk_bool_vec(sliced);
        }
        return mk_err("runtime", "head requiere DataFrame o vector", 0);
    }

    if (nombre == "select") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::DF || args[1]->tipo != ValorImpl::STR)
            return mk_err("runtime", "select requiere DataFrame y nombre de columna", 0);
        const auto& df = args[0]->df_val;
        const std::string& col = args[1]->str_val;
        auto it = df.columnas.find(col);
        if (it == df.columnas.end()) return mk_err("runtime", "Columna no encontrada: " + col, 0);
        const auto& c = it->second;
        if (c.tipo == "num") return mk_vec(c.nums);
        if (c.tipo == "str") return mk_str_vec(c.strs);
        if (c.tipo == "bool") return mk_bool_vec(c.bools);
        return mk_err("runtime", "Tipo de columna desconocido", 0);
    }

    if (nombre == "drop") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::DF || args[1]->tipo != ValorImpl::STR)
            return mk_err("runtime", "drop requiere DataFrame y nombre de columna", 0);
        const auto& df = args[0]->df_val;
        const std::string& col_a_eliminar = args[1]->str_val;
        if (df.columnas.find(col_a_eliminar) == df.columnas.end())
            return mk_err("runtime", "Columna no encontrada: " + col_a_eliminar, 0);
        DataFrame resultado;
        for (const auto& nombre_col : df.nombres_columnas) {
            if (nombre_col != col_a_eliminar) {
                resultado.nombres_columnas.push_back(nombre_col);
                resultado.columnas[nombre_col] = df.columnas.at(nombre_col);
            }
        }
        return mk_df(std::move(resultado));
    }

    if (nombre == "drop_nan") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::DF || args[1]->tipo != ValorImpl::STR)
            return mk_err("runtime", "drop_nan requiere DataFrame y nombre de columna", 0);
        const auto& df = args[0]->df_val;
        const std::string& col = args[1]->str_val;
        auto it = df.columnas.find(col);
        if (it == df.columnas.end())
            return mk_err("runtime", "Columna no encontrada: " + col, 0);
        const Columna& columna_ref = it->second;
        std::vector<bool> mascara;
        for (size_t i = 0; i < columna_ref.size(); ++i) {
            mascara.push_back(!columna_ref.es_null(i));
        }
        DataFrame resultado;
        resultado.nombres_columnas = df.nombres_columnas;
        for (const auto& [nombre, col_data] : df.columnas) {
            Columna nueva_col(col_data.tipo);
            for (size_t i = 0; i < col_data.size(); ++i) {
                if (mascara[i]) {
                    if (col_data.tipo == "num") {
                        nueva_col.nums.push_back(col_data.nums[i]);
                        nueva_col.null_bitmap.push_back(col_data.null_bitmap[i]);
                    } else if (col_data.tipo == "str") {
                        nueva_col.strs.push_back(col_data.strs[i]);
                        nueva_col.null_bitmap.push_back(col_data.null_bitmap[i]);
                    } else if (col_data.tipo == "bool") {
                        nueva_col.bools.push_back(col_data.bools[i]);
                        nueva_col.null_bitmap.push_back(col_data.null_bitmap[i]);
                    }
                }
            }
            resultado.columnas[nombre] = std::move(nueva_col);
        }
        return mk_df(std::move(resultado));
    }

    if (nombre == "group_by") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::DF)
            return mk_err("runtime", "group_by requiere DataFrame y columna(s)", 0);

        const auto& df = args[0]->df_val;

        std::vector<std::string> group_cols;
        for (size_t i = 1; i < args.size(); ++i) {
            if (args[i]->tipo != ValorImpl::STR)
                return mk_err("runtime", "group_by: columnas deben ser strings", 0);
            group_cols.push_back(args[i]->str_val);
        }

        std::map<std::string, std::vector<double>> groups;
        for (size_t row = 0; row < df.filas(); ++row) {
            std::string key;
            for (const auto& gc : group_cols) {
                auto it = df.columnas.find(gc);
                if (it == df.columnas.end()) continue;
                const auto& col = it->second;
                if (col.tipo == "num") {
                    if (col.null_bitmap[row]) key += "NULL|";
                    else key += std::to_string(col.nums[row]) + "|";
                } else if (col.tipo == "str") {
                    key += col.strs[row] + "|";
                } else if (col.tipo == "bool") {
                    key += (col.bools[row] ? "true" : "false") + std::string("|");
                }
            }
            groups[key].push_back(static_cast<double>(row));
        }

        std::map<std::string, ValorZeta> result;
        result["_type"] = mk_str("grouped_df");
        result["_data"] = args[0];
        result["_group_cols"] = mk_str_vec(group_cols);

        std::map<std::string, ValorZeta> groups_dict;
        for (auto& [k, v] : groups) {
            groups_dict[k] = mk_vec(v);
        }
        result["_groups"] = mk_dict(std::move(groups_dict));

        return mk_dict(std::move(result));
    }

    if (nombre == "agg") {
        if (args.size() < 3 || args[0]->tipo != ValorImpl::DICT)
            return mk_err("runtime", "agg requiere grouped_df, columna, y funcion", 0);

        auto it_type = args[0]->dict_val.find("_type");
        if (it_type == args[0]->dict_val.end() || !it_type->second || it_type->second->str_val != "grouped_df")
            return mk_err("runtime", "agg: primer argumento debe ser grouped_df (de group_by)", 0);

        if (args[1]->tipo != ValorImpl::STR)
            return mk_err("runtime", "agg: columna debe ser string", 0);
        if (args[2]->tipo != ValorImpl::STR)
            return mk_err("runtime", "agg: funcion debe ser string", 0);

        const std::string& col_name = args[1]->str_val;
        const std::string& func_name = args[2]->str_val;

        auto& dict = args[0]->dict_val;
        const DataFrame& df = dict.at("_data")->df_val;
        auto& groups = dict.at("_groups")->dict_val;
        auto& group_cols = dict.at("_group_cols")->str_vec_val;

        auto col_it = df.columnas.find(col_name);
        if (col_it == df.columnas.end())
            return mk_err("runtime", "agg: columna '" + col_name + "' no encontrada", 0);

        DataFrame result;
        for (const auto& gc : group_cols) {
            result.nombres_columnas.push_back(gc);
            result.columnas[gc] = Columna("str");
        }
        std::string agg_col_name = col_name + "_" + func_name;
        result.nombres_columnas.push_back(agg_col_name);
        result.columnas[agg_col_name] = Columna("num");

        for (auto& [key, indices_val] : groups) {
            std::vector<std::string> key_parts;
            std::string part;
            std::string k = key;
            if (!k.empty() && k.back() == '|') k.pop_back();
            std::istringstream iss(k);
            while (std::getline(iss, part, '|')) {
                key_parts.push_back(part);
            }

            for (size_t i = 0; i < group_cols.size(); ++i) {
                if (i < key_parts.size()) {
                    result.columnas[group_cols[i]].strs.push_back(key_parts[i]);
                    result.columnas[group_cols[i]].null_bitmap.push_back(false);
                } else {
                    result.columnas[group_cols[i]].strs.push_back("");
                    result.columnas[group_cols[i]].null_bitmap.push_back(true);
                }
            }

            const auto& idx_vec = indices_val->vec_val;
            std::vector<double> values;
            for (double idx_d : idx_vec) {
                size_t idx = static_cast<size_t>(idx_d);
                if (col_it->second.tipo == "num" && idx < col_it->second.nums.size()) {
                    double v = col_it->second.nums[idx];
                    if (!col_it->second.null_bitmap[idx]) values.push_back(v);
                }
            }

            double agg_val = crear_null();
            if (!values.empty()) {
                if (func_name == "sum") {
                    agg_val = 0;
                    for (double v : values) agg_val += v;
                } else if (func_name == "mean") {
                    double s = 0;
                    for (double v : values) s += v;
                    agg_val = s / values.size();
                } else if (func_name == "count") {
                    agg_val = static_cast<double>(values.size());
                } else if (func_name == "min") {
                    agg_val = values[0];
                    for (double v : values) if (v < agg_val) agg_val = v;
                } else if (func_name == "max") {
                    agg_val = values[0];
                    for (double v : values) if (v > agg_val) agg_val = v;
                } else if (func_name == "stddev") {
                    double s = 0;
                    for (double v : values) s += v;
                    double mean = s / values.size();
                    double sq_sum = 0;
                    for (double v : values) sq_sum += (v - mean) * (v - mean);
                    agg_val = std::sqrt(sq_sum / values.size());
                } else if (func_name == "median") {
                    std::vector<double> sorted = values;
                    std::sort(sorted.begin(), sorted.end());
                    size_t n = sorted.size();
                    if (n % 2 == 0) agg_val = (sorted[n/2 - 1] + sorted[n/2]) / 2.0;
                    else agg_val = sorted[n/2];
                }
            }

            result.columnas[agg_col_name].nums.push_back(agg_val);
            result.columnas[agg_col_name].null_bitmap.push_back(es_null(agg_val));
        }

        return mk_df(std::move(result));
    }

    if (nombre == "merge") {
        if (args.size() < 3 || args[0]->tipo != ValorImpl::DF || args[1]->tipo != ValorImpl::DF)
            return mk_err("runtime", "merge requiere dos DataFrames y una columna", 0);
        if (args[2]->tipo != ValorImpl::STR)
            return mk_err("runtime", "merge: columna debe ser string", 0);

        const DataFrame& left = args[0]->df_val;
        const DataFrame& right = args[1]->df_val;
        const std::string& key_col = args[2]->str_val;

        auto left_it = left.columnas.find(key_col);
        auto right_it = right.columnas.find(key_col);
        if (left_it == left.columnas.end())
            return mk_err("runtime", "merge: columna '" + key_col + "' no encontrada en primer DataFrame", 0);
        if (right_it == right.columnas.end())
            return mk_err("runtime", "merge: columna '" + key_col + "' no encontrada en segundo DataFrame", 0);

        std::map<std::string, std::vector<size_t>> right_index;
        for (size_t i = 0; i < right.filas(); ++i) {
            std::string k;
            if (right_it->second.tipo == "num") {
                if (!right_it->second.null_bitmap[i]) k = std::to_string(right_it->second.nums[i]);
            } else if (right_it->second.tipo == "str") {
                k = right_it->second.strs[i];
            } else if (right_it->second.tipo == "bool") {
                k = right_it->second.bools[i] ? "true" : "false";
            }
            right_index[k].push_back(i);
        }

        DataFrame result;
        for (const auto& col_name : left.nombres_columnas) {
            result.nombres_columnas.push_back(col_name);
            result.columnas[col_name] = Columna(left.columnas.at(col_name).tipo);
        }
        for (const auto& col_name : right.nombres_columnas) {
            if (col_name != key_col) {
                result.nombres_columnas.push_back(col_name);
                result.columnas[col_name] = Columna(right.columnas.at(col_name).tipo);
            }
        }

        for (size_t i = 0; i < left.filas(); ++i) {
            std::string k;
            if (left_it->second.tipo == "num") {
                if (!left_it->second.null_bitmap[i]) k = std::to_string(left_it->second.nums[i]);
                else continue;
            } else if (left_it->second.tipo == "str") {
                k = left_it->second.strs[i];
            } else if (left_it->second.tipo == "bool") {
                k = left_it->second.bools[i] ? "true" : "false";
            }

            auto match_it = right_index.find(k);
            if (match_it != right_index.end()) {
                for (size_t j : match_it->second) {
                    for (const auto& col_name : left.nombres_columnas) {
                        const auto& col = left.columnas.at(col_name);
                        if (col.tipo == "num") {
                            result.columnas[col_name].nums.push_back(col.nums[i]);
                            result.columnas[col_name].null_bitmap.push_back(col.null_bitmap[i]);
                        } else if (col.tipo == "str") {
                            result.columnas[col_name].strs.push_back(col.strs[i]);
                            result.columnas[col_name].null_bitmap.push_back(col.null_bitmap[i]);
                        } else if (col.tipo == "bool") {
                            result.columnas[col_name].bools.push_back(col.bools[i]);
                            result.columnas[col_name].null_bitmap.push_back(col.null_bitmap[i]);
                        }
                    }
                    for (const auto& col_name : right.nombres_columnas) {
                        if (col_name == key_col) continue;
                        const auto& col = right.columnas.at(col_name);
                        if (col.tipo == "num") {
                            result.columnas[col_name].nums.push_back(col.nums[j]);
                            result.columnas[col_name].null_bitmap.push_back(col.null_bitmap[j]);
                        } else if (col.tipo == "str") {
                            result.columnas[col_name].strs.push_back(col.strs[j]);
                            result.columnas[col_name].null_bitmap.push_back(col.null_bitmap[j]);
                        } else if (col.tipo == "bool") {
                            result.columnas[col_name].bools.push_back(col.bools[j]);
                            result.columnas[col_name].null_bitmap.push_back(col.null_bitmap[j]);
                        }
                    }
                }
            }
        }

        return mk_df(std::move(result));
    }

    // ============================================================
    // EXPLORACIÓN DE DATOS
    // ============================================================

    if (nombre == "info") {
        if (args.empty() || args[0]->tipo != ValorImpl::DF)
            return mk_err("runtime", "info requiere un DataFrame", 0);
        
        const auto& df = args[0]->df_val;
        size_t filas = df.filas();
        size_t cols = df.columnas_count();
        
        std::ostringstream ss;
        ss << "DataFrame: " << filas << " filas x " << cols << " columnas\n\n";
        ss << std::left << std::setw(14) << "Columna" 
           << std::setw(8) << "Tipo" 
           << std::setw(10) << "No-Null" 
           << std::setw(8) << "Null" 
           << "%Null\n";
        ss << std::string(50, '-') << "\n";
        
        size_t total_nums = 0, total_strs = 0, total_bools = 0;
        
        for (const auto& col_name : df.nombres_columnas) {
            auto it = df.columnas.find(col_name);
            if (it == df.columnas.end()) continue;
            const auto& col = it->second;
            
            size_t total = col.size();
            size_t null_count = 0;
            for (bool b : col.null_bitmap) if (b) null_count++;
            size_t non_null = total - null_count;
            
            double pct = total > 0 ? (null_count * 100.0 / total) : 0;
            
            ss << std::left << std::setw(14) << col_name
               << std::setw(8) << col.tipo
               << std::setw(10) << non_null
               << std::setw(8) << null_count;
            ss << std::fixed << std::setprecision(1) << pct << "%\n";
            
            if (col.tipo == "num") total_nums++;
            else if (col.tipo == "str") total_strs++;
            else if (col.tipo == "bool") total_bools++;
        }
        
        ss << "\nTipos: num(" << total_nums << "), str(" << total_strs 
           << "), bool(" << total_bools << ")\n";
        
        return mk_str(ss.str());
    }

    if (nombre == "describe") {
        if (args.empty() || args[0]->tipo != ValorImpl::DF)
            return mk_err("runtime", "describe requiere un DataFrame", 0);
        
        const auto& df = args[0]->df_val;
        std::vector<std::string> num_cols;
        
        for (const auto& col_name : df.nombres_columnas) {
            auto it = df.columnas.find(col_name);
            if (it != df.columnas.end() && it->second.tipo == "num")
                num_cols.push_back(col_name);
        }
        
        if (num_cols.empty())
            return mk_str("No hay columnas numericas\n");
        
        std::ostringstream ss;
        ss << "Estadisticas: " << num_cols.size() << " columnas numericas\n\n";
        
        std::map<std::string, std::vector<double>> stats;
        
        for (const auto& col_name : num_cols) {
            const auto& col = df.columnas.at(col_name);
            std::vector<double> clean;
            for (size_t i = 0; i < col.nums.size(); ++i)
                if (!col.null_bitmap[i]) clean.push_back(col.nums[i]);
            
            double count_val = static_cast<double>(clean.size());
            double mean_val = fn_mean(clean);
            double stddev_val = fn_stddev(clean);
            double min_val = clean.empty() ? crear_null() : clean.front();
            double max_val = clean.empty() ? crear_null() : clean.back();
            double p25 = fn_percentile(clean, 25);
            double p50 = fn_percentile(clean, 50);
            double p75 = fn_percentile(clean, 75);
            
            stats[col_name] = {count_val, mean_val, stddev_val, min_val, p25, p50, p75, max_val};
        }
        
        std::vector<std::string> labels = {"count", "mean", "std", "min", "25%", "50%", "75%", "max"};
        
        ss << std::right << std::setw(8) << "";
        for (const auto& col : num_cols) ss << std::setw(12) << col;
        ss << "\n" << std::string(8 + 12*num_cols.size(), '-') << "\n";
        
        for (size_t row = 0; row < labels.size(); ++row) {
            ss << std::right << std::setw(8) << labels[row];
            for (const auto& col : num_cols) {
                if (es_null(stats[col][row])) ss << std::setw(12) << "null";
                else ss << std::setw(12) << std::fixed << std::setprecision(1) << stats[col][row];
            }
            ss << "\n";
        }
        
        return mk_str(ss.str());
    }

    if (nombre == "tail") {
        if (args.empty()) return mk_err("runtime", "tail requiere un DataFrame o vector", 0);
        int n = 5;
        if (args.size() >= 2 && args[1]->tipo == ValorImpl::NUM) n = static_cast<int>(args[1]->num_val);
        
        if (args[0]->tipo == ValorImpl::DF) {
            const auto& df = args[0]->df_val;
            int total = static_cast<int>(df.filas());
            int start = std::max(0, total - n);
            
            DataFrame result;
            result.nombres_columnas = df.nombres_columnas;
            for (const auto& [col_name, col] : df.columnas) {
                Columna sliced(col.tipo);
                for (int i = start; i < total; ++i) {
                    if (col.tipo == "num") {
                        sliced.nums.push_back(col.nums[i]);
                        sliced.null_bitmap.push_back(col.null_bitmap[i]);
                    } else if (col.tipo == "str") {
                        sliced.strs.push_back(col.strs[i]);
                        sliced.null_bitmap.push_back(col.null_bitmap[i]);
                    } else if (col.tipo == "bool") {
                        sliced.bools.push_back(col.bools[i]);
                        sliced.null_bitmap.push_back(col.null_bitmap[i]);
                    }
                }
                result.columnas[col_name] = std::move(sliced);
            }
            return mk_df(result);
        }
        if (args[0]->tipo == ValorImpl::VEC) {
            const auto& v = args[0]->vec_val;
            int start = std::max(0, static_cast<int>(v.size()) - n);
            std::vector<double> sliced(v.begin() + start, v.end());
            return mk_vec(sliced);
        }
        if (args[0]->tipo == ValorImpl::STR_VEC) {
            const auto& v = args[0]->str_vec_val;
            int start = std::max(0, static_cast<int>(v.size()) - n);
            std::vector<std::string> sliced(v.begin() + start, v.end());
            return mk_str_vec(sliced);
        }
        return mk_err("runtime", "tail requiere DataFrame o vector", 0);
    }

    if (nombre == "sample") {
        if (args.empty() || args[0]->tipo != ValorImpl::DF)
            return mk_err("runtime", "sample requiere un DataFrame", 0);
        int n = 5;
        if (args.size() >= 2 && args[1]->tipo == ValorImpl::NUM) n = static_cast<int>(args[1]->num_val);
        
        const auto& df = args[0]->df_val;
        int total = static_cast<int>(df.filas());
        n = std::min(n, total);
        
        std::vector<int> indices(total);
        std::iota(indices.begin(), indices.end(), 0);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(indices.begin(), indices.end(), gen);
        indices.resize(n);
        std::sort(indices.begin(), indices.end());
        
        DataFrame result;
        result.nombres_columnas = df.nombres_columnas;
        for (const auto& [col_name, col] : df.columnas) {
            Columna sliced(col.tipo);
            for (int i : indices) {
                if (col.tipo == "num") {
                    sliced.nums.push_back(col.nums[i]);
                    sliced.null_bitmap.push_back(col.null_bitmap[i]);
                } else if (col.tipo == "str") {
                    sliced.strs.push_back(col.strs[i]);
                    sliced.null_bitmap.push_back(col.null_bitmap[i]);
                } else if (col.tipo == "bool") {
                    sliced.bools.push_back(col.bools[i]);
                    sliced.null_bitmap.push_back(col.null_bitmap[i]);
                }
            }
            result.columnas[col_name] = std::move(sliced);
        }
        return mk_df(result);
    }

    if (nombre == "value_counts") {
        if (args.empty()) return mk_err("runtime", "value_counts requiere un vector", 0);
        
        if (args[0]->tipo == ValorImpl::VEC) {
            std::map<double, size_t> freq;
            for (double v : args[0]->vec_val) if (!es_null(v)) freq[v]++;
            std::vector<std::pair<double, size_t>> sorted(freq.begin(), freq.end());
            std::sort(sorted.begin(), sorted.end(),
                [](const std::pair<double, size_t>& a, const std::pair<double, size_t>& b) { return a.second > b.second; });
            
            DataFrame result;
            result.nombres_columnas = {"valor", "count"};
            result.columnas["valor"] = Columna("num");
            result.columnas["count"] = Columna("num");
            for (auto& [v, c] : sorted) {
                result.columnas["valor"].nums.push_back(v);
                result.columnas["valor"].null_bitmap.push_back(false);
                result.columnas["count"].nums.push_back(static_cast<double>(c));
                result.columnas["count"].null_bitmap.push_back(false);
            }
            return mk_df(result);
        }
        if (args[0]->tipo == ValorImpl::STR_VEC) {
            std::map<std::string, size_t> freq;
            for (const auto& s : args[0]->str_vec_val) freq[s]++;
            std::vector<std::pair<std::string, size_t>> sorted(freq.begin(), freq.end());
            std::sort(sorted.begin(), sorted.end(),
                [](const std::pair<std::string, size_t>& a, const std::pair<std::string, size_t>& b) { return a.second > b.second; });
            
            DataFrame result;
            result.nombres_columnas = {"valor", "count"};
            result.columnas["valor"] = Columna("str");
            result.columnas["count"] = Columna("num");
            for (auto& [v, c] : sorted) {
                result.columnas["valor"].strs.push_back(v);
                result.columnas["valor"].null_bitmap.push_back(false);
                result.columnas["count"].nums.push_back(static_cast<double>(c));
                result.columnas["count"].null_bitmap.push_back(false);
            }
            return mk_df(result);
        }
        return mk_err("runtime", "value_counts requiere vector o str_vec", 0);
    }

    if (nombre == "nunique") {
        if (args.empty()) return mk_err("runtime", "nunique requiere un vector", 0);
        
        if (args[0]->tipo == ValorImpl::VEC) {
            std::set<double> unique;
            for (double v : args[0]->vec_val) if (!es_null(v)) unique.insert(v);
            return mk_num(static_cast<double>(unique.size()));
        }
        if (args[0]->tipo == ValorImpl::STR_VEC) {
            std::set<std::string> unique(args[0]->str_vec_val.begin(), args[0]->str_vec_val.end());
            return mk_num(static_cast<double>(unique.size()));
        }
        return mk_err("runtime", "nunique requiere vector o str_vec", 0);
    }

    if (nombre == "median") {
        if (args.empty() || args[0]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "median requiere un vector", 0);
        return mk_num(fn_median(args[0]->vec_val));
    }

    if (nombre == "percentile") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::NUM)
            return mk_err("runtime", "percentile requiere vector y numero (0-100)", 0);
        return mk_num(fn_percentile(args[0]->vec_val, args[1]->num_val));
    }

    if (nombre == "mode") {
        if (args.empty() || args[0]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "mode requiere un vector", 0);
        return mk_num(fn_mode(args[0]->vec_val));
    }

    if (nombre == "cor") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "cor requiere dos vectores", 0);
        return mk_num(fn_cor(args[0]->vec_val, args[1]->vec_val));
    }

    if (nombre == "cov") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "cov requiere dos vectores", 0);
        return mk_num(fn_cov(args[0]->vec_val, args[1]->vec_val));
    }

    if (nombre == "isna") {
        if (args.empty()) return mk_err("runtime", "isna requiere un DataFrame o vector", 0);
        
        if (args[0]->tipo == ValorImpl::DF) {
            const auto& df = args[0]->df_val;
            DataFrame result;
            result.nombres_columnas = df.nombres_columnas;
            for (const auto& [col_name, col] : df.columnas) {
                Columna bool_col("bool");
                for (size_t i = 0; i < col.null_bitmap.size(); ++i) {
                    bool_col.bools.push_back(col.null_bitmap[i]);
                    bool_col.null_bitmap.push_back(false);
                }
                result.columnas[col_name] = std::move(bool_col);
            }
            return mk_df(result);
        }
        if (args[0]->tipo == ValorImpl::VEC) {
            std::vector<bool> flags;
            for (double v : args[0]->vec_val) flags.push_back(es_null(v));
            return mk_bool_vec(flags);
        }
        return mk_err("runtime", "isna requiere DataFrame o vector", 0);
    }

    if (nombre == "duplicated") {
        if (args.empty()) return mk_err("runtime", "duplicated requiere un vector", 0);
        
        if (args[0]->tipo == ValorImpl::VEC) {
            std::set<double> seen;
            std::vector<bool> result;
            for (double v : args[0]->vec_val) {
                if (es_null(v)) { result.push_back(false); continue; }
                bool dup = seen.count(v) > 0;
                seen.insert(v);
                result.push_back(dup);
            }
            return mk_bool_vec(result);
        }
        if (args[0]->tipo == ValorImpl::STR_VEC) {
            std::set<std::string> seen;
            std::vector<bool> result;
            for (const auto& s : args[0]->str_vec_val) {
                bool dup = seen.count(s) > 0;
                seen.insert(s);
                result.push_back(dup);
            }
            return mk_bool_vec(result);
        }
        return mk_err("runtime", "duplicated requiere vector o str_vec", 0);
    }

    if (nombre == "cut") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::NUM)
            return mk_err("runtime", "cut requiere vector y numero de bins", 0);
        
        const auto& vec = args[0]->vec_val;
        int bins = static_cast<int>(args[1]->num_val);
        if (bins <= 0) return mk_err("runtime", "cut: bins debe ser > 0", 0);
        
        double min_val = fn_min(vec);
        double max_val = fn_max(vec);
        if (es_null(min_val) || es_null(max_val))
            return mk_err("runtime", "cut: vector vacio o todo null", 0);
        
        double range = max_val - min_val;
        if (range == 0) range = 1.0;
        double step = range / bins;
        
        std::vector<double> result;
        for (double v : vec) {
            if (es_null(v)) { result.push_back(crear_null()); continue; }
            int bin = static_cast<int>((v - min_val) / step);
            if (bin >= bins) bin = bins - 1;
            result.push_back(static_cast<double>(bin + 1));
        }
        return mk_vec(result);
    }

    if (nombre == "qcut") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::NUM)
            return mk_err("runtime", "qcut requiere vector y numero de cuantiles", 0);
        
        const auto& vec = args[0]->vec_val;
        int q = static_cast<int>(args[1]->num_val);
        if (q <= 0) return mk_err("runtime", "qcut: q debe ser > 0", 0);
        
        std::vector<double> clean;
        for (double v : vec) if (!es_null(v)) clean.push_back(v);
        if (clean.empty()) return mk_err("runtime", "qcut: vector vacio", 0);
        
        std::sort(clean.begin(), clean.end());
        
        std::vector<double> cuts;
        for (int i = 1; i < q; ++i) {
            double pos = (static_cast<double>(i) / q) * (clean.size() - 1);
            size_t lo = static_cast<size_t>(pos);
            size_t hi = lo + 1;
            if (hi >= clean.size()) cuts.push_back(clean.back());
            else {
                double frac = pos - lo;
                cuts.push_back(clean[lo] + frac * (clean[hi] - clean[lo]));
            }
        }
        
        std::vector<double> result;
        for (double v : vec) {
            if (es_null(v)) { result.push_back(crear_null()); continue; }
            int bin = 0;
            for (double c : cuts) {
                if (v > c) bin++;
                else break;
            }
            result.push_back(static_cast<double>(bin + 1));
        }
        return mk_vec(result);
    }

    // ============================================================
    // DISTRIBUCIONES Y TESTING ESTADÍSTICO
    // ============================================================

    if (nombre == "dnorm") {
        if (args.empty()) return mk_err("runtime", "dnorm requiere al menos un argumento", 0);
        double x = args[0]->num_val;
        double mean = (args.size() >= 2) ? args[1]->num_val : 0.0;
        double sd = (args.size() >= 3) ? args[2]->num_val : 1.0;
        return mk_num(dnorm(x, mean, sd));
    }

    if (nombre == "pnorm") {
        if (args.empty()) return mk_err("runtime", "pnorm requiere al menos un argumento", 0);
        double x = args[0]->num_val;
        double mean = (args.size() >= 2) ? args[1]->num_val : 0.0;
        double sd = (args.size() >= 3) ? args[2]->num_val : 1.0;
        return mk_num(pnorm(x, mean, sd));
    }

    if (nombre == "qnorm") {
        if (args.empty()) return mk_err("runtime", "qnorm requiere al menos un argumento", 0);
        double p = args[0]->num_val;
        double mean = (args.size() >= 2) ? args[1]->num_val : 0.0;
        double sd = (args.size() >= 3) ? args[2]->num_val : 1.0;
        return mk_num(qnorm(p, mean, sd));
    }

    if (nombre == "dgamma") {
        if (args.size() < 3) return mk_err("runtime", "dgamma requiere x, shape, rate", 0);
        return mk_num(dgamma(args[0]->num_val, args[1]->num_val, args[2]->num_val));
    }

    if (nombre == "dbeta") {
        if (args.size() < 3) return mk_err("runtime", "dbeta requiere x, alpha, beta", 0);
        return mk_num(dbeta(args[0]->num_val, args[1]->num_val, args[2]->num_val));
    }

    if (nombre == "dunif") {
        if (args.empty()) return mk_err("runtime", "dunif requiere al menos un argumento", 0);
        double x = args[0]->num_val;
        double min_val = (args.size() >= 2) ? args[1]->num_val : 0.0;
        double max_val = (args.size() >= 3) ? args[2]->num_val : 1.0;
        return mk_num(dunif(x, min_val, max_val));
    }

    if (nombre == "dt_dist") {
        if (args.size() < 2) return mk_err("runtime", "dt_dist requiere x y df", 0);
        return mk_num(dt_dist(args[0]->num_val, args[1]->num_val));
    }

    if (nombre == "df_dist") {
        if (args.size() < 3) return mk_err("runtime", "df_dist requiere x, df1, df2", 0);
        return mk_num(df_dist(args[0]->num_val, args[1]->num_val, args[2]->num_val));
    }

    if (nombre == "dchisq") {
        if (args.size() < 2) return mk_err("runtime", "dchisq requiere x y df", 0);
        return mk_num(dchisq(args[0]->num_val, args[1]->num_val));
    }

    if (nombre == "t_test") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "t_test requiere dos vectores", 0);
        double t_stat = t_test_statistic(args[0]->vec_val, args[1]->vec_val);
        double df = args[0]->vec_val.size() + args[1]->vec_val.size() - 2;
        double p_val = t_test_pvalue(t_stat, df);
        
        std::map<std::string, ValorZeta> result;
        result["t_statistic"] = mk_num(t_stat);
        result["df"] = mk_num(df);
        result["p_value"] = mk_num(p_val);
        result["significant"] = mk_num(p_val < 0.05 ? 1.0 : 0.0);
        return mk_dict(std::move(result));
    }

    if (nombre == "anova") {
        if (args.empty() || args[0]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "anova requiere vector de vectores", 0);
        
        // Expect vector of vectors (from group_by)
        std::vector<std::vector<double>> groups;
        for (const auto& v : args[0]->vec_val) {
            // Each element should be a vector
            // For simplicity, treat input as flat and split by groups
        }
        
        // Alternative: accept multiple vectors as arguments
        for (size_t i = 0; i < args.size(); ++i) {
            if (args[i]->tipo == ValorImpl::VEC) {
                groups.push_back(args[i]->vec_val);
            }
        }
        
        if (groups.size() < 2)
            return mk_err("runtime", "anova requiere al menos 2 grupos", 0);
        
        double f_stat = anova_f_statistic(groups);
        
        std::map<std::string, ValorZeta> result;
        result["f_statistic"] = mk_num(f_stat);
        result["groups"] = mk_num(static_cast<double>(groups.size()));
        return mk_dict(std::move(result));
    }

    if (nombre == "chi_square") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "chi_square requiere dos vectores (observed, expected)", 0);
        double chi2 = chi_square_statistic(args[0]->vec_val, args[1]->vec_val);
        double df = static_cast<double>(args[0]->vec_val.size() - 1);
        
        std::map<std::string, ValorZeta> result;
        result["chi_square"] = mk_num(chi2);
        result["df"] = mk_num(df);
        return mk_dict(std::move(result));
    }

    if (nombre == "linear_regression") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "linear_regression requiere dos vectores (x, y)", 0);
        
        RegressionResult reg = linear_regression(args[0]->vec_val, args[1]->vec_val);
        
        std::map<std::string, ValorZeta> result;
        result["slope"] = mk_num(reg.slope);
        result["intercept"] = mk_num(reg.intercept);
        result["r_squared"] = mk_num(reg.r_squared);
        result["std_error"] = mk_num(reg.std_error);
        result["predicted"] = mk_vec(reg.predicted);
        
        // Formula string
        std::ostringstream ss;
        ss << "y = " << reg.slope << " * x + " << reg.intercept;
        result["formula"] = mk_str(ss.str());
        
        return mk_dict(std::move(result));
    }

    // ============================================================
    // WINDOW FUNCTIONS
    // ============================================================

    if (nombre == "cumsum") {
        if (args.empty() || args[0]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "cumsum requiere un vector", 0);
        return mk_vec(fn_cumsum(args[0]->vec_val));
    }

    if (nombre == "cummax") {
        if (args.empty() || args[0]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "cummax requiere un vector", 0);
        return mk_vec(fn_cummax(args[0]->vec_val));
    }

    if (nombre == "cummin") {
        if (args.empty() || args[0]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "cummin requiere un vector", 0);
        return mk_vec(fn_cummin(args[0]->vec_val));
    }

    if (nombre == "rolling_mean") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::NUM)
            return mk_err("runtime", "rolling_mean requiere vector y ventana", 0);
        size_t window = static_cast<size_t>(args[1]->num_val);
        return mk_vec(fn_rolling_mean(args[0]->vec_val, window));
    }

    if (nombre == "rolling_std") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::NUM)
            return mk_err("runtime", "rolling_std requiere vector y ventana", 0);
        size_t window = static_cast<size_t>(args[1]->num_val);
        return mk_vec(fn_rolling_std(args[0]->vec_val, window));
    }

    if (nombre == "rolling_sum") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::NUM)
            return mk_err("runtime", "rolling_sum requiere vector y ventana", 0);
        size_t window = static_cast<size_t>(args[1]->num_val);
        return mk_vec(fn_rolling_sum(args[0]->vec_val, window));
    }

    if (nombre == "rolling_min") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::NUM)
            return mk_err("runtime", "rolling_min requiere vector y ventana", 0);
        size_t window = static_cast<size_t>(args[1]->num_val);
        return mk_vec(fn_rolling_min(args[0]->vec_val, window));
    }

    if (nombre == "rolling_max") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::NUM)
            return mk_err("runtime", "rolling_max requiere vector y ventana", 0);
        size_t window = static_cast<size_t>(args[1]->num_val);
        return mk_vec(fn_rolling_max(args[0]->vec_val, window));
    }

    if (nombre == "lag") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::NUM)
            return mk_err("runtime", "lag requiere vector y n", 0);
        size_t n = static_cast<size_t>(args[1]->num_val);
        return mk_vec(fn_lag(args[0]->vec_val, n));
    }

    if (nombre == "lead") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::NUM)
            return mk_err("runtime", "lead requiere vector y n", 0);
        size_t n = static_cast<size_t>(args[1]->num_val);
        return mk_vec(fn_lead(args[0]->vec_val, n));
    }

    if (nombre == "diff") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::NUM)
            return mk_err("runtime", "diff requiere vector y n", 0);
        size_t n = static_cast<size_t>(args[1]->num_val);
        return mk_vec(fn_diff(args[0]->vec_val, n));
    }

    if (nombre == "row_number") {
        if (args.empty() || args[0]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "row_number requiere un vector", 0);
        return mk_vec(fn_row_number(args[0]->vec_val));
    }

    if (nombre == "rank") {
        if (args.empty() || args[0]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "rank requiere un vector", 0);
        return mk_vec(fn_rank(args[0]->vec_val));
    }

    if (nombre == "pct_change") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::NUM)
            return mk_err("runtime", "pct_change requiere vector y n", 0);
        size_t n = static_cast<size_t>(args[1]->num_val);
        return mk_vec(fn_pct_change(args[0]->vec_val, n));
    }

    // ============================================================
    // DATA CLEANING
    // ============================================================

    if (nombre == "drop_duplicates") {
        if (args.empty() || args[0]->tipo != ValorImpl::DF)
            return mk_err("runtime", "drop_duplicates requiere un DataFrame", 0);
        
        const auto& df = args[0]->df_val;
        DataFrame result;
        result.nombres_columnas = df.nombres_columnas;
        for (const auto& [col_name, col] : df.columnas) {
            result.columnas[col_name] = Columna(col.tipo);
        }
        
        // Track seen row signatures
        std::set<std::string> seen;
        
        for (size_t row = 0; row < df.filas(); ++row) {
            // Create row signature
            std::string sig;
            for (const auto& col_name : df.nombres_columnas) {
                auto it = df.columnas.find(col_name);
                if (it == df.columnas.end()) continue;
                const auto& col = it->second;
                if (col.null_bitmap[row]) { sig += "N|"; continue; }
                if (col.tipo == "num") sig += std::to_string(col.nums[row]) + "|";
                else if (col.tipo == "str") sig += col.strs[row] + "|";
                else if (col.tipo == "bool") sig += (col.bools[row] ? "T" : "F") + std::string("|");
            }
            
            if (seen.count(sig) == 0) {
                seen.insert(sig);
                for (const auto& col_name : df.nombres_columnas) {
                    auto it = df.columnas.find(col_name);
                    if (it == df.columnas.end()) continue;
                    const auto& col = it->second;
                    if (col.tipo == "num") {
                        result.columnas[col_name].nums.push_back(col.nums[row]);
                        result.columnas[col_name].null_bitmap.push_back(col.null_bitmap[row]);
                    } else if (col.tipo == "str") {
                        result.columnas[col_name].strs.push_back(col.strs[row]);
                        result.columnas[col_name].null_bitmap.push_back(col.null_bitmap[row]);
                    } else if (col.tipo == "bool") {
                        result.columnas[col_name].bools.push_back(col.bools[row]);
                        result.columnas[col_name].null_bitmap.push_back(col.null_bitmap[row]);
                    }
                }
            }
        }
        
        return mk_df(result);
    }

    if (nombre == "rename") {
        if (args.size() < 3 || args[0]->tipo != ValorImpl::DF || 
            args[1]->tipo != ValorImpl::STR || args[2]->tipo != ValorImpl::STR)
            return mk_err("runtime", "rename requiere DataFrame, nombre viejo, nombre nuevo", 0);
        
        const auto& df = args[0]->df_val;
        const std::string& old_name = args[1]->str_val;
        const std::string& new_name = args[2]->str_val;
        
        auto it = df.columnas.find(old_name);
        if (it == df.columnas.end())
            return mk_err("runtime", "rename: columna '" + old_name + "' no encontrada", 0);
        
        DataFrame result;
        for (const auto& col_name : df.nombres_columnas) {
            if (col_name == old_name) {
                result.nombres_columnas.push_back(new_name);
                result.columnas[new_name] = it->second;
            } else {
                result.nombres_columnas.push_back(col_name);
                result.columnas[col_name] = df.columnas.at(col_name);
            }
        }
        
        return mk_df(result);
    }

    if (nombre == "select_cols") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::DF || args[1]->tipo != ValorImpl::STR_VEC)
            return mk_err("runtime", "select_cols requiere DataFrame y vector de columnas", 0);
        
        const auto& df = args[0]->df_val;
        const auto& cols = args[1]->str_vec_val;
        
        DataFrame result;
        for (const auto& col_name : cols) {
            auto it = df.columnas.find(col_name);
            if (it == df.columnas.end())
                return mk_err("runtime", "select_cols: columna '" + col_name + "' no encontrada", 0);
            result.nombres_columnas.push_back(col_name);
            result.columnas[col_name] = it->second;
        }
        
        return mk_df(result);
    }

    if (nombre == "drop_cols") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::DF || args[1]->tipo != ValorImpl::STR_VEC)
            return mk_err("runtime", "drop_cols requiere DataFrame y vector de columnas", 0);
        
        const auto& df = args[0]->df_val;
        const auto& cols_to_drop = args[1]->str_vec_val;
        std::set<std::string> drop_set(cols_to_drop.begin(), cols_to_drop.end());
        
        DataFrame result;
        for (const auto& col_name : df.nombres_columnas) {
            if (drop_set.count(col_name) == 0) {
                result.nombres_columnas.push_back(col_name);
                result.columnas[col_name] = df.columnas.at(col_name);
            }
        }
        
        return mk_df(result);
    }

    if (nombre == "fillna") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "fillna requiere vector y estrategia", 0);
        
        const auto& vec = args[0]->vec_val;
        std::string strategy = (args[1]->tipo == ValorImpl::STR) ? args[1]->str_val : "value";
        
        std::vector<double> result = vec;
        
        if (strategy == "mean") {
            double mean_val = fn_mean(vec);
            for (auto& v : result) if (es_null(v)) v = mean_val;
        } else if (strategy == "median") {
            double median_val = fn_median(vec);
            for (auto& v : result) if (es_null(v)) v = median_val;
        } else if (strategy == "mode") {
            double mode_val = fn_mode(vec);
            for (auto& v : result) if (es_null(v)) v = mode_val;
        } else if (strategy == "ffill") {
            double last_valid = crear_null();
            for (size_t i = 0; i < result.size(); ++i) {
                if (!es_null(result[i])) last_valid = result[i];
                else if (!es_null(last_valid)) result[i] = last_valid;
            }
        } else if (strategy == "bfill") {
            double next_valid = crear_null();
            for (int i = static_cast<int>(result.size()) - 1; i >= 0; --i) {
                if (!es_null(result[i])) next_valid = result[i];
                else if (!es_null(next_valid)) result[i] = next_valid;
            }
        } else if (strategy == "zero") {
            for (auto& v : result) if (es_null(v)) v = 0.0;
        } else {
            // Use as literal value
            double fill_val = args[1]->num_val;
            for (auto& v : result) if (es_null(v)) v = fill_val;
        }
        
        return mk_vec(result);
    }

    if (nombre == "replace_val") {
        if (args.size() < 3 || args[0]->tipo != ValorImpl::DF || 
            args[1]->tipo != ValorImpl::STR || args[2]->tipo != ValorImpl::STR)
            return mk_err("runtime", "replace_val requiere DataFrame, columna, old, new", 0);
        
        const auto& df = args[0]->df_val;
        const std::string& col_name = args[1]->str_val;
        
        auto it = df.columnas.find(col_name);
        if (it == df.columnas.end())
            return mk_err("runtime", "replace_val: columna '" + col_name + "' no encontrada", 0);
        
        std::string old_val = (args.size() > 2 && args[2]->tipo == ValorImpl::STR) ? args[2]->str_val : "";
        std::string new_val = (args.size() > 3 && args[3]->tipo == ValorImpl::STR) ? args[3]->str_val : "";
        
        DataFrame result;
        result.nombres_columnas = df.nombres_columnas;
        
        for (const auto& cn : df.nombres_columnas) {
            const auto& col = df.columnas.at(cn);
            Columna new_col(col.tipo);
            
            if (cn == col_name && col.tipo == "str") {
                for (size_t i = 0; i < col.size(); ++i) {
                    if (!col.null_bitmap[i] && col.strs[i] == old_val) {
                        new_col.strs.push_back(new_val);
                    } else {
                        new_col.strs.push_back(col.null_bitmap[i] ? "" : col.strs[i]);
                    }
                    new_col.null_bitmap.push_back(col.null_bitmap[i]);
                }
            } else {
                if (col.tipo == "num") { new_col.nums = col.nums; }
                else if (col.tipo == "str") { new_col.strs = col.strs; }
                else if (col.tipo == "bool") { new_col.bools = col.bools; }
                new_col.null_bitmap = col.null_bitmap;
            }
            
            result.columnas[cn] = std::move(new_col);
        }
        
        return mk_df(result);
    }

    if (nombre == "clip") {
        if (args.size() < 3 || args[0]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "clip requiere vector, min y max", 0);
        
        const auto& vec = args[0]->vec_val;
        double min_val = args[1]->num_val;
        double max_val = args[2]->num_val;
        
        std::vector<double> result;
        result.reserve(vec.size());
        for (double v : vec) {
            if (es_null(v)) { result.push_back(crear_null()); continue; }
            if (v < min_val) result.push_back(min_val);
            else if (v > max_val) result.push_back(max_val);
            else result.push_back(v);
        }
        
        return mk_vec(result);
    }

    if (nombre == "trim") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC || args[1]->tipo != ValorImpl::NUM)
            return mk_err("runtime", "trim requiere vector y número de std devs", 0);
        
        const auto& vec = args[0]->vec_val;
        double n_std = args[1]->num_val;
        
        double mean_val = fn_mean(vec);
        double std_val = fn_stddev(vec);
        
        if (es_null(mean_val) || es_null(std_val) || std_val == 0)
            return mk_vec(vec);
        
        double lower = mean_val - n_std * std_val;
        double upper = mean_val + n_std * std_val;
        
        std::vector<double> result;
        result.reserve(vec.size());
        for (double v : vec) {
            if (es_null(v)) { result.push_back(crear_null()); continue; }
            if (v < lower || v > upper) result.push_back(crear_null());
            else result.push_back(v);
        }
        
        return mk_vec(result);
    }

    if (nombre == "normalize") {
        if (args.empty() || args[0]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "normalize requiere un vector", 0);
        
        const auto& vec = args[0]->vec_val;
        double min_val = fn_min(vec);
        double max_val = fn_max(vec);
        
        if (es_null(min_val) || es_null(max_val) || max_val == min_val)
            return mk_vec(vec);
        
        double range = max_val - min_val;
        std::vector<double> result;
        result.reserve(vec.size());
        for (double v : vec) {
            if (es_null(v)) { result.push_back(crear_null()); continue; }
            result.push_back((v - min_val) / range);
        }
        
        return mk_vec(result);
    }

    if (nombre == "standardize") {
        if (args.empty() || args[0]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "standardize requiere un vector", 0);
        
        const auto& vec = args[0]->vec_val;
        double mean_val = fn_mean(vec);
        double std_val = fn_stddev(vec);
        
        if (es_null(mean_val) || es_null(std_val) || std_val == 0)
            return mk_vec(vec);
        
        std::vector<double> result;
        result.reserve(vec.size());
        for (double v : vec) {
            if (es_null(v)) { result.push_back(crear_null()); continue; }
            result.push_back((v - mean_val) / std_val);
        }
        
        return mk_vec(result);
    }

    if (nombre == "split") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::STR || args[1]->tipo != ValorImpl::STR)
            return mk_err("runtime", "split requiere string y separador", 0);
        std::vector<std::string> parts;
        std::string s = args[0]->str_val, sep = args[1]->str_val;
        size_t pos = 0;
        while ((pos = s.find(sep)) != std::string::npos) {
            parts.push_back(s.substr(0, pos));
            s.erase(0, pos + sep.length());
        }
        parts.push_back(s);
        return mk_str_vec(parts);
    }

    if (nombre == "join") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::STR_VEC || args[1]->tipo != ValorImpl::STR)
            return mk_err("runtime", "join requiere vector<string> y separador", 0);
        std::string result;
        const auto& v = args[0]->str_vec_val;
        const std::string& sep = args[1]->str_val;
        for (size_t i = 0; i < v.size(); ++i) {
            if (i > 0) result += sep;
            result += v[i];
        }
        return mk_str(result);
    }

    if (nombre == "replace") {
        if (args.size() < 3 || args[0]->tipo != ValorImpl::STR || args[1]->tipo != ValorImpl::STR || args[2]->tipo != ValorImpl::STR)
            return mk_err("runtime", "replace requiere string, old, new", 0);
        std::string s = args[0]->str_val, old = args[1]->str_val, new_s = args[2]->str_val;
        size_t pos = 0;
        while ((pos = s.find(old, pos)) != std::string::npos) {
            s.replace(pos, old.length(), new_s);
            pos += new_s.length();
        }
        return mk_str(s);
    }

    if (nombre == "find") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::STR || args[1]->tipo != ValorImpl::STR)
            return mk_err("runtime", "find requiere string y subcadena", 0);
        size_t pos = args[0]->str_val.find(args[1]->str_val);
        return mk_num(pos == std::string::npos ? -1.0 : static_cast<double>(pos));
    }

    if (nombre == "map") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "map requiere vector y funcion", 0);
        const auto& v = args[0]->vec_val;
        std::vector<double> result;
        result.reserve(v.size());
        for (size_t i = 0; i < v.size(); ++i) {
            std::vector<ValorZeta> call_args;
            call_args.push_back(mk_num(v[i]));
            auto r = llamar_usuario(args[1], call_args);
            if (r && r->tipo == ValorImpl::NUM) result.push_back(r->num_val);
            else result.push_back(crear_null());
        }
        return mk_vec(result);
    }

    if (nombre == "filter") {
        if (args.size() < 2 || args[0]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "filter requiere vector y funcion", 0);
        const auto& v = args[0]->vec_val;
        std::vector<double> result;
        for (size_t i = 0; i < v.size(); ++i) {
            std::vector<ValorZeta> call_args;
            call_args.push_back(mk_num(v[i]));
            auto r = llamar_usuario(args[1], call_args);
            if (r && r->tipo == ValorImpl::BOOL && r->bool_val) result.push_back(v[i]);
        }
        return mk_vec(result);
    }

    if (nombre == "reduce") {
        if (args.size() < 3 || args[0]->tipo != ValorImpl::VEC)
            return mk_err("runtime", "reduce requiere vector, funcion, y valor inicial", 0);
        const auto& v = args[0]->vec_val;
        ValorZeta acc = args[2];
        for (size_t i = 0; i < v.size(); ++i) {
            std::vector<ValorZeta> call_args;
            call_args.push_back(acc);
            call_args.push_back(mk_num(v[i]));
            acc = llamar_usuario(args[1], call_args);
        }
        return acc;
    }

    if (nombre == "plot") {
        if (args.empty()) return mk_err("runtime", "plot requiere al menos un vector o DataFrame", 0);

        std::string tipo = "line";
        std::string titulo = "";
        if (args.size() >= 2 && args[1]->tipo == ValorImpl::STR) tipo = args[1]->str_val;
        if (args.size() >= 3 && args[2]->tipo == ValorImpl::STR) titulo = args[2]->str_val;

        if (args[0]->tipo == ValorImpl::VEC) {
            const auto& data = args[0]->vec_val;
            if (data.empty()) return mk_err("runtime", "plot: vector vacio", 0);

            double max_val = data[0], min_val = data[0];
            for (const auto& v : data) {
                if (!es_null(v)) {
                    if (v > max_val) max_val = v;
                    if (v < min_val) min_val = v;
                }
            }
            double range = max_val - min_val;
            if (range == 0) range = 1;

            int height = 10;
            int width = std::min(static_cast<int>(data.size()), 60);

            std::cout << "\n";
            if (!titulo.empty()) std::cout << titulo << "\n";
            std::cout << std::string(50, '-') << "\n";

            for (int row = height; row >= 1; --row) {
                double threshold = min_val + (range * row / height);
                std::cout << std::setw(8) << std::fixed << std::setprecision(1) << threshold << " |";
                for (int i = 0; i < width && i < static_cast<int>(data.size()); ++i) {
                    if (!es_null(data[i])) {
                        int bar_h = static_cast<int>((data[i] - min_val) / range * height);
                        if (bar_h >= row) std::cout << "##";
                        else std::cout << "  ";
                    } else {
                        std::cout << "  ";
                    }
                }
                std::cout << "\n";
            }
            std::cout << std::string(8, ' ') << " +" << std::string(width * 2, '-') << "\n";
            std::cout << "\n";

            GraficoConfig config;
            config.tipo_grafico = tipo;
            config.titulo = titulo;
            config.eje_x = "x";
            config.eje_y = "y";
            return mk_grafico(config);
        }

        if (args[0]->tipo == ValorImpl::DF) {
            const auto& df = args[0]->df_val;
            if (df.columnas.empty()) return mk_err("runtime", "plot: DataFrame vacio", 0);
            std::cout << "\n[Plot DataFrame] columnas: " << df.columnas.size() << ", filas: " << df.filas() << "\n";
            GraficoConfig config;
            config.tipo_grafico = tipo;
            config.titulo = titulo;
            return mk_grafico(config);
        }

        return mk_err("runtime", "plot requiere un vector o DataFrame como datos", 0);
    }

    if (nombre == "metric") {
        if (args.empty()) return mk_err("runtime", "metric requiere nombre y valor", 0);
        if (args[0]->tipo != ValorImpl::STR) return mk_err("runtime", "metric requiere nombre como string", 0);
        if (args.size() < 2 || args[1]->tipo != ValorImpl::NUM) return mk_err("runtime", "metric requiere valor numerico", 0);

        MetricaKPI kpi;
        kpi.nombre = args[0]->str_val;
        kpi.valor = args[1]->num_val;
        metricas_.push_back(kpi);
        return mk_metrica(kpi);
    }

    if (nombre == "dashboard") {
        DashboardConfig config;
        if (args.size() >= 1 && args[0]->tipo == ValorImpl::STR) {
            config.titulo = args[0]->str_val;
        }
        if (args.size() >= 2 && args[1]->tipo == ValorImpl::STR) {
            config.autor = args[1]->str_val;
        }
        return mk_dashboard(config);
    }

    if (nombre == "serve") {
        if (args.empty()) return mk_err("runtime", "serve requiere un dashboard", 0);
        if (args[0]->tipo != ValorImpl::DASHBOARD) return mk_err("runtime", "serve requiere un dashboard valido", 0);

        dashboard_ = args[0]->dashboard_val;
        int puerto = 8080;
        if (args.size() >= 2 && args[1]->tipo == ValorImpl::NUM) {
            puerto = static_cast<int>(args[1]->num_val);
        }

        std::string filename = "dashboard.html";
        if (args.size() >= 3 && args[2]->tipo == ValorImpl::STR) {
            filename = args[2]->str_val;
        }

        std::ofstream html(filename);
        if (!html.is_open()) {
            return mk_err("io", "No se pudo crear archivo: " + filename, 0);
        }

        std::string titulo = dashboard_->titulo.empty() ? "Zeta Dashboard" : dashboard_->titulo;

        html << "<!DOCTYPE html>\n<html><head><meta charset='utf-8'><title>" << titulo << "</title>\n";
        html << "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>\n";
        html << "<style>body{font-family:sans-serif;margin:20px;background:#f5f5f5;}\n";
        html << ".card{background:#fff;border-radius:8px;padding:20px;margin:10px;box-shadow:0 2px 4px rgba(0,0,0,0.1);}\n";
        html << ".metrics{display:flex;gap:10px;flex-wrap:wrap;}\n";
        html << ".metric{background:#e3f2fd;padding:15px;border-radius:8px;min-width:150px;}\n";
        html << ".metric h3{margin:0 0 5px;color:#1976d2;}\n";
        html << ".metric .value{font-size:24px;font-weight:bold;}\n";
        html << "canvas{max-width:600px;margin:0 auto;}</style></head>\n";
        html << "<body><h1>" << titulo << "</h1>\n";

        if (!dashboard_->autor.empty()) {
            html << "<p>Autor: " << dashboard_->autor << "</p>\n";
        }

        html << "<div class='metrics'>\n";
        for (const auto& m : metricas_) {
            html << "<div class='metric'><h3>" << m.nombre << "</h3>\n";
            html << "<div class='value'>" << m.valor << "</div></div>\n";
        }
        html << "</div>\n";

        html << "<div class='card'><canvas id='chart'></canvas></div>\n";
        html << "<script>\nconst ctx = document.getElementById('chart').getContext('2d');\n";
        html << "new Chart(ctx, {type: 'line', data: {labels: [], datasets: []}});\n";
        html << "</script></body></html>\n";
        html.close();

        return mk_str("Dashboard generado: " + filename + " (abrelo en el navegador)");
    }

    if (nombre == "route") {
        if (args.size() != 3) return mk_err("runtime", "route requiere 3 argumentos: (metodo, path, handler)", 0);
        if (args[0]->tipo != ValorImpl::STR) return mk_err("runtime", "route: metodo debe ser string (GET, POST, PUT, DELETE, PATCH)", 0);
        if (args[1]->tipo != ValorImpl::STR) return mk_err("runtime", "route: path debe ser string y empezar con /", 0);
        if (args[2]->tipo != ValorImpl::FUNC) return mk_err("runtime", "route: handler debe ser una funcion", 0);

        std::string metodo = args[0]->str_val;
        std::string path = args[1]->str_val;

        if (metodo != "GET" && metodo != "POST" && metodo != "PUT" &&
            metodo != "DELETE" && metodo != "PATCH") {
            return mk_err("runtime", "route: metodo invalido '" + metodo + "' (usa GET, POST, PUT, DELETE, PATCH)", 0);
        }
        if (path.empty() || path[0] != '/') {
            return mk_err("runtime", "route: path debe empezar con /", 0);
        }

        RutaRegistrada r;
        r.metodo = metodo;
        r.path = path;
        r.handler = args[2];
        rutas_registradas_.push_back(r);
        return mk_num(static_cast<double>(rutas_registradas_.size()));
    }

    if (nombre == "load_csv") {
        if (args.empty()) return mk_err("runtime", "load_csv requiere una ruta de archivo", 0);
        if (args[0]->tipo != ValorImpl::STR) return mk_err("runtime", "load_csv requiere un string como ruta", 0);

        std::string ruta = resolver_ruta(args[0]->str_val);
        char delim = ',';
        if (args.size() >= 2 && args[1]->tipo == ValorImpl::STR && !args[1]->str_val.empty()) {
            delim = args[1]->str_val[0];
        }

        std::ifstream archivo(ruta);
        if (!archivo.is_open()) {
            return mk_err("io", "No se pudo abrir el archivo: " + ruta, 0);
        }

        // Read header
        std::string linea;
        if (!std::getline(archivo, linea)) {
            return mk_err("io", "Archivo CSV vacio: " + ruta, 0);
        }

        // Parse header with manual tokenizer
        std::vector<std::string> headers;
        {
            size_t pos = 0;
            while (pos < linea.size()) {
                size_t end = linea.find(delim, pos);
                if (end == std::string::npos) end = linea.size();
                std::string tok = linea.substr(pos, end - pos);
                // trim whitespace
                size_t s = tok.find_first_not_of(" \t\r\n\"");
                size_t e = tok.find_last_not_of(" \t\r\n\"");
                headers.push_back(s != std::string::npos ? tok.substr(s, e - s + 1) : "");
                pos = end + 1;
            }
        }

        size_t nc = headers.size();
        if (nc == 0) return mk_err("parse", "CSV sin columnas: " +ruta, 0);

        // Reserve structures
        std::vector<std::vector<std::string>> datos_raw(nc);

        // Count lines for reserve
        {
            std::streampos cur = archivo.tellg();
            size_t line_count = 0;
            std::string tmp;
            while (std::getline(archivo, tmp)) if (!tmp.empty()) ++line_count;
            archivo.clear();
            archivo.seekg(cur);
            for (size_t i = 0; i < nc; ++i) datos_raw[i].reserve(line_count);
        }

        // Single pass: read + parse + store raw
        while (std::getline(archivo, linea)) {
            if (linea.empty()) continue;
            size_t pos = 0;
            size_t col_idx = 0;
            while (col_idx < nc) {
                size_t end = linea.find(delim, pos);
                if (end == std::string::npos) end = linea.size();
                std::string tok = linea.substr(pos, end - pos);
                // trim
                size_t s = tok.find_first_not_of(" \t\r\n\"");
                size_t e = tok.find_last_not_of(" \t\r\n\"");
                datos_raw[col_idx].push_back(s != std::string::npos ? tok.substr(s, e - s + 1) : "");
                pos = end + 1;
                col_idx++;
            }
        }

        // Infer types and build columns in one pass per column
        std::map<std::string, Columna> columnas;
        std::vector<std::string> nombres;
        nombres.reserve(nc);

        for (size_t ci = 0; ci < nc; ++ci) {
            nombres.push_back(headers[ci]);
            const auto& col_data = datos_raw[ci];
            size_t nrows = col_data.size();

            // Type detection: check first non-null, non-empty value
            int tipo_detectado = 0; // 0=unset, 1=num, 2=bool, 3=str
            for (const auto& v : col_data) {
                if (v.empty() || v == "null" || v == "NA" || v == "NaN" || v == "n/a") continue;
                if (v == "true" || v == "false" || v == "TRUE" || v == "FALSE" || v == "True" || v == "False") {
                    tipo_detectado = 2; break;
                }
                double dummy;
                auto [ptr, ec] = std::from_chars(v.data(), v.data() + v.size(), dummy);
                if (ec == std::errc()) { tipo_detectado = 1; break; }
                tipo_detectado = 3; break;
            }

            if (tipo_detectado == 1) {
                // Numeric column
                Columna c("num");
                c.nums.reserve(nrows);
                c.null_bitmap.reserve(nrows);
                for (const auto& v : col_data) {
                    if (v.empty() || v == "null" || v == "NA" || v == "NaN" || v == "n/a") {
                        c.nums.push_back(crear_null());
                        c.null_bitmap.push_back(true);
                    } else {
                        double val;
                        auto [ptr, ec] = std::from_chars(v.data(), v.data() + v.size(), val);
                        c.nums.push_back(ec == std::errc() ? val : crear_null());
                        c.null_bitmap.push_back(ec != std::errc());
                    }
                }
                columnas[headers[ci]] = std::move(c);
            } else if (tipo_detectado == 2) {
                // Boolean column
                Columna c("bool");
                c.bools.reserve(nrows);
                c.null_bitmap.reserve(nrows);
                for (const auto& v : col_data) {
                    if (v.empty() || v == "null" || v == "NA" || v == "NaN" || v == "n/a") {
                        c.bools.push_back(false);
                        c.null_bitmap.push_back(true);
                    } else {
                        c.bools.push_back(v == "true" || v == "TRUE" || v == "True");
                        c.null_bitmap.push_back(false);
                    }
                }
                columnas[headers[ci]] = std::move(c);
            } else {
                // String column (default)
                Columna c("str");
                c.strs.reserve(nrows);
                c.null_bitmap.reserve(nrows);
                for (const auto& v : col_data) {
                    if (v.empty() || v == "null" || v == "NA" || v == "NaN" || v == "n/a") {
                        c.strs.push_back("");
                        c.null_bitmap.push_back(true);
                    } else {
                        c.strs.push_back(std::move(const_cast<std::string&>(v)));
                        c.null_bitmap.push_back(false);
                    }
                }
                columnas[headers[ci]] = std::move(c);
            }
        }

        DataFrame df;
        df.nombres_columnas = std::move(nombres);
        df.columnas = std::move(columnas);
        if (!df.validar_simetria()) {
            return mk_err("parse", "CSV con columnas de longitudes distintas en: " + ruta, 0);
        }

        return mk_df(std::move(df));
    }

    if (nombre == "load_json") {
        if (args.empty()) return mk_err("runtime", "load_json requiere una ruta de archivo", 0);
        if (args[0]->tipo != ValorImpl::STR) return mk_err("runtime", "load_json requiere un string como ruta", 0);

        std::string ruta = resolver_ruta(args[0]->str_val);
        std::ifstream archivo(ruta);
        if (!archivo.is_open()) {
            return mk_err("io", "No se pudo abrir el archivo: " + ruta, 0);
        }

        nlohmann::json j;
        try {
            j = nlohmann::json::parse(archivo);
        } catch (const std::exception& e) {
            return mk_err("parse", "JSON invalido en " + ruta + ": " + e.what(), 0);
        }

        if (!j.is_array() || j.empty()) {
            return mk_err("parse", "load_json requiere un array no vacio de objetos en: " + ruta, 0);
        }

        // Collect all column names in order
        std::vector<std::string> nombres;
        for (const auto& item : j) {
            if (!item.is_object()) continue;
            for (auto it = item.begin(); it != item.end(); ++it) {
                if (std::find(nombres.begin(), nombres.end(), it.key()) == nombres.end()) {
                    nombres.push_back(it.key());
                }
            }
        }
        if (nombres.empty()) {
            return mk_err("parse", "load_json: objetos sin propiedades en: " + ruta, 0);
        }

        size_t nc = nombres.size();
        size_t nrows = j.size();

        // Detect column types from first non-null value
        std::vector<int> col_types(nc, 0); // 0=unset,1=num,2=bool,3=str
        for (size_t ci = 0; ci < nc; ++ci) {
            for (const auto& item : j) {
                if (!item.contains(nombres[ci])) continue;
                const auto& val = item[nombres[ci]];
                if (val.is_null()) continue;
                if (val.is_number()) { col_types[ci] = 1; break; }
                if (val.is_boolean()) { col_types[ci] = 2; break; }
                if (val.is_string()) { col_types[ci] = 3; break; }
                col_types[ci] = 3; break;
            }
        }

        // Build columns
        std::map<std::string, Columna> columnas;
        for (size_t ci = 0; ci < nc; ++ci) {
            if (col_types[ci] == 1) {
                Columna c("num");
                c.nums.reserve(nrows);
                c.null_bitmap.reserve(nrows);
                for (const auto& item : j) {
                    if (!item.contains(nombres[ci]) || item[nombres[ci]].is_null()) {
                        c.nums.push_back(crear_null());
                        c.null_bitmap.push_back(true);
                    } else {
                        c.nums.push_back(item[nombres[ci]].get<double>());
                        c.null_bitmap.push_back(false);
                    }
                }
                columnas[nombres[ci]] = std::move(c);
            } else if (col_types[ci] == 2) {
                Columna c("bool");
                c.bools.reserve(nrows);
                c.null_bitmap.reserve(nrows);
                for (const auto& item : j) {
                    if (!item.contains(nombres[ci]) || item[nombres[ci]].is_null()) {
                        c.bools.push_back(false);
                        c.null_bitmap.push_back(true);
                    } else {
                        c.bools.push_back(item[nombres[ci]].get<bool>());
                        c.null_bitmap.push_back(false);
                    }
                }
                columnas[nombres[ci]] = std::move(c);
            } else {
                Columna c("str");
                c.strs.reserve(nrows);
                c.null_bitmap.reserve(nrows);
                for (const auto& item : j) {
                    if (!item.contains(nombres[ci]) || item[nombres[ci]].is_null()) {
                        c.strs.push_back("");
                        c.null_bitmap.push_back(true);
                    } else if (item[nombres[ci]].is_string()) {
                        c.strs.push_back(item[nombres[ci]].get<std::string>());
                        c.null_bitmap.push_back(false);
                    } else {
                        c.strs.push_back(item[nombres[ci]].dump());
                        c.null_bitmap.push_back(false);
                    }
                }
                columnas[nombres[ci]] = std::move(c);
            }
        }

        DataFrame df;
        df.nombres_columnas = std::move(nombres);
        df.columnas = std::move(columnas);
        return mk_df(std::move(df));
    }

    if (nombre == "load_xlsx") {
        if (args.empty()) return mk_err("runtime", "load_xlsx requiere una ruta de archivo", 0);
        if (args[0]->tipo != ValorImpl::STR) return mk_err("runtime", "load_xlsx requiere un string como ruta", 0);
        int sheet = (args.size() >= 2 && args[1]->tipo == ValorImpl::NUM) ? (int)args[1]->num_val : 0;
        std::string ruta = resolver_ruta(args[0]->str_val);
        DataFrame df = load_xlsx_file(ruta, sheet);
        if (df.nombres_columnas.empty()) {
            return mk_err("io", "No se pudo leer XLSX: " + ruta, 0);
        }
        return mk_df(std::move(df));
    }

    if (nombre == "save_xlsx") {
        if (args.size() < 2) return mk_err("runtime", "save_xlsx requiere ruta y DataFrame", 0);
        if (args[0]->tipo != ValorImpl::STR) return mk_err("runtime", "save_xlsx requiere un string como ruta", 0);
        if (args[1]->tipo != ValorImpl::DF) return mk_err("runtime", "save_xlsx requiere un DataFrame", 0);
        std::string ruta = resolver_ruta(args[0]->str_val);
        std::string msg = save_xlsx_file(ruta, args[1]->df_val);
        return mk_str(msg);
    }

    if (nombre == "save_csv") {
        if (args.size() < 2) return mk_err("runtime", "save_csv requiere ruta y DataFrame", 0);
        if (args[0]->tipo != ValorImpl::STR) return mk_err("runtime", "save_csv requiere un string como ruta", 0);
        if (args[1]->tipo != ValorImpl::DF) return mk_err("runtime", "save_csv requiere un DataFrame", 0);

        const DataFrame& df = args[1]->df_val;
        char delim = ',';
        if (args.size() >= 3 && args[2]->tipo == ValorImpl::STR && !args[2]->str_val.empty()) {
            delim = args[2]->str_val[0];
        }

        std::string ruta = resolver_ruta(args[0]->str_val);
        std::ofstream out(ruta);
        if (!out.is_open()) return mk_err("io", "No se pudo crear: " + ruta, 0);

        for (size_t i = 0; i < df.nombres_columnas.size(); ++i) {
            if (i > 0) out << delim;
            out << df.nombres_columnas[i];
        }
        out << '\n';

        size_t nrows = 0;
        for (const auto& [name, col] : df.columnas) {
            if (col.size() > nrows) nrows = col.size();
        }

        for (size_t ri = 0; ri < nrows; ++ri) {
            for (size_t ci = 0; ci < df.nombres_columnas.size(); ++ci) {
                if (ci > 0) out << delim;
                const auto& col = df.columnas.at(df.nombres_columnas[ci]);
                if (col.null_bitmap[ri]) {
                    // empty
                } else if (col.tipo == "num") {
                    char buf[64];
                    auto [ptr, ec] = std::to_chars(buf, buf + 64, col.nums[ri], std::chars_format::general, 15);
                    if (ec == std::errc()) out.write(buf, ptr - buf);
                } else if (col.tipo == "bool") {
                    out << (col.bools[ri] ? "true" : "false");
                } else {
                    out << col.strs[ri];
                }
            }
            out << '\n';
        }
        out.close();
        return mk_str("Guardado CSV: " + args[0]->str_val + " (" + std::to_string(nrows) + " filas)");
    }

    if (nombre == "load_lib") {
        if (args.size() < 2) return mk_err("runtime", "load_lib requiere ruta y diccionario de funciones", 0);
        if (args[0]->tipo != ValorImpl::STR) return mk_err("runtime", "load_lib requiere un string como ruta", 0);
        if (args[1]->tipo != ValorImpl::DICT) return mk_err("runtime", "load_lib requiere un dict de {nombre: firma}", 0);

        std::string ruta = args[0]->str_val;
        std::string ruta_abs = resolver_ruta(ruta);
        if (ruta_abs == ruta && !fs::path(ruta).is_absolute()) {
            if (fs::exists("./lib/" + ruta)) ruta_abs = "./lib/" + ruta;
            else {
                for (const auto& ip : include_paths_) {
                    fs::path p = fs::path(ip) / ruta;
                    if (fs::exists(p)) { ruta_abs = p.string(); break; }
                }
            }
        }

        try {
            DlLibrary* lib = DlRegistry::instancia().cargar(ruta_abs);
            for (const auto& [nombre, _] : args[1]->dict_val) {
                void* sym = lib->obtener_simbolo(nombre);
                auto fn_val = mk_func("::dl::" + nombre, {}, sym, nullptr);
                ambito_global_->definir(nombre, fn_val);
            }
            return mk_str("loaded: " + ruta_abs);
        } catch (const std::exception& e) {
            return mk_err("dl", std::string("Error cargando lib: ") + e.what(), 0);
        }
    }

    if (nombre == "plugin") {
        // plugin("lib.so") - Load plugin with v2 ABI support
        // plugin("lib.so", ["func1", "func2"]) - Load and register specific functions
        if (args.empty() || args[0]->tipo != ValorImpl::STR)
            return mk_err("runtime", "plugin requiere ruta del plugin", 0);

        std::string ruta = args[0]->str_val;
        std::string ruta_abs = resolver_ruta(ruta);
        if (ruta_abs == ruta && !fs::path(ruta).is_absolute()) {
            if (fs::exists("./lib/" + ruta)) ruta_abs = "./lib/" + ruta;
            else {
                for (const auto& ip : include_paths_) {
                    fs::path p = fs::path(ip) / ruta;
                    if (fs::exists(p)) { ruta_abs = p.string(); break; }
                }
            }
        }

        try {
            ZetaPlugin* plugin = DlRegistry::instancia().cargar_plugin(ruta_abs);
            if (!plugin) {
                return mk_err("plugin", "No se pudo cargar plugin: " + ruta, 0);
            }

            // Print plugin info
            const auto& info = plugin->info();
            std::cout << "[PLUGIN] " << info.name << " v" << info.version << "\n";
            if (!info.description.empty()) {
                std::cout << "  " << info.description << "\n";
            }
            if (!info.author.empty()) {
                std::cout << "  Author: " << info.author << "\n";
            }

            // Register functions from plugin
            // Try common function names
            std::vector<std::string> func_names;
            if (args.size() > 1 && args[1]->tipo == ValorImpl::STR_VEC) {
                // User specified function names
                for (const auto& f : args[1]->str_vec_val) {
                    func_names.push_back(f);
                }
            } else {
                // Auto-detect: try common names
                func_names = {"median", "percentile", "mode", "gcd", "factorial", 
                             "is_prime", "fibonacci", "sum_vec", "greet"};
            }

            int registered = 0;
            for (const auto& func_name : func_names) {
                if (plugin->tiene_funcion(func_name)) {
                    // Register function name in global scope
                    // The actual calling mechanism will be handled through the plugin system
                    ambito_global_->definir(func_name, mk_str("[plugin:" + info.name + "::" + func_name + "]"));
                    registered++;
                }
            }

            return mk_str("plugin loaded: " + info.name + " v" + info.version + 
                          " (" + std::to_string(registered) + " functions)");
        } catch (const std::exception& e) {
            return mk_err("plugin", std::string("Error cargando plugin: ") + e.what(), 0);
        }
    }

    if (nombre == "plugin_info") {
        // plugin_info("lib.so") - Get plugin info without loading
        if (args.empty() || args[0]->tipo != ValorImpl::STR)
            return mk_err("runtime", "plugin_info requiere ruta del plugin", 0);

        std::string ruta = args[0]->str_val;
        std::string ruta_abs = resolver_ruta(ruta);

        try {
            ZetaPlugin temp_plugin;
            if (!temp_plugin.cargar(ruta_abs)) {
                return mk_err("plugin", "No se pudo cargar plugin: " + temp_plugin.ultimo_error(), 0);
            }

            const auto& info = temp_plugin.info();
            std::map<std::string, ValorZeta> result;
            result["name"] = mk_str(info.name);
            result["version"] = mk_str(info.version);
            result["description"] = mk_str(info.description);
            result["author"] = mk_str(info.author);
            result["path"] = mk_str(ruta_abs);
            
            return mk_dict(std::move(result));
        } catch (const std::exception& e) {
            return mk_err("plugin", std::string("Error: ") + e.what(), 0);
        }
    }

    if (nombre == "scene") {
        if (args.empty()) return mk_err("runtime", "scene requiere titulo", 0);
        std::string titulo = (args[0]->tipo == ValorImpl::STR) ? args[0]->str_val : "scene";
        std::string autor = (args.size() > 1 && args[1]->tipo == ValorImpl::STR) ? args[1]->str_val : "zeta";
        auto s = std::make_shared<SceneSpec>();
        s->titulo = titulo;
        s->autor = autor;
        s->created_at = static_cast<double>(std::time(nullptr));
        s->updated_at = s->created_at;
        grafo_actual_ = s;
        return mk_scene(s);
    }

    if (nombre == "layout") {
        if (!grafo_actual_) return mk_err("scene", "No hay scene activa. Llama primero a scene()", 0);
        if (args.empty() || args[0]->tipo != ValorImpl::STR) return mk_err("runtime", "layout requiere tipo (grid/vertical/horizontal/absolute)", 0);
        grafo_actual_->layout.tipo = args[0]->str_val;
        if (args.size() > 1 && args[1]->tipo == ValorImpl::NUM) grafo_actual_->layout.cols = static_cast<int>(args[1]->num_val);
        if (args.size() > 2 && args[2]->tipo == ValorImpl::NUM) grafo_actual_->layout.gap = static_cast<int>(args[2]->num_val);
        if (args.size() > 3 && args[3]->tipo == ValorImpl::STR) grafo_actual_->layout.background = args[3]->str_val;
        grafo_actual_->updated_at = static_cast<double>(std::time(nullptr));
        return mk_scene(grafo_actual_);
    }

    if (nombre == "add_metric") {
        if (!grafo_actual_) return mk_err("scene", "No hay scene activa. Llama primero a scene()", 0);
        if (args.size() < 2) return mk_err("runtime", "add_metric requiere nombre y valor", 0);
        SceneNode n;
        n.tipo = "metric";
        n.titulo = (args[0]->tipo == ValorImpl::STR) ? args[0]->str_val : "";
        if (args[1]->tipo == ValorImpl::NUM) n.nums["value"] = args[1]->num_val;
        else if (args[1]->tipo == ValorImpl::STR) { n.tipo = "text"; n.strs["text"] = args[1]->str_val; }
        if (args.size() > 2 && args[2]->tipo == ValorImpl::STR) n.strs["unit"] = args[2]->str_val;
        if (args.size() > 3 && args[3]->tipo == ValorImpl::STR) n.strs["color"] = args[3]->str_val;
        grafo_actual_->nodes.push_back(std::move(n));
        grafo_actual_->updated_at = static_cast<double>(std::time(nullptr));
        return mk_scene(grafo_actual_);
    }

    auto add_plot = [&](const std::string& tipo, const std::vector<ValorZeta>& a) -> ValorZeta {
        if (!grafo_actual_) return mk_err("scene", "No hay scene activa. Llama primero a scene()", 0);
        if (a.size() < 2) return mk_err("runtime", tipo + " requiere al menos data y titulo", 0);
        if (a[0]->tipo != ValorImpl::DF) return mk_err("runtime", tipo + " requiere un DataFrame", 0);
        SceneNode n;
        n.tipo = tipo;
        n.titulo = (a[1]->tipo == ValorImpl::STR) ? a[1]->str_val : tipo;
        const auto& df = a[0]->df_val;
        for (const auto& [k, col] : df.columnas) {
            if (col.tipo == "num") n.series[k] = col.nums;
        }
        if (a.size() > 2 && a[2]->tipo == ValorImpl::STR) n.cols["x"] = a[2]->str_val;
        if (a.size() > 3 && a[3]->tipo == ValorImpl::STR) n.cols["y"] = a[3]->str_val;
        if (a.size() > 4 && a[4]->tipo == ValorImpl::NUM) n.nums["bins"] = a[4]->num_val;
        grafo_actual_->nodes.push_back(std::move(n));
        grafo_actual_->updated_at = static_cast<double>(std::time(nullptr));
        return mk_scene(grafo_actual_);
    };

    if (nombre == "add_line_plot") return add_plot("line_plot", args);
    if (nombre == "add_bar_chart") return add_plot("bar_chart", args);
    if (nombre == "add_scatter") return add_plot("scatter", args);
    if (nombre == "add_heatmap") return add_plot("heatmap", args);
    if (nombre == "add_linear_regression") return add_plot("linear_regression", args);

    if (nombre == "add_histogram") {
        if (!grafo_actual_) return mk_err("scene", "No hay scene activa. Llama primero a scene()", 0);
        if (args.size() < 3) return mk_err("runtime", "add_histogram requiere data, titulo y columna", 0);
        if (args[0]->tipo != ValorImpl::DF) return mk_err("runtime", "add_histogram requiere un DataFrame", 0);
        SceneNode n;
        n.tipo = "histogram";
        n.titulo = (args[1]->tipo == ValorImpl::STR) ? args[1]->str_val : "histogram";
        const auto& df = args[0]->df_val;
        for (const auto& [k, col] : df.columnas) {
            if (col.tipo == "num") n.series[k] = col.nums;
        }
        if (args[2]->tipo == ValorImpl::STR) n.cols["y"] = args[2]->str_val;
        if (args.size() > 3 && args[3]->tipo == ValorImpl::NUM) n.nums["bins"] = args[3]->num_val;
        grafo_actual_->nodes.push_back(std::move(n));
        grafo_actual_->updated_at = static_cast<double>(std::time(nullptr));
        return mk_scene(grafo_actual_);
    }

    if (nombre == "add_box_plot") {
        if (!grafo_actual_) return mk_err("scene", "No hay scene activa. Llama primero a scene()", 0);
        if (args.size() < 3) return mk_err("runtime", "add_box_plot requiere data, titulo y columna", 0);
        if (args[0]->tipo != ValorImpl::DF) return mk_err("runtime", "add_box_plot requiere un DataFrame", 0);
        SceneNode n;
        n.tipo = "box_plot";
        n.titulo = (args[1]->tipo == ValorImpl::STR) ? args[1]->str_val : "box_plot";
        const auto& df = args[0]->df_val;
        for (const auto& [k, col] : df.columnas) {
            if (col.tipo == "num") n.series[k] = col.nums;
        }
        if (args[2]->tipo == ValorImpl::STR) n.cols["y"] = args[2]->str_val;
        grafo_actual_->nodes.push_back(std::move(n));
        grafo_actual_->updated_at = static_cast<double>(std::time(nullptr));
        return mk_scene(grafo_actual_);
    }

    if (nombre == "guardar_grafo") {
        if (!grafo_actual_) return mk_err("scene", "No hay scene activa", 0);
        std::string ruta = (args.size() > 0 && args[0]->tipo == ValorImpl::STR) ? resolver_ruta(args[0]->str_val) : "grafo.json";
        return mk_str(guardar_grafo_json(ruta, *grafo_actual_));
    }

    if (nombre == "cargar_grafo") {
        if (args.empty() || args[0]->tipo != ValorImpl::STR) return mk_err("runtime", "cargar_grafo requiere ruta", 0);
        std::string ruta = resolver_ruta(args[0]->str_val);
        auto s = cargar_grafo_json(ruta);
        if (!s) return mk_err("io", "No se pudo cargar: " + ruta, 0);
        grafo_actual_ = s;
        return mk_scene(s);
    }

    if (nombre == "grafo_actual") {
        if (!grafo_actual_) return mk_null_val();
        return mk_scene(grafo_actual_);
    }

    if (nombre == "clear_arena") {
        // Reset the arena allocator — frees all temporary values at once.
        // This is a manual memory management primitive for large workloads.
        // After calling this, any arena-allocated temporaries become invalid.
        // Values stored in variables (via shared_ptr) are NOT affected.
        valor_arena_.reset();
        return mk_null_val();
    }

    if (nombre == "arena_bytes") {
        // Return the number of bytes currently allocated in the arena.
        return mk_num(static_cast<double>(valor_arena_.bytes_used()));
    }

    if (nombre == "zeta_version") {
        // Return the Zeta language version string.
        return mk_str("0.2.0");
    }

    return mk_err("runtime", "Funcion no definida: " + nombre, 0);
}

std::vector<bool> Interpreter::fn_is_null_bool(const std::vector<double>& vec) {
    std::vector<bool> resultado;
    resultado.reserve(vec.size());
    for (const auto& v : vec) {
        resultado.push_back(es_null(v));
    }
    return resultado;
}

ValorZeta Interpreter::evaluar_acceso_columnas(const NodoAST& nodo) {
    auto df_val = evaluar(*nodo.hijos[0]);

    if (df_val && df_val->tipo == ValorImpl::DF) {
        auto it = df_val->df_val.columnas.find(nodo.valor_texto);
        if (it == df_val->df_val.columnas.end()) {
            return mk_err("runtime", "Columna no encontrada: " + nodo.valor_texto, nodo.linea);
        }
        const auto& col = it->second;
        if (col.tipo == "num") return mk_vec(col.nums);
        if (col.tipo == "str") return mk_str_vec(col.strs);
        if (col.tipo == "bool") return mk_bool_vec(col.bools);
        return mk_err("runtime", "Tipo de columna desconocido: " + col.tipo, nodo.linea);
    }

    return mk_err("runtime", "Acceso a columnas solo valido en DataFrames", nodo.linea);
}

ValorZeta Interpreter::evaluar_filtro_filas(const NodoAST& nodo) {
    auto df_val = evaluar(*nodo.hijos[0]);

    if (df_val && df_val->tipo == ValorImpl::DF) {
        std::vector<bool> mascara;
        for (size_t i = 0; i < df_val->df_val.filas(); ++i) {
            auto ambito_fila = std::make_shared<TablaSimbolos>(ambito_global_);

            for (const auto& [nombre, col] : df_val->df_val.columnas) {
                if (i < col.size()) {
                    if (col.tipo == "num") ambito_fila->definir(nombre, mk_num(col.nums[i]));
                    else if (col.tipo == "str") ambito_fila->definir(nombre, mk_str(col.strs[i]));
                    else if (col.tipo == "bool") ambito_fila->definir(nombre, mk_bool(col.bools[i]));
                }
            }

            auto ambito_anterior = ambito_global_;
            ambito_global_ = ambito_fila;
            auto cond = evaluar(*nodo.hijos[1]);
            ambito_global_ = ambito_anterior;

            if (cond && cond->tipo == ValorImpl::BOOL) {
                mascara.push_back(cond->bool_val);
            } else {
                mascara.push_back(false);
            }
        }

        DataFrame resultado;
        resultado.nombres_columnas = df_val->df_val.nombres_columnas;
        for (const auto& [nombre, col] : df_val->df_val.columnas) {
            Columna nueva_col(col.tipo);
            for (size_t i = 0; i < col.size(); ++i) {
                if (mascara[i]) {
                    if (col.tipo == "num") {
                        nueva_col.nums.push_back(col.nums[i]);
                        nueva_col.null_bitmap.push_back(col.null_bitmap[i]);
                    } else if (col.tipo == "str") {
                        nueva_col.strs.push_back(col.strs[i]);
                        nueva_col.null_bitmap.push_back(col.null_bitmap[i]);
                    } else if (col.tipo == "bool") {
                        nueva_col.bools.push_back(col.bools[i]);
                        nueva_col.null_bitmap.push_back(col.null_bitmap[i]);
                    }
                }
            }
            resultado.columnas[nombre] = std::move(nueva_col);
        }
        return mk_df(resultado);
    }

    return mk_err("runtime", "Filtro de filas solo valido en DataFrames", nodo.linea);
}

ValorZeta Interpreter::evaluar_bloque(const NodoAST& nodo) {
    ValorZeta ultimo = mk_null_val();
    for (const auto& hijo : nodo.hijos) {
        auto val = evaluar(*hijo);
        if (val && val->tipo == ValorImpl::DICT &&
            val->str_val == "__ZETA_RETURN_VALUE__" &&
            val->dict_val.count("__value__")) {
            return val;
        }
        if (val && val->tipo == ValorImpl::STR &&
            (val->str_val == "__ZETA_BREAK__" || val->str_val == "__ZETA_CONTINUE__")) {
            return val;
        }
        ultimo = val;
        if (is_error(ultimo)) return ultimo;
    }
    return ultimo;
}

ValorZeta Interpreter::evaluar_clase(const NodoAST& nodo) {
    auto class_def = std::make_shared<ClassDef>();
    class_def->nombre = nodo.valor_texto;
    if (!nodo.parametros.empty()) {
        class_def->padre = nodo.parametros[0];
    }
    for (const auto& hijo : nodo.hijos) {
        if (hijo->tipo == TipoNodoAST::ASIGNACION) {
            // Field default
            std::string fname = hijo->valor_texto;
            ValorZeta fval = evaluar(*hijo);
            if (is_error(fval)) return fval;
            class_def->campos_default[fname] = fval;
        } else if (hijo->tipo == TipoNodoAST::DECLARACION_FN) {
            // Method
            auto fn_val = mk_func(hijo->valor_texto, hijo->parametros,
                          hijo->hijos.empty() ? nullptr : hijo->hijos[0].get(),
                          ambito_global_);
            class_def->metodos[hijo->valor_texto] = fn_val;
        }
    }
    // Register as a global so `new Nombre(...)` can find it
    // (the class name itself is stored as a DICT of {kind:"class", def:...})
    // But we want users to be able to look it up by name. Store it as a
    // sentinel DICT with the class name.
    nlohmann::json j;
    j["__class__"] = class_def->nombre;
    j["__padre__"] = class_def->padre;
    nlohmann::json campos_j = nlohmann::json::object();
    for (const auto& [k, v] : class_def->campos_default) {
        campos_j[k] = static_cast<double>(0);  // placeholder, real values live in ObjetoZeta
    }
    j["__campos__"] = campos_j;
    j["__metodos__"] = nlohmann::json::object();
    for (const auto& [k, v] : class_def->metodos) {
        j["__metodos__"][k] = true;
    }
    ValorZeta class_marker = mk_str(j.dump());
    clases_definidas_[class_def->nombre] = class_def;
    ambito_global_->definir(nodo.valor_texto, class_marker);
    return mk_null_val();
}

ValorZeta Interpreter::evaluar_new(const NodoAST& nodo) {
    std::string class_name = nodo.valor_texto;
    auto it = clases_definidas_.find(class_name);
    if (it == clases_definidas_.end()) {
        return mk_err("runtime", "Clase no definida: " + class_name, 0);
    }
    auto class_def = it->second;

    auto obj = std::make_shared<ObjetoZeta>();
    obj->clase = class_name;
    // Copy default fields (with inheritance)
    std::string cur = class_name;
    while (!cur.empty()) {
        auto cit = clases_definidas_.find(cur);
        if (cit == clases_definidas_.end()) break;
        for (const auto& [k, v] : cit->second->campos_default) {
            if (obj->campos.find(k) == obj->campos.end()) {
                obj->campos[k] = v;
            }
        }
        cur = cit->second->padre;
    }

    ValorZeta obj_val = mk_obj(obj);

    // Call init method if exists (walk inheritance chain)
    ValorZeta init_method;
    {
        std::string cur_init = class_name;
        while (!cur_init.empty()) {
            auto cit_init = clases_definidas_.find(cur_init);
            if (cit_init == clases_definidas_.end()) break;
            auto mit_init = cit_init->second->metodos.find("init");
            if (mit_init != cit_init->second->metodos.end()) {
                init_method = mit_init->second;
                break;
            }
            cur_init = cit_init->second->padre;
        }
    }
    if (init_method) {
        std::vector<ValorZeta> args;
        args.push_back(obj_val);
        for (const auto& hijo : nodo.hijos) {
            args.push_back(evaluar(*hijo));
        }
        auto prev_this = this_actual_;
        this_actual_ = obj_val;
        ValorZeta result = llamar_usuario(init_method, args);
        this_actual_ = prev_this;
        if (is_error(result)) return result;
    } else {
        // No init: still evaluate args in case they have side effects
        for (const auto& hijo : nodo.hijos) {
            evaluar(*hijo);
        }
    }
    return obj_val;
}

ValorZeta Interpreter::evaluar_this(const NodoAST& nodo) {
    if (!this_actual_) {
        return mk_err("runtime", "'this' usado fuera de un metodo", 0);
    }
    return this_actual_;
}

ValorZeta Interpreter::evaluar_llamada_metodo(const NodoAST& nodo) {
    // hijos[0] = obj, hijos[1..] = args
    if (nodo.hijos.empty()) {
        return mk_err("runtime", "Llamada a metodo sin objeto", 0);
    }
    ValorZeta obj_val = evaluar(*nodo.hijos[0]);
    if (is_error(obj_val)) return obj_val;
    if (!obj_val || obj_val->tipo != ValorImpl::OBJ) {
        return mk_err("runtime", "Intento de llamar metodo en algo que no es objeto", 0);
    }
    std::string method_name = nodo.valor_texto;
    auto obj = obj_val->obj_val;
    // Walk inheritance chain
    std::string cur = obj->clase;
    ValorZeta method_val;
    while (!cur.empty()) {
        auto cit = clases_definidas_.find(cur);
        if (cit == clases_definidas_.end()) break;
        auto mit = cit->second->metodos.find(method_name);
        if (mit != cit->second->metodos.end()) {
            method_val = mit->second;
            break;
        }
        cur = cit->second->padre;
    }
    if (!method_val) {
        return mk_err("runtime", "Metodo '" + method_name + "' no existe en clase '" + obj->clase + "'", 0);
    }
    std::vector<ValorZeta> args;
    args.push_back(obj_val);
    for (size_t i = 1; i < nodo.hijos.size(); ++i) {
        args.push_back(evaluar(*nodo.hijos[i]));
    }
    auto prev_this = this_actual_;
    this_actual_ = obj_val;
    ValorZeta result = llamar_usuario(method_val, args);
    this_actual_ = prev_this;
    return result;
}

ValorZeta Interpreter::evaluar_if(const NodoAST& nodo) {
    auto cond = evaluar(*nodo.hijos[0]);

    if (cond && cond->tipo == ValorImpl::BOOL) {
        if (cond->bool_val) {
            return evaluar(*nodo.hijos[1]);
        } else if (nodo.hijos.size() > 2) {
            return evaluar(*nodo.hijos[2]);
        }
    }
    return mk_null_val();
}

ValorZeta Interpreter::evaluar_for(const NodoAST& nodo) {
    auto coleccion = evaluar(*nodo.hijos[0]);
    auto nombre_var = nodo.valor_texto;

    if (coleccion && coleccion->tipo == ValorImpl::VEC) {
        for (const auto& elem : coleccion->vec_val) {
            auto ambito_local = std::make_shared<TablaSimbolos>(ambito_global_);
            ambito_local->definir(nombre_var, mk_num(elem));
            auto anterior = ambito_global_;
            ambito_global_ = ambito_local;
            auto val = evaluar(*nodo.hijos[1]);
            ambito_global_ = anterior;
            if (val && val->tipo == ValorImpl::STR && val->str_val == "__ZETA_BREAK__") break;
            if (val && val->tipo == ValorImpl::STR && val->str_val == "__ZETA_CONTINUE__") continue;
            if (val && val->tipo == ValorImpl::DICT &&
                val->str_val == "__ZETA_RETURN_VALUE__") {
                return val;
            }
        }
    }

    return mk_null_val();
}

ValorZeta Interpreter::evaluar_while(const NodoAST& nodo) {
    while (true) {
        auto cond = evaluar(*nodo.hijos[0]);
        if (!cond || cond->tipo != ValorImpl::BOOL || !cond->bool_val) break;
        auto val = evaluar(*nodo.hijos[1]);
        if (val && val->tipo == ValorImpl::STR && val->str_val == "__ZETA_BREAK__") break;
        if (val && val->tipo == ValorImpl::STR && val->str_val == "__ZETA_CONTINUE__") continue;
        if (val && val->tipo == ValorImpl::DICT &&
            val->str_val == "__ZETA_RETURN_VALUE__") {
            return val;
        }
    }
    return mk_null_val();
}

ValorZeta Interpreter::evaluar_acceso_indice(const NodoAST& nodo) {
    auto obj = evaluar(*nodo.hijos[0]);
    auto indice = evaluar(*nodo.hijos[1]);

    if (!obj || !indice) return mk_err("runtime", "Objeto o indice invalido", nodo.linea);

    if (obj->tipo == ValorImpl::VEC && indice->tipo == ValorImpl::NUM) {
        if (es_null(indice->num_val)) return mk_err("runtime", "Indice es null", nodo.linea);
        int idx = static_cast<int>(indice->num_val);
        if (idx < 0) idx += static_cast<int>(obj->vec_val.size());
        if (idx < 0 || idx >= static_cast<int>(obj->vec_val.size())) {
            return mk_err("runtime", "Indice fuera de rango", nodo.linea);
        }
        return mk_num(obj->vec_val[idx]);
    }

    if (obj->tipo == ValorImpl::DICT && indice->tipo == ValorImpl::STR) {
        auto it = obj->dict_val.find(indice->str_val);
        if (it == obj->dict_val.end()) {
            return mk_err("runtime", "Clave no existe: " + indice->str_val, nodo.linea);
        }
        return it->second;
    }

    return mk_err("runtime", "Acceso por indice no soportado para este tipo", nodo.linea);
}

ValorZeta Interpreter::evaluar_acceso_matriz(const NodoAST& nodo) {
    auto mat = evaluar(*nodo.hijos[0]);
    auto fila_val = evaluar(*nodo.hijos[1]);
    auto col_val = evaluar(*nodo.hijos[2]);

    if (!mat || mat->tipo != ValorImpl::MATRIZ) return mk_err("runtime", "Acceso a matriz requiere tipo matriz", nodo.linea);
    if (!fila_val || fila_val->tipo != ValorImpl::NUM) return mk_err("runtime", "Indice de fila invalido", nodo.linea);
    if (!col_val || col_val->tipo != ValorImpl::NUM) return mk_err("runtime", "Indice de columna invalido", nodo.linea);
    if (es_null(fila_val->num_val) || es_null(col_val->num_val)) return mk_err("runtime", "Indice es null", nodo.linea);

    int fila = static_cast<int>(fila_val->num_val);
    int col = static_cast<int>(col_val->num_val);

    if (fila < 0) fila += static_cast<int>(mat->matriz_val.size());
    if (col < 0) col += static_cast<int>(mat->matriz_val[0].size());

    if (fila < 0 || fila >= static_cast<int>(mat->matriz_val.size())) {
        return mk_err("runtime", "Fila fuera de rango", nodo.linea);
    }
    if (col < 0 || col >= static_cast<int>(mat->matriz_val[fila].size())) {
        return mk_err("runtime", "Columna fuera de rango", nodo.linea);
    }

    return mk_num(mat->matriz_val[fila][col]);
}

ValorZeta Interpreter::evaluar_asignacion_indice(const NodoAST& nodo) {
    auto& acceso = nodo.hijos[0];
    auto valor = evaluar(*nodo.hijos[1]);

    if (acceso->tipo == TipoNodoAST::ACCESO_INDICE) {
        auto var_node = acceso->hijos[0].get();
        if (var_node->tipo != TipoNodoAST::VARIABLE) {
            return mk_err("runtime", "Asignacion por indice requiere variable", nodo.linea);
        }

        auto* var_ptr = ambito_global_->buscar(var_node->valor_texto);
        if (!var_ptr) return mk_err("runtime", "Variable no definida: " + var_node->valor_texto, nodo.linea);

        auto indice = evaluar(*acceso->hijos[1]);
        if (!indice) return mk_err("runtime", "Indice invalido", nodo.linea);

        if ((*var_ptr)->tipo == ValorImpl::VEC) {
            if (indice->tipo != ValorImpl::NUM) return mk_err("runtime", "Vector requiere indice numerico", nodo.linea);
            if (es_null(indice->num_val)) return mk_err("runtime", "Indice es null", nodo.linea);
            int idx = static_cast<int>(indice->num_val);
            if (idx < 0) idx += static_cast<int>((*var_ptr)->vec_val.size());
            if (idx < 0 || idx >= static_cast<int>((*var_ptr)->vec_val.size())) {
                return mk_err("runtime", "Indice fuera de rango", nodo.linea);
            }
            if (valor->tipo != ValorImpl::NUM) return mk_err("runtime", "Vector solo acepta numeros", nodo.linea);
            (*var_ptr)->vec_val[idx] = valor->num_val;
            return valor;
        }

        if ((*var_ptr)->tipo == ValorImpl::DICT) {
            if (indice->tipo != ValorImpl::STR) return mk_err("runtime", "Dict requiere clave string", nodo.linea);
            (*var_ptr)->dict_val[indice->str_val] = valor;
            return valor;
        }

        return mk_err("runtime", "Tipo no soporta asignacion por indice", nodo.linea);
    }

    if (acceso->tipo == TipoNodoAST::ACCESO_MATRIZ) {
        auto var_node = acceso->hijos[0].get();
        if (var_node->tipo != TipoNodoAST::VARIABLE) {
            return mk_err("runtime", "Asignacion a matriz requiere variable", nodo.linea);
        }

        auto* var_ptr = ambito_global_->buscar(var_node->valor_texto);
        if (!var_ptr) return mk_err("runtime", "Variable no definida: " + var_node->valor_texto, nodo.linea);
        if ((*var_ptr)->tipo != ValorImpl::MATRIZ) return mk_err("runtime", "Variable no es matriz", nodo.linea);

        auto fila_val = evaluar(*acceso->hijos[1]);
        auto col_val = evaluar(*acceso->hijos[2]);
        if (!fila_val || fila_val->tipo != ValorImpl::NUM) return mk_err("runtime", "Indice de fila invalido", nodo.linea);
        if (!col_val || col_val->tipo != ValorImpl::NUM) return mk_err("runtime", "Indice de columna invalido", nodo.linea);
        if (es_null(fila_val->num_val) || es_null(col_val->num_val)) return mk_err("runtime", "Indice es null", nodo.linea);

        int fila = static_cast<int>(fila_val->num_val);
        int col = static_cast<int>(col_val->num_val);

        if (fila < 0) fila += static_cast<int>((*var_ptr)->matriz_val.size());
        if (col < 0) col += static_cast<int>((*var_ptr)->matriz_val[0].size());

        if (fila < 0 || fila >= static_cast<int>((*var_ptr)->matriz_val.size())) {
            return mk_err("runtime", "Fila fuera de rango", nodo.linea);
        }
        if (col < 0 || col >= static_cast<int>((*var_ptr)->matriz_val[fila].size())) {
            return mk_err("runtime", "Columna fuera de rango", nodo.linea);
        }

        if (valor->tipo != ValorImpl::NUM) return mk_err("runtime", "Matriz solo acepta numeros", nodo.linea);
        (*var_ptr)->matriz_val[fila][col] = valor->num_val;
        return valor;
    }

    return mk_err("runtime", "Nodo de acceso no soportado para asignacion", nodo.linea);
}

void Interpreter::agregar_include_path(const std::string& path) {
    include_paths_.push_back(path);
}

void Interpreter::set_script_path(const std::string& path) {
    script_dir_ = fs::path(path).parent_path().string();
    if (script_dir_.empty()) script_dir_ = ".";
}

std::string Interpreter::resolver_ruta(const std::string& ruta) const {
    if (fs::path(ruta).is_absolute()) return ruta;
    if (!script_dir_.empty()) {
        std::string intento = (fs::path(script_dir_) / ruta).string();
        if (fs::exists(intento)) return intento;
    }
    return ruta;
}

std::filesystem::path Interpreter::resolver_ruta_modulo(const std::string& ruta) const {
    fs::path p(ruta);
    if (p.is_absolute() && fs::exists(p)) return p;

    std::vector<std::string> candidatos_ext{".zl", ""};
    std::vector<fs::path> bases;
    if (!script_dir_.empty()) bases.push_back(fs::path(script_dir_) / "lib");
    bases.push_back(fs::current_path() / "lib");
    for (const auto& ip : include_paths_) bases.push_back(fs::path(ip) / "lib");
    if (const char* home = std::getenv("HOME")) bases.push_back(fs::path(home) / ".zeta" / "lib");

    for (const auto& ext : candidatos_ext) {
        for (const auto& base : bases) {
            fs::path intento = base / (ruta + ext);
            if (fs::exists(intento)) return intento;
        }
    }
    return {};
}

static std::string nombre_base_modulo(const std::string& ruta) {
    fs::path p(ruta);
    std::string stem = p.stem().string();
    return stem;
}

static std::string canonicalizar_ruta(const std::filesystem::path& p) {
    if (!p.empty()) return fs::weakly_canonical(p).string();
    return p.string();
}

ValorZeta Interpreter::cargar_modulo(const std::string& ruta) {
    std::string alias = nombre_base_modulo(ruta);
    return cargar_con_alias(ruta, alias);
}

ValorZeta Interpreter::cargar_con_alias(const std::string& ruta, const std::string& alias) {
    fs::path ruta_abs = resolver_ruta_modulo(ruta);
    if (ruta_abs.empty()) {
        return mk_err("import", "No se encontro el modulo: " + ruta, 0);
    }
    std::string canon = canonicalizar_ruta(ruta_abs);

    auto it = modulos_cache_.find(canon);
    if (it != modulos_cache_.end()) {
        std::map<std::string, ValorZeta> ns;
        for (const auto& nombre : it->second.exports) {
            auto sym_it = it->second.simbolos.find(nombre);
            if (sym_it != it->second.simbolos.end()) {
                ns[nombre] = sym_it->second;
            }
        }
        ambito_global_->definir(alias, mk_dict(ns));
        return mk_dict(ns);
    }

    if (std::find(pila_imports_.begin(), pila_imports_.end(), canon) != pila_imports_.end()) {
        return mk_err("import", "Ciclo de importacion detectado: " + canon, 0);
    }

    std::ifstream archivo(ruta_abs);
    if (!archivo.is_open()) {
        return mk_err("import", "No se pudo abrir: " + canon, 0);
    }
    std::stringstream ss;
    ss << archivo.rdbuf();
    std::string codigo = ss.str();

    pila_imports_.push_back(canon);

    auto ambito_previo = ambito_global_->variables;
    bool cargando_previo = cargando_modulo_;
    std::set<std::string> exports_previos = exports_modulo_actual_;
    cargando_modulo_ = true;
    exports_modulo_actual_.clear();

    std::shared_ptr<NodoAST> ast_vivo;
    ValorZeta resultado_ejec(mk_null_val());
    try {
        Lexer lexer(codigo);
        auto tokens = lexer.tokenizar();
        Parser parser(std::move(tokens));
        auto ast = parser.parsear();
        ast_vivo = std::move(ast);
        resultado_ejec = evaluar(*ast_vivo);
    } catch (const std::exception& e) {
        cargando_modulo_ = cargando_previo;
        exports_modulo_actual_ = exports_previos;
        pila_imports_.pop_back();
        return mk_err("import", std::string("Error en modulo ") + canon + ": " + e.what(), 0);
    }

    if (is_error(resultado_ejec)) {
        cargando_modulo_ = cargando_previo;
        exports_modulo_actual_ = exports_previos;
        pila_imports_.pop_back();
        return resultado_ejec;
    }

    std::set<std::string> simbolos_agregados;
    for (const auto& [nombre, _] : ambito_global_->variables) {
        if (ambito_previo.find(nombre) == ambito_previo.end()) {
            simbolos_agregados.insert(nombre);
        }
    }

    std::set<std::string> publicos;
    if (exports_modulo_actual_.empty()) {
        publicos = simbolos_agregados;
    } else {
        for (const auto& exp : exports_modulo_actual_) {
            if (simbolos_agregados.count(exp)) publicos.insert(exp);
        }
    }

    ModuleSnapshot snap;
    snap.ruta = canon;
    snap.exports = publicos;
    snap.ast = ast_vivo;
    for (const auto& nombre : publicos) {
        auto vit = ambito_global_->variables.find(nombre);
        if (vit != ambito_global_->variables.end()) {
            snap.simbolos[nombre] = vit->second;
        }
    }

    std::map<std::string, ValorZeta> ns;
    for (const auto& nombre : publicos) {
        ns[nombre] = snap.simbolos[nombre];
    }

    ambito_global_->definir(alias, mk_dict(ns));

    modulos_cache_[canon] = std::move(snap);

    for (const auto& nombre : publicos) {
        ambito_global_->variables.erase(nombre);
    }

    cargando_modulo_ = cargando_previo;
    exports_modulo_actual_ = exports_previos;
    pila_imports_.pop_back();

    return mk_dict(ns);
}

ValorZeta Interpreter::cargar_selectivo(const std::string& ruta, const std::vector<std::string>& nombres) {
    fs::path ruta_abs = resolver_ruta_modulo(ruta);
    if (ruta_abs.empty()) {
        return mk_err("import", "No se encontro el modulo: " + ruta, 0);
    }
    std::string canon = canonicalizar_ruta(ruta_abs);

    auto it = modulos_cache_.find(canon);
    if (it != modulos_cache_.end()) {
        for (const auto& nombre : nombres) {
            auto sit = it->second.simbolos.find(nombre);
            if (sit == it->second.simbolos.end()) {
                return mk_err("import", "El modulo " + canon + " no exporta: " + nombre, 0);
            }
            ambito_global_->variables[nombre] = sit->second;
        }
        return mk_null_val();
    }

    if (std::find(pila_imports_.begin(), pila_imports_.end(), canon) != pila_imports_.end()) {
        return mk_err("import", "Ciclo de importacion detectado: " + canon, 0);
    }

    std::ifstream archivo(ruta_abs);
    if (!archivo.is_open()) {
        return mk_err("import", "No se pudo abrir: " + canon, 0);
    }
    std::stringstream ss;
    ss << archivo.rdbuf();
    std::string codigo = ss.str();

    pila_imports_.push_back(canon);

    auto ambito_previo = ambito_global_->variables;
    bool cargando_previo = cargando_modulo_;
    std::set<std::string> exports_previos = exports_modulo_actual_;
    cargando_modulo_ = true;
    exports_modulo_actual_.clear();

    std::shared_ptr<NodoAST> ast_vivo;
    ValorZeta resultado_ejec(mk_null_val());
    try {
        Lexer lexer(codigo);
        auto tokens = lexer.tokenizar();
        Parser parser(std::move(tokens));
        auto ast = parser.parsear();
        ast_vivo = std::move(ast);
        resultado_ejec = evaluar(*ast_vivo);
    } catch (const std::exception& e) {
        cargando_modulo_ = cargando_previo;
        exports_modulo_actual_ = exports_previos;
        pila_imports_.pop_back();
        return mk_err("import", std::string("Error en modulo ") + canon + ": " + e.what(), 0);
    }

    if (is_error(resultado_ejec)) {
        cargando_modulo_ = cargando_previo;
        exports_modulo_actual_ = exports_previos;
        pila_imports_.pop_back();
        return resultado_ejec;
    }

    std::set<std::string> simbolos_agregados;
    for (const auto& [nombre, _] : ambito_global_->variables) {
        if (ambito_previo.find(nombre) == ambito_previo.end()) {
            simbolos_agregados.insert(nombre);
        }
    }

    std::set<std::string> publicos;
    if (exports_modulo_actual_.empty()) {
        publicos = simbolos_agregados;
    } else {
        for (const auto& exp : exports_modulo_actual_) {
            if (simbolos_agregados.count(exp)) publicos.insert(exp);
        }
    }

    ModuleSnapshot snap;
    snap.ruta = canon;
    snap.exports = publicos;
    snap.ast = ast_vivo;
    for (const auto& nombre : publicos) {
        auto vit = ambito_global_->variables.find(nombre);
        if (vit != ambito_global_->variables.end()) {
            snap.simbolos[nombre] = vit->second;
        }
    }

    for (const auto& nombre : nombres) {
        auto sit = snap.simbolos.find(nombre);
        if (sit == snap.simbolos.end()) {
            modulos_cache_[canon] = std::move(snap);
            cargando_modulo_ = cargando_previo;
            exports_modulo_actual_ = exports_previos;
            pila_imports_.pop_back();
            return mk_err("import", "El modulo " + canon + " no exporta: " + nombre, 0);
        }
        ambito_global_->variables[nombre] = sit->second;
    }

    std::set<std::string> solicitados_set(nombres.begin(), nombres.end());
    std::vector<std::string> no_solicitados;
    for (const auto& nombre : publicos) {
        if (!solicitados_set.count(nombre)) {
            no_solicitados.push_back(nombre);
        }
    }

    modulos_cache_[canon] = std::move(snap);

    for (const auto& nombre : no_solicitados) {
        ambito_global_->variables.erase(nombre);
    }

    cargando_modulo_ = cargando_previo;
    exports_modulo_actual_ = exports_previos;
    pila_imports_.pop_back();

    return mk_null_val();
}

}
