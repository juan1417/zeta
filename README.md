# Documentación del Lenguaje Zeta

> **Zeta** es un lenguaje analítico y de transformación de datos con sintaxis híbrida Rust/Python, motor de cómputo C++20 nativo, sistema de tipos dinámicos, propagación de errores y *null* como datos, importadores tipo Rust, ABI C para cargar `.so`/`.dll` con un solo `load_lib(...)`, y tres renderers que consumen un mismo *scene spec* JSON declarativo.

---

## Tabla de contenidos

| # | Documento | Qué cubre |
|---|-----------|-----------|
| 1 | [Filosofía y diseño](docs/01-filosofia.md) | Por qué Zeta, decisiones de diseño, audiencia, el "por qué" del lenguaje |
| 2 | [Sintaxis](docs/02-sintaxis.md) | Tokens, literales, operadores, sigil `$`, colecciones `<>`, `<<>>`, `{}`, indexado `()` `[]`, `[[]]`, ternario |
| 3 | [Tipos de datos y estructuras](docs/03-tipos-y-estructuras.md) | Los 15 `ValorZeta`, `null` como NaN, vectores, matrices, diccionarios, DataFrames |
| 4 | [Control de flujo y funciones](docs/04-control-y-funciones.md) | `if/else`, `for/in`, `while`, `break`, `continue`, ternario, funciones `fn`, recursión, cierres |
| 5 | [Funciones nativas (referencia)](docs/05-funciones-nativas.md) | Todas las funciones built-in agrupadas por categoría con ejemplos |
| 6 | [Imports y módulos](docs/06-imports-modulos.md) | `include`, `export`, namespace `::`, alias `as`, detección de ciclos, caché de módulos |
| 7 | [I/O: CSV y cliente HTTP via C ABI](docs/07-io.md) | `load_csv`, `ZETA_PATH`, parseo robusto, cliente HTTP user-defined (libcurl via `load_lib`) |
| 8 | [Errores y null como datos](docs/08-errores-null.md) | `?` propagación, `is_error`, `is_null`, `mk_err`, `mk_null_val` |
| 9 | [Escenas y visualización](docs/09-escenas-visualizacion.md) | `scene`, `layout`, `add_metric`, `add_line_plot`, `add_bar_chart`, `add_scatter`, `add_histogram`, `add_box_plot`, `add_heatmap`, `add_linear_regression`, `guardar_grafo`, `cargar_grafo` |
| 10 | [Carga de librerías nativas (C ABI)](docs/10-c-abi-loadlib.md) | `load_lib`, `ZETA_API`, `zeta_fn_t`, marshalling de vectores, ejemplo `lib/test_lib.cpp` |
| 11 | [Servidor HTTP](docs/11-servidor-http.md) | `zeta_server`, endpoints REST (`/api/datos`, `/api/metricas`, `/api/dashboard`, `/api/grafo`, `/api/run`) |
| 12 | [Renderers](docs/12-renderers.md) | `zeta_dashboard` (OpenGL+ImGui+ImPlot) y `zeta_term` (ANSI 24-bit) |
| 13 | [Arquitectura interna](docs/13-arquitectura.md) | Pipeline compilador: Lexer → Parser → AST → Interpreter → Server/Renderer |
| 14 | [Build y empaquetado](docs/14-build-paquete.md) | `build.sh` (cli/server/dashboard/term/all/clean), `package.sh` (`dist/zeta-X.Y.Z.tar.gz`), `test_e2e.sh` |
| 15 | [Ejemplos completos (cookbook)](docs/15-ejemplos.md) | 18 scripts `.zl` listos para correr, de "hola mundo" a regresión lineal y quicksort |

---

## Quickstart (5 minutos)

```bash
# 1. Compilar todo
./build.sh all

# 2. Ejecutar un script
./zeta tests/dashboard_scene.zl

# 3. Iniciar el server (en una terminal)
./zeta_server --port 8080

# 4a. Lanzar el dashboard nativo (OpenGL, requiere X11)
./zeta_dashboard --host localhost --port 8080

# 4b. O el renderer de terminal (sin OpenGL, SSH-friendly)
./zeta_term --host localhost --port 8080 --width 120
```

Hola mundo en Zeta:

```zeta
print("Hola, Zeta!")

# Una variable siempre empieza con $
$edad = 30
print("Tengo", $edad, "años")

# Vectores con <>
$nums = <1, 2, 3, 4, 5>
print("Suma =", sum($nums))
print("Promedio =", mean($nums))

# DataFrame desde CSV
$datos = load_csv("tests/datos.csv")
print("Filas:", len($datos))
print("Columna 'ventas':", $datos:ventas)
```

---

## Audiencia

- **Analistas de datos** que quieren transformar, resumir y graficar sin levantar un notebook.
- **Ingenieros de sistemas** que necesitan un motor embebible con ABI C y sin GC.
- **Diseñadores de DSL** que toman el lexer/parser como base y montan su propia lógica sobre el `Interpreter`.

## Binarios incluidos

| Binario | Tamaño | Descripción | Dependencias |
|---------|--------|-------------|--------------|
| `zeta` | ~640 KB | Intérprete CLI | Solo libstdc++ |
| `zeta_server` | ~1.2 MB | Servidor HTTP REST con Crow | libstdc++, asio (header-only), json (header-only) |
| `zeta_dashboard` | ~3.6 MB | Renderer OpenGL nativo (ImGui+ImPlot) | libGL, libGLEW, libX11, libpthread |
| `zeta_term` | ~234 KB | Renderer ANSI 24-bit para terminal | libstdc++ (sin OpenGL) |
