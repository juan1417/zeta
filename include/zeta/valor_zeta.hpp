#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <limits>
#include <cmath>
#include <functional>

namespace zeta {

struct TablaSimbolos;
struct ErrorZeta;
struct DataFrame;
struct GraficoConfig;
struct MetricaKPI;
struct DashboardConfig;
struct SceneSpec;
struct SceneNode;
struct SceneLayout;
struct ClassDef;
struct ObjetoZeta;
struct RutaRegistrada;

inline double crear_null() {
    return std::numeric_limits<double>::quiet_NaN();
}

inline bool es_null(double v) {
    return v != v;
}

struct ErrorZeta {
    std::string tipo;
    std::string mensaje;
    int linea;
};

struct Columna {
    std::string tipo;  // "num", "str", "bool"
    std::vector<double> nums;
    std::vector<std::string> strs;
    std::vector<bool> bools;
    std::vector<bool> null_bitmap;  // true = null en esa posicion

    Columna() : tipo("num") {}
    explicit Columna(const std::string& t) : tipo(t) {}

    size_t size() const {
        if (tipo == "num") return nums.size();
        if (tipo == "str") return strs.size();
        if (tipo == "bool") return bools.size();
        return 0;
    }

    bool es_null(size_t idx) const {
        if (idx >= null_bitmap.size()) return true;
        return null_bitmap[idx];
    }
};

struct DataFrame {
    std::vector<std::string> nombres_columnas;
    std::map<std::string, Columna> columnas;

    bool validar_simetria() const {
        if (columnas.empty()) return true;
        size_t n = columnas.begin()->second.size();
        for (const auto& [k, v] : columnas) {
            if (v.size() != n) return false;
        }
        return true;
    }

    size_t filas() const {
        if (columnas.empty()) return 0;
        return columnas.begin()->second.size();
    }

    size_t columnas_count() const {
        return columnas.size();
    }
};

struct GraficoConfig {
    std::string tipo_grafico;
    std::string titulo;
    std::string eje_x;
    std::string eje_y;
    int bins = 10;
};

struct MetricaKPI {
    std::string nombre;
    double valor;
};

struct DashboardConfig {
    std::string titulo;
    std::string autor;
};

struct SceneNode {
    std::string tipo;
    std::string id;
    std::string titulo;
    std::map<std::string, std::string> cols;
    std::map<std::string, double> nums;
    std::map<std::string, std::string> strs;
    std::map<std::string, std::vector<double>> series;
};

struct SceneLayout {
    std::string tipo = "grid";
    int cols = 2;
    int gap = 10;
    std::string background = "#1e1e1e";
};

struct SceneSpec {
    std::string titulo;
    std::string autor;
    SceneLayout layout;
    std::vector<SceneNode> nodes;
    double created_at = 0;
    double updated_at = 0;

    SceneNode* find_node(const std::string& id) {
        for (auto& n : nodes) {
            if (n.id == id) return &n;
        }
        return nullptr;
    }
};

struct ClassDef {
    std::string nombre;
    std::string padre;
    std::map<std::string, std::shared_ptr<struct ValorImpl>> campos_default;
    std::map<std::string, std::shared_ptr<struct ValorImpl>> metodos;
};

struct ObjetoZeta {
    std::string clase;
    std::map<std::string, std::shared_ptr<struct ValorImpl>> campos;
};

struct RutaRegistrada {
    std::string metodo;
    std::string path;
    std::shared_ptr<struct ValorImpl> handler;
};

struct ValorImpl {
    enum Tipo {
        BOOL, NUM, STR, VEC, BOOL_VEC, STR_VEC,
        MATRIZ, DICT, DF, ERR,
        GRAFICO, METRICA, DASHBOARD, SCENE,
        FUNC, OBJ
    };

    Tipo tipo;
    bool bool_val = false;
    double num_val = 0.0;
    std::string str_val;
    std::vector<double> vec_val;
    std::vector<bool> bool_vec_val;
    std::vector<std::string> str_vec_val;
    std::vector<std::vector<double>> matriz_val;
    std::map<std::string, std::shared_ptr<ValorImpl>> dict_val;
    DataFrame df_val;
    ErrorZeta err_val;
    GraficoConfig grafico_val;
    MetricaKPI metrica_val;
    DashboardConfig dashboard_val;
    std::shared_ptr<SceneSpec> scene_val;
    std::string func_nombre;
    std::vector<std::string> func_params;
    void* func_cuerpo = nullptr;
    std::shared_ptr<TablaSimbolos> func_cierre;
    std::shared_ptr<ObjetoZeta> obj_val;
};

using ValorZeta = std::shared_ptr<ValorImpl>;

ValorZeta mk_num(double v);
ValorZeta mk_str(const std::string& v);
ValorZeta mk_bool(bool v);
ValorZeta mk_vec(const std::vector<double>& v);
ValorZeta mk_bool_vec(const std::vector<bool>& v);
ValorZeta mk_str_vec(const std::vector<std::string>& v);
ValorZeta mk_matriz(const std::vector<std::vector<double>>& v);
ValorZeta mk_dict(const std::map<std::string, ValorZeta>& v);
ValorZeta mk_df(const DataFrame& v);
ValorZeta mk_df(DataFrame&& v);
ValorZeta mk_err(const std::string& tipo, const std::string& msg, int linea);
ValorZeta mk_grafico(const GraficoConfig& v);
ValorZeta mk_metrica(const MetricaKPI& v);
ValorZeta mk_dashboard(const DashboardConfig& v);
ValorZeta mk_scene(std::shared_ptr<SceneSpec> v);
ValorZeta mk_func(const std::string& nombre,
                   const std::vector<std::string>& params,
                   void* cuerpo,
                   std::shared_ptr<TablaSimbolos> cierre);
ValorZeta mk_obj(std::shared_ptr<ObjetoZeta> v);
ValorZeta mk_null_val();

double get_num(const ValorZeta& v);
const std::string& get_str(const ValorZeta& v);
bool get_bool(const ValorZeta& v);
const std::vector<double>& get_vec(const ValorZeta& v);

bool es_null_valor(const ValorZeta& v);
std::string tipo_nombre(const ValorZeta& v);

} // namespace zeta
