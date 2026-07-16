# 6. Imports y módulos

Zeta tiene un sistema de imports tipo **Rust**: declarativos, con caché, detección de ciclos, y tres formas (`full`, `selectivo`, `alias`).

## 6.1. Formas de `include`

### Full import

```zeta
include "statslib"
```

Importa todos los símbolos exportados del módulo y los expone dentro de un **namespace** (dict) con el nombre base del archivo.

```zeta
include "statslib"
# Crea el dict 'statslib' en el ámbito global
print(statslib::mean($datos))
```

### Selective import

```zeta
include "statslib"::{mean, stddev}
```

Importa solo los nombres listados. Estos se exponen **directamente en el ámbito global** (sin prefijo):

```zeta
include "statslib"::{mean, stddev}
print(mean($datos))    # sin prefijo
print(stddev($datos))
```

Si el módulo no exporta un nombre solicitado, es un **error de import** (no un warning).

### Aliased import

```zeta
include "statslib" as st
```

Importa todos los símbolos bajo un alias en lugar del nombre del archivo:

```zeta
include "statslib" as st
print(st::correlation($a, $b))
```

## 6.2. Export: controlar la API pública

En el archivo del módulo, usa `export { nombre1, nombre2, ... }` para declarar explícitamente qué nombres son públicos:

```zeta
# lib/statslib.zl

fn mean($vec) {
    # ... implementación
}

fn stddev($vec) {
    # ... implementación
}

fn _helper($x) {
    # ... no se exporta
}

export { mean, stddev }
```

### ¿Qué pasa si no hay `export`?

**Todos los símbolos nuevos** del módulo se hacen públicos (modo permisivo). Esto es por compatibilidad con scripts simples donde no quieres pensar en `export`.

Con `export`, solo los nombres listados son accesibles desde fuera.

## 6.3. Resolución de rutas

Cuando haces `include "statslib"`, el intérprete busca en este orden:

1. **Ruta exacta** tal como la escribiste (relativa al working dir).
2. **`./lib/` + nombre** (`./lib/statslib`).
3. **Cada path en `ZETA_PATH`** (variable de entorno separada por `:`).

```bash
# Ejemplo de ZETA_PATH
export ZETA_PATH="/usr/local/lib/zeta:/home/juan/proyectos/zeta/lib"
./zeta mi_script.zl
```

En `mi_script.zl`:

```zeta
include "statslib"   # busca en CWD, ./lib/, /usr/local/lib/zeta/, /home/juan/proyectos/zeta/lib/
```

## 6.4. Caché de módulos

Los módulos importados se **cachean** en `Interpreter::modulos_cache_` (un `map<string, ModuleSnapshot>` con la ruta canónica como clave). Si importas el mismo módulo dos veces, **solo se ejecuta una vez**.

```zeta
include "statslib"        # ejecuta statslib.zl, cachea
include "statslib"::{mean}    # usa caché, no re-ejecuta
```

El snapshot guarda:

- `ruta` canónica
- `simbolos`: `map<string, ValorZeta>` con los nombres exportados
- `exports`: `set<string>` con los nombres públicos
- `ast`: `shared_ptr<NodoAST>` con el AST completo (mantiene vivas las funciones que capturan cierres)

El `ast` se mantiene vivo para que los cierres de las funciones (que apuntan a `NodoAST*` raw en `func_cuerpo`) sigan siendo válidos.

## 6.5. Detección de ciclos

Si `A.zl` incluye `B.zl` que incluye `A.zl`, el intérprete detecta el ciclo y lanza un error:

```
Error de import: Ciclo de importacion detectado: /path/A.zl
```

El chequeo usa una **pila de imports** (`pila_imports_`) que se empuja al entrar a un módulo y se pop al salir.

## 6.6. Aislamiento de scope

Cuando se importa un módulo, sus definiciones se ejecutan en el **ámbito global** (no hay un scope de módulo separado). Pero los nombres definidos por el módulo se eliminan del ámbito global después de importarse, excepto los que están en el `dict` del namespace:

```zeta
# Antes del import
include "statslib"

# Después del import:
# - 'mean' y 'stddev' se eliminan del global (si el módulo los definió)
# - 'statslib' se crea como dict con {mean, stddev}
# - O 'mean' y 'stddev' se quedan en el global (si se importaron selectivamente)
```

Esto evita contaminar el espacio de nombres del usuario con helpers internos.

## 6.7. Ejemplo completo: módulo `statslib`

