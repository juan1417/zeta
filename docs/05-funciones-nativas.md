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

## 5.8. Data Manipulation

### `group_by($df, $col1, [$col2...])` → `dict`

Groups a DataFrame by one or more columns. Returns a grouped structure that can be used with `agg`.

```zeta
$ventas = {
    "canal": <"online", "tienda", "online", "tienda">,
    "monto": <100, 200, 150, 250>
}

$grouped = group_by($ventas, "canal")
print($grouped)  # Dict with _type, _data, _group_cols, _groups
```

### `agg($grouped, $col, $func)` → `DataFrame`

Aggregates a grouped DataFrame column. Supported functions: `sum`, `mean`, `count`, `min`, `max`, `stddev`, `median`.

```zeta
$grouped = group_by($ventas, "canal")
$result = agg($grouped, "monto", "sum")
print($result)
# canal  | monto_sum
# online | 250
# tienda | 450
```

Multiple group columns:
```zeta
$grouped = group_by($ventas, "canal", "region")
$result = agg($grouped, "monto", "mean")
```

### `merge($df1, $df2, $on)` → `DataFrame`

Inner join two DataFrames on a key column.

```zeta
$clientes = {
    "id": <1, 2, 3>,
    "nombre": <"Ana", "Bob", "Carol">
}
$compras = {
    "cliente_id": <1, 2, 1, 3>,
    "producto": <"A", "B", "C">
}

$merged = merge($clientes, $compras, "id")
print($merged)
# id | nombre | cliente_id | producto
# 1  | Ana    | 1          | A
# 1  | Ana    | 1          | C
# 2  | Bob    | 2          | B
# 3  | Carol  | 3          | A
```

## 5.9. Matrices y álgebra


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

## 5.10. Programación funcional

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

## 5.11. Visualización y reporting

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

## 5.12. I/O

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

## 5.13. Sistema y errores

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

## 5.14. Gestión de memoria (v0.2)

### `clear_arena()` → `null`

Resetea el arena allocator, liberando todas las allocaciones temporales de golpe. O(1) — no importa cuántos valores haya. Los valores almacenados en variables (via `shared_ptr`) NO se ven afectados.

```zeta
# Útil en scripts largos que cargan muchos datos
$data = load_csv("archivo_grande.csv")
# ... procesamiento ...
clear_arena()  # Libera memoria temporal
```

### `arena_bytes()` → `num`

Devuelve el número de bytes actualmente allocados en el arena. Útil para profiling.

```zeta
print("Arena:", arena_bytes(), "bytes")
clear_arena()
print("Arena después de clear:", arena_bytes(), "bytes")
```

### `zeta_version()` → `string`

Devuelve la versión actual del lenguaje Zeta.

```zeta
print("Zeta version:", zeta_version())  # "0.2.0"
```

## 5.15. Exploración de Datos

### `info($df)` → `string`

Muestra información resumen de un DataFrame: columnas, tipos, valores no-nulos, nulos, y porcentaje de nulls.

```zeta
$df = load_csv("ventas.csv")
info($df)
# DataFrame: 10,000 filas x 8 columnas
# 
# Columna      Tipo    No-Null   Null    %Null
# --------------------------------------------------
# id           num     10,000    0       0.0%
# nombre       str     9,850     150     1.5%
# edad         num     9,700     300     3.0%
# salario      num     10,000    0       0.0%
# departamento str     9,500     500     5.0%
# 
# Tipos: num(3), str(2), bool(0)
```

### `describe($df)` → `string`

Muestra estadísticas descriptivas para columnas numéricas: count, mean, std, min, 25%, 50%, 75%, max.

