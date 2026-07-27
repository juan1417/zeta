#include "zeta/window_functions.hpp"
#include "zeta/valor_zeta.hpp"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace zeta {

// Cumulative sum
std::vector<double> fn_cumsum(const std::vector<double>& vec) {
    std::vector<double> result;
    result.reserve(vec.size());
    double sum = 0.0;
    for (double v : vec) {
        if (es_null(v)) { result.push_back(crear_null()); continue; }
        sum += v;
        result.push_back(sum);
    }
    return result;
}

// Cumulative max
std::vector<double> fn_cummax(const std::vector<double>& vec) {
    std::vector<double> result;
    result.reserve(vec.size());
    double max_val = crear_null();
    for (double v : vec) {
        if (es_null(v)) { result.push_back(crear_null()); continue; }
        if (es_null(max_val) || v > max_val) max_val = v;
        result.push_back(max_val);
    }
    return result;
}

// Cumulative min
std::vector<double> fn_cummin(const std::vector<double>& vec) {
    std::vector<double> result;
    result.reserve(vec.size());
    double min_val = crear_null();
    for (double v : vec) {
        if (es_null(v)) { result.push_back(crear_null()); continue; }
        if (es_null(min_val) || v < min_val) min_val = v;
        result.push_back(min_val);
    }
    return result;
}

// Rolling mean
std::vector<double> fn_rolling_mean(const std::vector<double>& vec, size_t window) {
    std::vector<double> result;
    result.reserve(vec.size());
    if (window == 0) window = 1;
    
    for (size_t i = 0; i < vec.size(); ++i) {
        size_t start = (i + 1 >= window) ? i + 1 - window : 0;
        double sum = 0.0;
        size_t count = 0;
        bool has_null = false;
        
        for (size_t j = start; j <= i; ++j) {
            if (es_null(vec[j])) { has_null = true; break; }
            sum += vec[j];
            ++count;
        }
        
        if (has_null || count == 0) result.push_back(crear_null());
        else result.push_back(sum / count);
    }
    return result;
}

// Rolling std
std::vector<double> fn_rolling_std(const std::vector<double>& vec, size_t window) {
    std::vector<double> result;
    result.reserve(vec.size());
    if (window == 0) window = 1;
    
    for (size_t i = 0; i < vec.size(); ++i) {
        size_t start = (i + 1 >= window) ? i + 1 - window : 0;
        double sum = 0.0;
        double sum_sq = 0.0;
        size_t count = 0;
        bool has_null = false;
        
        for (size_t j = start; j <= i; ++j) {
            if (es_null(vec[j])) { has_null = true; break; }
            sum += vec[j];
            sum_sq += vec[j] * vec[j];
            ++count;
        }
        
        if (has_null || count < 2) { result.push_back(crear_null()); continue; }
        double mean = sum / count;
        double variance = (sum_sq / count) - (mean * mean);
        result.push_back(variance > 0 ? std::sqrt(variance) : 0.0);
    }
    return result;
}

// Rolling sum
std::vector<double> fn_rolling_sum(const std::vector<double>& vec, size_t window) {
    std::vector<double> result;
    result.reserve(vec.size());
    if (window == 0) window = 1;
    
    for (size_t i = 0; i < vec.size(); ++i) {
        size_t start = (i + 1 >= window) ? i + 1 - window : 0;
        double sum = 0.0;
        bool has_null = false;
        
        for (size_t j = start; j <= i; ++j) {
            if (es_null(vec[j])) { has_null = true; break; }
            sum += vec[j];
        }
        
        result.push_back(has_null ? crear_null() : sum);
    }
    return result;
}

// Rolling min
std::vector<double> fn_rolling_min(const std::vector<double>& vec, size_t window) {
    std::vector<double> result;
    result.reserve(vec.size());
    if (window == 0) window = 1;
    
    for (size_t i = 0; i < vec.size(); ++i) {
        size_t start = (i + 1 >= window) ? i + 1 - window : 0;
        double min_val = crear_null();
        bool has_null = false;
        
        for (size_t j = start; j <= i; ++j) {
            if (es_null(vec[j])) { has_null = true; break; }
            if (es_null(min_val) || vec[j] < min_val) min_val = vec[j];
        }
        
        result.push_back(has_null ? crear_null() : min_val);
    }
    return result;
}

// Rolling max
std::vector<double> fn_rolling_max(const std::vector<double>& vec, size_t window) {
    std::vector<double> result;
    result.reserve(vec.size());
    if (window == 0) window = 1;
    
    for (size_t i = 0; i < vec.size(); ++i) {
        size_t start = (i + 1 >= window) ? i + 1 - window : 0;
        double max_val = crear_null();
        bool has_null = false;
        
        for (size_t j = start; j <= i; ++j) {
            if (es_null(vec[j])) { has_null = true; break; }
            if (es_null(max_val) || vec[j] > max_val) max_val = vec[j];
        }
        
        result.push_back(has_null ? crear_null() : max_val);
    }
    return result;
}

// Lag (previous n values)
std::vector<double> fn_lag(const std::vector<double>& vec, size_t n) {
    std::vector<double> result;
    result.reserve(vec.size());
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i < n) result.push_back(crear_null());
        else result.push_back(vec[i - n]);
    }
    return result;
}

// Lead (next n values)
std::vector<double> fn_lead(const std::vector<double>& vec, size_t n) {
    std::vector<double> result;
    result.reserve(vec.size());
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i + n >= vec.size()) result.push_back(crear_null());
        else result.push_back(vec[i + n]);
    }
    return result;
}

// Diff (difference with previous n)
std::vector<double> fn_diff(const std::vector<double>& vec, size_t n) {
    std::vector<double> result;
    result.reserve(vec.size());
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i < n || es_null(vec[i]) || es_null(vec[i - n])) {
            result.push_back(crear_null());
        } else {
            result.push_back(vec[i] - vec[i - n]);
        }
    }
    return result;
}

// Row number (sequential 1,2,3...)
std::vector<double> fn_row_number(const std::vector<double>& vec) {
    std::vector<double> result;
    result.reserve(vec.size());
    for (size_t i = 0; i < vec.size(); ++i) {
        result.push_back(static_cast<double>(i + 1));
    }
    return result;
}

// Rank (with ties)
std::vector<double> fn_rank(const std::vector<double>& vec) {
    std::vector<double> result(vec.size(), 0.0);
    std::vector<std::pair<double, size_t>> indexed;
    
    for (size_t i = 0; i < vec.size(); ++i) {
        if (!es_null(vec[i])) indexed.push_back({vec[i], i});
    }
    
    std::sort(indexed.begin(), indexed.end());
    
    for (size_t i = 0; i < indexed.size(); ++i) {
        // Handle ties: assign average rank
        size_t j = i;
        while (j < indexed.size() && indexed[j].first == indexed[i].first) ++j;
        double avg_rank = static_cast<double>(i + j + 1) / 2.0; // 1-indexed average
        for (size_t k = i; k < j; ++k) {
            result[indexed[k].second] = avg_rank;
        }
        i = j - 1; // Skip processed ties
    }
    
    return result;
}

// Percent change
std::vector<double> fn_pct_change(const std::vector<double>& vec, size_t n) {
    std::vector<double> result;
    result.reserve(vec.size());
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i < n || es_null(vec[i]) || es_null(vec[i - n]) || vec[i - n] == 0.0) {
            result.push_back(crear_null());
        } else {
            result.push_back((vec[i] - vec[i - n]) / vec[i - n] * 100.0);
        }
    }
    return result;
}

} // namespace zeta
