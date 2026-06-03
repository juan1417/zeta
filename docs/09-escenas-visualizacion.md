# 9. Escenas y visualización

Las **escenas** son el sistema declarativo de Zeta para describir visualizaciones. Una escena es un árbol de nodos (métricas, gráficos, textos) que se persiste en RAM y disco, y que cualquier consumidor (OpenGL dashboard, terminal ANSI, HTML/Chart.js) puede renderizar leyendo el mismo JSON.

## 9.1. El modelo

```
SceneSpec {
    string titulo
    string autor
    SceneLayout layout { tipo, cols, gap, background }
    vector<SceneNode> nodes
    double created_at
    double updated_at
}

SceneNode {
    string tipo       // "metric", "line_plot", "bar_chart", etc.
    string titulo
    string id          // opcional
    map<string, string> cols     // x_col, y_col
    map<string, double> nums     // bins, alpha
    map<string, string> strs     // color, marker
    map<string, vector<double>> series  // datos
}
```

## 9.2. Crear una escena: `scene`

```zeta
$scn = scene("Mi Reporte", "zeta")
# o
$scn = scene("Mi Reporte")   # autor por defecto "zeta"
```

Crea una nueva escena vacía y la asigna como **escena activa** (estado interno del intérprete). Las llamadas subsiguientes a `layout`, `add_*`, etc. la modifican.

## 9.3. Configurar el layout: `layout`

```zeta
layout("grid", 2, 12, "#1a1a1a")
```

Parámetros:

1. **tipo** (string): `"grid"` (por defecto), `"vertical"`, `"horizontal"`, `"absolute"`.
2. **cols** (num): número de columnas en grid (por defecto 2).
3. **gap** (num): separación entre nodos en píxeles (OpenGL) o chars (terminal). El renderer de terminal lo ignora y usa 2.
4. **background** (string): color de fondo en hex (`"#1e1e1a"`, `"#ffffff"`).

```zeta
# Ejemplo: layout vertical con 0 gap
layout("vertical", 1, 0, "#000000")

# Ejemplo: grid 3x3
layout("grid", 3, 20, "#f5f5f5")
```

## 9.4. Agregar métricas: `add_metric`

```zeta
add_metric("Ventas totales", sum($datos:ventas), "$", "#4caf50")
```

Parámetros:

1. **nombre** (string): título visible.
2. **valor** (num): valor numérico principal.
3. **unidad** (string, opcional): texto después del número (e.g., `"$"`, `"%"`, `"kg"`).
4. **color** (string, opcional): hex color para el número.

```zeta
add_metric("Margen", 0.42, "%", "#2196f3")
add_metric("Conteo", len($datos), "", "#ff9800")
```

### Texto en lugar de número

Si el segundo argumento es un string, se renderiza como un nodo de **texto** (no métrica):

```zeta
add_metric("Estado", "Operativo", "", "#4caf50")
# Equivale a un nodo de tipo "text"
```

## 9.5. Agregar gráficos de línea: `add_line_plot`

```zeta
add_line_plot($datos, "Tendencia de ventas", "idx", "ventas")
```

Parámetros:

1. **DataFrame**: datos a graficar.
2. **título** (string): título del plot.
3. **columna_x** (string): nombre de la columna para el eje X.
4. **columna_y** (string): nombre de la columna para el eje Y.

El gráfico conecta los puntos `(x[i], y[i])` con líneas.

## 9.6. Agregar gráficos de barras: `add_bar_chart`

```zeta
add_bar_chart($datos, "Gastos mensuales", "idx", "gastos")
```

Misma firma que `add_line_plot` pero renderiza barras verticales.

## 9.7. Agregar scatter: `add_scatter`

```zeta
add_scatter($datos, "Ventas vs Gastos", "ventas", "gastos")
```

Puntos individuales (sin línea). Útil para correlación.

## 9.8. Agregar histograma: `add_histogram`

```zeta
add_histogram($datos, "Distribución de ventas", "ventas", 6)
```

Parámetros:

1. DataFrame
2. Título
3. Columna a binear
4. **bins** (num, opcional): número de bins (por defecto 10).

## 9.9. Agregar box plot: `add_box_plot`

```zeta
add_box_plot($datos, "Box de ventas", "ventas")
```

Parámetros:

1. DataFrame
2. Título
3. Columna a resumir (calcula Q1, mediana, Q3, min, max).

## 9.10. Agregar heatmap: `add_heatmap`

```zeta
add_heatmap($datos, "Heatmap", "x", "y")
```

*(Actualmente hace fallback a scatter en el renderer OpenGL; el de terminal no lo soporta)*.

## 9.11. Agregar regresión lineal: `add_linear_regression`

```zeta
add_linear_regression($datos, "Regresión V vs G", "ventas", "gastos")
```

Calcula `y = slope * x + intercept` por mínimos cuadrados y dibuja los puntos + la línea de ajuste + la ecuación.

## 9.12. Persistir y cargar

### Guardar a disco

