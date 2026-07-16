#pragma once
#include "json.hpp"

namespace zeta_lsp {

using json = nlohmann::json;

class Transport {
public:
    json read_message();
    void write_message(const json& msg);
};

} // namespace zeta_lsp
