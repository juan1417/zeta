#pragma once

#ifdef _WIN32
    #define ZETA_API __declspec(dllexport)
    #define ZETA_CALL __cdecl
#else
    #define ZETA_API __attribute__((visibility("default")))
    #define ZETA_CALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef double (*zeta_fn_t)(int n_args, const double* args);

#ifdef __cplusplus
}
#endif
