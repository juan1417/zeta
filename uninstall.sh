#!/bin/bash
# Zeta Language Uninstaller
#
# Usage:
#   ./uninstall.sh              # Uninstall from /usr/local
#   ./uninstall.sh /home/user/.local  # Uninstall from custom prefix

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

info()  { echo -e "${BLUE}[INFO]${NC} $1"; }
ok()    { echo -e "${GREEN}[OK]${NC} $1"; }
warn()  { echo -e "${YELLOW}[WARN]${NC} $1"; }

PREFIX="${1:-${PREFIX:-/usr/local}}"
BIN_DIR="${PREFIX}/bin"
LIB_DIR="${PREFIX}/share/zeta"
ZETA_DIR="${PREFIX}/share/zeta"

echo -e "${BLUE}=== Zeta Language Uninstaller ===${NC}"
echo
info "Install prefix: ${PREFIX}"

# Remove binaries
info "Removing binaries..."
for bin in zeta zeta_server zeta_dashboard zeta_term; do
    if [ -f "${BIN_DIR}/${bin}" ]; then
        rm -f "${BIN_DIR}/${bin}"
        ok "  Removed ${BIN_DIR}/${bin}"
    fi
done

# Remove Zeta directory
if [ -d "${ZETA_DIR}" ]; then
    info "Removing ${ZETA_DIR}..."
    rm -rf "${ZETA_DIR}"
    ok "  Removed ${ZETA_DIR}"
fi

echo
echo -e "${GREEN}=== Uninstall complete! ===${NC}"
