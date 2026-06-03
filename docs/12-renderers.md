# 12. Renderers

Zeta tiene dos renderers nativos que consumen el mismo JSON de escena (`GET /api/grafo`):

| Renderer | Tecnología | Peso | Requiere |
|----------|-----------|------|----------|
| `zeta_dashboard` | OpenGL 3.3 + GLFW + GLEW + ImGui + ImPlot | ~3.6 MB | X11, OpenGL |
| `zeta_term` | ANSI 24-bit + half-block | ~234 KB | Solo terminal TTY |

Ambos se conectan al servidor, leen la escena, y la renderizan. **Elige el que se ajuste a tu entorno**: el dashboard para workstations con X11, el terminal para SSH o servers headless.

## 12.1. `zeta_dashboard` — OpenGL nativo

### Compilar

```bash
./build.sh dashboard
```

Requiere:
- GLFW 3.4 (en `deps/glfw-build/lib/libglfw3.a`, compilado estáticamente).
- GLEW 2.3 (del sistema, en `/usr/lib/libGLEW.so`).
- ImGui 1.91.8 + ImPlot 0.16 (header-only, en `deps/imgui/` y `deps/implot/`).
- `stb_image_write.h` (en `deps/imgui/`, para screenshots).

### Ejecutar

```bash
./zeta_dashboard --host localhost --port 8080
```

Flags:
- `--host HOST`: servidor Zeta (default `localhost`).
- `--port N`: puerto (default `8080`).
- `--refresh SECS`: intervalo de refresco (default 2s).
- `--screenshot RUTA`: en lugar de mostrar ventana, genera un PNG.
- `--width N --height N`: tamaño de la ventana (o screenshot).
- `--wait N`: frames a esperar antes del screenshot (para que se estabilice el render).

### Modo screenshot

```bash
./zeta_dashboard --host localhost --port 8080 \
    --screenshot tests/dashboard.png --width 1600 --height 1400 --wait 10
```

Crea una ventana oculta, hace N frames, y exporta un PNG via `glReadPixels` + `stb_image_write`. Útil para reportes automatizados, CI, etc.

### Qué muestra

- **Panel principal**: la escena renderizada con ImGui (panel de control) e ImPlot (gráficos).
- **Panel lateral**: la configuración de la escena (titulo, autor, layout, número de nodos).
- **Auto-refresh**: cada 2 segundos (configurable) hace `GET /api/grafo` y re-renderiza.

### Per-node renderers

Cada tipo de nodo se renderiza con su función específica:

| Tipo | Renderer | Descripción |
|------|----------|-------------|
| `metric` | `render_metric` | Texto grande con valor y unidad |
| `text` | `render_text` | Texto envuelto |
| `line_plot` | `ImPlot::PlotLine` | Línea conectando puntos |
| `bar_chart` | `ImPlot::PlotBars` | Barras verticales |
| `scatter` | `ImPlot::PlotScatter` | Puntos individuales |
| `histogram` | `ImPlot::PlotHistogram` | Distribución por bins |
| `box_plot` | Custom (PlotErrorBars + PlotLine) | Q1/Q3 box, mediana, whiskers |
| `linear_regression` | Scatter + fit line + equation | Puntos + línea de ajuste |
| `heatmap` | Fallback a scatter | (work in progress) |

### Tema oscuro

Fondo `#1e1e1e`, paneles `#252526`, texto `#d4d4d4`, acento azul `#007acc`. Configurable en el código (`src/renderer/main.cpp`).

## 12.2. `zeta_term` — ANSI 24-bit

### Compilar

```bash
./build.sh term
```

Solo requiere:
- libstdc++
- Header `nlohmann/json.hpp` (en `deps/`)

**No** requiere OpenGL, GLFW, ImGui. Es completamente portable.

### Ejecutar

```bash
./zeta_term --host localhost --port 8080 --width 120
```

Flags:
- `--host HOST`: servidor (default `localhost`).
- `--port N`: puerto (default `8080`).
- `--file RUTA`: en lugar de consultar el server, lee un JSON local.
- `--width N`: ancho forzado (default: autodetect via `ioctl(TIOCGWINSZ)` o `$COLUMNS`).
- `--no-color`: desactiva ANSI 24-bit (para terminales sin soporte).

### Autodetección de ancho

```cpp
struct winsize w;
if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) return w.ws_col;
const char* cols = std::getenv("COLUMNS");
if (cols) { int n = atoi(cols); if (n > 0) return n; }
return 120;  // fallback
```

Esto significa que **se redimensiona automáticamente** si cambias el tamaño de la terminal (aunque requiere re-ejecutar el binario).

### Salida de ejemplo

