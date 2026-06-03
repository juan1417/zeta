# 1. Filosofía y diseño

## 1.1. ¿Qué es Zeta?

Zeta es un **lenguaje analítico y de transformación de datos** con tres propiedades que lo distinguen:

1. **Sintaxis híbrida Rust/Python, parsing sin GC.** El lexer/parser están escritos en C++20 sin alocación dinámica innecesaria y producen un AST que vive como `unique_ptr<NodoAST>` y `shared_ptr<SceneSpec>`. No hay сборщик мусора; los recursos se liberan determinísticamente por RAII.

2. **Cómputo y errores son datos.** Todos los valores viven en un `std::shared_ptr<ValorImpl>` (alias `ValorZeta`) con un `enum Tipo` de 15 valores. Los errores son un tipo más (`ERR`), no excepciones, y la propagación con `?` los encadena explícitamente. Los `null` son `quiet NaN` (un valor `double` con la convención IEEE 754), no un tipo separado.

3. **ABI C como ciudadano de primera clase.** Cargar una `.so` o `.dll` es una sola expresión:

   ```zeta
   load_lib("libtestnative.so", {"native_pow": "fn($a, $b) -> $a"})
   $resultado = native_pow(2, 10)   # 1024
   ```

   No hay FFI, no hay `ctypes`, no hay binding generator. La librería exporta funciones C con la macro `ZETA_API` y firma `double fn(int n_args, const double* args)`. El marshaller aplana vectores a un buffer plano de doubles y devuelve un double.

## 1.2. ¿Por qué otro lenguaje?

La motivación concreta es construir una **plataforma de dashboards desacoplada**:

```
        ┌─────────────┐
        │  zeta CLI   │  ejecuta scripts .zl
        └──────┬──────┘
               │ AST
        ┌──────▼──────┐
        │ Interpreter │  estado en RAM
        └──────┬──────┘
               │ JSON
   ┌───────────┼───────────┐
   │           │           │
┌──▼──┐    ┌───▼───┐    ┌───▼───┐
│ HTTP│    │  Open │    │  ANSI │   ← tres consumidores,
│ API │    │  GL   │    │  term │      una sola fuente de verdad
└─────┘    └───────┘    └───────┘
```

La mayoría de lenguajes analíticos (Python/pandas, R, Julia) son **monolíticos**: el lenguaje, la librería de plotting y la UI son inseparables. Zeta invierte esto: el lenguaje solo computa; las visualizaciones son **un JSON declarativo que cualquier consumidor puede renderizar**. Por eso existen tres renderers hoy (`zeta_dashboard` con ImGui+ImPlot, `zeta_term` con ANSI, y el HTML/Chart.js embebido en `zeta_server`) que leen el mismo endpoint `GET /api/grafo`.

## 1.3. Decisiones de diseño y por qué

| Decisión | Alternativa | Justificación |
|----------|-------------|---------------|
| **Sigil `$` obligatorio** en variables | Inferir por contexto (JS, Python) | El `$datos:ventas` se parsea inequívocamente como DataFrame-access; sin él, `$datos` y la keyword `datos` colisionan al servir HTML embebido. El parser se vuelve 30% más simple. |
| **Vectores `<>` y matrices `<<>>`** | `[1, 2, 3]` y `[[1,2],[3,4]]` | El ángulo es ambigüo con `>` (comparación), pero el lexer lo desambigua por la pila `profundidad_coleccion_`. Visualmente es claro y libera `[]` para indexado. |
| **Diccionarios `{"clave": valor}`** con `:` y `,` | `obj.clave` | Permite definir JSON literal en línea: `{"idx": $idx, "ventas": $datos:ventas}`. El `:` se reutiliza para `EXTRACTOR` (DataFrame col) y ternario según contexto (lexer usa `esperando_valor_` y `profundidad_coleccion_`). |
| **Null como `quiet NaN`** | Tipo `Option<T>` o `null` literal | IEEE 754 ya define el comportamiento de NaN: cualquier operación aritmética sobre NaN da NaN, las comparaciones dan `false`. Eso significa que `null + 5` es automáticamente `null` sin código defensivo. `is_null(x)` simplemente chequea `x != x`. |
| **Errores como `mk_err("tipo", "msg", linea)`** | Excepciones | Los errores se serializan a JSON, viajan por el AST, persisten en DataFrames. Se pueden *almacenar* en una columna, *propagar* con `?`, *filtrar* con `is_error(x)`. |
| **Funciones declaradas como `fn` con sigil en params** | `def nombre(a, b)` | Refuerza la consistencia con variables: si tiene `$`, es un nombre; si no, es una keyword. |
| **Import tipo Rust `include "x"::{a, b}`** | `from x import a, b` | Las llaves son léxicamente inequívocas y el lexer no necesita lookahead. `as alias` permite colisiones de nombres. |
| **C ABI marshalling plano** | JSON marshalling, protobuf | Un `double*` es la representación más eficiente posible para datos numéricos. No hay alocación en el hot path, los datos se copian por valor, y la latencia de llamada es predecible. |
| **OOP: `class` con `$self` (no `this` obligatorio)** | `class` con `self`/`this` implicito | Convención Go/Rust: el primer parametro es siempre `$self`. Mantiene la regla de sigilos sin excepciones y deja al usuario decidir si quiere declarar el receptor o no. Soporta herencia simple via `extends` sin `super` (v1). |
| **Renderers separados, no un core gráfico** | Un solo motor con backends | El renderer de terminal es 8× más pequeño (234 KB vs 3.6 MB) porque no arrastra GLFW/ImGui. Cero acoplamento: ambos leen el mismo JSON. |
| **Compilación con `clang++` directo** | CMake, Meson, Bazel | `build.sh` con 5 targets (`cli|server|dashboard|term|all`) y un `clean` es suficiente para el alcance actual. CMake existe (`CMakeLists.txt`) como alternativa para IDEs. |
| **Header-only deps (crow, asio, json)** | Compilar librerías externas | Reduce el tiempo de build y elimina un paso de configuración. Crow es single-header. Asio se puede usar como header-only. nlohmann/json es header-only. |
| **GLFW compilado a `.a` estático, glew dinámico** | Todo estático | GLFW se compila una vez y queda en `deps/glfw-build/lib/libglfw3.a` (no requiere X11 headers en runtime). GLEW ya está en `/usr/lib/`. |

