#include "zeta/errores.hpp"

namespace zeta {

ValorZeta crear_error(const std::string& tipo, const std::string& mensaje, int linea) {
    return mk_err(tipo, mensaje, linea);
}

bool is_error(const ValorZeta& v) {
    return v && v->tipo == ValorImpl::ERR;
}

ErrorZeta& obtener_error(const ValorZeta& v) {
    return v->err_val;
}

} // namespace zeta
