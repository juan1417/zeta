#pragma once
#include "zeta/parser.hpp"
#include "zeta/valor_zeta.hpp"
#include "zeta/arena.hpp"
#include "deps/json.hpp"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <filesystem>
#include <optional>

namespace zeta {

struct TablaSimbolos {
    TablaSimbolos* padre = nullptr;  // Borrowed reference — always outlives child
    std::map<std::string, ValorZeta> variables;

    TablaSimbolos() = default;
    explicit TablaSimbolos(TablaSimbolos* parent)
        : padre(parent) {}
    explicit TablaSimbolos(std::shared_ptr<TablaSimbolos> parent)
        : padre(parent.get()) {}

    void definir(const std::string& nombre, const ValorZeta& valor) {
        variables[nombre] = valor;
    }

    ValorZeta* buscar(const std::string& nombre) {
        auto it = variables.find(nombre);
        if (it != variables.end()) return &it->second;
        if (padre) return padre->buscar(nombre);
        return nullptr;
    }

    void actualizar(const std::string& nombre, const ValorZeta& valor) {
        auto it = variables.find(nombre);
        if (it != variables.end()) {
            it->second = valor;
        } else if (padre) {
            padre->actualizar(nombre, valor);
        } else {
            definir(nombre, valor);
        }
    }
};

struct ModuleSnapshot {
    std::string ruta;
    std::map<std::string, ValorZeta> simbolos;
    std::set<std::string> exports;
    std::shared_ptr<NodoAST> ast;
};

class Interpreter {
public:
    Interpreter();
    ValorZeta ejecutar(std::unique_ptr<NodoAST> ast);

    nlohmann::json obtener_datos_json();
    nlohmann::json obtener_dashboard_json();
    std::vector<nlohmann::json> obtener_metricas_json();
    nlohmann::json obtener_grafo_json();

    const std::vector<RutaRegistrada>& obtener_rutas() const;
    ValorZeta llamar_usuario_directo(const ValorZeta& handler, const std::vector<ValorZeta>& args);

    void agregar_include_path(const std::string& path);
    void set_script_path(const std::string& path);
    std::string resolver_ruta(const std::string& ruta) const;

private:
    std::shared_ptr<TablaSimbolos> ambito_global_;
    std::shared_ptr<NodoAST> ast_actual_;
    std::vector<MetricaKPI> metricas_;
    std::optional<DashboardConfig> dashboard_;
    std::vector<std::string> include_paths_;
    std::string script_dir_;
    std::map<std::string, ModuleSnapshot> modulos_cache_;
    std::vector<std::string> pila_imports_;
    bool cargando_modulo_ = false;
    std::set<std::string> exports_modulo_actual_;
    std::shared_ptr<SceneSpec> grafo_actual_;
    std::vector<RutaRegistrada> rutas_registradas_;
    std::map<std::string, std::shared_ptr<ClassDef>> clases_definidas_;
    ValorZeta this_actual_;

    // Arena allocator for temporary values during expression evaluation.
    // Reset after each top-level statement for deterministic memory reclamation.
    Arena valor_arena_;

    ValorZeta evaluar(const NodoAST& nodo);
    ValorZeta evaluar_variable(const NodoAST& nodo);
    ValorZeta evaluar_asignacion(const NodoAST& nodo);
    ValorZeta evaluar_binaria(const NodoAST& nodo);
    ValorZeta evaluar_unaria(const NodoAST& nodo);
    ValorZeta evaluar_ternaria(const NodoAST& nodo);
    ValorZeta evaluar_vector(const NodoAST& nodo);
    ValorZeta evaluar_matriz(const NodoAST& nodo);
    ValorZeta evaluar_diccionario(const NodoAST& nodo);
    ValorZeta evaluar_llamada_funcion(const NodoAST& nodo);
    ValorZeta evaluar_acceso_columnas(const NodoAST& nodo);
    ValorZeta evaluar_filtro_filas(const NodoAST& nodo);
    ValorZeta evaluar_acceso_indice(const NodoAST& nodo);
    ValorZeta evaluar_acceso_matriz(const NodoAST& nodo);
    ValorZeta evaluar_asignacion_indice(const NodoAST& nodo);
    ValorZeta evaluar_bloque(const NodoAST& nodo);
    ValorZeta evaluar_if(const NodoAST& nodo);
    ValorZeta evaluar_for(const NodoAST& nodo);
    ValorZeta evaluar_while(const NodoAST& nodo);
    ValorZeta evaluar_clase(const NodoAST& nodo);
    ValorZeta evaluar_new(const NodoAST& nodo);
    ValorZeta evaluar_this(const NodoAST& nodo);
    ValorZeta evaluar_llamada_metodo(const NodoAST& nodo);

    ValorZeta llamar_usuario(const ValorZeta& func, const std::vector<ValorZeta>& args);
    ValorZeta llamar_nativa(const std::string& nombre, const std::vector<ValorZeta>& args);
    std::string valor_a_string(const ValorZeta& val);

    std::vector<bool> fn_is_null_bool(const std::vector<double>& vec);

    std::filesystem::path resolver_ruta_modulo(const std::string& ruta) const;
    ValorZeta cargar_modulo(const std::string& ruta);
    ValorZeta cargar_con_alias(const std::string& ruta, const std::string& alias);
    ValorZeta cargar_selectivo(const std::string& ruta, const std::vector<std::string>& nombres);
};

} // namespace zeta