```zeta
describe($df)
# Estadisticas: 4 columnas numericas
# 
#        id        edad      salario     score
# ------------------------------------------------
# count   10000     9700      10000       9200
# mean    5000.0    34.5      52000.0     72.3
# std     2887.0    12.1      15000.0     18.5
# min     1.0       18.0      25000.0     0.0
# 25%     2500.0    25.0      42000.0     58.0
# 50%     5000.0    33.0      50000.0     74.0
# 75%     7500.0    42.0      60000.0     88.0
# max     10000.0   65.0      120000.0    100.0
```

### `value_counts($col)` → `DataFrame`

Cuenta la frecuencia de cada valor único en un vector. Retorna DataFrame con columnas `valor` y `count`.

```zeta
value_counts($df:canal)
# canal    | count
# ---------------
# online   | 2500
# tienda   | 2000
# rrhh     | 1500

value_counts(<1, 1, 2, 3, 3, 3>)
# valor | count
# -------------
# 3     | 3
# 1     | 2
# 2     | 1
```

### `median($vec)` → `num`

Retorna el valor mediano (percentil 50).

```zeta
median(<1, 2, 3, 4, 5>)  # 3
median(<1, 2, 3, 4>)     # 2.5
median(<10, null, 30>)    # 10 (ignora nulls)
```

### `percentile($vec, $q)` → `num`

Retorna el percentil q (0-100) con interpolación lineal.

```zeta
percentile(<1,2,3,4,5>, 75)  # 4.0
percentile(<1,2,3,4,5>, 25)  # 2.0
percentile(<1,2,3,4,5>, 50)  # 3.0
```

### `mode($vec)` → `num`

Retorna el valor más frecuente.

```zeta
mode(<1, 1, 2, 3, 3, 3>)  # 3
mode(<5, 5, 5, 1, 2>)     # 5
```

### `cor($v1, $v2)` → `num`

Correlación de Pearson entre dos vectores (-1 a 1).

```zeta
cor(<1,2,3,4,5>, <2,4,6,8,10>)  # 1.0 (correlación perfecta)
cor($df:edad, $df:score)         # 0.85
```

### `cov($v1, $v2)` → `num`

Covarianza entre dos vectores.

```zeta
cov(<1,2,3,4,5>, <2,4,6,8,10>)  # 4.0
cov($df:edad, $df:score)         # 12.5
```

### `nunique($vec)` → `num`

Cantidad de valores únicos (ignora nulls).

```zeta
nunique(<1, 2, 2, 3, 3, 3>)  # 3
nunique($df:canal)             # 5
```

### `isna($df | $vec)` → `DataFrame` o `vector<bool>`

Retorna DataFrame o vector booleano indicando posición de nulls.

```zeta
isna($df)  # DataFrame de bools (True donde hay null)
isna(<1, null, 3>)  # <false, true, false>
```

### `duplicated($vec)` → `vector<bool>`

Retorna vector booleano indicando duplicados (True en la segunda aparición en adelante).

```zeta
duplicated(<1, 2, 1, 3>)  # <false, false, true, false>
duplicated(<"a", "b", "a">)  # <false, false, true>
```

### `cut($vec, $bins)` → `vector`

Discretiza un vector numérico en N bins de igual ancho. Retorna vector 1-indexed.

```zeta
cut(<1,2,3,4,5>, 3)  # <1, 1, 2, 2, 3>
cut(<10,20,30,40,50>, 2)  # <1, 1, 2, 2, 2>
```

### `qcut($vec, $q)` → `vector`

Discretiza un vector numérico en q cuantiles. Retorna vector 1-indexed.

```zeta
qcut(<1,2,3,4,5>, 4)  # <1, 1, 2, 3, 4>
qcut(<1,2,3,4,5,6,7,8>, 4)  # <1, 1, 2, 2, 3, 3, 4, 4>
```

### `tail($df | $vec, $n=5)` → `DataFrame` o `vector`

Retorna las últimas N filas o elementos.

```zeta
tail($df, 5)    # Últimas 5 filas
tail($vec, 3)   # Últimos 3 elementos
tail($str_vec, 2)  # Últimos 2 strings
```

