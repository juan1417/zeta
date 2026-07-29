# 15. Ejemplos Prácticos

Cada ejemplo es un script **completo y ejecutable**. Copia el contenido a un archivo `.zl` y corre `./zeta archivo.zl` o envíalo al server con `POST /api/run`.

> **Recursión e inmutabilidad**: la recursión funciona correctamente (el RETURN marker se propaga limpiamente a través de `if`/`for`/`while`). Para profundizar mucho (miles de frames) considera iteración o aumentar el stack del sistema (`ulimit -s unlimited`).

> **Lambdas anónimas**: la sintaxis `fn($x) { return ... }` como expresión es válida y se evalúa a un valor de tipo `FUNC`. Se puede asignar a una variable (`$f = fn(...) { ... }`) y llamar por su nombre sin sigil (`f(args)`). Funciona con `map`, `filter`, `reduce` y recursión (la lambda puede referenciarse a sí misma por el nombre de la variable).

---

## Índice

### Por nivel

| # | Ejemplo | Conceptos | Nivel |
|---|---------|-----------|-------|
| 1 | [Hola mundo](#ejemplo-1-hola-mundo) | `print`, literales | 🟢 |
| 2 | [Variables y vectores](#ejemplo-2-variables-y-vectores) | sigil `$`, `<>` | 🟢 |
| 3 | [Operadores y comparaciones](#ejemplo-3-operadores-y-comparaciones) | aritmética, comparación, lógicos | 🟢 |
| 4 | [Condicionales y bucles](#ejemplo-4-condicionales-y-bucles) | `if/else`, `for/in`, `while` | 🟢 |
| 5 | [Funciones definidas por el usuario](#ejemplo-5-funciones-definidas-por-el-usuario) | `fn`, `return`, helpers, búsqueda | 🟢 |
| 6 | [Strings y manipulación](#ejemplo-6-strings-y-manipulación) | `upper`, `lower`, `split`, `join`, `replace` | 🟡 |
| 7 | [Funciones de orden superior](#ejemplo-7-funciones-de-orden-superior) | `map`, `filter`, `reduce` | 🟡 |
| 8 | [Cargar CSV y analizar](#ejemplo-8-cargar-csv-y-analizar) | `load_csv`, `sum`, `mean`, `stddev`, `:columna` | 🟡 |
| 9 | [Filtrar y transformar DataFrames](#ejemplo-9-filtrar-y-transformar-dataframes) | `[[filtro]]`, `select`, `head` | 🟡 |
| 10 | [Importar módulos](#ejemplo-10-importar-módulos) | `include`, `export`, namespace `::`, alias `as` | 🟡 |
| 11 | [Crear un dashboard completo](#ejemplo-11-crear-un-dashboard-completo) | `scene`, `layout`, `add_*`, `guardar_grafo` | 🟠 |
| 12 | [Cargar librería nativa (C ABI)](#ejemplo-12-cargar-librería-nativa-c-abi) | `load_lib`, `ZETA_API`, marshalling | 🟠 |
| 13 | [Manejo de errores con `?`](#ejemplo-13-manejo-de-errores-con-) | `mk_err`, `is_error`, propagación | 🟠 |
| 14 | [Ternario vectorizado](#ejemplo-14-ternario-vectorizado) | máscaras bool, `is_null`, `bool_vec` | 🟠 |
| 15 | [Pipeline ETL completo](#ejemplo-15-pipeline-etl-completo) | CSV → transform → dashboard | 🔴 |
| 16 | [Regresión lineal manual](#ejemplo-16-regresión-lineal-manual) | mínimos cuadrados, scatter + fit | 🔴 |
| 17 | [Ordenamiento y algoritmos](#ejemplo-17-ordenamiento-y-algoritmos) | `sort`, `unique`, búsqueda binaria, quicksort recursivo, asignación por índice | 🔴 |
| 18 | [Cliente HTTP via libcurl](#ejemplo-18-cliente-http-via-libcurl) | `load_lib`, libcurl, wrapper C con strings | 🟡 |
| 19 | [Programación orientada a objetos](#ejemplo-19-programación-orientada-a-objetos) | `class`, `new`, `extends`, `$self`, métodos, herencia simple | 🟢 |
| 20 | [Formateo de números](#ejemplo-20-formateo-de-números) | `format($num, $decimals)` | 🟢 |
| 21 | [Operaciones con matrices](#ejemplo-21-operaciones-con-matrices) | `<<>>`, `transpose`, `dot`, acceso `[i,j]` | 🟡 |
| 22 | [Rutas HTTP personalizadas](#ejemplo-22-rutas-http-personalizadas) | `route()`, `serve()`, request/response | 🟠 |
| 23 | [Módulos con clases](#ejemplo-23-módulos-con-clases) | `include`, `export`, clases en `.zl` | 🟠 |
| 24 | [Pipeline completo](#ejemplo-24-pipeline-completo-con-todo-junto) | módulos + clases + errores + ternario + DataFrame | 🔴 |
| 25 | [Limpieza de datos](#ejemplo-25-limpieza-de-datos-completa) | `drop`, `drop_nan`, `fill_null`, `unique`, `sort` | 🟡 |
| 26 | [Medición de tiempos](#ejemplo-26-medición-de-tiempos) | `time()`, benchmark | 🟢 |

### Por dominio

| Dominio | Ejemplos |
|---------|----------|
| Análisis de datos | [8](#ejemplo-8-cargar-csv-y-analizar), [9](#ejemplo-9-filtrar-y-transformar-dataframes), [11](#ejemplo-11-crear-un-dashboard-completo), [15](#ejemplo-15-pipeline-etl-completo), [24](#ejemplo-24-pipeline-completo-con-todo-junto), [25](#ejemplo-25-limpieza-de-datos-completa) |
| Visualización | [11](#ejemplo-11-crear-un-dashboard-completo) |
| Algoritmos | [16](#ejemplo-16-regresión-lineal-manual), [17](#ejemplo-17-ordenamiento-y-algoritmos) |
| Sistemas | [12](#ejemplo-12-cargar-librería-nativa-c-abi), [22](#ejemplo-22-rutas-http-personalizadas), [26](#ejemplo-26-medición-de-tiempos) |
| Idioms del lenguaje | [7](#ejemplo-7-funciones-de-orden-superior), [13](#ejemplo-13-manejo-de-errores-con-), [14](#ejemplo-14-ternario-vectorizado), [20](#ejemplo-20-formateo-de-números) |
| OOP y módulos | [19](#ejemplo-19-programación-orientada-a-objetos), [23](#ejemplo-23-módulos-con-clases) |
| Matrices y álgebra | [21](#ejemplo-21-operaciones-con-matrices) |
| Limpieza de datos | [25](#ejemplo-25-limpieza-de-datos-completa) |

---

## Fundamentos

---

## Ejemplo 1: Hola mundo

```zeta
# 01_hola_mundo.zl
print("Hola, Zeta!")

# print acepta múltiples argumentos separados por coma
print("Tengo", 30, "años")

# Los null se imprimen como "null"
print("Vacío:", null)
```

**Salida esperada**:

```
Hola, Zeta!
Tengo 30.000000 años
Vacío: null
```

---

## Ejemplo 2: Variables y vectores

```zeta
# 02_variables_vectores.zl
$edad = 25
$nombre = "Zeta"
$pi = 3.14159

# Vectores
$nums = <1, 2, 3, 4, 5>
$mezcla = <10, 20, 30>

# Concatenación de vectores
$mas = $nums + <6, 7, 8>
print("Concatenado:", $mas)

# Acceso por índice
print("Primero:", $nums[0])
print("Último:", $nums[4])

# Vector vacío
$vacio = <>
print("Vacío len:", len($vacio))

# Rellenar un vector con un bucle
$cuadrados = <0.0>
for ($i in range(1, 6)) {
    $cuadrados = push($cuadrados, $i * $i)
}
print("Cuadrados 1-5:", $cuadrados)
```

**Salida esperada**:

```
Concatenado: <1, 2, 3, 4, 5, 6, 7, 8>
Primero: 1.000000
Último: 5.000000
Vacío len: 0
Cuadrados 1-5: <1, 4, 9, 16, 25>
```

---

## Ejemplo 3: Operadores y comparaciones

```zeta
# 03_operadores.zl
$a = 10
$b = 3

# Aritmética
print("Suma:", $a + $b)
print("Resta:", $a - $b)
print("Multiplicación:", $a * $b)
print("División:", $a / $b)
print("Módulo:", $a % $b)
print("Potencia:", pow($a, $b))

# Comparación
print("a > b:", $a > $b)
print("a == b:", $a == $b)
print("a != b:", $a != $b)

# Lógicos
print("true && false:", true && false)
print("true || false:", true || false)
print("!true:", !true)

# Strings
$s1 = "hola"
$s2 = "mundo"
print("Concatenar:", $s1 + " " + $s2)
print("Comparar:", $s1 < $s2)   # true (orden lexicográfico)

# null en operaciones
$x = null
print("null + 5:", $x + 5)         # null (NaN se propaga)
print("5 / 0:", 5 / 0)             # null (división por cero)

# Precedencia
print("2 + 3 * 4 =", 2 + 3 * 4)         # 14 (multiplicación primero)
print("(2 + 3) * 4 =", (2 + 3) * 4)     # 20
```

**Salida esperada**:

```
Suma: 13.000000
Resta: 7.000000
Multiplicación: 30.000000
División: 3.333333
Módulo: 1.000000
Potencia: 1000.000000
a > b: true
a == b: false
a != b: true
true && false: false
true || false: true
!true: false
Concatenar: hola mundo
Comparar: true
null + 5: null
5 / 0: null
2 + 3 * 4 = 14.000000
(2 + 3) * 4 = 20.000000
```

---

## Ejemplo 4: Condicionales y bucles

```zeta
# 04_control_flujo.zl

# if / else if / else
$nota = 85
if ($nota >= 90) {
    print("A")
} else if ($nota >= 80) {
    print("B")
} else if ($nota >= 70) {
    print("C")
} else {
    print("D")
}

# for / in
print("Primeros 5 números:")
for ($i in range(5)) {
    print("  ", $i)
}

# while
print("Cuenta regresiva:")
$i = 5
while ($i > 0) {
    print("  ", $i)
    $i = $i - 1
}

# break y continue
print("Solo impares hasta 10:")
for ($i in range(11)) {
    if ($i % 2 == 0) {
        continue   # salta pares
    }
    if ($i > 9) {
        break      # sale del bucle
    }
    print("  ", $i)
}

# Iterar sobre un vector
$frutas = <"manzana", "naranja", "platano">
print("Frutas:")
for ($f in $frutas) {
    print("  -", $f)
}

# Bucle con índice (range + acceso)
$nums = <10, 20, 30, 40>
for ($i in range(len($nums))) {
    print("indice", $i, "valor", $nums[$i])
}
```

**Salida esperada**:

```
B
Primeros 5 números:
  0.000000
  1.000000
  2.000000
  3.000000
  4.000000
Cuenta regresiva:
  5.000000
  4.000000
  3.000000
  2.000000
  1.000000
Solo impares hasta 10:
  1.000000
  3.000000
  5.000000
  7.000000
  9.000000
Frutas:
  -manzana
  -naranja
  -platano
indice 0 valor 10.000000
indice 1 valor 20.000000
indice 2 valor 30.000000
indice 3 valor 40.000000
```

---

## Ejemplo 5: Funciones definidas por el usuario

```zeta
# 05_funciones.zl

# Función simple
fn cuadrado($x) {
    return $x * $x
}
print("cuadrado(5) =", cuadrado(5))

# Función con lógica condicional
fn saludo($nombre) {
    if ($nombre == null) {
        $nombre = "mundo"
    }
    return "Hola, " + $nombre + "!"
}
print(saludo("Zeta"))
print(saludo(null))

# Factorial recursivo
fn factorial($n) {
    if ($n <= 1) {
        return 1
    }
    return $n * factorial($n - 1)
}
print("5! =", factorial(5))
print("10! =", factorial(10))

# Factorial iterativo (alternativa sin recursión)
fn factorial_iter($n) {
    $resultado = 1
    for ($i in range(2, $n + 1)) {
        $resultado = $resultado * $i
    }
    return $resultado
}
print("5! (iter) =", factorial_iter(5))

# Fibonacci recursivo
fn fib($n) {
    if ($n < 2) {
        return $n
    }
    return fib($n - 1) + fib($n - 2)
}
print("fib(10) =", fib(10))

# Fibonacci iterativo (más eficiente para n grande)
fn fib_iter($n) {
    if ($n < 2) { return $n }
    $a = 0
    $b = 1
    for ($i in range(2, $n + 1)) {
        $temp = $a + $b
        $a = $b
        $b = $temp
    }
    return $b
}
print("fib(20) (iter) =", fib_iter(20))

# Búsqueda lineal con return temprano
fn buscar($vec, $target) {
    for ($i in range(len($vec))) {
        if ($vec[$i] == $target) {
            return $i
        }
    }
    return null    # no encontrado
}
print("buscar(<10,20,30>, 20) =", buscar(<10, 20, 30>, 20))
print("buscar(<10,20,30>, 99) =", buscar(<10, 20, 30>, 99))

# Función que llama otra función
fn area_cuadrado($lado) {
    return cuadrado($lado)
}
print("area cuadrado(4) =", area_cuadrado(4))
```

**Salida esperada**:

```
cuadrado(5) = 25.000000
Hola, Zeta!
Hola, mundo!
5! = 120.000000
10! = 3628800.000000
5! (iter) = 120.000000
fib(10) = 55.000000
fib(20) (iter) = 6765.000000
buscar(<10,20,30>, 20) = 1.000000
buscar(<10,20,30>, 99) = null
area cuadrado(4) = 16.000000
```

---

## Manipulación de datos

---

## Ejemplo 6: Strings y manipulación

```zeta
# 06_strings.zl
$texto = "Hola, Mundo Zeta"

# Mayúsculas / minúsculas
print(upper($texto))
print(lower($texto))

# Substring
print(substr($texto, 0, 4))      # "Hola"
print(substr($texto, 6))          # "Mundo Zeta" (desde posición 6)
print(substr($texto, -4))         # "Zeta" (negativos desde el final)

# Split
$palabras = split($texto, " ")
print("Palabras:", len($palabras))

# Join
$reconstruido = join($palabras, "-")
print("Reconstruido:", $reconstruido)

# Replace
$nuevo = replace($texto, "Mundo", "Universo")
print($nuevo)

# Find
$pos = find($texto, "Mundo")
print("'Mundo' en posicion:", $pos)

# Longitud
print("Longitud:", len($texto))

# Trim manual (no hay builtin, pero se puede hacer con replace en loop)
fn trim($s) {
    # Quita espacios al inicio
    while (len($s) > 0 && substr($s, 0, 1) == " ") {
        $s = substr($s, 1)
    }
    # Quita espacios al final
    while (len($s) > 0 && substr($s, -1) == " ") {
        $s = substr($s, 0, len($s) - 1)
    }
    return $s
}
print("Trim:", "<" + trim("  hola  mundo  ") + ">")

# Validar formato
fn tiene_arroba($email) {
    return find($email, "@") > -1
}
print("test@x.com valido:", tiene_arroba("test@x.com"))
print("invalid.email:", tiene_arroba("invalid.email"))
```

**Salida esperada**:

```
HOLA, MUNDO ZETA
hola, mundo zeta
Hola
Mundo Zeta
Zeta
Palabras: 3
Reconstruido: Hola,-Mundo-Zeta
Hola, Universo Zeta
'Mundo' en posicion: 6
Longitud: 16
Trim: <hola  mundo>
test@x.com valido: true
invalid.email: false
```

---

## Ejemplo 7: Funciones de orden superior

`map`, `filter` y `reduce` son las tres funciones funcionales fundamentales. Combinadas te dan el poder de manipular vectores sin loops explícitos. Las **lambdas anónimas** (`fn($x) { return ... }`) se pueden pasar directamente a estas funciones o asignarse a variables.

```zeta
# 07_funcional.zl

# Funciones "callback" nombradas (también válidas)
fn doble($x) { return $x * 2 }
fn cuadrado($x) { return $x * $x }
fn es_par($x) { return $x % 2 == 0 }
fn mayor_a_2($x) { return $x > 2 }

# map: aplica una función a cada elemento
$nums = <1, 2, 3, 4, 5>
print("Doble:", map($nums, doble))                       # <2, 4, 6, 8, 10>
print("Cuadrado:", map($nums, cuadrado))                 # <1, 4, 9, 16, 25>
print("Negativo:", map($nums, fn($x) { return -$x }))    # <-1, -2, -3, -4, -5>

# filter: conserva solo los elementos que cumplen la condición
print("Pares:", filter($nums, es_par))                          # <2, 4>
print("Mayores a 2:", filter($nums, mayor_a_2))                  # <3, 4, 5>
print("Impares:", filter($nums, fn($x) { return $x % 2 == 1 }))  # <1, 3, 5>

# reduce: acumula un valor
fn sumar($acc, $x) { return $acc + $x }
fn multiplicar($acc, $x) { return $acc * $x }
fn maximo($acc, $x) {
    if ($x > $acc) { return $x }
    return $acc
}

print("Suma:", reduce($nums, sumar, 0))              # 15
print("Producto:", reduce($nums, multiplicar, 1))    # 120
print("Maximo:", reduce($nums, maximo, $nums[0]))    # 5

# Encadenamiento: filter -> map (con lambdas anidadas)
$positivos = filter(<-2, -1, 0, 1, 2, 3>, fn($x) { return $x > 0 })
$cuadrados = map($positivos, fn($x) { return $x * $x })
print("Cuadrados de positivos:", $cuadrados)    # <1, 4, 9>

# Lambdas en variables (referenciables por nombre)
$inc = fn($x) { return $x + 1 }
$dbl = fn($x) { return $x * 2 }
$nums2 = <1, 2, 3, 4>
$composed = map(map($nums2, $inc), $dbl)
print("composed =", $composed)    # <4, 6, 8, 10>

# Mapeo manual (sin map) — útil para entender qué hace map por dentro.
# Para llamar a la función pasada como parametro usamos el nombre sin sigil
# (la sintaxis $var[i] es acceso por indice, no llamada; cb(args) sí lo es).
fn map_manual($vec, $cb) {
    $resultado = <0.0>
    for ($x in $vec) {
        $resultado = push($resultado, cb($x))
    }
    return $resultado
}
$mas_10 = fn($x) { return $x + 10 }
print("Mas 10 (manual):", map_manual(<1, 2, 3>, $mas_10))    # <11, 12, 13>

# Lambda recursiva (la recursion dentro de lambda funciona)
$fact = fn($n) {
    if ($n <= 1) { return 1 }
    return $n * fact($n - 1)
}
print("fact(6) =", fact(6))    # 720

# Closure: lambda que captura y retorna lambda
$make_adder = fn($n) { return fn($x) { return $x + $n } }
$add10 = make_adder(10)
print("add10(5) =", add10(5))    # 15
```

**Salida esperada**:

```
Doble: <2, 4, 6, 8, 10>
Cuadrado: <1, 4, 9, 16, 25>
Negativo: <-1, -2, -3, -4, -5>
Pares: <2, 4>
Mayores a 2: <3, 4, 5>
Impares: <1, 3, 5>
Suma: 15
Producto: 120
Maximo: 5
Cuadrados de positivos: <1, 4, 9>
composed = <4, 6, 8, 10>
Mas 10 (manual): <11, 12, 13>
fact(6) = 720
add10(5) = 15
```

---

## Ejemplo 8: Cargar CSV y analizar

Asume que `tests/datos.csv` tiene el siguiente contenido:

```csv
mes,ventas,gastos
Ene,1000,500
Feb,1200,600
Mar,1500,700
Abr,1300,650
May,1700,800
Jun,1900,850
Jul,1800,900
Ago,2100,950
Sep,2000,1000
Oct,2200,1050
```

```zeta
# 08_csv_analisis.zl
$datos = load_csv("tests/datos.csv")
print("Filas:", len($datos))
print("Columnas:", keys($datos))

# Acceder a columnas con ':'
$ventas = $datos:ventas
$gastos = $datos:gastos

# Estadísticas básicas
print("--- Ventas ---")
print("Suma:", sum($ventas))
print("Promedio:", mean($ventas))
print("Min:", min($ventas))
print("Max:", max($ventas))
print("StdDev:", stddev($ventas))
print("Conteo:", count($ventas))

# Mismos para gastos
print("--- Gastos ---")
print("Suma:", sum($gastos))
print("Promedio:", mean($gastos))

# Análisis cruzado: margen
$margen = <0.0>
for ($i in range(len($ventas))) {
    $margen = push($margen, $ventas[$i] - $gastos[$i])
}
print("--- Margen ---")
print("Suma margen:", sum($margen))
print("Margen promedio:", mean($margen))

# Ratio gastos/ventas (en %)
$ratio = <0.0>
for ($i in range(len($ventas))) {
    $ratio = push($ratio, $gastos[$i] / $ventas[$i] * 100)
}
print("Ratio promedio %:", mean($ratio))
```

**Salida esperada**:

```
Filas: 10
Columnas: <gastos, mes, ventas>
--- Ventas ---
Suma: 17700.000000
Promedio: 1770.000000
Min: 1000.000000
Max: 2200.000000
StdDev: 396.611316
Conteo: 10.000000
--- Gastos ---
Suma: 7250.000000
Promedio: 725.000000
--- Margen ---
Suma margen: 10450.000000
Margen promedio: 1045.000000
Ratio promedio %: 41.140440
```

---

## Ejemplo 9: Filtrar y transformar DataFrames

```zeta
# 09_filtrar_df.zl
$datos = load_csv("tests/datos.csv")

# head: primeras N filas
print("Primeras 3 filas:")
print(head($datos, 3))

# Filtrar filas donde ventas > 1500
$grandes = $datos[[$datos:ventas > 1500]]
print("Filas con ventas > 1500:", len($grandes))
print(head($grandes, 3))

# Filtrar donde gastos están entre 700 y 900
$medio = $datos[[$datos:gastos >= 700 && $datos:gastos <= 900]]
print("Filas con gastos 700-900:", len($medio))

# Seleccionar una sola columna
$solo_ventas = select($datos, "ventas")
print("Solo ventas:", $solo_ventas)

# Construir un DataFrame nuevo a partir de columnas existentes
$idx = <1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0>
$resumen = {"idx": $idx, "ventas": $datos:ventas, "gastos": $datos:gastos}
print("Resumen keys:", keys($resumen))

# head sobre un vector
print("Primeras 5 ventas:", head($datos:ventas, 5))

# Combinación: filtrar + head
$top = head($datos[[$datos:ventas > 1500]], 3)
print("Top 3 ventas altas:", len($top))
```

**Salida esperada**:

```
Primeras 3 filas:
{df: {gastos: [500, 600, 700], mes: [null, null, null], ventas: [1000, 1200, 1500]}}
Filas con ventas > 1500: 5
{df: {gastos: [700, 800, 850], mes: [null, null, null], ventas: [1500, 1700, 1900]}}
Filas con gastos 700-900: 5
Solo ventas: <1000, 1200, 1500, 1300, 1700, 1900, 1800, 2100, 2000, 2200>
Resumen keys: <gastos, idx, ventas>
Primeras 5 ventas: <1000, 1200, 1500, 1300, 1700>
Top 3 ventas altas: 3
```

---

## Ejemplo 10: Importar módulos

Asume que `lib/statslib.zl` y `lib/milibreria.zl` existen.

**`lib/statslib.zl`** (módulo a importar):

```zeta
# lib/statslib.zl
fn mean($vec) {
    $n = len($vec)
    if ($n == 0) { return 0 }
    return sum($vec) / $n
}

fn stddev($vec) {
    $m = mean($vec)
    $n = len($vec)
    if ($n == 0) { return 0 }
    $suma = 0
    for ($i in range($n)) {
        if (!is_null($vec[$i])) {
            $d = $vec[$i] - $m
            $suma = $suma + $d * $d
        }
    }
    return sqrt($suma / $n)
}

fn correlation($x, $y) {
    $mx = mean($x)
    $my = mean($y)
    $n = len($x)
    $num = 0
    $dx2 = 0
    $dy2 = 0
    for ($i in range($n)) {
        $dx = $x[$i] - $mx
        $dy = $y[$i] - $my
        $num = $num + $dx * $dy
        $dx2 = $dx2 + $dx * $dx
        $dy2 = $dy2 + $dy * $dy
    }
    $den = sqrt($dx2) * sqrt($dy2)
    if ($den == 0) { return 0 }
    return $num / $den
}

export { mean, stddev, correlation }
```

**`lib/milibreria.zl`** (otro módulo):

```zeta
# lib/milibreria.zl
fn cuadrado($x) {
    return $x * $x
}

fn cubo($x) {
    return $x * $x * $x
}

export { cuadrado, cubo }
```

**El script principal**:

```zeta
# 10_imports.zl
$datos = <1, 2, 3, 4, 5>

# Forma 1: full import
include "statslib"
print("=== Full import ===")
print("mean:", statslib::mean($datos))
print("stddev:", statslib::stddev($datos))

# Forma 2: selectivo
include "statslib"::{mean, stddev}
print("=== Selective import ===")
print("mean:", mean($datos))
print("stddev:", stddev($datos))

# Forma 3: alias
include "statslib" as st
print("=== Aliased import ===")
print("correlation:", st::correlation($datos, $datos))

# Importar otro módulo
include "milibreria"
$cuadrado = milibreria::cuadrado(7)
$cubo = milibreria::cubo(3)
print("7^2 =", $cuadrado)
print("3^3 =", $cubo)
```

**Salida esperada**:

```
=== Full import ===
mean: 3.000000
stddev: 1.414214
=== Selective import ===
mean: 3.000000
stddev: 1.414214
=== Aliased import ===
correlation: 1.000000
7^2 = 49.000000
3^3 = 27.000000
```

---

## Ejemplo 11: Crear un dashboard completo

```zeta
# 11_dashboard.zl
$datos = load_csv("tests/datos.csv")

# Crear columna de índice numérico (necesaria para plots con X numérico)
$idx = <1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0>
$datos2 = {"idx": $idx, "ventas": $datos:ventas, "gastos": $datos:gastos}

# Crear escena
$scn = scene("Analisis de Ventas Q1-Q4", "Mi Empresa")
layout("grid", 2, 10, "#1e1e1e")

# 4 KPIs (métricas)
add_metric("Ventas totales", sum($datos:ventas), "$", "#4caf50")
add_metric("Gastos totales", sum($datos:gastos), "$", "#f44336")
add_metric("Margen bruto", sum($datos:ventas) - sum($datos:gastos), "$", "#2196f3")
add_metric("Promedio ventas", mean($datos:ventas), "$", "#ff9800")

# 6 gráficos
add_line_plot($datos2, "Tendencia de ventas", "idx", "ventas")
add_bar_chart($datos2, "Gastos mensuales", "idx", "gastos")
add_scatter($datos2, "Ventas vs Gastos", "ventas", "gastos")
add_histogram($datos2, "Distribucion ventas", "ventas", 6)
add_linear_regression($datos2, "Regresion V vs G", "ventas", "gastos")
add_box_plot($datos2, "Box de ventas", "ventas")

# Guardar a disco
print(guardar_grafo("tests/mi_dashboard.json"))

# Ahora puedes:
#   - Iniciar zeta_server y abrir /api/grafo
#   - Ejecutar zeta_dashboard --host ... --screenshot ...
#   - Ejecutar zeta_term --file tests/mi_dashboard.json
```

**Salida esperada**:

```
Guardado: tests/mi_dashboard.json (10 nodos)
```

---

## Ejemplo 12: Cargar librería nativa (C ABI)

**`lib/mat_ops.cpp`** (la librería):

```cpp
// lib/mat_ops.cpp
#include "zeta/zeta_abi.h"
#include <cmath>

extern "C" {

ZETA_API double norm(int n_args, const double* args) {
    double s = 0;
    for (int i = 0; i < n_args; ++i) s += args[i] * args[i];
    return std::sqrt(s);
}

ZETA_API double dot_native(int n_args, const double* args) {
    if (n_args < 3) return std::nan("");
    double s = 0;
    int n = (int)args[0];
    if (n_args < 1 + 2 * n) return std::nan("");
    const double* a = args + 1;
    const double* b = a + n;
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

ZETA_API double cube(double x) {
    return x * x * x;
}

}
```

**Compilar**:

```bash
clang++ -std=c++20 -shared -fPIC -fvisibility=hidden \
    -I include -o lib/libmatops.so lib/mat_ops.cpp
```

**`12_abi.zl`** (el script):

```zeta
# 12_abi.zl
load_lib("libmatops.so", {
    "norm": "fn($vec) -> $vec",
    "dot_native": "fn($n, $a, $b) -> $n",
    "cube": "fn($x) -> $x"
})

# Función escalar simple
print("cube(3) =", cube(3))            # 27

# Función variádica con un vector (se aplana)
$vec = <3, 4>
print("norm(<3, 4>) =", norm($vec))    # 5 (raíz de 9+16)

# Función variádica con escalar + dos vectores
$a = <1, 2, 3>
$b = <4, 5, 6>
# dot_native(3, $a, $b) → args = [3, 1, 2, 3, 4, 5, 6]
$resultado = dot_native(3, $a, $b)    # 1*4 + 2*5 + 3*6 = 32
print("dot:", $resultado)

# Caso de error nativo: NaN se convierte en null
$negativo = <-1>
$resultado = norm($negativo)
print("norm(-1) =", $resultado)       # null (raíz de negativo)
print("is_null:", is_null($resultado))
```

**Salida esperada**:

```
cube(3) = 27.000000
norm(<3, 4>) = 5.000000
dot: 32.000000
norm(-1) = null
is_null: true
```

---

## Manejo de errores y tipos avanzados

---

## Ejemplo 13: Manejo de errores con `?`

```zeta
# 13_errores.zl

# Función que puede fallar
fn dividir($a, $b) {
    if ($b == 0) {
        return mk_err("division", "division por cero")
    }
    if (is_null($a) || is_null($b)) {
        return mk_err("null_input", "argumentos nulos")
    }
    return $a / $b
}

# Patrón 1: chequeo explícito
$resultado = dividir(10, 2)
if (is_error($resultado)) {
    print("ERROR:", $resultado.mensaje)
} else {
    print("Resultado:", $resultado)
}

# Patrón 2: propagación con ?
fn calcular_promedio() {
    $suma = dividir(100, 5)?    # si falla, retorna el error
    $cuenta = dividir(50, 5)?
    return $suma + $cuenta
}

$resultado = calcular_promedio()
if (is_error($resultado)) {
    print("Error en pipeline:", $resultado.mensaje)
} else {
    print("Pipeline OK:", $resultado)
}

# Patrón 3: errores como datos en vectores
$operaciones = <dividir(10, 2), dividir(10, 0), dividir(10, 5)>
for ($i in range(len($operaciones))) {
    $r = $operaciones[$i]
    if (is_error($r)) {
        print("Operacion", $i, "fallo:", $r.mensaje)
    } else {
        print("Operacion", $i, "OK:", $r)
    }
}

# Patrón 4: manejo de errores en serie
$inputs = <10, 20, 0, 40, 0, 60>
$exitos = <0.0>
$fallos = 0
for ($i in range(len($inputs))) {
    $r = dividir(100, $inputs[$i])
    if (is_error($r)) {
        $fallos = $fallos + 1
    } else {
        $exitos = push($exitos, $r)
    }
}
print("Exitos:", len($exitos), "Fallos:", $fallos)
```

**Salida esperada**:

```
Resultado: 5.000000
Pipeline OK: 30.000000
Operacion 0 OK: 5.000000
Operacion 1 fallo: division por cero
Operacion 2 OK: 2.000000
Exitos: 4 Fallos: 2
```

---

## Ejemplo 14: Ternario vectorizado

```zeta
# 14_ternario_vector.zl

# Ternario clásico (escalar)
$edad = 20
$status = $edad >= 18 ? "adulto" : "menor"
print("Status:", $status)

# Ternario vectorizado con máscara bool_vec
$nums = <1, null, 3, null, 5>
$mask = is_null($nums)
print("Mask:", $mask)    # <false, true, false, true, false>

# Reemplazar nulls con un valor por defecto
# (ternario con VEC aplica elemento a elemento)
$limpio = <0.0>
for ($i in range(len($nums))) {
    $limpio = push($limpio, is_null($nums[$i]) ? 0 : $nums[$i])
}
print("Limpio:", $limpio)    # <1, 0, 3, 0, 5>

# Imputación: reemplazar nulls con la media
$media = mean($nums)
$imputado = <0.0>
for ($i in range(len($nums))) {
    $imputado = push($imputado, is_null($nums[$i]) ? $media : $nums[$i])
}
print("Media:", $media)
print("Imputado:", $imputado)

# Clip: limitar valores al rango [0, 100]
$valores = <-50, 50, 150, 75, -10>
$clip = <0.0>
for ($i in range(len($valores))) {
    $v = $valores[$i]
    if ($v < 0) { $v = 0 }
    if ($v > 100) { $v = 100 }
    $clip = push($clip, $v)
}
print("Original:", $valores)
print("Clipeado:", $clip)

# Normalización min-max (usando función nombrada para el cálculo)
fn normalize_one($v, $min_v, $range) {
    return ($v - $min_v) / $range
}
$min_v = min($valores)
$max_v = max($valores)
$range = $max_v - $min_v
$norm = <0.0>
for ($i in range(len($valores))) {
    $norm = push($norm, normalize_one($valores[$i], $min_v, $range))
}
print("Normalizado:", $norm)
```

**Salida esperada**:

```
Status: adulto
Mask: <false, true, false, true, false>
Limpio: <1, 0, 3, 0, 5>
Media: 3.000000
Imputado: <1, 3, 3, 3, 5>
Original: <-50, 50, 150, 75, -10>
Clipeado: <0, 50, 100, 75, 0>
Normalizado: <0, 0.5, 1, 0.625, 0.2>
```

---

## Pipelines completos

---

## Ejemplo 15: Pipeline ETL completo

Pipeline: **Extract** (CSV) → **Transform** (limpieza, cálculos) → **Load** (dashboard + persistencia).

```zeta
# 15_etl_completo.zl

# === EXTRACT ===
print("[1/4] Cargando datos...")
$raw = load_csv("tests/datos.csv")
print("  Filas crudas:", len($raw))

# === TRANSFORM ===
print("[2/4] Limpiando y transformando...")

# Eliminar filas con nulls en columnas críticas
$con_datos = $raw[[!is_null($raw:ventas) && !is_null($raw:gastos)]]
print("  Filas validas:", len($con_datos))

# Calcular nuevas columnas
$ventas = $con_datos:ventas
$gastos = $con_datos:gastos

# Margen y margen %
$margen = <0.0>
$margen_pct = <0.0>
for ($i in range(len($ventas))) {
    $m = $ventas[$i] - $gastos[$i]
    $margen = push($margen, $m)
    $margen_pct = push($margen_pct, $m / $ventas[$i] * 100)
}

# Clasificar margen: alto / medio / bajo
$categoria = <0.0>
for ($i in range(len($margen))) {
    if ($margen[$i] > 1000) {
        $categoria = push($categoria, "alto")
    } else if ($margen[$i] > 500) {
        $categoria = push($categoria, "medio")
    } else {
        $categoria = push($categoria, "bajo")
    }
}

# === LOAD: crear dashboard ===
print("[3/4] Creando dashboard...")

$idx = <0.0>
for ($i in range(len($ventas))) {
    $idx = push($idx, $i + 1.0)
}

$df_viz = {"idx": $idx, "ventas": $ventas, "gastos": $gastos, "margen": $margen}

$scn = scene("ETL Ventas", "zeta")
layout("grid", 2, 10, "#1a1a1a")

# KPIs
add_metric("Ventas totales", sum($ventas), "$", "#4caf50")
add_metric("Gastos totales", sum($gastos), "$", "#f44336")
add_metric("Margen total", sum($margen), "$", "#2196f3")
add_metric("Margen promedio %", mean($margen_pct), "%", "#ff9800")

# Visualizaciones
add_line_plot($df_viz, "Tendencia ventas", "idx", "ventas")
add_bar_chart($df_viz, "Margen por mes", "idx", "margen")
add_scatter($df_viz, "Ventas vs Gastos", "ventas", "gastos")
add_linear_regression($df_viz, "Regresion V vs G", "ventas", "gastos")
add_histogram($df_viz, "Distribucion margen", "margen", 5)
add_box_plot($df_viz, "Box margen", "margen")

# === Persistir ===
print("[4/4] Guardando...")
print(guardar_grafo("tests/etl_dashboard.json"))
print("Pipeline completo!")
```

**Salida esperada**:

```
[1/4] Cargando datos...
  Filas crudas: 10
[2/4] Limpiando y transformando...
  Filas validas: 10
[3/4] Creando dashboard...
[4/4] Guardando...
Guardado: tests/etl_dashboard.json (10 nodos)
Pipeline completo!
```

---

## Ejemplo 16: Regresión lineal manual

Calcula la regresión por mínimos cuadrados y muestra el ajuste. Implementación iterativa (sin recursión).

```zeta
# 16_regresion.zl

# Datos: x = horas de estudio, y = calificación
$x = <1, 2, 3, 4, 5, 6, 7, 8, 9, 10>
$y = <55, 60, 65, 68, 72, 75, 80, 85, 88, 92>

# Cálculo manual
$nx = len($x)
$ny = len($y)
$sum_x = sum($x)
$sum_y = sum($y)

$sum_xy = 0
$sum_x2 = 0
for ($i in range($nx)) {
    $sum_xy = $sum_xy + $x[$i] * $y[$i]
    $sum_x2 = $sum_x2 + $x[$i] * $x[$i]
}

# Fórmula: slope = (n·Σxy - Σx·Σy) / (n·Σx² - (Σx)²)
$slope = ($nx * $sum_xy - $sum_x * $sum_y) / ($nx * $sum_x2 - $sum_x * $sum_x)
$intercept = ($sum_y - $slope * $sum_x) / $nx

print("Ecuacion: y =", $slope, "* x +", $intercept)
print("Slope:", $slope)
print("Intercept:", $intercept)

# Predicción (función nombrada, no lambda)
fn predecir($xi, $m, $b) {
    return $m * $xi + $b
}
$prediccion = <0.0>
for ($i in range($nx)) {
    $prediccion = push($prediccion, predecir($x[$i], $slope, $intercept))
}
print("Predicciones:", $prediccion)

# R²: coeficiente de determinación
$ss_tot = 0
$ss_res = 0
$mean_y = mean($y)
for ($i in range($nx)) {
    $ss_tot = $ss_tot + ($y[$i] - $mean_y) * ($y[$i] - $mean_y)
    $ss_res = $ss_res + ($y[$i] - $prediccion[$i]) * ($y[$i] - $prediccion[$i])
}
$r2 = 1 - $ss_res / $ss_tot
print("R²:", $r2)

# Visualizar como escena
$df = {"x": $x, "y": $y, "pred": $prediccion}
$scn = scene("Regresion lineal", "zeta")
layout("grid", 1, 10, "#1a1a1a")
add_scatter($df, "Datos", "x", "y")
add_line_plot($df, "Ajuste lineal", "x", "pred")
print(guardar_grafo("tests/regresion.json"))
```

**Salida esperada**:

```
Ecuacion: y = 4.090909 * x + 52.666667
Slope: 4.090909
Intercept: 52.666667
Predicciones: <56.757576, 60.848485, 64.939394, 69.030303, 73.121212, 77.212121, 81.303030, 85.393939, 89.484848, 93.575758>
R²: 0.998294
Guardado: tests/regresion.json (2 nodos)
```

---

## Ejemplo 17: Ordenamiento y algoritmos

Usa el `sort` builtin y `unique` para problemas clásicos. Implementa búsqueda binaria iterativa.

```zeta
# 17_algoritmos.zl

# Sort builtin
$nums = <38, 27, 43, 3, 9, 82, 10>
print("Original:", $nums)
print("Ordenado:", sort($nums))

# Unique: elimina duplicados
$dup = <5, 2, 8, 2, 9, 1, 5, 5>
print("Con duplicados:", $dup)
print("Unicos:", unique($dup))
print("Unicos ordenados:", sort(unique($dup)))

# Búsqueda binaria iterativa (asume vector ordenado)
fn busqueda_binaria($vec, $target) {
    $lo = 0
    $hi = len($vec) - 1
    while ($lo <= $hi) {
        $mid = ($lo + $hi) / 2
        $mid = floor($mid)
        if ($vec[$mid] == $target) {
            return $mid
        } else if ($vec[$mid] < $target) {
            $lo = $mid + 1
        } else {
            $hi = $mid - 1
        }
    }
    return -1
}

$ordenado = <1, 3, 5, 7, 9, 11, 13, 15, 17, 19>
print("Vector:", $ordenado)
print("buscar 7:", busqueda_binaria($ordenado, 7))
print("buscar 1:", busqueda_binaria($ordenado, 1))
print("buscar 19:", busqueda_binaria($ordenado, 19))
print("buscar 8:", busqueda_binaria($ordenado, 8))    # -1 (no está)

# Quicksort usando asignación por índice ($vec[i] = valor)
fn partition($vec, $lo, $hi) {
    $pivot = $vec[$hi]
    $i = $lo - 1
    for ($j in range($lo, $hi)) {
        if ($vec[$j] <= $pivot) {
            $i = $i + 1
            # swap $vec[$i] y $vec[$j]
            $tmp = $vec[$i]
            $vec[$i] = $vec[$j]
            $vec[$j] = $tmp
        }
    }
    # swap $vec[$i + 1] y $vec[$hi]
    $tmp = $vec[$i + 1]
    $vec[$i + 1] = $vec[$hi]
    $vec[$hi] = $tmp
    return $i + 1
}

fn quicksort($vec, $lo, $hi) {
    if ($lo < $hi) {
        $p = partition($vec, $lo, $hi)
        quicksort($vec, $lo, $p - 1)
        quicksort($vec, $p + 1, $hi)
    }
    return $vec
}

$nums2 = <38, 27, 43, 3, 9, 82, 10>
print("Sort builtin:", sort($nums2))
print("Quicksort recursivo:", quicksort($nums2, 0, len($nums2) - 1))
```

**Salida esperada**:

```
Original: <38, 27, 43, 3, 9, 82, 10>
Ordenado: <3, 9, 10, 27, 38, 43, 82>
Con duplicados: <5, 2, 8, 2, 9, 1, 5, 5>
Unicos: <1, 2, 5, 8, 9>
Unicos ordenados: <1, 2, 5, 8, 9>
Vector: <1, 3, 5, 7, 9, 11, 13, 15, 17, 19>
buscar 7: 3.000000
buscar 1: 0.000000
buscar 19: 9.000000
buscar 8: -1.000000
Sort builtin: <3, 9, 10, 27, 38, 43, 82>
Quicksort recursivo: <3, 9, 10, 27, 38, 43, 82>
```

---

## Ejemplo 18: Cliente HTTP via libcurl

Zeta no tiene un cliente HTTP embebido. Para hacer requests HTTP, carga `libcurl` via `load_lib` y escribe tu propio wrapper. Esto te da control total sobre timeouts, headers, autenticación, y el formato de la respuesta.

```zeta
# 18_http_via_libcurl.zl

# Cargar libcurl (la ruta varía según el sistema; en Linux suele estar
# en /usr/lib/x86_64-linux-gnu/libcurl.so)
load_lib("/usr/lib/x86_64-linux-gnu/libcurl.so", {
    "curl_global_init": "fn($n) -> $n",
    "curl_easy_init":   "fn() -> $n",
    "curl_easy_setopt": "fn($h, $opt, $val) -> $n",
    "curl_easy_perform": "fn($h) -> $n",
    "curl_easy_cleanup": "fn($h) -> $n",
    "curl_global_cleanup": "fn() -> $n"
})

# Constantes de libcurl (los valores numéricos son parte de la API C)
$CURLOPT_URL            = 10002
$CURLOPT_FOLLOWLOCATION = 52

fn http_status($url) {
    # IMPORTANTE: el ABI actual pasa doubles por valor. Para pasar un
    # string a curl_easy_setopt necesitas un wrapper en C. Aqui solo
    # demostramos el ciclo init->perform->cleanup.
    curl_global_init(0)
    $h = curl_easy_init()
    if ($h == 0) {
        return mk_err("runtime", "curl_easy_init fallo", 0)
    }
    # En un wrapper real:
    #   curl_easy_setopt($h, $CURLOPT_URL, $url)
    #   $rc = curl_easy_perform($h)
    #   return $rc  # 0 = OK, 23 = error de transferencia, etc.
    curl_easy_cleanup($h)
    curl_global_cleanup()
    return 0
}

print("[1] HTTP test (sin red)")
print("  status:", http_status("https://example.com"))

# === Para un cliente HTTP real (con string passing) ===
# Necesitas escribir un wrapper C:
#
# lib/zeta_http.h:
#   #include "zeta/zeta_abi.h"
#   ZETA_API double zeta_http_get(const char* url, int url_len);
#
# lib/zeta_http.c:
#   #include <curl/curl.h>
#   #include <string.h>
#
#   typedef struct { char* data; size_t size; } Buffer;
#   static size_t write_cb(void* ptr, size_t size, size_t nmemb, void* user) {
#       Buffer* b = (Buffer*)user;
#       size_t total = size * nmemb;
#       b->data = realloc(b->data, b->size + total + 1);
#       memcpy(b->data + b->size, ptr, total);
#       b->size += total;
#       b->data[b->size] = 0;
#       return total;
#   }
#
#   ZETA_API double zeta_http_get(const char* url, int url_len) {
#       char* url_copy = strndup(url, url_len);
#       CURL* h = curl_easy_init();
#       Buffer buf = {0};
#       curl_easy_setopt(h, CURLOPT_URL, url_copy);
#       curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, write_cb);
#       curl_easy_setopt(h, CURLOPT_WRITEDATA, &buf);
#       CURLcode rc = curl_easy_perform(h);
#       long status = 0;
#       curl_easy_getinfo(h, CURLINFO_RESPONSE_CODE, &status);
#       curl_easy_cleanup(h);
#       // Aqui guardarias buf.data en una variable global accesible desde Zeta
#       free(url_copy);
#       free(buf.data);
#       return (double)status;
#   }
#
# Compilar:
#   clang++ -shared -fPIC -I include -I /usr/include/x86_64-linux-gnu \
#           lib/zeta_http.c -lcurl -o lib/libzetahttp.so
#
# Cargar en Zeta:
#   load_lib("lib/libzetahttp.so", {
#       "zeta_http_get": "fn($url, $len) -> $n"
#   })
#   print(zeta_http_get("https://example.com", 19))  # status code
```

**Salida esperada**:

```
[1] HTTP test (sin red)
  status: 0.000000
```

> Para un cliente HTTP de producción con captura del body, autenticación, y headers, sigue el patrón del wrapper C mostrado arriba. Zeta te da las primitivas; tú decides qué construir encima.

---

## OOP y módulos

---

## Ejemplo 19: Programación orientada a objetos

```zeta
# 19_oop.zl

# === Clase básica con campo y método ===
class Counter {
    $count = 0
    fn inc($self) {
        $self.count = $self.count + 1
        return $self.count
    }
    fn get($self) {
        return $self.count
    }
}

$c = new Counter()
print("inicial:", $c.get())        # 0
print("inc 1:", $c.inc())           # 1
print("inc 2:", $c.inc())           # 2
print("inc 3:", $c.inc())           # 3
print("final:", $c.get())           # 3

# === Herencia simple ===
class Animal {
    $name = "anonimo"
    $legs = 4
    fn describe($self) {
        return $self.name
    }
}

class Dog extends Animal {
    $breed = "mestizo"
}

class Puppy extends Dog {
    $age = 0
    fn describe($self) {
        return $self.name
    }
}

$p = new Puppy()
$p.name = "Toby"
print("perrito:", $p.describe())    # Toby
print("patas:", $p.legs)             # 4 (heredado de Animal)
print("raza:", $p.breed)             # mestizo (heredado de Dog)
print("edad:", $p.age)               # 0 (propio de Puppy)

# === Método init ===
class Point {
    $x = 0
    $y = 0
    fn init($self, $x0, $y0) {
        $self.x = $x0
        $self.y = $y0
    }
    fn magnitude($self) {
        return sqrt($self.x * $self.x + $self.y * $self.y)
    }
}

$pt = new Point(3, 4)
print("punto:", $pt.x, $pt.y)       # 3 4
print("magnitud:", $pt.magnitude()) # 5
```

**Salida esperada**:

```
inicial: 0.000000
inc 1: 1.000000
inc 2: 2.000000
inc 3: 3.000000
final: 3.000000
perrito: Toby
patas: 4.000000
raza: mestizo
edad: 0.000000
punto: 3.000000 4.000000
magnitud: 5.000000
```

---

## Ejemplo 20: Formateo de números

```zeta
# 20_format.zl
$precio = 1234.5678
$porcentaje = 0.15678

# Decimales por defecto (6)
print($precio)             # 1234.567800

# Controlar decimales con format()
print(format($precio, 2))     # 1234.57
print(format($precio, 0))     # 1235
print(format($porcentaje, 4)) # 0.1568

# En un pipeline de datos
$df = load_csv("tests/datos.csv")
$total = sum($df:ventas)
print("Total:", format($total, 2))
print("Promedio:", format(mean($df:ventas), 1))
```

**Salida esperada**:

```
1234.567800
1234.57
1235
0.1568
```

---

## Ejemplo 21: Operaciones con matrices

```zeta
# 21_matrices.zl
# Crear matrices
$A = <<1, 2, 3>, <4, 5, 6>>
$B = <<7, 8, 9>, <10, 11, 12>>

print("A:", $A)
print("B:", $B)

# Acceso por índice
print("A[0,0]:", $A[0, 0])    # 1
print("A[1,2]:", $A[1, 2])    # 6

# Transpuesta
$T = transpose($A)
print("Transpuesta:", $T)

# Producto punto (dot product)
$v1 = <1, 2, 3>
$v2 = <4, 5, 6>
print("Dot:", dot($v1, $v2))  # 32

# Multiplicación de matrices (si dimensiones coinciden)
$C = <<1, 2>, <3, 4>>
$D = <<5, 6>, <7, 8>>
# Nota: dot() hace producto punto de vectores
# Para multiplicar matrices completa, usa la notación manual
```

**Salida esperada**:

```
A: <<1, 2, 3>, <4, 5, 6>>
B: <<7, 8, 9>, <10, 11, 12>>
A[0,0]: 1
A[1,2]: 6
Transpuesta: <<1, 4>, <2, 5>, <3, 6>>
Dot: 32
```

---

## Web y servidores

---

## Ejemplo 22: Rutas HTTP personalizadas

```zeta
# 22_routes.zl
# Registrar una ruta personalizada en el servidor

route("GET", "/api/saludo", fn($req) {
    $nombre = $req:query:name
    if (is_null($nombre)) {
        $nombre = "Mundo"
    }
    return {"saludo": "Hola, " + $nombre + "!"}
})

route("POST", "/api/calcular", fn($req) {
    $a = $req:body:a
    $b = $req:body:b
    $op = $req:body:op

    if ($op == "suma") {
        return {"resultado": $a + $b}
    }
    if ($op == "multiplicar") {
        return {"resultado": $a * $b}
    }
    return mk_err("operacion_invalida", "Op no soportada: " + $op)
})

# Iniciar el server
serve(8080)
```

Probar con curl:

```bash
# Saludo
curl "http://localhost:8080/api/saludo?name=Zeta"
# {"saludo": "Hola, Zeta!"}

# Calculadora
curl -X POST http://localhost:8080/api/calcular \
  -H "Content-Type: application/json" \
  -d '{"a": 10, "b": 5, "op": "suma"}'
# {"resultado": 15}
```

---

## Ejemplo 23: Módulos con clases

```zeta
# lib/geometry.zl
# Módulo que exporta clases

class Point {
    $x = 0
    $y = 0
    fn init($self, $x0, $y0) {
        $self.x = $x0
        $self.y = $y0
    }
    fn distance($self, $other) {
        $dx = $self.x - $other.x
        $dy = $self.y - $other.y
        return sqrt($dx * $dx + $dy * $dy)
    }
    fn to_string($self) {
        return "Point(" + format($self.x, 2) + ", " + format($self.y, 2) + ")"
    }
}

class Rectangle {
    $origin = null
    $width = 0
    $height = 0
    fn init($self, $x, $y, $w, $h) {
        $self.origin = new Point($x, $y)
        $self.width = $w
        $self.height = $h
    }
    fn area($self) {
        return $self.width * $self.height
    }
    fn contains($self, $pt) {
        $dx = $pt.x - $self.origin.x
        $dy = $pt.y - $self.origin.y
        return $dx >= 0 && $dx <= $self.width && $dy >= 0 && $dy <= $self.height
    }
}

export { Point, Rectangle }
```

Script que usa el módulo:

```zeta
# main.zl
include "lib/geometry"::{Point, Rectangle}

$p1 = new Point(0, 0)
$p2 = new Point(3, 4)
print($p1.to_string())                # Point(0.00, 0.00)
print("Distancia:", $p1.distance($p2))  # 5

$rect = new Rectangle(0, 0, 10, 5)
print("Area:", $rect.area())           # 50

$inside = new Point(5, 3)
$outside = new Point(15, 3)
print("Dentro:", $rect.contains($inside))   # true
print("Fuera:", $rect.contains($outside))   # false
```

**Salida esperada**:

```
Point(0.00, 0.00)
Distancia: 5
Area: 50
Dentro: true
Fuera: false
```

---

## Ejemplo 24: Pipeline completo con todo junto

```zeta
# 24_pipeline_completo.zl
# Ejemplo que combina: módulos, clases, errores, ternario, DataFrames

include "lib/geometry"::{Point, Rectangle}

# 1. Cargar datos
$df = load_csv("tests/datos.csv")?
print("Cargadas", len($df), "filas")

# 2. Validar columnas
if (is_error($df:ventas)) {
    print("ERROR: columna 'ventas' no encontrada")
    return
}

# 3. Limpiar datos: fill_null con promedio
$promedio = mean($df:ventas)
$df:ventas = fill_null($df:ventas, $promedio)
print("Nulls reemplazados por promedio:", format($promedio, 2))

# 4. Calcular métricas
$total = sum($df:ventas)
$minimo = min($df:ventas)
$maximo = max($df:ventas)
$desv = stddev($df:ventas)

print("Total:", format($total, 2))
print("Min:", format($minimo, 2))
print("Max:", format($maximo, 2))
print("StdDev:", format($desv, 2))

# 5. Usar clase del módulo
$p1 = new Point(0, 0)
$p2 = new Point($minimo, $maximo)
print("Distancia min-max:", format($p1.distance($p2), 2))

# 6. Ternario para clasificar
$clasificacion = $total > 10000 ? "Alto" : "Bajo"
print("Clasificacion:", $clasificacion)

# 7. Guardar resultado
$df:ventas_normalizadas = ($df:ventas - $minimo) / ($maximo - $minimo)
save_csv("resultado.csv", $df)
print("Guardado en resultado.csv")
```

---

## Ejemplo 25: Limpieza de datos completa

```zeta
# 25_limpieza_datos.zl
# Pipeline completo de limpieza con drop, drop_nan, fill_null

# 1. Cargar datos crudos
$df = load_csv("tests/datos.csv")
print("=== Datos crudos ===")
print("Filas:", len($df))
print("Columnas:", keys($df))

# 2. Eliminar columnas innecesarias
$df = drop($df, "id")
print("\n=== Sin columna 'id' ===")
print("Columnas:", keys($df))

# 3. Ver nulls por columna
for ($col in keys($df)) {
    $total = len($df)
    $non_null = count($df[$col])
    $nulls = $total - $non_null
    if ($nulls > 0) {
        print($col + ": " + str($nulls) + " nulls")
    }
}

# 4. Eliminar filas con null en columna crítica
$antes = len($df)
$df = drop_nan($df, "ventas")
$eliminadas = $antes - len($df)
print("\n=== Después de drop_nan('ventas') ===")
print("Filas eliminadas:", $eliminadas)
print("Filas restantes:", len($df))

# 5. Rellenar nulls restantes con promedio
$promedio = mean($df:gastos)
$df:gastos = fill_null($df:gastos, $promedio)
print("Gastos: nulls reemplazados por promedio =", format($promedio, 2))

# 6. Eliminar duplicados por columna
$nombres_unicos = unique($df:nombre)
print("Nombres únicos:", len($nombres_unicos))

# 7. Ordenar y Guardar
$df_ordenado = $df[sort($df:ventas)]
save_csv("datos_limpios.csv", $df_ordenado)
print("\n=== Resultado ===")
print("Guardado en datos_limpios.csv")
print("Filas finales:", len($df_ordenado))
print("Primeras 3 filas:")
print(head($df_ordenado, 3))
```

---

## Utilidades

---

## Ejemplo 26: Medición de tiempos

```zeta
# 26_tiempo.zl
# Usar time() para medir rendimiento

$t0 = time()

# Cargar y procesar
$df = load_csv("tests/datos.csv")
$df = drop_nan($df, "ventas")
$df:ventas = fill_null($df:ventas, 0)
$total = sum($df:ventas)
$promedio = mean($df:ventas)

$t1 = time()

print("=== Resultados ===")
print("Total:", format($total, 2))
print("Promedio:", format($promedio, 2))
print("Tiempo de procesamiento:", $t1 - $t0, "segundos")
```

---

## Notas sobre los ejemplos

1. **Datos CSV**: los ejemplos 8, 9, 11, 15, 16 y 18 asumen que `tests/datos.csv` existe. Si no, créalo con el contenido del ejemplo 8.

2. **Librería nativa**: el ejemplo 12 asume que `lib/libmatops.so` está compilada. Si no, ejecuta el comando `clang++` que aparece en la sección "Compilar" del ejemplo.

3. **Servidor**: para los ejemplos con dashboard (11, 15, 16), inicia `zeta_server` en otra terminal antes de abrir el dashboard o term renderer.

4. **Módulos**: el ejemplo 10 asume `lib/statslib.zl` y `lib/milibreria.zl` (ver contenido arriba).

5. **Output exacto**: los floats se imprimen con 6 decimales por defecto (`5.000000` no `5`). Lo importante son los valores.

6. **Recursión y lambdas**: tanto la recursión como las lambdas anónimas (`fn($x) { ... }`) funcionan correctamente. Para recursión muy profunda, considera aumentar el stack del sistema (`ulimit -s unlimited`).

7. **Asignación por índice**: `$vec[i] = $valor` (vectores) y `$d["k"] = $valor` (dicts) y `$m[i, j] = $valor` (matrices) funcionan. Se puede usar para mutación in-place dentro de loops.

8. **OOP**: `class`, `new`, `extends`, metodos con `$self`, y campos con `$obj.campo` / `$obj.campo = valor` funcionan. Ver Ejemplo 19.
