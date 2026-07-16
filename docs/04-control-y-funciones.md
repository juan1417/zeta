# 4. Control de flujo y funciones

## 4.1. Condicionales: `if` / `else`

```zeta
if (condicion) {
    # bloque si verdadero
} else {
    # bloque si falso (opcional)
}
```

`if` requiere paréntesis alrededor de la condición. El bloque es obligatorio (siempre `{}`).

### `if` sin `else`

```zeta
if ($x > 100) {
    print("grande")
}
```

### `if` / `else if` / `else`

```zeta
if ($edad < 18) {
    print("menor")
} else if ($edad < 65) {
    print("adulto")
} else {
    print("senior")
}
```

Encadenar con `else if` se desazucara como `else { if (...) { ... } }` recursivamente. El parser maneja la recursión en `declaracion_if()`.

### Condición debe ser `bool`

`if` espera exactamente un `bool`. Si pasas un `num` o `str`, el intérprete lanza un error de tipo. Esto es intencional: en Python/JavaScript es común confundir `if ($x)` (truthy) con `if ($x != 0)` (comparación explícita). Zeta obliga a ser explícito.

```zeta
if ($x) { ... }       # ERROR: esperaba bool, recibio num
if ($x != 0) { ... }  # OK
```

## 4.2. Bucle `for` / `in`

```zeta
for ($variable in expresion_iterable) {
    # cuerpo
}
```

`expresion_iterable` debe ser un `VEC` (los demás tipos no son iterables todavía). `$variable` se asigna a cada elemento en cada iteración.

### Ejemplos

```zeta
# Iterar números
for ($i in range(10)) {
    print($i)        # 0, 1, 2, ..., 9
}

# Iterar un vector
$nums = <10, 20, 30>
for ($n in $nums) {
    print($n)
}

# Iterar con índice (usando range + indexado)
for ($i in range(len($nums))) {
    print("índice", $i, "valor", $nums[$i])
}
```

### `break` y `continue`

```zeta
for ($i in range(100)) {
    if ($i == 50) { break }       # sale del bucle
    if ($i % 2 == 0) { continue } # salta a la siguiente iteración
    print($i)
}
```

Internamente, `break` y `continue` lanzan un valor sentinel (`__ZETA_BREAK__`, `__ZETA_CONTINUE__`) que el bucle captura. No hay manera de que el código de usuario los vea accidentalmente (son strings mágicos que el intérprete filtra).

## 4.3. Bucle `while`

```zeta
$i = 0
while ($i < 10) {
    print($i)
    $i = $i + 1
}
```

`while` evalúa la condición antes de cada iteración. Si es `false` desde el inicio, el cuerpo nunca se ejecuta. Acepta `break` y `continue` igual que `for`.

## 4.4. Operador ternario

Expresión condicional inline:

```zeta
$status = $edad >= 18 ? "adulto" : "menor"
print($status)
```

Forma general: `condicion ? valor_si : valor_no`. La condición debe ser `bool`.

### Ternario vectorizado

Si la condición es un `VEC` (de doubles donde cada elemento se interpreta como truthy/falsy) o un `BOOL_VEC`, el ternario se evalúa **elemento a elemento**:

```zeta
# is_null devuelve BOOL_VEC
$mask = is_null(<1, null, 3, null, 5>)   # <false, true, false, true, false>
$result = $mask ? 0 : <1, 2, 3, 4, 5>    # <1, 0, 3, 0, 5>
```

Si `valor_si` y `valor_no` son vectores del mismo tamaño que la máscara, se indexa posición a posición. Si son escalares, se difunden (broadcast) a todo el vector.

## 4.5. Definición de funciones: `fn`

```zeta
fn nombre($param1, $param2) {
    # cuerpo
    return $resultado
}
```

- Los parámetros **siempre llevan `$`**.
- El cuerpo es un bloque `{}`.
- `return` es opcional; si se omite, la función retorna `null`.
- Las funciones se registran en el ámbito global al evaluarse la declaración.

