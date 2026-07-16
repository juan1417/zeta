# 3. Tipos de datos y estructuras

Zeta tiene un **sistema de tipos dinámico** con 15 tipos de primera clase, todos accesibles vía `type(x)` y serializables a JSON. Los valores son inmutables (las asignaciones reemplazan, no mutan) excepto donde se indique lo contrario.

## 3.1. El tipo `ValorZeta`

Internamente, todo valor es `std::shared_ptr<ValorImpl>` donde `ValorImpl` contiene un `enum Tipo` y 15 campos, uno por cada tipo. Las funciones de fábrica son `mk_<tipo>(valor)` y los accessors son `get_<tipo>(valor)`. Ver `include/zeta/valor_zeta.hpp`.

```cpp
enum Tipo {
    BOOL, NUM, STR, VEC, BOOL_VEC, STR_VEC,
    MATRIZ, DICT, DF, ERR,
    GRAFICO, METRICA, DASHBOARD, SCENE,
    FUNC
};
```

## 3.2. Tabla de tipos

| Enum | Nombre (`type()`) | Representación interna | Ejemplo literal |
|------|-------------------|------------------------|-----------------|
| `BOOL` | `"bool"` | `bool_val` | `true`, `false` |
| `NUM` | `"num"` | `double num_val` (NaN = null) | `42`, `3.14`, `null` |
| `STR` | `"str"` | `string str_val` | `"hola"` |
| `VEC` | `"vec"` | `vector<double> vec_val` | `<1, 2, 3>` |
| `BOOL_VEC` | `"bool_vec"` | `vector<bool> bool_vec_val` | `is_null(<1, 2>)` |
| `STR_VEC` | `"str_vec"` | `vector<string> str_vec_val` | `split("a,b", ",")` |
| `MATRIZ` | `"matriz"` | `vector<vector<double>>` | `<<1, 2>, <3, 4>>` |
| `DICT` | `"dict"` | `map<string, ValorZeta>` | `{"a": 1}` |
| `DF` | `"df"` | `DataFrame` (ver §3.6) | `load_csv("...")` |
| `ERR` | `"err"` | `ErrorZeta{tipo, mensaje, linea}` | `mk_err(...)` |
| `GRAFICO` | `"grafico"` | `GraficoConfig{tipo, titulo, eje_x, eje_y, bins}` | `plot(...)` |
| `METRICA` | `"metrica"` | `MetricaKPI{nombre, valor}` | `metric(...)` |
| `DASHBOARD` | `"dashboard"` | `DashboardConfig{titulo, autor, elementos}` | `dashboard(...)` |
| `SCENE` | `"scene"` | `shared_ptr<SceneSpec>` | `scene(...)` |
| `FUNC` | `"func"` | `{nombre, params, cuerpo, cierre}` | `fn` declarada |

## 3.3. Booleanos

`true` y `false` son los únicos valores booleanos. En contextos que esperan bool (if, while, &&, ||), cualquier otro tipo causa error de tipo.

```zeta
$es_mayor = $edad >= 18
$es_valido = !is_null($valor)
```

## 3.4. Números y `null`

Los números son `double` (64 bits, IEEE 754). El literal `null` se almacena como **quiet NaN** (un valor `double` con la convención IEEE 754). Esto significa:

| Operación | Resultado con null |
|-----------|---------------------|
| `null + 5` | `null` (NaN se propaga) |
| `null * 2` | `null` |
| `5 / 0` | `null` (división por 0 → NaN) |
| `null == null` | `false` (NaN nunca es igual a sí mismo) |
| `is_null(null)` | `true` (chequea `x != x`) |

Por eso la **división por 0 no lanza excepción**: produce `null` naturalmente. Para detectarlo:

```zeta
$resultado = $a / $b
if (is_null($resultado)) {
    print("División por 0 o null detectado")
}
```

## 3.5. Cadenas

`std::string` UTF-8 (no hay tipo separado para char). Operador `+` concatena:

```zeta
$nombre = "Zeta"
$version = "0.1.0"
$completo = $nombre + " " + $version    # "Zeta 0.1.0"
```

Métodos asociados: `len`, `upper`, `lower`, `substr`, `split`, `join`, `replace`, `find`, `reverse`. Ver [Funciones nativas](./docs/05-funciones-nativas.md).

## 3.6. Vectores

