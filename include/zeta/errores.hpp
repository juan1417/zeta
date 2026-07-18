#pragma once
#include "zeta/valor_zeta.hpp"

namespace zeta {

ValorZeta crear_error(const std::string& tipo, const std::string& mensaje, int linea);
bool is_error(const ValorZeta& v);
ErrorZeta& obtener_error(const ValorZeta& v);

} // namespace zeta
