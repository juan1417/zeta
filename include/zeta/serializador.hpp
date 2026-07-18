#pragma once
#include "zeta/valor_zeta.hpp"
#include "deps/json.hpp"

namespace zeta {

nlohmann::json valor_a_json(const ValorZeta& v);
nlohmann::json dataframe_a_json(const DataFrame& df);
nlohmann::json dashboard_a_json(const DashboardConfig& d);

} // namespace zeta
