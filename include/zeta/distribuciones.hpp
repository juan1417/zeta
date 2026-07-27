#pragma once
#include <vector>
#include <cmath>

namespace zeta {

// Normal distribution
double dnorm(double x, double mean = 0.0, double sd = 1.0);
double pnorm(double x, double mean = 0.0, double sd = 1.0);
double qnorm(double p, double mean = 0.0, double sd = 1.0);

// Gamma distribution
double dgamma(double x, double shape, double rate);

// Beta distribution
double dbeta(double x, double alpha, double beta);

// Uniform distribution
double dunif(double x, double min = 0.0, double max = 1.0);

// Student's t distribution
double dt_dist(double x, double df);

// F distribution
double df_dist(double x, double df1, double df2);

// Chi-squared distribution
double dchisq(double x, double df);

// Statistical tests
double t_test_statistic(const std::vector<double>& sample1, const std::vector<double>& sample2);
double t_test_pvalue(double t_stat, double df);
double anova_f_statistic(const std::vector<std::vector<double>>& groups);
double chi_square_statistic(const std::vector<double>& observed, const std::vector<double>& expected);

// Regression
struct RegressionResult {
    double slope;
    double intercept;
    double r_squared;
    double std_error;
    std::vector<double> predicted;
};

RegressionResult linear_regression(const std::vector<double>& x, const std::vector<double>& y);

} // namespace zeta
