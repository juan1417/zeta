// Auto-generated from zeta.schema.json — DO NOT EDIT
// Run: python3 schema/gen_typescript.py

export enum ZetaType {
  ANY = "any",
  BOOL = "bool",
  BOOL_VEC = "bool_vec",
  DF = "df",
  DICT = "dict",
  ERR = "err",
  FUNC = "func",
  MATRIZ = "matriz",
  NULL = "null",
  NUM = "num",
  SCENE = "scene",
  STR = "str",
  STR_VEC = "str_vec",
  VEC = "vec",
}

export interface ParamInfo {
  name: string;
  type: string;
  optional: boolean;
}

export interface FunctionInfo {
  name: string;
  category: string;
  description: string;
  params: ParamInfo[];
  returns: string;
  examples: string[];
  since: string;
}

export interface CategoryInfo {
  id: string;
  label: string;
  order: number;
}

export interface ZetaSchema {
  title: string;
  version: string;
  types: Record<string, { description: string }>;
  categories: CategoryInfo[];
  functions: FunctionInfo[];
}

export const ZETA_TYPES: Record<string, string> = {
  "any": "Any type",
  "bool": "Boolean value: true or false",
  "bool_vec": "Vector of boolean values",
  "df": "DataFrame (tabular data)",
  "dict": "Dictionary (key-value pairs)",
  "err": "Error value",
  "func": "Function reference",
  "matriz": "2D matrix of numeric values",
  "null": "Null/void value",
  "num": "Numeric value (float64)",
  "scene": "Scene specification for visualization",
  "str": "String value",
  "str_vec": "Vector of string values",
  "vec": "Vector of numeric values",
};

