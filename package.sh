#!/bin/bash
# package.sh - Crea un paquete de distribución de Zeta
# Uso: ./package.sh [version]

set -e
cd "$(dirname "$0")"

VERSION="${1:-0.1.0}"
NAME="zeta-${VERSION}"
DIST="dist/${NAME}"

echo "=== Packaging Zeta v${VERSION} ==="

# 0. Build all
echo "[1/5] Building all binaries..."
./build.sh all 2>&1 | tail -4

# 1. Limpiar dist
rm -rf dist
mkdir -p "${DIST}/bin"
mkdir -p "${DIST}/lib"
mkdir -p "${DIST}/examples"
mkdir -p "${DIST}/tests"
mkdir -p "${DIST}/docs"

# 2. Copiar binarios
echo "[2/5] Copying binaries..."
cp zeta zeta_server zeta_dashboard zeta_term "${DIST}/bin/"
chmod +x "${DIST}/bin/"*

# 3. Copiar librerías y ejemplos
echo "[3/5] Copying libs and examples..."
cp lib/*.so lib/*.zl "${DIST}/lib/" 2>/dev/null || true
cp lib/test_lib.cpp "${DIST}/lib/" 2>/dev/null || true
cp tests/dashboard_scene.zl "${DIST}/examples/"
cp tests/datos.csv "${DIST}/examples/"
cp tests/test_import.zl "${DIST}/examples/"

# 4. Copiar tests
cp tests/test_*.zl "${DIST}/tests/" 2>/dev/null || true
cp tests/test_*.zeta "${DIST}/tests/" 2>/dev/null || true
cp tests/datos.csv "${DIST}/tests/" 2>/dev/null || true

# 5. Generar README
echo "[4/5] Generating README..."
cat > "${DIST}/README.md" <<EOF
# Zeta Language v${VERSION}

Lenguaje analítico con sintaxis similar a Rust/Python y motor de visualizacion nativo.

## Binarios

- `bin/zeta` — Intérprete CLI
- `bin/zeta_server` — Servidor HTTP con API REST (Crow)
- `bin/zeta_dashboard` — Renderer nativo OpenGL+ImGui+ImPlot (requiere X11)
- `bin/zeta_term` — Renderer ANSI de terminal (sin OpenGL)

## Uso rapido

\`\`\`bash
# Ejecutar un script
./bin/zeta examples/dashboard_scene.zl

# Iniciar el server (en una terminal)
./bin/zeta_server --port 8080

# Lanzar el dashboard (en otra terminal, requiere X11)
./bin/zeta_dashboard --host localhost --port 8080

# O el renderer de terminal (SSH-friendly, no requiere X11)
./bin/zeta_term --host localhost --port 8080
\`\`\`

## Compilar librerias nativas

\`\`\`bash
clang++ -std=c++20 -shared -fPIC -fvisibility=hidden \\
    -I include -o lib/libtestnative.so lib/test_lib.cpp
\`\`\`

## Sintaxis basica

\`\`\`
# Variables
\$datos = load_csv("datos.csv")

# Funciones nativas
add_metric("Total", sum(\$datos:ventas))
add_line_plot(\$datos, "Tendencia", "mes", "ventas")

# Importar librerias
include "statslib"
mean(\$datos:ventas)
\`\`\`

## API REST (zeta_server)

- \`GET /\` — UI HTML basica
- \`GET /api/datos\` — Variables globales como JSON
- \`GET /api/metricas\` — Metricas KPI
- \`GET /api/dashboard\` — Config dashboard
- \`GET /api/grafo\` — Scene spec para renderer
- \`POST /api/run\` — Ejecutar codigo Zeta (\`{"code": "..."}\`)

## Mas informacion

Ver archivos en \`tests/\` para ejemplos completos.
EOF

# 6. Script de inicio
cat > "${DIST}/start.sh" <<'EOF'
#!/bin/bash
cd "$(dirname "$0")"
PORT="${PORT:-8080}"

echo "Iniciando zeta_server en puerto $PORT..."
./bin/zeta_server --port $PORT &
SRV_PID=$!

sleep 1
echo "Iniciando zeta_dashboard..."
./bin/zeta_dashboard --host localhost --port $PORT

kill $SRV_PID 2>/dev/null
EOF
chmod +x "${DIST}/start.sh"

# 7. Tarball
echo "[5/5] Creating tarball..."
cd dist
tar czf "${NAME}.tar.gz" "${NAME}/"
ls -la "${NAME}.tar.gz"
echo
echo "=== OK ==="
echo "Paquete: dist/${NAME}.tar.gz"
du -sh "${NAME}/"
ls -la "${NAME}/bin/"