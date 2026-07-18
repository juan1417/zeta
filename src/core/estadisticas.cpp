#include "zeta/estadisticas.hpp"
#include "zeta/valor_zeta.hpp"
#include <cmath>
#include <algorithm>

namespace zeta {

double fn_mean(const std::vector<double>& vec) {
    double sum = 0.0;
    size_t count = 0;
    for (double v : vec) {
        if (!es_null(v)) { sum += v; ++count; }
    }
    return count > 0 ? sum / count : crear_null();
}

size_t fn_count(const std::vector<double>& vec) {
    size_t count = 0;
    for (double v : vec) {
        if (!es_null(v)) ++count;
    }
    return count;
}

double fn_sum(const std::vector<double>& vec) {
    double sum = 0.0;
    bool any = false;
    for (double v : vec) {
        if (!es_null(v)) { sum += v; any = true; }
    }
    return any ? sum : crear_null();
}

double fn_min(const std::vector<double>& vec) {
    double result = crear_null();
    for (double v : vec) {
        if (!es_null(v)) {
            if (es_null(result) || v < result) result = v;
        }
    }
    return result;
}

double fn_max(const std::vector<double>& vec) {
    double result = crear_null();
    for (double v : vec) {
        if (!es_null(v)) {
            if (es_null(result) || v > result) result = v;
        }
    }
    return result;
}

double fn_stddev(const std::vector<double>& vec) {
    double m = fn_mean(vec);
    if (es_null(m)) return crear_null();
    double sum = 0.0;
    size_t n = 0;
    for (double v : vec) {
        if (!es_null(v)) { sum += (v - m) * (v - m); ++n; }
    }
    return n > 0 ? std::sqrt(sum / n) : crear_null();
}

std::vector<bool> fn_is_null(const std::vector<double>& vec) {
    std::vector<bool> result;
    result.reserve(vec.size());
    for (double v : vec) result.push_back(es_null(v));
    return result;
}

std::vector<double> rellenar_nulls_con_media(const std::vector<double>& vec) {
    double m = fn_mean(vec);
    std::vector<double> result = vec;
    for (auto& v : result) {
        if (es_null(v)) v = m;
    }
    return result;
}

} // namespace zeta
