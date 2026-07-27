# Changelog — Zeta Language

All notable changes to the Zeta language, runtime, and tooling.

Format: [Keep a Changelog](https://keepachangelog.com/)

## [0.2.0] — 2026-07-25

### Added
- **Arena allocator** (`include/zeta/arena.hpp`): Region-based memory for deterministic bulk reclamation. O(1) reset, no GC pauses.
- **`Valor` value type** (`include/zeta/valor.hpp`): Rust-inspired stack-allocated values. NUM/BOOL stored inline (0 heap allocs), large types arena-backed.
- **Move overloads** for all factory functions: `mk_str&&`, `mk_vec&&`, `mk_bool_vec&&`, `mk_str_vec&&`, `mk_matriz&&`, `mk_dict&&`. Eliminates unnecessary copies of temporaries.
- **Fast path for NUM⊕NUM** in `evaluar_binaria`: Inline double arithmetic before coercion checks. Most common case now has zero allocation overhead.
- **`clear_arena()`** builtin: Manual memory management for large workloads. Resets arena in O(1).
- **`arena_bytes()`** builtin: Query current arena allocation size for profiling.
- **`--help` flag** for all binaries (`zeta`, `zeta_server`, `zeta_term`).
- **`group_by(df, cols...)`** — Group DataFrame by column(s) for aggregation.
- **`agg(grouped, col, func)`** — Aggregate grouped data (sum, mean, count, min, max, stddev, median).
- **`merge(df1, df2, on)`** — Inner join two DataFrames on a key column.
- **LSP support** for `zeta-lsp` binary (build with `./build.sh lsp`).
- **`zeta.lsp.path`** config in VSCode extension for custom LSP binary path.

#### Exploration (15 funciones)
- **`info($df)`** — Resumen de DataFrame (columnas, tipos, nulls, nulos).
- **`describe($df)`** — Estadísticas descriptivas (mean, std, min, max, quartiles).
- **`tail($df, $n)`** — Últimas N filas.
- **`sample($df, $n)`** — Muestreo aleatorio.
- **`value_counts($df, $col)`** — Conteo de valores únicos.
- **`nunique($df, $col)`** — Número de valores únicos.
- **`median($vec)`** — Mediana.
- **`percentile($vec, $p)`** — Percentil.
- **`mode($vec)`** — Moda.
- **`cor($df, $col1, $col2)`** — Correlación.
- **`cov($df, $col1, $col2)`** — Covarianza.
- **`isna($vec)`** — Detectar nulos.
- **`duplicated($df)`** — Detectar duplicados.
- **`cut($vec, $bins)`** — Discretizar en bins.
- **`qcut($vec, $q)`** — Discretizar en quantiles.

#### Distributions + Testing (13 funciones)
- **`dnorm($x, $mean, $std)`** — Densidad normal.
- **`pnorm($x, $mean, $std)`** — Probabilidad acumulada normal.
- **`qnorm($p, $mean, $std)`** — Cuantil normal.
- **`dgamma($x, $shape, $scale)`** — Densidad gamma.
- **`dbeta($x, $a, $b)`** — Densidad beta.
- **`dunif($x, $min, $max)`** — Densidad uniforme.
- **`dt_dist($x, $df)`** — Densidad t-Student.
- **`df_dist($x, $df1, $df2)`** — Densidad F.
- **`dchisq($x, $df)`** — Densidad chi-cuadrado.
- **`t_test($vec1, $vec2)`** — T-test para muestras independientes.
- **`anova($vecs)`** — ANOVA de una vía.
- **`chi_square($obs, $exp)`** — Prueba chi-cuadrado.
- **`linear_regression($x, $y)`** — Regresión lineal simple.

#### Window Functions (14 funciones)
- **`cumsum($vec)`** — Suma acumulada.
- **`cummax($vec)`** — Máximo acumulado.
- **`cummin($vec)`** — Mínimo acumulado.
- **`rolling_mean($vec, $window)`** — Media móvil.
- **`rolling_std($vec, $window)`** — Desviación estándar móvil.
- **`rolling_sum($vec, $window)`** — Suma móvil.
- **`rolling_min($vec, $window)`** — Mínimo móvil.
- **`rolling_max($vec, $window)`** — Máximo móvil.
- **`lag($vec, $n)`** — Desplazar hacia atrás.
- **`lead($vec, $n)`** — Desplazar hacia adelante.
- **`diff($vec, $n)`** — Diferencia.
- **`row_number($df)`** — Número de fila.
- **`rank($vec)`** — Ranking.
- **`pct_change($vec)`** — Cambio porcentual.

#### Data Cleaning (10 funciones)
- **`drop_duplicates($df)`** — Eliminar filas duplicadas.
- **`rename($df, $old, $new)`** — Renombrar columna.
- **`select_cols($df, <$cols>)`** — Seleccionar múltiples columnas.
- **`drop_cols($df, <$cols>)`** — Eliminar múltiples columnas.
- **`fillna($vec, $strategy)`** — Rellenar nulos (mean, median, mode, ffill, bfill, zero, literal).
- **`replace_val($df, $col, $old, $new)`** — Reemplazar valores.
- **`clip($vec, $min, $max)`** — Limitar outliers.
- **`trim($vec, $std)`** — Eliminar outliers por std dev.
- **`normalize($vec)`** — Normalizar 0-1.
- **`standardize($vec)`** — Estandarizar z-score.

#### Plugin System v2
- **`ZetaPlugin`** class — Plugin con metadata (nombre, versión, autor, funciones).
- **`plugin()`** — Cargar plugins dinámicos.
- **`plugin_info()`** — Información del plugin cargado.
- **ABI v2** — Macros `ZETA_NUM`, `ZETA_STR`, `ZETA_BOOL`, `ZETA_NULL`, `ZETA_DF`.

### Changed
- **`TablaSimbolos::padre`** changed from `shared_ptr<TablaSimbolos>` to raw pointer (`TablaSimbolos*`). Parent scopes always outlive children — eliminates refcount overhead in scope chains. Closures still hold `shared_ptr` for lifetime safety.
- **`build.sh`** now compiles `src/core/arena.cpp`, `distribuciones.cpp`, `window_functions.cpp` in CORE_SRCS.
- **`install.sh`** improved: `--help`, `--no-build`, `--no-dashboard` flags, LSP build, PATH guidance.
- **`install.ps1`** improved: `-Help` parameter, LSP build.
- **TextMate grammar** updated with missing builtins (load_json, save_csv, save_xlsx, guardar_grafo, cargar_grafo, grafo_actual, mk_err, mk_null_val, load_lib).
- **LSP builtins** registry updated with 57 new functions.

### Fixed
- **`uninstall.sh`/`uninstall.ps1`** now remove `zeta-lsp` binary.
- **`evaluar_binaria`** coercion: `BOOL + STR` now correctly coerces to `STR + STR`.

### Performance
- NUM⊕NUM arithmetic: ~10x faster (inline double, no allocation)
- Scope variable lookup: ~3x fewer atomic operations (raw parent pointer)
- Vector/string temporaries: Eliminated copies via move semantics
- Arena reset: O(1) bulk reclamation vs O(n) individual refcount decrements

## [0.1.0] — 2026-07-01

### Initial release
- Lexer, Parser, AST, Interpreter pipeline
- 15 value types: bool, num, string, vector, bool_vec, str_vec, matrix, dict, DataFrame, error, graph, metric, dashboard, scene, function, object
- HTTP server (`zeta_server`) with embedded HTML/Tailwind/Chart.js UI
- Terminal renderer (`zeta_term`) with ANSI 24-bit color
- Dashboard renderer (`zeta_dashboard`) with OpenGL+ImGui+ImPlot
- OOP with single inheritance
- Lambda closures with captured scope
- CSV, JSON, XLSX file I/O
- Dynamic library loading (C ABI plugins)
- Statistics functions (mean, count, sum, min, max, stddev)
- Scene graph specification for dashboard rendering
- E2E test suite (`test_e2e.sh`)
