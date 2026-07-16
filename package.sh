#!/bin/bash
# package.sh - Crea paquetes de distribución de Zeta por plataforma
# Uso: ./package.sh [version]
#
# Genera:
#   dist/zeta-linux-x64.tar.gz
#   dist/zeta-macos-arm64.tar.gz (en macOS ARM)
#   dist/zeta-windows-x64.zip (solo binarios, sin dashboard)

set -e
cd "$(dirname "$0")"

VERSION="${1:-0.1.1}"
OS=$(uname -s | tr '[:upper:]' '[:lower:]')
ARCH=$(uname -m)

case "$ARCH" in
    x86_64|amd64)  ARCH_NAME="x64" ;;
    aarch64|arm64) ARCH_NAME="arm64" ;;
    *)             ARCH_NAME="$ARCH" ;;
esac

PKG_NAME="zeta-${OS}-${ARCH_NAME}"
DIST="dist/${PKG_NAME}"

echo "=== Packaging Zeta v${VERSION} (${OS}-${ARCH_NAME}) ==="

# 0. Build
echo "[1/6] Building binaries..."
./build.sh cli 2>&1 | tail -1
./build.sh server 2>&1 | tail -1
./build.sh term 2>&1 | tail -1
if [ "$OS" = "linux" ]; then
    ./build.sh dashboard 2>&1 | tail -1 || true
fi

# 1. Clean dist
rm -rf dist
mkdir -p "${DIST}/bin"
mkdir -p "${DIST}/lib"
mkdir -p "${DIST}/examples"
mkdir -p "${DIST}/tests"
mkdir -p "${DIST}/docs"

# 2. Copy binaries
echo "[2/6] Copying binaries..."
for bin in zeta zeta_server zeta_term; do
    if [ -f "$bin" ]; then
        cp "$bin" "${DIST}/bin/"
        chmod +x "${DIST}/bin/$bin"
    fi
done
# Dashboard only on Linux
if [ -f "zeta_dashboard" ] && [ "$OS" = "linux" ]; then
    cp zeta_dashboard "${DIST}/bin/"
    chmod +x "${DIST}/bin/zeta_dashboard"
fi

# 3. Copy libs
echo "[3/6] Copying libraries..."
cp lib/*.zl "${DIST}/lib/" 2>/dev/null || true

# 4. Copy examples
cp tests/dashboard_scene.zl "${DIST}/examples/" 2>/dev/null || true
cp tests/datos.csv "${DIST}/examples/" 2>/dev/null || true
cp tests/test_import.zl "${DIST}/examples/" 2>/dev/null || true

# 5. Copy docs
cp docs/*.md "${DIST}/docs/" 2>/dev/null || true

# 6. Copy installer scripts
echo "[4/6] Copying installers..."
cp install.sh "${DIST}/" 2>/dev/null && chmod +x "${DIST}/install.sh" || true
cp uninstall.sh "${DIST}/" 2>/dev/null && chmod +x "${DIST}/uninstall.sh" || true
cp install.ps1 "${DIST}/" 2>/dev/null || true
cp uninstall.ps1 "${DIST}/" 2>/dev/null || true

# 7. Generate README
echo "[5/6] Generating README..."
cat > "${DIST}/README.md" <<EOF
# Zeta Language v${VERSION} (${OS}-${ARCH_NAME})

Lenguaje de análisis y transformación de datos con sintaxis híbrida Rust/Python.

## Instalación rápida

### Linux / macOS
\`\`\`bash
chmod +x install.sh
./install.sh
\`\`\`

### Windows (PowerShell)
\`\`\`powershell
Set-ExecutionPolicy -Scope Process -ExecutionPolicy Bypass
.\install.ps1
\`\`\`

## Binarios incluidos

- \`bin/zeta\` — Intérprete CLI
- \`bin/zeta_server\` — Servidor HTTP REST
- \`bin/zeta_term\` — Renderer ANSI de terminal
$(if [ "$OS" = "linux" ]; then echo "- \`bin/zeta_dashboard\` — Renderer OpenGL (Linux only)"; fi)

## Uso rápido

\`\`\`bash
zeta --help
zeta script.zl
zeta_server --port 8080
zeta_term --host localhost
\`\`\`

## API REST (zeta_server)

- \`GET /\` — UI web
- \`GET /api/datos\` — Variables como JSON
- \`GET /api/metricas\` — Métricas KPI
- \`POST /api/run\` — Ejecutar código Zeta

## Mas información

Ver \`docs/\` y \`examples/\` para más detalles.
EOF

# 8. Create archive
echo "[6/6] Creating archive..."
cd dist

if [ "$OS" = "windows" ] || [ "${CREATE_ZIP:-0}" = "1" ]; then
    zip -r "${PKG_NAME}.zip" "${PKG_NAME}/" 2>/dev/null || tar czf "${PKG_NAME}.tar.gz" "${PKG_NAME}/"
    ARCHIVE="${PKG_NAME}.tar.gz"
else
    tar czf "${PKG_NAME}.tar.gz" "${PKG_NAME}/"
    ARCHIVE="${PKG_NAME}.tar.gz"
fi

cd ..

echo
echo "=== OK ==="
echo "Paquete: dist/${ARCHIVE}"
du -sh "dist/${PKG_NAME}/"
ls -la "dist/${PKG_NAME}/bin/"