```zeta
print(guardar_grafo("tests/dashboard_scene.json"))
# Imprime: "Guardado: tests/dashboard_scene.json (10 nodos)"
```

`guardar_grafo($ruta)` escribe la escena activa a un JSON.

### Cargar desde disco

```zeta
$scn = cargar_grafo("tests/dashboard_scene.json")
# $scn es ahora la escena activa
```

### Obtener referencia a la escena actual

```zeta
$actual = grafo_actual()
```

Devuelve la escena activa o `null` si no hay ninguna.

## 9.13. El JSON de escena

El formato JSON de una escena (el que consume `zeta_dashboard` y `zeta_term`):

```json
{
  "titulo": "Analisis de Ventas",
  "autor": "zeta",
  "created_at": 1717000000.0,
  "updated_at": 1717000000.0,
  "layout": {
    "tipo": "grid",
    "cols": 2,
    "gap": 10,
    "background": "#1a1a1a"
  },
  "nodes": [
    {
      "tipo": "metric",
      "titulo": "Ventas totales",
      "nums": { "value": 17700.0 },
      "strs": { "unit": "$", "color": "#4caf50" }
    },
    {
      "tipo": "line_plot",
      "titulo": "Tendencia de ventas",
      "cols": { "x": "idx", "y": "ventas" },
      "data": {
        "idx": [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0],
        "ventas": [1000.0, 1200.0, 1500.0, 1300.0, 1700.0, 1900.0, 1800.0, 2100.0, 2000.0, 2200.0],
        "gastos": [500.0, 600.0, 700.0, 650.0, 800.0, 850.0, 900.0, 950.0, 1000.0, 1050.0]
      }
    }
  ]
}
```

**Punto clave**: los datos se **embeben en el JSON** (`data: {col: [v1, v2, ...]}`), no se referencian por nombre. Esto permite que cualquier consumidor renderice sin tener acceso al DataFrame original.

## 9.14. NaN ↔ null en la serialización

Cuando un `null` (NaN) llega a JSON, se serializa como `null` (no como `"NaN"`). Al deserializar, `null` se vuelve a convertir a NaN:

```cpp
// En grafo_json.cpp
if (j.is_null()) return crear_null();    // JSON null → NaN
```

Esto preserva la información "este valor es null" a través de la frontera JSON.

## 9.15. Consumidores de escenas

| Consumidor | Comando | Renderiza | Deps |
|------------|---------|-----------|------|
| `zeta_dashboard` | `./zeta_dashboard --host ...` | OpenGL+ImGui+ImPlot | X11, OpenGL 3.3 |
| `zeta_term` | `./zeta_term --host ...` | ANSI 24-bit half-block | Solo libc |
| HTML embebido | `GET /` (en `zeta_server`) | Chart.js | Browser |
| Cualquier cliente HTTP | `GET /api/grafo` | Lo que quieras | - |

## 9.16. Ejemplo completo

```zeta
# tests/dashboard_scene.zl
$datos = load_csv("tests/datos.csv")
$idx = <1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0>
$datos2 = {"idx": $idx, "ventas": $datos:ventas, "gastos": $datos:gastos}

$scn = scene("Analisis de Ventas", "zeta")
layout("grid", 2, 12, "#1a1a1a")

# KPIs
add_metric("Ventas totales", sum($datos:ventas), "$", "#4caf50")
add_metric("Gastos totales", sum($datos:gastos), "$", "#f44336")
add_metric("Margen bruto", sum($datos:ventas) - sum($datos:gastos), "$", "#2196f3")
add_metric("Promedio ventas", mean($datos:ventas), "$", "#ff9800")

# Gráficos
add_line_plot($datos2, "Tendencia de ventas", "idx", "ventas")
add_bar_chart($datos2, "Gastos mensuales", "idx", "gastos")
add_scatter($datos2, "Ventas vs Gastos", "ventas", "gastos")
add_histogram($datos2, "Distribucion de ventas", "ventas", 6)
add_linear_regression($datos2, "Regresion V vs G", "ventas", "gastos")
add_box_plot($datos2, "Box de ventas", "ventas")

print(guardar_grafo("tests/dashboard_scene.json"))
```

## 9.17. Mejores prácticas

1. **Usa un DataFrame con columna `idx` numérica** para los ejes X de los plots. Si usas una columna de texto (`mes`), los plots no se renderizan correctamente (todos los X son null).

2. **Calcula los KPIs en el script, no en el renderer**: el renderer solo visualiza. Si necesitas una "tasa de conversión", calcúlala en Zeta y pásala como número a `add_metric`.

3. **Guarda a disco después de cada modificación importante** con `guardar_grafo`. Así tienes un snapshot recuperable.

4. **Carga el mismo grafo en todos los consumidores**: `zeta_dashboard` y `zeta_term` pueden abrir el mismo `dashboard_scene.json` y mostrarán lo mismo (con diferente calidad visual).

5. **El layout gap es renderer-specific**: en el OpenGL renderer es en píxeles (10-20 es razonable), en el terminal renderer se ignora y siempre es 2 chars.
