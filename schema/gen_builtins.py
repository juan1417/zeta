#!/usr/bin/env python3
"""Generate lsp/builtins.cpp from zeta.schema.json"""

import json
import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SCHEMA_PATH = os.path.join(SCRIPT_DIR, "zeta.schema.json")
OUTPUT_PATH = os.path.join(SCRIPT_DIR, "..", "lsp", "builtins.cpp")

# Map schema types to the C++ types used in the existing builtins.cpp
TYPE_MAP = {
    "num": "number",
    "str": "string",
    "bool": "bool",
    "vec": "vector",
    "bool_vec": "vector<bool>",
    "str_vec": "vector<string>",
    "matriz": "matrix",
    "dict": "dict",
    "df": "DataFrame",
    "err": "error",
    "scene": "scene",
    "func": "function",
    "null": "null",
    "any": "any",
    # Compound types pass through
    "df|vec": "DataFrame|vector",
    "str|num": "string|number",
    "any...": "any...",
}


def map_type(t):
    return TYPE_MAP.get(t, t)


def category_comment(cat_id, cat_label):
    return f"        // {cat_label}"


def gen_param(p):
    name = p["name"]
    typ = map_type(p["type"])
    optional = p.get("optional", False)
    if optional:
        return f'{{"{name}", "{typ}", true}}'
    return f'{{"{name}", "{typ}"}}'


def escape_cpp(s):
    return s.replace("\\", "\\\\").replace('"', '\\"')


def gen_function_entry(func):
    name = func["name"]
    ret = map_type(func["returns"])
    desc = escape_cpp(func["description"])
    example = escape_cpp(func["examples"][0]) if func["examples"] else ""
    params_str = ", ".join(gen_param(p) for p in func["params"])
    return f'        {{"{name}", {{{params_str}}}, "{ret}", "{desc}", "{example}"}},'


def main():
    with open(SCHEMA_PATH, "r") as f:
        schema = json.load(f)

    categories = {c["id"]: c for c in schema["categories"]}
    functions = schema["functions"]

    # Group functions by category, preserving category order
    ordered_cats = sorted(schema["categories"], key=lambda c: c["order"])
    grouped = {}
    for func in functions:
        cat = func["category"]
        grouped.setdefault(cat, []).append(func)

    lines = []
    lines.append('#include "builtins.hpp"')
    lines.append("")
    lines.append("namespace zeta_lsp {")
    lines.append("")
    lines.append("BuiltinRegistry::BuiltinRegistry() {")
    lines.append("    builtins_ = {")

    for cat in ordered_cats:
        cat_id = cat["id"]
        cat_label = cat["label"]
        funcs = grouped.get(cat_id, [])
        if not funcs:
            continue
        lines.append(category_comment(cat_id, cat_label))
        for func in funcs:
            lines.append(gen_function_entry(func))
        lines.append("")

    lines.append("    };")
    lines.append("")
    lines.append("    for (auto& b : builtins_) {")
    lines.append("        by_name_[b.name] = &b;")
    lines.append("    }")
    lines.append("}")
    lines.append("")
    lines.append("const std::vector<BuiltinInfo>& BuiltinRegistry::search(const std::string& prefix) const {")
    lines.append("    static std::vector<BuiltinInfo> results;")
    lines.append("    results.clear();")
    lines.append("    for (auto& b : builtins_) {")
    lines.append('        if (b.name.substr(0, prefix.size()) == prefix) {')
    lines.append("            results.push_back(b);")
    lines.append("        }")
    lines.append("    }")
    lines.append("    return results;")
    lines.append("}")
    lines.append("")
    lines.append("const BuiltinInfo* BuiltinRegistry::find(const std::string& name) const {")
    lines.append("    auto it = by_name_.find(name);")
    lines.append("    return it != by_name_.end() ? it->second : nullptr;")
    lines.append("}")
    lines.append("")
    lines.append("} // namespace zeta_lsp")
    lines.append("")

    os.makedirs(os.path.dirname(OUTPUT_PATH), exist_ok=True)
    with open(OUTPUT_PATH, "w") as f:
        f.write("\n".join(lines))

    print(f"Generated {OUTPUT_PATH} ({len(functions)} functions in {len(ordered_cats)} categories)")


if __name__ == "__main__":
    main()
