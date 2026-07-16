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

        // I/O
        {"load_csv", {{"path", "string"}}, "DataFrame", "Load CSV file into DataFrame", "load_csv(\"data.csv\")"},
        {"load_xlsx", {{"path", "string"}, {"sheet", "number", true}}, "DataFrame", "Load Excel file", "load_xlsx(\"data.xlsx\")"},
        {"plot", {{"data", "vector"}, {"type", "string", true}, {"title", "string", true}}, "graph", "Create a plot", "plot($scores, \"bar\", \"Scores\")"},
        {"metric", {{"name", "string"}, {"value", "number"}}, "metric", "Create a KPI metric", "metric(\"Revenue\", 50000)"},
        {"dashboard", {{"title", "string"}, {"author", "string"}}, "dashboard", "Create dashboard config", "dashboard(\"Sales\", \"Team\")"},
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
