# 5. Funciones nativas (referencia completa)

Las funciones nativas (built-in) se llaman sin keyword especial (`print(x)`, no `zeta.print(x)`). El lexer reconoce sus nombres como tokens reservados. Esta es la lista completa, agrupada por categoría.

## 5.1. Estadísticas y agregación

### `sum($vec)` → `num`

Suma de los elementos de un vector. NaN/null se ignoran (pero si todos son null, devuelve `null`).

```zeta
print(sum(<1, 2, 3, 4, 5>))     # 15
print(sum(<1, null, 3>))         # 4 (null ignorado)
print(sum(load_csv("datos.csv"):ventas))
```

### `mean($vec)` → `num`

Promedio aritmético. Internamente `sum(vec) / count(vec)`, ignorando nulls.

```zeta
print(mean(<10, 20, 30>))    # 20
print(mean($datos:ventas))
```

### `min($vec)` / `max($vec)` → `num`

Mínimo y máximo. Nulls se ignoran.

```zeta
print(min(<3, 1, 4, 1, 5>))   # 1
print(max($datos:temperatura))
```

### `stddev($vec)` → `num`

Desviación estándar (poblacional, divide entre N, no N-1). Nulls se ignoran.

```zeta
print(stddev(<1, 2, 3, 4, 5>))    # 1.41421...
```

### `count($vec)` → `num`

Número de elementos **no nulos**.

```zeta
print(count(<1, 2, 3, 4, 5>))     # 5
print(count(<1, null, 3, null>))   # 2
```

## 5.2. Matemáticas escalares

### `abs($x)` → `num`

Valor absoluto.

```zeta
print(abs(-42))    # 42
print(abs(3.14))   # 3.14
```

### `round($x)` → `num`

