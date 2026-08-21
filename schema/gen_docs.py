#!/usr/bin/env python3
"""Generate docs/05-funciones-nativas-generated.md from zeta.schema.json"""

import json
import os
import sys

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SCHEMA_PATH = os.path.join(SCRIPT_DIR, "zeta.schema.json")
OUTPUT_PATH = os.path.join(SCRIPT_DIR, "..", "docs", "05-funciones-nativas-generated.md")


def main():
    with open(SCHEMA_PATH, "r") as f:
        schema = json.load(f)

    categories = sorted(schema["categories"], key=lambda c: c["order"])
    functions = schema["functions"]

    # Group by category
    grouped = {}
    for func in functions:
        grouped.setdefault(func["category"], []).append(func)

    lines = []

    # --- Title ---
    lines.append("# Funciones Nativas de Zeta (Generadas)")
    lines.append("")
    lines.append(f"> Auto-generated from `zeta.schema.json` v{schema.get('version', '?')}.")
    lines.append(f"> Total: **{len(functions)}** funciones en **{len(categories)}** categorías.")
    lines.append("")

    # --- TOC ---
    lines.append("## Tabla de Contenidos")
    lines.append("")
    for cat in categories:
        cat_id = cat["id"]
        cat_label = cat["label"]
        count = len(grouped.get(cat_id, []))
        anchor = cat_label.lower().replace(" ", "-").replace("/", "")
        lines.append(f"- [{cat_label}](#{anchor}) ({count})")
    lines.append("")

    # --- Summary table ---
    lines.append("## Resumen por Categoría")
    lines.append("")
    lines.append("| Categoría | Funciones | Cantidad |")
    lines.append("|-----------|-----------|----------|")
    for cat in categories:
        cat_id = cat["id"]
        cat_label = cat["label"]
        funcs = grouped.get(cat_id, [])
        names = ", ".join(f"`{f['name']}`" for f in funcs)
        lines.append(f"| {cat_label} | {names} | {len(funcs)} |")
    lines.append("")

    # --- Per-category sections ---
    for cat in categories:
        cat_id = cat["id"]
        cat_label = cat["label"]
        funcs = grouped.get(cat_id, [])
        if not funcs:
            continue

        lines.append(f"## {cat_label}")
        lines.append("")

        # Quick reference table
        lines.append("### Referencia Rápida")
        lines.append("")
        lines.append("| Función | Parámetros | Retorna | Descripción |")
        lines.append("|---------|------------|---------|-------------|")
        for func in funcs:
            params_parts = []
            for p in func["params"]:
                opt = " (opt)" if p.get("optional", False) else ""
                params_parts.append(f"*{p['name']}*: `{p['type']}`{opt}")
            params_str = ", ".join(params_parts) if params_parts else "—"
            returns = f"`{func['returns']}`"
            desc = func["description"]
            lines.append(f"| `{func['name']}` | {params_str} | {returns} | {desc} |")
        lines.append("")

        # Detailed docs
        lines.append("### Detalle")
        lines.append("")
        for func in funcs:
            name = func["name"]
            desc = func["description"]
            ret = func["returns"]
            since = func.get("since", "0.1.0")

            lines.append(f"#### `{name}`")
            lines.append("")
            lines.append(f"**Descripción:** {desc}")
            lines.append("")
            lines.append(f"**Retorna:** `{ret}` | **Desde:** v{since}")
            lines.append("")

            if func["params"]:
                lines.append("**Parámetros:**")
                lines.append("")
                lines.append("| Nombre | Tipo | Requerido | Descripción |")
                lines.append("|--------|------|-----------|-------------|")
                for p in func["params"]:
                    req = "No" if p.get("optional", False) else "Sí"
                    lines.append(f"| `{p['name']}` | `{p['type']}` | {req} | — |")
                lines.append("")

            if func["examples"]:
                lines.append("**Ejemplo:**")
                lines.append("")
                lines.append("```zeta")
                for ex in func["examples"]:
                    lines.append(ex)
                lines.append("```")
                lines.append("")

        lines.append("---")
        lines.append("")

    # --- Footer ---
    lines.append("*Generado automáticamente por `schema/gen_docs.py`*")
    lines.append("")

    os.makedirs(os.path.dirname(OUTPUT_PATH), exist_ok=True)
    with open(OUTPUT_PATH, "w") as f:
        f.write("\n".join(lines))

    print(f"Generated {OUTPUT_PATH} ({len(functions)} functions, {len(categories)} categories)")


if __name__ == "__main__":
    main()
