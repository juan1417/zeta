#include "zeta/dl_loader.hpp"
#include <stdexcept>
#ifndef _WIN32
#include <dlfcn.h>
#endif

namespace zeta {

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
        throw std::runtime_error("LoadLibrary fallo: codigo " + std::to_string(err));
    }
#else
    handle_ = dlopen(ruta.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (!handle_) {
        const char* err = dlerror();
        throw std::runtime_error(std::string("dlopen fallo: ") + (err ? err : "desconocido"));
    }
#endif
    return true;
}

void* DlLibrary::obtener_simbolo(const std::string& nombre) {
    if (!handle_) {
        throw std::runtime_error("Libreria no cargada");
    }
#ifdef _WIN32
    void* sym = reinterpret_cast<void*>(GetProcAddress(handle_, nombre.c_str()));
    if (!sym) {
        DWORD err = GetLastError();
        throw std::runtime_error("GetProcAddress fallo para '" + nombre + "': codigo " + std::to_string(err));
    }
#else
    dlerror();
    void* sym = dlsym(handle_, nombre.c_str());
    const char* err = dlerror();
    if (err) {
        throw std::runtime_error(std::string("dlsym fallo: ") + err);
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

DlRegistry& DlRegistry::instancia() {
    static DlRegistry inst;
    return inst;
}

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
