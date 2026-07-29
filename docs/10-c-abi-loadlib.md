# 10. Carga de Librerías Natives (C ABI)

Zeta puede cargar **librerías compartidas** (`.so` en Linux, `.dll` en Windows, `.dylib` en macOS) con un solo `load_lib(...)` y llamar a funciones C exportadas con la macro `ZETA_API`. Esto es **FFI sin código adicional** — sin `ctypes`, sin `cffi`, sin binding generators.

## Tabla de Contenidos

- [Resumen Rápido](#resumen-rápido)
- [10.1. La Macro `ZETA_API`](#101-la-macro-zeta_api)
- [10.2. Dos Formas de Exportar](#102-dos-formas-de-exportar)
- [10.3. Compilar la Librería](#103-compilar-la-librería)
- [10.4. Cargar y Usar: `load_lib`](#104-cargar-y-usar-load_lib)
- [10.5. Marshalling de Vectores](#105-marshalling-de-vectores)
- [10.6. Manejo de Errores Nativos](#106-manejo-de-errores-nativos)
- [10.7. Ejemplo Completo: `lib/test_lib.cpp`](#107-ejemplo-completo-libtest_libcpp)
- [10.8. Ciclo de Vida de la Librería](#108-ciclo-de-vida-de-la-librería)
- [10.9. Resolución de Rutas](#109-resolución-de-rutas)
- [10.10. Limitaciones](#1010-limitaciones)
- [10.11. Patrones de Uso](#1011-patrones-de-uso)
- [10.12. Por Qué Este Modelo y No ctypes](#1012-por-qué-este-modelo-y-no-ctypes)

---

## Resumen Rápido

| Aspecto | ZETA_API | ctypes (Python) |
|---------|----------|-----------------|
| Overhead por llamada | ~50ns (solo marshalling) | ~500ns (parsing + marshalling) |
| Complejidad del binding | 1 macro + 1 línea de `load_lib` | Definir `argtypes`, `restype` por función |
| Tipos soportados | `double` (nativos) | Cualquiera (pero hay que declarar) |
| Hot paths | Apto | No apto sin `cffi` |
| Compilación | Header + lib compartido | Solo header (FFI declarativa) |

---

## 10.1. La Macro `ZETA_API`

`include/zeta/zeta_abi.h` define:

```c
#ifdef _WIN32
    #define ZETA_API __declspec(dllexport)
    #define ZETA_CALL __cdecl
#else
    #define ZETA_API __attribute__((visibility("default")))
    #define ZETA_CALL
#endif

typedef double (*zeta_fn_t)(int n_args, const double* args);
```

**Toda función exportada debe**:

1. Usar `extern "C"` (para evitar name mangling de C++).
2. Estar marcada con `ZETA_API`.
3. Tener firma `double fn(int n_args, const double* args)` **o** ser una función escalar `double fn(double x)`.

---

## 10.2. Dos Formas de Exportar

### Forma A: función escalar (1 double)

```cpp
extern "C" {
    ZETA_API double native_double(double x) {
        return x * 2.0;
    }
    
    ZETA_API double native_sqrt(double x) {
        return std::sqrt(x);
    }
}
```

`ZETA_API double fn(double x)` — el marshaller pasa el argumento único directamente.

### Forma B: función variádica (n doubles)

```cpp
extern "C" {
    ZETA_API double native_pow(int n_args, const double* args) {
        if (n_args < 2) return std::nan("");
        return std::pow(args[0], args[1]);
    }
    
    ZETA_API double native_sum_vec(int n_args, const double* args) {
        double s = 0.0;
        for (int i = 0; i < n_args; ++i) s += args[i];
        return s;
    }
}
```

`ZETA_API double fn(int n_args, const double* args)` — el marshaller aplana los argumentos a un buffer contiguo.

**El marshaller detecta automáticamente** qué forma usar: si el símbolo en el `.so` coincide con la firma variádica (recibe `(int, const double*)`), se usa esa; si no, se asume la escalar.

---

## 10.3. Compilar la Librería

```bash
clang++ -std=c++20 -shared -fPIC -fvisibility=hidden \
    -I include \
    -o lib/libtestnative.so \
    lib/test_lib.cpp
```

Flags importantes:

| Flag | Descripción |
|------|-------------|
| `-shared` | Produce un `.so` (no un ejecutable) |
| `-fPIC` | Posición-independiente (requerido para `.so`) |
| `-fvisibility=hidden` | Oculta todos los símbolos por defecto |
| `ZETA_API` | En cada función, **reabre** la visibilidad para esa función específica |

---

## 10.4. Cargar y Usar: `load_lib`

```zeta
load_lib("libtestnative.so", {
    "native_double": "fn($x) -> $x",
    "native_pow": "fn($a, $b) -> $a",
    "native_sqrt": "fn($x) -> $x",
    "native_pi": "fn() -> $x"
})
```

| # | Parámetro | Tipo | Descripción |
|---|-----------|------|-------------|
| 1 | `ruta` | string | Ruta al `.so`. Si es relativa, busca en CWD → `./lib/` → `ZETA_PATH` |
| 2 | `dict_funciones` | dict | Mapa de `nombre_símbolo → firma`. La firma es metadata, no se valida en runtime |

### Uso

```zeta
print("native_double(21) =", native_double(21))      # 42
print("native_pow(2, 10) =", native_pow(2, 10))      # 1024
print("native_sqrt(144) =", native_sqrt(144))         # 12
print("native_pi() =", native_pi())                    # 3.14159...
```

Las funciones se exponen en el **ámbito global** después de `load_lib`. No necesitan prefijo de namespace.

---

## 10.5. Marshalling de Vectores

Si pasas un vector Zeta a una función variádica, el marshaller lo **aplana a un buffer de doubles**:

```zeta
$nums = <1, 2, 3, 4, 5>
$resultado = native_sum_vec($nums)    # se convierte a native_sum_vec(5, [1, 2, 3, 4, 5])
print($resultado)    # 15
```

Si pasas un escalar junto con un vector, el orden importa:

```zeta
# native_product(int n, const double* args) hace producto de todos
$base = 2
$nums = <3, 4, 5>
$Resultado = native_product($base, $nums)    # 2 * 3 * 4 * 5 = 120
```

**Importante**: los vectores se pasan por referencia C (puntero al buffer interno). La función no debe **retener** ese puntero después de retornar (el GC de Zeta podría mover el buffer).

---

## 10.6. Manejo de Errores Nativos

Si la función retorna `NaN` (e.g., `sqrt(-1)` en una librería que respeta IEEE 754), Zeta lo trata como `null`:

```cpp
extern "C" {
    ZETA_API double native_log(double x) {
        if (x <= 0) return std::nan("");    // se convierte en null
        return std::log(x);
    }
}
```

```zeta
print(is_null(native_log(-1)))    # true
print(is_null(native_log(0)))     # true
print(native_log(2.718))          # ~1
```

Si quieres retornar errores más ricos, puedes usar una convención propia (e.g., return code en el primer arg), pero el modelo simple es **un double, NaN = error**.

---

## 10.7. Ejemplo Completo: `lib/test_lib.cpp`

```cpp
#include "zeta/zeta_abi.h"
#include <cmath>

extern "C" {

ZETA_API double native_double(double x) {
    return x * 2.0;
}

ZETA_API double native_sum_vec(int n_args, const double* args) {
    double s = 0.0;
    for (int i = 0; i < n_args; ++i) s += args[i];
    return s;
}

ZETA_API double native_product(int n_args, const double* args) {
    double p = 1.0;
    for (int i = 0; i < n_args; ++i) p *= args[i];
    return p;
}

ZETA_API double native_pow(int n_args, const double* args) {
    if (n_args < 2) return std::nan("");
    return std::pow(args[0], args[1]);
}

ZETA_API double native_sqrt(double x) {
    return std::sqrt(x);
}

ZETA_API double native_pi(int n_args, const double* args) {
    (void)args; (void)n_args;
    return 3.141592653589793;
}

}
```

Compilar:

```bash
clang++ -std=c++20 -shared -fPIC -fvisibility=hidden \
    -I include -o lib/libtestnative.so lib/test_lib.cpp
```

Usar:

```zeta
# tests/test_dl.zl
load_lib("libtestnative.so", {
    "native_double": "fn($x) -> $x",
    "native_pow": "fn($a, $b) -> $a",
    "native_sqrt": "fn($x) -> $x",
    "native_pi": "fn() -> $x"
})

print("native_double(21) =", native_double(21))
print("native_pow(2, 10) =", native_pow(2, 10))
print("native_sqrt(144) =", native_sqrt(144))
print("native_pi() =", native_pi())
```

---

## 10.8. Ciclo de Vida de la Librería

Las librerías cargadas se mantienen abiertas durante **toda la vida del proceso**. Esto es intencional: cargarlas y descargarlas es costoso, y `dlclose` puede ser problemático con símbolos estáticos.

`DlRegistry::instancia()` mantiene un `map<string, unique_ptr<DlLibrary>>` con todas las librerías cargadas. Si llamas `load_lib` dos veces con la misma ruta, retorna la misma instancia (no recarga).

---

## 10.9. Resolución de Rutas

`load_lib` busca en este orden:

| Prioridad | Ruta | Descripción |
|-----------|------|-------------|
| 1 | Ruta exacta | Tal como la pasaste (relativa o absoluta) |
| 2 | `./lib/` + ruta | Directorio de librerías local |
| 3 | Cada path en `ZETA_PATH` | Separado por `:` |
| 4 | Error `dl` | Si nada coincide |

```bash
# Configurar ZETA_PATH para librerías compartidas
export ZETA_PATH="/usr/local/lib/zeta:/opt/custom_libs"
./zeta mi_script.zl
```

---

## 10.10. Limitaciones

| Limitación | Descripción |
|------------|-------------|
| Solo números | No hay marshalling de strings, structs, ni DataFrames. Codifícalo en el `double` |
| Sin callbacks | No puedes pasar una función Zeta como puntero C |
| Sin threading | Las llamadas son síncronas en el thread del intérprete |
| Sin gestión de memoria | El caller y el callee comparten el heap del proceso; no hay arenas separadas |

---

## 10.11. Patrones de Uso

### Wrapper de librería externa

```cpp
// lib/optimlib_native.cpp
// Wrapper de OptimLib (https://kthohr.github.io/optim/) para Zeta
#include "zeta/zeta_abi.h"
#include <optim.hpp>

extern "C" {
    ZETA_API double native_rosenbrock(int n_args, const double* args) {
        // args = [x0, x1]
        double x0 = args[0], x1 = args[1];
        return std::pow(1 - x0, 2) + 100 * std::pow(x1 - x0*x0, 2);
    }
    
    ZETA_API double native_gradient(int n_args, const double* args) {
        // Implementación deOptimLib
        // ...
    }
}
```

### Funciones trigonométricas rápidas

```cpp
#include "zeta/zeta_abi.h"
#include <cmath>
extern "C" {
    ZETA_API double fast_sin(double x) { return std::sin(x); }
    ZETA_API double fast_cos(double x) { return std::cos(x); }
    ZETA_API double fast_tan(double x) { return std::tan(x); }
}
```

### Algoritmos numéricos (BLAS, LAPACK)

BLAS/LAPACK tienen interfaz C nativa (`cblas_dgemv`, `LAPACK_dgesv`, etc.). El truco es wrappear las llamadas en funciones con la convención `ZETA_API`:

```cpp
#include "zeta/zeta_abi.h"
extern "C" {
    extern int cblas_dgemv(...);  // de libcblas
    
    ZETA_API double native_matvec(int n_args, const double* args) {
        // args = [m, n, alpha, A_rowmajor, x, beta, y]
        // Implementación que llama a cblas_dgemv
        // ...
    }
}
```

---

## 10.12. Por Qué Este Modelo y No ctypes

**`ZETA_API` es para hot paths numéricos**: algoritmos de BLAS, simulaciones, optimizadores. Para usar una librería Python, abre un subproceso con `subprocess` desde una función nativa.
