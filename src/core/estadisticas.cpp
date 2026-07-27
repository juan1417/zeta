#include "zeta/estadisticas.hpp"
#include "zeta/valor_zeta.hpp"
#include <cmath>
#include <algorithm>
#include <map>
#include <set>

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

double fn_median(const std::vector<double>& vec) {
    std::vector<double> clean;
    for (double v : vec) if (!es_null(v)) clean.push_back(v);
    if (clean.empty()) return crear_null();
    std::sort(clean.begin(), clean.end());
    size_t n = clean.size();
    if (n % 2 == 0) return (clean[n/2 - 1] + clean[n/2]) / 2.0;
    return clean[n/2];
}

double fn_percentile(const std::vector<double>& vec, double q) {
    std::vector<double> clean;
    for (double v : vec) if (!es_null(v)) clean.push_back(v);
    if (clean.empty()) return crear_null();
    std::sort(clean.begin(), clean.end());
    double pos = (q / 100.0) * (clean.size() - 1);
    size_t lo = static_cast<size_t>(pos);
    size_t hi = lo + 1;
    if (hi >= clean.size()) return clean.back();
    double frac = pos - lo;
    return clean[lo] + frac * (clean[hi] - clean[lo]);
}

double fn_mode(const std::vector<double>& vec) {
    std::map<double, size_t> freq;
    for (double v : vec) if (!es_null(v)) freq[v]++;
    if (freq.empty()) return crear_null();
    auto best = std::max_element(freq.begin(), freq.end(),
        [](const std::pair<const double, size_t>& a, const std::pair<const double, size_t>& b) { return a.second < b.second; });
    return best->first;
}

double fn_cor(const std::vector<double>& x, const std::vector<double>& y) {
    size_t n = std::min(x.size(), y.size());
    double sx = 0, sy = 0, sxx = 0, syy = 0, sxy = 0;
    size_t count = 0;
    for (size_t i = 0; i < n; ++i) {
        if (es_null(x[i]) || es_null(y[i])) continue;
        sx += x[i]; sy += y[i];
        sxx += x[i]*x[i]; syy += y[i]*y[i];
        sxy += x[i]*y[i]; ++count;
    }
    if (count < 2) return crear_null();
    double num = count * sxy - sx * sy;
    double den = std::sqrt((count*sxx - sx*sx) * (count*syy - sy*sy));
    return den == 0 ? crear_null() : num / den;
}

double fn_cov(const std::vector<double>& x, const std::vector<double>& y) {
    size_t n = std::min(x.size(), y.size());
    double sx = 0, sy = 0, sxy = 0;
    size_t count = 0;
    for (size_t i = 0; i < n; ++i) {
        if (es_null(x[i]) || es_null(y[i])) continue;
        sx += x[i]; sy += y[i]; sxy += x[i]*y[i]; ++count;
    }
    if (count < 2) return crear_null();
    return (sxy - sx*sy/count) / count;
}

} // namespace zeta
