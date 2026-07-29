# 5. Funciones Nativas — Referencia Completa

Las funciones nativas (built-in) se llaman sin keyword especial (`print(x)`, no `zeta.print(x)`). El lexer reconoce sus nombres como tokens reservados. Esta es la lista completa, agrupada por categoría.

---

## Tabla de Contenidos

- [5.1 Resumen Rápido](#51-resumen-rápido)
- [5.2 Estadística y Agregación](#52-estadística-y-agregación)
- [5.3 Matemáticas Escalares](#53-matemáticas-escalares)
- [5.4 Cadenas](#54-cadenas)
- [5.5 Vectores y Manipulación](#55-vectores-y-manipulación)
- [5.6 Diccionarios](#56-diccionarios)
- [5.7 DataFrames](#57-dataframes)
- [5.8 Exploración de Datos](#58-exploración-de-datos)
- [5.9 Distribuciones Estadísticas](#59-distribuciones-estadísticas)
- [5.10 Testing Estadístico y Regresión](#510-testing-estadístico-y-regresión)
- [5.11 Window Functions](#511-window-functions)
- [5.12 Limpieza de Datos](#512-limpieza-de-datos)
- [5.13 I/O](#513-io)
- [5.14 Visualización y Reporting](#514-visualización-y-reporting)
- [5.15 Inspección y Tipos](#515-inspección-y-tipos)
- [5.16 Programación Funcional](#516-programación-funcional)
- [5.17 Matrices y Álgebra](#517-matrices-y-álgebra)
- [5.18 Sistema y Errores](#518-sistema-y-errores)
- [5.19 Gestión de Memoria (v0.2)](#519-gestión-de-memoria-v02)
- [5.20 Plugins](#520-plugins)

---

## 5.1 Resumen Rápido

| Categoría | Funciones | Cantidad |
|-----------|-----------|----------|
| Estadística | sum, mean, min, max, stddev, count, median, percentile, mode, cor, cov | 11 |
| Matemáticas | abs, round, floor, ceil, pow, sqrt, format | 7 |
| Cadenas | len, upper, lower, substr, split, join, replace, find | 8 |
| Vectores | push, reverse, sort, unique, range, head, map, filter, reduce | 9 |
| DataFrames | select, drop, drop_nan, group_by, agg, merge | 6 |
| Exploración | info, describe, tail, sample, value_counts, nunique, isna, duplicated, cut, qcut | 10 |
| Distribuciones | dnorm, pnorm, qnorm, dgamma, dbeta, dunif, dt_dist, df_dist, dchisq | 9 |
| Statistical Tests | t_test, anova, chi_square, linear_regression | 4 |
| Window Functions | cumsum, cummax, cummin, rolling_mean, rolling_std, rolling_sum, rolling_min, rolling_max, lag, lead, diff, row_number, rank, pct_change | 14 |
| Limpieza de Datos | drop_duplicates, rename, select_cols, drop_cols, fillna, replace_val, clip, trim, normalize, standardize | 10 |
| I/O | load_csv, load_json, load_xlsx, save_csv, save_xlsx, guardar_grafo, cargar_grafo, grafo_actual | 8 |
| Visualización | plot, metric, dashboard, serve | 4 |
| Inspección | type, is_null, is_error, fill_null, mk_null_val | 5 |
| Programación Funcional | map, filter, reduce | 3 |
| Matrices | transpose, dot | 2 |
| Sistema | print, time, load_lib, mk_err | 4 |
| Diccionarios | keys, values | 2 |
| Memoria | clear_arena, arena_bytes, zeta_version | 3 |
| Plugins | plugin, plugin_info | 2 |

**Total: ~107 funciones nativas**

---

## 5.2 Estadística y Agregación

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `sum` | `sum($vec)` | `num` | Suma de elementos (nulls ignorados) |
| `mean` | `mean($vec)` | `num` | Promedio aritmético |
| `min` | `min($vec)` | `num` | Mínimo |
| `max` | `max($vec)` | `num` | Máximo |
| `stddev` | `stddev($vec)` | `num` | Desviación estándar poblacional |
| `count` | `count($vec)` | `num` | Cantidad de elementos no nulos |
| `median` | `median($vec)` | `num` | Mediana (percentil 50) |
| `percentile` | `percentile($vec, $p)` | `num` | Percentil (0-100) con interpolación lineal |
| `mode` | `mode($vec)` | `num` | Valor más frecuente |
| `cor` | `cor($v1, $v2)` | `num` | Correlación de Pearson (-1 a 1) |
| `cov` | `cov($v1, $v2)` | `num` | Covarianza |

---

### `sum($vec)` → `num`

Suma de los elementos de un vector. NaN/null se ignoran (pero si todos son null, devuelve `null`).

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$vec` | `vec` | Sí | Vector numérico |

**Ejemplo:**

```zeta
print(sum(<1, 2, 3, 4, 5>))     # 15
print(sum(<1, null, 3>))         # 4 (null ignorado)
print(sum($df:ventas))           # suma de columna
```

**Notas:** Si todos los elementos son null, retorna `null`.

---

### `mean($vec)` → `num`

Promedio aritmético. Internamente `sum(vec) / count(vec)`, ignorando nulls.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$vec` | `vec` | Sí | Vector numérico |

**Ejemplo:**

```zeta
print(mean(<10, 20, 30>))    # 20
print(mean($df:ventas))      # promedio de columna
```

---

### `min($vec)` / `max($vec)` → `num`

Mínimo y máximo. Nulls se ignoran.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$vec` | `vec` | Sí | Vector numérico |

**Ejemplo:**

```zeta
print(min(<3, 1, 4, 1, 5>))   # 1
print(max($df:temperatura))    # máximo de columna
```

---

### `stddev($vec)` → `num`

Desviación estándar (poblacional, divide entre N, no N-1). Nulls se ignoran.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$vec` | `vec` | Sí | Vector numérico |

**Ejemplo:**

```zeta
print(stddev(<1, 2, 3, 4, 5>))    # 1.41421...
```

**Notas:** Usa la fórmula poblacional (divide entre N).

---

### `count($vec)` → `num`

Número de elementos **no nulos**.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$vec` | `vec` | Sí | Vector |

**Ejemplo:**

```zeta
print(count(<1, 2, 3, 4, 5>))     # 5
print(count(<1, null, 3, null>))   # 2
```

---

### `median($vec)` → `num`

Retorna el valor mediano (percentil 50). Nulls se ignoran.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$vec` | `vec` | Sí | Vector numérico |

**Ejemplo:**

```zeta
print(median(<1, 2, 3, 4, 5>))    # 3
print(median(<1, 2, 3, 4>))       # 2.5
print(median(<10, null, 30>))     # 10 (ignora nulls)
```

**Notas:** Para vectores pares, promedia los dos valores centrales.

---

### `percentile($vec, $p)` → `num`

Retorna el percentil `$p` (0-100) con interpolación lineal. Nulls se ignoran.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$vec` | `vec` | Sí | Vector numérico |
| `$p` | `num` | Sí | Percentil (0-100) |

**Ejemplo:**

```zeta
print(percentile(<1,2,3,4,5>, 75))    # 4.0
print(percentile(<1,2,3,4,5>, 25))    # 2.0
print(percentile(<1,2,3,4,5>, 50))    # 3.0 (igual a median)
```

---

### `mode($vec)` → `num`

Retorna el valor más frecuente. Si hay empate, retorna el primero encontrado.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$vec` | `vec` | Sí | Vector |

**Ejemplo:**

```zeta
print(mode(<1, 1, 2, 3, 3, 3>))    # 3
print(mode(<5, 5, 5, 1, 2>))       # 5
```

---

### `cor($v1, $v2)` → `num`

Correlación de Pearson entre dos vectores. Retorna un valor entre -1 y 1.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$v1` | `vec` | Sí | Primer vector numérico |
| `$v2` | `vec` | Sí | Segundo vector numérico |

**Ejemplo:**

```zeta
print(cor(<1,2,3,4,5>, <2,4,6,8,10>))    # 1.0 (correlación perfecta)
print(cor($df:edad, $df:score))           # 0.85
```

**Notas:** Los vectores deben tener la misma longitud. Devuelve `null` si hay longitudes distintas o si todos los valores son null.

---

### `cov($v1, $v2)` → `num`

Covarianza entre dos vectores.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$v1` | `vec` | Sí | Primer vector numérico |
| `$v2` | `vec` | Sí | Segundo vector numérico |

**Ejemplo:**

```zeta
print(cov(<1,2,3,4,5>, <2,4,6,8,10>))    # 4.0
print(cov($df:edad, $df:score))           # 12.5
```

**Notas:** Los vectores deben tener la misma longitud.

---

## 5.3 Matemáticas Escalares

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `abs` | `abs($x)` | `num` | Valor absoluto |
| `round` | `round($x)` | `num` | Redondeo al entero más cercano |
| `floor` | `floor($x)` | `num` | Redondeo hacia abajo |
| `ceil` | `ceil($x)` | `num` | Redondeo hacia arriba |
| `pow` | `pow($base, $exp)` | `num` | Potencia |
| `sqrt` | `sqrt($x)` | `num` | Raíz cuadrada |
| `format` | `format($num, $decimals)` | `str` | Formateo de número con decimales |

---

### `abs($x)` → `num`

Valor absoluto.

```zeta
print(abs(-42))    # 42
print(abs(3.14))   # 3.14
```

---

### `round($x)` → `num`

Redondeo al entero más cercano (usa `std::round` de C, no banker's rounding).

```zeta
print(round(3.7))    # 4
print(round(3.4))    # 3
print(round(-3.5))   # -4
```

---

### `floor($x)` / `ceil($x)` → `num`

Redondeo hacia abajo / hacia arriba.

```zeta
print(floor(3.7))    # 3
print(ceil(3.2))     # 4
print(floor(-3.2))   # -4
print(ceil(-3.7))    # -3
```

---

### `pow($base, $exp)` → `num`

Potencia. Equivalente a `$base ** $exp`.

```zeta
print(pow(2, 10))    # 1024
print(pow(3, 2))     # 9
print(pow(2, 0.5))   # 1.41421 (raíz cuadrada via pow)
```

---

### `sqrt($x)` → `num`

Raíz cuadrada. Devuelve `null` para `$x < 0`.

```zeta
print(sqrt(144))    # 12
print(sqrt(2))      # 1.41421
print(sqrt(-1))     # null
```

---

### `format($num, $decimals)` → `str`

Formatea un número con `$decimals` decimales. Útil para mostrar resultados legibles.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$num` | `num` | Sí | Número a formatear |
| `$decimals` | `num` | Sí | Cantidad de decimales |

```zeta
print(format(3.14159, 2))    # "3.14"
print(format(1000.0, 0))     # "1000"
print(format(0.123456, 4))   # "0.1235"
```

---

## 5.4 Cadenas

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `len` | `len($str)` | `num` | Longitud en bytes |
| `upper` | `upper($str)` | `str` | A mayúsculas |
| `lower` | `lower($str)` | `str` | A minúsculas |
| `substr` | `substr($str, $inicio, [$longitud])` | `str` | Subcadena |
| `split` | `split($str, $sep)` | `str_vec` | Dividir por separador |
| `join` | `join($str_vec, $sep)` | `str` | Unir con separador |
| `replace` | `replace($str, $old, $new)` | `str` | Reemplazar ocurrencias |
| `find` | `find($str, $sub)` | `num` | Posición de primera ocurrencia |

---

### `len($str)` → `num`

Longitud en bytes (no en caracteres UTF-8; tenlo en cuenta con emojis).

```zeta
print(len("hola"))         # 4
print(len(""))             # 0
print(len("ñoño"))         # 6 (cada char UTF-8 ocupa 2 bytes)
```

---

### `upper($str)` / `lower($str)` → `str`

A mayúsculas / minúsculas.

```zeta
print(upper("hola"))    # "HOLA"
print(lower("HOLA"))    # "hola"
```

---

### `substr($str, $inicio, [$longitud])` → `str`

Subcadena desde `$inicio` (0-based) hasta el final o por `$longitud` caracteres. Soporta índices negativos (cuentan desde el final).

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$str` | `str` | Sí | Cadena de entrada |
| `$inicio` | `num` | Sí | Índice inicial (0-based, negativos = desde el final) |
| `$longitud` | `num` | No | Longitud de la subcadena (default: hasta el final) |

```zeta
print(substr("hola mundo", 5))        # "mundo"
print(substr("hola mundo", 0, 4))    # "hola"
print(substr("hola mundo", -5))      # "mundo"
```

---

### `split($str, $sep)` → `str_vec`

Divide un string por separador.

```zeta
print(split("a,b,c", ","))           # ["a", "b", "c"]
print(split("uno dos tres", " "))    # ["uno", "dos", "tres"]
```

---

### `join($str_vec, $sep)` → `str`

Une un vector de strings con un separador.

```zeta
print(join(["a", "b", "c"], "-"))    # "a-b-c"
print(join(["x", "y", "z"], ""))      # "xyz"
```

---

### `replace($str, $old, $new)` → `str`

Reemplaza todas las ocurrencias de `$old` por `$new`.

```zeta
print(replace("hola mundo", "mundo", "Zeta"))    # "hola Zeta"
print(replace("aaa", "a", "b"))                  # "bbb"
```

---

### `find($str, $sub)` → `num`

Posición de la primera ocurrencia de `$sub` en `$str`. Devuelve `-1` si no se encuentra.

```zeta
print(find("hola mundo", "mundo"))    # 5
print(find("hola mundo", "Zeta"))     # -1
```

---

## 5.5 Vectores y Manipulación

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `len` | `len($vec)` | `num` | Número de elementos |
| `push` | `push($vec, $valor)` | `vec` | Agrega elemento al final (nuevo vector) |
| `reverse` | `reverse($vec)` | `vec` | Invierte el orden |
| `sort` | `sort($vec)` | `vec` | Ordena ascendentemente |
| `unique` | `unique($vec)` | `vec` | Elimina duplicados |
| `range` | `range($fin, [$inicio], [$paso])` | `vec` | Genera rango numérico |
| `head` | `head($x, [$n])` | `df` o `vec` | Primeras N filas/elementos |
| `map` | `map($vec, $fn)` | `vec` | Aplica función a cada elemento |
| `filter` | `filter($vec, $fn)` | `vec` | Filtra por condición |
| `reduce` | `reduce($vec, $fn, $inicial)` | `num` | Reduce a un solo valor |

---

### `len($vec)` → `num`

Número de elementos.

```zeta
print(len(<1, 2, 3, 4, 5>))    # 5
print(len($df:ventas))          # 10
```

---

### `push($vec, $valor)` → `vec`

Agrega un elemento al final. **Devuelve un nuevo vector** (Zeta es inmutable).

```zeta
$v = <1, 2, 3>
$v2 = push($v, 4)
print($v2)    # <1, 2, 3, 4>
print($v)     # <1, 2, 3> (no se modificó)
```

---

### `reverse($vec)` → `vec`

Invierte el orden.

```zeta
print(reverse(<1, 2, 3>))    # <3, 2, 1>
```

---

### `sort($vec)` → `vec`

Ordena ascendentemente.

```zeta
print(sort(<3, 1, 4, 1, 5>))    # <1, 1, 3, 4, 5>
```

---

### `unique($vec)` → `vec`

Elimina duplicados (ordena primero).

```zeta
print(unique(<1, 2, 2, 3, 3, 3>))    # <1, 2, 3>
```

---

### `range($inicio_o_fin, [$fin], [$paso])` → `vec`

Genera un rango numérico. Tres formas:

```zeta
range(5)            # <0, 1, 2, 3, 4>            (un argumento: 0..N)
range(2, 5)         # <2, 3, 4>                   (dos argumentos: inicio..fin exclusivo)
range(0, 10, 2)     # <0, 2, 4, 6, 8>             (tres argumentos: con paso)
```

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$inicio_o_fin` | `num` | Sí | Fin (si 1 arg) o Inicio (si 2-3 args) |
| `$fin` | `num` | No | Fin exclusivo |
| `$paso` | `num` | No | Paso (default: 1) |

---

### `head($df_o_vec, $n=5)` → `df` o `vec`

Primeras N filas/elementos. Default: 5.

```zeta
print(head($datos, 3))         # DataFrame con 3 filas
print(head(<1, 2, 3, 4, 5>))   # <1, 2, 3>
```

---

### `map($vec, $fn)` → `vec`

Aplica una función a cada elemento. La función recibe un escalar y devuelve un escalar.

```zeta
fn doble($x) { return $x * 2 }
print(map(<1, 2, 3>, doble))    # <2, 4, 6>
```

---

### `filter($vec, $fn)` → `vec`

Filtra elementos que cumplen una condición. La función recibe un escalar y devuelve un bool.

```zeta
fn mayor_a_2($x) { return $x > 2 }
print(filter(<1, 2, 3, 4, 5>, mayor_a_2))    # <3, 4, 5>
```

---

### `reduce($vec, $fn, $inicial)` → `num`

Reduce un vector a un solo valor aplicando una función acumuladora. `$fn` recibe `(acumulador, elemento)` y devuelve el nuevo acumulador.

```zeta
fn sumar($acc, $x) { return $acc + $x }
print(reduce(<1, 2, 3, 4, 5>, sumar, 0))    # 15
```

---

## 5.6 Diccionarios

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `keys` | `keys($dict)` | `str_vec` | Nombres de claves (orden alfabético) |
| `values` | `values($dict)` | `vec` | Valores numéricos del diccionario |

---

### `keys($dict)` → `str_vec`

Nombres de las claves (ordenados alfabéticamente).

```zeta
print(keys({"a": 1, "b": 2, "c": 3}))    # ["a", "b", "c"]
```

---

### `values($dict)` → `vec`

Valores numéricos del diccionario. Los valores no numéricos se convierten a `null`.

```zeta
print(values({"a": 1, "b": 2, "c": 3}))    # <1, 2, 3>
```

---

## 5.7 DataFrames

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `select` | `select($df, $columna)` | `vec` | Extrae una columna como vector |
| `drop` | `drop($df, $columna)` | `df` | Elimina una columna |
| `drop_nan` | `drop_nan($df, $columna)` | `df` | Elimina filas con null en columna |
| `group_by` | `group_by($df, $col1, [$col2...])` | `dict` | Agrupa por columnas |
| `agg` | `agg($grouped, $col, $func)` | `df` | Agrega grupo con función |
| `merge` | `merge($df1, $df2, $on)` | `df` | Inner join por columna clave |

---

### `select($df, $columna)` → `vec`

Devuelve una columna del DataFrame como vector. Equivalente a la notación `$df:columna`.

```zeta
$ventas = select($datos, "ventas")    # equivalente a $datos:ventas
```

---

### `head($df_o_vec, $n=5)` → `df` o `vec`

Primeras N filas/elementos.

```zeta
print(head($datos, 3))         # DataFrame con 3 filas
print(head(<1, 2, 3, 4, 5>))   # <1, 2, 3>
```

---

### `drop($df, $columna)` → `df`

Elimina una columna del DataFrame y devuelve uno nuevo sin ella.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$df` | `df` | Sí | DataFrame de entrada |
| `$columna` | `str` | Sí | Nombre de columna a eliminar |

```zeta
$df = load_csv("datos.csv")
print(keys($df))             # ["id", "nombre", "ventas", "gastos"]

$df_limpio = drop($df, "id")
print(keys($df_limpio))      # ["nombre", "ventas", "gastos"]

# Encadenar múltiples drops
$df_final = drop(drop($df, "id"), "timestamp")
```

---

### `drop_nan($df, $columna)` → `df`

Elimina todas las filas donde la columna indicada tiene `null`. Devuelve un DataFrame nuevo.

```zeta
$df = load_csv("ventas.csv")
print(len($df))              # 100

$df_limpio = drop_nan($df, "ventas")
print(len($df_limpio))       # 85 (se eliminaron 15 filas con null)
```

---

### `group_by($df, $col1, [$col2...])` → `dict`

Agrupa un DataFrame por una o más columnas. Retorna una estructura agrupada para usar con `agg`.

```zeta
$ventas = {
    "canal": <"online", "tienda", "online", "tienda">,
    "monto": <100, 200, 150, 250>
}

$grouped = group_by($ventas, "canal")
print($grouped)  # Dict con _type, _data, _group_cols, _groups
```

**Múltiples columnas de agrupación:**

```zeta
$grouped = group_by($ventas, "canal", "region")
```

---

### `agg($grouped, $col, $func)` → `DataFrame`

Agrega una columna de un DataFrame agrupado. Funciones soportadas: `sum`, `mean`, `count`, `min`, `max`, `stddev`, `median`.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$grouped` | `dict` | Sí | Estructura de group_by |
| `$col` | `str` | Sí | Columna a agregar |
| `$func` | `str` | Sí | Función de agregación |

```zeta
$grouped = group_by($ventas, "canal")
$result = agg($grouped, "monto", "sum")
print($result)
# canal  | monto_sum
# online | 250
# tienda | 450
```

---

### `merge($df1, $df2, $on)` → `DataFrame`

Inner join de dos DataFrames por una columna clave.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$df1` | `df` | Sí | Primer DataFrame |
| `$df2` | `df` | Sí | Segundo DataFrame |
| `$on` | `str` | Sí | Nombre de columna clave |

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

---

## 5.8 Exploración de Datos

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `info` | `info($df)` | `string` | Resumen de DataFrame (columnas, tipos, nulls) |
| `describe` | `describe($df)` | `string` | Estadísticas descriptivas |
| `tail` | `tail($x, [$n])` | `df` o `vec` | Últimas N filas/elementos |
| `sample` | `sample($df, [$n])` | `df` | Muestreo aleatorio |
| `value_counts` | `value_counts($col)` | `DataFrame` | Conteo de valores únicos |
| `nunique` | `nunique($vec)` | `num` | Número de valores únicos |
| `isna` | `isna($df/vec)` | `DataFrame` o `bool_vec` | Detectar nulos |
| `duplicated` | `duplicated($vec)` | `bool_vec` | Detectar duplicados |
| `cut` | `cut($vec, $bins)` | `vec` | Discretizar en bins |
| `qcut` | `qcut($vec, $q)` | `vec` | Discretizar en quantiles |

---

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

---

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

---

### `tail($df_o_vec, $n=5)` → `DataFrame` o `vector`

Retorna las últimas N filas o elementos. Default: 5.

```zeta
print(tail($df, 5))        # Últimas 5 filas
print(tail($vec, 3))       # Últimos 3 elementos
print(tail($str_vec, 2))   # Últimos 2 strings
```

---

### `sample($df, $n=5)` → `DataFrame`

Retorna una muestra aleatoria de N filas (sin reemplazo).

```zeta
sample($df, 10)     # 10 filas aleatorias
sample($df, 100)    # 100 filas aleatorias
```

---

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

---

### `nunique($vec)` → `num`

Cantidad de valores únicos (ignora nulls).

```zeta
print(nunique(<1, 2, 2, 3, 3, 3>))    # 3
print(nunique($df:canal))              # 5
```

---

### `isna($df | $vec)` → `DataFrame` o `vector<bool>`

Alias de `is_null` para DataFrames. Retorna DataFrame o vector booleano indicando posición de nulls.

```zeta
isna($df)              # DataFrame de bools (true donde hay null)
isna(<1, null, 3>)    # <false, true, false>
```

---

### `duplicated($vec)` → `vector<bool>`

Retorna vector booleano indicando duplicados (true en la segunda aparición en adelante).

```zeta
print(duplicated(<1, 2, 1, 3>))        # <false, false, true, false>
print(duplicated(<"a", "b", "a">))     # <false, false, true>
```

---

### `cut($vec, $bins)` → `vector`

Discretiza un vector numérico en N bins de igual ancho. Retorna vector 1-indexed.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$vec` | `vec` | Sí | Vector numérico |
| `$bins` | `num` | Sí | Número de bins |

```zeta
print(cut(<1,2,3,4,5>, 3))         # <1, 1, 2, 2, 3>
print(cut(<10,20,30,40,50>, 2))    # <1, 1, 2, 2, 2>
```

---

### `qcut($vec, $q)` → `vector`

Discretiza un vector numérico en `$q` cuantiles. Retorna vector 1-indexed.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$vec` | `vec` | Sí | Vector numérico |
| `$q` | `num` | Sí | Número de cuantiles |

```zeta
print(qcut(<1,2,3,4,5>, 4))              # <1, 1, 2, 3, 4>
print(qcut(<1,2,3,4,5,6,7,8>, 4))       # <1, 1, 2, 2, 3, 3, 4, 4>
```

---

## 5.9 Distribuciones Estadísticas

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `dnorm` | `dnorm($x, [$mean], [$sd])` | `num` | Densidad normal |
| `pnorm` | `pnorm($x, [$mean], [$sd])` | `num` | CDF normal |
| `qnorm` | `qnorm($p, [$mean], [$sd])` | `num` | Cuantil normal |
| `dgamma` | `dgamma($x, $shape, $rate)` | `num` | Densidad gamma |
| `dbeta` | `dbeta($x, $alpha, $beta)` | `num` | Densidad beta |
| `dunif` | `dunif($x, [$min], [$max])` | `num` | Densidad uniforme |
| `dt_dist` | `dt_dist($x, $df)` | `num` | Densidad t-Student |
| `df_dist` | `df_dist($x, $df1, $df2)` | `num` | Densidad F |
| `dchisq` | `dchisq($x, $df)` | `num` | Densidad chi-cuadrado |

---

### `dnorm($x, [$mean], [$sd])` → `num`

Densidad de probabilidad de la distribución normal.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$x` | `num` | Sí | Valor |
| `$mean` | `num` | No | Media (default: 0) |
| `$sd` | `num` | No | Desviación estándar (default: 1) |

```zeta
print(dnorm(0))          # 0.3989 (normal estándar)
print(dnorm(1.96))       # 0.0584
print(dnorm(0, 0, 1))    # 0.3989 (misma que arriba)
print(dnorm(5, 10, 2))   # 0.0883 (normal media=10, sd=2)
```

---

### `pnorm($x, [$mean], [$sd])` → `num`

Función de distribución acumulada (CDF) de la normal.

```zeta
print(pnorm(0))          # 0.5
print(pnorm(1.96))       # 0.975
print(pnorm(-1.96))      # 0.025
```

---

### `qnorm($p, [$mean], [$sd])` → `num`

Quantil (inversa de la CDF) de la normal.

```zeta
print(qnorm(0.5))        # 0.0
print(qnorm(0.975))      # 1.96
print(qnorm(0.025))      # -1.96
```

---

### `dgamma($x, $shape, $rate)` → `num`

Densidad de probabilidad de la distribución gamma.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$x` | `num` | Sí | Valor |
| `$shape` | `num` | Sí | Parámetro de forma |
| `$rate` | `num` | Sí | Parámetro de tasa |

```zeta
print(dgamma(1, 2, 1))    # 0.368
print(dgamma(2, 3, 1))    # 0.271
```

---

### `dbeta($x, $alpha, $beta)` → `num`

Densidad de probabilidad de la distribución beta.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$x` | `num` | Sí | Valor (0-1) |
| `$alpha` | `num` | Sí | Parámetro alpha |
| `$beta` | `num` | Sí | Parámetro beta |

```zeta
print(dbeta(0.5, 2, 2))    # 1.5
print(dbeta(0.3, 1, 5))    # 2.62
```

---

### `dunif($x, [$min], [$max])` → `num`

Densidad de probabilidad de la distribución uniforme.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$x` | `num` | Sí | Valor |
| `$min` | `num` | No | Límite inferior (default: 0) |
| `$max` | `num` | No | Límite superior (default: 1) |

```zeta
print(dunif(0.5))        # 1.0 (en [0,1])
print(dunif(1.5))        # 0.0 (fuera de [0,1])
print(dunif(3, 0, 10))   # 0.1
```

---

### `dt_dist($x, $df)` → `num`

Densidad de probabilidad de la distribución t de Student.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$x` | `num` | Sí | Valor |
| `$df` | `num` | Sí | Grados de libertad |

```zeta
print(dt_dist(0, 10))    # 0.389
print(dt_dist(2, 5))     # 0.068
```

---

### `df_dist($x, $df1, $df2)` → `num`

Densidad de probabilidad de la distribución F.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$x` | `num` | Sí | Valor |
| `$df1` | `num` | Sí | Grados de libertad numerador |
| `$df2` | `num` | Sí | Grados de libertad denominador |

```zeta
print(df_dist(1, 5, 10))     # 0.000013
print(df_dist(2, 10, 20))    # 0.0013
```

---

### `dchisq($x, $df)` → `num`

Densidad de probabilidad de la distribución chi-cuadrado.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$x` | `num` | Sí | Valor |
| `$df` | `num` | Sí | Grados de libertad |

```zeta
print(dchisq(2, 3))    # 2.46
print(dchisq(5, 4))    # 0.176
```

---

## 5.10 Testing Estadístico y Regresión

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `t_test` | `t_test($vec1, $vec2)` | `dict` | T-test muestras independientes |
| `anova` | `anova($vecs)` | `dict` | ANOVA una vía |
| `chi_square` | `chi_square($obs, $exp)` | `dict` | Prueba chi-cuadrado |
| `linear_regression` | `linear_regression($x, $y)` | `dict` | Regresión lineal simple |

---

### `t_test($sample1, $sample2)` → `dict`

Test t de Student para dos muestras independientes.

**Retorna:**

| Clave | Tipo | Descripción |
|-------|------|-------------|
| `t_statistic` | `num` | Estadístico t |
| `df` | `num` | Grados de libertad |
| `p_value` | `num` | p-valor |
| `significant` | `num` | 1 si p < 0.05 |

```zeta
$a = <10, 12, 14, 16, 18>
$b = <20, 22, 24, 26, 28>
$result = t_test($a, $b)
print($result:t_statistic)    # -5.0
print($result:p_value)        # 0.001
print($result:significant)    # 1 (rechaza H0)
```

---

### `anova($group1, $group2, [...])` → `dict`

Análisis de varianza de una vía (one-way ANOVA). Acepta 2 o más grupos.

**Retorna:**

| Clave | Tipo | Descripción |
|-------|------|-------------|
| `f_statistic` | `num` | Estadístico F |
| `groups` | `num` | Número de grupos |

```zeta
$g1 = <10, 12, 14>
$g2 = <20, 22, 24>
$g3 = <30, 32, 34>
$result = anova($g1, $g2, $g3)
print($result:f_statistic)    # 75.0
```

---

### `chi_square($observed, $expected)` → `dict`

Test de chi-cuadrado de bondad de ajuste.

**Retorna:**

| Clave | Tipo | Descripción |
|-------|------|-------------|
| `chi_square` | `num` | Estadístico chi-cuadrado |
| `df` | `num` | Grados de libertad |

```zeta
$obs = <50, 30, 20>
$exp = <40, 40, 20>
$result = chi_square($obs, $exp)
print($result:chi_square)    # 5.0
print($result:df)            # 2
```

---

### `linear_regression($x, $y)` → `dict`

Regresión lineal simple.

**Retorna:**

| Clave | Tipo | Descripción |
|-------|------|-------------|
| `slope` | `num` | Pendiente |
| `intercept` | `num` | Intercepto |
| `r_squared` | `num` | Coeficiente de determinación |
| `std_error` | `num` | Error estándar |
| `predicted` | `vec` | Valores predichos |
| `formula` | `str` | Fórmula legible |

```zeta
$x = <1, 2, 3, 4, 5>
$y = <2, 4, 5, 4, 5>
$result = linear_regression($x, $y)
print($result:slope)        # 0.6
print($result:intercept)    # 2.2
print($result:r_squared)    # 0.6
print($result:formula)      # "y = 0.6 * x + 2.2"
print($result:predicted)    # <2.8, 3.4, 4.0, 4.6, 5.2>
```

---

## 5.11 Window Functions

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `cumsum` | `cumsum($vec)` | `vec` | Suma acumulada |
| `cummax` | `cummax($vec)` | `vec` | Máximo acumulado |
| `cummin` | `cummin($vec)` | `vec` | Mínimo acumulado |
| `rolling_mean` | `rolling_mean($vec, $window)` | `vec` | Media móvil |
| `rolling_std` | `rolling_std($vec, $window)` | `vec` | Desviación estándar móvil |
| `rolling_sum` | `rolling_sum($vec, $window)` | `vec` | Suma móvil |
| `rolling_min` | `rolling_min($vec, $window)` | `vec` | Mínimo móvil |
| `rolling_max` | `rolling_max($vec, $window)` | `vec` | Máximo móvil |
| `lag` | `lag($vec, $n)` | `vec` | Desplazar hacia atrás |
| `lead` | `lead($vec, $n)` | `vec` | Desplazar hacia adelante |
| `diff` | `diff($vec, $n)` | `vec` | Diferencia |
| `row_number` | `row_number($vec)` | `vec` | Número de fila secuencial |
| `rank` | `rank($vec)` | `vec` | Ranking con manejo de empates |
| `pct_change` | `pct_change($vec, $n)` | `vec` | Cambio porcentual |

---

### `cumsum($vec)` → `vector`

Suma acumulada.

```zeta
print(cumsum(<10, 20, 30, 40>))    # <10, 30, 60, 100>
```

---

### `cummax($vec)` → `vector`

Máximo acumulado.

```zeta
print(cummax(<10, 30, 20, 40>))    # <10, 30, 30, 40>
```

---

### `cummin($vec)` → `vector`

Mínimo acumulado.

```zeta
print(cummin(<40, 20, 30, 10>))    # <40, 20, 20, 10>
```

---

### `rolling_mean($vec, $window)` → `vector`

Promedio móvil con ventana deslizante.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$vec` | `vec` | Sí | Vector de entrada |
| `$window` | `num` | Sí | Tamaño de la ventana |

```zeta
$ventas = <100, 150, 120, 180, 200>
print(rolling_mean($ventas, 3))
# <100, 125, 123.3, 150, 166.7>
```

**Notas:** Los primeros `$window - 1` elementos pueden ser `null` si no hay suficientes datos para la ventana completa.

---

### `rolling_std($vec, $window)` → `vector`

Desviación estándar móvil.

```zeta
print(rolling_std($ventas, 3))    # [<null>, 25.0, 20.5, 24.5, 34.0]
```

---

### `rolling_sum($vec, $window)` → `vector`

Suma móvil.

```zeta
print(rolling_sum($ventas, 3))    # [100, 250, 370, 450, 500]
```

---

### `rolling_min($vec, $window)` → `vector`

Mínimo móvil.

```zeta
print(rolling_min($ventas, 3))    # [100, 100, 100, 120, 120]
```

---

### `rolling_max($vec, $window)` → `vector`

Máximo móvil.

```zeta
print(rolling_max($ventas, 3))    # [100, 150, 150, 180, 200]
```

---

### `lag($vec, $n)` → `vector`

Valor de hace `$n` filas (shift hacia atrás). Rellena con `null` al inicio.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$vec` | `vec` | Sí | Vector de entrada |
| `$n` | `num` | Sí | Posiciones a desplazar |

```zeta
print(lag(<10, 20, 30, 40>, 1))    # <null, 10, 20, 30>
print(lag(<10, 20, 30, 40>, 2))    # <null, null, 10, 20>
```

---

### `lead($vec, $n)` → `vector`

Valor dentro de `$n` filas (shift hacia adelante). Rellena con `null` al final.

```zeta
print(lead(<10, 20, 30, 40>, 1))    # <20, 30, 40, null>
print(lead(<10, 20, 30, 40>, 2))    # <30, 40, null, null>
```

---

### `diff($vec, $n)` → `vector`

Diferencia con valor anterior (`$n` filas atrás). Rellena con `null` al inicio.

```zeta
print(diff(<100, 150, 120, 180>, 1))    # <null, 50, -30, 60>
print(diff(<100, 150, 120, 180>, 2))    # <null, null, 20, 30>
```

---

### `row_number($vec)` → `vector`

Número de fila secuencial (1, 2, 3...). Útil para crear índices.

```zeta
print(row_number(<10, 20, 30>))    # <1, 2, 3>
```

---

### `rank($vec)` → `vector`

Ranking con manejo de empates (promedio de rangos).

```zeta
print(rank(<85, 92, 78, 92, 88>))    # <2, 4.5, 1, 4.5, 3>
# 78 es el menor → rank 1
# 85 es segundo → rank 2
# 88 es tercero → rank 3
# 92 y 92 empatan → ranks 4 y 5, promedio = 4.5
```

---

### `pct_change($vec, $n)` → `vector`

Cambio porcentual vs valor anterior (`$n` filas atrás). Rellena con `null` al inicio.

```zeta
print(pct_change(<100, 150, 120, 180>, 1))
# <null, 50%, -20%, 50%>
# (150-100)/100 = 50%
# (120-150)/150 = -20%
```

---

## 5.12 Limpieza de Datos

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `drop_duplicates` | `drop_duplicates($df)` | `df` | Eliminar filas duplicadas |
| `rename` | `rename($df, $old, $new)` | `df` | Renombrar columna |
| `select_cols` | `select_cols($df, <$cols>)` | `df` | Seleccionar múltiples columnas |
| `drop_cols` | `drop_cols($df, <$cols>)` | `df` | Eliminar múltiples columnas |
| `fillna` | `fillna($vec, $strategy)` | `vec` | Rellenar nulos |
| `replace_val` | `replace_val($df, $col, $old, $new)` | `df` | Reemplazar valores |
| `clip` | `clip($vec, $min, $max)` | `vec` | Limitar outliers |
| `trim` | `trim($vec, $std)` | `vec` | Eliminar outliers por std dev |
| `normalize` | `normalize($vec)` | `vec` | Normalizar 0-1 |
| `standardize` | `standardize($vec)` | `vec` | Estandarizar z-score |
| `fill_null` | `fill_null($vec, $default)` | `vec` | Reemplazar nulls con valor |

---

### `drop_duplicates($df)` → `df`

Elimina filas duplicadas del DataFrame (basándose en todas las columnas).

```zeta
$df = load_csv("datos.csv")
print(len($df))                    # 1000
$df_limpio = drop_duplicates($df)
print(len($df_limpio))             # 850 (150 duplicados eliminados)
```

---

### `rename($df, $old, $new)` → `df`

Renombra una columna del DataFrame.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$df` | `df` | Sí | DataFrame de entrada |
| `$old` | `str` | Sí | Nombre actual de la columna |
| `$new` | `str` | Sí | Nuevo nombre |

```zeta
$df = load_csv("datos.csv")
$df_renamed = rename($df, "old_col", "new_col")
print(keys($df_renamed))
```

---

### `select_cols($df, <$cols>)` → `df`

Selecciona múltiples columnas del DataFrame y devuelve uno nuevo solo con esas.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$df` | `df` | Sí | DataFrame de entrada |
| `<$cols>` | `str_vec` | Sí | Lista de nombres de columnas |

```zeta
$df = load_csv("datos.csv")
$df_subset = select_cols($df, ["nombre", "edad", "salario"])
print(keys($df_subset))    # ["nombre", "edad", "salario"]
```

---

### `drop_cols($df, <$cols>)` → `df`

Elimina múltiples columnas del DataFrame.

```zeta
$df = load_csv("datos.csv")
$df_limpio = drop_cols($df, ["id", "timestamp", "temp"])
print(keys($df_limpio))
```

---

### `fillna($vec, $strategy)` → `vec`

Rellena valores nulos con una estrategia.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$vec` | `vec` | Sí | Vector de entrada |
| `$strategy` | `str` o `num` | Sí | Estrategia de relleno |

**Estrategias soportadas:**

| Estrategia | Descripción |
|------------|-------------|
| `"mean"` | Rellena con la media |
| `"median"` | Rellena con la mediana |
| `"mode"` | Rellena con la moda |
| `"ffill"` | Forward fill (valor anterior) |
| `"bfill"` | Backward fill (valor siguiente) |
| `"zero"` | Rellena con 0 |
| `$literal` | Rellena con un valor literal específico |

```zeta
$datos = <10, null, 30, null, 50>

# Estrategia numérica literal
$limpio = fillna($datos, 0)
print($limpio)           # <10, 0, 30, 0, 50>

# Estrategia con string
$limpio2 = fillna($datos, "mean")
print($limpio2)          # <10, 30, 30, 30, 50>

# Forward fill
$limpio3 = fillna($datos, "ffill")
print($limpio3)          # <10, 10, 30, 30, 50>
```

---

### `replace_val($df, $col, $old, $new)` → `df`

Reemplaza todos los valores `$old` por `$new` en la columna indicada.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$df` | `df` | Sí | DataFrame de entrada |
| `$col` | `str` | Sí | Nombre de columna |
| `$old` | `str` o `num` | Sí | Valor a reemplazar |
| `$new` | `str` o `num` | Sí | Valor de reemplazo |

```zeta
$df = load_csv("datos.csv")
$df = replace_val($df, "canal", "old_value", "new_value")
```

---

### `clip($vec, $min, $max)` → `vec`

Limita los valores a un rango `[$min, $max]`. Los valores fuera del rango se recortan al límite más cercano.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$vec` | `vec` | Sí | Vector numérico |
| `$min` | `num` | Sí | Límite inferior |
| `$max` | `num` | Sí | Límite superior |

```zeta
print(clip(<1, 5, 10, 15, 20>, 5, 15))    # <5, 5, 10, 15, 15>
print(clip(<-10, 0, 50, 100>, 0, 50))     # <0, 0, 50, 50>
```

---

### `trim($vec, $std)` → `vec`

Elimina outliers que estén más de `$std` desviaciones estándar de la media.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$vec` | `vec` | Sí | Vector numérico |
| `$std` | `num` | Sí | Número de desviaciones estándar |

```zeta
$datos = <10, 12, 11, 13, 100>    # 100 es outlier
print(trim($datos, 2))            # <10, 12, 11, 13> (100 eliminado)
```

---

### `normalize($vec)` → `vec`

Normaliza el vector al rango [0, 1] usando la fórmula `(x - min) / (max - min)`.

```zeta
print(normalize(<10, 20, 30, 40, 50>))    # <0.0, 0.25, 0.5, 0.75, 1.0>
print(normalize(<0, 100>))                # <0.0, 1.0>
```

---

### `standardize($vec)` → `vec`

Estandariza el vector a media=0, desviación estándar=1 usando la fórmula `(x - mean) / stddev`.

```zeta
print(standardize(<10, 20, 30, 40, 50>))
# <−1.414, −0.707, 0.0, 0.707, 1.414>
```

---

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

---

## 5.13 I/O

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `load_csv` | `load_csv($ruta, [$delim])` | `df` | Carga CSV/TSV |
| `load_json` | `load_json($ruta)` | `df` | Carga JSON array de objetos |
| `load_xlsx` | `load_xlsx($ruta)` | `df` | Carga Excel |
| `save_csv` | `save_csv($ruta, $df, [$delim])` | `null` | Guarda CSV |
| `save_xlsx` | `save_xlsx($ruta, $df)` | `null` | Guarda Excel |
| `guardar_grafo` | `guardar_grafo($ruta)` | `null` | Guarda escena como JSON |
| `cargar_grafo` | `cargar_grafo($ruta)` | `scene` | Carga escena desde JSON |
| `grafo_actual` | `grafo_actual()` | `scene` | Obtiene escena actual |

---

### `load_csv($ruta, $delim=",")` → `df`

Carga un CSV/TSV como DataFrame.

**Parámetros:**

| Parámetro | Tipo | Obligatorio | Descripción |
|-----------|------|-------------|-------------|
| `$ruta` | `str` | Sí | Ruta al archivo |
| `$delim` | `str` | No | Delimitador (default: `,`) |

```zeta
$df = load_csv("tests/datos.csv")
$df_tsv = load_csv("datos.tsv", "\t")
```

---

### `load_json($ruta)` → `df`

Carga un JSON (array de objetos) como DataFrame.

```zeta
$df = load_json("personas.json")
print($df:nombre)
```

---

### `load_xlsx($ruta)` → `df`

Carga un archivo Excel como DataFrame.

```zeta
$df = load_xlsx("reporte.xlsx")
```

---

### `save_csv($ruta, $df, $delim=",")`

Guarda un DataFrame como CSV.

```zeta
save_csv("backup.csv", $df)
save_csv("datos.tsv", $df, "\t")
```

---

### `save_xlsx($ruta, $df)`

Guarda un DataFrame como Excel.

```zeta
save_xlsx("reporte.xlsx", $df)
```

---

### `guardar_grafo($ruta)`

Guarda la escena actual como JSON.

```zeta
$scn = scene("Mi Dashboard", "zeta")
add_metric($scn, "Total", 42)
guardar_grafo("escena.json")
```

---

### `cargar_grafo($ruta)` → `scene`

Carga una escena desde un JSON guardado.

```zeta
$scn = cargar_grafo("escena.json")
```

---

### `grafo_actual()` → `scene`

Obtiene la escena actual (la última creada con `scene()`).

```zeta
$scn = scene("Test", "zeta")
add_metric($scn, "X", 10)
$current = grafo_actual()
```

---

## 5.14 Visualización y Reporting

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `plot` | `plot($data, [$tipo], [$titulo])` | `grafico` | Plot ASCII en consola |
| `metric` | `metric($nombre, $valor)` | `metrica` | Registra un KPI |
| `dashboard` | `dashboard($titulo, [$autor])` | `dashboard` | Crea un dashboard |
| `serve` | `serve($dashboard, [$puerto], [$archivo])` | `str` | Genera HTML con dashboard |

---

### `plot($data, [$tipo], [$titulo])` → `grafico`

Imprime un plot ASCII en consola (para debugging). `$tipo` puede ser `"line"`, `"bar"`, `"scatter"`. `$data` puede ser un vector o un DataFrame.

```zeta
plot($datos:ventas, "line", "Tendencia")
plot($datos, "bar", "Resumen")
```

Devuelve un `GRAFICO` que se puede usar con `serve()`.

---

### `metric($nombre, $valor)` → `metrica`

Registra un KPI con nombre y valor. Devuelve un `METRICA` y lo agrega a la lista interna del intérprete (visible vía `/api/metricas`).

```zeta
$total = sum($datos:ventas)
metric("Ventas totales", $total)
metric("Promedio", mean($datos:ventas))
```

---

### `dashboard($titulo, [$autor])` → `dashboard`

Crea un dashboard con título y autor opcional. Se llena con `add_*`.

```zeta
$db = dashboard("Mi Reporte", "zeta")
```

---

### `serve($dashboard, [$puerto], [$archivo])` → `str`

Genera un archivo HTML con el dashboard (Chart.js embebido). **Obsoleto en favor de `zeta_server` + `zeta_dashboard` + `zeta_term`**, pero se mantiene para compatibilidad.

```zeta
serve($db, 8080, "reporte.html")
```

---

## 5.15 Inspección y Tipos

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `type` | `type($x)` | `str` | Nombre del tipo |
| `is_null` | `is_null($x)` | `bool` o `bool_vec` | Verifica si es null |
| `is_error` | `is_error($x)` | `bool` | Verifica si es error |
| `mk_null_val` | `mk_null_val()` | `num` (NaN) | Crea valor null |
| `mk_err` | `mk_err($tipo, $mensaje)` | `err` | Crea valor de error |

---

### `type($x)` → `str`

Nombre del tipo como string.

```zeta
print(type(42))         # "num"
print(type("hola"))     # "str"
print(type(<1, 2>))     # "vec"
print(type(true))       # "bool"
print(type(null))       # "num" (null es un num con NaN)
print(type(load_csv("datos.csv")))   # "df"
```

---

### `is_null($x)` → `bool` o `bool_vec`

Verifica si el valor es null (NaN). Si le pasas un vector, devuelve un `BOOL_VEC` con la verificación elemento a elemento.

```zeta
print(is_null(null))            # true
print(is_null(0))               # false
print(is_null(0 / 0))           # true (división por cero = null)
print(is_null(<1, null, 3>))    # <false, true, false>
```

---

### `is_error($x)` → `bool`

Verifica si el valor es un error.

```zeta
$e = mk_err("test", "mensaje")
print(is_error($e))     # true
print(is_error(42))      # false
```

---

### `mk_null_val()` → `num`

Devuelve un valor `null` (NaN). Útil para crear nulls explícitos en funciones.

```zeta
$x = mk_null_val()
print(is_null($x))    # true

fn safe_div($a, $b) {
    if ($b == 0) {
        return mk_null_val()
    }
    return $a / $b
}

print(safe_div(10, 0))    # null
print(safe_div(10, 2))    # 5
```

---

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

---

## 5.16 Programación Funcional

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `map` | `map($vec, $fn)` | `vec` | Aplica función a cada elemento |
| `filter` | `filter($vec, $fn)` | `vec` | Filtra por condición |
| `reduce` | `reduce($vec, $fn, $inicial)` | `num` | Reduce a un solo valor |

---

### `map($vec, $fn)` → `vec`

Aplica una función a cada elemento. La función recibe un escalar y devuelve un escalar.

```zeta
fn doble($x) { return $x * 2 }
print(map(<1, 2, 3>, doble))    # <2, 4, 6>
```

---

### `filter($vec, $fn)` → `vec`

Filtra elementos que cumplen una condición. La función recibe un escalar y devuelve un bool.

```zeta
fn mayor_a_2($x) { return $x > 2 }
print(filter(<1, 2, 3, 4, 5>, mayor_a_2))    # <3, 4, 5>
```

---

### `reduce($vec, $fn, $inicial)` → `num`

Reduce un vector a un solo valor aplicando una función acumuladora. `$fn` recibe `(acumulador, elemento)` y devuelve el nuevo acumulador.

```zeta
fn sumar($acc, $x) { return $acc + $x }
print(reduce(<1, 2, 3, 4, 5>, sumar, 0))    # 15
```

---

## 5.17 Matrices y Álgebra

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `transpose` | `transpose($matriz)` | `matriz` | Transpone matriz |
| `dot` | `dot($vec1, $vec2)` | `num` | Producto punto |

---

### `transpose($matriz)` → `matriz`

Transpone una matriz (intercambia filas y columnas).

```zeta
$m = <<1, 2, 3>,
      <4, 5, 6>>
print(transpose($m))    # <<1, 4>, <2, 5>, <3, 6>>
```

---

### `dot($vec1, $vec2)` → `num`

Producto punto de dos vectores. Devuelve `null` si tienen longitudes distintas o si hay nulls.

```zeta
print(dot(<1, 2, 3>, <4, 5, 6>))    # 32 (= 1*4 + 2*5 + 3*6)
```

---

## 5.18 Sistema y Errores

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `print` | `print(...)` | `null` | Imprime valores en consola |
| `time` | `time()` | `num` | Timestamp actual (epoch seconds) |
| `load_lib` | `load_lib($ruta, $dict_funciones)` | `str` | Carga librería nativa (.so/.dll) |

---

### `print(...)`

Imprime valores en la consola. Acepta múltiples argumentos separados por comas.

```zeta
print("Hello", "World")         # Hello World
print(42)                        # 42
print(<1, 2, 3>)                 # <1, 2, 3>
print("Valor:", $x, " tipo:", type($x))
```

---

### `time()` → `num`

Devuelve el timestamp actual en segundos (epoch time). Útil para medir tiempos de ejecución.

```zeta
$t0 = time()
# ... operación larga ...
$t1 = time()
print("Tiempo:", $t1 - $t0, "segundos")
```

---

### `load_lib($ruta, $dict_funciones)` → `str`

Carga una librería nativa (`.so`/`.dll`) y registra funciones. Ver [C ABI](./10-c-abi-loadlib.md).

```zeta
load_lib("libtestnative.so", {
    "native_pow": "fn($a, $b) -> $a",
    "native_pi": "fn() -> $x"
})
```

---

## 5.19 Gestión de Memoria (v0.2)

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `clear_arena` | `clear_arena()` | `null` | Resetea arena allocator (O(1)) |
| `arena_bytes` | `arena_bytes()` | `num` | Bytes allocados en arena |
| `zeta_version` | `zeta_version()` | `str` | Versión de Zeta |

---

### `clear_arena()` → `null`

Resetea el arena allocator, liberando todas las allocaciones temporales de golpe. O(1) — no importa cuántos valores haya. Los valores almacenados en variables (via `shared_ptr`) NO se ven afectados.

```zeta
# Útil en scripts largos que cargan muchos datos
$data = load_csv("archivo_grande.csv")
# ... procesamiento ...
clear_arena()  # Libera memoria temporal
```

---

### `arena_bytes()` → `num`

Devuelve el número de bytes actualmente allocados en el arena. Útil para profiling.

```zeta
print("Arena:", arena_bytes(), "bytes")
clear_arena()
print("Arena después de clear:", arena_bytes(), "bytes")
```

---

### `zeta_version()` → `string`

Devuelve la versión actual del lenguaje Zeta.

```zeta
print("Zeta version:", zeta_version())    # "0.2.0"
```

---

## 5.20 Plugins

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `plugin` | `plugin($ruta)` | `bool` | Carga plugin dinámico |
| `plugin_info` | `plugin_info()` | `dict` | Info del plugin cargado |

---

### `plugin($ruta)` → `bool`

Carga un plugin nativo (`.so`/`.dll`) desde la ruta indicada. Retorna `true` si se cargó correctamente, `false` en caso de error.

```zeta
$ok = plugin("mi_plugin.so")
if ($ok) {
    print("Plugin cargado correctamente")
}
```

---

### `plugin_info()` → `dict`

Retorna información sobre el plugin actualmente cargado (nombre, versión, funciones registradas).

```zeta
$info = plugin_info()
print($info)
```

---
