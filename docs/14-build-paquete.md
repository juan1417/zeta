# 14. Build y empaquetado

## 14.1. `build.sh`: compilar todo o por partes

```bash
./build.sh all
```

Targets disponibles:

| Target | Produce | Dependencias |
|--------|---------|--------------|
| `cli` | `zeta` | libstdc++ |
| `server` | `zeta_server` | libstdc++ + Crow + Asio (header-only) |
| `dashboard` | `zeta_dashboard` | libstdc++ + libGL + libGLEW + libX11 + GLFW estático + ImGui + ImPlot |
| `term` | `zeta_term` | libstdc++ + nlohmann/json (header-only) |
| `all` | los 4 anteriores | todas |
| `clean` | — (borra binarios) | — |

### Ejemplos

```bash
# Compilar solo el CLI (rápido, para iterar scripts)
./build.sh cli

# Compilar el server (útil para API)
./build.sh server

# Compilar el dashboard (necesita GLFW ya compilado)
./build.sh dashboard

# Compilar todo
./build.sh all

# Limpiar
./build.sh clean
```

### Flags de compilación

Cada target usa `clang++ -std=c++20 -O2 -Wall`. El `build.sh` añade los includes necesarios:

- **Todos**: `-I include -I .`
- **Server**: nada extra (Crow está en `deps/`)
- **Dashboard**: `-I deps/glfw-build/include -I deps/imgui -I deps/imgui/backends -I deps/implot`
- **Term**: nada extra (nlohmann/json en `deps/`)

Y los links:

- **CLI**: nada
- **Server**: nada (header-only)
- **Dashboard**: `deps/glfw-build/lib/libglfw3.a` + sources de ImGui/ImPlot + `-lGLEW -lGL -lX11 -lpthread -ldl -lm`
- **Term**: nada

## 14.2. Compilar las dependencias externas

### GLFW 3.4 (solo la primera vez)

`deps/glfw-build/` contiene el resultado de la compilación manual de GLFW (sin CMake, para evitar overhead):

```bash
cd deps
wget https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip
unzip glfw-3.4
cd glfw-3.4

# Generar mappings.h (reemplaza @GLFW_*_MAPPINGS@)
sed -i 's/@GLFW_XKB_MAPPINGS@/.../' src/mappings.h.in > src/mappings.h

# Compilar manualmente
clang++ -D_GLFW_X11 -D_GLFW_GLX -D_GLFW_BUILD \
    -fPIC -O2 -c src/*.c -I include -I src
ar rcs lib/libglfw3.a *.o

# Copiar a deps/glfw-build/
cp lib/libglfw3.a ../glfw-build/lib/
cp -r include/* ../glfw-build/include/

# Crear shim de GLAD (porque ImGui espera glad/glad.h)
mkdir -p ../glfw-build/include/glad
echo '#include <GL/glew.h>' > ../glfw-build/include/glad/glad.h
```

Si ya existe `deps/glfw-build/lib/libglfw3.a`, no necesitas recompilar.

### ImGui 1.91.8

Clonar o descargar el release:

```bash
cd deps
git clone https://github.com/ocornut/imgui.git
cd imgui
git checkout v1.91.8
```

Necesitas `backends/` también:

```bash
git clone https://github.com/ocornut/imgui.git
# backends/ viene con el repo
```

Y `stb_image_write.h` (para screenshots):

```bash
wget https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h
mv stb_image_write.h deps/imgui/
```

### ImPlot 0.16

```bash
cd deps
git clone https://github.com/epezent/implot.git
cd implot
git checkout v0.16
```

### Crow

Header-only:

```bash
cd deps
wget https://github.com/CrowCpp/Crow/releases/download/v1.2.0/crow_all.h
```

### Asio

Header-only (vía el submodule de Crow, o standalone):

```bash
cd deps
wget https://sourceforge.net/projects/asio/files/asio/1.28.0/asio-1.28.0.tar.gz
tar xf asio-1.28.0.tar.gz
# asio/include/ es lo que necesitamos
```

### nlohmann/json

```bash
cd deps
wget https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
```

## 14.3. Compilar la librería de ejemplo

```bash
clang++ -std=c++20 -shared -fPIC -fvisibility=hidden \
    -I include -o lib/libtestnative.so lib/test_lib.cpp
```

## 14.4. `test_e2e.sh`: test end-to-end

```bash
./test_e2e.sh
```

Pasos:

1. **Build all** (`./build.sh all`).
2. **Generar escena**: `./zeta tests/dashboard_scene.zl` → produce `tests/dashboard_scene.json` con 10 nodos.
3. **Iniciar server**: `./zeta_server --port 8095 &`.
4. **POST /api/run** con el contenido del `.zl`.
5. **GET /api/grafo**: verifica que el server reporta 10 nodos.
6. **Screenshot** vía `zeta_dashboard --screenshot ...`.
7. **Terminal render** vía `zeta_term` y verifica títulos de nodos.
8. **Cleanup**: mata el server.

PASS si:

- `NODES == API_NODES` (10 == 10)
- `SCREENSHOT` existe
- `TERM_TITLES > 0`

## 14.5. `package.sh`: crear distribución

```bash
./package.sh 0.1.0
```

Crea `dist/zeta-0.1.0/` con:

