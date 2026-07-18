#pragma once
#include "zeta/valor_zeta.hpp"
#include <string>

namespace zeta {
    DataFrame load_xlsx_file(const std::string& ruta, int sheet_index = 0);
    std::string save_xlsx_file(const std::string& ruta, const DataFrame& df);
}
