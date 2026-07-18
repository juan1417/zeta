#include "zeta/valor_zeta.hpp"

namespace zeta {

ValorZeta mk_num(double v) {
    auto p = std::make_shared<ValorImpl>();
    p->tipo = ValorImpl::NUM;
    p->num_val = v;
    return p;
}

ValorZeta mk_str(const std::string& v) {
    auto p = std::make_shared<ValorImpl>();
    p->tipo = ValorImpl::STR;
    p->str_val = v;
    return p;
}

ValorZeta mk_bool(bool v) {
    auto p = std::make_shared<ValorImpl>();
    p->tipo = ValorImpl::BOOL;
    p->bool_val = v;
    return p;
}

ValorZeta mk_vec(const std::vector<double>& v) {
    auto p = std::make_shared<ValorImpl>();
    p->tipo = ValorImpl::VEC;
    p->vec_val = v;
    return p;
}

ValorZeta mk_bool_vec(const std::vector<bool>& v) {
    auto p = std::make_shared<ValorImpl>();
    p->tipo = ValorImpl::BOOL_VEC;
    p->bool_vec_val = v;
    return p;
}

ValorZeta mk_str_vec(const std::vector<std::string>& v) {
    auto p = std::make_shared<ValorImpl>();
    p->tipo = ValorImpl::STR_VEC;
    p->str_vec_val = v;
    return p;
}

ValorZeta mk_matriz(const std::vector<std::vector<double>>& v) {
    auto p = std::make_shared<ValorImpl>();
    p->tipo = ValorImpl::MATRIZ;
    p->matriz_val = v;
    return p;
}

ValorZeta mk_dict(const std::map<std::string, ValorZeta>& v) {
    auto p = std::make_shared<ValorImpl>();
    p->tipo = ValorImpl::DICT;
    p->dict_val = v;
    return p;
}

ValorZeta mk_df(const DataFrame& v) {
    auto p = std::make_shared<ValorImpl>();
    p->tipo = ValorImpl::DF;
    p->df_val = v;
    return p;
}

ValorZeta mk_df(DataFrame&& v) {
    auto p = std::make_shared<ValorImpl>();
    p->tipo = ValorImpl::DF;
    p->df_val = std::move(v);
    return p;
}

ValorZeta mk_err(const std::string& tipo, const std::string& msg, int linea) {
    auto p = std::make_shared<ValorImpl>();
    p->tipo = ValorImpl::ERR;
    p->err_val = {tipo, msg, linea};
    return p;
}

ValorZeta mk_grafico(const GraficoConfig& v) {
    auto p = std::make_shared<ValorImpl>();
    p->tipo = ValorImpl::GRAFICO;
    p->grafico_val = v;
    return p;
}

ValorZeta mk_metrica(const MetricaKPI& v) {
    auto p = std::make_shared<ValorImpl>();
    p->tipo = ValorImpl::METRICA;
    p->metrica_val = v;
    return p;
}

ValorZeta mk_dashboard(const DashboardConfig& v) {
    auto p = std::make_shared<ValorImpl>();
    p->tipo = ValorImpl::DASHBOARD;
    p->dashboard_val = v;
    return p;
}

ValorZeta mk_scene(std::shared_ptr<SceneSpec> v) {
    auto p = std::make_shared<ValorImpl>();
    p->tipo = ValorImpl::SCENE;
    p->scene_val = v;
    return p;
}

ValorZeta mk_func(const std::string& nombre,
                   const std::vector<std::string>& params,
                   void* cuerpo,
                   std::shared_ptr<TablaSimbolos> cierre) {
    auto p = std::make_shared<ValorImpl>();
    p->tipo = ValorImpl::FUNC;
    p->func_nombre = nombre;
    p->func_params = params;
    p->func_cuerpo = cuerpo;
    p->func_cierre = cierre;
    return p;
}

ValorZeta mk_obj(std::shared_ptr<ObjetoZeta> v) {
    auto p = std::make_shared<ValorImpl>();
    p->tipo = ValorImpl::OBJ;
    p->obj_val = v;
    return p;
}

ValorZeta mk_null_val() {
    auto p = std::make_shared<ValorImpl>();
    p->tipo = ValorImpl::NUM;
    p->num_val = crear_null();
    return p;
}

double get_num(const ValorZeta& v) { return v->num_val; }
const std::string& get_str(const ValorZeta& v) { return v->str_val; }
bool get_bool(const ValorZeta& v) { return v->bool_val; }
const std::vector<double>& get_vec(const ValorZeta& v) { return v->vec_val; }

bool es_null_valor(const ValorZeta& v) {
    if (!v) return true;
    if (v->tipo == ValorImpl::NUM) return es_null(v->num_val);
    return false;
}

std::string tipo_nombre(const ValorZeta& v) {
    if (!v) return "null";
    switch (v->tipo) {
        case ValorImpl::BOOL:      return "bool";
        case ValorImpl::NUM:       return "double";
        case ValorImpl::STR:       return "string";
        case ValorImpl::VEC:       return "vector";
        case ValorImpl::BOOL_VEC:  return "bool_vec";
        case ValorImpl::STR_VEC:   return "str_vec";
        case ValorImpl::MATRIZ:    return "matriz";
        case ValorImpl::DICT:      return "dict";
        case ValorImpl::DF:        return "df";
        case ValorImpl::ERR:       return "error";
        case ValorImpl::GRAFICO:   return "grafico";
        case ValorImpl::METRICA:   return "metrica";
        case ValorImpl::DASHBOARD: return "dashboard";
        case ValorImpl::SCENE:     return "scene";
        case ValorImpl::FUNC:      return "func";
        case ValorImpl::OBJ:       return v->obj_val ? v->obj_val->clase : "objeto";
        default:                   return "unknown";
    }
}

} // namespace zeta
