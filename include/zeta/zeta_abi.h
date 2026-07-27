#pragma once

// ============================================================
// ZETA ABI v2 - Plugin System for Zeta Language
// ============================================================
// Use this header to create native plugins for Zeta.
//
// Compile example:
//   clang++ -std=c++20 -shared -fPIC -fvisibility=hidden \
//       -DZETA_BUILDING_DLL -o libmi_plugin.so mi_plugin.cpp
//
// Usage in Zeta:
//   plugin("libmi_plugin.so")
//   result = mi_function(1, 2, 3)
// ============================================================

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

// ============================================================
// ABI Version
// ============================================================
#define ZETA_ABI_VERSION 2

// ============================================================
// Value Types
// ============================================================
typedef enum {
    ZETA_TYPE_NULL = 0,
    ZETA_TYPE_NUM  = 1,
    ZETA_TYPE_STR  = 2,
    ZETA_TYPE_VEC  = 3,
    ZETA_TYPE_BOOL = 4
} ZetaValueType;

// ============================================================
// Extended Value (for v2 ABI)
// ============================================================
typedef struct {
    ZetaValueType type;
    union {
        double num;
        const char* str;
        struct {
            int len;
            double* data;
        } vec;
        int boolean;
    } value;
} ZetaValue;

// ============================================================
// Plugin Info (metadata)
// ============================================================
typedef struct {
    const char* name;
    const char* version;
    const char* description;
    const char* author;
} ZetaPluginInfo;

// ============================================================
// Function Signature Types
// ============================================================

// v1: Simple double functions (legacy)
typedef double (*zeta_fn_v1)(int n_args, const double* args);

// v2: Extended value functions (new)
typedef ZetaValue (*zeta_fn_v2)(int argc, ZetaValue* argv);

// Plugin info function
typedef ZetaPluginInfo* (*zeta_info_fn)();

// ============================================================
// Helper Macros for Plugin Authors
// ============================================================

// Create a num value
#define ZETA_NUM(x) ((ZetaValue){ZETA_TYPE_NUM, {.num = (x)}})

// Create a string value (must remain in scope!)
#define ZETA_STR(s) ((ZetaValue){ZETA_TYPE_STR, {.str = (s)}})

// Create a null value
#define ZETA_NULL() ((ZetaValue){ZETA_TYPE_NULL, {.num = 0}})

// Create a bool value
#define ZETA_BOOL(b) ((ZetaValue){ZETA_TYPE_BOOL, {.boolean = (b)}})

// ============================================================
// Legacy v1 ABI (for backward compatibility)
// ============================================================
typedef double (*zeta_fn_t)(int n_args, const double* args);

#ifdef __cplusplus
}
#endif
