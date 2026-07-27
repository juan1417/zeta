#!/bin/bash
# Zeta Language Installer
# Supports Linux (x86_64, aarch64) and macOS (x86_64, arm64)
#
# Usage:
#   ./install.sh                    # Install to /usr/local
#   ./install.sh /home/user/.local  # Install to custom prefix
#   PREFIX=/opt/zeta ./install.sh   # Install via env var
#   ./install.sh --help             # Show help

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

info()  { echo -e "${BLUE}[INFO]${NC} $1"; }
ok()    { echo -e "${GREEN}[OK]${NC} $1"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; exit 1; }

show_help() {
    echo -e "${BLUE}Zeta Language Installer${NC}"
    echo
    echo "Uso: ./install.sh [opciones] [prefix]"
    echo
    echo "Opciones:"
    echo "  --help, -h          Mostrar esta ayuda"
    echo "  --no-build          No compilar, solo instalar binarios existentes"
    echo "  --no-dashboard      No compilar el dashboard (requiere X11/GL)"
    echo
    echo "Ejemplos:"
    echo "  ./install.sh                         Instalar en /usr/local"
    echo "  ./install.sh ~/.local                Instalar en ~/.local"
    echo "  PREFIX=/opt/zeta ./install.sh        Instalar en /opt/zeta"
    echo "  ./install.sh --no-build ~/zeta       Solo instalar, sin compilar"
    echo
    echo "Despues de instalar, agregua al PATH:"
    echo "  export PATH=\"\$HOME/.local/bin:\$PATH\""
    echo
}

# Parse arguments
NO_BUILD=false
NO_DASHBOARD=false
PREFIX_ARG=""

while [[ $# -gt 0 ]]; do
    case $1 in
        --help|-h)
            show_help
            exit 0
            ;;
        --no-build)
            NO_BUILD=true
            shift
            ;;
        --no-dashboard)
            NO_DASHBOARD=true
            shift
            ;;
        -*)
            error "Opcion desconocida: $1 (usa --help para ver opciones)"
            ;;
        *)
            PREFIX_ARG="$1"
            shift
            ;;
    esac
done

# Detect OS and arch
detect_platform() {
    OS=$(uname -s | tr '[:upper:]' '[:lower:]')
    ARCH=$(uname -m)

    case "$OS" in
        linux)   PLATFORM="linux" ;;
        darwin)  PLATFORM="macos" ;;
        *)       error "Unsupported OS: $OS" ;;
    esac

    case "$ARCH" in
        x86_64|amd64)  ARCH_NAME="x64" ;;
        aarch64|arm64) ARCH_NAME="arm64" ;;
        *)             error "Unsupported architecture: $ARCH" ;;
    esac

    info "Detected: ${PLATFORM}-${ARCH_NAME}"
}

# Check dependencies
check_deps() {
    local missing=()

    if ! command -v clang++ &>/dev/null; then
        if command -v g++ &>/dev/null; then
            warn "clang++ not found, using g++"
            CXX="g++"
        else
            error "Neither clang++ nor g++ found. Install with:\n  Linux: sudo apt install clang\n  macOS: xcode-select --install"
        fi
    else
        CXX="clang++"
    fi

    if ! command -v cc &>/dev/null && ! command -v gcc &>/dev/null; then
        warn "C compiler (cc/gcc) not found, will skip miniz compilation"
    fi

    info "Using compiler: $($CXX --version 2>&1 | head -1)"
}

# Set install prefix
set_prefix() {
    PREFIX="${1:-${PREFIX:-/usr/local}}"
    BIN_DIR="${PREFIX}/bin"
    LIB_DIR="${PREFIX}/share/zeta/lib"
    DOC_DIR="${PREFIX}/share/zeta/docs"

    # Check if we need sudo
    SUDO=""
    if [ ! -w "$(dirname "$BIN_DIR")" ] 2>/dev/null; then
        if command -v sudo &>/dev/null; then
            SUDO="sudo"
            info "Need elevated permissions for ${PREFIX}"
            sudo -v 2>/dev/null || error "Cannot obtain sudo privileges"
        else
            error "No write permission to ${PREFIX} and sudo is not available.\n  Try: $0 ~/.local"
        fi
    fi

    info "Install prefix: ${PREFIX}"
}

