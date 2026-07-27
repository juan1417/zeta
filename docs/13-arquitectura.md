# 13. Arquitectura interna

Este documento describe el **pipeline de ejecución** de un programa Zeta: cómo el código fuente se convierte en cómputo, y qué pasa por debajo.

## 13.1. Vista general

```
   archivo.zl
       │
       ▼
   ┌──────┐    ┌──────┐    ┌────────────┐
   │Lexer │───▶│Parser│───▶│  NodoAST   │
   └──────┘    └──────┘    └─────┬──────┘
   (tokens)    (LL(1) rec)       │
                                 ▼
                          ┌─────────────┐
                          │ Interpreter │◀──── include / load_lib
                          │  (evaluar)  │
                          └──────┬──────┘
                                 │ estado en RAM
                                 ▼
                       ┌──────────────────┐
                       │ TablaSimbolos +  │
                       │ SceneSpec +      │
                       │ métricas +       │
                       │ dashboard config │
                       └──────┬───────────┘
                              │ JSON
                  ┌───────────┼───────────┐
                  ▼           ▼           ▼
             zeta_server  zeta_dash   zeta_term
             (Crow)      (OpenGL)    (ANSI)
```

## 13.2. Lexer: de texto a tokens

**Archivo**: `src/lexer/lexer.cpp` (201 líneas). **Header**: `include/zeta/lexer.hpp`.

El lexer es un **scanner manual character-by-character** (no usa generadores como `lex`/`flex`). Características:

- **Greedy matching**: consume el token más largo posible.
- **Context-aware**: campos `esperando_valor_` y `profundidad_coleccion_` desambiguan `<`, `>`, `:`, `?` según el contexto.
- **Sin lookahead**: una sola pasada lineal.
- **Comentarios**: `#` hasta fin de línea.
- **Errores**: lanza `std::runtime_error` con línea y columna.

### Tokens principales

```cpp
enum class TipoToken {
    SIGIL,                  // $
    IDENTIFICADOR,          // foo, bar_baz
    NUMERO,                 // 42, 3.14
    CADENA,                 // "hola"
    TRUE_LITERAL, FALSE_LITERAL, NULL_LITERAL,

    VECTOR_ABRE, VECTOR_CIERRA,            // < >
    MATRIZ_ABRE, MATRIZ_CIERRA,            // << >>
    DICCIONARIO_ABRE, DICCIONARIO_CIERRA,  // { }
    COMA,

    ASIGNAR,                // =
    SUMA, RESTA, MULTIPLICACION, DIVISION, MODULO,
    IGUAL, DIFERENTE, MAYOR_QUE, MENOR_QUE, MAYOR_IGUAL, MENOR_IGUAL,
    AND, OR, NOT,
    EXTRACTOR,              // :
    TERNARIO_PREG, TERNARIO_SINO,           // ? :
    FILTRO_ABRE, FILTRO_CIERRA,            // [[ ]]
    CORCHETE_ABRE, CORCHETE_CIERRA,        // [ ]
    PARENTESIS_ABRE, PARENTESIS_CIERRA,    // ( )

    FN, IF, ELSE, FOR, WHILE, IN, RETURN, PRINT, BREAK, CONTINUE,
    INCLUDE, AS, EXPORT, PUNTO_DOBLE,      // ::

    LOAD_CSV, PLOT, SERVE, METRIC, DASHBOARD, ROUTE,
    IS_NULL, IS_ERROR, MEAN, COUNT, SUM_FN, MIN_FN, MAX_FN, STDDEV,
    ABS, ROUND, FLOOR, CEIL, POW, SQRT,
    LEN, UPPER, LOWER, SUBSTR,
    REVERSE, SORT_FN, UNIQUE, PUSH,
    KEYS, VALUES, TYPE_FN, RANGE,
    TRANSPOSE, DOT, HEAD, SELECT,
    SPLIT, JOIN, REPLACE, FIND,
    MAP_FN, FILTER_FN, REDUCE,

    EOF_TOKEN
};
```

### Por qué `esperando_valor_`

El token `<` puede ser:

