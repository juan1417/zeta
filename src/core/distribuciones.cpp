#include "zeta/distribuciones.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>

namespace zeta {

// Helper: log gamma function (Stirling's approximation)
static double lgamma(double x) {
    static const double c[7] = {
        -1.9999999999980918e-14,
        7.6942231582405058e-14,
        -1.3091572429352186e-12,
        2.3141288497699108e-12,
        1.5697740187396035e-8,
        -3.5208956155367005e-7,
        -1.0167883293699487e-4
    };
    static const double p[8] = {
        2.5066282746310005e0,
        76.180091729471461e0,
        -86.505320329416772e0,
        24.014098240830911e0,
        -1.231739572450155e0,
        1.20865097386617926e-3,
        -5.3952393849536836e-6,
        4.8890560841673084e-9
    };
    
    double y = x;
    double tmp = x + 5.5;
    tmp -= (x + 0.5) * std::log(tmp);
    double ser = 1.000000000190015;
    for (int j = 0; j < 7; ++j) ser += c[j] / ++y;
    return -tmp + std::log(2.5066282746310005 * ser / x);
}

// Helper: incomplete beta function (continued fraction)
static double ibeta(double a, double b, double x) {
    if (x < 0.0 || x > 1.0) return 0.0;
    if (x == 0.0 || x == 1.0) return x;
    
    double lbeta = lgamma(a) + lgamma(b) - lgamma(a + b);
    double front = std::exp(std::log(x) * a + std::log(1.0 - x) * b - lbeta);
    
    // Lentz's algorithm
    double f = 1.0, c = 1.0, d = 1.0 - (a + 1) * x / (a + 1);
    if (std::abs(d) < 1e-30) d = 1e-30;
    d = 1.0 / d;
    f = d;
    
    for (int i = 1; i <= 200; ++i) {
        double m = i;
        double an = i * (b - i) * x / ((a + 2*i - 1) * (a + 2*i));
        d = 1.0 + an * d;
        if (std::abs(d) < 1e-30) d = 1e-30;
        c = 1.0 + an / c;
        if (std::abs(c) < 1e-30) c = 1e-30;
        d = 1.0 / d;
        f *= d * c;
        
        an = -(a + m) * (a + b + m) * x / ((a + 2*m) * (a + 2*m + 1));
        d = 1.0 + an * d;
        if (std::abs(d) < 1e-30) d = 1e-30;
        c = 1.0 + an / c;
        if (std::abs(c) < 1e-30) c = 1e-30;
        d = 1.0 / d;
        double delta = d * c;
        f *= delta;
        
        if (std::abs(delta - 1.0) < 1e-10) break;
    }
    
    return front * f / a;
}

// Helper: incomplete beta (regularized)
static double pbeta(double x, double a, double b) {
    if (x < (a + 1) / (a + b + 2)) {
        return ibeta(a, b, x);
    } else {
        return 1.0 - ibeta(b, a, 1.0 - x);
    }
}

// Helper: regularized incomplete gamma
static double pgamma(double x, double a) {
    if (x <= 0.0) return 0.0;
    
    double gla = lgamma(a);
    
    if (x < a + 1.0) {
        // Series
        double sum = 1.0 / a;
        double term = 1.0 / a;
        for (int n = 1; n < 200; ++n) {
            term *= x / (a + n);
            sum += term;
            if (std::abs(term) < 1e-10 * sum) break;
        }
        return sum * std::exp(-x + a * std::log(x) - gla);
    } else {
        // Continued fraction
        double f = 1.0, c = 1.0, d = 1.0 - x / (a + 1);
        if (std::abs(d) < 1e-30) d = 1e-30;
        d = 1.0 / d;
        f = d;
        
        for (int i = 1; i < 200; ++i) {
            double an = -i * (i - a);
            double bn = x + 2.0 * i + 1.0 - a;
            d = bn + an * d;
            if (std::abs(d) < 1e-30) d = 1e-30;
            c = bn + an / c;
            if (std::abs(c) < 1e-30) c = 1e-30;
            d = 1.0 / d;
            double delta = d * c;
            f *= delta;
            
            an = -(a + i) * (a + i + 1);
            bn = x + 2.0 * i + 2.0 - a;
            d = bn + an * d;
            if (std::abs(d) < 1e-30) d = 1e-30;
            c = bn + an / c;
            if (std::abs(c) < 1e-30) c = 1e-30;
            d = 1.0 / d;
            delta = d * c;
            f *= delta;
            
            if (std::abs(delta - 1.0) < 1e-10) break;
        }
        
        return 1.0 - f * std::exp(-x + a * std::log(x) - gla);
    }
}

// Normal distribution
double dnorm(double x, double mean, double sd) {
    if (sd <= 0.0) return 0.0;
    double z = (x - mean) / sd;
    return std::exp(-0.5 * z * z) / (sd * std::sqrt(2.0 * M_PI));
}

double pnorm(double x, double mean, double sd) {
    if (sd <= 0.0) return 0.0;
    double z = (x - mean) / sd;
    return 0.5 * (1.0 + std::erf(z / std::sqrt(2.0)));
}

double qnorm(double p, double mean, double sd) {
    if (p <= 0.0 || p >= 1.0) return mean;
    
    // Rational approximation (Abramowitz and Stegun)
    double t;
    if (p < 0.5) {
        t = std::sqrt(-2.0 * std::log(p));
    } else {
        t = std::sqrt(-2.0 * std::log(1.0 - p));
    }
    
    double c0 = 2.515517;
    double c1 = 0.802853;
    double c2 = 0.010328;
    double d1 = 1.432788;
    double d2 = 0.189269;
    double d3 = 0.001308;
    
    double z = t - (c0 + t * (c1 + t * c2)) / (1.0 + t * (d1 + t * (d2 + t * d3)));
    
    if (p < 0.5) z = -z;
    return mean + sd * z;
}

// Gamma distribution
double dgamma(double x, double shape, double rate) {
    if (x <= 0.0 || shape <= 0.0 || rate <= 0.0) return 0.0;
    return std::pow(rate, shape) * std::exp(-rate * x) * std::pow(x, shape - 1.0) / std::tgamma(shape);
}

// Beta distribution
double dbeta(double x, double alpha, double beta) {
    if (x <= 0.0 || x >= 1.0 || alpha <= 0.0 || beta <= 0.0) return 0.0;
    double gla = lgamma(alpha);
    double glb = lgamma(beta);
    double glab = lgamma(alpha + beta);
    return std::exp((alpha - 1.0) * std::log(x) + (beta - 1.0) * std::log(1.0 - x) - gla - glb + glab);
}

// Uniform distribution
double dunif(double x, double min, double max) {
    if (max <= min) return 0.0;
    return (x >= min && x <= max) ? 1.0 / (max - min) : 0.0;
}

// Student's t distribution
double dt_dist(double x, double df) {
    if (df <= 0.0) return 0.0;
    double coeff = std::tgamma((df + 1.0) / 2.0) / (std::sqrt(df * M_PI) * std::tgamma(df / 2.0));
    return coeff * std::pow(1.0 + x * x / df, -(df + 1.0) / 2.0);
}

// F distribution
double df_dist(double x, double df1, double df2) {
    if (x <= 0.0 || df1 <= 0.0 || df2 <= 0.0) return 0.0;
    double gl1 = lgamma(df1 / 2.0);
    double gl2 = lgamma(df2 / 2.0);
    double glsum = lgamma((df1 + df2) / 2.0);
    
    double num = std::exp(gl1 + gl2 - glsum) * std::pow(df1 / df2, df1 / 2.0) * std::pow(x, df1 / 2.0 - 1.0);
    double den = std::pow(1.0 + df1 * x / df2, (df1 + df2) / 2.0);
    
    return num / den;
}

// Chi-squared distribution
double dchisq(double x, double df) {
    if (x <= 0.0 || df <= 0.0) return 0.0;
    double k2 = df / 2.0;
    return std::exp((k2 - 1.0) * std::log(x) - x / 2.0 - k2 * std::log(2.0) - lgamma(k2));
}

// T-test
double t_test_statistic(const std::vector<double>& sample1, const std::vector<double>& sample2) {
    size_t n1 = sample1.size(), n2 = sample2.size();
    if (n1 < 2 || n2 < 2) return 0.0;
    
    double mean1 = 0, mean2 = 0;
    for (double v : sample1) mean1 += v;
    for (double v : sample2) mean2 += v;
    mean1 /= n1;
    mean2 /= n2;
    
    double var1 = 0, var2 = 0;
    for (double v : sample1) var1 += (v - mean1) * (v - mean1);
    for (double v : sample2) var2 += (v - mean2) * (v - mean2);
    var1 /= (n1 - 1);
    var2 /= (n2 - 1);
    
    double se = std::sqrt(var1 / n1 + var2 / n2);
    if (se == 0) return 0.0;
    
    return (mean1 - mean2) / se;
}

double t_test_pvalue(double t_stat, double df) {
    // Approximate p-value using normal for large df
    if (df > 100) {
        double p = pnorm(std::abs(t_stat));
        return 2.0 * (1.0 - p);
    }
    
    // For smaller df, use t-distribution CDF
    // Two-tailed test
    double p = 0.0;
    double x = t_stat;
    
    // Numerical integration (trapezoidal)
    int n = 1000;
    double a = -10.0, b = x;
    double h = (b - a) / n;
    
    double sum = dt_dist(a, df) + dt_dist(b, df);
    for (int i = 1; i < n; ++i) {
        sum += 2.0 * dt_dist(a + i * h, df);
    }
    p = sum * h / 2.0;
    
    return 2.0 * (1.0 - p);
}

// ANOVA
double anova_f_statistic(const std::vector<std::vector<double>>& groups) {
    size_t k = groups.size();
    if (k < 2) return 0.0;
    
    // Grand mean
    double grand_mean = 0;
    size_t total_n = 0;
    for (const auto& g : groups) {
        for (double v : g) grand_mean += v;
        total_n += g.size();
    }
    grand_mean /= total_n;
    
    // Between-group variance
    double ssb = 0;
    for (const auto& g : groups) {
        double group_mean = 0;
        for (double v : g) group_mean += v;
        group_mean /= g.size();
        ssb += g.size() * (group_mean - grand_mean) * (group_mean - grand_mean);
    }
    
    // Within-group variance
    double ssw = 0;
    for (const auto& g : groups) {
        double group_mean = 0;
        for (double v : g) group_mean += v;
        group_mean /= g.size();
        for (double v : g) ssw += (v - group_mean) * (v - group_mean);
    }
    
    double msb = ssb / (k - 1);
    double msw = ssw / (total_n - k);
    
    if (msw == 0) return 0.0;
    return msb / msw;
}

// Chi-square test
double chi_square_statistic(const std::vector<double>& observed, const std::vector<double>& expected) {
    if (observed.size() != expected.size()) return 0.0;
    
    double chi2 = 0;
    for (size_t i = 0; i < observed.size(); ++i) {
        if (expected[i] > 0) {
            double diff = observed[i] - expected[i];
            chi2 += diff * diff / expected[i];
        }
    }
    return chi2;
}

// Linear regression
RegressionResult linear_regression(const std::vector<double>& x, const std::vector<double>& y) {
    RegressionResult result = {0, 0, 0, 0, {}};
    
    size_t n = std::min(x.size(), y.size());
    if (n < 2) return result;
    
    double sum_x = 0, sum_y = 0, sum_xy = 0, sum_xx = 0;
    for (size_t i = 0; i < n; ++i) {
        sum_x += x[i];
        sum_y += y[i];
        sum_xy += x[i] * y[i];
        sum_xx += x[i] * x[i];
    }
    
    double mean_x = sum_x / n;
    double mean_y = sum_y / n;
    
    double ss_xy = sum_xy - n * mean_x * mean_y;
    double ss_xx = sum_xx - n * mean_x * mean_x;
    
    if (ss_xx == 0) return result;
    
    result.slope = ss_xy / ss_xx;
    result.intercept = mean_y - result.slope * mean_x;
    
    // R-squared
    double ss_total = 0, ss_resid = 0;
    for (size_t i = 0; i < n; ++i) {
        double pred = result.slope * x[i] + result.intercept;
        ss_resid += (y[i] - pred) * (y[i] - pred);
        ss_total += (y[i] - mean_y) * (y[i] - mean_y);
    }
    
    result.r_squared = (ss_total > 0) ? 1.0 - ss_resid / ss_total : 0.0;
    
    // Standard error
    if (n > 2) {
        result.std_error = std::sqrt(ss_resid / (n - 2));
    }
    
    // Predicted values
    result.predicted.resize(n);
    for (size_t i = 0; i < n; ++i) {
        result.predicted[i] = result.slope * x[i] + result.intercept;
    }
    
    return result;
}

} // namespace zeta
