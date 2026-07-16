#pragma once
#include <string>
#include <vector>
#include <map>

namespace zeta_lsp {

struct BuiltinParam {
    std::string name;
    std::string type;
    bool optional = false;
};

struct BuiltinInfo {
    std::string name;
    std::vector<BuiltinParam> params;
    std::string returnType;
    std::string description;
    std::string example;
};

class BuiltinRegistry {
public:
    BuiltinRegistry();
    const std::vector<BuiltinInfo>& all() const { return builtins_; }
    const std::vector<BuiltinInfo>& search(const std::string& prefix) const;
    const BuiltinInfo* find(const std::string& name) const;

private:
    std::vector<BuiltinInfo> builtins_;
    std::map<std::string, const BuiltinInfo*> by_name_;
};

} // namespace zeta_lsp