# Build binaries
build_binaries() {
    local SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    cd "$SCRIPT_DIR"

    if [ "$NO_BUILD" = true ]; then
        info "Skipping build (--no-build)"
        cd - >/dev/null
        return
    fi

    info "Building Zeta binaries..."

    # Build CLI
    info "  Compiling zeta (CLI)..."
    if ./build.sh cli 2>&1 | tail -1; then
        ok "  zeta built"
    else
        warn "  zeta build failed"
    fi

    # Build server
    info "  Compiling zeta_server (HTTP)..."
    if ./build.sh server 2>&1 | tail -1; then
        ok "  zeta_server built"
    else
        warn "  zeta_server build failed"
    fi

    # Build term renderer
    info "  Compiling zeta_term (terminal)..."
    if ./build.sh term 2>&1 | tail -1; then
        ok "  zeta_term built"
    else
        warn "  zeta_term build failed"
    fi

    # Build LSP
    info "  Compiling zeta-lsp (Language Server)..."
    if ./build.sh lsp 2>&1 | tail -1; then
        ok "  zeta-lsp built"
    else
        warn "  zeta-lsp build failed"
    fi

    # Dashboard only on Linux (requires X11)
    if [ "$PLATFORM" = "linux" ] && [ "$NO_DASHBOARD" = false ]; then
        info "  Compiling zeta_dashboard (OpenGL)..."
        if ./build.sh dashboard 2>&1 | tail -1; then
            ok "  zeta_dashboard built"
        else
            warn "  zeta_dashboard build failed (missing X11/GL libs?)"
            warn "  Install: sudo apt install libglfw3-dev libglew-dev libgl-dev"
        fi
    elif [ "$NO_DASHBOARD" = true ]; then
        info "  Skipping dashboard (--no-dashboard)"
    fi

    cd - >/dev/null
}

# Install files
install_files() {
    local SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    cd "$SCRIPT_DIR"

    # Create directories
    info "Creating directories..."
    $SUDO mkdir -p "$BIN_DIR"
    $SUDO mkdir -p "$LIB_DIR"
    $SUDO mkdir -p "$DOC_DIR"

    # Install binaries
    info "Installing binaries to ${BIN_DIR}..."
    for bin in zeta zeta_server zeta_term zeta-lsp; do
        if [ -f "$bin" ]; then
            $SUDO cp "$bin" "$BIN_DIR/"
            $SUDO chmod +x "$BIN_DIR/$bin"
            ok "  ${bin} -> ${BIN_DIR}/${bin}"
        fi
    done

    # Dashboard (if built)
    if [ -f "zeta_dashboard" ]; then
        $SUDO cp "zeta_dashboard" "$BIN_DIR/"
        $SUDO chmod +x "$BIN_DIR/zeta_dashboard"
        ok "  zeta_dashboard -> ${BIN_DIR}/zeta_dashboard"
    fi

    # Install .zl library files
    info "Installing Zeta libraries to ${LIB_DIR}..."
    if ls lib/*.zl 2>/dev/null | head -1 >/dev/null 2>&1; then
        $SUDO cp lib/*.zl "$LIB_DIR/" 2>/dev/null || true
        ok "  Library files installed"
    else
        warn "  No .zl library files found"
    fi

    # Install docs
    info "Installing documentation to ${DOC_DIR}..."
    if ls docs/*.md 2>/dev/null | head -1 >/dev/null 2>&1; then
        $SUDO cp docs/*.md "$DOC_DIR/" 2>/dev/null || true
        ok "  Documentation installed"
    fi

    cd - >/dev/null
}

# Verify installation
verify_install() {
    info "Verifying installation..."

    local errors=0
    for bin in zeta zeta_server zeta_term zeta-lsp; do
        if [ -x "${BIN_DIR}/${bin}" ]; then
            ok "  ${bin} OK"
        else
            warn "  ${bin} not found"
            errors=$((errors + 1))
        fi
    done

    if [ "$errors" -eq 0 ]; then
        echo
        echo -e "${GREEN}=== Installation complete! ===${NC}"
        echo
        echo "Installed to: ${PREFIX}"
        echo "Binaries:     ${BIN_DIR}"
        echo "Libraries:    ${LIB_DIR}"
        echo
        echo "Quick start:"
        echo "  zeta --help                    # Show CLI options"
        echo "  zeta script.zl                 # Run a script"
        echo "  zeta_server --port 8080        # Start HTTP server"
        echo "  zeta_term --host localhost      # Terminal renderer"
        if [ "$PLATFORM" = "linux" ] && [ -x "${BIN_DIR}/zeta_dashboard" ]; then
            echo "  zeta_dashboard --host localhost # OpenGL dashboard"
        fi
        echo
        echo "Add to PATH (if not already):"
        echo "  export PATH=\"${BIN_DIR}:\$PATH\""
        echo "  echo 'export PATH=\"${BIN_DIR}:\$PATH\"' >> ~/.bashrc"
    fi
}

# Main
main() {
    echo -e "${BLUE}=== Zeta Language Installer ===${NC}"
    echo

    detect_platform
    check_deps
    set_prefix "$PREFIX_ARG"
    build_binaries
    install_files
    verify_install
}

main
