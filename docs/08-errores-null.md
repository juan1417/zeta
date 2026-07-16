# 8. Errores y null como datos

Zeta trata los **errores como valores**, no como excepciones. Esto tiene tres consecuencias poderosas:

1. **Los errores son serializables** (a JSON, a disco, por red).
2. **Los errores se pueden almacenar** en vectores, DataFrames, etc.
3. **La propagación con `?` es explícita y opcional** — puedes inspeccionar errores manualmente.

## 8.1. Crear errores: `mk_err`

```zeta
fn dividir($a, $b) {
    if ($b == 0) {
        return mk_err("division", "no se puede dividir por cero")
    }
    if (is_null($a) || is_null($b)) {
        return mk_err("null_input", "argumentos nulos")
    }
    return $a / $b
}
```

`mk_err($tipo, $mensaje)` devuelve un `ValorZeta` de tipo `ERR` con un `ErrorZeta{tipo, mensaje, linea}`.

### Tipos de error comunes

Convención recomendada (no impuesta):

| Tipo | Cuándo usarlo |
|------|---------------|
| `"io"` | Errores de I/O (archivo no encontrado, fallo de red) |
| `"parse"` | Errores de parseo (CSV malformado, JSON inválido) |
| `"type"` | Error de tipo (operación sobre tipo incorrecto) |
| `"division"` | División por cero |
| `"index"` | Índice fuera de rango |
| `"null_input"` | Argumento null donde se requiere un valor |
| `"runtime"` | Error genérico en tiempo de ejecución |
| `"import"` | Error de import (módulo no encontrado, ciclo) |
| `"dl"` | Error al cargar librería nativa |
| `"network"` | Error de red (cliente HTTP user-defined) |
| `"scene"` | Error de escena (no hay scene activa, nodo inválido) |

## 8.2. Detectar errores: `is_error`

```zeta
$resultado = dividir(10, 0)
if (is_error($resultado)) {
    print("ERROR:", $resultado.mensaje)
    print("Tipo:", $resultado.tipo)
}
```

`is_error(x)` devuelve `true` si el valor es un `ERR`.

## 8.3. Propagación con `?`

El operador `?` después de una asignación **propaga el error automáticamente**:

```zeta
$x = leer_archivo("datos.csv")?
```

Equivale a:

```zeta
$x_temp = leer_archivo("datos.csv")
if (is_error($x_temp)) {
    return $x_temp    # propaga el error al llamante
}
$x = $x_temp
```

El operador `?` solo funciona **después de `=`** (asignación). No se puede usar en medio de una expresión.

### Propagación encadenada

```zeta
fn procesar_datos() {
    $crudo = load_csv("input.csv")?      # si falla, retorna el error
    $limpio = limpiar($crudo)?            # si falla, retorna
    return analizar($limpio)?             # si falla, retorna
}
```

Si cualquiera de las llamadas retorna un error, la función `procesar_datos` retorna ese error inmediatamente. El llamante puede chequear con `is_error`.

## 8.4. Errores en vectores y DataFrames

Como los errores son valores, se pueden almacenar en colecciones:

```zeta
$errores = <mk_err("a", "msg1"), mk_err("b", "msg2"), 42>

for ($i in range(len($errores))) {
    if (is_error($errores[$i])) {
        print("Error en posición", $i, ":", $errores[$i].mensaje)
    } else {
        print("Valor:", $errores[$i])
    }
}
```

Esto es útil para **batches de operaciones**: ejecutas N transformaciones, recoges los errores, y al final los reportas en bulk.

## 8.5. Errores en el servidor HTTP

Cuando `zeta_server` recibe un `POST /api/run` con código que produce un error, retorna:

```json
{
  "error": "division por cero"
}
```

con HTTP code 500. Los errores de parseo (código inválido) también retornan 500 con el mensaje de excepción.

## 8.6. `null` vs `ERR`

`null` y `ERR` son **conceptos diferentes**:

| Concepto | Representación | Cuándo usarlo |
|----------|----------------|---------------|
| `null` (NaN) | `double` con valor NaN | "Falta un valor" (dato ausente, no aplica, no se midió) |
| `ERR` | Struct con tipo, mensaje, línea | "Algo falló" (error de programación, input inválido, I/O) |

Ejemplo: una columna de "temperatura" puede tener `null` para días sin medición (es un dato ausente, no un error). Pero si el archivo CSV está corrupto, eso es un `ERR` que debe propagarse.