### Funciones sin valor de retorno

```zeta
fn saludar($nombre) {
    print("Hola,", $nombre)
}

saludar("Mundo")    # imprime "Hola, Mundo"
# El valor de retorno es null
```

### Parámetros por valor

Los parámetros se pasan **por valor** (se copia el `ValorZeta`, que es un `shared_ptr`, así que la copia es barata — solo se incrementa el refcount).

```zeta
fn modificar($x) {
    $x = 99
    # $x se modifica solo dentro de este scope
}

$y = 10
modificar($y)
print($y)    # 10 (no se modificó)
```

### Recursión

```zeta
fn factorial($n) {
    if ($n <= 1) {
        return 1
    }
    return $n * factorial($n - 1)
}

print(factorial(5))    # 120
```

No hay límite de recursión configurable, pero cada llamada consume stack. Para recursión profunda, considera `for`.

### Funciones como valores (higher-order)

```zeta
fn doble($x) { return $x * 2 }
fn triple($x) { return $x * 3 }

$nums = <1, 2, 3, 4>
print(map($nums, doble))    # <2, 4, 6, 8>
print(map($nums, triple))   # <3, 6, 9, 12>

# filter
fn mayor_a_2($x) { return $x > 2 }
print(filter($nums, mayor_a_2))   # <3, 4>

# reduce
fn sumar($acc, $x) { return $acc + $x }
print(reduce($nums, sumar, 0))    # 10
```

### Lambdas anónimas

Además de las funciones con nombre, Zeta acepta **lambdas anónimas** como expresión: `fn($params) { cuerpo }`. Evalúan a un valor de tipo `FUNC` y se pueden asignar a una variable, pasar a `map`/`filter`/`reduce`, o usar directamente:

```zeta
# Asignada a variable (se referencia por nombre sin sigil)
$sq = fn($x) { return $x * $x }
print(sq(5))    # 25

# Pasada inline a map/filter/reduce
$nums = <1, 2, 3, 4, 5>
print(map($nums, fn($x) { return $x * 2 }))     # <2, 4, 6, 8, 10>
print(filter($nums, fn($x) { return $x > 2 }))  # <3, 4, 5>

# Lambda con múltiples argumentos
$add = fn($a, $b) { return $a + $b }
print($add(3, 4))    # 7  (pero también funciona add(3, 4))
```

Para invocar una función almacenada en una variable, usa el nombre **sin sigil**: `f(args)`. La sintaxis `$f[i]` es acceso por índice (lee el i-ésimo elemento del valor almacenado en `$f`).

### Lambdas recursivas

Una lambda puede referenciarse a sí misma a través del nombre de la variable donde se guarda:

```zeta
$fact = fn($n) {
    if ($n <= 1) {
        return 1
    }
    return $n * fact($n - 1)
}
print(fact(5))    # 120
```

Esto funciona porque la lambda se evalúa en un `mk_func` con su `func_cierre = ambito_global_`, y la variable `fact` ya está en el scope global en el momento de la llamada.

### Closures (cierres)

Las funciones capturan el **ámbito donde se definieron** (closure). Si defines una función dentro de otra o en el ámbito global, ve todas las variables de ese scope.

```zeta
$base = 10

fn con_base($x) {
    return $base + $x   # $base viene del scope global
}

print(con_base(5))    # 15
```

### Shadowing

Puedes redeclarar una variable en un scope interno; el shadowing termina cuando el scope muere:

```zeta
$x = 10
fn f() {
    $x = 99    # nueva $x local
    print($x)  # 99
}
f()
print($x)      # 10 (la $x global no cambió)
```

## 4.6. `return`

`return` termina la función inmediatamente y devuelve el valor de la expresión siguiente:

```zeta
fn clasificar($x) {
    if ($x > 0) { return "positivo" }
    if ($x < 0) { return "negativo" }
    return "cero"
}
```