```
dist/zeta-0.1.0/
├── bin/
│   ├── zeta              (CLI, ~640 KB)
│   ├── zeta_server       (HTTP server, ~1.2 MB)
│   ├── zeta_dashboard    (OpenGL renderer, ~3.6 MB)
│   └── zeta_term         (ANSI renderer, ~234 KB)
├── lib/
│   ├── libtestnative.so  (librería de ejemplo)
│   └── test_lib.cpp      (fuente)
├── examples/
│   ├── dashboard_scene.zl
│   ├── datos.csv
│   └── test_import.zl
├── tests/
│   ├── test_*.zl
│   ├── test_*.zeta
│   └── datos.csv
├── docs/
│   └── README.md (placeholder)
├── README.md
└── start.sh              (script para iniciar todo)
```

Y lo comprime en `dist/zeta-0.1.0.tar.gz` (~5.5 MB).

### Uso del paquete

```bash
# Extraer
tar xzf zeta-0.1.0.tar.gz
cd zeta-0.1.0

# Iniciar todo (server + dashboard)
./start.sh

# O solo el server
./bin/zeta_server --port 8080

# O el term renderer
./bin/zeta_term --host localhost --port 8080

# Compilar la librería de ejemplo
clang++ -std=c++20 -shared -fPIC -fvisibility=hidden \
    -I include -o lib/libtestnative.so lib/test_lib.cpp
```

## 14.6. Compilación cruzada

### Windows (desde Linux)

```bash
# Instalar MinGW
sudo apt install mingw-w64 g++-mingw-w64-x86-64

# Configurar
export CC=x86_64-w64-mingw32-gcc
export CXX=x86_64-w64-mingw32-g++

# Compilar (solo CLI y server, dashboard requiere más trabajo)
x86_64-w64-mingw32-g++ -std=c++20 -O2 \
    -I include -I . \
    -o zeta.exe src/main.cpp src/lexer/lexer.cpp src/parser/parser.cpp \
    src/interpreter/interpreter.cpp src/core/*.cpp
```

El dashboard OpenGL en Windows requiere GLFW precompilado para Windows y linkear contra `gdi32`, `opengl32`, etc. Es ~2 horas de trabajo la primera vez.

### macOS

```bash
# Instalar dependencias
brew install glm nlohmann-json glfw glew

# Compilar
clang++ -std=c++20 -O2 -I include -I . \
    -I /opt/homebrew/include \
    -L /opt/homebrew/lib \
    -o zeta src/main.cpp src/lexer/lexer.cpp src/parser/parser.cpp \
    src/interpreter/interpreter.cpp src/core/*.cpp
```

`zeta_dashboard` y `zeta_term` deberían funcionar en macOS con ajustes menores (ImGui ya tiene backend para Cocoa, GLFW soporta Cocoa).

## 14.7. CMake (alternativa)

El proyecto incluye un `CMakeLists.txt` para los que prefieren CMake:

```bash
mkdir build && cd build
cmake ..
make
```

Targets:

- `zeta` (CLI)
- `zeta_server` (HTTP server)
- `zeta_dashboard` (renderer OpenGL)
- `zeta_term` (renderer ANSI)
- `test_native_lib` (librería de ejemplo)

**Limitación actual**: el `CMakeLists.txt` no está 100% sincronizado con `build.sh`. Si vas a usar CMake, verifica los paths.

## 14.8. CI/CD

### GitHub Actions ejemplo

```yaml
name: build
on: [push, pull_request]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install deps
        run: |
          sudo apt-get install -y libglew-dev libglfw3-dev libglu1-mesa-dev
      - name: Build
        run: ./build.sh all
      - name: Test
        run: ./test_e2e.sh
```

### Docker

```dockerfile
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y \
    clang libglew-dev libglfw3-dev libglu1-mesa-dev \
    libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev
WORKDIR /zeta
COPY . .
RUN ./build.sh all
EXPOSE 8080
CMD ["./zeta_server", "--port", "8080"]
```

## 14.9. Debug builds

Para debug:

```bash
# En build.sh, cambiar -O2 por -O0 -g
# O usar clang++ directamente:
clang++ -std=c++20 -O0 -g -fsanitize=address,undefined \
    -I include -I . \
    -o zeta_dbg src/main.cpp src/lexer/lexer.cpp src/parser/parser.cpp \
    src/interpreter/interpreter.cpp src/core/*.cpp
```

Con `-fsanitize=address,undefined` activas ASan (detecta leaks, out-of-bounds) y UBSan (detecta UB). Útil cuando sospechas bugs en C++.

Para profiling:

```bash
# Perf (Linux)
perf record -g ./zeta mi_script.zl
perf report

# O con callgrind
valgrind --tool=callgrind ./zeta mi_script.zl
```

## 14.10. Sistema de versiones

El proyecto sigue **Semantic Versioning** parcialmente:

- **Mayor**: cambios incompatibles en la sintaxis o ABI.
- **Menor**: nuevas funciones nativas, nuevos tipos de nodo.
- **Patch**: bugfixes, optimizaciones.

La versión actual está en el `package.sh` y en los mensajes de `start.sh`. No hay un archivo `VERSION` (considerado innecesario para el alcance actual).

## 14.11. Próximos pasos del build

- [ ] CMake completo y sincronizado.
- [ ] Build con `ninja` en lugar de `make`.
- [ ] Soporte oficial para Windows (MSVC + MinGW).
- [ ] Tests unitarios con Catch2 o GoogleTest.
- [ ] Integración con `vcpkg` o `conan` para dependencias.