export const ZETA_FUNCTIONS: FunctionInfo[] = [
  {
    name: "sum",
    category: "statistics",
    description: "Sum all values in a vector",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "num",
    examples: [
      "sum(<1,2,3>)",
    ],
    since: "0.1.0",
  },
  {
    name: "mean",
    category: "statistics",
    description: "Calculate mean of a vector",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "num",
    examples: [
      "mean(<1,2,3,4,5>)",
    ],
    since: "0.1.0",
  },
  {
    name: "min",
    category: "statistics",
    description: "Find minimum value in a vector",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "num",
    examples: [
      "min(<3,1,2>)",
    ],
    since: "0.1.0",
  },
  {
    name: "max",
    category: "statistics",
    description: "Find maximum value in a vector",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "num",
    examples: [
      "max(<3,1,2>)",
    ],
    since: "0.1.0",
  },
  {
    name: "stddev",
    category: "statistics",
    description: "Calculate standard deviation of a vector",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "num",
    examples: [
      "stddev(<1,2,3,4,5>)",
    ],
    since: "0.1.0",
  },
  {
    name: "count",
    category: "statistics",
    description: "Count non-null values in a vector",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "num",
    examples: [
      "count(<1,2,3>)",
    ],
    since: "0.1.0",
  },
  {
    name: "median",
    category: "statistics",
    description: "Calculate median of a vector",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "num",
    examples: [
      "median(<1,2,3,4,5>)",
    ],
    since: "0.1.0",
  },
  {
    name: "percentile",
    category: "statistics",
    description: "Calculate percentile of a vector",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "p", type: "num", optional: false },
    ],
    returns: "num",
    examples: [
      "percentile(<1,2,3,4,5>, 50)",
    ],
    since: "0.1.0",
  },
  {
    name: "mode",
    category: "statistics",
    description: "Find mode (most frequent value) of a vector",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "num",
    examples: [
      "mode(<1,2,2,3>)",
    ],
    since: "0.1.0",
  },
  {
    name: "cor",
    category: "statistics",
    description: "Calculate Pearson correlation between two vectors",
    params: [
      { name: "x", type: "vec", optional: false },
      { name: "y", type: "vec", optional: false },
    ],
    returns: "num",
    examples: [
      "cor(<1,2,3>, <4,5,6>)",
    ],
    since: "0.1.0",
  },
  {
    name: "cov",
    category: "statistics",
    description: "Calculate covariance between two vectors",
    params: [
      { name: "x", type: "vec", optional: false },
      { name: "y", type: "vec", optional: false },
    ],
    returns: "num",
    examples: [
      "cov(<1,2,3>, <4,5,6>)",
    ],
    since: "0.1.0",
  },
  {
    name: "abs",
    category: "math",
    description: "Absolute value of a number",
    params: [
      { name: "x", type: "num", optional: false },
    ],
    returns: "num",
    examples: [
      "abs(-5)",
    ],
    since: "0.1.0",
  },
  {
    name: "round",
    category: "math",
    description: "Round a number to nearest integer",
    params: [
      { name: "x", type: "num", optional: false },
    ],
    returns: "num",
    examples: [
      "round(3.7)",
    ],
    since: "0.1.0",
  },
  {
    name: "floor",
    category: "math",
    description: "Floor a number (round down)",
    params: [
      { name: "x", type: "num", optional: false },
    ],
    returns: "num",
    examples: [
      "floor(3.7)",
    ],
    since: "0.1.0",
  },
  {
    name: "ceil",
    category: "math",
    description: "Ceiling a number (round up)",
    params: [
      { name: "x", type: "num", optional: false },
    ],
    returns: "num",
    examples: [
      "ceil(3.2)",
    ],
    since: "0.1.0",
  },
  {
    name: "pow",
    category: "math",
    description: "Raise base to exponent",
    params: [
      { name: "base", type: "num", optional: false },
      { name: "exp", type: "num", optional: false },
    ],
    returns: "num",
    examples: [
      "pow(2, 3)",
    ],
    since: "0.1.0",
  },
  {
    name: "sqrt",
    category: "math",
    description: "Square root of a number",
    params: [
      { name: "x", type: "num", optional: false },
    ],
    returns: "num",
    examples: [
      "sqrt(9)",
    ],
    since: "0.1.0",
  },
  {
    name: "format",
    category: "math",
    description: "Format a number with specified decimal places",
    params: [
      { name: "x", type: "num", optional: false },
      { name: "decimals", type: "num", optional: false },
    ],
    returns: "str",
    examples: [
      "format(3.14159, 2)",
    ],
    since: "0.1.0",
  },
  {
    name: "len",
    category: "strings",
    description: "Get length of string or vector",
    params: [
      { name: "s", type: "any", optional: false },
    ],
    returns: "num",
    examples: [
      "len(\"hello\")",
    ],
    since: "0.1.0",
  },
  {
    name: "upper",
    category: "strings",
    description: "Convert string to uppercase",
    params: [
      { name: "s", type: "str", optional: false },
    ],
    returns: "str",
    examples: [
      "upper(\"hello\")",
    ],
    since: "0.1.0",
  },
  {
    name: "lower",
    category: "strings",
    description: "Convert string to lowercase",
    params: [
      { name: "s", type: "str", optional: false },
    ],
    returns: "str",
    examples: [
      "lower(\"HELLO\")",
    ],
    since: "0.1.0",
  },
  {
    name: "substr",
    category: "strings",
    description: "Extract substring from string",
    params: [
      { name: "s", type: "str", optional: false },
      { name: "start", type: "num", optional: false },
      { name: "end", type: "num", optional: true },
    ],
    returns: "str",
    examples: [
      "substr(\"hello\", 1, 3)",
    ],
    since: "0.1.0",
  },
  {
    name: "split",
    category: "strings",
    description: "Split string by delimiter",
    params: [
      { name: "s", type: "str", optional: false },
      { name: "delimiter", type: "str", optional: false },
    ],
    returns: "str_vec",
    examples: [
      "split(\"a,b,c\", \",\")",
    ],
    since: "0.1.0",
  },
  {
    name: "join",
    category: "strings",
    description: "Join vector of strings with separator",
    params: [
      { name: "vec", type: "str_vec", optional: false },
      { name: "separator", type: "str", optional: false },
    ],
    returns: "str",
    examples: [
      "join(<\"a\",\"b\",\"c\">, \",\")",
    ],
    since: "0.1.0",
  },
  {
    name: "replace",
    category: "strings",
    description: "Replace occurrences in string",
    params: [
      { name: "s", type: "str", optional: false },
      { name: "old", type: "str", optional: false },
      { name: "new", type: "str", optional: false },
    ],
    returns: "str",
    examples: [
      "replace(\"hello world\", \"world\", \"zeta\")",
    ],
    since: "0.1.0",
  },
  {
    name: "find",
    category: "strings",
    description: "Find position of substring in string",
    params: [
      { name: "s", type: "str", optional: false },
      { name: "substr", type: "str", optional: false },
    ],
    returns: "num",
    examples: [
      "find(\"hello world\", \"world\")",
    ],
    since: "0.1.0",
  },
  {
    name: "push",
    category: "vectors",
    description: "Add element to end of vector",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "val", type: "num", optional: false },
    ],
    returns: "vec",
    examples: [
      "push(<1,2>, 3)",
    ],
    since: "0.1.0",
  },
  {
    name: "reverse",
    category: "vectors",
    description: "Reverse a vector",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "vec",
    examples: [
      "reverse(<1,2,3>)",
    ],
    since: "0.1.0",
  },
  {
    name: "sort",
    category: "vectors",
    description: "Sort a vector in ascending order",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "vec",
    examples: [
      "sort(<3,1,2>)",
    ],
    since: "0.1.0",
  },
  {
    name: "unique",
    category: "vectors",
    description: "Get unique values from a vector",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "vec",
    examples: [
      "unique(<1,2,2,3,3>)",
    ],
    since: "0.1.0",
  },
  {
    name: "range",
    category: "vectors",
    description: "Generate a sequence of numbers",
    params: [
      { name: "start", type: "num", optional: false },
      { name: "end", type: "num", optional: true },
      { name: "step", type: "num", optional: true },
    ],
    returns: "vec",
    examples: [
      "range(1, 10, 2)",
    ],
    since: "0.1.0",
  },
  {
    name: "keys",
    category: "dicts",
    description: "Get all keys from a dictionary",
    params: [
      { name: "d", type: "dict", optional: false },
    ],
    returns: "str_vec",
    examples: [
      "keys({\"a\": 1, \"b\": 2})",
    ],
    since: "0.1.0",
  },
  {
    name: "values",
    category: "dicts",
    description: "Get all values from a dictionary",
    params: [
      { name: "d", type: "dict", optional: false },
    ],
    returns: "vec",
    examples: [
      "values({\"a\": 1, \"b\": 2})",
    ],
    since: "0.1.0",
  },
  {
    name: "head",
    category: "dataframes",
    description: "Get first n rows of DataFrame or vector",
    params: [
      { name: "df", type: "df|vec", optional: false },
      { name: "n", type: "num", optional: true },
    ],
    returns: "df|vec",
    examples: [
      "head($df, 5)",
    ],
    since: "0.1.0",
  },
  {
    name: "select",
    category: "dataframes",
    description: "Select a column from DataFrame",
    params: [
      { name: "df", type: "df", optional: false },
      { name: "col", type: "str", optional: false },
    ],
    returns: "vec",
    examples: [
      "select($df, \"age\")",
    ],
    since: "0.1.0",
  },
  {
    name: "drop",
    category: "dataframes",
    description: "Drop a column from DataFrame",
    params: [
      { name: "df", type: "df", optional: false },
      { name: "col", type: "str", optional: false },
    ],
    returns: "df",
    examples: [
      "drop($df, \"id\")",
    ],
    since: "0.1.0",
  },
  {
    name: "drop_nan",
    category: "dataframes",
    description: "Drop rows with NaN in specified column",
    params: [
      { name: "df", type: "df", optional: false },
      { name: "col", type: "str", optional: false },
    ],
    returns: "df",
    examples: [
      "drop_nan($df, \"age\")",
    ],
    since: "0.1.0",
  },
  {
    name: "group_by",
    category: "dataframes",
    description: "Group DataFrame by column(s)",
    params: [
      { name: "df", type: "df", optional: false },
      { name: "cols", type: "str_vec", optional: false },
    ],
    returns: "dict",
    examples: [
      "group_by($df, <\"category\">)",
    ],
    since: "0.1.0",
  },
  {
    name: "agg",
    category: "dataframes",
    description: "Aggregate grouped data with function",
    params: [
      { name: "groups", type: "dict", optional: false },
      { name: "col", type: "str", optional: false },
      { name: "fn", type: "str", optional: false },
    ],
    returns: "df",
    examples: [
      "agg($groups, \"sales\", \"sum\")",
    ],
    since: "0.1.0",
  },
  {
    name: "merge",
    category: "dataframes",
    description: "Merge two DataFrames on column",
    params: [
      { name: "df1", type: "df", optional: false },
      { name: "df2", type: "df", optional: false },
      { name: "on", type: "str", optional: false },
    ],
    returns: "df",
    examples: [
      "merge($df1, $df2, \"id\")",
    ],
    since: "0.1.0",
  },
  {
    name: "info",
    category: "exploration",
    description: "Get DataFrame information (columns, types, non-null counts)",
    params: [
      { name: "df", type: "df", optional: false },
    ],
    returns: "str",
    examples: [
      "info($df)",
    ],
    since: "0.2.0",
  },
  {
    name: "describe",
    category: "exploration",
    description: "Get descriptive statistics for DataFrame",
    params: [
      { name: "df", type: "df", optional: false },
    ],
    returns: "str",
    examples: [
      "describe($df)",
    ],
    since: "0.2.0",
  },
  {
    name: "tail",
    category: "exploration",
    description: "Get last n rows of DataFrame or vector",
    params: [
      { name: "df", type: "df|vec", optional: false },
      { name: "n", type: "num", optional: true },
    ],
    returns: "df|vec",
    examples: [
      "tail($df, 5)",
    ],
    since: "0.2.0",
  },
  {
    name: "sample",
    category: "exploration",
    description: "Random sample of n rows from DataFrame",
    params: [
      { name: "df", type: "df", optional: false },
      { name: "n", type: "num", optional: true },
    ],
    returns: "df",
    examples: [
      "sample($df, 100)",
    ],
    since: "0.2.0",
  },
  {
    name: "value_counts",
    category: "exploration",
    description: "Count occurrences of each value in vector",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "df",
    examples: [
      "value_counts(<1,1,2,3,3,3>)",
    ],
    since: "0.2.0",
  },
  {
    name: "nunique",
    category: "exploration",
    description: "Count number of unique values in vector",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "num",
    examples: [
      "nunique(<1,2,2,3>)",
    ],
    since: "0.2.0",
  },
  {
    name: "isna",
    category: "exploration",
    description: "Check for NaN values in DataFrame or vector",
    params: [
      { name: "df", type: "df|vec", optional: false },
    ],
    returns: "df|vec",
    examples: [
      "isna($df)",
    ],
    since: "0.2.0",
  },
  {
    name: "duplicated",
    category: "exploration",
    description: "Find duplicate values in vector",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "vec",
    examples: [
      "duplicated(<1,2,2,3>)",
    ],
    since: "0.2.0",
  },
  {
    name: "cut",
    category: "exploration",
    description: "Bin continuous values into discrete intervals",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "bins", type: "num", optional: false },
    ],
    returns: "vec",
    examples: [
      "cut(<1,2,3,4,5>, 3)",
    ],
    since: "0.2.0",
  },
  {
    name: "qcut",
    category: "exploration",
    description: "Bin continuous values into quantile-based intervals",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "quantiles", type: "num", optional: false },
    ],
    returns: "vec",
    examples: [
      "qcut(<1,2,3,4,5>, 4)",
    ],
    since: "0.2.0",
  },
  {
    name: "dnorm",
    category: "distributions",
    description: "Normal distribution density function",
    params: [
      { name: "x", type: "num", optional: false },
      { name: "mean", type: "num", optional: true },
      { name: "sd", type: "num", optional: true },
    ],
    returns: "num",
    examples: [
      "dnorm(0)",
    ],
    since: "0.2.0",
  },
  {
    name: "pnorm",
    category: "distributions",
    description: "Normal distribution cumulative distribution function",
    params: [
      { name: "x", type: "num", optional: false },
      { name: "mean", type: "num", optional: true },
      { name: "sd", type: "num", optional: true },
    ],
    returns: "num",
    examples: [
      "pnorm(1.96)",
    ],
    since: "0.2.0",
  },
  {
    name: "qnorm",
    category: "distributions",
    description: "Normal distribution quantile function (inverse CDF)",
    params: [
      { name: "p", type: "num", optional: false },
      { name: "mean", type: "num", optional: true },
      { name: "sd", type: "num", optional: true },
    ],
    returns: "num",
    examples: [
      "qnorm(0.975)",
    ],
    since: "0.2.0",
  },
  {
    name: "dgamma",
    category: "distributions",
    description: "Gamma distribution density function",
    params: [
      { name: "x", type: "num", optional: false },
      { name: "shape", type: "num", optional: false },
      { name: "rate", type: "num", optional: false },
    ],
    returns: "num",
    examples: [
      "dgamma(1, 2, 1)",
    ],
    since: "0.2.0",
  },
  {
    name: "dbeta",
    category: "distributions",
    description: "Beta distribution density function",
    params: [
      { name: "x", type: "num", optional: false },
      { name: "alpha", type: "num", optional: false },
      { name: "beta", type: "num", optional: false },
    ],
    returns: "num",
    examples: [
      "dbeta(0.5, 2, 2)",
    ],
    since: "0.2.0",
  },
  {
    name: "dunif",
    category: "distributions",
    description: "Uniform distribution density function",
    params: [
      { name: "x", type: "num", optional: false },
      { name: "min", type: "num", optional: true },
      { name: "max", type: "num", optional: true },
    ],
    returns: "num",
    examples: [
      "dunif(0.5)",
    ],
    since: "0.2.0",
  },
  {
    name: "dt_dist",
    category: "distributions",
    description: "Student's t-distribution density function",
    params: [
      { name: "x", type: "num", optional: false },
      { name: "df", type: "num", optional: false },
    ],
    returns: "num",
    examples: [
      "dt_dist(0, 10)",
    ],
    since: "0.2.0",
  },
  {
    name: "df_dist",
    category: "distributions",
    description: "F-distribution density function",
    params: [
      { name: "x", type: "num", optional: false },
      { name: "df1", type: "num", optional: false },
      { name: "df2", type: "num", optional: false },
    ],
    returns: "num",
    examples: [
      "df_dist(1, 5, 10)",
    ],
    since: "0.2.0",
  },
  {
    name: "dchisq",
    category: "distributions",
    description: "Chi-squared distribution density function",
    params: [
      { name: "x", type: "num", optional: false },
      { name: "df", type: "num", optional: false },
    ],
    returns: "num",
    examples: [
      "dchisq(2, 3)",
    ],
    since: "0.2.0",
  },
  {
    name: "t_test",
    category: "tests",
    description: "Two-sample t-test",
    params: [
      { name: "x", type: "vec", optional: false },
      { name: "y", type: "vec", optional: false },
    ],
    returns: "dict",
    examples: [
      "t_test(<1,2,3>, <4,5,6>)",
    ],
    since: "0.2.0",
  },
  {
    name: "anova",
    category: "tests",
    description: "One-way ANOVA test",
    params: [
      { name: "x", type: "vec", optional: false },
      { name: "y", type: "vec", optional: false },
      { name: "rest", type: "vec", optional: true },
    ],
    returns: "dict",
    examples: [
      "anova(<1,2>, <3,4>, <5,6>)",
    ],
    since: "0.2.0",
  },
  {
    name: "chi_square",
    category: "tests",
    description: "Chi-squared test of independence",
    params: [
      { name: "observed", type: "vec", optional: false },
      { name: "expected", type: "vec", optional: false },
    ],
    returns: "dict",
    examples: [
      "chi_square(<10,20>, <15,15>)",
    ],
    since: "0.2.0",
  },
  {
    name: "linear_regression",
    category: "regression",
    description: "Simple linear regression",
    params: [
      { name: "x", type: "vec", optional: false },
      { name: "y", type: "vec", optional: false },
    ],
    returns: "dict",
    examples: [
      "linear_regression(<1,2,3>, <2,4,5>)",
    ],
    since: "0.2.0",
  },
  {
    name: "cumsum",
    category: "window",
    description: "Cumulative sum of vector",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "vec",
    examples: [
      "cumsum(<1,2,3>)",
    ],
    since: "0.2.0",
  },
  {
    name: "cummax",
    category: "window",
    description: "Cumulative maximum of vector",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "vec",
    examples: [
      "cummax(<3,1,4,1,5>)",
    ],
    since: "0.2.0",
  },
  {
    name: "cummin",
    category: "window",
    description: "Cumulative minimum of vector",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "vec",
    examples: [
      "cummin(<3,1,4,1,5>)",
    ],
    since: "0.2.0",
  },
  {
    name: "rolling_mean",
    category: "window",
    description: "Rolling mean over a window",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "window", type: "num", optional: false },
    ],
    returns: "vec",
    examples: [
      "rolling_mean(<1,2,3,4,5>, 3)",
    ],
    since: "0.2.0",
  },
  {
    name: "rolling_std",
    category: "window",
    description: "Rolling standard deviation over a window",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "window", type: "num", optional: false },
    ],
    returns: "vec",
    examples: [
      "rolling_std(<1,2,3,4,5>, 3)",
    ],
    since: "0.2.0",
  },
  {
    name: "rolling_sum",
    category: "window",
    description: "Rolling sum over a window",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "window", type: "num", optional: false },
    ],
    returns: "vec",
    examples: [
      "rolling_sum(<1,2,3,4,5>, 3)",
    ],
    since: "0.2.0",
  },
  {
    name: "rolling_min",
    category: "window",
    description: "Rolling minimum over a window",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "window", type: "num", optional: false },
    ],
    returns: "vec",
    examples: [
      "rolling_min(<3,1,4,1,5>, 3)",
    ],
    since: "0.2.0",
  },
  {
    name: "rolling_max",
    category: "window",
    description: "Rolling maximum over a window",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "window", type: "num", optional: false },
    ],
    returns: "vec",
    examples: [
      "rolling_max(<3,1,4,1,5>, 3)",
    ],
    since: "0.2.0",
  },
  {
    name: "lag",
    category: "window",
    description: "Shift values forward by n positions",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "n", type: "num", optional: false },
    ],
    returns: "vec",
    examples: [
      "lag(<1,2,3,4>, 1)",
    ],
    since: "0.2.0",
  },
  {
    name: "lead",
    category: "window",
    description: "Shift values backward by n positions",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "n", type: "num", optional: false },
    ],
    returns: "vec",
    examples: [
      "lead(<1,2,3,4>, 1)",
    ],
    since: "0.2.0",
  },
  {
    name: "diff",
    category: "window",
    description: "Difference between consecutive values",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "n", type: "num", optional: true },
    ],
    returns: "vec",
    examples: [
      "diff(<1,3,6,10>)",
    ],
    since: "0.2.0",
  },
  {
    name: "row_number",
    category: "window",
    description: "Assign row numbers to vector",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "vec",
    examples: [
      "row_number(<5,3,1,4>)",
    ],
    since: "0.2.0",
  },
  {
    name: "rank",
    category: "window",
    description: "Assign ranks to values in vector",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "vec",
    examples: [
      "rank(<5,3,1,4>)",
    ],
    since: "0.2.0",
  },
  {
    name: "pct_change",
    category: "window",
    description: "Percentage change between values",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "n", type: "num", optional: true },
    ],
    returns: "vec",
    examples: [
      "pct_change(<100,110,105>)",
    ],
    since: "0.2.0",
  },
  {
    name: "drop_duplicates",
    category: "cleaning",
    description: "Remove duplicate rows from DataFrame",
    params: [
      { name: "df", type: "df", optional: false },
    ],
    returns: "df",
    examples: [
      "drop_duplicates($df)",
    ],
    since: "0.2.0",
  },
  {
    name: "rename",
    category: "cleaning",
    description: "Rename a column in DataFrame",
    params: [
      { name: "df", type: "df", optional: false },
      { name: "old", type: "str", optional: false },
      { name: "new", type: "str", optional: false },
    ],
    returns: "df",
    examples: [
      "rename($df, \"old_name\", \"new_name\")",
    ],
    since: "0.2.0",
  },
  {
    name: "select_cols",
    category: "cleaning",
    description: "Select specific columns from DataFrame",
    params: [
      { name: "df", type: "df", optional: false },
      { name: "cols", type: "str_vec", optional: false },
    ],
    returns: "df",
    examples: [
      "select_cols($df, <\"name\",\"age\">)",
    ],
    since: "0.2.0",
  },
  {
    name: "drop_cols",
    category: "cleaning",
    description: "Drop multiple columns from DataFrame",
    params: [
      { name: "df", type: "df", optional: false },
      { name: "cols", type: "str_vec", optional: false },
    ],
    returns: "df",
    examples: [
      "drop_cols($df, <\"id\",\"temp\">)",
    ],
    since: "0.2.0",
  },
  {
    name: "fillna",
    category: "cleaning",
    description: "Fill NaN values with a value",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "val", type: "str|num", optional: false },
    ],
    returns: "vec",
    examples: [
      "fillna(<1, NaN, 3>, 0)",
    ],
    since: "0.2.0",
  },
  {
    name: "replace_val",
    category: "cleaning",
    description: "Replace values in DataFrame column",
    params: [
      { name: "df", type: "df", optional: false },
      { name: "col", type: "str", optional: false },
      { name: "old", type: "str", optional: false },
      { name: "new", type: "str", optional: false },
    ],
    returns: "df",
    examples: [
      "replace_val($df, \"status\", \"old\", \"new\")",
    ],
    since: "0.2.0",
  },
  {
    name: "clip",
    category: "cleaning",
    description: "Clip values to a range",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "min", type: "num", optional: false },
      { name: "max", type: "num", optional: false },
    ],
    returns: "vec",
    examples: [
      "clip(<1,5,10>, 2, 8)",
    ],
    since: "0.2.0",
  },
  {
    name: "trim",
    category: "cleaning",
    description: "Trim outliers by percentiles",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "percentile", type: "num", optional: false },
    ],
    returns: "vec",
    examples: [
      "trim(<1,2,3,100>, 10)",
    ],
    since: "0.2.0",
  },
  {
    name: "normalize",
    category: "cleaning",
    description: "Normalize vector to [0,1] range",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "vec",
    examples: [
      "normalize(<1,2,3,4,5>)",
    ],
    since: "0.2.0",
  },
  {
    name: "standardize",
    category: "cleaning",
    description: "Standardize vector to z-scores (mean=0, sd=1)",
    params: [
      { name: "vec", type: "vec", optional: false },
    ],
    returns: "vec",
    examples: [
      "standardize(<1,2,3,4,5>)",
    ],
    since: "0.2.0",
  },
  {
    name: "load_csv",
    category: "io",
    description: "Load CSV file into DataFrame",
    params: [
      { name: "path", type: "str", optional: false },
      { name: "sep", type: "str", optional: true },
    ],
    returns: "df",
    examples: [
      "load_csv(\"data.csv\")",
    ],
    since: "0.1.0",
  },
  {
    name: "load_json",
    category: "io",
    description: "Load JSON file into DataFrame",
    params: [
      { name: "path", type: "str", optional: false },
    ],
    returns: "df",
    examples: [
      "load_json(\"data.json\")",
    ],
    since: "0.1.0",
  },
  {
    name: "load_xlsx",
    category: "io",
    description: "Load Excel file into DataFrame",
    params: [
      { name: "path", type: "str", optional: false },
      { name: "sheet", type: "num", optional: true },
    ],
    returns: "df",
    examples: [
      "load_xlsx(\"data.xlsx\", 0)",
    ],
    since: "0.2.0",
  },
  {
    name: "save_csv",
    category: "io",
    description: "Save DataFrame to CSV file",
    params: [
      { name: "path", type: "str", optional: false },
      { name: "df", type: "df", optional: false },
      { name: "sep", type: "str", optional: true },
    ],
    returns: "null",
    examples: [
      "save_csv(\"output.csv\", $df)",
    ],
    since: "0.1.0",
  },
  {
    name: "save_xlsx",
    category: "io",
    description: "Save DataFrame to Excel file",
    params: [
      { name: "path", type: "str", optional: false },
      { name: "df", type: "df", optional: false },
    ],
    returns: "null",
    examples: [
      "save_xlsx(\"output.xlsx\", $df)",
    ],
    since: "0.2.0",
  },
  {
    name: "scene",
    category: "visualization",
    description: "Create a new visualization scene",
    params: [
      { name: "title", type: "str", optional: false },
      { name: "themes", type: "str_vec", optional: true },
    ],
    returns: "scene",
    examples: [
      "scene(\"My Dashboard\")",
    ],
    since: "0.1.0",
  },
  {
    name: "add_metric",
    category: "visualization",
    description: "Add a metric card to the scene",
    params: [
      { name: "label", type: "str", optional: false },
      { name: "value", type: "num", optional: false },
      { name: "unit", type: "str", optional: true },
      { name: "color", type: "str", optional: true },
    ],
    returns: "null",
    examples: [
      "add_metric(\"Revenue\", 50000, \"$\")",
    ],
    since: "0.1.0",
  },
  {
    name: "add_line_plot",
    category: "visualization",
    description: "Add a line plot to the scene",
    params: [
      { name: "df", type: "df", optional: false },
      { name: "x", type: "str", optional: false },
      { name: "y", type: "str", optional: false },
      { name: "title", type: "str", optional: false },
    ],
    returns: "null",
    examples: [
      "add_line_plot($df, \"date\", \"sales\", \"Sales Over Time\")",
    ],
    since: "0.1.0",
  },
  {
    name: "add_bar_chart",
    category: "visualization",
    description: "Add a bar chart to the scene",
    params: [
      { name: "df", type: "df", optional: false },
      { name: "x", type: "str", optional: false },
      { name: "y", type: "str", optional: false },
      { name: "title", type: "str", optional: false },
    ],
    returns: "null",
    examples: [
      "add_bar_chart($df, \"category\", \"count\", \"Distribution\")",
    ],
    since: "0.1.0",
  },
  {
    name: "add_scatter",
    category: "visualization",
    description: "Add a scatter plot to the scene",
    params: [
      { name: "df", type: "df", optional: false },
      { name: "x", type: "str", optional: false },
      { name: "y", type: "str", optional: false },
      { name: "title", type: "str", optional: false },
    ],
    returns: "null",
    examples: [
      "add_scatter($df, \"height\", \"weight\", \"Correlation\")",
    ],
    since: "0.1.0",
  },
  {
    name: "add_histogram",
    category: "visualization",
    description: "Add a histogram to the scene",
    params: [
      { name: "df", type: "df", optional: false },
      { name: "col", type: "str", optional: false },
      { name: "title", type: "str", optional: false },
      { name: "bins", type: "num", optional: true },
    ],
    returns: "null",
    examples: [
      "add_histogram($df, \"age\", \"Age Distribution\", 20)",
    ],
    since: "0.1.0",
  },
  {
    name: "add_box_plot",
    category: "visualization",
    description: "Add a box plot to the scene",
    params: [
      { name: "df", type: "df", optional: false },
      { name: "col", type: "str", optional: false },
      { name: "title", type: "str", optional: false },
    ],
    returns: "null",
    examples: [
      "add_box_plot($df, \"score\", \"Score Distribution\")",
    ],
    since: "0.1.0",
  },
  {
    name: "add_linear_regression",
    category: "visualization",
    description: "Add a scatter plot with linear regression line",
    params: [
      { name: "df", type: "df", optional: false },
      { name: "x", type: "str", optional: false },
      { name: "y", type: "str", optional: false },
      { name: "title", type: "str", optional: false },
    ],
    returns: "null",
    examples: [
      "add_linear_regression($df, \"x\", \"y\", \"Regression\")",
    ],
    since: "0.1.0",
  },
  {
    name: "mk_err",
    category: "errors",
    description: "Create an error value",
    params: [
      { name: "code", type: "str", optional: false },
      { name: "msg", type: "str", optional: false },
    ],
    returns: "err",
    examples: [
      "mk_err(\"E001\", \"Invalid input\")",
    ],
    since: "0.1.0",
  },
  {
    name: "mk_null_val",
    category: "errors",
    description: "Create a null value",
    params: [
    ],
    returns: "null",
    examples: [
      "mk_null_val()",
    ],
    since: "0.1.0",
  },
  {
    name: "type",
    category: "types",
    description: "Get type name of a value",
    params: [
      { name: "val", type: "any", optional: false },
    ],
    returns: "str",
    examples: [
      "type(42)",
    ],
    since: "0.1.0",
  },
  {
    name: "is_null",
    category: "types",
    description: "Check if a value is null",
    params: [
      { name: "val", type: "any", optional: false },
    ],
    returns: "bool",
    examples: [
      "is_null(null)",
    ],
    since: "0.1.0",
  },
  {
    name: "is_error",
    category: "types",
    description: "Check if a value is an error",
    params: [
      { name: "val", type: "any", optional: false },
    ],
    returns: "bool",
    examples: [
      "is_error(mk_err(\"E001\", \"error\"))",
    ],
    since: "0.1.0",
  },
  {
    name: "map",
    category: "functional",
    description: "Apply function to each element of vector",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "fn", type: "func", optional: false },
    ],
    returns: "vec",
    examples: [
      "map(<1,2,3>, $x -> $x * 2)",
    ],
    since: "0.1.0",
  },
  {
    name: "filter",
    category: "functional",
    description: "Filter vector elements by predicate",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "fn", type: "func", optional: false },
    ],
    returns: "vec",
    examples: [
      "filter(<1,2,3,4>, $x -> $x > 2)",
    ],
    since: "0.1.0",
  },
  {
    name: "reduce",
    category: "functional",
    description: "Reduce vector to single value",
    params: [
      { name: "vec", type: "vec", optional: false },
      { name: "fn", type: "func", optional: false },
      { name: "init", type: "any", optional: false },
    ],
    returns: "any",
    examples: [
      "reduce(<1,2,3>, $a $b -> $a + $b, 0)",
    ],
    since: "0.1.0",
  },
  {
    name: "transpose",
    category: "matrix",
    description: "Transpose a matrix",
    params: [
      { name: "m", type: "matriz", optional: false },
    ],
    returns: "matriz",
    examples: [
      "transpose(<<1,2>,<3,4>>>)",
    ],
    since: "0.1.0",
  },
  {
    name: "dot",
    category: "matrix",
    description: "Dot product of two vectors",
    params: [
      { name: "a", type: "vec", optional: false },
      { name: "b", type: "vec", optional: false },
    ],
    returns: "num",
    examples: [
      "dot(<1,2>, <3,4>)",
    ],
    since: "0.1.0",
  },
  {
    name: "load_lib",
    category: "ffi",
    description: "Load a native plugin library",
    params: [
      { name: "path", type: "str", optional: false },
      { name: "config", type: "dict", optional: false },
    ],
    returns: "null",
    examples: [
      "load_lib(\"my_plugin.so\", {})",
    ],
    since: "0.2.0",
  },
  {
    name: "plugin",
    category: "ffi",
    description: "Check if a plugin is loaded",
    params: [
      { name: "name", type: "str", optional: false },
    ],
    returns: "bool",
    examples: [
      "plugin(\"my_plugin\")",
    ],
    since: "0.2.0",
  },
  {
    name: "plugin_info",
    category: "ffi",
    description: "Get information about a loaded plugin",
    params: [
      { name: "name", type: "str", optional: false },
    ],
    returns: "dict",
    examples: [
      "plugin_info(\"my_plugin\")",
    ],
    since: "0.2.0",
  },
  {
    name: "clear_arena",
    category: "memory",
    description: "Reset arena allocator (bulk free all arena memory)",
    params: [
    ],
    returns: "null",
    examples: [
      "clear_arena()",
    ],
    since: "0.2.0",
  },
  {
    name: "arena_bytes",
    category: "memory",
    description: "Query current arena allocation size in bytes",
    params: [
    ],
    returns: "num",
    examples: [
      "arena_bytes()",
    ],
    since: "0.2.0",
  },
  {
    name: "zeta_version",
    category: "system",
    description: "Get Zeta runtime version string",
    params: [
    ],
    returns: "str",
    examples: [
      "zeta_version()",
    ],
    since: "0.1.0",
  },
  {
    name: "print",
    category: "system",
    description: "Print values to stdout",
    params: [
      { name: "values", type: "any...", optional: false },
    ],
    returns: "null",
    examples: [
      "print(\"Hello\", 42)",
    ],
    since: "0.1.0",
  },
  {
    name: "time",
    category: "system",
    description: "Get current Unix timestamp in seconds",
    params: [
    ],
    returns: "num",
    examples: [
      "time()",
    ],
    since: "0.1.0",
  },
  {
    name: "route",
    category: "system",
    description: "Define an HTTP route handler for the server",
    params: [
      { name: "method", type: "str", optional: false },
      { name: "path", type: "str", optional: false },
      { name: "handler", type: "func", optional: false },
    ],
    returns: "null",
    examples: [
      "route(\"GET\", \"/api/data\", $req -> $data)",
    ],
    since: "0.1.0",
  },
];

