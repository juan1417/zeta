// test_lib.cpp - Ejemplo de librería nativa para Zeta
// Compilar con: clang++ -std=c++20 -shared -fPIC -fvisibility=hidden
//   -DZETA_BUILDING_DLL -o libtestnative.so test_lib.cpp

#include "zeta/zeta_abi.h"
#include <cmath>
#include <cstring>

extern "C" {

// Doble del valor
ZETA_API double native_double(double x) {
    return x * 2.0;
}

// Suma de todos los elementos (los argumentos vienen aplanados)
ZETA_API double native_sum_vec(int n_args, const double* args) {
    double s = 0.0;
    for (int i = 0; i < n_args; ++i) {
        s += args[i];
    }
    return s;
}

// Producto de todos los elementos
ZETA_API double native_product(int n_args, const double* args) {
    double p = 1.0;
    for (int i = 0; i < n_args; ++i) {
        p *= args[i];
    }
    return p;
}

// Potencia: base^exponente
ZETA_API double native_pow(int n_args, const double* args) {
    if (n_args < 2) return std::nan("");
    return std::pow(args[0], args[1]);
}

// Raíz cuadrada
ZETA_API double native_sqrt(int n_args, const double* args) {
    if (n_args < 1) return std::nan("");
    return std::sqrt(args[0]);
}

// Constante PI
ZETA_API double native_pi(int n_args, const double* args) {
    (void)args;
    (void)n_args;
    return 3.141592653589793;
}

}