### `sample($df, $n=5)` → `DataFrame`

Retorna una muestra aleatoria de N filas (sin reemplazo).

```zeta
sample($df, 10)  # 10 filas aleatorias
sample($df, 100)  # 100 filas aleatorias
```

## 5.16. Distribuciones Estadísticas

### `dnorm($x, [$mean], [$sd])` → `num`

Densidad de probabilidad de la distribución normal.

```zeta
dnorm(0)          # 0.3989 (normal estándar)
dnorm(1.96)       # 0.0584
dnorm(0, 0, 1)    # 0.3989 (misma que arriba)
dnorm(5, 10, 2)   # 0.0883 (normal media=10, sd=2)
```

### `pnorm($x, [$mean], [$sd])` → `num`

Función de distribución acumulada (CDF) de la normal.

```zeta
pnorm(0)          # 0.5
pnorm(1.96)       # 0.975
pnorm(-1.96)      # 0.025
```

### `qnorm($p, [$mean], [$sd])` → `num`

Quantil (inversa de la CDF) de la normal.

```zeta
qnorm(0.5)        # 0.0
qnorm(0.975)      # 1.96
qnorm(0.025)      # -1.96
```

### `dgamma($x, $shape, $rate)` → `num`

Densidad de probabilidad de la distribución gamma.

```zeta
dgamma(1, 2, 1)   # 0.368
dgamma(2, 3, 1)   # 0.271
```

### `dbeta($x, $alpha, $beta)` → `num`

Densidad de probabilidad de la distribución beta.

```zeta
dbeta(0.5, 2, 2)  # 1.5
dbeta(0.3, 1, 5)  # 2.62
```

### `dunif($x, [$min], [$max])` → `num`

Densidad de probabilidad de la distribución uniforme.

```zeta
dunif(0.5)        # 1.0 (en [0,1])
dunif(1.5)        # 0.0 (fuera de [0,1])
dunif(3, 0, 10)   # 0.1
```

### `dt_dist($x, $df)` → `num`

Densidad de probabilidad de la distribución t de Student.

```zeta
dt_dist(0, 10)    # 0.389
dt_dist(2, 5)     # 0.068
```

### `df_dist($x, $df1, $df2)` → `num`

Densidad de probabilidad de la distribución F.

```zeta
df_dist(1, 5, 10)   # 0.000013
df_dist(2, 10, 20)  # 0.0013
```

### `dchisq($x, $df)` → `num`

Densidad de probabilidad de la distribución chi-cuadrado.

```zeta
dchisq(2, 3)      # 2.46
dchisq(5, 4)      # 0.176
```

## 5.17. Testing Estadístico

### `t_test($sample1, $sample2)` → `dict`

Test t de Student para dos muestras independientes.

Retorna: `t_statistic`, `df`, `p_value`, `significant` (1 si p < 0.05)

```zeta
$a = <10, 12, 14, 16, 18>
$b = <20, 22, 24, 26, 28>
result = t_test($a, $b)
print(result:t_statistic)  # -5.0
print(result:p_value)      # 0.001
print(result:significant)  # 1 (rechaza H0)
```

### `anova($group1, $group2, [...])` → `dict`

Análisis de varianza de una vía (one-way ANOVA).

Retorna: `f_statistic`, `groups`

```zeta
$g1 = <10, 12, 14>
$g2 = <20, 22, 24>
$g3 = <30, 32, 34>
result = anova($g1, $g2, $g3)
print(result:f_statistic)  # 75.0
```

### `chi_square($observed, $expected)` → `dict`

Test de chi-cuadrado de bondad de ajuste.

Retorna: `chi_square`, `df`

```zeta
$obs = <50, 30, 20>
$exp = <40, 40, 20>
result = chi_square($obs, $exp)
print(result:chi_square)  # 5.0
print(result:df)          # 2
```

## 5.18. Regresión

### `linear_regression($x, $y)` → `dict`