- **Vector abre** `<1, 2, 3>`: cuando esperamos un valor (después de `=`, `(`, `,`).
- **Comparación** `if ($a < 5)`: cuando esperamos un operador (después de un valor).

El lexer rastrea esto con la flag `esperando_valor_`. Cuando ve `<`, mira el flag y emite el token apropiado.

### Por qué `profundidad_coleccion_`

Para matrices `<<1, 2>, <3, 4>>`:

- Después de `<<` (matriz abre), el siguiente `<` es **vector abre** (primera fila).
- Después de la primera fila, las siguientes filas también empiezan con `<`.

La profundidad se incrementa con `<<` y se decrementa con `>>`. Mientras `profundidad_coleccion_ > 0`, cualquier `<` es vector-abre.

## 13.3. Parser: de tokens a AST

**Archivo**: `src/parser/parser.cpp` (839 líneas). **Header**: `include/zeta/parser.hpp`.

Parser **LL(1) recursivo descendente**. Cada nivel de precedencia tiene su propia función:

```
expresion()              → expresion_ternaria()
expresion_ternaria()     → expresion_or() ? expr : expr
expresion_or()           → expresion_and() || expresion_and() ...
expresion_and()          → expresion_igualdad() && expresion_igualdad() ...
expresion_igualdad()     → expresion_comparacion() (== | !=) ...
expresion_comparacion()  → expresion_termino() (< | > | <= | >=) ...
expresion_termino()      → expresion_factor() (+ | -) ...
expresion_factor()       → expresion_unaria() (* | / | %) ...
expresion_unaria()       → (- | !) expresion_unaria() | expresion_primaria()
expresion_primaria()     → literal | identificador | vector | matriz | dict
```

### El AST

```cpp
struct NodoAST {
    TipoNodoAST tipo;
    std::string valor_texto;     // identificador, operador, etc.
    double valor_numerico;        // para literales numéricos
    bool valor_bool;              // para literales bool
    std::vector<std::unique_ptr<NodoAST>> hijos;
    std::string nombre_funcion;   // para LLAMADA_FUNCION
    std::vector<std::string> parametros;  // para DECLARACION_FN
    int linea, columna;
};
```

**Todos los nodos son polimórficos** vía el enum `tipo`. El `vector<unique_ptr<NodoAST>>` hijos es la recursión.

### Statements

```cpp
declaracion() → declaracion_asignacion() | declaracion_fn() | declaracion_print()
              | declaracion_if() | declaracion_for() | declaracion_while()
              | declaracion_return() | declaracion_inclusion() | declaracion_exportacion()
              | BREAK | CONTINUE
```

Cada `declaracion_X()` consume los tokens esperados y construye el sub-Árbol correspondiente.

### Errores de parseo

El parser lanza `std::runtime_error("Token inesperado: esperaba tipo X pero obtuvo 'Y' en línea Z")` cuando el token actual no coincide con el esperado. **No hay recovery** — un error de parseo aborta la ejecución.

## 13.4. Interpreter: de AST a cómputo

**Archivo**: `src/interpreter/interpreter.cpp` (1955 líneas). **Header**: `include/zeta/interpreter.hpp`.

El corazón del lenguaje. Métodos principales:

- `ValorZeta ejecutar(std::unique_ptr<NodoAST> ast)`: punto de entrada.
- `evaluar(const NodoAST&)`: switch sobre `TipoNodoAST`, delega a métodos específicos.
- `llamar_nativa(nombre, args)`: dispatch de funciones built-in (~1500 líneas).
- `llamar_usuario(func, args)`: invoca una `fn` definida por el usuario.
- `cargar_modulo(ruta)`, `cargar_selectivo(ruta, nombres)`, `cargar_con_alias(ruta, alias)`: sistema de imports.

### Estado del intérprete

```cpp
class Interpreter {
    std::shared_ptr<TablaSimbolos> ambito_global_;
    std::vector<MetricaKPI> metricas_;
    std::optional<DashboardConfig> dashboard_;
    std::vector<std::string> include_paths_;
    std::map<std::string, ModuleSnapshot> modulos_cache_;
    std::vector<std::string> pila_imports_;
    std::shared_ptr<SceneSpec> grafo_actual_;
};
```