`std::vector<double>` (densos, no sparse). Indexado 0-based con `[]`:

```zeta
$nums = <10, 20, 30, 40, 50>
print($nums[0])       # 10
print($nums[4])       # 50
print(len($nums))     # 5

# Concatenación
$mas = $nums + <60, 70>   # <10, 20, 30, 40, 50, 60, 70>

# Estadísticas
print(sum($nums))     # 150
print(mean($nums))    # 30
print(min($nums))     # 10
print(max($nums))     # 50
```

Iteración con `for`:

```zeta
for ($x in $nums) {
    print($x)
}
```

## 3.7. Matrices

`std::vector<std::vector<double>>` (rectangulares, no jagged). Indexado con `[]` y coma:

```zeta
$m = <<1, 2, 3>,
      <4, 5, 6>,
      <7, 8, 9>>
print($m[0, 0])       # 1
print($m[2, 2])       # 9
print($m[1, 0])       # 4
```

Funciones: `transpose($m)`, `dot($a, $b)` (producto punto de dos vectores).

## 3.8. Diccionarios

`std::map<std::string, ValorZeta>`. **Ordenados** alfabéticamente (no `unordered_map`) para serialización determinística.

```zeta
$d = {"nombre": "Zeta", "version": 0.1, "activo": true}
print($d:nombre)       # Error: ':' solo funciona en DataFrames
# Correcto: acceso por string con keys()
print(keys($d))        # <"activo", "nombre", "version">  (ordenado)
print(values($d))      # <true, "Zeta", 0.1>
```

