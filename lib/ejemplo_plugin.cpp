// lib/ejemplo_plugin.cpp
// Ejemplo de plugin v2 para Zeta Language
//
// Compilar:
//   clang++ -std=c++20 -shared -fPIC -fvisibility=hidden \
//       -DZETA_BUILDING_DLL -o libejemplo_plugin.so ejemplo_plugin.cpp
//
// Usar en Zeta:
//   plugin("libejemplo_plugin.so")
//   result = mi_suma(10, 20)
//   result = mi_factorial(5)

#include "zeta/zeta_abi.h"
#include <cmath>
#include <cstring>
#include <cstdio>

extern "C" {

// ============================================================
// Plugin Info (metadata)
// ============================================================

ZETA_API ZetaPluginInfo* zeta_info() {
    static ZetaPluginInfo info = {
        "ejemplo_plugin",
        "1.0.0",
        "Ejemplo de plugin para Zeta con funciones matematicas",
        "Zeta Team"
    };
    return &info;
}

// ============================================================
// v1 Functions (legacy, double only)
// ============================================================

// Simple sum of all arguments
ZETA_API double mi_suma(int n_args, const double* args) {
    double sum = 0.0;
    for (int i = 0; i < n_args; ++i) {
        sum += args[i];
    }
    return sum;
}

// Product of all arguments
ZETA_API double mi_producto(int n_args, const double* args) {
    double product = 1.0;
    for (int i = 0; i < n_args; ++i) {
        product *= args[i];
    }
    return product;
}

// ============================================================
// v2 Functions (extended types)
// ============================================================

// Factorial with extended types
ZETA_API ZetaValue zeta_factorial(int argc, ZetaValue* argv) {
    if (argc < 1 || argv[0].type != ZETA_TYPE_NUM) {
        return ZETA_NULL();
    }
    
    int n = static_cast<int>(argv[0].value.num);
    if (n < 0) return ZETA_NULL();
    
    double result = 1.0;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    
    return ZETA_NUM(result);
}

// GCD (Greatest Common Divisor)
ZETA_API ZetaValue zeta_gcd(int argc, ZetaValue* argv) {
    if (argc < 2 || argv[0].type != ZETA_TYPE_NUM || argv[1].type != ZETA_TYPE_NUM) {
        return ZETA_NULL();
    }
    
    int a = static_cast<int>(argv[0].value.num);
    int b = static_cast<int>(argv[1].value.num);
    
    while (b != 0) {
        int t = b;
        b = a % b;
        a = t;
    }
    
    return ZETA_NUM(static_cast<double>(a));
}

// Check if number is prime
ZETA_API ZetaValue zeta_is_prime(int argc, ZetaValue* argv) {
    if (argc < 1 || argv[0].type != ZETA_TYPE_NUM) {
        return ZETA_BOOL(0);
    }
    
    int n = static_cast<int>(argv[0].value.num);
    if (n < 2) return ZETA_BOOL(0);
    if (n == 2) return ZETA_BOOL(1);
    if (n % 2 == 0) return ZETA_BOOL(0);
    
    for (int i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return ZETA_BOOL(0);
    }
    
    return ZETA_BOOL(1);
}

// Fibonacci (iterative)
ZETA_API ZetaValue zeta_fibonacci(int argc, ZetaValue* argv) {
    if (argc < 1 || argv[0].type != ZETA_TYPE_NUM) {
        return ZETA_NULL();
    }
    
    int n = static_cast<int>(argv[0].value.num);
    if (n < 0) return ZETA_NULL();
    if (n <= 1) return ZETA_NUM(static_cast<double>(n));
    
    double a = 0.0, b = 1.0;
    for (int i = 2; i <= n; ++i) {
        double temp = a + b;
        a = b;
        b = temp;
    }
    
    return ZETA_NUM(b);
}

// Sum of vector elements
ZETA_API ZetaValue zeta_sum_vec(int argc, ZetaValue* argv) {
    if (argc < 1 || argv[0].type != ZETA_TYPE_VEC) {
        return ZETA_NULL();
    }
    
    double sum = 0.0;
    const ZetaValue& vec = argv[0];
    for (int i = 0; i < vec.value.vec.len; ++i) {
        sum += vec.value.vec.data[i];
    }
    
    return ZETA_NUM(sum);
}

// Create greeting string
ZETA_API ZetaValue zeta_greet(int argc, ZetaValue* argv) {
    if (argc < 1 || argv[0].type != ZETA_TYPE_STR) {
        return ZETA_STR("Hello, World!");
    }
    
    // Note: This creates a static string - in production,
    // you'd need proper memory management
    static char buffer[256];
    snprintf(buffer, sizeof(buffer), "Hello, %s!", argv[0].value.str);
    
    return ZETA_STR(buffer);
}

}
