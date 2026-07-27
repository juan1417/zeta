#pragma once
// Valor — Rust-inspired value type for Zeta.
//
// Unlike ValorZeta (shared_ptr<ValorImpl>), Valor is a stack-allocated value type:
// - NUM/BOOL: stored inline (0 heap allocations, 8 bytes)
// - STR/VEC/etc: pointer to arena-allocated ValorImpl (1 arena bump, no refcount)
// - Empty: null value (8 bytes)
//
// Usage:
//   Valor v(42.0);           // inline num, zero heap
//   Valor s("hello");        // arena-allocated string
//   ValorZeta z = v.to_zeta(); // convert to shared_ptr for storage
//
// Design rules (Rust-inspired):
// - Values are MOVED, not copied (use std::move)
// - Arena values live until arena.reset()
// - No reference counting overhead for temporaries
// - Conversion to ValorZeta creates a heap copy (for persistent storage)

#include "zeta/valor_zeta.hpp"
#include "zeta/arena.hpp"
#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <cstring>

namespace zeta {

class Valor {
public:
    // Type tag — determines which union member is active
    enum class Tag : uint8_t {
        EMPTY = 0,
        NUM, BOOL,
        STR, VEC, BOOL_VEC, STR_VEC,
        MATRIZ, DICT, DF, ERR,
        GRAFICO, METRICA, DASHBOARD, SCENE,
        FUNC, OBJ
    };

private:
    Tag tag_ = Tag::EMPTY;

    // Inline storage for small types (no heap allocation)
    union InlineStorage {
        double num_val;
        bool bool_val;

        InlineStorage() : num_val(0.0) {}
        ~InlineStorage() {}
    } inline_;

    // Pointer to arena-allocated ValorImpl (for large types)
    ValorImpl* ptr_ = nullptr;

public:
    // ── Constructors ──────────────────────────────────────────────

    Valor() = default;

    // Inline constructors (zero heap allocation)
    explicit Valor(double v) : tag_(Tag::NUM) { inline_.num_val = v; }
    explicit Valor(bool v) : tag_(Tag::BOOL) { inline_.bool_val = v; }

    // Arena-backed constructor (1 arena bump, no heap)
    explicit Valor(ValorImpl* p)
        : tag_(tag_from_tipo(p->tipo)), ptr_(p) {}

    // Null value
    static Valor null() { return Valor(); }

    // ── Move semantics (Rust-style) ───────────────────────────────

    Valor(Valor&& other) noexcept
        : tag_(other.tag_), inline_(other.inline_), ptr_(other.ptr_) {
        other.tag_ = Tag::EMPTY;
        other.ptr_ = nullptr;
    }

