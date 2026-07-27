#pragma once
#include <vector>
#include <cstddef>

namespace zeta {

double fn_mean(const std::vector<double>& vec);
size_t fn_count(const std::vector<double>& vec);
double fn_sum(const std::vector<double>& vec);
double fn_min(const std::vector<double>& vec);
double fn_max(const std::vector<double>& vec);
double fn_stddev(const std::vector<double>& vec);
double fn_median(const std::vector<double>& vec);
double fn_percentile(const std::vector<double>& vec, double q);
double fn_mode(const std::vector<double>& vec);
double fn_cor(const std::vector<double>& x, const std::vector<double>& y);
double fn_cov(const std::vector<double>& x, const std::vector<double>& y);

std::vector<bool> fn_is_null(const std::vector<double>& vec);
std::vector<double> rellenar_nulls_con_media(const std::vector<double>& vec);

} // namespace zeta
