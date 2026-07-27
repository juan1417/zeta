#pragma once
#include <string>
#include <map>
#include <memory>
#include <vector>
#include "zeta/zeta_abi.h"

#ifdef _WIN32
#include <windows.h>
using dl_handle = HMODULE;
#else
using dl_handle = void*;
#endif

namespace zeta {

// ============================================================
// DlLibrary - Low-level dynamic library loading
// ============================================================
class DlLibrary {
public:
    DlLibrary();
    ~DlLibrary();

    bool cargar(const std::string& ruta);
    void* obtener_simbolo(const std::string& nombre);
    std::string ultimo_error() const;

    dl_handle handle() const { return handle_; }
    const std::string& ruta() const { return ruta_; }

private:
    dl_handle handle_ = nullptr;
    std::string ruta_;
};

// ============================================================
// ZetaPlugin - High-level plugin with metadata
// ============================================================
struct PluginInfo {
    std::string name;
    std::string version;
    std::string description;
    std::string author;
};

class ZetaPlugin {
public:
    ZetaPlugin();
    ~ZetaPlugin();

    // Load plugin from .so/.dll file
    bool cargar(const std::string& ruta);

    // Get plugin metadata
    const PluginInfo& info() const { return info_; }

    // Call v1 function (legacy, double only)
    double call_v1(const std::string& func_name, int argc, const double* argv);

    // Call v2 function (extended types)
    ZetaValue call_v2(const std::string& func_name, int argc, ZetaValue* argv);

    // Check if function exists
    bool tiene_funcion(const std::string& name) const;

    // Get all function names
    std::vector<std::string> funciones() const;

    // Error handling
    std::string ultimo_error() const { return error_; }

private:
    std::unique_ptr<DlLibrary> lib_;
    PluginInfo info_;
    std::string error_;

    // Function pointers
    zeta_info_fn info_fn_ = nullptr;
    std::map<std::string, zeta_fn_v1> funcs_v1_;
    std::map<std::string, zeta_fn_v2> funcs_v2_;

    void cargar_metadata();
    void cargar_funciones();
};

// ============================================================
// DlRegistry - Global plugin registry
// ============================================================
class DlRegistry {
public:
    static DlRegistry& instancia();

    // Load plugin
    ZetaPlugin* cargar_plugin(const std::string& ruta);

    // Get loaded plugin
    ZetaPlugin* obtener_plugin(const std::string& ruta);

    // Get all loaded plugins
    const std::map<std::string, std::unique_ptr<ZetaPlugin>>& plugins() const { return plugins_; }

    // Legacy support
    DlLibrary* cargar(const std::string& ruta);
    void* obtener_simbolo(const std::string& ruta, const std::string& simbolo);

private:
    std::map<std::string, std::unique_ptr<DlLibrary>> libs_;  // Legacy
    std::map<std::string, std::unique_ptr<ZetaPlugin>> plugins_;
};

} // namespace zeta
