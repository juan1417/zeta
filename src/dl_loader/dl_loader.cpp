#include "zeta/dl_loader.hpp"
#include <stdexcept>
#include <algorithm>
#include <iostream>
#ifndef _WIN32
#include <dlfcn.h>
#endif

namespace zeta {

// ============================================================
// DlLibrary - Low-level dynamic library loading
// ============================================================

DlLibrary::DlLibrary() = default;

DlLibrary::~DlLibrary() {
    if (handle_) {
#ifdef _WIN32
        FreeLibrary(handle_);
#else
        dlclose(handle_);
#endif
    }
}

bool DlLibrary::cargar(const std::string& ruta) {
    ruta_ = ruta;
#ifdef _WIN32
    handle_ = LoadLibraryA(ruta.c_str());
    if (!handle_) {
        DWORD err = GetLastError();
        throw std::runtime_error("LoadLibrary failed: code " + std::to_string(err));
    }
#else
    handle_ = dlopen(ruta.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!handle_) {
        const char* err = dlerror();
        throw std::runtime_error(std::string("dlopen failed: ") + (err ? err : "unknown"));
    }
#endif
    return true;
}

void* DlLibrary::obtener_simbolo(const std::string& nombre) {
    if (!handle_) {
        throw std::runtime_error("Library not loaded");
    }
#ifdef _WIN32
    void* sym = reinterpret_cast<void*>(GetProcAddress(handle_, nombre.c_str()));
    if (!sym) {
        DWORD err = GetLastError();
        throw std::runtime_error("GetProcAddress failed for '" + nombre + "': code " + std::to_string(err));
    }
#else
    dlerror();
    void* sym = dlsym(handle_, nombre.c_str());
    const char* err = dlerror();
    if (err) {
        throw std::runtime_error(std::string("dlsym failed: ") + err);
    }
#endif
    return sym;
}

std::string DlLibrary::ultimo_error() const {
#ifdef _WIN32
    DWORD err = GetLastError();
    return "Windows error " + std::to_string(err);
#else
    const char* err = dlerror();
    return err ? err : "";
#endif
}

// ============================================================
// ZetaPlugin - High-level plugin with metadata
// ============================================================

ZetaPlugin::ZetaPlugin() = default;

ZetaPlugin::~ZetaPlugin() = default;

bool ZetaPlugin::cargar(const std::string& ruta) {
    try {
        lib_ = std::make_unique<DlLibrary>();
        lib_->cargar(ruta);
        
        // Try to load info function (v2 ABI)
        try {
            info_fn_ = reinterpret_cast<zeta_info_fn>(
                lib_->obtener_simbolo("zeta_info")
            );
            cargar_metadata();
        } catch (...) {
            // No info function - use filename as name
            info_.name = ruta;
            info_.version = "unknown";
            info_.description = "Plugin loaded from " + ruta;
        }
        
        // Load functions
        cargar_funciones();
        
        return true;
    } catch (const std::exception& e) {
        error_ = e.what();
        return false;
    }
}

void ZetaPlugin::cargar_metadata() {
    if (!info_fn_) return;
    
    ZetaPluginInfo* raw_info = info_fn_();
    if (raw_info) {
        info_.name = raw_info->name ? raw_info->name : "unknown";
        info_.version = raw_info->version ? raw_info->version : "0.0.0";
        info_.description = raw_info->description ? raw_info->description : "";
        info_.author = raw_info->author ? raw_info->author : "";
    }
}

void ZetaPlugin::cargar_funciones() {
    if (!lib_) return;
    
    // Try common function naming patterns
    // For v2: look for functions with _v2 suffix or specific names
    // For v1: look for functions directly
    
    // This is a simple heuristic - in practice, plugins should
    // register their functions through the info mechanism
}

double ZetaPlugin::call_v1(const std::string& func_name, int argc, const double* argv) {
    if (!lib_) {
        throw std::runtime_error("Plugin not loaded");
    }
    
    // Try to find v1 function
    void* sym = lib_->obtener_simbolo(func_name.c_str());
    if (sym) {
        zeta_fn_v1 fn = reinterpret_cast<zeta_fn_v1>(sym);
        return fn(argc, argv);
    }
    
    throw std::runtime_error("Function not found: " + func_name);
}

ZetaValue ZetaPlugin::call_v2(const std::string& func_name, int argc, ZetaValue* argv) {
    if (!lib_) {
        throw std::runtime_error("Plugin not loaded");
    }
    
    // Try v2 function name (e.g., "zeta_median")
    std::string v2_name = "zeta_" + func_name;
    void* sym = lib_->obtener_simbolo(v2_name.c_str());
    if (sym) {
        zeta_fn_v2 fn = reinterpret_cast<zeta_fn_v2>(sym);
        return fn(argc, argv);
    }
    
    // Try original name
    sym = lib_->obtener_simbolo(func_name.c_str());
    if (sym) {
        // Assume v1 if no v2 found
        zeta_fn_v1 fn = reinterpret_cast<zeta_fn_v1>(sym);
        double result = fn(argc, nullptr);
        return ZETA_NUM(result);
    }
    
    throw std::runtime_error("Function not found: " + func_name);
}

bool ZetaPlugin::tiene_funcion(const std::string& name) const {
    // Check if function exists in library
    if (!lib_) return false;
    
    try {
        const_cast<DlLibrary*>(lib_.get())->obtener_simbolo(name);
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<std::string> ZetaPlugin::funciones() const {
    // Return known functions
    // In a real implementation, we'd enumerate symbols
    return {};
}

// ============================================================
// DlRegistry - Global plugin registry
// ============================================================

DlRegistry& DlRegistry::instancia() {
    static DlRegistry inst;
    return inst;
}

ZetaPlugin* DlRegistry::cargar_plugin(const std::string& ruta) {
    auto it = plugins_.find(ruta);
    if (it != plugins_.end()) {
        return it->second.get();
    }
    
    auto plugin = std::make_unique<ZetaPlugin>();
    if (!plugin->cargar(ruta)) {
        return nullptr;
    }
    
    ZetaPlugin* ptr = plugin.get();
    plugins_[ruta] = std::move(plugin);
    return ptr;
}

ZetaPlugin* DlRegistry::obtener_plugin(const std::string& ruta) {
    auto it = plugins_.find(ruta);
    return (it != plugins_.end()) ? it->second.get() : nullptr;
}

// Legacy support
DlLibrary* DlRegistry::cargar(const std::string& ruta) {
    auto it = libs_.find(ruta);
    if (it != libs_.end()) {
        return it->second.get();
    }
    auto lib = std::make_unique<DlLibrary>();
    lib->cargar(ruta);
    DlLibrary* ptr = lib.get();
    libs_[ruta] = std::move(lib);
    return ptr;
}

void* DlRegistry::obtener_simbolo(const std::string& ruta, const std::string& simbolo) {
    DlLibrary* lib = cargar(ruta);
    return lib->obtener_simbolo(simbolo);
}

}
