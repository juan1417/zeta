# AGENTS.md — Zeta Language

## What this is

C++20 custom language ("Zeta") — a data analysis/transformation language with Rust/Python-inspired syntax. Pipeline: Lexer → Parser → AST → Interpreter. Outputs to CLI, HTTP server, or renderers (OpenGL dashboard, ANSI terminal).

Five binaries: `zeta` (CLI), `zeta_server` (HTTP/Crow), `zeta_dashboard` (OpenGL+ImGui+ImPlot), `zeta_term` (ANSI terminal), `zeta-lsp` (Language Server Protocol).

## Build system

Primary: `build.sh` (shell, uses `clang++ -std=c++20 -O2`).
CMake alternative: `CMakeLists.txt` — **out of sync with build.sh** (missing xlsx_reader, dl_loader, grafo_json, arena sources). Use `build.sh` as source of truth.

```
./build.sh cli        # zeta interpreter
./build.sh server     # zeta_server (HTTP)
./build.sh dashboard  # zeta_dashboard — requires GLFW prebuilt lib (deps/glfw-build/lib/libglfw3.a)
./build.sh term       # zeta_term (no OpenGL)
./build.sh lsp        # zeta-lsp (Language Server)
./build.sh all        # all five
./build.sh clean      # remove binaries
```

Compiler flags include `-Wno-deprecated-literal-operator -Wno-unused-variable -Wno-unused-but-set-variable`. These warnings are intentionally suppressed.

Build also compiles vendored C sources from `deps/miniz/` (miniz ZIP library) into `build_tmp/` as `.o` files. Requires `-lz` (zlib) at link time.

Dashboard build requires pre-built GLFW 3.4 static lib in `deps/glfw-build/lib/`. Headers exist but lib is missing — must be compiled from source or installed via system package.

## Source structure

### Headers (`include/zeta/`)
- `valor_zeta.hpp` — Core types: `ValorZeta` (shared_ptr<ValorImpl>), `DataFrame`, `ErrorZeta`, `SceneSpec`, factory functions (`mk_num`, `mk_str`, etc.) with move overloads
- `valor.hpp` — **NEW v0.2**: `Valor` value type — Rust-inspired stack-allocated values with arena backing. NUM/BOOL stored inline (0 heap), large types arena-backed.
- `arena.hpp` — **NEW v0.2**: Arena allocator — region-based memory for deterministic bulk reclamation. O(1) reset, no GC pauses.
- `lexer.hpp` — `TipoToken` enum (80+ values), `Token`, `Lexer` class
- `parser.hpp` — `TipoNodoAST` enum (50+ values), `NodoAST`, `Parser` class
- `interpreter.hpp` — `TablaSimbolos` (inline, raw parent pointer), `Interpreter` class with arena member
- `estadisticas.hpp` — `fn_mean`, `fn_count`, `fn_sum`, `fn_min`, `fn_max`, `fn_stddev`
- `errores.hpp` — `crear_error`, `is_error`, `obtener_error`
- `serializador.hpp` — `valor_a_json`, `dataframe_a_json`, `dashboard_a_json`
- `grafo_json.hpp` — `guardar_grafo_json`, `cargar_grafo_json`
- `dl_loader.hpp` — `DlLibrary`, `DlRegistry` (C ABI plugin loading)
- `zeta_abi.h` — `ZETA_API`, `zeta_fn_t` (C ABI for native plugins)
- `xlsx_reader.hpp` — `load_xlsx_file`, `save_xlsx_file` (Excel support via pugixml + miniz)

### Source (`src/`)
- `main.cpp` — CLI entrypoint (`zeta <file.zl> [--debug] --help`)
- `server_main.cpp` — HTTP server entrypoint (Crow, embeds HTML frontend with Tailwind+Chart.js via CDN)
- `renderer/main.cpp` — Dashboard entrypoint (OpenGL, uses stb_image_write for `--screenshot`)
- `term/main.cpp` — Terminal renderer entrypoint (ANSI 24-bit color, standalone — no zeta core deps)
- `lexer/lexer.cpp` — Tokenizer
- `parser/parser.cpp` — Parser → AST (1100+ lines)
- `interpreter/interpreter.cpp` — Interpreter (2800+ lines, core logic, arena integration, fast paths)
- `core/valor_zeta.cpp` — Factory functions for all 15 types with move overloads
- `core/arena.cpp` — Arena allocator (header-only, compilation anchor)
- `core/estadisticas.cpp` — Statistics functions (null-aware)
- `core/errores.cpp` — Error creation and checking
- `core/serializador.cpp` — Value → JSON serialization
- `core/grafo_json.cpp` — SceneSpec JSON save/load
- `core/xlsx_reader.cpp` — Excel file reading/writing (pugixml + miniz)
- `core/tabla_simbolos.cpp` — Stub (TablaSimbolos defined inline in interpreter.hpp)
- `dl_loader/dl_loader.cpp` — Dynamic library loading

### LSP (`lsp/`)
- `zeta-lsp.cpp` — LSP server entrypoint
- `transport.cpp/hpp` — JSON-RPC transport
- `analyzer.cpp/hpp` — Static analysis for completions/hover/go-to-def
- `builtins.cpp/hpp` — Built-in function signatures for autocompletion (60+ functions)

### Libraries (`lib/`)
- `statslib.zl` — Statistics library (importable from Zeta scripts)
- `statslib_restricted.zl` — Restricted version
- `test_lib.cpp` — Example native plugin (C ABI)

### Documentation (`docs/`)
- `guia_rapida.md` — Quick start guide
- `arquitectura.md` — Architecture deep dive
- `benchmarks.md` — Performance benchmarks
- `api_reference.md` — Built-in function reference
- `CHANGELOG.md` — Version history