### Evaluación de statements

`evaluar(BLOQUE)` itera sobre los hijos y evalúa cada uno. El valor del bloque es el valor del último statement (similar a un `do-while` implícito).

Para `if`, `for`, `while`: se evalúa la condición, y si es verdadera, se evalúa el cuerpo. `break` y `continue` lanzan sentinels que los bucles filtran.

### Evaluación de expresiones

Cada tipo de nodo tiene su método:

- `evaluar_binaria`: dispatch sobre `+`, `-`, `*`, `/`, `%`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `&&`, `||`.
- `evaluar_unaria`: dispatch sobre `-`, `!`.
- `evaluar_ternaria`: si la condición es bool, evalúa la rama correspondiente. Si es VEC/BOOL_VEC, vectoriza.
- `evaluar_acceso_columnas`: busca en el DataFrame por nombre de columna.
- `evaluar_filtro_filas`: itera filas del DF, evalúa la condición en cada una, construye DF filtrado.
- `evaluar_llamada_funcion`: distingue entre nativa y de usuario.

El intérprete también soporta funciones de manipulación de datos: `group_by`, `agg`, y `merge` para operaciones de agregación y joins sobre DataFrames.

## 13.5. Tabla de Símbolos

**Archivo**: `src/core/tabla_simbolos.cpp` (40 líneas). **Header**: `include/zeta/tabla_simbolos.hpp`.

```cpp
class TablaSimbolos {
public:
    std::shared_ptr<TablaSimbolos> padre;
    std::map<std::string, ValorZeta> variables;

    void definir(const std::string& nombre, const ValorZeta& valor);
    ValorZeta obtener(const std::string& nombre) const;
};
```

Implementa **lexical scoping**: cada bloque (`{}`) crea un nuevo `TablaSimbolos` con puntero al padre. La búsqueda sube por la cadena hasta encontrar el nombre o llegar al global.

```zeta
$x = 1
{
    $x = 2    # shadowing: crea nueva $x en este scope
    print($x)    # 2
}
print($x)    # 1
```

## 13.6. El tipo `ValorZeta`

**Archivo**: `include/zeta/valor_zeta.hpp` (145 líneas). **Implementación**: `src/core/valor_zeta.cpp` (195 líneas).

```cpp
using ValorZeta = std::shared_ptr<ValorImpl>;

struct ValorImpl {
    enum Tipo { BOOL, NUM, STR, VEC, BOOL_VEC, STR_VEC, MATRIZ, DICT, DF, ERR,
                GRAFICO, METRICA, DASHBOARD, SCENE, FUNC };
    Tipo tipo;

    bool bool_val;
    double num_val;
    std::string str_val;
    std::vector<double> vec_val;
    // ... 12 campos más
};
```

### Fábricas

```cpp
ValorZeta mk_num(double v);          // crea NUM
ValorZeta mk_str(string v);          // crea STR
ValorZeta mk_vec(vector<double> v);  // crea VEC
// ... 15 factories
```

### Accessors

```cpp
double get_num(const ValorZeta& v);
const std::string& get_str(const ValorZeta& v);
// ... 15 accessors
```

### Por qué `shared_ptr`

- **Pasar valores por valor** es barato (un refcount).
- **Funciones como valores**: una `fn` declarada en un módulo puede ser referenciada desde muchos lugares sin copiar el AST.
- **Cierres**: la `TablaSimbolos` del closure retiene las variables capturadas sin duplicar.

### NaN como null

```cpp
double crear_null() {
    return std::numeric_limits<double>::quiet_NaN();
}

bool es_null(double valor) {
    return valor != valor;  // NaN != NaN
}
```

IEEE 754 garantiza que `NaN != NaN` y que cualquier operación sobre NaN da NaN. Esto hace que la propagación de null sea "gratis" en hardware.

## 13.7. Serialización

**Archivo**: `src/core/serializador.cpp` (115 líneas). **Header**: `include/zeta/serializador.hpp`.

