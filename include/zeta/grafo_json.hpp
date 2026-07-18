#pragma once
#include "zeta/valor_zeta.hpp"
#include "deps/json.hpp"
#include <string>
#include <memory>

namespace zeta {

std::string guardar_grafo_json(const std::string& ruta, const SceneSpec& s);
std::shared_ptr<SceneSpec> cargar_grafo_json(const std::string& ruta);

} // namespace zeta
