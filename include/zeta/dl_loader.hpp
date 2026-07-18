#pragma once
#include <string>
#include <map>
#include <memory>

#ifdef _WIN32
#include <windows.h>
using dl_handle = HMODULE;
#else
using dl_handle = void*;
#endif

namespace zeta {

class DlLibrary {
public:
    DlLibrary();
    ~DlLibrary();

    bool cargar(const std::string& ruta);
    void* obtener_simbolo(const std::string& nombre);
    std::string ultimo_error() const;

private:
    dl_handle handle_ = nullptr;
    std::string ruta_;
};

class DlRegistry {
public:
    static DlRegistry& instancia();

    DlLibrary* cargar(const std::string& ruta);
    void* obtener_simbolo(const std::string& ruta, const std::string& simbolo);

private:
    std::map<std::string, std::unique_ptr<DlLibrary>> libs_;
};

} // namespace zeta