    Valor& operator=(Valor&& other) noexcept {
        if (this != &other) {
            tag_ = other.tag_;
            inline_ = other.inline_;
            ptr_ = other.ptr_;
            other.tag_ = Tag::EMPTY;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    // No copy — use std::move or .clone()
    Valor(const Valor&) = delete;
    Valor& operator=(const Valor&) = delete;

    // ── Accessors ─────────────────────────────────────────────────

    Tag tag() const { return tag_; }
    bool is_empty() const { return tag_ == Tag::EMPTY; }
    bool is_num() const { return tag_ == Tag::NUM; }
    bool is_bool() const { return tag_ == Tag::BOOL; }
    bool is_inline() const { return tag_ == Tag::NUM || tag_ == Tag::BOOL; }

    double get_num() const { return inline_.num_val; }
    bool get_bool() const { return inline_.bool_val; }
    ValorImpl* get_ptr() const { return ptr_; }

    // ── Conversion to ValorZeta (for persistent storage) ──────────
    // This creates a heap copy — use sparingly.

    ValorZeta to_zeta() const {
        switch (tag_) {
            case Tag::EMPTY: return mk_null_val();
            case Tag::NUM:   return mk_num(inline_.num_val);
            case Tag::BOOL:  return mk_bool(inline_.bool_val);
            default:
                // For arena-backed types, create a heap copy
                if (ptr_) {
                    return valor_from_ptr(ptr_);
                }
                return mk_null_val();
        }
    }

    // ── Implicit conversion to ValorZeta ──────────────────────────

    operator ValorZeta() const { return to_zeta(); }

    // ── Comparison ────────────────────────────────────────────────

    bool operator==(const Valor& other) const {
        if (tag_ != other.tag_) return false;
        switch (tag_) {
            case Tag::EMPTY: return true;
            case Tag::NUM:   return inline_.num_val == other.inline_.num_val;
            case Tag::BOOL:  return inline_.bool_val == other.inline_.bool_val;
            default:         return ptr_ == other.ptr_; // pointer equality for large types
        }
    }

    bool operator!=(const Valor& other) const { return !(*this == other); }

private:
    // ── Helpers ───────────────────────────────────────────────────

    static Tag tag_from_tipo(ValorImpl::Tipo t) {
        switch (t) {
            case ValorImpl::NUM:       return Tag::NUM;
            case ValorImpl::BOOL:      return Tag::BOOL;
            case ValorImpl::STR:       return Tag::STR;
            case ValorImpl::VEC:       return Tag::VEC;
            case ValorImpl::BOOL_VEC:  return Tag::BOOL_VEC;
            case ValorImpl::STR_VEC:   return Tag::STR_VEC;
            case ValorImpl::MATRIZ:    return Tag::MATRIZ;
            case ValorImpl::DICT:      return Tag::DICT;
            case ValorImpl::DF:        return Tag::DF;
            case ValorImpl::ERR:       return Tag::ERR;
            case ValorImpl::GRAFICO:   return Tag::GRAFICO;
            case ValorImpl::METRICA:   return Tag::METRICA;
            case ValorImpl::DASHBOARD: return Tag::DASHBOARD;
            case ValorImpl::SCENE:     return Tag::SCENE;
            case ValorImpl::FUNC:      return Tag::FUNC;
            case ValorImpl::OBJ:       return Tag::OBJ;
            default:                   return Tag::EMPTY;
        }
    }

    // Create a heap-allocated ValorZeta from a raw pointer
    // This COPIES the data (doesn't take ownership)
    static ValorZeta valor_from_ptr(const ValorImpl* p) {
        if (!p) return mk_null_val();
        switch (p->tipo) {
            case ValorImpl::NUM:       return mk_num(p->num_val);
            case ValorImpl::BOOL:      return mk_bool(p->bool_val);
            case ValorImpl::STR:       return mk_str(p->str_val);
            case ValorImpl::VEC:       return mk_vec(p->vec_val);
            case ValorImpl::BOOL_VEC:  return mk_bool_vec(p->bool_vec_val);
            case ValorImpl::STR_VEC:   return mk_str_vec(p->str_vec_val);
            case ValorImpl::MATRIZ:    return mk_matriz(p->matriz_val);
            case ValorImpl::DICT:      return mk_dict(p->dict_val);
            case ValorImpl::DF:        return mk_df(p->df_val);
            case ValorImpl::ERR:       return mk_err(p->err_val.tipo, p->err_val.mensaje, p->err_val.linea);
            case ValorImpl::GRAFICO:   return mk_grafico(p->grafico_val);
            case ValorImpl::METRICA:   return mk_metrica(p->metrica_val);
            case ValorImpl::DASHBOARD: return mk_dashboard(p->dashboard_val);
            case ValorImpl::SCENE:     return mk_scene(p->scene_val);
            case ValorImpl::FUNC:      return mk_func(p->func_nombre, p->func_params, p->func_cuerpo, p->func_cierre);
            case ValorImpl::OBJ:       return mk_obj(p->obj_val);
            default:                   return mk_null_val();
        }
    }
};

// ── Arena-backed factory functions (Rust-style) ────────────────────
// These create values in the arena — no heap allocation.
// The returned Valor is a lightweight value type (no refcount).

inline Valor valor_num(double v, Arena& arena) {
    auto* p = arena.create<ValorImpl>();
    p->tipo = ValorImpl::NUM;
    p->num_val = v;
    return Valor(p);
}

inline Valor valor_bool(bool v, Arena& arena) {
    auto* p = arena.create<ValorImpl>();
    p->tipo = ValorImpl::BOOL;
    p->bool_val = v;
    return Valor(p);
}

inline Valor valor_vec(std::vector<double>&& v, Arena& arena) {
    auto* p = arena.create<ValorImpl>();
    p->tipo = ValorImpl::VEC;
    p->vec_val = std::move(v);
    return Valor(p);
}

inline Valor valor_str(std::string&& v, Arena& arena) {
    auto* p = arena.create<ValorImpl>();
    p->tipo = ValorImpl::STR;
    p->str_val = std::move(v);
    return Valor(p);
}

inline Valor valor_dict(std::map<std::string, ValorZeta>&& v, Arena& arena) {
    auto* p = arena.create<ValorImpl>();
    p->tipo = ValorImpl::DICT;
    p->dict_val = std::move(v);
    return Valor(p);
}

inline Valor valor_df(DataFrame&& v, Arena& arena) {
    auto* p = arena.create<ValorImpl>();
    p->tipo = ValorImpl::DF;
    p->df_val = std::move(v);
    return Valor(p);
}

// ── Inline helpers (zero allocation) ───────────────────────────────

inline Valor valor_num_inline(double v) {
    return Valor(v);  // Stored in union, no arena needed
}

inline Valor valor_bool_inline(bool v) {
    return Valor(v);  // Stored in union, no arena needed
}

} // namespace zeta