`valor_a_json(ValorZeta)` convierte cualquier tipo Zeta a `nlohmann::json`. La serialización es **recursiva** y maneja los 15 tipos. NaN se serializa como `null`.

## 13.8. El grafo (SceneSpec)

**Header**: en `include/zeta/valor_zeta.hpp`. **Implementación**: en `src/core/valor_zeta.cpp`. **Serialización**: `src/core/grafo_json.cpp` (119 líneas). **Header**: `include/zeta/grafo_json.hpp`.

```cpp
struct SceneSpec {
    std::string titulo, autor;
    SceneLayout layout;
    std::vector<SceneNode> nodes;
    double created_at, updated_at;
    SceneNode* find_node(const std::string& id);
};
```

`grafo_json.cpp` usa `nlohmann/json` para serializar/deserializar. Las funciones clave:

```cpp
nlohmann::json scene_a_json(const SceneSpec& s);
std::shared_ptr<SceneSpec> json_a_scene(const nlohmann::json& j);
bool guardar_grafo_json(const std::string& ruta, const SceneSpec& s);
std::shared_ptr<SceneSpec> cargar_grafo_json(const std::string& ruta);
```

## 13.9. El sistema de imports

**Archivo**: parte de `src/interpreter/interpreter.cpp` (~250 líneas).

```
include "x"              → cargar_modulo("x")
include "x"::{a, b}      → cargar_selectivo("x", {"a", "b"})
include "x" as y         → cargar_con_alias("x", "y")
```

`ModuleSnapshot` cachea el resultado de un módulo:

```cpp
struct ModuleSnapshot {
    std::string ruta;
    std::map<std::string, ValorZeta> simbolos;
    std::set<std::string> exports;
    std::shared_ptr<NodoAST> ast;  // mantiene vivo el AST para los cierres
};
```

`pila_imports_` detecta ciclos:

```cpp
if (std::find(pila_imports_.begin(), pila_imports_.end(), canon) != pila_imports_.end()) {
    return mk_err("import", "Ciclo detectado: " + canon);
}
```

## 13.10. El DL Loader

**Archivo**: `src/dl_loader/dl_loader.cpp`. **Header**: `include/zeta/dl_loader.hpp`.

```cpp
class DlLibrary {
    dl_handle handle_;
    bool cargar(const std::string& ruta);   // dlopen
    void* obtener_simbolo(const std::string& nombre);  // dlsym
};

class DlRegistry {
    static DlRegistry& instancia();
    DlLibrary* cargar(const std::string& ruta);
    // ...
};
```

El `Registry` mantiene librerías abiertas durante toda la vida del proceso. El marshaller en `llamar_usuario` detecta el prefijo `::dl::` en `func_nombre` y despacha a C en lugar de a AST.

## 13.11. La ABI C

**Header**: `include/zeta/zeta_abi.h` (22 líneas).

```c
typedef double (*zeta_fn_t)(int n_args, const double* args);
```

Es la **única** definición de la ABI. Las funciones nativas reciben `n_args` doubles y devuelven un double. La convención NaN-como-error se aplica en la frontera.

## 13.12. El servidor HTTP

**Archivo**: `src/server_main.cpp` (386 líneas).

Usa **Crow** (header-only, basado en Asio). Endpoints definidos con macros `CROW_ROUTE`. La instancia de `Interpreter` se comparte entre threads (con un `shared_ptr`).

```cpp
auto interpreter = std::make_shared<Interpreter>();
// ...
CROW_ROUTE(app, "/api/datos")
([&](const crow::request&, crow::response& res) {
    auto json = interpreter->obtener_datos_json();
    res.write(json.dump(2));
});
```

## 13.13. Extender el lenguaje

### Añadir una función nativa

1. En `include/zeta/lexer.hpp`, agrega un `TipoToken::MI_FN` (si quieres que sea keyword) o usa un nombre de identificador normal.
2. En `src/parser/parser.cpp`, en `parsear_llamada_nativa`, agrega el case.
3. En `src/interpreter/interpreter.cpp`, en `llamar_nativa`, agrega la rama `if (nombre == "mi_fn") { ... }`.
4. Si usaste un `TipoToken` nuevo, agrégalo a las reservadas en `src/lexer/lexer.cpp`.