Si `return` se omite, la función retorna `null` al final del bloque.

### Múltiples returns

`return` puede aparecer en cualquier parte del cuerpo. El primero que se ejecuta gana:

```zeta
fn buscar($vec, $target) {
    for ($i in range(len($vec))) {
        if ($vec[$i] == $target) {
            return $i
        }
    }
    return -1    # no encontrado
}
```

## 4.7. Anidamiento

Los bloques se pueden anidar arbitrariamente. Cada `{}` abre un nuevo scope con visibilidad de las variables del scope padre (lexical scoping):

```zeta
fn outer() {
    $x = 10
    fn inner() {
        $y = 20
        return $x + $y    # $x viene de outer, $y es local
    }
    return inner()
}
print(outer())    # 30
```

## 4.8. Ámbitos: cómo se resuelven los nombres

Zeta usa **tablas de símbolos anidadas** (`TablaSimbolos` con puntero al padre). Cuando el intérprete busca `$x`:

1. Busca en el scope actual.
2. Si no lo encuentra, sube al scope padre.
3. Continúa hasta el scope global.
4. Si no lo encuentra en el global, es un error de runtime ("variable no definida").

```zeta
$x = "global"

fn f() {
    print($x)    # "global" (sube al scope global)
}

fn g() {
    $x = "local en g"
    fn h() {
        print($x)    # "local en g" (sube a g, no a global)
    }
    h()
}
```

## 4.9. Resumen de palabras clave de control

| Keyword | Uso |
|---------|-----|
| `if (cond) { ... }` | Condicional |
| `else { ... }` | Rama del `if` |
| `for ($x in $iter) { ... }` | Bucle sobre iterable |
| `while (cond) { ... }` | Bucle con condición |
| `break` | Sale del bucle |
| `continue` | Salta a la siguiente iteración |
| `return expr` | Retorna de la función |
| `fn nombre($p1, $p2) { ... }` | Define función |
| `cond ? si : no` | Ternario inline |
| `asignacion = valor?` | Propagación de error |

## 4.10. Patrones comunes

### Map + filter encadenados

```zeta
fn cuadrado($x) { return $x * $x }
fn es_positivo($x) { return $x > 0 }

$nums = <-1, 2, -3, 4, -5>
$positivos = filter($nums, es_positivo)        # <2, 4>
$cuadrados = map($positivos, cuadrado)          # <4, 16>
```

### Búsqueda con salida temprana

```zeta
fn buscar_en_df($df, $col, $target) {
    $vals = $df:$col
    for ($i in range(len($vals))) {
        if ($vals[$i] == $target) {
            return $i
        }
    }
    return -1
}
```

### Validación al inicio

```zeta
fn procesar($datos) {
    if (is_null($datos)) {
        print("Datos nulos, abortando")
        return
    }
    if (len($datos) == 0) {
        print("Datos vacíos, abortando")
        return
    }
    # ... lógica principal
}
```

### Reducción con acumulador

```zeta
fn producto_total($vec) {
    return reduce($vec, fn($acc, $x) { return $acc * $x }, 1)
}
print(producto_total(<2, 3, 4>))    # 24
```

## 4.11. Programación orientada a objetos

Zeta tiene soporte de primera clase para OOP estilo Python/JS-Go. Las clases encapsulan estado (campos) y comportamiento (metodos) con herencia simple.

### Definir una clase

```zeta
class Counter {
    $count = 0           # campo con valor por defecto
    fn inc($self) {      # metodo: $self es el receptor
        $self.count = $self.count + 1
        return $self.count
    }
    fn get($self) {
        return $self.count
    }
}
```

Reglas:

