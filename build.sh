#!/bin/bash
# Build script for Zeta Language
# Usage:
#   ./build.sh              # Build main zeta binary
#   ./build.sh server       # Build zeta_server (HTTP server)
#   ./build.sh dashboard    # Build zeta_dashboard (native OpenGL renderer)
#   ./build.sh all          # Build everything

set -e
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

CORE_SRCS="src/core/valor_zeta.cpp src/core/tabla_simbolos.cpp src/core/estadisticas.cpp src/core/errores.cpp src/core/serializador.cpp src/core/grafo_json.cpp src/core/xlsx_reader.cpp src/core/arena.cpp src/core/distribuciones.cpp src/core/window_functions.cpp deps/pugixml/src/pugixml.cpp src/lexer/lexer.cpp src/parser/parser.cpp src/interpreter/interpreter.cpp src/dl_loader/dl_loader.cpp"
INCLUDES="-std=c++20 -I include -I ."
FLAGS="-Wno-deprecated-literal-operator -Wno-unused-variable -Wno-unused-but-set-variable -O2"

MINIZ_C_SRCS="deps/miniz/miniz.c deps/miniz/miniz_zip.c deps/miniz/miniz_tdef.c deps/miniz/miniz_tinfl.c"
OPENXLSX_INCLUDES="-I deps/pugixml/src -I deps/miniz"

build_zeta() {
    echo "[BUILD] Compiling zeta (CLI)..."
    mkdir -p build_tmp
    for f in $MINIZ_C_SRCS; do
        obj="build_tmp/$(basename $f .c).o"
        cc -c -O2 -I deps/miniz -o "$obj" "$f" 2>/dev/null
    done
    MINIZ_OBJS=$(ls build_tmp/miniz*.o 2>/dev/null | tr '\n' ' ')
    clang++ $INCLUDES $FLAGS $OPENXLSX_INCLUDES -o zeta src/main.cpp $CORE_SRCS $MINIZ_OBJS -lz 2>&1
    echo "[BUILD] OK -> ./zeta"
}

build_server() {
    echo "[BUILD] Compiling zeta_server (HTTP)..."
    mkdir -p build_tmp
    for f in $MINIZ_C_SRCS; do
        obj="build_tmp/$(basename $f .c).o"
        cc -c -O2 -I deps/miniz -o "$obj" "$f" 2>/dev/null
    done
    MINIZ_OBJS=$(ls build_tmp/miniz*.o 2>/dev/null | tr '\n' ' ')
    clang++ $INCLUDES $FLAGS -I deps/asio -DASIO_STANDALONE $OPENXLSX_INCLUDES -o zeta_server \
        src/server_main.cpp $CORE_SRCS $MINIZ_OBJS -lpthread -lz 2>&1
    echo "[BUILD] OK -> ./zeta_server"
}

build_dashboard() {
    echo "[BUILD] Compiling zeta_dashboard (native OpenGL renderer)..."
    clang++ -std=c++20 -O2 \
        -I . \
        -I deps/glfw-build/include \
        -I deps/imgui \
        -I deps/imgui/backends \
        -I deps/implot \
        -o zeta_dashboard \
        src/renderer/main.cpp \
        deps/glfw-build/lib/libglfw3.a \
        deps/imgui/imgui.cpp \
        deps/imgui/imgui_draw.cpp \
        deps/imgui/imgui_tables.cpp \
        deps/imgui/imgui_widgets.cpp \
        deps/imgui/backends/imgui_impl_glfw.cpp \
        deps/imgui/backends/imgui_impl_opengl3.cpp \
        deps/implot/implot.cpp \
        deps/implot/implot_items.cpp \
        -lGLEW -lGL -lX11 -lpthread -ldl -lm 2>&1
    echo "[BUILD] OK -> ./zeta_dashboard"
}

build_term() {
    echo "[BUILD] Compiling zeta_term (terminal renderer, no OpenGL)..."
    clang++ -std=c++20 -O2 \
        -I . \
        -o zeta_term \
        src/term/main.cpp 2>&1
    echo "[BUILD] OK -> ./zeta_term"
}

build_lsp() {
    echo "[BUILD] Compiling zeta-lsp (Language Server)..."
    mkdir -p build_tmp
    for f in $MINIZ_C_SRCS; do
        obj="build_tmp/$(basename $f .c).o"
        cc -c -O2 -I deps/miniz -o "$obj" "$f" 2>/dev/null
    done
    MINIZ_OBJS=$(ls build_tmp/miniz*.o 2>/dev/null | tr '\n' ' ')
    clang++ -std=c++20 -O2 \
        -I include -I . -I deps \
        -Wno-deprecated-literal-operator -Wno-unused-variable -Wno-unused-but-set-variable \
        $OPENXLSX_INCLUDES \
        -o zeta-lsp \
        lsp/zeta-lsp.cpp lsp/transport.cpp lsp/builtins.cpp lsp/analyzer.cpp \
        src/lexer/lexer.cpp src/parser/parser.cpp src/core/valor_zeta.cpp \
        src/core/errores.cpp src/core/estadisticas.cpp $MINIZ_OBJS -lz 2>&1
    echo "[BUILD] OK -> ./zeta-lsp"
}

case "${1:-cli}" in
    cli|zeta)    build_zeta ;;
    server)      build_server ;;
    dashboard|renderer) build_dashboard ;;
    term|terminal) build_term ;;
    lsp)         build_lsp ;;
    all)
        build_zeta
        build_server
        build_dashboard
        build_term
        build_lsp
        ;;
    clean)
        rm -f zeta zeta_server zeta_dashboard zeta_term zeta-lsp
        echo "[CLEAN] Removed binaries"
        ;;
    *)
        echo "Usage: $0 [cli|server|dashboard|term|lsp|all|clean]"
        exit 1
        ;;
esac
