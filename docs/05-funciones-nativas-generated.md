# Funciones Nativas de Zeta (Generadas)

> Auto-generated from `zeta.schema.json` v0.2.0.
> Total: **119** funciones en **21** categorías.

## Tabla de Contenidos

- [Statistics](#statistics) (11)
- [Math](#math) (7)
- [Strings](#strings) (8)
- [Vectors](#vectors) (5)
- [Dictionaries](#dictionaries) (2)
- [DataFrames](#dataframes) (7)
- [Exploration](#exploration) (10)
- [Distributions](#distributions) (9)
- [Statistical Tests](#statistical-tests) (3)
- [Regression](#regression) (1)
- [Window Functions](#window-functions) (14)
- [Data Cleaning](#data-cleaning) (10)
- [Input/Output](#inputoutput) (5)
- [Visualization](#visualization) (8)
- [Error Handling](#error-handling) (2)
- [Type Checking](#type-checking) (3)
- [Functional](#functional) (3)
- [Matrix](#matrix) (2)
- [Foreign Function Interface](#foreign-function-interface) (3)
- [Memory Management](#memory-management) (2)
- [System](#system) (4)

## Resumen por Categoría

| Categoría | Funciones | Cantidad |
|-----------|-----------|----------|
| Statistics | `sum`, `mean`, `min`, `max`, `stddev`, `count`, `median`, `percentile`, `mode`, `cor`, `cov` | 11 |
| Math | `abs`, `round`, `floor`, `ceil`, `pow`, `sqrt`, `format` | 7 |
| Strings | `len`, `upper`, `lower`, `substr`, `split`, `join`, `replace`, `find` | 8 |
| Vectors | `push`, `reverse`, `sort`, `unique`, `range` | 5 |
| Dictionaries | `keys`, `values` | 2 |
| DataFrames | `head`, `select`, `drop`, `drop_nan`, `group_by`, `agg`, `merge` | 7 |
| Exploration | `info`, `describe`, `tail`, `sample`, `value_counts`, `nunique`, `isna`, `duplicated`, `cut`, `qcut` | 10 |
| Distributions | `dnorm`, `pnorm`, `qnorm`, `dgamma`, `dbeta`, `dunif`, `dt_dist`, `df_dist`, `dchisq` | 9 |
| Statistical Tests | `t_test`, `anova`, `chi_square` | 3 |
| Regression | `linear_regression` | 1 |
| Window Functions | `cumsum`, `cummax`, `cummin`, `rolling_mean`, `rolling_std`, `rolling_sum`, `rolling_min`, `rolling_max`, `lag`, `lead`, `diff`, `row_number`, `rank`, `pct_change` | 14 |
| Data Cleaning | `drop_duplicates`, `rename`, `select_cols`, `drop_cols`, `fillna`, `replace_val`, `clip`, `trim`, `normalize`, `standardize` | 10 |
| Input/Output | `load_csv`, `load_json`, `load_xlsx`, `save_csv`, `save_xlsx` | 5 |
| Visualization | `scene`, `add_metric`, `add_line_plot`, `add_bar_chart`, `add_scatter`, `add_histogram`, `add_box_plot`, `add_linear_regression` | 8 |
| Error Handling | `mk_err`, `mk_null_val` | 2 |
| Type Checking | `type`, `is_null`, `is_error` | 3 |
| Functional | `map`, `filter`, `reduce` | 3 |
| Matrix | `transpose`, `dot` | 2 |
| Foreign Function Interface | `load_lib`, `plugin`, `plugin_info` | 3 |
| Memory Management | `clear_arena`, `arena_bytes` | 2 |
| System | `zeta_version`, `print`, `time`, `route` | 4 |

## Statistics

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `sum` | *vec*: `vec` | `num` | Sum all values in a vector |
| `mean` | *vec*: `vec` | `num` | Calculate mean of a vector |
| `min` | *vec*: `vec` | `num` | Find minimum value in a vector |
| `max` | *vec*: `vec` | `num` | Find maximum value in a vector |
| `stddev` | *vec*: `vec` | `num` | Calculate standard deviation of a vector |
| `count` | *vec*: `vec` | `num` | Count non-null values in a vector |
| `median` | *vec*: `vec` | `num` | Calculate median of a vector |
| `percentile` | *vec*: `vec`, *p*: `num` | `num` | Calculate percentile of a vector |
| `mode` | *vec*: `vec` | `num` | Find mode (most frequent value) of a vector |
| `cor` | *x*: `vec`, *y*: `vec` | `num` | Calculate Pearson correlation between two vectors |
| `cov` | *x*: `vec`, *y*: `vec` | `num` | Calculate covariance between two vectors |

### Detalle

#### `sum`

**Descripción:** Sum all values in a vector

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
sum(<1,2,3>)
```

#### `mean`

**Descripción:** Calculate mean of a vector

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
mean(<1,2,3,4,5>)
```

#### `min`

**Descripción:** Find minimum value in a vector

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
min(<3,1,2>)
```

#### `max`

**Descripción:** Find maximum value in a vector

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
max(<3,1,2>)
```

#### `stddev`

**Descripción:** Calculate standard deviation of a vector

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
stddev(<1,2,3,4,5>)
```

#### `count`

**Descripción:** Count non-null values in a vector

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
count(<1,2,3>)
```

#### `median`

**Descripción:** Calculate median of a vector

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
median(<1,2,3,4,5>)
```

#### `percentile`

**Descripción:** Calculate percentile of a vector

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `p` | `num` | Sí | — |

**Ejemplo:**

```zeta
percentile(<1,2,3,4,5>, 50)
```

#### `mode`

**Descripción:** Find mode (most frequent value) of a vector

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
mode(<1,2,2,3>)
```

#### `cor`

**Descripción:** Calculate Pearson correlation between two vectors

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `vec` | Sí | — |
| `y` | `vec` | Sí | — |

**Ejemplo:**

```zeta
cor(<1,2,3>, <4,5,6>)
```

#### `cov`

**Descripción:** Calculate covariance between two vectors

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `vec` | Sí | — |
| `y` | `vec` | Sí | — |

**Ejemplo:**

```zeta
cov(<1,2,3>, <4,5,6>)
```

---

## Math

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `abs` | *x*: `num` | `num` | Absolute value of a number |
| `round` | *x*: `num` | `num` | Round a number to nearest integer |
| `floor` | *x*: `num` | `num` | Floor a number (round down) |
| `ceil` | *x*: `num` | `num` | Ceiling a number (round up) |
| `pow` | *base*: `num`, *exp*: `num` | `num` | Raise base to exponent |
| `sqrt` | *x*: `num` | `num` | Square root of a number |
| `format` | *x*: `num`, *decimals*: `num` | `str` | Format a number with specified decimal places |

### Detalle

#### `abs`

**Descripción:** Absolute value of a number

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `num` | Sí | — |

**Ejemplo:**

```zeta
abs(-5)
```

#### `round`

**Descripción:** Round a number to nearest integer

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `num` | Sí | — |

**Ejemplo:**

```zeta
round(3.7)
```

#### `floor`

**Descripción:** Floor a number (round down)

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `num` | Sí | — |

**Ejemplo:**

```zeta
floor(3.7)
```

#### `ceil`

**Descripción:** Ceiling a number (round up)

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `num` | Sí | — |

**Ejemplo:**

```zeta
ceil(3.2)
```

#### `pow`

**Descripción:** Raise base to exponent

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `base` | `num` | Sí | — |
| `exp` | `num` | Sí | — |

**Ejemplo:**

```zeta
pow(2, 3)
```

#### `sqrt`

**Descripción:** Square root of a number

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `num` | Sí | — |

**Ejemplo:**

```zeta
sqrt(9)
```

#### `format`

**Descripción:** Format a number with specified decimal places

**Retorna:** `str` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `num` | Sí | — |
| `decimals` | `num` | Sí | — |

**Ejemplo:**

```zeta
format(3.14159, 2)
```

---

## Strings

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `len` | *s*: `any` | `num` | Get length of string or vector |
| `upper` | *s*: `str` | `str` | Convert string to uppercase |
| `lower` | *s*: `str` | `str` | Convert string to lowercase |
| `substr` | *s*: `str`, *start*: `num`, *end*: `num` (opt) | `str` | Extract substring from string |
| `split` | *s*: `str`, *delimiter*: `str` | `str_vec` | Split string by delimiter |
| `join` | *vec*: `str_vec`, *separator*: `str` | `str` | Join vector of strings with separator |
| `replace` | *s*: `str`, *old*: `str`, *new*: `str` | `str` | Replace occurrences in string |
| `find` | *s*: `str`, *substr*: `str` | `num` | Find position of substring in string |

### Detalle

#### `len`

**Descripción:** Get length of string or vector

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `s` | `any` | Sí | — |

**Ejemplo:**

```zeta
len("hello")
```

#### `upper`

**Descripción:** Convert string to uppercase

**Retorna:** `str` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `s` | `str` | Sí | — |

**Ejemplo:**

```zeta
upper("hello")
```

#### `lower`

**Descripción:** Convert string to lowercase

**Retorna:** `str` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `s` | `str` | Sí | — |

**Ejemplo:**

```zeta
lower("HELLO")
```

#### `substr`

**Descripción:** Extract substring from string

**Retorna:** `str` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `s` | `str` | Sí | — |
| `start` | `num` | Sí | — |
| `end` | `num` | No | — |

**Ejemplo:**

```zeta
substr("hello", 1, 3)
```

#### `split`

**Descripción:** Split string by delimiter

**Retorna:** `str_vec` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `s` | `str` | Sí | — |
| `delimiter` | `str` | Sí | — |

**Ejemplo:**

```zeta
split("a,b,c", ",")
```

#### `join`

**Descripción:** Join vector of strings with separator

**Retorna:** `str` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `str_vec` | Sí | — |
| `separator` | `str` | Sí | — |

**Ejemplo:**

```zeta
join(<"a","b","c">, ",")
```

#### `replace`

**Descripción:** Replace occurrences in string

**Retorna:** `str` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `s` | `str` | Sí | — |
| `old` | `str` | Sí | — |
| `new` | `str` | Sí | — |

**Ejemplo:**

```zeta
replace("hello world", "world", "zeta")
```

#### `find`

**Descripción:** Find position of substring in string

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `s` | `str` | Sí | — |
| `substr` | `str` | Sí | — |

**Ejemplo:**

```zeta
find("hello world", "world")
```

---

## Vectors

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `push` | *vec*: `vec`, *val*: `num` | `vec` | Add element to end of vector |
| `reverse` | *vec*: `vec` | `vec` | Reverse a vector |
| `sort` | *vec*: `vec` | `vec` | Sort a vector in ascending order |
| `unique` | *vec*: `vec` | `vec` | Get unique values from a vector |
| `range` | *start*: `num`, *end*: `num` (opt), *step*: `num` (opt) | `vec` | Generate a sequence of numbers |

### Detalle

#### `push`

**Descripción:** Add element to end of vector

**Retorna:** `vec` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `val` | `num` | Sí | — |

**Ejemplo:**

```zeta
push(<1,2>, 3)
```

#### `reverse`

**Descripción:** Reverse a vector

**Retorna:** `vec` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
reverse(<1,2,3>)
```

#### `sort`

**Descripción:** Sort a vector in ascending order

**Retorna:** `vec` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
sort(<3,1,2>)
```

#### `unique`

**Descripción:** Get unique values from a vector

**Retorna:** `vec` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
unique(<1,2,2,3,3>)
```

#### `range`

**Descripción:** Generate a sequence of numbers

**Retorna:** `vec` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `start` | `num` | Sí | — |
| `end` | `num` | No | — |
| `step` | `num` | No | — |

**Ejemplo:**

```zeta
range(1, 10, 2)
```

---

## Dictionaries

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `keys` | *d*: `dict` | `str_vec` | Get all keys from a dictionary |
| `values` | *d*: `dict` | `vec` | Get all values from a dictionary |

### Detalle

#### `keys`

**Descripción:** Get all keys from a dictionary

**Retorna:** `str_vec` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `d` | `dict` | Sí | — |

**Ejemplo:**

```zeta
keys({"a": 1, "b": 2})
```

#### `values`

**Descripción:** Get all values from a dictionary

**Retorna:** `vec` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `d` | `dict` | Sí | — |

**Ejemplo:**

```zeta
values({"a": 1, "b": 2})
```

---

## DataFrames

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `head` | *df*: `df|vec`, *n*: `num` (opt) | `df|vec` | Get first n rows of DataFrame or vector |
| `select` | *df*: `df`, *col*: `str` | `vec` | Select a column from DataFrame |
| `drop` | *df*: `df`, *col*: `str` | `df` | Drop a column from DataFrame |
| `drop_nan` | *df*: `df`, *col*: `str` | `df` | Drop rows with NaN in specified column |
| `group_by` | *df*: `df`, *cols*: `str_vec` | `dict` | Group DataFrame by column(s) |
| `agg` | *groups*: `dict`, *col*: `str`, *fn*: `str` | `df` | Aggregate grouped data with function |
| `merge` | *df1*: `df`, *df2*: `df`, *on*: `str` | `df` | Merge two DataFrames on column |

### Detalle

#### `head`

**Descripción:** Get first n rows of DataFrame or vector

**Retorna:** `df|vec` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df|vec` | Sí | — |
| `n` | `num` | No | — |

**Ejemplo:**

```zeta
head($df, 5)
```

#### `select`

**Descripción:** Select a column from DataFrame

**Retorna:** `vec` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df` | Sí | — |
| `col` | `str` | Sí | — |

**Ejemplo:**

```zeta
select($df, "age")
```

#### `drop`

**Descripción:** Drop a column from DataFrame

**Retorna:** `df` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df` | Sí | — |
| `col` | `str` | Sí | — |

**Ejemplo:**

```zeta
drop($df, "id")
```

#### `drop_nan`

**Descripción:** Drop rows with NaN in specified column

**Retorna:** `df` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df` | Sí | — |
| `col` | `str` | Sí | — |

**Ejemplo:**

```zeta
drop_nan($df, "age")
```

#### `group_by`

**Descripción:** Group DataFrame by column(s)

**Retorna:** `dict` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df` | Sí | — |
| `cols` | `str_vec` | Sí | — |

**Ejemplo:**

```zeta
group_by($df, <"category">)
```

#### `agg`

**Descripción:** Aggregate grouped data with function

**Retorna:** `df` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `groups` | `dict` | Sí | — |
| `col` | `str` | Sí | — |
| `fn` | `str` | Sí | — |

**Ejemplo:**

```zeta
agg($groups, "sales", "sum")
```

#### `merge`

**Descripción:** Merge two DataFrames on column

**Retorna:** `df` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df1` | `df` | Sí | — |
| `df2` | `df` | Sí | — |
| `on` | `str` | Sí | — |

**Ejemplo:**

```zeta
merge($df1, $df2, "id")
```

---

## Exploration

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `info` | *df*: `df` | `str` | Get DataFrame information (columns, types, non-null counts) |
| `describe` | *df*: `df` | `str` | Get descriptive statistics for DataFrame |
| `tail` | *df*: `df|vec`, *n*: `num` (opt) | `df|vec` | Get last n rows of DataFrame or vector |
| `sample` | *df*: `df`, *n*: `num` (opt) | `df` | Random sample of n rows from DataFrame |
| `value_counts` | *vec*: `vec` | `df` | Count occurrences of each value in vector |
| `nunique` | *vec*: `vec` | `num` | Count number of unique values in vector |
| `isna` | *df*: `df|vec` | `df|vec` | Check for NaN values in DataFrame or vector |
| `duplicated` | *vec*: `vec` | `vec` | Find duplicate values in vector |
| `cut` | *vec*: `vec`, *bins*: `num` | `vec` | Bin continuous values into discrete intervals |
| `qcut` | *vec*: `vec`, *quantiles*: `num` | `vec` | Bin continuous values into quantile-based intervals |

### Detalle

#### `info`

**Descripción:** Get DataFrame information (columns, types, non-null counts)

**Retorna:** `str` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df` | Sí | — |

**Ejemplo:**

```zeta
info($df)
```

#### `describe`

**Descripción:** Get descriptive statistics for DataFrame

**Retorna:** `str` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df` | Sí | — |

**Ejemplo:**

```zeta
describe($df)
```

#### `tail`

**Descripción:** Get last n rows of DataFrame or vector

**Retorna:** `df|vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df|vec` | Sí | — |
| `n` | `num` | No | — |

**Ejemplo:**

```zeta
tail($df, 5)
```

#### `sample`

**Descripción:** Random sample of n rows from DataFrame

**Retorna:** `df` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df` | Sí | — |
| `n` | `num` | No | — |

**Ejemplo:**

```zeta
sample($df, 100)
```

#### `value_counts`

**Descripción:** Count occurrences of each value in vector

**Retorna:** `df` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
value_counts(<1,1,2,3,3,3>)
```

#### `nunique`

**Descripción:** Count number of unique values in vector

**Retorna:** `num` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
nunique(<1,2,2,3>)
```

#### `isna`

**Descripción:** Check for NaN values in DataFrame or vector

**Retorna:** `df|vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df|vec` | Sí | — |

**Ejemplo:**

```zeta
isna($df)
```

#### `duplicated`

**Descripción:** Find duplicate values in vector

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
duplicated(<1,2,2,3>)
```

#### `cut`

**Descripción:** Bin continuous values into discrete intervals

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `bins` | `num` | Sí | — |

**Ejemplo:**

```zeta
cut(<1,2,3,4,5>, 3)
```

#### `qcut`

**Descripción:** Bin continuous values into quantile-based intervals

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `quantiles` | `num` | Sí | — |

**Ejemplo:**

```zeta
qcut(<1,2,3,4,5>, 4)
```

---

## Distributions

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `dnorm` | *x*: `num`, *mean*: `num` (opt), *sd*: `num` (opt) | `num` | Normal distribution density function |
| `pnorm` | *x*: `num`, *mean*: `num` (opt), *sd*: `num` (opt) | `num` | Normal distribution cumulative distribution function |
| `qnorm` | *p*: `num`, *mean*: `num` (opt), *sd*: `num` (opt) | `num` | Normal distribution quantile function (inverse CDF) |
| `dgamma` | *x*: `num`, *shape*: `num`, *rate*: `num` | `num` | Gamma distribution density function |
| `dbeta` | *x*: `num`, *alpha*: `num`, *beta*: `num` | `num` | Beta distribution density function |
| `dunif` | *x*: `num`, *min*: `num` (opt), *max*: `num` (opt) | `num` | Uniform distribution density function |
| `dt_dist` | *x*: `num`, *df*: `num` | `num` | Student's t-distribution density function |
| `df_dist` | *x*: `num`, *df1*: `num`, *df2*: `num` | `num` | F-distribution density function |
| `dchisq` | *x*: `num`, *df*: `num` | `num` | Chi-squared distribution density function |

### Detalle

#### `dnorm`

**Descripción:** Normal distribution density function

**Retorna:** `num` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `num` | Sí | — |
| `mean` | `num` | No | — |
| `sd` | `num` | No | — |

**Ejemplo:**

```zeta
dnorm(0)
```

#### `pnorm`

**Descripción:** Normal distribution cumulative distribution function

**Retorna:** `num` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `num` | Sí | — |
| `mean` | `num` | No | — |
| `sd` | `num` | No | — |

**Ejemplo:**

```zeta
pnorm(1.96)
```

#### `qnorm`

**Descripción:** Normal distribution quantile function (inverse CDF)

**Retorna:** `num` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `p` | `num` | Sí | — |
| `mean` | `num` | No | — |
| `sd` | `num` | No | — |

**Ejemplo:**

```zeta
qnorm(0.975)
```

#### `dgamma`

**Descripción:** Gamma distribution density function

**Retorna:** `num` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `num` | Sí | — |
| `shape` | `num` | Sí | — |
| `rate` | `num` | Sí | — |

**Ejemplo:**

```zeta
dgamma(1, 2, 1)
```

#### `dbeta`

**Descripción:** Beta distribution density function

**Retorna:** `num` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `num` | Sí | — |
| `alpha` | `num` | Sí | — |
| `beta` | `num` | Sí | — |

**Ejemplo:**

```zeta
dbeta(0.5, 2, 2)
```

#### `dunif`

**Descripción:** Uniform distribution density function

**Retorna:** `num` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `num` | Sí | — |
| `min` | `num` | No | — |
| `max` | `num` | No | — |

**Ejemplo:**

```zeta
dunif(0.5)
```

#### `dt_dist`

**Descripción:** Student's t-distribution density function

**Retorna:** `num` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `num` | Sí | — |
| `df` | `num` | Sí | — |

**Ejemplo:**

```zeta
dt_dist(0, 10)
```

#### `df_dist`

**Descripción:** F-distribution density function

**Retorna:** `num` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `num` | Sí | — |
| `df1` | `num` | Sí | — |
| `df2` | `num` | Sí | — |

**Ejemplo:**

```zeta
df_dist(1, 5, 10)
```

#### `dchisq`

**Descripción:** Chi-squared distribution density function

**Retorna:** `num` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `num` | Sí | — |
| `df` | `num` | Sí | — |

**Ejemplo:**

```zeta
dchisq(2, 3)
```

---

## Statistical Tests

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `t_test` | *x*: `vec`, *y*: `vec` | `dict` | Two-sample t-test |
| `anova` | *x*: `vec`, *y*: `vec`, *rest*: `vec` (opt) | `dict` | One-way ANOVA test |
| `chi_square` | *observed*: `vec`, *expected*: `vec` | `dict` | Chi-squared test of independence |

### Detalle

#### `t_test`

**Descripción:** Two-sample t-test

**Retorna:** `dict` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `vec` | Sí | — |
| `y` | `vec` | Sí | — |

**Ejemplo:**

```zeta
t_test(<1,2,3>, <4,5,6>)
```

#### `anova`

**Descripción:** One-way ANOVA test

**Retorna:** `dict` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `vec` | Sí | — |
| `y` | `vec` | Sí | — |
| `rest` | `vec` | No | — |

**Ejemplo:**

```zeta
anova(<1,2>, <3,4>, <5,6>)
```

#### `chi_square`

**Descripción:** Chi-squared test of independence

**Retorna:** `dict` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `observed` | `vec` | Sí | — |
| `expected` | `vec` | Sí | — |

**Ejemplo:**

```zeta
chi_square(<10,20>, <15,15>)
```

---

## Regression

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `linear_regression` | *x*: `vec`, *y*: `vec` | `dict` | Simple linear regression |

### Detalle

#### `linear_regression`

**Descripción:** Simple linear regression

**Retorna:** `dict` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `x` | `vec` | Sí | — |
| `y` | `vec` | Sí | — |

**Ejemplo:**

```zeta
linear_regression(<1,2,3>, <2,4,5>)
```

---

## Window Functions

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `cumsum` | *vec*: `vec` | `vec` | Cumulative sum of vector |
| `cummax` | *vec*: `vec` | `vec` | Cumulative maximum of vector |
| `cummin` | *vec*: `vec` | `vec` | Cumulative minimum of vector |
| `rolling_mean` | *vec*: `vec`, *window*: `num` | `vec` | Rolling mean over a window |
| `rolling_std` | *vec*: `vec`, *window*: `num` | `vec` | Rolling standard deviation over a window |
| `rolling_sum` | *vec*: `vec`, *window*: `num` | `vec` | Rolling sum over a window |
| `rolling_min` | *vec*: `vec`, *window*: `num` | `vec` | Rolling minimum over a window |
| `rolling_max` | *vec*: `vec`, *window*: `num` | `vec` | Rolling maximum over a window |
| `lag` | *vec*: `vec`, *n*: `num` | `vec` | Shift values forward by n positions |
| `lead` | *vec*: `vec`, *n*: `num` | `vec` | Shift values backward by n positions |
| `diff` | *vec*: `vec`, *n*: `num` (opt) | `vec` | Difference between consecutive values |
| `row_number` | *vec*: `vec` | `vec` | Assign row numbers to vector |
| `rank` | *vec*: `vec` | `vec` | Assign ranks to values in vector |
| `pct_change` | *vec*: `vec`, *n*: `num` (opt) | `vec` | Percentage change between values |

### Detalle

#### `cumsum`

**Descripción:** Cumulative sum of vector

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
cumsum(<1,2,3>)
```

#### `cummax`

**Descripción:** Cumulative maximum of vector

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
cummax(<3,1,4,1,5>)
```

#### `cummin`

**Descripción:** Cumulative minimum of vector

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
cummin(<3,1,4,1,5>)
```

#### `rolling_mean`

**Descripción:** Rolling mean over a window

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `window` | `num` | Sí | — |

**Ejemplo:**

```zeta
rolling_mean(<1,2,3,4,5>, 3)
```

#### `rolling_std`

**Descripción:** Rolling standard deviation over a window

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `window` | `num` | Sí | — |

**Ejemplo:**

```zeta
rolling_std(<1,2,3,4,5>, 3)
```

#### `rolling_sum`

**Descripción:** Rolling sum over a window

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `window` | `num` | Sí | — |

**Ejemplo:**

```zeta
rolling_sum(<1,2,3,4,5>, 3)
```

#### `rolling_min`

**Descripción:** Rolling minimum over a window

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `window` | `num` | Sí | — |

**Ejemplo:**

```zeta
rolling_min(<3,1,4,1,5>, 3)
```

#### `rolling_max`

**Descripción:** Rolling maximum over a window

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `window` | `num` | Sí | — |

**Ejemplo:**

```zeta
rolling_max(<3,1,4,1,5>, 3)
```

#### `lag`

**Descripción:** Shift values forward by n positions

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `n` | `num` | Sí | — |

**Ejemplo:**

```zeta
lag(<1,2,3,4>, 1)
```

#### `lead`

**Descripción:** Shift values backward by n positions

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `n` | `num` | Sí | — |

**Ejemplo:**

```zeta
lead(<1,2,3,4>, 1)
```

#### `diff`

**Descripción:** Difference between consecutive values

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `n` | `num` | No | — |

**Ejemplo:**

```zeta
diff(<1,3,6,10>)
```

#### `row_number`

**Descripción:** Assign row numbers to vector

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
row_number(<5,3,1,4>)
```

#### `rank`

**Descripción:** Assign ranks to values in vector

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
rank(<5,3,1,4>)
```

#### `pct_change`

**Descripción:** Percentage change between values

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `n` | `num` | No | — |

**Ejemplo:**

```zeta
pct_change(<100,110,105>)
```

---

## Data Cleaning

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `drop_duplicates` | *df*: `df` | `df` | Remove duplicate rows from DataFrame |
| `rename` | *df*: `df`, *old*: `str`, *new*: `str` | `df` | Rename a column in DataFrame |
| `select_cols` | *df*: `df`, *cols*: `str_vec` | `df` | Select specific columns from DataFrame |
| `drop_cols` | *df*: `df`, *cols*: `str_vec` | `df` | Drop multiple columns from DataFrame |
| `fillna` | *vec*: `vec`, *val*: `str|num` | `vec` | Fill NaN values with a value |
| `replace_val` | *df*: `df`, *col*: `str`, *old*: `str`, *new*: `str` | `df` | Replace values in DataFrame column |
| `clip` | *vec*: `vec`, *min*: `num`, *max*: `num` | `vec` | Clip values to a range |
| `trim` | *vec*: `vec`, *percentile*: `num` | `vec` | Trim outliers by percentiles |
| `normalize` | *vec*: `vec` | `vec` | Normalize vector to [0,1] range |
| `standardize` | *vec*: `vec` | `vec` | Standardize vector to z-scores (mean=0, sd=1) |

### Detalle

#### `drop_duplicates`

**Descripción:** Remove duplicate rows from DataFrame

**Retorna:** `df` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df` | Sí | — |

**Ejemplo:**

```zeta
drop_duplicates($df)
```

#### `rename`

**Descripción:** Rename a column in DataFrame

**Retorna:** `df` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df` | Sí | — |
| `old` | `str` | Sí | — |
| `new` | `str` | Sí | — |

**Ejemplo:**

```zeta
rename($df, "old_name", "new_name")
```

#### `select_cols`

**Descripción:** Select specific columns from DataFrame

**Retorna:** `df` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df` | Sí | — |
| `cols` | `str_vec` | Sí | — |

**Ejemplo:**

```zeta
select_cols($df, <"name","age">)
```

#### `drop_cols`

**Descripción:** Drop multiple columns from DataFrame

**Retorna:** `df` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df` | Sí | — |
| `cols` | `str_vec` | Sí | — |

**Ejemplo:**

```zeta
drop_cols($df, <"id","temp">)
```

#### `fillna`

**Descripción:** Fill NaN values with a value

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `val` | `str|num` | Sí | — |

**Ejemplo:**

```zeta
fillna(<1, NaN, 3>, 0)
```

#### `replace_val`

**Descripción:** Replace values in DataFrame column

**Retorna:** `df` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df` | Sí | — |
| `col` | `str` | Sí | — |
| `old` | `str` | Sí | — |
| `new` | `str` | Sí | — |

**Ejemplo:**

```zeta
replace_val($df, "status", "old", "new")
```

#### `clip`

**Descripción:** Clip values to a range

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `min` | `num` | Sí | — |
| `max` | `num` | Sí | — |

**Ejemplo:**

```zeta
clip(<1,5,10>, 2, 8)
```

#### `trim`

**Descripción:** Trim outliers by percentiles

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `percentile` | `num` | Sí | — |

**Ejemplo:**

```zeta
trim(<1,2,3,100>, 10)
```

#### `normalize`

**Descripción:** Normalize vector to [0,1] range

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
normalize(<1,2,3,4,5>)
```

#### `standardize`

**Descripción:** Standardize vector to z-scores (mean=0, sd=1)

**Retorna:** `vec` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |

**Ejemplo:**

```zeta
standardize(<1,2,3,4,5>)
```

---

## Input/Output

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `load_csv` | *path*: `str`, *sep*: `str` (opt) | `df` | Load CSV file into DataFrame |
| `load_json` | *path*: `str` | `df` | Load JSON file into DataFrame |
| `load_xlsx` | *path*: `str`, *sheet*: `num` (opt) | `df` | Load Excel file into DataFrame |
| `save_csv` | *path*: `str`, *df*: `df`, *sep*: `str` (opt) | `null` | Save DataFrame to CSV file |
| `save_xlsx` | *path*: `str`, *df*: `df` | `null` | Save DataFrame to Excel file |

### Detalle

#### `load_csv`

**Descripción:** Load CSV file into DataFrame

**Retorna:** `df` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `path` | `str` | Sí | — |
| `sep` | `str` | No | — |

**Ejemplo:**

```zeta
load_csv("data.csv")
```

#### `load_json`

**Descripción:** Load JSON file into DataFrame

**Retorna:** `df` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `path` | `str` | Sí | — |

**Ejemplo:**

```zeta
load_json("data.json")
```

#### `load_xlsx`

**Descripción:** Load Excel file into DataFrame

**Retorna:** `df` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `path` | `str` | Sí | — |
| `sheet` | `num` | No | — |

**Ejemplo:**

```zeta
load_xlsx("data.xlsx", 0)
```

#### `save_csv`

**Descripción:** Save DataFrame to CSV file

**Retorna:** `null` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `path` | `str` | Sí | — |
| `df` | `df` | Sí | — |
| `sep` | `str` | No | — |

**Ejemplo:**

```zeta
save_csv("output.csv", $df)
```

#### `save_xlsx`

**Descripción:** Save DataFrame to Excel file

**Retorna:** `null` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `path` | `str` | Sí | — |
| `df` | `df` | Sí | — |

**Ejemplo:**

```zeta
save_xlsx("output.xlsx", $df)
```

---

## Visualization

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `scene` | *title*: `str`, *themes*: `str_vec` (opt) | `scene` | Create a new visualization scene |
| `add_metric` | *label*: `str`, *value*: `num`, *unit*: `str` (opt), *color*: `str` (opt) | `null` | Add a metric card to the scene |
| `add_line_plot` | *df*: `df`, *x*: `str`, *y*: `str`, *title*: `str` | `null` | Add a line plot to the scene |
| `add_bar_chart` | *df*: `df`, *x*: `str`, *y*: `str`, *title*: `str` | `null` | Add a bar chart to the scene |
| `add_scatter` | *df*: `df`, *x*: `str`, *y*: `str`, *title*: `str` | `null` | Add a scatter plot to the scene |
| `add_histogram` | *df*: `df`, *col*: `str`, *title*: `str`, *bins*: `num` (opt) | `null` | Add a histogram to the scene |
| `add_box_plot` | *df*: `df`, *col*: `str`, *title*: `str` | `null` | Add a box plot to the scene |
| `add_linear_regression` | *df*: `df`, *x*: `str`, *y*: `str`, *title*: `str` | `null` | Add a scatter plot with linear regression line |

### Detalle

#### `scene`

**Descripción:** Create a new visualization scene

**Retorna:** `scene` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `title` | `str` | Sí | — |
| `themes` | `str_vec` | No | — |

**Ejemplo:**

```zeta
scene("My Dashboard")
```

#### `add_metric`

**Descripción:** Add a metric card to the scene

**Retorna:** `null` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `label` | `str` | Sí | — |
| `value` | `num` | Sí | — |
| `unit` | `str` | No | — |
| `color` | `str` | No | — |

**Ejemplo:**

```zeta
add_metric("Revenue", 50000, "$")
```

#### `add_line_plot`

**Descripción:** Add a line plot to the scene

**Retorna:** `null` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df` | Sí | — |
| `x` | `str` | Sí | — |
| `y` | `str` | Sí | — |
| `title` | `str` | Sí | — |

**Ejemplo:**

```zeta
add_line_plot($df, "date", "sales", "Sales Over Time")
```

#### `add_bar_chart`

**Descripción:** Add a bar chart to the scene

**Retorna:** `null` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df` | Sí | — |
| `x` | `str` | Sí | — |
| `y` | `str` | Sí | — |
| `title` | `str` | Sí | — |

**Ejemplo:**

```zeta
add_bar_chart($df, "category", "count", "Distribution")
```

#### `add_scatter`

**Descripción:** Add a scatter plot to the scene

**Retorna:** `null` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df` | Sí | — |
| `x` | `str` | Sí | — |
| `y` | `str` | Sí | — |
| `title` | `str` | Sí | — |

**Ejemplo:**

```zeta
add_scatter($df, "height", "weight", "Correlation")
```

#### `add_histogram`

**Descripción:** Add a histogram to the scene

**Retorna:** `null` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df` | Sí | — |
| `col` | `str` | Sí | — |
| `title` | `str` | Sí | — |
| `bins` | `num` | No | — |

**Ejemplo:**

```zeta
add_histogram($df, "age", "Age Distribution", 20)
```

#### `add_box_plot`

**Descripción:** Add a box plot to the scene

**Retorna:** `null` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df` | Sí | — |
| `col` | `str` | Sí | — |
| `title` | `str` | Sí | — |

**Ejemplo:**

```zeta
add_box_plot($df, "score", "Score Distribution")
```

#### `add_linear_regression`

**Descripción:** Add a scatter plot with linear regression line

**Retorna:** `null` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `df` | `df` | Sí | — |
| `x` | `str` | Sí | — |
| `y` | `str` | Sí | — |
| `title` | `str` | Sí | — |

**Ejemplo:**

```zeta
add_linear_regression($df, "x", "y", "Regression")
```

---

## Error Handling

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `mk_err` | *code*: `str`, *msg*: `str` | `err` | Create an error value |
| `mk_null_val` | — | `null` | Create a null value |

### Detalle

#### `mk_err`

**Descripción:** Create an error value

**Retorna:** `err` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `code` | `str` | Sí | — |
| `msg` | `str` | Sí | — |

**Ejemplo:**

```zeta
mk_err("E001", "Invalid input")
```

#### `mk_null_val`

**Descripción:** Create a null value

**Retorna:** `null` | **Desde:** v0.1.0

**Ejemplo:**

```zeta
mk_null_val()
```

---

## Type Checking

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `type` | *val*: `any` | `str` | Get type name of a value |
| `is_null` | *val*: `any` | `bool` | Check if a value is null |
| `is_error` | *val*: `any` | `bool` | Check if a value is an error |

### Detalle

#### `type`

**Descripción:** Get type name of a value

**Retorna:** `str` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `val` | `any` | Sí | — |

**Ejemplo:**

```zeta
type(42)
```

#### `is_null`

**Descripción:** Check if a value is null

**Retorna:** `bool` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `val` | `any` | Sí | — |

**Ejemplo:**

```zeta
is_null(null)
```

#### `is_error`

**Descripción:** Check if a value is an error

**Retorna:** `bool` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `val` | `any` | Sí | — |

**Ejemplo:**

```zeta
is_error(mk_err("E001", "error"))
```

---

## Functional

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `map` | *vec*: `vec`, *fn*: `func` | `vec` | Apply function to each element of vector |
| `filter` | *vec*: `vec`, *fn*: `func` | `vec` | Filter vector elements by predicate |
| `reduce` | *vec*: `vec`, *fn*: `func`, *init*: `any` | `any` | Reduce vector to single value |

### Detalle

#### `map`

**Descripción:** Apply function to each element of vector

**Retorna:** `vec` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `fn` | `func` | Sí | — |

**Ejemplo:**

```zeta
map(<1,2,3>, $x -> $x * 2)
```

#### `filter`

**Descripción:** Filter vector elements by predicate

**Retorna:** `vec` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `fn` | `func` | Sí | — |

**Ejemplo:**

```zeta
filter(<1,2,3,4>, $x -> $x > 2)
```

#### `reduce`

**Descripción:** Reduce vector to single value

**Retorna:** `any` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `vec` | `vec` | Sí | — |
| `fn` | `func` | Sí | — |
| `init` | `any` | Sí | — |

**Ejemplo:**

```zeta
reduce(<1,2,3>, $a $b -> $a + $b, 0)
```

---

## Matrix

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `transpose` | *m*: `matriz` | `matriz` | Transpose a matrix |
| `dot` | *a*: `vec`, *b*: `vec` | `num` | Dot product of two vectors |

### Detalle

#### `transpose`

**Descripción:** Transpose a matrix

**Retorna:** `matriz` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `m` | `matriz` | Sí | — |

**Ejemplo:**

```zeta
transpose(<<1,2>,<3,4>>>)
```

#### `dot`

**Descripción:** Dot product of two vectors

**Retorna:** `num` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `a` | `vec` | Sí | — |
| `b` | `vec` | Sí | — |

**Ejemplo:**

```zeta
dot(<1,2>, <3,4>)
```

---

## Foreign Function Interface

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `load_lib` | *path*: `str`, *config*: `dict` | `null` | Load a native plugin library |
| `plugin` | *name*: `str` | `bool` | Check if a plugin is loaded |
| `plugin_info` | *name*: `str` | `dict` | Get information about a loaded plugin |

### Detalle

#### `load_lib`

**Descripción:** Load a native plugin library

**Retorna:** `null` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `path` | `str` | Sí | — |
| `config` | `dict` | Sí | — |

**Ejemplo:**

```zeta
load_lib("my_plugin.so", {})
```

#### `plugin`

**Descripción:** Check if a plugin is loaded

**Retorna:** `bool` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `name` | `str` | Sí | — |

**Ejemplo:**

```zeta
plugin("my_plugin")
```

#### `plugin_info`

**Descripción:** Get information about a loaded plugin

**Retorna:** `dict` | **Desde:** v0.2.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `name` | `str` | Sí | — |

**Ejemplo:**

```zeta
plugin_info("my_plugin")
```

---

## Memory Management

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `clear_arena` | — | `null` | Reset arena allocator (bulk free all arena memory) |
| `arena_bytes` | — | `num` | Query current arena allocation size in bytes |

### Detalle

#### `clear_arena`

**Descripción:** Reset arena allocator (bulk free all arena memory)

**Retorna:** `null` | **Desde:** v0.2.0

**Ejemplo:**

```zeta
clear_arena()
```

#### `arena_bytes`

**Descripción:** Query current arena allocation size in bytes

**Retorna:** `num` | **Desde:** v0.2.0

**Ejemplo:**

```zeta
arena_bytes()
```

---

## System

### Referencia Rápida

| Función | Parámetros | Retorna | Descripción |
|---------|------------|---------|-------------|
| `zeta_version` | — | `str` | Get Zeta runtime version string |
| `print` | *values*: `any...` | `null` | Print values to stdout |
| `time` | — | `num` | Get current Unix timestamp in seconds |
| `route` | *method*: `str`, *path*: `str`, *handler*: `func` | `null` | Define an HTTP route handler for the server |

### Detalle

#### `zeta_version`

**Descripción:** Get Zeta runtime version string

**Retorna:** `str` | **Desde:** v0.1.0

**Ejemplo:**

```zeta
zeta_version()
```

#### `print`

**Descripción:** Print values to stdout

**Retorna:** `null` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `values` | `any...` | Sí | — |

**Ejemplo:**

```zeta
print("Hello", 42)
```

#### `time`

**Descripción:** Get current Unix timestamp in seconds

**Retorna:** `num` | **Desde:** v0.1.0

**Ejemplo:**

```zeta
time()
```

#### `route`

**Descripción:** Define an HTTP route handler for the server

**Retorna:** `null` | **Desde:** v0.1.0

**Parámetros:**

| Nombre | Tipo | Requerido | Descripción |
|--------|------|-----------|-------------|
| `method` | `str` | Sí | — |
| `path` | `str` | Sí | — |
| `handler` | `func` | Sí | — |

**Ejemplo:**

```zeta
route("GET", "/api/data", $req -> $data)
```

---

*Generado automáticamente por `schema/gen_docs.py`*