### Root files
- `VERSION` — Current version string (`0.2.0`)
- `CHANGELOG.md` — Detailed changelog
- `package.sh` — Package distribution archives
- `install.sh` / `install.ps1` — Cross-platform installers
- `uninstall.sh` / `uninstall.ps1` — Uninstallers

## Testing

- **C++ unit tests**: `tests/test_core.cpp` — assert-based, no framework. Build with:
  ```
  clang++ -std=c++20 -I include tests/test_core.cpp src/core/valor_zeta.cpp src/core/estadisticas.cpp src/core/errores.cpp src/core/serializador.cpp -o test_zeta
  ./test_zeta
  ```
- **E2E tests**: `./test_e2e.sh` — full pipeline test. **Requires**: `python3`, `curl`, `rg` (ripgrep), and X11 for dashboard screenshot step. Uses port 8095. Kills existing `zeta_server`/`zeta_dashboard` processes on that port via `pkill -9`.
- **Zeta scripts**: `tests/*.zl` and `tests/*.zeta` — runnable via `./zeta <file>`.
- No CI workflows configured. No linting, formatting, or typecheck tooling.

## Dependencies (all vendored in `deps/`)

- `deps/asio/` — Asio (standalone, for server networking)
- `deps/crow_all.h` — Crow HTTP framework
- `deps/json.hpp` — nlohmann/json
- `deps/imgui/` — ImGui 1.91.8 (with GLFW+OpenGL3 backends)
- `deps/implot/` — ImPlot 0.16
- `deps/glfw-build/` — GLFW 3.4 headers (pre-built static lib missing)
- `deps/miniz/` — Miniz (ZIP compression, used for XLSX support)
- `deps/pugixml/` — pugixml (XML parser, used for XLSX support)

## Packaging and installation

- `./package.sh [version]` — creates `dist/zeta-<os>-<arch>.tar.gz`. Defaults to version from `VERSION` file. Builds cli/server/term; dashboard only on Linux.
- `./install.sh [prefix]` — builds and installs to `/usr/local` (or custom prefix). Needs `clang++` (or `g++` fallback) and `cc`/`gcc` for miniz. Supports `--help`, `--no-build`, `--no-dashboard`.
- `./uninstall.sh` — removes installed files (including `zeta-lsp`).

## Zeta language conventions

- Variables always prefixed with `$` (e.g. `$data`, `$count`)
- Collections: `<>` vectors, `<<>>` matrices, `{}` dicts
- Indexing: `[]` for vector, dict, and matrix (with comma for 2D)
- Colon access for DataFrame columns: `$df:column_name`
- Sigil `$` for variable binding, `?` for error propagation
- `null` is represented as NaN internally
- `.zl` and `.zeta` file extensions both work
- Numbers support: `1,000` (comma thousands), `1.5e3` (scientific), `PI`, `E`, `INFINITY` constants
- `format($num, $decimals)` for controlling decimal output

## Memory model (v0.2)

Zeta uses a hybrid memory model inspired by Rust:

### Arena allocator
```zeta
$bytes = arena_bytes()    # Query arena size
clear_arena()             # Reset arena — O(1) bulk reclamation
```

- Values are allocated in a region (arena)
- Individual `delete` is NOT possible (by design)
- `clear_arena()` frees ALL arena values at once
- Values stored in variables (via `shared_ptr`) are NOT affected by arena reset

### Valor value type
```cpp
// Stack-allocated, zero heap for small types
Valor v(42.0);           // NUM — stored in union (8 bytes)
Valor b(true);           // BOOL — stored in union (8 bytes)

// Arena-backed for large types
Valor s(valor_str("hello", arena));  // STR — 1 arena bump
Valor vec(valor_vec(std::move(v), arena));  // VEC — 1 arena bump

// Convert to ValorZeta for persistent storage
ValorZeta z = v.to_zeta();  // Creates heap copy only when needed
```

### Scope chains
```cpp
// TablaSimbolos::padre is a raw pointer (borrowed reference)
// Parent always outlives child — no refcount needed
TablaSimbolos* padre = nullptr;
```

### What this means for scripts
- `mk_num(42)` still works (creates shared_ptr, for backward compatibility)
- Arena values are for internal optimization — scripts don't see the difference
- `clear_arena()` can be called manually in long-running scripts to free memory
- No GC pauses — all cleanup is deterministic

## Built-in functions (v0.2)

### Memory management
- `clear_arena()` — Reset arena allocator
- `arena_bytes()` — Query arena allocation size
- `zeta_version()` — Return version string

### All previous functions still work (mean, count, sum, min, max, stddev, etc.)

## Gotchas

- `zeta_dashboard` requires X11 + pre-built GLFW — will fail on headless/SSH systems. Use `zeta_term` instead.
- The E2E test kills existing `zeta_server`/`zeta_dashboard` processes on port 8095 via `pkill -9`.
- `package.sh` version defaults to value in `VERSION` file if no argument given.
- No linting, formatting, or typecheck tooling configured — maintain existing code style.
- `CMakeLists.txt` is out of sync with `build.sh` — use `build.sh` as source of truth.
- `build_tmp/` contains intermediate `.o` files from miniz compilation — safe to delete.
- `zeta_server` embeds an HTML UI served at `/` — no separate frontend build step.
- `zeta_term` is standalone (no core deps) — it only reads JSON from the server's `/api/grafo`.
- The LSP binary `zeta-lsp` is in `.gitignore` — must be built locally.
- Arena allocator is header-only — no separate library needed.
