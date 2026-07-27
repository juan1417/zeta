#include "builtins.hpp"

namespace zeta_lsp {

BuiltinRegistry::BuiltinRegistry() {
    builtins_ = {
        // Null/Error checks
        {"is_null", {{"x", "any"}}, "bool", "Check if a value is null (NaN)", "$x | is_null()"},
        {"fill_null", {{"val", "any"}, {"default", "any"}}, "any", "Replace null values with a default", "$data | fill_null(0)"},
        {"is_error", {{"x", "any"}}, "bool", "Check if a value is an error", "$result | is_error()"},

        // Statistics
        {"mean", {{"data", "Collection"}}, "number", "Arithmetic mean of a collection", "$df:salary | mean()"},
        {"count", {{"data", "Collection"}}, "number", "Count non-null elements", "$df:name | count()"},
        {"sum", {{"data", "Collection"}}, "number", "Sum all values", "$df:sales | sum()"},
        {"min", {{"data", "Collection"}}, "number", "Minimum value", "$df:score | min()"},
        {"max", {{"data", "Collection"}}, "number", "Maximum value", "$df:score | max()"},
        {"stddev", {{"data", "Collection"}}, "number", "Standard deviation", "$df:salary | stddev()"},

        // Math
        {"abs", {{"x", "number"}}, "number", "Absolute value", "abs(-5)"},
        {"round", {{"x", "number"}}, "number", "Round to nearest integer", "round(3.7)"},
        {"floor", {{"x", "number"}}, "number", "Round down", "floor(3.7)"},
        {"ceil", {{"x", "number"}}, "number", "Round up", "ceil(3.2)"},
        {"pow", {{"base", "number"}, {"exp", "number"}}, "number", "Power", "pow(2, 3)"},
        {"sqrt", {{"x", "number"}}, "number", "Square root", "sqrt(16)"},
        {"format", {{"x", "number"}, {"decimals", "number"}}, "string", "Format number with decimals", "format(3.14159, 2)"},

        // Time
        {"time", {}, "number", "Current time in seconds", "time()"},

        // String
        {"len", {{"x", "string|vector|dict"}}, "number", "Length of string, vector, or dict", "len($name)"},
        {"upper", {{"s", "string"}}, "string", "Uppercase string", "upper($name)"},
        {"lower", {{"s", "string"}}, "string", "Lowercase string", "lower($name)"},
        {"substr", {{"s", "string"}, {"start", "number"}, {"len", "number", true}}, "string", "Extract substring", "substr($name, 0, 3)"},
        {"split", {{"s", "string"}, {"sep", "string"}}, "vector<string>", "Split string by separator", "split($csv_line, \",\")"},
        {"join", {{"vec", "vector<string>"}, {"sep", "string"}}, "string", "Join strings with separator", "join($words, \" \")"},
        {"replace", {{"s", "string"}, {"old", "string"}, {"new", "string"}}, "string", "Replace substring", "replace($text, \"a\", \"b\")"},
        {"find", {{"s", "string"}, {"sub", "string"}}, "number", "Find position of substring (or -1)", "find($text, \"hello\")"},

        // Vector
        {"reverse", {{"vec", "vector"}}, "vector", "Reverse a vector", "reverse($data)"},
        {"sort", {{"vec", "vector"}}, "vector", "Sort a vector", "sort($scores)"},
        {"unique", {{"vec", "vector"}}, "vector", "Remove duplicates", "unique($tags)"},
        {"push", {{"vec", "vector"}, {"val", "number"}}, "vector", "Append element to vector", "push($list, 42)"},
        {"range", {{"start", "number"}, {"end", "number", true}, {"step", "number", true}}, "vector", "Generate numeric range", "range(1, 10)"},
        {"map", {{"vec", "vector"}, {"fn", "function"}}, "vector", "Apply function to each element", "map($data, fn($x) $x * 2)"},
        {"filter", {{"vec", "vector"}, {"fn", "function"}}, "vector", "Filter elements by predicate", "filter($data, fn($x) $x > 0)"},
        {"reduce", {{"vec", "vector"}, {"fn", "function"}, {"init", "any"}}, "any", "Reduce vector to single value", "reduce($data, fn($a, $b) $a + $b, 0)"},

        // Dict
        {"keys", {{"d", "dict"}}, "vector<string>", "Get dictionary keys", "keys($config)"},
        {"values", {{"d", "dict"}}, "vector", "Get dictionary values", "values($config)"},

        // Type
        {"type", {{"x", "any"}}, "string", "Get type name of a value", "type($data)"},

        // Matrix
        {"transpose", {{"m", "matrix"}}, "matrix", "Transpose a matrix", "transpose($m)"},
        {"dot", {{"a", "vector"}, {"b", "vector"}}, "number", "Dot product of two vectors", "dot($x, $y)"},

        // DataFrame
        {"head", {{"df", "DataFrame|vector"}, {"n", "number", true}}, "DataFrame|vector", "First N elements (default 5)", "head($df)"},
        {"select", {{"df", "DataFrame"}, {"col", "string"}}, "vector", "Select a column from DataFrame", "select($df, \"name\")"},
        {"drop", {{"df", "DataFrame"}, {"col", "string"}}, "DataFrame", "Drop a column from DataFrame", "drop($df, \"id\")"},
        {"drop_nan", {{"df", "DataFrame"}, {"col", "string"}}, "DataFrame", "Drop rows with null in column", "drop_nan($df, \"age\")"},
        {"group_by", {{"df", "DataFrame"}, {"col", "string", true}}, "dict", "Group DataFrame by column(s)", "group_by($df, \"region\")"},
        {"agg", {{"grouped", "dict"}, {"col", "string"}, {"func", "string"}}, "DataFrame", "Aggregate grouped data", "agg($grouped, \"sales\", \"sum\")"},
        {"merge", {{"left", "DataFrame"}, {"right", "DataFrame"}, {"on", "string"}}, "DataFrame", "Inner join two DataFrames", "merge($a, $b, \"id\")"},

        // Exploration / Data Quality
        {"info", {{"df", "DataFrame"}}, "string", "Show DataFrame info: columns, types, nulls", "info($df)"},
        {"describe", {{"df", "DataFrame"}}, "string", "Statistical summary of numeric columns", "describe($df)"},
        {"tail", {{"df", "DataFrame|vector"}, {"n", "number", true}}, "DataFrame|vector", "Last N elements (default 5)", "tail($df)"},
        {"sample", {{"df", "DataFrame"}, {"n", "number", true}}, "DataFrame", "Random sample of N rows (default 5)", "sample($df)"},
        {"value_counts", {{"vec", "vector|vector<string>"}}, "DataFrame", "Frequency counts of each value", "value_counts($df:col)"},
        {"nunique", {{"vec", "vector|vector<string>"}}, "number", "Count of unique values", "nunique($df:col)"},
        {"median", {{"data", "vector"}}, "number", "Median value", "$df:salary | median()"},
        {"percentile", {{"data", "vector"}, {"q", "number"}}, "number", "Percentile (0-100)", "percentile($scores, 75)"},
        {"mode", {{"data", "vector"}}, "number", "Most frequent value", "mode($df:dept)"},
        {"cor", {{"x", "vector"}, {"y", "vector"}}, "number", "Pearson correlation", "cor($df:age, $df:score)"},
        {"cov", {{"x", "vector"}, {"y", "vector"}}, "number", "Covariance", "cov($df:x, $df:y)"},
        {"isna", {{"df", "DataFrame|vector"}}, "DataFrame|vector<bool>", "True where null", "isna($df)"},
        {"duplicated", {{"vec", "vector|vector<string>"}}, "vector<bool>", "True where duplicate", "duplicated($df:id)"},
        {"cut", {{"vec", "vector"}, {"bins", "number"}}, "vector", "Discretize into N bins", "cut($df:age, 5)"},
        {"qcut", {{"vec", "vector"}, {"q", "number"}}, "vector", "Discretize into q quantiles", "qcut($df:age, 4)"},

        // Data Cleaning
        {"drop_duplicates", {{"df", "DataFrame"}}, "DataFrame", "Remove duplicate rows", "drop_duplicates($df)"},
        {"rename", {{"df", "DataFrame"}, {"old", "string"}, {"new", "string"}}, "DataFrame", "Rename column", "rename($df, \"old\", \"new\")"},
        {"select_cols", {{"df", "DataFrame"}, {"cols", "vector<string>"}}, "DataFrame", "Select multiple columns", "select_cols($df, <\"a\",\"b\">)"},
        {"drop_cols", {{"df", "DataFrame"}, {"cols", "vector<string>"}}, "DataFrame", "Drop multiple columns", "drop_cols($df, <\"a\",\"b\">)"},
        {"fillna", {{"vec", "vector"}, {"strategy", "string|number"}}, "vector", "Fill nulls (mean/median/mode/ffill/bfill/zero/value)", "fillna($col, \"mean\")"},
        {"replace_val", {{"df", "DataFrame"}, {"col", "string"}, {"old", "string"}, {"new", "string"}}, "DataFrame", "Replace values in column", "replace_val($df, \"dept\", \"IT\", \"Tech\")"},
        {"clip", {{"vec", "vector"}, {"min", "number"}, {"max", "number"}}, "vector", "Clip values to range", "clip($scores, 0, 100)"},
        {"trim", {{"vec", "vector"}, {"std", "number"}}, "vector", "Remove outliers by std dev", "trim($scores, 3)"},
        {"normalize", {{"vec", "vector"}}, "vector", "Normalize to 0-1 range", "normalize($scores)"},
        {"standardize", {{"vec", "vector"}}, "vector", "Standardize to z-score", "standardize($scores)"},

        // Distributions
        {"dnorm", {{"x", "number"}, {"mean", "number", true}, {"sd", "number", true}}, "number", "Normal density", "dnorm(0)"},
        {"pnorm", {{"x", "number"}, {"mean", "number", true}, {"sd", "number", true}}, "number", "Normal CDF", "pnorm(1.96)"},
        {"qnorm", {{"p", "number"}, {"mean", "number", true}, {"sd", "number", true}}, "number", "Normal quantile", "qnorm(0.975)"},
        {"dgamma", {{"x", "number"}, {"shape", "number"}, {"rate", "number"}}, "number", "Gamma density", "dgamma(1, 2, 1)"},
        {"dbeta", {{"x", "number"}, {"alpha", "number"}, {"beta", "number"}}, "number", "Beta density", "dbeta(0.5, 2, 2)"},
        {"dunif", {{"x", "number"}, {"min", "number", true}, {"max", "number", true}}, "number", "Uniform density", "dunif(0.5)"},
        {"dt_dist", {{"x", "number"}, {"df", "number"}}, "number", "Student's t density", "dt_dist(0, 10)"},
        {"df_dist", {{"x", "number"}, {"df1", "number"}, {"df2", "number"}}, "number", "F density", "df_dist(1, 5, 10)"},
        {"dchisq", {{"x", "number"}, {"df", "number"}}, "number", "Chi-squared density", "dchisq(2, 3)"},

        // Statistical Tests
        {"t_test", {{"sample1", "vector"}, {"sample2", "vector"}}, "dict", "Two-sample t-test", "t_test($a, $b)"},
        {"anova", {{"group1", "vector"}, {"group2", "vector"}, {"rest", "vector", true}}, "dict", "One-way ANOVA", "anova($g1, $g2, $g3)"},
        {"chi_square", {{"observed", "vector"}, {"expected", "vector"}}, "dict", "Chi-squared test", "chi_square($obs, $exp)"},

        // Regression
        {"linear_regression", {{"x", "vector"}, {"y", "vector"}}, "dict", "Simple linear regression", "linear_regression($x, $y)"},

        // Window Functions
        {"cumsum", {{"vec", "vector"}}, "vector", "Cumulative sum", "cumsum($sales)"},
        {"cummax", {{"vec", "vector"}}, "vector", "Cumulative max", "cummax($temps)"},
        {"cummin", {{"vec", "vector"}}, "vector", "Cumulative min", "cummin($temps)"},
        {"rolling_mean", {{"vec", "vector"}, {"window", "number"}}, "vector", "Rolling mean", "rolling_mean($sales, 7)"},
        {"rolling_std", {{"vec", "vector"}, {"window", "number"}}, "vector", "Rolling std dev", "rolling_std($sales, 7)"},
        {"rolling_sum", {{"vec", "vector"}, {"window", "number"}}, "vector", "Rolling sum", "rolling_sum($sales, 7)"},
        {"rolling_min", {{"vec", "vector"}, {"window", "number"}}, "vector", "Rolling min", "rolling_min($temps, 24)"},
        {"rolling_max", {{"vec", "vector"}, {"window", "number"}}, "vector", "Rolling max", "rolling_max($temps, 24)"},
        {"lag", {{"vec", "vector"}, {"n", "number"}}, "vector", "Previous n values", "lag($sales, 1)"},
        {"lead", {{"vec", "vector"}, {"n", "number"}}, "vector", "Next n values", "lead($sales, 1)"},
        {"diff", {{"vec", "vector"}, {"n", "number"}}, "vector", "Difference with previous n", "diff($sales, 1)"},
        {"row_number", {{"vec", "vector"}}, "vector", "Sequential row number (1,2,3...)", "row_number($sales)"},
        {"rank", {{"vec", "vector"}}, "vector", "Rank with ties", "rank($scores)"},
        {"pct_change", {{"vec", "vector"}, {"n", "number"}}, "vector", "Percent change vs previous n", "pct_change($sales, 1)"},

        // I/O
        {"load_csv", {{"path", "string"}, {"delim", "string", true}}, "DataFrame", "Load CSV file into DataFrame", "load_csv(\"data.csv\")"},
        {"load_json", {{"path", "string"}}, "DataFrame", "Load JSON array as DataFrame", "load_json(\"data.json\")"},
        {"load_xlsx", {{"path", "string"}, {"sheet", "number", true}}, "DataFrame", "Load Excel file", "load_xlsx(\"data.xlsx\")"},
        {"save_csv", {{"path", "string"}, {"df", "DataFrame"}, {"delim", "string", true}}, "null", "Save DataFrame as CSV", "save_csv(\"out.csv\", $df)"},
        {"save_xlsx", {{"path", "string"}, {"df", "DataFrame"}}, "null", "Save DataFrame as Excel", "save_xlsx(\"out.xlsx\", $df)"},

        // Scene/Graph
        {"guardar_grafo", {{"path", "string"}}, "null", "Save current scene to JSON file", "guardar_grafo(\"scene.json\")"},
        {"cargar_grafo", {{"path", "string"}}, "scene", "Load scene from JSON file", "cargar_grafo(\"scene.json\")"},
        {"grafo_actual", {}, "scene", "Get the current active scene", "grafo_actual()"},

        // Errors
        {"mk_err", {{"type", "string"}, {"message", "string"}}, "error", "Create an error value", "mk_err(\"runtime\", \"something failed\")"},
        {"mk_null_val", {}, "null", "Create an explicit null value", "mk_null_val()"},

        // FFI
        {"load_lib", {{"path", "string"}, {"funcs", "dict"}}, "null", "Load native library (.so/.dll)", "load_lib(\"lib.so\", {\"fn\": \"fn($a) -> $a\"})"},
        {"plugin", {{"path", "string"}, {"funcs", "vector<string>", true}}, "string", "Load plugin with v2 ABI", "plugin(\"lib.so\")"},
        {"plugin_info", {{"path", "string"}}, "dict", "Get plugin metadata", "plugin_info(\"lib.so\")"},

        // Memory management (Arena allocator)
        {"clear_arena", {}, "null", "Reset arena allocator — frees all temporary values at once", "clear_arena()"},
        {"arena_bytes", {}, "number", "Return bytes currently allocated in the arena", "arena_bytes()"},

        // Version
        {"zeta_version", {}, "string", "Return the Zeta language version string", "zeta_version()"},

        // Visualization
        {"plot", {{"data", "vector"}, {"type", "string", true}, {"title", "string", true}}, "graph", "Create a plot", "plot($scores, \"bar\", \"Scores\")"},
        {"metric", {{"name", "string"}, {"value", "number"}}, "metric", "Create a KPI metric", "metric(\"Revenue\", 50000)"},
        {"dashboard", {{"title", "string"}, {"author", "string", true}}, "dashboard", "Create dashboard config", "dashboard(\"Sales\", \"Team\")"},
        {"serve", {{"port", "number"}}, "null", "Start HTTP server", "serve(8080)"},
        {"route", {{"method", "string"}, {"path", "string"}, {"handler", "function"}}, "null", "Register HTTP route", "route(\"GET\", \"/api\", fn() {...})"},
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
