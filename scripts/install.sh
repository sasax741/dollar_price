#!/bin/sh
# Install script for Dollar Price Checker
# Usage: curl -sSL https://raw.githubusercontent.com/sasax741/dollar_price/main/scripts/install.sh | sh
#   or:  ./scripts/install.sh [--prefix=/usr/local]

set -e

REPO="sasax741/dollar_price"
BINARY="dollar"
VERSION="${VERSION:-latest}"

# Colors
RED='\033[0;31m'; GREEN='\033[0;32m'; NC='\033[0m'
info() { printf "${GREEN}%s${NC}\n" "$*"; }
err()  { printf "${RED}%s${NC}\n" "$*" >&2; exit 1; }

# Default prefix
PREFIX="${PREFIX:-/usr/local}"

# Parse args
for arg in "$@"; do
    case "$arg" in
        --prefix=*) PREFIX="${arg#*=}" ;;
        --help) echo "Usage: $0 [--prefix=/usr/local]"; exit 0 ;;
    esac
done

# Detect platform
OS="$(uname -s)"
ARCH="$(uname -m)"

case "$OS" in
    Linux)  OS="linux" ;;
    Darwin) OS="darwin" ;;
    *)      err "Unsupported OS: $OS" ;;
esac

case "$ARCH" in
    x86_64|amd64) ARCH="amd64" ;;
    aarch64|arm64) ARCH="arm64" ;;
    *) err "Unsupported architecture: $ARCH" ;;
esac

# If latest, resolve the tag from GitHub API
if [ "$VERSION" = "latest" ]; then
    info "Fetching latest release..."
    LATEST_URL="https://api.github.com/repos/${REPO}/releases/latest"
    VERSION=$(curl -sL "$LATEST_URL" | grep '"tag_name"' | cut -d'"' -f4)
    [ -z "$VERSION" ] && err "Could not determine latest version"
    info "Latest version: $VERSION"
fi

install_from_source() {
    info "Building from source..."
    TMPDIR=$(mktemp -d)
    cd "$TMPDIR"
    curl -sL "https://github.com/${REPO}/archive/refs/tags/${VERSION}.tar.gz" | tar xz --strip-components=1
    
    # Check dependencies
    for dep in gcc make pkg-config; do
        command -v "$dep" >/dev/null 2>&1 || err "Missing dependency: $dep"
    done
    command -v curl-config >/dev/null 2>&1 || err "Missing dependency: libcurl-dev"
    pkg-config --exists jansson 2>/dev/null || err "Missing dependency: libjansson-dev"

    make
    PREFIX="$PREFIX" make install
    cd / && rm -rf "$TMPDIR"
    info "Installed from source to $PREFIX/bin/$BINARY"
}

install_from_binary() {
    info "Downloading binary..."
    
    ASSET="dollar-${OS}-${ARCH}"
    URL="https://github.com/${REPO}/releases/download/${VERSION}/${ASSET}"
    CHECKSUM_URL="${URL}.sha256"
    
    TMPDIR=$(mktemp -d)
    cd "$TMPDIR"
    
    curl -sLO "$URL"
    curl -sLO "$CHECKSUM_URL"
    
    # Verify checksum
    sha256sum -c "${ASSET}.sha256" 2>/dev/null || {
        if command -v shasum >/dev/null 2>&1; then
            shasum -a 256 -c "${ASSET}.sha256" 2>/dev/null
        fi
    } || err "Checksum verification failed"
    
    chmod +x "$ASSET"
    
    # Install
    BINDIR="${PREFIX}/bin"
    mkdir -p "$BINDIR"
    cp "$ASSET" "${BINDIR}/${BINARY}"
    
    cd / && rm -rf "$TMPDIR"
    info "Installed $BINARY to $BINDIR"
}

# Linux: binary, macOS/arm64: build from source
if [ "$OS" = "linux" ] && [ "$ARCH" = "amd64" ]; then
    install_from_binary
elif [ "$OS" = "darwin" ] || [ "$ARCH" = "arm64" ]; then
    install_from_source
else
    install_from_source
fi

# Verify
"$BINARY" --help >/dev/null 2>&1 || err "Installation verification failed"
info "✅ Dollar Price Checker $VERSION installed successfully!"
info "Run '${BINARY}' to check dollar rates."
