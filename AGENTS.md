# AGENTS.md — Zeta Language

## What this is

C++20 custom language ("Zeta") — a data analysis/transformation language with Rust/Python-inspired syntax. Pipeline: Lexer → Parser → AST → Interpreter. Outputs to CLI, HTTP server, or renderers (OpenGL dashboard, ANSI terminal).

Four binaries: `zeta` (CLI), `zeta_server` (HTTP/Crow), `zeta_dashboard` (OpenGL+ImGui+ImPlot), `zeta_term` (ANSI terminal).

## Build system

Primary: `build.sh` (shell, uses `clang++ -std=c++20 -O2`).
CMake alternative: `CMakeLists.txt` — **out of sync with build.sh**, don't rely on it.

```
./build.sh cli        # zeta interpreter ✅ compiles
./build.sh server     # zeta_server (HTTP) ✅ compiles
./build.sh dashboard  # zeta_dashboard — requires GLFW prebuilt lib (deps/glfw-build/lib/libglfw3.a)
./build.sh term       # zeta_term (no OpenGL) ✅ compiles
./build.sh all        # all four
./build.sh clean      # remove binaries
```

Compiler flags include `-Wno-deprecated-literal-operator -Wno-unused-variable -Wno-unused-but-set-variable`. These warnings are intentionally suppressed.

Dashboard build requires pre-built GLFW 3.4 static lib in `deps/glfw-build/lib/`. Headers exist but lib is missing — must be compiled from source or installed via system package.

## Source structure

### Headers (`include/zeta/`)
- `valor_zeta.hpp` — Core types: `ValorZeta` (shared_ptr<ValorImpl>), `DataFrame`, `ErrorZeta`, `SceneSpec`, factory functions (`mk_num`, `mk_str`, etc.)
- `lexer.hpp` — `TipoToken` enum (80+ values), `Token`, `Lexer` class
- `parser.hpp` — `TipoNodoAST` enum (50+ values), `NodoAST`, `Parser` class
- `interpreter.hpp` — `TablaSimbolos` (inline), `Interpreter` class
- `estadisticas.hpp` — `fn_mean`, `fn_count`, `fn_sum`, `fn_min`, `fn_max`, `fn_stddev`
- `errores.hpp` — `crear_error`, `is_error`, `obtener_error`
- `serializador.hpp` — `valor_a_json`, `dataframe_a_json`, `dashboard_a_json`
- `grafo_json.hpp` — `guardar_grafo_json`, `cargar_grafo_json`
- `dl_loader.hpp` — `DlLibrary`, `DlRegistry` (C ABI plugin loading)
- `zeta_abi.h` — `ZETA_API`, `zeta_fn_t` (C ABI for native plugins)

### Source (`src/`)
- `main.cpp` — CLI entrypoint
- `server_main.cpp` — HTTP server entrypoint (Crow)
- `renderer/main.cpp` — Dashboard entrypoint (OpenGL)
- `term/main.cpp` — Terminal renderer entrypoint (ANSI)
- `lexer/lexer.cpp` — Tokenizer (246 lines)
- `parser/parser.cpp` — Parser → AST (1100+ lines)
- `interpreter/interpreter.cpp` — Interpreter (2140+ lines, core logic)
- `core/valor_zeta.cpp` — Factory functions for all 15 types
- `core/estadisticas.cpp` — Statistics functions (null-aware)
- `core/errores.cpp` — Error creation and checking
- `core/serializador.cpp` — Value → JSON serialization
- `core/grafo_json.cpp` — SceneSpec JSON save/load
- `core/tabla_simbolos.cpp` — Stub (TablaSimbolos defined inline in interpreter.hpp)
- `dl_loader/dl_loader.cpp` — Dynamic library loading

## Testing

- **C++ unit tests**: `tests/test_core.cpp` — assert-based, no framework. Compiled by CMake as `test_zeta` target or manually.
- **E2E tests**: `./test_e2e.sh` — full pipeline test. Requires X11 for dashboard screenshot step. Uses port 8095.
- **Zeta scripts**: `tests/*.zl` and `tests/*.zeta` — runnable via `./zeta <file>`.

## Dependencies (all header-only or vendored in `deps/`)

- `deps/asio/` — Asio (standalone, for server networking)
- `deps/crow_all.h` — Crow HTTP framework
- `deps/json.hpp` — nlohmann/json
- `deps/imgui/` — ImGui 1.91.8 (with GLFW+OpenGL3 backends)
- `deps/implot/` — ImPlot 0.16
- `deps/glfw-build/` — GLFW 3.4 headers (pre-built static lib missing)

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

## Gotchas

- `zeta_dashboard` requires X11 + pre-built GLFW — will fail on headless/SSH systems. Use `zeta_term` instead.
- The E2E test kills existing `zeta_server`/`zeta_dashboard` processes on port 8095 via `pkill -9`.
- `package.sh` version defaults to `0.1.0` if no argument given.
- No linting, formatting, or typecheck tooling configured — maintain existing code style.
- `CMakeLists.txt` is out of sync with `build.sh` — use `build.sh` as source of truth.
