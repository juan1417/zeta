#include "builtins.hpp"

namespace zeta_lsp {

BuiltinRegistry::BuiltinRegistry() {
    builtins_ = {
        // Statistics
        {"sum", {{"vec", "vector"}}, "number", "Sum all values in a vector", "sum(<1,2,3>)"},
        {"mean", {{"vec", "vector"}}, "number", "Calculate mean of a vector", "mean(<1,2,3,4,5>)"},
        {"min", {{"vec", "vector"}}, "number", "Find minimum value in a vector", "min(<3,1,2>)"},
        {"max", {{"vec", "vector"}}, "number", "Find maximum value in a vector", "max(<3,1,2>)"},
        {"stddev", {{"vec", "vector"}}, "number", "Calculate standard deviation of a vector", "stddev(<1,2,3,4,5>)"},
        {"count", {{"vec", "vector"}}, "number", "Count non-null values in a vector", "count(<1,2,3>)"},
        {"median", {{"vec", "vector"}}, "number", "Calculate median of a vector", "median(<1,2,3,4,5>)"},
        {"percentile", {{"vec", "vector"}, {"p", "number"}}, "number", "Calculate percentile of a vector", "percentile(<1,2,3,4,5>, 50)"},
        {"mode", {{"vec", "vector"}}, "number", "Find mode (most frequent value) of a vector", "mode(<1,2,2,3>)"},
        {"cor", {{"x", "vector"}, {"y", "vector"}}, "number", "Calculate Pearson correlation between two vectors", "cor(<1,2,3>, <4,5,6>)"},
        {"cov", {{"x", "vector"}, {"y", "vector"}}, "number", "Calculate covariance between two vectors", "cov(<1,2,3>, <4,5,6>)"},

        // Math
        {"abs", {{"x", "number"}}, "number", "Absolute value of a number", "abs(-5)"},
        {"round", {{"x", "number"}}, "number", "Round a number to nearest integer", "round(3.7)"},
        {"floor", {{"x", "number"}}, "number", "Floor a number (round down)", "floor(3.7)"},
        {"ceil", {{"x", "number"}}, "number", "Ceiling a number (round up)", "ceil(3.2)"},
        {"pow", {{"base", "number"}, {"exp", "number"}}, "number", "Raise base to exponent", "pow(2, 3)"},
        {"sqrt", {{"x", "number"}}, "number", "Square root of a number", "sqrt(9)"},
        {"format", {{"x", "number"}, {"decimals", "number"}}, "string", "Format a number with specified decimal places", "format(3.14159, 2)"},

        // Strings
        {"len", {{"s", "any"}}, "number", "Get length of string or vector", "len(\"hello\")"},
        {"upper", {{"s", "string"}}, "string", "Convert string to uppercase", "upper(\"hello\")"},
        {"lower", {{"s", "string"}}, "string", "Convert string to lowercase", "lower(\"HELLO\")"},
        {"substr", {{"s", "string"}, {"start", "number"}, {"end", "number", true}}, "string", "Extract substring from string", "substr(\"hello\", 1, 3)"},
        {"split", {{"s", "string"}, {"delimiter", "string"}}, "vector<string>", "Split string by delimiter", "split(\"a,b,c\", \",\")"},
        {"join", {{"vec", "vector<string>"}, {"separator", "string"}}, "string", "Join vector of strings with separator", "join(<\"a\",\"b\",\"c\">, \",\")"},
        {"replace", {{"s", "string"}, {"old", "string"}, {"new", "string"}}, "string", "Replace occurrences in string", "replace(\"hello world\", \"world\", \"zeta\")"},
        {"find", {{"s", "string"}, {"substr", "string"}}, "number", "Find position of substring in string", "find(\"hello world\", \"world\")"},

        // Vectors
        {"push", {{"vec", "vector"}, {"val", "number"}}, "vector", "Add element to end of vector", "push(<1,2>, 3)"},
        {"reverse", {{"vec", "vector"}}, "vector", "Reverse a vector", "reverse(<1,2,3>)"},
        {"sort", {{"vec", "vector"}}, "vector", "Sort a vector in ascending order", "sort(<3,1,2>)"},
        {"unique", {{"vec", "vector"}}, "vector", "Get unique values from a vector", "unique(<1,2,2,3,3>)"},
        {"range", {{"start", "number"}, {"end", "number", true}, {"step", "number", true}}, "vector", "Generate a sequence of numbers", "range(1, 10, 2)"},

        // Dictionaries
        {"keys", {{"d", "dict"}}, "vector<string>", "Get all keys from a dictionary", "keys({\"a\": 1, \"b\": 2})"},
        {"values", {{"d", "dict"}}, "vector", "Get all values from a dictionary", "values({\"a\": 1, \"b\": 2})"},

        // DataFrames
        {"head", {{"df", "DataFrame|vector"}, {"n", "number", true}}, "DataFrame|vector", "Get first n rows of DataFrame or vector", "head($df, 5)"},
        {"select", {{"df", "DataFrame"}, {"col", "string"}}, "vector", "Select a column from DataFrame", "select($df, \"age\")"},
        {"drop", {{"df", "DataFrame"}, {"col", "string"}}, "DataFrame", "Drop a column from DataFrame", "drop($df, \"id\")"},
        {"drop_nan", {{"df", "DataFrame"}, {"col", "string"}}, "DataFrame", "Drop rows with NaN in specified column", "drop_nan($df, \"age\")"},
        {"group_by", {{"df", "DataFrame"}, {"cols", "vector<string>"}}, "dict", "Group DataFrame by column(s)", "group_by($df, <\"category\">)"},
        {"agg", {{"groups", "dict"}, {"col", "string"}, {"fn", "string"}}, "DataFrame", "Aggregate grouped data with function", "agg($groups, \"sales\", \"sum\")"},
        {"merge", {{"df1", "DataFrame"}, {"df2", "DataFrame"}, {"on", "string"}}, "DataFrame", "Merge two DataFrames on column", "merge($df1, $df2, \"id\")"},

        // Exploration
        {"info", {{"df", "DataFrame"}}, "string", "Get DataFrame information (columns, types, non-null counts)", "info($df)"},
        {"describe", {{"df", "DataFrame"}}, "string", "Get descriptive statistics for DataFrame", "describe($df)"},
        {"tail", {{"df", "DataFrame|vector"}, {"n", "number", true}}, "DataFrame|vector", "Get last n rows of DataFrame or vector", "tail($df, 5)"},
        {"sample", {{"df", "DataFrame"}, {"n", "number", true}}, "DataFrame", "Random sample of n rows from DataFrame", "sample($df, 100)"},
        {"value_counts", {{"vec", "vector"}}, "DataFrame", "Count occurrences of each value in vector", "value_counts(<1,1,2,3,3,3>)"},
        {"nunique", {{"vec", "vector"}}, "number", "Count number of unique values in vector", "nunique(<1,2,2,3>)"},
        {"isna", {{"df", "DataFrame|vector"}}, "DataFrame|vector", "Check for NaN values in DataFrame or vector", "isna($df)"},
        {"duplicated", {{"vec", "vector"}}, "vector", "Find duplicate values in vector", "duplicated(<1,2,2,3>)"},
        {"cut", {{"vec", "vector"}, {"bins", "number"}}, "vector", "Bin continuous values into discrete intervals", "cut(<1,2,3,4,5>, 3)"},
        {"qcut", {{"vec", "vector"}, {"quantiles", "number"}}, "vector", "Bin continuous values into quantile-based intervals", "qcut(<1,2,3,4,5>, 4)"},

        // Distributions
        {"dnorm", {{"x", "number"}, {"mean", "number", true}, {"sd", "number", true}}, "number", "Normal distribution density function", "dnorm(0)"},
        {"pnorm", {{"x", "number"}, {"mean", "number", true}, {"sd", "number", true}}, "number", "Normal distribution cumulative distribution function", "pnorm(1.96)"},
        {"qnorm", {{"p", "number"}, {"mean", "number", true}, {"sd", "number", true}}, "number", "Normal distribution quantile function (inverse CDF)", "qnorm(0.975)"},
        {"dgamma", {{"x", "number"}, {"shape", "number"}, {"rate", "number"}}, "number", "Gamma distribution density function", "dgamma(1, 2, 1)"},
        {"dbeta", {{"x", "number"}, {"alpha", "number"}, {"beta", "number"}}, "number", "Beta distribution density function", "dbeta(0.5, 2, 2)"},
        {"dunif", {{"x", "number"}, {"min", "number", true}, {"max", "number", true}}, "number", "Uniform distribution density function", "dunif(0.5)"},
        {"dt_dist", {{"x", "number"}, {"df", "number"}}, "number", "Student's t-distribution density function", "dt_dist(0, 10)"},
        {"df_dist", {{"x", "number"}, {"df1", "number"}, {"df2", "number"}}, "number", "F-distribution density function", "df_dist(1, 5, 10)"},
        {"dchisq", {{"x", "number"}, {"df", "number"}}, "number", "Chi-squared distribution density function", "dchisq(2, 3)"},

        // Statistical Tests
        {"t_test", {{"x", "vector"}, {"y", "vector"}}, "dict", "Two-sample t-test", "t_test(<1,2,3>, <4,5,6>)"},
        {"anova", {{"x", "vector"}, {"y", "vector"}, {"rest", "vector", true}}, "dict", "One-way ANOVA test", "anova(<1,2>, <3,4>, <5,6>)"},
        {"chi_square", {{"observed", "vector"}, {"expected", "vector"}}, "dict", "Chi-squared test of independence", "chi_square(<10,20>, <15,15>)"},

        // Regression
        {"linear_regression", {{"x", "vector"}, {"y", "vector"}}, "dict", "Simple linear regression", "linear_regression(<1,2,3>, <2,4,5>)"},

        // Window Functions
        {"cumsum", {{"vec", "vector"}}, "vector", "Cumulative sum of vector", "cumsum(<1,2,3>)"},
        {"cummax", {{"vec", "vector"}}, "vector", "Cumulative maximum of vector", "cummax(<3,1,4,1,5>)"},
        {"cummin", {{"vec", "vector"}}, "vector", "Cumulative minimum of vector", "cummin(<3,1,4,1,5>)"},
        {"rolling_mean", {{"vec", "vector"}, {"window", "number"}}, "vector", "Rolling mean over a window", "rolling_mean(<1,2,3,4,5>, 3)"},
        {"rolling_std", {{"vec", "vector"}, {"window", "number"}}, "vector", "Rolling standard deviation over a window", "rolling_std(<1,2,3,4,5>, 3)"},
        {"rolling_sum", {{"vec", "vector"}, {"window", "number"}}, "vector", "Rolling sum over a window", "rolling_sum(<1,2,3,4,5>, 3)"},
        {"rolling_min", {{"vec", "vector"}, {"window", "number"}}, "vector", "Rolling minimum over a window", "rolling_min(<3,1,4,1,5>, 3)"},
        {"rolling_max", {{"vec", "vector"}, {"window", "number"}}, "vector", "Rolling maximum over a window", "rolling_max(<3,1,4,1,5>, 3)"},
        {"lag", {{"vec", "vector"}, {"n", "number"}}, "vector", "Shift values forward by n positions", "lag(<1,2,3,4>, 1)"},
        {"lead", {{"vec", "vector"}, {"n", "number"}}, "vector", "Shift values backward by n positions", "lead(<1,2,3,4>, 1)"},
        {"diff", {{"vec", "vector"}, {"n", "number", true}}, "vector", "Difference between consecutive values", "diff(<1,3,6,10>)"},
        {"row_number", {{"vec", "vector"}}, "vector", "Assign row numbers to vector", "row_number(<5,3,1,4>)"},
        {"rank", {{"vec", "vector"}}, "vector", "Assign ranks to values in vector", "rank(<5,3,1,4>)"},
        {"pct_change", {{"vec", "vector"}, {"n", "number", true}}, "vector", "Percentage change between values", "pct_change(<100,110,105>)"},

        // Data Cleaning
        {"drop_duplicates", {{"df", "DataFrame"}}, "DataFrame", "Remove duplicate rows from DataFrame", "drop_duplicates($df)"},
        {"rename", {{"df", "DataFrame"}, {"old", "string"}, {"new", "string"}}, "DataFrame", "Rename a column in DataFrame", "rename($df, \"old_name\", \"new_name\")"},
        {"select_cols", {{"df", "DataFrame"}, {"cols", "vector<string>"}}, "DataFrame", "Select specific columns from DataFrame", "select_cols($df, <\"name\",\"age\">)"},
        {"drop_cols", {{"df", "DataFrame"}, {"cols", "vector<string>"}}, "DataFrame", "Drop multiple columns from DataFrame", "drop_cols($df, <\"id\",\"temp\">)"},
        {"fillna", {{"vec", "vector"}, {"val", "string|number"}}, "vector", "Fill NaN values with a value", "fillna(<1, NaN, 3>, 0)"},
        {"replace_val", {{"df", "DataFrame"}, {"col", "string"}, {"old", "string"}, {"new", "string"}}, "DataFrame", "Replace values in DataFrame column", "replace_val($df, \"status\", \"old\", \"new\")"},
        {"clip", {{"vec", "vector"}, {"min", "number"}, {"max", "number"}}, "vector", "Clip values to a range", "clip(<1,5,10>, 2, 8)"},
        {"trim", {{"vec", "vector"}, {"percentile", "number"}}, "vector", "Trim outliers by percentiles", "trim(<1,2,3,100>, 10)"},
        {"normalize", {{"vec", "vector"}}, "vector", "Normalize vector to [0,1] range", "normalize(<1,2,3,4,5>)"},
        {"standardize", {{"vec", "vector"}}, "vector", "Standardize vector to z-scores (mean=0, sd=1)", "standardize(<1,2,3,4,5>)"},

        // Input/Output
        {"load_csv", {{"path", "string"}, {"sep", "string", true}}, "DataFrame", "Load CSV file into DataFrame", "load_csv(\"data.csv\")"},
        {"load_json", {{"path", "string"}}, "DataFrame", "Load JSON file into DataFrame", "load_json(\"data.json\")"},
        {"load_xlsx", {{"path", "string"}, {"sheet", "number", true}}, "DataFrame", "Load Excel file into DataFrame", "load_xlsx(\"data.xlsx\", 0)"},
        {"save_csv", {{"path", "string"}, {"df", "DataFrame"}, {"sep", "string", true}}, "null", "Save DataFrame to CSV file", "save_csv(\"output.csv\", $df)"},
        {"save_xlsx", {{"path", "string"}, {"df", "DataFrame"}}, "null", "Save DataFrame to Excel file", "save_xlsx(\"output.xlsx\", $df)"},

        // Visualization
        {"scene", {{"title", "string"}, {"themes", "vector<string>", true}}, "scene", "Create a new visualization scene", "scene(\"My Dashboard\")"},
        {"add_metric", {{"label", "string"}, {"value", "number"}, {"unit", "string", true}, {"color", "string", true}}, "null", "Add a metric card to the scene", "add_metric(\"Revenue\", 50000, \"$\")"},
        {"add_line_plot", {{"df", "DataFrame"}, {"x", "string"}, {"y", "string"}, {"title", "string"}}, "null", "Add a line plot to the scene", "add_line_plot($df, \"date\", \"sales\", \"Sales Over Time\")"},
        {"add_bar_chart", {{"df", "DataFrame"}, {"x", "string"}, {"y", "string"}, {"title", "string"}}, "null", "Add a bar chart to the scene", "add_bar_chart($df, \"category\", \"count\", \"Distribution\")"},
        {"add_scatter", {{"df", "DataFrame"}, {"x", "string"}, {"y", "string"}, {"title", "string"}}, "null", "Add a scatter plot to the scene", "add_scatter($df, \"height\", \"weight\", \"Correlation\")"},
        {"add_histogram", {{"df", "DataFrame"}, {"col", "string"}, {"title", "string"}, {"bins", "number", true}}, "null", "Add a histogram to the scene", "add_histogram($df, \"age\", \"Age Distribution\", 20)"},
        {"add_box_plot", {{"df", "DataFrame"}, {"col", "string"}, {"title", "string"}}, "null", "Add a box plot to the scene", "add_box_plot($df, \"score\", \"Score Distribution\")"},
        {"add_linear_regression", {{"df", "DataFrame"}, {"x", "string"}, {"y", "string"}, {"title", "string"}}, "null", "Add a scatter plot with linear regression line", "add_linear_regression($df, \"x\", \"y\", \"Regression\")"},

        // Error Handling
        {"mk_err", {{"code", "string"}, {"msg", "string"}}, "error", "Create an error value", "mk_err(\"E001\", \"Invalid input\")"},
        {"mk_null_val", {}, "null", "Create a null value", "mk_null_val()"},

        // Type Checking
        {"type", {{"val", "any"}}, "string", "Get type name of a value", "type(42)"},
        {"is_null", {{"val", "any"}}, "bool", "Check if a value is null", "is_null(null)"},
        {"is_error", {{"val", "any"}}, "bool", "Check if a value is an error", "is_error(mk_err(\"E001\", \"error\"))"},

        // Functional
        {"map", {{"vec", "vector"}, {"fn", "function"}}, "vector", "Apply function to each element of vector", "map(<1,2,3>, $x -> $x * 2)"},
        {"filter", {{"vec", "vector"}, {"fn", "function"}}, "vector", "Filter vector elements by predicate", "filter(<1,2,3,4>, $x -> $x > 2)"},
        {"reduce", {{"vec", "vector"}, {"fn", "function"}, {"init", "any"}}, "any", "Reduce vector to single value", "reduce(<1,2,3>, $a $b -> $a + $b, 0)"},

        // Matrix
        {"transpose", {{"m", "matrix"}}, "matrix", "Transpose a matrix", "transpose(<<1,2>,<3,4>>>)"},
        {"dot", {{"a", "vector"}, {"b", "vector"}}, "number", "Dot product of two vectors", "dot(<1,2>, <3,4>)"},

        // Foreign Function Interface
        {"load_lib", {{"path", "string"}, {"config", "dict"}}, "null", "Load a native plugin library", "load_lib(\"my_plugin.so\", {})"},
        {"plugin", {{"name", "string"}}, "bool", "Check if a plugin is loaded", "plugin(\"my_plugin\")"},
        {"plugin_info", {{"name", "string"}}, "dict", "Get information about a loaded plugin", "plugin_info(\"my_plugin\")"},

        // Memory Management
        {"clear_arena", {}, "null", "Reset arena allocator (bulk free all arena memory)", "clear_arena()"},
        {"arena_bytes", {}, "number", "Query current arena allocation size in bytes", "arena_bytes()"},

        // System
        {"zeta_version", {}, "string", "Get Zeta runtime version string", "zeta_version()"},
        {"print", {{"values", "any..."}}, "null", "Print values to stdout", "print(\"Hello\", 42)"},
        {"time", {}, "number", "Get current Unix timestamp in seconds", "time()"},
        {"route", {{"method", "string"}, {"path", "string"}, {"handler", "function"}}, "null", "Define an HTTP route handler for the server", "route(\"GET\", \"/api/data\", $req -> $data)"},

    };

    for (auto& b : builtins_) {
        by_name_[b.name] = &b;
    }
}

const std::vector<BuiltinInfo>& BuiltinRegistry::search(const std::string& prefix) const {
    static std::vector<BuiltinInfo> results;
    results.clear();
    for (auto& b : builtins_) {
        if (b.name.substr(0, prefix.size()) == prefix) {
            results.push_back(b);
        }
    }
    return results;
}

const BuiltinInfo* BuiltinRegistry::find(const std::string& name) const {
    auto it = by_name_.find(name);
    return it != by_name_.end() ? it->second : nullptr;
}

} // namespace zeta_lsp
