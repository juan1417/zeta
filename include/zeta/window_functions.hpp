#pragma once
#include <vector>
#include <cstddef>

namespace zeta {

// Cumulative functions
std::vector<double> fn_cumsum(const std::vector<double>& vec);
std::vector<double> fn_cummax(const std::vector<double>& vec);
std::vector<double> fn_cummin(const std::vector<double>& vec);

// Rolling window functions
std::vector<double> fn_rolling_mean(const std::vector<double>& vec, size_t window);
std::vector<double> fn_rolling_std(const std::vector<double>& vec, size_t window);
std::vector<double> fn_rolling_sum(const std::vector<double>& vec, size_t window);
std::vector<double> fn_rolling_min(const std::vector<double>& vec, size_t window);
std::vector<double> fn_rolling_max(const std::vector<double>& vec, size_t window);

// Shift functions
std::vector<double> fn_lag(const std::vector<double>& vec, size_t n);
std::vector<double> fn_lead(const std::vector<double>& vec, size_t n);
std::vector<double> fn_diff(const std::vector<double>& vec, size_t n);

// Ranking functions
std::vector<double> fn_row_number(const std::vector<double>& vec);
std::vector<double> fn_rank(const std::vector<double>& vec);

// Change functions
std::vector<double> fn_pct_change(const std::vector<double>& vec, size_t n);

} // namespace zeta