Regresión lineal simple.

Retorna: `slope`, `intercept`, `r_squared`, `std_error`, `predicted`, `formula`

```zeta
$x = <1, 2, 3, 4, 5>
$y = <2, 4, 5, 4, 5>
result = linear_regression($x, $y)
print(result:slope)        # 0.6
print(result:intercept)    # 2.2
print(result:r_squared)    # 0.6
print(result:formula)      # "y = 0.6 * x + 2.2"
print(result:predicted)    # <2.8, 3.4, 4.0, 4.6, 5.2>
```

## 5.19. Window Functions

### `cumsum($vec)` → `vector`

Suma acumulada.

```zeta
cumsum(<10, 20, 30, 40>)  # <10, 30, 60, 100>
```

### `cummax($vec)` → `vector`

Máximo acumulada.

```zeta
cummax(<10, 30, 20, 40>)  # <10, 30, 30, 40>
```

### `cummin($vec)` → `vector`

Mínimo acumulado.

```zeta
cummin(<40, 20, 30, 10>)  # <40, 20, 20, 10>
```

### `rolling_mean($vec, $window)` → `vector`

Promedio móvil con ventana deslizante.

```zeta
$ventas = <100, 150, 120, 180, 200>
rolling_mean($ventas, 3)
# [100, 125, 123.3, 150, 166.7]
# Solo tiene 3 elementos para el cálculo completo
```

### `rolling_std($vec, $window)` → `vector`

Desviación estándar móvil.

```zeta
rolling_std($ventas, 3)  # [<null>, 25.0, 20.5, 24.5, 34.0]
```

### `rolling_sum($vec, $window)` → `vector`

Suma móvil.

```zeta
rolling_sum($ventas, 3)  # [100, 250, 370, 450, 500]
```

### `rolling_min($vec, $window)` → `vector`

Mínimo móvil.

```zeta
rolling_min($ventas, 3)  # [100, 100, 100, 120, 120]
```

### `rolling_max($vec, $window)` → `vector`

Máximo móvil.

```zeta
rolling_max($ventas, 3)  # [100, 150, 150, 180, 200]
```

### `lag($vec, $n)` → `vector`

Valor de hace n filas (shift hacia atrás).

```zeta
lag(<10, 20, 30, 40>, 1)  # <null, 10, 20, 30>
lag(<10, 20, 30, 40>, 2)  # <null, null, 10, 20>
```

### `lead($vec, $n)` → `vector`

Valor dentro de n filas (shift hacia adelante).

```zeta
lead(<10, 20, 30, 40>, 1)  # <20, 30, 40, null>
lead(<10, 20, 30, 40>, 2)  # <30, 40, null, null>
```

### `diff($vec, $n)` → `vector`

Diferencia con valor anterior (n filas atrás).

```zeta
diff(<100, 150, 120, 180>, 1)  # <null, 50, -30, 60>
diff(<100, 150, 120, 180>, 2)  # <null, null, 20, 30>
```

### `row_number($vec)` → `vector`

Número de fila secuencial (1, 2, 3...).

```zeta
row_number(<10, 20, 30>)  # <1, 2, 3>
```

### `rank($vec)` → `vector`

Ranking con manejo de empates (promedio de rangos).

```zeta
rank(<85, 92, 78, 92, 88>)  # <2, 4.5, 1, 4.5, 3>
# 78 es el menor → rank 1
# 85 es segundo → rank 2
# 88 es tercero → rank 3
# 92 y 92 empatan → ranks 4 y 5, promedio = 4.5
```

### `pct_change($vec, $n)` → `vector`

Cambio porcentual vs valor anterior (n filas atrás).

```zeta
pct_change(<100, 150, 120, 180>, 1)
# <null, 50%, -20%, 50%>
# (150-100)/100 = 50%
# (120-150)/150 = -20%
```

