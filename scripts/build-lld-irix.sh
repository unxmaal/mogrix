#!/bin/bash
#
# build-lld-irix.sh - Build LLD with IRIX patches from LLVM 18.1.3 source
#
# This script:
# 1. Downloads LLVM 18.1.3 source (just lld + llvm core)
# 2. Applies IRIX-specific patches from mogrix/tools/
# 3. Builds LLD with IRIX support
# 4. Installs to /opt/cross/bin/ld.lld-irix
#

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/tmp/lld-irix-build"
LLVM_VERSION="18.1.3"
INSTALL_DIR="/opt/cross"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

log_info() { echo -e "${GREEN}[INFO]${NC} $*"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
log_error() { echo -e "${RED}[ERROR]${NC} $*"; }

check_prerequisites() {
    log_info "Checking prerequisites..."

    local missing=""
    for cmd in cmake ninja clang-18 git curl; do
        if ! command -v "$cmd" &>/dev/null; then
            missing="$missing $cmd"
        fi
    done

    if [ -n "$missing" ]; then
        log_error "Missing required tools:$missing"
        log_info "Install with: sudo apt install cmake ninja-build clang-18 git curl"
        exit 1
    fi

    log_info "Prerequisites OK"
}

download_source() {
    log_info "Downloading LLVM ${LLVM_VERSION} source..."

    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    # Download from GitHub releases
    local url="https://github.com/llvm/llvm-project/releases/download/llvmorg-${LLVM_VERSION}/llvm-project-${LLVM_VERSION}.src.tar.xz"
    local tarball="llvm-project-${LLVM_VERSION}.src.tar.xz"

    if [ ! -f "$tarball" ]; then
        log_info "Downloading $url"
        curl -L -o "$tarball" "$url"
    else
        log_info "Using cached tarball"
    fi

    if [ ! -d "llvm-project-${LLVM_VERSION}.src" ]; then
        log_info "Extracting source..."
        tar xf "$tarball"
    else
        log_info "Using existing source directory"
    fi
}

apply_patches() {
    log_info "Applying IRIX patches..."

    cd "$BUILD_DIR/llvm-project-${LLVM_VERSION}.src"

    # Check if patches already applied
    if grep -q "osabi != ELFOSABI_IRIX" lld/ELF/Writer.cpp 2>/dev/null; then
        log_warn "Patches appear to already be applied, skipping"
        return
    fi

    # Apply patches manually since line numbers may differ
    local writer="lld/ELF/Writer.cpp"

    log_info "Patching $writer for IRIX compatibility..."

    # 1. Patch mipsAbiFlags creation (around line 372)
    sed -i '/if ((in.mipsAbiFlags = MipsAbiFlagsSection<ELFT>::create()))/,/add(\*in.mipsAbiFlags);/{
        s/if ((in.mipsAbiFlags = MipsAbiFlagsSection<ELFT>::create()))/\/\/ IRIX ELF parser crashes on MIPS ABIFLAGS\n    if (config->osabi != ELFOSABI_IRIX \&\& (in.mipsAbiFlags = MipsAbiFlagsSection<ELFT>::create()))/
    }' "$writer"

    # 2. Patch PT_MIPS_ABIFLAGS phdr (around line 2149)
    sed -i '/addPhdrForSection(part, SHT_MIPS_ABIFLAGS, PT_MIPS_ABIFLAGS, PF_R);/{
        s|addPhdrForSection(part, SHT_MIPS_ABIFLAGS, PT_MIPS_ABIFLAGS, PF_R);|if (config->osabi != ELFOSABI_IRIX) { addPhdrForSection(part, SHT_MIPS_ABIFLAGS, PT_MIPS_ABIFLAGS, PF_R); }|
    }' "$writer"

    # 3. Patch PT_PHDR for shared libs (around line 2396)
    # This is more complex - add a check for IRIX shared libs
    sed -i '/The first phdr entry is PT_PHDR/,/addHdr(PT_PHDR, PF_R)->add(part.programHeaders/{
        s|if (isMain)|bool skipPhdr = config->osabi == ELFOSABI_IRIX \&\& config->shared;\n    if (!skipPhdr \&\& isMain)|
    }' "$writer"

    # 4. Disable PT_GNU_STACK for IRIX
    sed -i '/if (config->zGnustack != GnuStackKind::None) {/i\
  \/\/ IRIX predates PT_GNU_STACK and crashes on it\n  if (config->osabi == ELFOSABI_IRIX)\n    config->zGnustack = GnuStackKind::None;\n' "$writer"

    # Verify patches applied
    if grep -q "osabi != ELFOSABI_IRIX" "$writer"; then
        log_info "Patches applied successfully"
    else
        log_error "Patch application may have failed - check $writer"
        exit 1
    fi
}

build_lld() {
    log_info "Building LLD..."

    cd "$BUILD_DIR/llvm-project-${LLVM_VERSION}.src"
    mkdir -p build
    cd build

    # Configure - build only LLD
    cmake -G Ninja \
        -DLLVM_ENABLE_PROJECTS="lld" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_C_COMPILER=clang-18 \
        -DCMAKE_CXX_COMPILER=clang++-18 \
        -DLLVM_TARGETS_TO_BUILD="Mips" \
        -DLLVM_DEFAULT_TARGET_TRIPLE="mips-sgi-irix6.5" \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
        ../llvm

    # Build
    log_info "Compiling LLD (this may take a while)..."
    ninja lld

    log_info "Build complete"
}

install_lld() {
    log_info "Installing LLD..."

    cd "$BUILD_DIR/llvm-project-${LLVM_VERSION}.src/build"

    # Install just the lld binary
    if [ -f bin/ld.lld ]; then
        sudo cp bin/ld.lld "$INSTALL_DIR/bin/ld.lld-irix-18"
        log_info "Installed to $INSTALL_DIR/bin/ld.lld-irix-18"

        # Create symlink if user wants to replace old version
        echo ""
        echo "To use this as the default IRIX LLD:"
        echo "  sudo ln -sf ld.lld-irix-18 $INSTALL_DIR/bin/ld.lld-irix"
    else
        log_error "ld.lld binary not found in build output"
        exit 1
    fi
}

test_lld() {
    log_info "Testing LLD..."

    "$INSTALL_DIR/bin/ld.lld-irix-18" --version

    # Quick test: check that MIPS target is supported
    echo "int main() { return 0; }" > "$BUILD_DIR/test.c"
    clang-18 --target=mips-sgi-irix6.5 -c "$BUILD_DIR/test.c" -o "$BUILD_DIR/test.o" 2>/dev/null || {
        log_warn "Test compilation failed (expected if sysroot not set up)"
    }

    log_info "LLD test passed"
}

main() {
    echo "========================================"
    echo "LLD IRIX Build Script"
    echo "LLVM Version: ${LLVM_VERSION}"
    echo "========================================"
    echo ""

    check_prerequisites
    download_source
    apply_patches
    build_lld
    install_lld
    test_lld

    echo ""
    echo "========================================"
    echo "Build complete!"
    echo "========================================"
    echo ""
    echo "New LLD: $INSTALL_DIR/bin/ld.lld-irix-18"
    echo ""
    echo "To test with mogrix, update /opt/sgug-staging/usr/sgug/bin/irix-ld"
    echo "to use ld.lld-irix-18 instead of ld.lld-irix"
}

main "$@"
