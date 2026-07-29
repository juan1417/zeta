# 2. Sintaxis

## Tabla de Contenidos

- [2.1. Estructura de un archivo `.zl`](#21-estructura-de-un-archivo-zl)
- [2.2. Comentarios](#22-comentarios)
- [2.3. Literales](#23-literales)
- [2.4. Variables: el sigil `$`](#24-variables-el-sigil-)
- [2.5. Asignación](#25-asignación)
- [2.6. Operadores](#26-operadores)
- [2.7. Vectores `<>`](#27-vectores-)
- [2.8. Matrices `<<>>`](#28-matrices-)
- [2.9. Diccionarios `{}`](#29-diccionarios-)
- [2.10. Indexado y acceso](#210-indexado-y-acceso)
- [2.11. Namespace `::`](#211-namespace-)
- [2.12. Funciones: sintaxis de llamada](#212-funciones-sintaxis-de-llamada)
- [2.13. Operador ternario `? :`](#213-operador-ternario-)
- [2.14. Estructuras de control](#214-estructuras-de-control)
- [2.15. Definición de funciones: `fn`](#215-definición-de-funciones-fn)
- [2.16. Imports: `include`](#216-imports-include)
- [2.17. Exposición: `export`](#217-exposición-export)
- [2.18. Print: `print(...)`](#218-print-print)
- [2.19. Tokens reservados (keywords)](#219-tokens-reservados-keywords)

---

## Resumen Rápido

### Literales

| Tipo | Sintaxis | Ejemplo | Tipo Interno |
|------|----------|---------|--------------|
| Número | Dígitos, opcional `.` | `42`, `3.14`, `0.5` | `NUM` |
| Número (separador miles) | Dígitos con `,` cada 3 | `1,000`, `1,000,000` | `NUM` |
| Número (científico) | Notación `e`/`E` con `+`/`-` | `1.5e3`, `2.5e-3`, `1E10` | `NUM` |
| Cadena | `"..."` con escapes `\n \t \" \\ \r` | `"hola\nmundo"` | `STR` |
| Booleano | `true` / `false` | `true` | `BOOL` |
| Nulo | `null` | `null` | `NUM` con valor `quiet NaN` |
| Vector | `<v1, v2, ...>` | `<1, 2, 3>` | `VEC` |
| Matriz | `<<f1, f2, ...>>` (filas como vectores) | `<<1, 2>, <3, 4>>` | `MATRIZ` |
| Diccionario | `{"clave": valor, ...}` | `{"a": 1, "b": 2}` | `DICT` |

### Operadores

| Categoría | Operadores | Descripción |
|-----------|------------|-------------|
| **Aritméticos** | `+` `-` `*` `/` `%` | Suma, resta, multiplicación, división, módulo |
| **Comparación** | `==` `!=` `>` `<` `>=` `<=` | Igualdad, desigualdad, orden |
| **Lógicos** | `&&` `\|\|` `!` | AND, OR, NOT |
| **Asignación** | `=` | Asignación (no es expresión) |
| **Propagación** | `?` | Propaga errores después de asignación |
| **Ternario** | `?` `:` | Expresión condicional inline |

### Precedencia de Operadores (mayor a menor)

| Precedencia | Operador | Descripción |
|-------------|----------|-------------|
| 1 (mayor) | `-x` `!x` | Unario |
| 2 | `*` `/` `%` | Factor |
| 3 | `+` `-` | Término |
| 4 | `>` `<` `>=` `<=` | Comparación |
| 5 | `==` `!=` | Igualdad |
| 6 | `&&` | Lógico Y |
| 7 | `\|\|` | Lógico O |
| 8 (menor) | `?` `:` | Ternario |

---

## 2.1. Estructura de un archivo `.zl`

Un archivo Zeta es una secuencia de **declaraciones** separadas por saltos de línea. **No usa `;` como terminador** — los saltos de línea son los separadores de statements. Esto simplifica el parser (no hay "injection" de ASI tipo JS) y se siente natural para scripts analíticos donde cada línea es una transformación.

```zeta
# Comentario de una línea con #
$x = 1
$y = 2
print($x + $y)
```

Un bloque (cuerpo de `if`, `for`, `while`, `fn`) se delimita con `{}`:

```zeta
fn suma($a, $b) {
    return $a + $b
}
```

---

## 2.2. Comentarios

```zeta
# Comentario de una sola línea, hasta el final de la línea
print(1)  # también después de código
```

Los comentarios son **ignorados por el lexer** (no producen tokens). No hay comentarios de bloque `/* */`.

---

## 2.3. Literales

| Tipo | Sintaxis | Ejemplo | Tipo interno |
|------|----------|---------|--------------|
| Número | Dígitos, opcional `.` | `42`, `3.14`, `0.5` | `NUM` |
| Número (separador miles) | Dígitos con `,` cada 3 | `1,000`, `1,000,000` | `NUM` |
| Número (científico) | Notación `e`/`E` con `+`/`-` | `1.5e3`, `2.5e-3`, `1E10` | `NUM` |
| Cadena | `"..."` con escapes `\n \t \" \\ \r` | `"hola\nmundo"` | `STR` |
| Booleano | `true` / `false` | `true` | `BOOL` |
| Nulo | `null` | `null` | `NUM` con valor `quiet NaN` |
| Vector | `<v1, v2, ...>` | `<1, 2, 3>` | `VEC` |
| Matriz | `<<f1, f2, ...>>` (filas como vectores) | `<<1, 2>, <3, 4>>` | `MATRIZ` |
| Diccionario | `{"clave": valor, ...}` | `{"a": 1, "b": 2}` | `DICT` |

### Por qué `null` no es un tipo

`null` se representa como un `double` con valor `quiet NaN` (la convención IEEE 754 para "not a number"). Esto significa:

- `null + 5` da `null` automáticamente (NaN se propaga).
- `null == null` da `false` (NaN nunca es igual a sí mismo).
- `is_null(x)` se implementa como `x != x` en una sola instrucción de CPU.

Si fuera un tipo separado, cada operación aritmética necesitaría un `if (es_null) return mk_null()` defensivo. Con NaN, el hardware lo hace gratis.

---

## 2.4. Variables: el sigil `$`

**Todas las variables empiezan con `$`.** Esto es obligatorio, no opcional.

```zeta
$x = 10              # OK
$nombre = "Zeta"     # OK
$lista = <1, 2, 3>   # OK
datos = 10           # ERROR de parseo
```

### Por qué `$` obligatorio

1. **Desambigua en el lexer**: sin `$`, el parser no sabe si `ventas` en `$datos:ventas` es una variable o un identificador. Con `$`, la producción es inequívoca: `$IDENTIFICADOR : IDENTIFICADOR`.
2. **Reserva keywords sin colisión**: `print`, `if`, `for` se pueden usar como claves de diccionario sin escapar.
3. **Permite HTML embebido en strings sin escaping**: `"<div>$valor</div>"` se distingue trivialmente.
4. **Visibilidad en scripts largos**: en un script de 500 líneas, escanear visualmente los `$` encuentra todas las variables de un vistazo.

---

## 2.5. Asignación

```zeta
$x = 10
$y = $x * 2
$df = {"col1": <1, 2, 3>, "col2": <4, 5, 6>}
```

La asignación es una **declaración**, no una expresión. Es decir, no puedes hacer `print($x = 5)` (eso sería un error de tipo en tiempo de parseo).

---

## 2.6. Operadores

### Aritméticos (precedencia: alta)

| Operador | Significado | Tipos aceptados |
|----------|-------------|-----------------|
| `+` | Suma | `num+num`, `str+str` (concat), `vec+vec` (concat) |
| `-` | Resta | `num-num` |
| `*` | Multiplicación | `num*num` |
| `/` | División | `num/num` (división por 0 da `null`) |
| `%` | Módulo | `num%num` (módulo por 0 da `null`) |

### Comparación

| Operador | Significado | Tipos aceptados |
|----------|-------------|-----------------|
| `==` | Igual | `num`, `str`, `bool` |
| `!=` | Distinto | `num`, `str`, `bool` |
| `>` | Mayor que | `num`, `str` (orden lexicográfico) |
| `<` | Menor que | `num`, `str` (orden lexicográfico) |
| `>=` | Mayor o igual | `num`, `str` (orden lexicográfico) |
| `<=` | Menor o igual | `num`, `str` (orden lexicográfico) |

### Lógicos

| Operador | Significado | Tipo requerido |
|----------|-------------|----------------|
| `&&` | Y lógico | ambos `bool` |
| `\|\|` | O lógico | ambos `bool` |
| `!` | Negación unaria | `bool` |

### Precedencia (de mayor a menor)

```
1. Unario:  -x  !x
2. Factor:  * / %
3. Término: + -
4. Comparación: > < >= <=
5. Igualdad: == !=
6. Lógico Y: &&
7. Lógico O: ||
8. Ternario:  ?  :
```

### Propagación de errores: `?`

El operador `?` se aplica **después de una asignación** y propaga errores:

```zeta
$x = leer_archivo("datos.csv")?
```

Si `leer_archivo` retorna un `ERR`, la asignación se aborta y el error se propaga al contexto llamante. Si retorna un valor normal, se asigna.

Internamente, el parser genera un nodo `PROPAGACION` que envuelve el valor; el intérprete chequea `is_error(val)` y lo retorna sin asignar.

---

## 2.7. Vectores `<>`

```zeta
$nums = <1, 2, 3, 4, 5>
$mixto = <1, "dos", true>     # OK (todos se vuelven numéricos al evaluar)
$desde_rango = range(10)      # <0, 1, 2, ..., 9>
$concat = <1, 2> + <3, 4>     # <1, 2, 3, 4>
```

El lexer maneja la ambigüedad de `<` con la pila `profundidad_coleccion_`: después de un `<<` (matriz), el siguiente `<` se interpreta como inicio de vector. Después de un `=` o `(`, `<` se interpreta como `<` numérico (vector-abre) si le sigue un dígito o `-`; de lo contrario, como operador de comparación.

---

## 2.8. Matrices `<<>>`

```zeta
$m = <<1, 2, 3>, <4, 5, 6>>   # 2 filas, 3 columnas
$m2 = <<<1, 2>, <3, 4>, <5, 6>>   # 3 filas, 2 columnas
```

La sintaxis es **`<<` + filas separadas por coma + `>>`**. Cada fila es un vector (con o sin `<>` explícito en la primera fila; las filas siguientes sí lo requieren). Internamente se almacena como `vector<vector<double>>`.

Acceso por índice doble: `$m[fila, columna]`.

---

## 2.9. Diccionarios `{}`

```zeta
$d = {"nombre": "Zeta", "version": 0.1, "nodos": <1, 2, 3>}
$d2 = {"a": 1, "b": 2}
```

Las claves son **siempre cadenas** (literales `"..."`). Los valores pueden ser cualquier expresión. Internamente es `std::map<std::string, ValorZeta>` (ordenados alfabéticamente, no `unordered_map`, para serialización determinística).

---

## 2.10. Indexado y acceso

### Por índice numérico: `[]`

```zeta
$nums = <10, 20, 30>
print($nums[0])      # 10
print($nums[2])      # 30
```

Los corchetes son **siempre** indexado numérico. Para vectores, matrices (con coma), diccionarios (devuelve un error si la clave no existe).

### Por índice doble (matriz): `[i, j]`

```zeta
$m = <<1, 2, 3>, <4, 5, 6>>
print($m[0, 0])      # 1
print($m[1, 2])      # 6
```

Los corchetes con coma son para matrices: `$m[fila, columna]`. Sin coma, son indexado simple para vectores y diccionarios.

### Acceso a columna de DataFrame: `:`

```zeta
$datos = load_csv("tests/datos.csv")
$ventas = $datos:ventas    # vector<double> con la columna "ventas"
```

El `:` después de un DataFrame es **acceso a columna**. Se puede encadenar para multi-columna (próximamente). Internamente es un nodo `ACCESO_COLUMNAS`.

### Filtrado de filas: `[[]]`

```zeta
$altos = $datos[[$datos:ventas > 1000]]   # DataFrame con filas donde ventas > 1000
```

Los corchetes dobles son **filtros booleanos por fila**. Internamente el intérprete itera cada fila, expone las columnas como variables en un scope temporal, evalúa la condición, y construye un nuevo DataFrame con las filas donde la condición es verdadera.

---

## 2.11. Namespace `::`

```zeta
include "statslib"
print(statslib::mean($datos))
```

El `::` se usa para **acceder a un símbolo dentro de un namespace** (típicamente un módulo importado). Es diferente de `:` (que es para columnas de DataFrame) y `.` (que no existe en Zeta para evitar ambigüedad con decimales en otros lenguajes).

---

## 2.12. Funciones: sintaxis de llamada

```zeta
# Función sin argumentos
print(42)

# Función con argumentos
sum($vector)
mean($datos:ventas)

# Argumentos múltiples separados por coma
pow(2, 10)

# Anidamiento
print(round(sqrt(144)))
```

Las funciones nativas se llaman **sin keyword** (estilo prefijo, como en C). No hay métodos (`$vec.len()` no funciona; es `len($vec)`). Esto simplifica el parser: `IDENTIFICADOR (` siempre es una llamada.

---

## 2.13. Operador ternario `? :`

```zeta
$status = $edad >= 18 ? "adulto" : "menor"
```

Es la sintaxis clásica. **Pero cuidado**: el lexer desambigua `?` (ternario) del operador `?` de propagación de errores **por contexto**:

- Después de `=` o `,` o `(` → es ternario.
- Después de una expresión de asignación completa → es propagación de errores.

Internamente, `?` ternario genera un nodo `TERNARIA` con tres hijos (cond, si, sino). El `?` de propagación genera un nodo `PROPAGACION` que envuelve el valor a asignar.

### Ternario vectorizado

El ternario también funciona sobre vectores y máscaras booleanas:

```zeta
$resultado = is_null($x) ? 0 : $x    # si x es null -> 0, si no -> x (vectorizado)
```

Si la condición es un `VEC` o `BOOL_VEC`, el ternario se evalúa **elemento a elemento** y devuelve un vector del mismo tamaño.

---

## 2.14. Estructuras de control

```zeta
# if/else
if ($x > 0) {
    print("positivo")
} else {
    print("no positivo")
}

# for/in
for ($i in range(10)) {
    print($i)
}

# while
$i = 0
while ($i < 10) {
    print($i)
    $i = $i + 1
}

# break, continue
for ($i in range(100)) {
    if ($i == 50) { break }
    if ($i % 2 == 0) { continue }
    print($i)
}
```

Ver detalles y semántica de cada uno en [Control de flujo y funciones](./docs/04-control-y-funciones.md).

---

## 2.15. Definición de funciones: `fn`

```zeta
fn cuadrado($x) {
    return $x * $x
}

print(cuadrado(5))   # 25

# Recursión
fn factorial($n) {
    if ($n <= 1) {
        return 1
    }
    return $n * factorial($n - 1)
}
```

Los parámetros **siempre llevan `$`**. El cuerpo es un bloque `{}` con declaraciones. La función se registra en el ámbito global y se puede llamar antes o después de su definición (la resolución es por nombre, no por orden).

### Lambdas anónimas

`fn` sin nombre (seguido directamente de `(`) es una **expresión** que evalúa a un valor de tipo `FUNC`. Se puede asignar a una variable o pasar a funciones de orden superior:

```zeta
$sq = fn($x) { return $x * $x }
print(sq(5))    # 25

$nums = <1, 2, 3>
print(map($nums, fn($x) { return $x * 10 }))    # <10, 20, 30>
```

Para llamar a una función almacenada en una variable, usa el nombre **sin sigil** (`f(args)`). La sintaxis `$f[i]` es acceso por índice.

---

## 2.16. Imports: `include`

```zeta
# Importar todo el módulo
include "statslib"
print(statslib::mean($datos))

# Importar selectivo
include "statslib"::{mean, stddev}
print(mean($datos))

# Importar con alias
include "statslib" as st
print(st::correlation($a, $b))
```

Ver detalles completos en [Imports y módulos](./docs/06-imports-modulos.md).

---

## 2.17. Exposición: `export`

```zeta
# En el archivo statslib.zl
fn mean($vec) { ... }
fn stddev($vec) { ... }

# Esto expone solo 'mean' y 'stddev'. 'mean' y 'stddev' son públicas.
export { mean, stddev }
```

`export` enumera explícitamente qué nombres del módulo son públicos. Si no hay `export`, todos los nombres nuevos del módulo son públicos (modo "todo es público" por compatibilidad con scripts simples). Ver [Imports](./docs/06-imports-modulos.md).

---

## 2.18. Print: `print(...)`

```zeta
print("Hola")
print("Tengo", 30, "años")     # múltiples args, separados por espacio
print($x + $y)
```

`print` acepta múltiples expresiones separadas por coma, las imprime separadas por un espacio, y termina con newline. El resultado es `null`.

---

## 2.19. Tokens reservados (keywords)

Lista completa de palabras que **no se pueden usar como nombre de variable** (deben ir con `$` y colisionan con keywords):

| Categoría | Keywords |
|-----------|----------|
| **Control de flujo** | `fn` `if` `else` `for` `while` `in` `return` `break` `continue` |
| **Módulos** | `include` `as` `export` |
| **Literales** | `true` `false` `null` |
| **I/O y visualización** | `load_csv` `plot` `serve` `metric` `dashboard` `route` |
| **OOP** | `class` `new` `this` `extends` |
| **Funciones de verificación** | `is_null` `is_error` |
| **Estadísticas** | `mean` `count` `sum` `min` `max` `stddev` |
| **Matemáticas** | `abs` `round` `floor` `ceil` `pow` `sqrt` |
| **Cadenas** | `len` `upper` `lower` `substr` |
| **Vectores** | `reverse` `sort` `unique` `push` |
| **Diccionarios** | `keys` `values` `type` `range` |
| **Matrices** | `transpose` `dot` `head` `select` |
| **Cadenas (avanzado)** | `split` `join` `replace` `find` |
| **Higher-order** | `map` `filter` `reduce` |

Estas son palabras reservadas a nivel lexer: si las escribes sin `$` se interpretan como la función built-in correspondiente. Para usarlas como claves de diccionario, deben ir como cadenas: `{"type": "valor"}` no `{"type": ...}`.