- `class Nombre { ... }` declara una clase. El nombre empieza con mayuscula por convencion.
- Los campos son `$nombre = valor_inicial`. Se inicializan en cada `new`.
- Los metodos son `fn nombre($self, $arg1, $arg2) { ... }`. **`$self` es siempre el primer parametro** (convencion Go/Rust, evita romper la regla de sigilos).
- Los campos y metodos pueden estar en cualquier orden dentro del bloque.

### Crear instancias

```zeta
$c = new Counter()           # sin argumentos
$d = new Counter(10)         # si la clase define un metodo `init`, recibe los args
```

`new` crea un objeto nuevo, copia los valores por defecto de los campos (recorriendo la cadena de herencia), y si existe un metodo `init` lo invoca automaticamente con `($self, $arg1, ...)`.

### Acceder y modificar campos

```zeta
print($c.count)        # lectura: sintaxis de acceso a campo
$c.count = 99          # escritura: asignacion a campo
```

### Llamar metodos

```zeta
print($c.inc())        # sintaxis: $obj.metodo(args)
```

El interpreter traduce internamente `$c.inc(args)` a `inc($c, args)`. El `$self` se enlaza automaticamente.

### Herencia simple

```zeta
class Animal {
    $name = "anonimo"
    $legs = 4
    fn describe($self) {
        return $self.name
    }
}

class Dog extends Animal {
    $breed = "mestizo"
    fn describe($self) {           # override
        return $self.name
    }
}

class Puppy extends Dog {
    $age = 0
    # hereda describe() de Dog
}
```

Reglas:

- `class Hija extends Padre { ... }` declara herencia simple.
- Los campos de la clase padre se copian primero; la hija puede sobreescribirlos redeclarandolos.
- Los metodos se buscan en la clase del objeto, luego en su padre, luego en el abuelo, etc.
- Un metodo de la hija puede llamar al del padre con el nombre del metodo directamente (no hay `super` en v1).

### El `this` explicito (opcional)

Aunque `$self` es el primer parametro, dentro de un metodo tambien podes usar `this` para referirte al receptor actual:

```zeta
class Foo {
    $x = 0
    fn get_x() {
        return $self.x      # equivalente a this.x
    }
}
```

`this` devuelve el mismo objeto que `$self`. Es azucar sintactico para mantener familiaridad con JavaScript/Java.

### Limitaciones v1

- **Sin herencia multiple** — solo `extends Padre` (un unico padre).
- **Sin `super`** — para llamar al metodo del padre, referenciarlo por nombre desde la clase hija.
- **Sin interfaces ni mixins** — solo clases concretas con campos y metodos.
- **Sin visibilidad** — todos los campos son publicos (`$obj.campo` siempre funciona).
- **Sin metodos estaticos** — si necesitas uno, usa una funcion regular en el mismo archivo.
- **Sin `class method`** — todo metodo es de instancia.
- **Constructores via `init`** — si declaras un metodo llamado `init`, se invoca automaticamente despues de `new`. No hay palabra clave `constructor`.
- **Sin sobrecarga de operadores** — `+` en objetos no llama a ningun metodo magico.

### Ejemplo completo

```zeta
class BankAccount {
    $owner = "anonimo"
    $balance = 0
    fn deposit($self, $amount) {
        $self.balance = $self.balance + $amount
        return $self.balance
    }
    fn withdraw($self, $amount) {
        if ($amount > $self.balance) {
            return -1
        }
        $self.balance = $self.balance - $amount
        return $self.balance
    }
    fn describe($self) {
        return $self.owner
    }
}

class SavingsAccount extends BankAccount {
    $interest = 0.05
    fn add_interest($self) {
        $interest_amount = $self.balance * $self.interest
        $self.deposit($interest_amount)
        return $self.balance
    }
}

$acc = new SavingsAccount()
$acc.owner = "Ana"
print("owner:", $acc.describe())     # owner: Ana
print("after deposit:", $acc.deposit(100))    # after deposit: 100
print("after interest:", $acc.add_interest())  # after interest: 105
print("balance:", $acc.balance)                 # balance: 105
```