```zeta
# lib/statslib.zl
fn mean($vec) {
    $n = len($vec)
    if ($n == 0) { return 0 }
    $suma = 0
    for ($i in range($n)) {
        if (!is_null($vec[$i])) {
            $suma = $suma + $vec[$i]
        }
    }
    return $suma / $n
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

Uso desde el script:

```zeta
# tests/test_import.zl
include "statslib"
$datos = <1, 2, 3, 4, 5>
print("mean =", statslib::mean($datos))         # 3
print("stddev =", statslib::stddev($datos))     # 1.41421...
print("corr =", statslib::correlation($datos, $datos))  # 1
```

## 6.8. Imports selectivos y alias combinados

Puedes combinar las tres formas, pero solo una por `include`:

```zeta
# OK: dos includes del mismo módulo
include "statslib"
include "statslib" as st   # alias

# OK: selectivo y full
include "statslib"::{mean}
include "utils"            # otro módulo
```

No es posible hacer `include "statslib"::{mean} as st::m` (no hay sintaxis para re-aliasar selectivo).

## 6.9. Namespace `::` vs `.`

- **`::`** es para acceder a un símbolo dentro de un namespace (módulo).
- **`.`** **no existe** en Zeta. Se podría confundir con decimales (`$x.5`), por eso se eligió `::`.

```zeta
include "statslib"
print(statslib::mean($x))   # OK
print(statslib.mean($x))    # ERROR de parseo
```

## 6.10. Orden de evaluación

Los `include` se ejecutan en el orden en que aparecen. Si A.zl importa B.zl, B se ejecuta completamente antes de continuar con A:

```zeta
# main.zl
include "a"     # ejecuta a.zl completo
include "b"     # luego ejecuta b.zl completo
print("main")   # al final
```

Si A.zl contiene `include "b"`, B se ejecuta dos veces (una desde main, una desde A), pero la **caché** evita la segunda ejecución: la segunda vez se usa el snapshot cacheado.

## 6.11. Recursión de imports

Un módulo puede importarse a sí mismo (o transitivamente). El chequeo de ciclos lo permite si no crea un loop infinito, pero en general es mala idea:

```zeta
# lib/recursive.zl
include "recursive"::{helper}    # CUIDADO: ciclo infinito si no hay caché
fn helper($x) { ... }
```

En la práctica, el `modulos_cache_` rompe el ciclo: la primera ejecución entra, la recursión encuentra la entrada en caché y retorna, la primera ejecución termina, y el módulo queda disponible.

## 6.12. Depuración de imports

Si un import falla, el error es descriptivo:

```
Error de import: No se encontro el modulo: statslib
```

```
Error de import: El modulo /path/statslib.zl no exporta: foo
```

```
Error de import: Ciclo de importacion detectado: /path/a.zl
```

```
Error de import: Error en modulo /path/statslib.zl: ...
```

## 6.13. La macro `ZETA_PATH` como estándar

Recomendamos crear una variable de entorno estándar para los módulos compartidos:

```bash
# En tu .bashrc / .config/fish/config.fish
export ZETA_PATH="$HOME/.local/lib/zeta:/opt/zeta/lib"
```

Esto permite que tus scripts `.zl` sean portables: los módulos en `~/.local/lib/zeta/` están disponibles sin cambios en el script.

## 6.14. Funciones nativas disponibles

Zeta incluye un conjunto de funciones nativas integradas en el intérprete. No necesitan `include` — están siempre disponibles.

### Estadística y agregación

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `sum($vec)` | `sum(vec)` | num | Suma total (ignora nulls) |
| `mean($vec)` | `mean(vec)` | num | Promedio |
| `min($vec)` | `min(vec)` | num | Valor mínimo |
| `max($vec)` | `max(vec)` | num | Valor máximo |
| `stddev($vec)` | `stddev(vec)` | num | Desviación estándar |
| `count($vec)` | `count(vec)` | num | Elementos no nulos |

### Matemáticas

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `abs($x)` | `abs(num)` | num | Valor absoluto |
| `round($x)` | `round(num)` | num | Redondeo |
| `floor($x)` | `floor(num)` | num | Redondeo hacia abajo |
| `ceil($x)` | `ceil(num)` | num | Redondeo hacia arriba |
| `pow($b, $e)` | `pow(num, num)` | num | Potencia |
| `sqrt($x)` | `sqrt(num)` | num | Raíz cuadrada |

### Cadenas

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `len($s)` | `len(str)` | num | Longitud |
| `upper($s)` | `upper(str)` | str | Mayúsculas |
| `lower($s)` | `lower(str)` | str | Minúsculas |
| `substr($s, $i, [$n])` | `substr(str, num, [num])` | str | Subcadena |
| `split($s, $sep)` | `split(str, str)` | str_vec | Dividir por separador |
| `join($v, $sep)` | `join(str_vec, str)` | str | Unir con separador |
| `replace($s, $old, $new)` | `replace(str, str, str)` | str | Reemplazar ocurrencias |
| `find($s, $sub)` | `find(str, str)` | num | Posición de subcadena (-1 si no encuentra) |

### Vectores

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `push($v, $val)` | `push(vec, num)` | vec | Agregar elemento |
| `reverse($v)` | `reverse(vec)` | vec | Invertir |
| `sort($v)` | `sort(vec)` | vec | Ordenar |
| `unique($v)` | `unique(vec)` | vec | Elementos únicos |
| `range($n)` / `range($i, $f)` / `range($i, $f, $p)` | `range(...)` | vec | Generar rango |
| `head($v, $n=5)` | `head(vec, [num])` | vec | Primeros N elementos |

### DataFrames

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `select($df, $col)` | `select(df, str)` | vec | Obtener columna |
| `drop($df, $col)` | `drop(df, str)` | df | Eliminar columna |
| `drop_nan($df, $col)` | `drop_nan(df, str)` | df | Eliminar filas con null |
| `head($df, $n=5)` | `head(df, [num])` | df | Primeras N filas |

### Diccionarios

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `keys($d)` | `keys(dict)` | str_vec | Claves |
| `values($d)` | `values(dict)` | vec | Valores |

### I/O

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `load_csv($ruta, $delim)` | `load_csv(str, [str])` | df | Cargar CSV/TSV |
| `load_json($ruta)` | `load_json(str)` | df | Cargar JSON |
| `load_xlsx($ruta)` | `load_xlsx(str)` | df | Cargar Excel |
| `save_csv($ruta, $df, $delim)` | `save_csv(str, df, [str])` | null | Guardar CSV |
| `save_xlsx($ruta, $df)` | `save_xlsx(str, df)` | null | Guardar Excel |
| `guardar_grafo($ruta)` | `guardar_grafo(str)` | null | Guardar escena JSON |
| `cargar_grafo($ruta)` | `cargar_grafo(str)` | scene | Cargar escena |
| `grafo_actual()` | `grafo_actual()` | scene | Obtener escena actual |

### Visualización (escenas)

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `scene($t, $a)` | `scene(str, str)` | scene | Crear escena |
| `layout($tipo, $cols, $gap, $bg)` | `layout(str, num, num, str)` | null | Configurar layout |
| `add_metric($t, $v, $u, $c)` | `add_metric(str, num, str, str)` | null | Agregar métrica |
| `add_line_plot($d, $t, $x, $y)` | `add_line_plot(dict, str, str, str)` | null | Gráfico de líneas |
| `add_bar_chart($d, $t, $x, $y)` | `add_bar_chart(dict, str, str, str)` | null | Gráfico de barras |
| `add_scatter($d, $t, $x, $y)` | `add_scatter(dict, str, str, str)` | null | Dispersión |
| `add_histogram($d, $t, $col, $bins)` | `add_histogram(dict, str, str, num)` | null | Histograma |
| `add_linear_regression($d, $t, $x, $y)` | `add_linear_regression(dict, str, str, str)` | null | Regresión lineal |

### Inspección y tipos

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `type($x)` | `type(any)` | str | Tipo del valor |
| `is_null($x)` | `is_null(any)` | bool | Es null |
| `is_error($x)` | `is_error(any)` | bool | Es error |

### Sistema

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `print(...)` | `print(any...)` | null | Imprimir |
| `time()` | `time()` | num | Timestamp actual |
| `format($n, $d)` | `format(num, num)` | str | Formatear número |
| `load_lib($ruta, $funcs)` | `load_lib(str, dict)` | str | Cargar librería nativa |
| `route($method, $path, $fn)` | `route(str, str, func)` | null | Definir ruta HTTP |

### Programación funcional

| Función | Firma | Retorna | Descripción |
|---------|-------|---------|-------------|
| `map($v, $fn)` | `map(vec, func)` | vec | Aplicar función a cada elemento |
| `filter($v, $fn)` | `filter(vec, func)` | vec | Filtrar elementos |
| `reduce($v, $fn, $init)` | `reduce(vec, func, num)` | num | Reducir a un valor |

## 6.15. Tabla resumen de imports

| Sintaxis | Crea namespace | Expone en global | Forma |
|----------|----------------|------------------|-------|
| `include "x"` | `x` (dict) | - | Full |
| `include "x"::{a, b}` | - | `a`, `b` | Selective |
| `include "x" as y` | `y` (dict) | - | Aliased |

Combinaciones válidas:

- `include "x"` y `include "x" as y` → dos namespaces con los mismos símbolos.
- `include "x"` y `include "x"::{a}` → namespace `x` y además `a` en global.

Inválido:

- `include "x"::{a} as y::a` (no hay sintaxis de re-aliasing selectivo).
- `include x` (sin comillas; debe ser literal).