```
 Zeta Dashboard  -  Analisis de Ventas  (zeta, 10 nodos) 
+Ventas totales----------------------------------+ +Gastos totales----------------------------------+
|                                                | |                                                |
|                                                | |                                                |
|                                                | |                                                |
|                                                | |                                                |
|                                                | |                                                |
|                                                | |                                                |
|                                                | |                                                |
|                   17700.00 $                   | |                   7250.00 $                    |
|                                                | |                                                |
|                                                ...   ... (continúa)
+------------------------------------------------+ +------------------------------------------------+
+Tendencia de ventas-----------------------------+ +Gastos mensuales--------------------------------+
|                                *            ***| |                                    ####        |
|                               * **      ****   | |                                    ####        |
|                    ****     **    **  **       | |                        ####        ####        |
...
```

Con color (24-bit ANSI), los números se ven en azul/verde/rojo según el color del `add_metric`, y los plots en azul. Sin color (`--no-color`), todo se ve en escala de grises con caracteres ASCII.

### Per-node renderers (ASCII)

| Tipo | Implementación |
|------|----------------|
| `metric` | Valor centrado con unidad (e.g., `"17700.00 $"`) |
| `text` | Texto envuelto en líneas |
| `line_plot` | Bresenham line drawing con `*` |
| `bar_chart` | `#` apilados verticalmente |
| `scatter` | `*` en cada punto |
| `histogram` | `#` por bin (densidad) |
| `box_plot` | Caja `[ ]` con mediana `=` y whiskers `+` |
| `linear_regression` | Línea naranja `-` + puntos azules `*` |
| `heatmap` | (no soportado, muestra `?`) |

### Layout

Grid con bordes ASCII:

- `+---+---+` para los bordes.
- `|` para lados verticales.
- `-` para lados horizontales.
- Título del nodo en el borde superior.
- `gap` se ignora y siempre es 2 chars.
- `cell_h` siempre es 14 filas.

### SSH-friendly

```bash
# Desde tu laptop
ssh usuario@server './zeta_term --host localhost --port 8080 --width 100 --no-color'
```

Sin X11, sin OpenGL, sin GPU. Solo necesitas una terminal que soporte UTF-8 (que es básicamente cualquier terminal moderna, incluyendo `tmux`, `screen`, `mosh`).

## 12.3. Comparación lado a lado

| Característica | `zeta_dashboard` | `zeta_term` |
|----------------|------------------|-------------|
| Tamaño binario | 3.6 MB | 234 KB |
| Deps runtime | libGL, libGLEW, libX11 | Solo libc |
| Velocidad de render | 60 FPS (con auto-refresh 2s) | Instantáneo (single render) |
| Calidad visual | Alta (anti-aliasing, texto claro) | Baja (ASCII) |
| Soporte de nodos | 9 (todos) | 8 (heatmap faltante) |
| Interactividad | Zoom, pan, hover (ImPlot) | Solo lectura |
| Screenshots | Sí (PNG) | No (pero puedes pipear a archivo) |
| Headless servers | No | Sí |
| Mobile/SSH | No | Sí |
| Auto-resize | Sí (GLFW window) | No (requiere re-ejec) |

## 12.4. Cuál usar

- **Desarrollo local con monitor**: `zeta_dashboard`.
- **Demo a un cliente vía SSH**: `zeta_term`.
- **CI/CD que genera reportes**: `zeta_dashboard --screenshot`.
- **Server headless que solo quiere logs visuales**: `zeta_term --no-color > log.txt`.
- **Laptop sin X11 (e.g., Mac con SSH)**: `zeta_term`.

## 12.5. Extender: añadir un nuevo renderer

El contrato es simple: hacer `GET /api/grafo`, parsear el JSON, renderizar. Pasos para un nuevo consumidor:

1. Compilar contra `deps/json.hpp` (header-only, no requiere link).
2. Definir structs que reflejen el schema de `SceneNode` (ver [Escenas](./docs/09-escenas-visualizacion.md#el-json-de-escena)).
3. Implementar un dispatcher por `node.tipo` (switch o map).
4. Conectar al servidor y hacer polling.

Tiempo estimado para un renderer básico: **1-2 días** para alguien con experiencia en C++/gráficos.

## 12.6. Limitaciones comunes

- **Ambos renderers son single-threaded**: no hay composición paralela.
- **Sin animaciones**: la escena se re-renderiza completa cada vez, no hay transiciones.
- **Sin interactividad para modificar**: ambos son solo-lectura. Para modificar la escena, usa `zeta_server` con `POST /api/run`.
- **Sin persistencia de la sesión**: cada ejecución del renderer es independiente.