Redondeo al entero más cercano (banker's rounding no; usa el de C `std::round`).

```zeta
print(round(3.7))    # 4
print(round(3.4))    # 3
print(round(-3.5))   # -4
```

### `floor($x)` / `ceil($x)` → `num`

Redondeo hacia abajo / hacia arriba.

```zeta
print(floor(3.7))    # 3
print(ceil(3.2))     # 4
print(floor(-3.2))   # -4
print(ceil(-3.7))    # -3
```

### `pow($base, $exp)` → `num`

Potencia. Equivalente a `$base ** $exp` en otros lenguajes.

```zeta
print(pow(2, 10))    # 1024
print(pow(3, 2))     # 9
print(pow(2, 0.5))   # 1.41421 (raíz cuadrada via pow)
```

### `format($num, $decimals)` → `str`

Formatea un número con `$decimals` decimales. Útil para mostrar resultados legibles.

```zeta
print(format(3.14159, 2))    # "3.14"
print(format(1000.0, 0))     # "1000"
print(format(0.123456, 4))   # "0.1235"
```

### `sqrt($x)` → `num`

Raíz cuadrada. Devuelve `null` para $x < 0.

```zeta
print(sqrt(144))    # 12
print(sqrt(2))      # 1.41421
print(sqrt(-1))     # null
```

## 5.3. Cadenas

### `len($str)` → `num`

Longitud en bytes (no en caracteres UTF-8; tenlo en cuenta con emojis).

```zeta
print(len("hola"))         # 4
print(len(""))             # 0
print(len("ñoño"))         # 6 (cada char UTF-8 ocupa 2 bytes)
```

### `upper($str)` / `lower($str)` → `str`

A mayúsculas / minúsculas.

```zeta
print(upper("hola"))    # "HOLA"
print(lower("HOLA"))    # "hola"
```

### `substr($str, $inicio, [$longitud])` → `str`

Subcadena desde `$inicio` (0-based) hasta el final o por `$longitud` caracteres.

```zeta
print(substr("hola mundo", 5))        # "mundo"
print(substr("hola mundo", 0, 4))    # "hola"
print(substr("hola mundo", -5))      # "mundo" (índices negativos cuentan desde el final)
```

### `split($str, $sep)` → `str_vec`

Divide un string por separador.

```zeta
print(split("a,b,c", ","))           # ["a", "b", "c"]
print(split("uno dos tres", " "))    # ["uno", "dos", "tres"]
```

### `join($str_vec, $sep)` → `str`

Une un vector de strings con un separador.

```zeta
print(join(["a", "b", "c"], "-"))    # "a-b-c"
print(join(["x", "y", "z"], ""))      # "xyz"
```

### `replace($str, $old, $new)` → `str`

Reemplaza todas las ocurrencias.

```zeta
print(replace("hola mundo", "mundo", "Zeta"))    # "hola Zeta"
print(replace("aaa", "a", "b"))                  # "bbb"
```

### `find($str, $sub)` → `num`

Posición de la primera ocurrencia de `$sub` en `$str`. Devuelve `-1` si no se encuentra.

```zeta
print(find("hola mundo", "mundo"))    # 5
print(find("hola mundo", "Zeta"))     # -1
```

## 5.4. Vectores y manipulación

### `len($vec)` → `num`

Número de elementos.

```zeta
print(len(<1, 2, 3, 4, 5>))    # 5
print(len($datos:ventas))       # 10
```

### `push($vec, $valor)` → `vec`

Agrega un elemento al final. **Devuelve un nuevo vector** (Zeta es inmutable).

```zeta
$v = <1, 2, 3>
$v2 = push($v, 4)
print($v2)    # <1, 2, 3, 4>
print($v)     # <1, 2, 3> (no se modificó)
```

### `reverse($vec)` → `vec`

Invierte el orden.

```zeta
print(reverse(<1, 2, 3>))    # <3, 2, 1>
```

### `sort($vec)` → `vec`

Ordena ascendentemente.

```zeta
print(sort(<3, 1, 4, 1, 5>))    # <1, 1, 3, 4, 5>
```

### `unique($vec)` → `vec`

Elimina duplicados (ordena primero).

```zeta
print(unique(<1, 2, 2, 3, 3, 3>))    # <1, 2, 3>
```

### `range($inicio_o_fin, [$fin], [$paso])` → `vec`

Genera un rango numérico. Tres formas:

```zeta
range(5)            # <0, 1, 2, 3, 4>            (un argumento: 0..N)
range(2, 5)         # <2, 3, 4>                   (dos argumentos: inicio..fin exclusivo)
range(0, 10, 2)     # <0, 2, 4, 6, 8>             (tres argumentos: con paso)
```

## 5.5. Inspección y tipos

### `type($x)` → `str`

Nombre del tipo como string.

```zeta
print(type(42))         # "num"
print(type("hola"))     # "str"
print(type(<1, 2>))     # "vec"
print(type(true))       # "bool"
print(type(null))       # "num" (null es un num con NaN)
print(type(load_csv("datos.csv")))   # "df"
print(type(scene("t")))              # "scene"
```

### `is_null($x)` → `bool` o `bool_vec`

Verifica si el valor es null (NaN). Si le pasas un vector, devuelve un `BOOL_VEC` con la verificación elemento a elemento.

```zeta
print(is_null(null))            # true
print(is_null(0))               # false
print(is_null(0 / 0))           # true (división por cero = null)
print(is_null(<1, null, 3>))    # <false, true, false>  (bool_vec)
```

### `is_error($x)` → `bool`

Verifica si el valor es un error.

```zeta
$e = mk_err("test", "mensaje")
print(is_error($e))     # true
print(is_error(42))      # false
```

### `fill_null($vec, $default)` → `vec`

Reemplaza todos los `null` de un vector con un valor por defecto. Devuelve un nuevo vector.

```zeta
$datos = <10, null, 30, null, 50>
$limpio = fill_null($datos, 0)
print($limpio)           # <10, 0, 30, 0, 50>

# Con valor promedio
$promedio = mean($datos)    # 30 (ignora nulls)
$Lleno = fill_null($datos, $promedio)
print($Lleno)               # <10, 30, 30, 30, 50>

# En DataFrames: reemplazar nulls antes de procesar
$df = load_csv("datos.csv")
$df:ventas = fill_null($df:ventas, 0)
print(sum($df:ventas))   # suma sin nulls
```

### `mk_null_val()` → `num`

Devuelve un valor `null` (NaN). Útil para crear nulls explícitos en funciones.

```zeta
$x = mk_null_val()
print(is_null($x))   # true

fn safe_div($a, $b) {
    if ($b == 0) {
        return mk_null_val()
    }
    return $a / $b
}

print(safe_div(10, 0))   # null
print(safe_div(10, 2))   # 5
```

## 5.6. Diccionarios

### `keys($dict)` → `str_vec`

Nombres de las claves (ordenados alfabéticamente).

```zeta
print(keys({"a": 1, "b": 2, "c": 3}))    # ["a", "b", "c"]
```

### `values($dict)` → `vec`

Valores numéricos del diccionario. Los valores no numéricos se convierten a `null`.

```zeta
print(values({"a": 1, "b": 2, "c": 3}))    # <1, 2, 3>
```

## 5.7. DataFrames

### `select($df, $columna)` → `vec`

Devuelve una columna del DataFrame como vector.

```zeta
$ventas = select($datos, "ventas")    # equivalente a $datos:ventas
```

### `head($df_o_vec, $n=5)` → `df` o `vec`

Primeras N filas/elementos.

```zeta
print(head($datos, 3))       # DataFrame con 3 filas
print(head(<1, 2, 3, 4, 5>)) # <1, 2, 3>
```

### `drop($df, $columna)` → `df`

Elimina una columna del DataFrame y devuelve uno nuevo sin ella.

```zeta
$df = load_csv("datos.csv")
print(keys($df))             # ["id", "nombre", "ventas", "gastos"]

$df_limpio = drop($df, "id")
print(keys($df_limpio))      # ["nombre", "ventas", "gastos"]

# Encadenar múltiples drops
$df_final = drop(drop($df, "id"), "timestamp")
```

### `drop_nan($df, $columna)` → `df`

Elimina todas las filas donde la columna indicada tiene `null`. Devuelve un DataFrame nuevo.

```zeta
$df = load_csv("ventas.csv")
print(len($df))              # 100

# Eliminar filas con null en 'ventas'
$df_limpio = drop_nan($df, "ventas")
print(len($df_limpio))       # 85 (se eliminaron 15 filas con null)

# Combinar con fill_null para limpieza completa
$df:ingresos = fill_null($df:ingresos, 0)
$df = drop_nan($df, "ventas")   # eliminar solo los que realmente faltan
```

## 5.8. Matrices y álgebra

### `transpose($matriz)` → `matriz`

Transpone una matriz (intercambia filas y columnas).

```zeta
$m = <<1, 2, 3>,
      <4, 5, 6>>
print(transpose($m))    # <<1, 4>, <2, 5>, <3, 6>>
```

### `dot($vec1, $vec2)` → `num`

Producto punto de dos vectores. Devuelve `null` si tienen longitudes distintas o si hay nulls.

```zeta
print(dot(<1, 2, 3>, <4, 5, 6>))    # 32 (= 1*4 + 2*5 + 3*6)
```

## 5.9. Programación funcional

### `map($vec, $fn)` → `vec`

Aplica una función a cada elemento. La función recibe un escalar y devuelve un escalar.

```zeta
fn doble($x) { return $x * 2 }
print(map(<1, 2, 3>, doble))    # <2, 4, 6>
```

### `filter($vec, $fn)` → `vec`

Filtra elementos que cumplen una condición. La función recibe un escalar y devuelve un bool.

```zeta
fn mayor_a_2($x) { return $x > 2 }
print(filter(<1, 2, 3, 4, 5>, mayor_a_2))    # <3, 4, 5>
```

### `reduce($vec, $fn, $inicial)` → `num`

Reduce un vector a un solo valor aplicando una función acumuladora. `$fn` recibe `(acumulador, elemento)` y devuelve el nuevo acumulador.

```zeta
fn sumar($acc, $x) { return $acc + $x }
print(reduce(<1, 2, 3, 4, 5>, sumar, 0))    # 15
```

## 5.10. Visualización y reporting

### `plot($data, [$tipo], [$titulo])` → `grafico`

Imprime un plot ASCII en consola (para debugging). `$tipo` puede ser `"line"`, `"bar"`, `"scatter"`. `$data` puede ser un vector o un DataFrame.

```zeta
plot($datos:ventas, "line", "Tendencia")
plot($datos, "bar", "Resumen")
```

Devuelve un `GRAFICO` que se puede usar con `serve()`.

### `metric($nombre, $valor)` → `metrica`

Registra un KPI con nombre y valor. Devuelve un `METRICA` y lo agrega a la lista interna del intérprete (visible vía `/api/metricas`).

```zeta
$total = sum($datos:ventas)
metric("Ventas totales", $total)
metric("Promedio", mean($datos:ventas))
```

### `dashboard($titulo, [$autor])` → `dashboard`

Crea un dashboard con título y autor opcional. Se llena con `add_*` (ver [Escenas y visualización](./docs/09-escenas-visualizacion.md)).

```zeta
$db = dashboard("Mi Reporte", "zeta")
```

### `serve($dashboard, [$puerto], [$archivo])` → `str`

Genera un archivo HTML con el dashboard (Chart.js embebido). **Obsoleto en favor de `zeta_server` + `zeta_dashboard` + `zeta_term`**, pero se mantiene para compatibilidad.

```zeta
serve($db, 8080, "reporte.html")
```

## 5.11. I/O

### `load_csv($ruta, $delim=",")` → `df`

Carga un CSV/TSV como DataFrame. Ver [I/O](./07-io.md).

```zeta
$df = load_csv("tests/datos.csv")
$df_tsv = load_csv("datos.tsv", "\t")
```

### `load_json($ruta)` → `df`

Carga un JSON (array de objetos) como DataFrame.

```zeta
$df = load_json("personas.json")
print($df:nombre)
```

### `load_xlsx($ruta)` → `df`

Carga un archivo Excel como DataFrame.

```zeta
$df = load_xlsx("reporte.xlsx")
```

### `save_csv($ruta, $df, $delim=",")`

Guarda un DataFrame como CSV.

```zeta
save_csv("backup.csv", $df)
save_csv("datos.tsv", $df, "\t")
```

### `save_xlsx($ruta, $df)`

Guarda un DataFrame como Excel.

```zeta
save_xlsx("reporte.xlsx", $df)
```

### `guardar_grafo($ruta)`

Guarda la escena actual como JSON.

```zeta
$scn = scene("Mi Dashboard", "zeta")
add_metric($scn, "Total", 42)
guardar_grafo("escena.json")
```

### `cargar_grafo($ruta)` → `scene`

Carga una escena desde un JSON guardado.

```zeta
$scn = cargar_grafo("escena.json")
```

### `grafo_actual()` → `scene`

Obtiene la escena actual (la última creada con `scene()`).

```zeta
$scn = scene("Test", "zeta")
add_metric($scn, "X", 10)
$current = grafo_actual()
```

### `load_lib($ruta, $dict_funciones)` → `str`

Carga una librería nativa (`.so`/`.dll`) y registra funciones. Ver [C ABI](./10-c-abi-loadlib.md).

```zeta
load_lib("libtestnative.so", {
    "native_pow": "fn($a, $b) -> $a",
    "native_pi": "fn() -> $x"
})
```

## 5.12. Sistema y errores

### `mk_err($tipo, $mensaje)` → `err`

Crea un valor de error. Útil para devolver errores desde funciones.

```zeta
fn dividir($a, $b) {
    if ($b == 0) {
        return mk_err("division", "división por cero")
    }
    return $a / $b
}
```

### `mk_null_val()` → `null`

Crea un valor null explícitamente.

```zeta
$z = mk_null_val()
print(is_null($z))    # true
```

### `time()` → `num`

Devuelve el timestamp actual en segundos (epoch time). Útil para medir tiempos de ejecución.

```zeta
$t0 = time()
# ... operación larga ...
$t1 = time()
print("Tiempo:", $t1 - $t0, "segundos")
```

## 5.13. Tabla resumen rápida

| Función | Categoría | Firma | Retorna |
|---------|-----------|-------|---------|
| `sum` | stats | `sum(vec)` | num |
| `mean` | stats | `mean(vec)` | num |
| `min` | stats | `min(vec)` | num |
| `max` | stats | `max(vec)` | num |
| `stddev` | stats | `stddev(vec)` | num |
| `count` | stats | `count(vec)` | num |
| `abs` | math | `abs(num)` | num |
| `round` | math | `round(num)` | num |
| `floor` | math | `floor(num)` | num |
| `ceil` | math | `ceil(num)` | num |
| `pow` | math | `pow(num, num)` | num |
| `sqrt` | math | `sqrt(num)` | num |
| `len` | universal | `len(x)` | num |
| `upper` | str | `upper(str)` | str |
| `lower` | str | `lower(str)` | str |
| `substr` | str | `substr(str, num, [num])` | str |
| `split` | str | `split(str, str)` | str_vec |
| `join` | str | `join(str_vec, str)` | str |
| `replace` | str | `replace(str, str, str)` | str |
| `find` | str | `find(str, str)` | num |
| `push` | vec | `push(vec, num)` | vec |
| `reverse` | vec | `reverse(vec)` | vec |
| `sort` | vec | `sort(vec)` | vec |
| `unique` | vec | `unique(vec)` | vec |
| `range` | vec | `range(num, [num], [num])` | vec |
| `type` | types | `type(x)` | str |
| `is_null` | null | `is_null(x)` | bool/bool_vec |
| `is_error` | errors | `is_error(x)` | bool |
| `keys` | dict | `keys(dict)` | str_vec |
| `values` | dict | `values(dict)` | vec |
| `select` | df | `select(df, str)` | vec |
| `head` | df/vec | `head(x, [num])` | df/vec |
| `drop` | df | `drop(df, str)` | df |
| `drop_nan` | df | `drop_nan(df, str)` | df |
| `transpose` | matriz | `transpose(matriz)` | matriz |
| `dot` | matriz | `dot(vec, vec)` | num |
| `map` | functional | `map(vec, fn)` | vec |
| `filter` | functional | `filter(vec, fn)` | vec |
| `reduce` | functional | `reduce(vec, fn, x)` | num |
| `plot` | viz | `plot(data, [str], [str])` | grafico |
| `metric` | viz | `metric(str, num)` | metrica |
| `dashboard` | viz | `dashboard(str, [str])` | dashboard |
| `serve` | viz | `serve(dashboard, [num], [str])` | str |
| `load_csv` | io | `load_csv(str, [str])` | df |
| `load_json` | io | `load_json(str)` | df |
| `load_xlsx` | io | `load_xlsx(str)` | df |
| `save_csv` | io | `save_csv(str, df, [str])` | null |
| `save_xlsx` | io | `save_xlsx(str, df)` | null |
| `guardar_grafo` | io | `guardar_grafo(str)` | null |
| `cargar_grafo` | io | `cargar_grafo(str)` | scene |
| `grafo_actual` | io | `grafo_actual()` | scene |
| `load_lib` | ffi | `load_lib(str, dict)` | str |
| `mk_err` | errors | `mk_err(str, str)` | err |
| `mk_null_val` | null | `mk_null_val()` | num (NaN) |
| `time` | sys | `time()` | num |
| `print` | io | `print(...)` | null |