## 1.4. El sistema de valores: por qué `std::variant` disfrazado

`ValorZeta = std::shared_ptr<ValorImpl>` parece un `std::variant` (y de hecho, internamente `ValorImpl` tiene 15 campos, uno por tipo). La diferencia es:

- **`std::variant<15 tipos>`** requiere conocer todos los tipos en compilación, y los accesos tienen que ser type-safe vía `std::get_if`. Esto es correcto pero verboso para 15 casos.
- **`shared_ptr<ValorImpl>`** con un `enum Tipo` es lo mismo pero **los 15 tipos viven en un struct con todos los campos reservados**. El `enum Tipo` es un discriminador runtime. Se pierde la type-safety estática, pero se gana:
  - Asignación trivial: `mk_num(3.14)` no necesita crear un variant, solo asignar `num_val = 3.14` y `tipo = NUM`.
  - Serialización JSON trivial: una función `valor_a_json(v)` recorre el enum y elige el campo correcto.
  - Pasar valores por APIs externas (HTTP, C ABI) es trivial: el receptor conoce el discriminador.

La función `tipo_nombre(const ValorZeta&)` mapea el enum a un string (`"num"`, `"vec"`, `"df"`, `"scene"`, etc.) para serialización.

## 1.5. La regla de oro: nada de magia

- **El parser es LL(1) recursivo descendente.** Cada regla gramatical es una función `expresion_X()`. La ambigüedad se resuelve con un campo `esperando_valor_` en el lexer (¿después de `=`, `(`, `,` esperamos un valor? → `<` se vuelve `<` vector-abre; si no, se vuelve `<` comparación).
- **El AST es opaco:** `NodoAST { TipoNodoAST tipo, valor_texto, valor_numerico, valor_bool, vector<unique_ptr<NodoAST>> hijos, ... }`. El `tipo` dispatchea en el `Interpreter::evaluar()`.
- **El símbolo es lineal:** una `TablaSimbolos` con un puntero a un `ambito_padre` para scope chaining (no hay `unordered_map<string, ValorZeta>` global oculto).
- **No hay macros, no hay preprocesador, no hay reflection.** El `export` enumera nombres explícitamente, no "todo lo que empieza con guión bajo".

## 1.6. Audiencia por nivel

- **Principiante**: empieza por [Quickstart](#quickstart-5-minutos) y [Sintaxis](./docs/02-sintaxis.md). Ejemplos en `tests/dashboard_scene.zl`.
- **Analista de datos**: ve directo a [Funciones nativas](./docs/05-funciones-nativas.md), [I/O](./docs/07-io.md) y [Escenas](./docs/09-escenas-visualizacion.md).
- **Ingeniero de sistemas / embedder**: [C ABI](./docs/10-c-abi-loadlib.md), [Servidor HTTP](./docs/11-servidor-http.md), [Renderers](./docs/12-renderers.md).
- **Autor del compilador**: [Arquitectura interna](./docs/13-arquitectura.md). El código cabe en 3575 líneas (lexer 201, parser 839, interpreter 1955, resto < 600).