```zeta
# null: dato ausente (no es error)
$temp = <-10, null, 5, null, 20>     # 2 días sin medición
$promedio = mean($temp)              # funciona, ignora nulls

# err: input inválido
$df = load_csv("archivo_inexistente.csv")
if (is_error($df)) {
    print("No se pudo cargar:", $df.mensaje)
}
```

## 8.7. `is_null` en diferentes tipos

`is_null()` funciona con todos los tipos de Zeta:

```zeta
print(is_null(null))            # true
print(is_null(0))               # false (0 es un número válido)
print(is_null(""))              # true (string vacío se considera null)
print(is_null(<1, null, 3>))    # <false, true, false>  (bool_vec)
print(is_null(<"a", "", "c">))  # <false, true, false>  (str_vec)
print(is_null(load_csv("x")))   # false (un DataFrame nunca es null)
```

## 8.8. `fill_null($valor, $defecto)` — reemplazar nulos

Reemplaza todos los `null` (NaN en num, string vacío en str) por un valor por defecto:

```zeta
$v = <1, null, 3, null, 5>
print(fill_null($v, 0))        # <1, 0, 3, 0, 5>

$nombres = <"Ana", "", "Carlos">
print(fill_null($nombres, "N/A"))   # <"Ana", "N/A", "Carlos">

$x = null
print(fill_null($x, 42))        # 42

$y = 10
print(fill_null($y, 42))        # 10 (ya tiene valor)
```

`is_null(vec)` devuelve un `BOOL_VEC` con la verificación elemento a elemento. Esto es muy útil para máscaras:

```zeta
$mask = is_null($datos:ventas)         # bool_vec
$limpio = $datos[[!$mask]]             # filtra filas donde NO es null
```

## 8.9. Crear nulls explícitamente: `mk_null_val()`

En algunos contextos, necesitas producir un null explícitamente (e.g., en un map):

```zeta
fn safe_sqrt($x) {
    if ($x < 0) {
        return mk_null_val()
    }
    return sqrt($x)
}

print(map(<4, -1, 9, -16>, safe_sqrt))    # <2, null, 3, null>
```

## 8.10. Conversión entre `null` y `ERR`

No hay conversión automática. Si tienes un `null` y quieres tratarlo como error, créalo explícitamente:

```zeta
fn check_not_null($x) {
    if (is_null($x)) {
        return mk_err("null_input", "se esperaba un valor no nulo")
    }
    return $x
}
```

Si tienes un `ERR` y quieres "aplanarlo" a null, no hay un builtin; tienes que chequear con `is_error` y retornar null manualmente.

## 8.11. Tabla de operadores sobre null

| Operación | Resultado con null |
|-----------|---------------------|
| `null + 5` | null |
| `null * 2` | null |
| `5 / null` | null |
| `null == null` | false (¡ojo!) |
| `null != null` | true (¡ojo!) |
| `null > 5` | false |
| `null < 5` | false |
| `!null` | error de tipo (no es bool) |
| `is_null(null)` | true |
| `is_error(null)` | false |
| `null ? "x" : "y"` | error de tipo (ternario requiere bool) |

**La asimetría de `==` y `!=` con null** es intencional y refleja el estándar IEEE 754. Si necesitas chequear null, usa `is_null(x)`, no `x == null`.

## 8.12. Ventajas del modelo

1. **No hay try/catch**: el flujo de control nunca se interrumpe inesperadamente.
2. **Errores son datos**: puedes analizarlos, agregarlos, graficarlos.
3. **Propagación explícita**: el `?` documenta visualmente qué llamadas pueden fallar.
4. **Sin overhead en el happy path**: un `?` se compila a una comparación y un jump; no hay unwinding.
5. **Testeable**: puedes mockear errores fácilmente (crear un `ERR` directamente en lugar de provocar la condición).

## 8.13. Cuándo **no** usar este modelo

- **Loops críticos con muchos errores esperados**: si esperas que el 50% de las operaciones fallen, considera un patrón diferente (e.g., `try_or_default(x, 0)`).
- **Stack traces**: no hay stack traces (solo la línea del error). Si necesitas debuggear errores profundos, añade contexto manualmente al `mensaje`.
- **Errores de memoria**: el intérprete no atrapa segfaults. Un null dereference o un error de C++ crashea el proceso. Usa el wrapper `zeta_server` para隔离 el proceso.
