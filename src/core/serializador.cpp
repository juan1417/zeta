#include "zeta/serializador.hpp"
#include "deps/json.hpp"
#include <cmath>

namespace zeta {

nlohmann::json valor_a_json(const ValorZeta& v) {
    if (!v) return nlohmann::json();
    switch (v->tipo) {
        case ValorImpl::NUM: {
            if (es_null(v->num_val)) return nlohmann::json();
            return v->num_val;
        }
        case ValorImpl::BOOL: return v->bool_val;
        case ValorImpl::STR: return v->str_val;
        case ValorImpl::VEC: {
            nlohmann::json j = nlohmann::json::array();
            for (double x : v->vec_val) {
                if (es_null(x)) j.push_back(nlohmann::json());
                else j.push_back(x);
            }
            return j;
        }
        case ValorImpl::BOOL_VEC: {
            nlohmann::json j = nlohmann::json::array();
            for (bool b : v->bool_vec_val) j.push_back(b);
            return j;
        }
        case ValorImpl::STR_VEC: {
            nlohmann::json j = nlohmann::json::array();
            for (const auto& s : v->str_vec_val) j.push_back(s);
            return j;
        }
        case ValorImpl::MATRIZ: {
            nlohmann::json j = nlohmann::json::array();
            for (const auto& fila : v->matriz_val) {
                nlohmann::json fj = nlohmann::json::array();
                for (double x : fila) {
                    if (es_null(x)) fj.push_back(nlohmann::json());
                    else fj.push_back(x);
                }
                j.push_back(fj);
            }
            return j;
        }
        case ValorImpl::DICT: {
            nlohmann::json j = nlohmann::json::object();
            for (const auto& [k, val] : v->dict_val) j[k] = valor_a_json(val);
            return j;
        }
        case ValorImpl::DF: return dataframe_a_json(v->df_val);
        case ValorImpl::ERR: return "<no serializable>";
        case ValorImpl::GRAFICO: return "<no serializable>";
        case ValorImpl::METRICA: {
            nlohmann::json j;
            j["nombre"] = v->metrica_val.nombre;
            j["valor"] = v->metrica_val.valor;
            return j;
        }
        case ValorImpl::DASHBOARD: return "<no serializable>";
        case ValorImpl::SCENE: return "<no serializable>";
        case ValorImpl::FUNC: return "<no serializable>";
        case ValorImpl::OBJ: return "<no serializable>";
        default: return nullptr;
    }
}

nlohmann::json dataframe_a_json(const DataFrame& df) {
    nlohmann::json j = nlohmann::json::object();
    for (const auto& [col_name, col] : df.columnas) {
        nlohmann::json arr = nlohmann::json::array();
        for (size_t i = 0; i < col.size(); ++i) {
            if (col.null_bitmap[i]) {
                arr.push_back(nlohmann::json());
            } else if (col.tipo == "num") {
                arr.push_back(col.nums[i]);
            } else if (col.tipo == "str") {
                arr.push_back(col.strs[i]);
            } else if (col.tipo == "bool") {
                arr.push_back(col.bools[i]);
            }
        }
        j[col_name] = arr;
    }
    return j;
}

nlohmann::json dashboard_a_json(const DashboardConfig& d) {
    nlohmann::json j;
    j["titulo"] = d.titulo;
    j["autor"] = d.autor;
    return j;
}

} // namespace zeta