export const ZETA_CATEGORIES: CategoryInfo[] = [
  { id: "statistics", label: "Statistics", order: 1 },
  { id: "math", label: "Math", order: 2 },
  { id: "strings", label: "Strings", order: 3 },
  { id: "vectors", label: "Vectors", order: 4 },
  { id: "dicts", label: "Dictionaries", order: 5 },
  { id: "dataframes", label: "DataFrames", order: 6 },
  { id: "exploration", label: "Exploration", order: 7 },
  { id: "distributions", label: "Distributions", order: 8 },
  { id: "tests", label: "Statistical Tests", order: 9 },
  { id: "regression", label: "Regression", order: 10 },
  { id: "window", label: "Window Functions", order: 11 },
  { id: "cleaning", label: "Data Cleaning", order: 12 },
  { id: "io", label: "Input/Output", order: 13 },
  { id: "visualization", label: "Visualization", order: 14 },
  { id: "errors", label: "Error Handling", order: 15 },
  { id: "types", label: "Type Checking", order: 16 },
  { id: "functional", label: "Functional", order: 17 },
  { id: "matrix", label: "Matrix", order: 18 },
  { id: "ffi", label: "Foreign Function Interface", order: 19 },
  { id: "memory", label: "Memory Management", order: 20 },
  { id: "system", label: "System", order: 21 },
];

export function getFunctionsByCategory(category: string): FunctionInfo[] {
  return ZETA_FUNCTIONS.filter((f) => f.category === category);
}

export function searchFunctions(query: string): FunctionInfo[] {
  const q = query.toLowerCase();
  return ZETA_FUNCTIONS.filter(
    (f) =>
      f.name.toLowerCase().includes(q) ||
      f.description.toLowerCase().includes(q) ||
      f.params.some((p) => p.name.toLowerCase().includes(q))
  );
}

export function getFunctionByName(name: string): FunctionInfo | undefined {
  return ZETA_FUNCTIONS.find((f) => f.name === name);
}
