#!/usr/bin/env python3
"""Generate zeta-types.ts from zeta.schema.json"""

import json
import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SCHEMA_PATH = os.path.join(SCRIPT_DIR, "zeta.schema.json")
OUTPUT_PATH = os.path.join(SCRIPT_DIR, "zeta-types.ts")


def main():
    with open(SCHEMA_PATH, "r") as f:
        schema = json.load(f)

    types = schema["types"]
    categories = sorted(schema["categories"], key=lambda c: c["order"])
    functions = schema["functions"]

    lines = []
    lines.append("// Auto-generated from zeta.schema.json — DO NOT EDIT")
    lines.append("// Run: python3 schema/gen_typescript.py")
    lines.append("")

    # --- ZetaType enum ---
    lines.append("export enum ZetaType {")
    for t_name in sorted(types.keys()):
        label = t_name.upper().replace(" ", "_")
        desc = types[t_name]["description"].replace('"', '\\"')
        lines.append(f'  {label} = "{t_name}",')
    lines.append("}")
    lines.append("")

    # --- ParamInfo interface ---
    lines.append("export interface ParamInfo {")
    lines.append("  name: string;")
    lines.append("  type: string;")
    lines.append("  optional: boolean;")
    lines.append("}")
    lines.append("")

    # --- FunctionInfo interface ---
    lines.append("export interface FunctionInfo {")
    lines.append("  name: string;")
    lines.append("  category: string;")
    lines.append("  description: string;")
    lines.append("  params: ParamInfo[];")
    lines.append("  returns: string;")
    lines.append("  examples: string[];")
    lines.append("  since: string;")
    lines.append("}")
    lines.append("")

    # --- CategoryInfo interface ---
    lines.append("export interface CategoryInfo {")
    lines.append("  id: string;")
    lines.append("  label: string;")
    lines.append("  order: number;")
    lines.append("}")
    lines.append("")

    # --- ZetaSchema interface ---
    lines.append("export interface ZetaSchema {")
    lines.append("  title: string;")
    lines.append("  version: string;")
    lines.append("  types: Record<string, { description: string }>;")
    lines.append("  categories: CategoryInfo[];")
    lines.append("  functions: FunctionInfo[];")
    lines.append("}")
    lines.append("")

    # --- Const arrays ---
    # ZETA_TYPES
    lines.append("export const ZETA_TYPES: Record<string, string> = {")
    for t_name in sorted(types.keys()):
        desc = types[t_name]["description"].replace('"', '\\"')
        lines.append(f'  "{t_name}": "{desc}",')
    lines.append("};")
    lines.append("")

    # ZETA_FUNCTIONS
    lines.append("export const ZETA_FUNCTIONS: FunctionInfo[] = [")
    for func in functions:
        lines.append("  {")
        lines.append(f'    name: "{func["name"]}",')
        lines.append(f'    category: "{func["category"]}",')
        desc = func["description"].replace('"', '\\"')
        lines.append(f'    description: "{desc}",')
        lines.append("    params: [")
        for p in func["params"]:
            opt = "true" if p.get("optional", False) else "false"
            lines.append(f'      {{ name: "{p["name"]}", type: "{p["type"]}", optional: {opt} }},')
        lines.append("    ],")
        lines.append(f'    returns: "{func["returns"]}",')
        lines.append("    examples: [")
        for ex in func["examples"]:
            ex_escaped = ex.replace('"', '\\"')
            lines.append(f'      "{ex_escaped}",')
        lines.append("    ],")
        lines.append(f'    since: "{func.get("since", "0.1.0")}",')
        lines.append("  },")
    lines.append("];")
    lines.append("")

    # ZETA_CATEGORIES
    lines.append("export const ZETA_CATEGORIES: CategoryInfo[] = [")
    for cat in categories:
        lines.append(f'  {{ id: "{cat["id"]}", label: "{cat["label"]}", order: {cat["order"]} }},')
    lines.append("];")
    lines.append("")

    # --- Helper functions ---
    lines.append("export function getFunctionsByCategory(category: string): FunctionInfo[] {")
    lines.append("  return ZETA_FUNCTIONS.filter((f) => f.category === category);")
    lines.append("}")
    lines.append("")

    lines.append("export function searchFunctions(query: string): FunctionInfo[] {")
    lines.append("  const q = query.toLowerCase();")
    lines.append("  return ZETA_FUNCTIONS.filter(")
    lines.append("    (f) =>")
    lines.append("      f.name.toLowerCase().includes(q) ||")
    lines.append("      f.description.toLowerCase().includes(q) ||")
    lines.append('      f.params.some((p) => p.name.toLowerCase().includes(q))')
    lines.append("  );")
    lines.append("}")
    lines.append("")

    lines.append("export function getFunctionByName(name: string): FunctionInfo | undefined {")
    lines.append("  return ZETA_FUNCTIONS.find((f) => f.name === name);")
    lines.append("}")
    lines.append("")

    os.makedirs(os.path.dirname(OUTPUT_PATH), exist_ok=True)
    with open(OUTPUT_PATH, "w") as f:
        f.write("\n".join(lines))

    print(f"Generated {OUTPUT_PATH} ({len(functions)} functions, {len(categories)} categories)")


if __name__ == "__main__":
    main()