### Añadir un tipo de nodo de escena

1. En `SceneNode` (header), agrega un nuevo campo opcional.
2. En `grafo_json.cpp`, actualiza `scene_node_a_json` y `json_a_scene`.
3. En los tres renderers (`src/renderer/main.cpp`, `src/term/main.cpp`, `src/server_main.cpp` HTML), agrega el handler.

### Añadir un renderer

1. Crea un nuevo ejecutable en `src/<nombre>/main.cpp`.
2. Usa `deps/json.hpp` para parsear.
3. Conecta al servidor con HTTP (ver `src/term/main.cpp` para un ejemplo minimal con sockets raw).
4. Implementa un dispatcher por `node.tipo`.
5. Agrega el target al `build.sh`.

## 13.14. Modelo de memoria (v0.2)

Zeta usa un **modelo híbrido inspirado en Rust**: reference counting para valores persistentes, arena allocator para temporales.

### Arena allocator

```cpp
// include/zeta/arena.hpp
class Arena {
    std::vector<Block> blocks_;  // 64KB-1MB bloques
    std::size_t current_block_;
    std::size_t current_offset_;
    std::vector<std::function<void()>> tracked_dtors_;
public:
    template<typename T, typename... Args>
    T* create(Args&&... args);  // Arena bump allocation
    void reset();               // O(1) bulk reclamation
    std::size_t bytes_used() const;
};
```

- **Bump allocation**: Solo incrementa un pointer. O(1), sin malloc/free.
- **Bulk reclamation**: `reset()` libera todo de golpe. O(número de bloques).
- **Destructor tracking**: Para tipos con destruidor no trivial (string, vector), registra callbacks que se llaman en `reset()`.

### Valor value type

```cpp
// include/zeta/valor.hpp
struct Valor {
    enum class Tag : uint8_t { EMPTY, NUM, BOOL, STR, VEC, ... };
    Tag tag_;
    union { double num_val; bool bool_val; };  // Inline para NUM/BOOL
    ValorImpl* ptr_;  // Arena-backed para tipos grandes
};
```

- **NUM/BOOL**: 8 bytes inline, 0 heap allocations.
- **STR/VEC/etc**: 1 arena bump, no refcount.
- **Conversión a ValorZeta**: `to_zeta()` crea heap copy solo cuando es necesario.

### Scope chains

```cpp
struct TablaSimbolos {
    TablaSimbolos* padre = nullptr;  // Raw pointer (borrowed reference)
    std::map<std::string, ValorZeta> variables;
};
```

- El padre **siempre** vive más que el hijo (scope nesting).
- Las closures mantienen el scope con `shared_ptr` (para lifetime safety).
- Las búsquedas de variables usan raw pointers — sin overhead de refcount.

### Impacto en performance

| Operación | Antes (v0.1) | Después (v0.2) | Mejora |
|-----------|--------------|----------------|--------|
| `mk_num(42)` | new + atomic inc | Union inline (8 bytes) | ~10x |
| Scope lookup | refcount chain | Raw pointer chase | ~3x |
| `clear_arena()` | N/A | O(1) bulk free | ∞ |
| Vector temporales | Heap + refcount | Arena bump | ~5x |

## 13.15. Métricas de tamaño

| Componente | Líneas | % del total |
|------------|--------|-------------|
| Lexer | 201 | 5.6% |
| Parser | 839 | 23.5% |
| Interpreter | 1955 | 54.7% |
| Core (valor, símbolos, errores, stats) | ~250 | 7.0% |
| Server | 386 | 10.8% |
| Renderer OpenGL | ~580 | — |
| Renderer Terminal | ~520 | — |
| DL Loader + ABI | ~100 | — |
| **Total (sin renderers)** | **~3700** | 100% |

El **54.7% del código está en el intérprete**, principalmente en `llamar_nativa` (las 50+ funciones built-in). Esto es esperable: la mayor parte del "lenguaje" son las funciones nativas.