## 5.20. Tabla resumen rápida

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
| `group_by` | manipulation | `group_by(df, str, ...)` | dict |
| `agg` | manipulation | `agg(dict, str, str)` | df |
| `merge` | manipulation | `merge(df, df, str)` | df |
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
| `clear_arena` | memory | `clear_arena()` | null |
| `arena_bytes` | memory | `arena_bytes()` | num |
| `zeta_version` | sys | `zeta_version()` | str |
| `info` | explore | `info(df)` | string |
| `describe` | explore | `describe(df)` | string |
| `value_counts` | explore | `value_counts(vec)` | df |
| `median` | stats | `median(vec)` | num |
| `percentile` | stats | `percentile(vec, num)` | num |
| `mode` | stats | `mode(vec)` | num |
| `cor` | stats | `cor(vec, vec)` | num |
| `cov` | stats | `cov(vec, vec)` | num |
| `nunique` | explore | `nunique(vec)` | num |
| `isna` | explore | `isna(df/vec)` | df/bool_vec |
| `duplicated` | explore | `duplicated(vec)` | bool_vec |
| `cut` | explore | `cut(vec, num)` | vec |
| `qcut` | explore | `qcut(vec, num)` | vec |
| `tail` | explore | `tail(df/vec, [num])` | df/vec |
| `sample` | explore | `sample(df, [num])` | df |
| `dnorm` | dist | `dnorm(num, [num], [num])` | num |
| `pnorm` | dist | `pnorm(num, [num], [num])` | num |
| `qnorm` | dist | `qnorm(num, [num], [num])` | num |
| `dgamma` | dist | `dgamma(num, num, num)` | num |
| `dbeta` | dist | `dbeta(num, num, num)` | num |
| `dunif` | dist | `dunif(num, [num], [num])` | num |
| `dt_dist` | dist | `dt_dist(num, num)` | num |
| `df_dist` | dist | `df_dist(num, num, num)` | num |
| `dchisq` | dist | `dchisq(num, num)` | num |
| `t_test` | test | `t_test(vec, vec)` | dict |
| `anova` | test | `anova(vec, vec, ...)` | dict |
| `chi_square` | test | `chi_square(vec, vec)` | dict |
| `linear_regression` | reg | `linear_regression(vec, vec)` | dict |
| `cumsum` | window | `cumsum(vec)` | vec |
| `cummax` | window | `cummax(vec)` | vec |
| `cummin` | window | `cummin(vec)` | vec |
| `rolling_mean` | window | `rolling_mean(vec, num)` | vec |
| `rolling_std` | window | `rolling_std(vec, num)` | vec |
| `rolling_sum` | window | `rolling_sum(vec, num)` | vec |
| `rolling_min` | window | `rolling_min(vec, num)` | vec |
| `rolling_max` | window | `rolling_max(vec, num)` | vec |
| `lag` | window | `lag(vec, num)` | vec |
| `lead` | window | `lead(vec, num)` | vec |
| `diff` | window | `diff(vec, num)` | vec |
| `row_number` | window | `row_number(vec)` | vec |
| `rank` | window | `rank(vec)` | vec |
| `pct_change` | window | `pct_change(vec, num)` | vec |
| `drop_duplicates` | clean | `drop_duplicates(df)` | df |
| `rename` | clean | `rename(df, str, str)` | df |
| `select_cols` | clean | `select_cols(df, vec)` | df |
| `drop_cols` | clean | `drop_cols(df, vec)` | df |
| `fillna` | clean | `fillna(vec, str/num)` | vec |
| `replace_val` | clean | `replace_val(df, str, str, str)` | df |
| `clip` | clean | `clip(vec, num, num)` | vec |
| `trim` | clean | `trim(vec, num)` | vec |
| `normalize` | clean | `normalize(vec)` | vec |
| `standardize` | clean | `standardize(vec)` | vec |
| `plugin` | plugin | `plugin(str)` | bool |
| `plugin_info` | plugin | `plugin_info()` | dict |