Nota: **no hay sintaxis de acceso por clave** en Zeta. Los diccionarios se usan principalmente como **namespace de módulos** (`statslib::mean(...)` es acceso a `statslib`'s dict). Para acceso dinámico, itera con `keys` y `values`.

## 3.9. DataFrames: el tipo estrella

`DataFrame` soporta columnas de tipo numérico (`num`), cadena de texto (`str`) y booleano (`bool`). Cada columna almacena sus datos en vectores separados con un bitmap de nulidad.

```cpp
struct Columna {
    std::string tipo;                    // "num", "str", "bool"
    std::vector<double> nums;            // datos numéricos (válido si tipo=="num")
    std::vector<std::string> strs;       // datos de texto (válido si tipo=="str")
    std::vector<bool> bools;             // datos booleanos (válido si tipo=="bool")
    std::vector<bool> null_bitmap;       // null_bitmap[i] == true ⟹ fila i es null
};

struct DataFrame {
    std::map<std::string, Columna> columnas;
    std::vector<std::string> nombres_columnas;  // orden de inserción
    size_t filas() const;
};
```

### Creación

```zeta
# Desde CSV — tipos inferidos automáticamente (num, str, bool)
$df = load_csv("tests/datos.csv")

# Construido a mano (como dict con vectores numéricos)
$idx = <1.0, 2.0, 3.0, 4.0, 5.0>
$ventas = <100, 200, 150, 300, 250>
$df2 = {"idx": $idx, "ventas": $ventas}
```

### Acceso a columna

```zeta
print($df:ventas)              # <100, 200, 150, 300, 250>
print(sum($df:ventas))         # 1000
print(mean($df:ventas))        # 200
```

### Filtrado de filas

```zeta
# Filas donde ventas > 150
$grandes = $df[[$df:ventas > 150]]
print($grandes)   # DataFrame con 3 filas: idx=2,4,5
```

Internamente, el intérprete itera cada fila, expone las columnas como variables en un scope temporal, evalúa la condición, y construye un nuevo DataFrame.

### Indexado (fila individual)

```zeta
$primera = $df[0]        # Devuelve un dict con los valores de la fila 0
print($primera:ventas)   # 100
```

### Selección de columnas

```zeta
$solo_ventas = select($df, "ventas")    # <100, 200, 150, 300, 250>
```

### Inspección

```zeta
print(len($df))           # número de filas
print(head($df, 3))       # primeras 3 filas (otro DataFrame)
print(keys($df))          # nombres de columnas
```

## 3.10. Errores como datos

`ErrorZeta` es un struct `{string tipo, string mensaje, int linea}`. Se crean con `mk_err("tipo", "msg", linea)` y se verifican con `is_error(x)`.

```zeta
# Un error es un valor como cualquier otro
$e = mk_err("division", "división por cero", 10)
if (is_error($e)) {
    print("Tipo:", $e.tipo, "Mensaje:", $e.mensaje)
}
```

Los errores **se propagan con `?`** y **se pueden almacenar en vectores/Dicts** para análisis posterior. Ver [Errores y null como datos](./docs/08-errores-null.md).

## 3.11. Tipos compuestos: GRAFICO, METRICA, DASHBOARD, SCENE, FUNC

Estos tipos son **contenedores de configuración** que el lenguaje usa para comunicarse con el exterior (HTTP API, renderers).

| Tipo | Estructura | Creado por | Consumido por |
|------|------------|------------|---------------|
| `GRAFICO` | `GraficoConfig{tipo, titulo, eje_x, eje_y, bins}` | `plot(...)` | `serve(...)`, `/api/dashboard` |
| `METRICA` | `MetricaKPI{nombre, valor}` | `metric(...)` | `serve(...)`, `/api/metricas` |
| `DASHBOARD` | `DashboardConfig{titulo, autor, elementos}` | `dashboard(...)` | `serve(...)`, `/api/dashboard` |
| `SCENE` | `SceneSpec{titulo, autor, layout, nodes, created_at, updated_at}` | `scene(...)` | `/api/grafo`, `zeta_dashboard`, `zeta_term` |
| `FUNC` | `{nombre, params, cuerpo (NodoAST*), cierre}` | `fn` declarada | Llamadas a funciones de usuario |

`SCENE` es el más importante: ver [Escenas y visualización](./docs/09-escenas-visualizacion.md).

## 3.12. Funciones como valores

Las funciones declaradas con `fn` se almacenan como `ValorZeta` de tipo `FUNC` con un `shared_ptr<NodoAST>` como cuerpo y un `shared_ptr<TablaSimbolos>` como cierre (scope donde se definieron).

```cpp
struct ValorImpl {
    ...
    Tipo tipo;        // = FUNC
    std::string func_nombre;
    std::vector<std::string> func_params;
    void* func_cuerpo;            // NodoAST*
    std::shared_ptr<TablaSimbolos> func_cierre;
};
```

Esto permite **higher-order functions**:

```zeta
fn doble($x) { return $x * 2 }

$nums = <1, 2, 3, 4>
$duplicado = map($nums, doble)   # <2, 4, 6, 8>
```

Ver `map`, `filter`, `reduce` en [Funciones nativas](./docs/05-funciones-nativas.md).

## 3.13. `type(x)` y serialización

```zeta
print(type(42))         # "num"
print(type("hola"))     # "str"
print(type(<1, 2>))     # "vec"
print(type(load_csv("datos.csv")))   # "df"
print(type(scene("t")))              # "scene"
print(type(new Counter()))           # "objeto"
```

## 3.14. Objetos (programacion orientada a objetos)

`OBJ` es un tipo compuesto que representa una instancia de una clase definida por el usuario. Tiene dos partes: una referencia a la clase (`clase`) y un mapa de campos (`campos`) accesibles via `$obj.campo` o via metodos.

```zeta
class Counter {
    $count = 0
    fn inc($self) {
        $self.count = $self.count + 1
        return $self.count
    }
}

$c = new Counter()
print($c.count)        # 0.000000
print($c.inc())        # 1.000000
print($c.count)        # 1.000000
```

Los objetos se crean con `new Nombre(args)`. Soportan herencia simple via `class Hija extends Padre`. Ver [4.13 Programacion orientada a objetos](./04-control-y-funciones.md#413-programacion-orientada-a-objetos) para la referencia completa.

## 3.15. Coerción de tipos

Zeta aplica coerción automática en operaciones binarias con tipos mixtos. La jerarquía es:

**`bool` → `num` → `str`**

| Operación | Resultado |
|-----------|-----------|
| `true + 10` | `11` (bool → num: true=1, false=0) |
| `false + 10` | `10` |
| `42 + " items"` | `"42 items"` (num → str) |
| `true + " value"` | `"true value"` (bool → str) |
| `"a" + 1` | `"a1"` (num → str para concat) |

En comparaciones, los booleanos se convierten a num:

```zeta
print(3 > true)      # true  (3 > 1)
print(2 == false)    # false (2 == 0)
```

## 3.16. `type()` — inspección de tipos

La función `type(x)` devuelve el nombre del tipo como string. Útil para debug y polimorfismo.
