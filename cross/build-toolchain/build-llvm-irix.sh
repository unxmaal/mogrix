#!/bin/bash
#
# build-llvm-irix.sh — Build complete LLVM cross-toolchain for IRIX 6.5
#
# Downloads upstream LLVM, applies IRIX patches, builds:
#   - clang (C/C++ cross-compiler targeting MIPS N32)
#   - LLD (linker with IRIX ELF support)
#   - libc++ (C++ standard library)
#   - libc++abi (C++ ABI support)
#
# No fork maintained. Patches are applied at build time from
# cross/build-toolchain/patches/*.patch
#
# Usage:
#   ./build-llvm-irix.sh [--version 22.1.2] [--install-dir /opt/cross]
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")"
PATCH_DIR="$SCRIPT_DIR/patches"

# Defaults
LLVM_VERSION="${LLVM_VERSION:-22.1.2}"
INSTALL_DIR="${INSTALL_DIR:-/opt/cross}"
BUILD_DIR="${BUILD_DIR:-$PROJECT_DIR/tmp/llvm-build}"
SRC_DIR="$BUILD_DIR/llvm-project-$LLVM_VERSION"
JOBS="${JOBS:-$(nproc)}"

# IRIX sysroot for cross-compilation headers
SYSROOT="${IRIX_SYSROOT:-/opt/irix-sysroot}"

log() { echo "=== $* ==="; }

# ─── Download ───
download_source() {
    if [ -d "$SRC_DIR/llvm" ]; then
        log "Source already exists at $SRC_DIR"
        return
    fi

    log "Downloading LLVM $LLVM_VERSION source"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    TARBALL="llvm-project-${LLVM_VERSION}.src.tar.xz"
    URL="https://github.com/llvm/llvm-project/releases/download/llvmorg-${LLVM_VERSION}/${TARBALL}"

    if [ ! -f "$TARBALL" ]; then
        curl -L -o "$TARBALL" "$URL"
    fi

    log "Extracting..."
    tar xf "$TARBALL"
    # Rename to consistent name
    mv "llvm-project-${LLVM_VERSION}.src" "$SRC_DIR" 2>/dev/null || true
}

# ─── Apply Patches ───
apply_patches() {
    log "Applying IRIX patches"
    cd "$SRC_DIR"

    # Check if already applied
    if [ -f ".irix_patches_applied" ]; then
        log "Patches already applied"
        return
    fi

    # Apply all .patch files from the patches directory
    if [ -d "$PATCH_DIR" ]; then
        for patch in "$PATCH_DIR"/*.patch; do
            [ -f "$patch" ] || continue
            echo "  Applying: $(basename "$patch")"
            patch -p1 < "$patch" || {
                echo "WARNING: Patch $(basename "$patch") failed — may need updating for LLVM $LLVM_VERSION"
            }
        done
    fi

    # Copy IRIX support files that are new additions (not patches to existing files)
    IRIX_FILES_DIR="$PATCH_DIR/irix-files"
    if [ -d "$IRIX_FILES_DIR" ]; then
        echo "  Copying IRIX support files..."
        cp -v "$IRIX_FILES_DIR"/libcxx-support-irix/* \
              "$SRC_DIR/libcxx/src/support/irix/" 2>/dev/null || true
        cp -v "$IRIX_FILES_DIR"/libcxx-include-support-irix/* \
              "$SRC_DIR/libcxx/include/__support/irix/" 2>/dev/null || true
    fi

    touch ".irix_patches_applied"
    log "Patches applied"
}

# ─── Build ───
build_toolchain() {
    log "Building LLVM toolchain (clang + LLD)"

    BUILD="$BUILD_DIR/build-toolchain"
    mkdir -p "$BUILD"
    cd "$BUILD"

    # Build just clang + LLD (host tools for cross-compilation)
    cmake -G Ninja "$SRC_DIR/llvm" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_DIR" \
        -DLLVM_ENABLE_PROJECTS="clang;lld" \
        -DLLVM_TARGETS_TO_BUILD="Mips;X86" \
        -DLLVM_DEFAULT_TARGET_TRIPLE="mips-sgi-irix6.5" \
        -DCLANG_DEFAULT_LINKER=lld \
        -DLLVM_INSTALL_UTILS=ON \
        -DLLVM_ENABLE_ASSERTIONS=OFF \
        -DLLVM_ENABLE_TERMINFO=OFF \
        -DLLVM_ENABLE_ZLIB=ON \
        -DLLVM_PARALLEL_LINK_JOBS=2

    log "Compiling (this will take a while)..."
    ninja -j"$JOBS" clang lld llvm-objdump llvm-readelf llvm-ar llvm-ranlib llvm-nm llvm-objcopy llvm-strip

    log "Installing..."
    ninja install-clang install-lld install-llvm-objdump install-llvm-readelf \
          install-llvm-ar install-llvm-ranlib install-llvm-nm install-llvm-objcopy install-llvm-strip

    # Create convenience symlinks
    cd "$INSTALL_DIR/bin"
    ln -sf clang clang-irix 2>/dev/null || true
    ln -sf lld ld.lld-irix 2>/dev/null || true
}

# ─── Build Runtime Libraries ───
# libc++ and libc++abi are cross-compiled using the existing build scripts
# in cross/build-runtime/ which use the installed clang.
build_runtime_note() {
    log "Toolchain built. Runtime libraries (libc++, libc++abi) are built separately:"
    echo "  cd $PROJECT_DIR/cross/build-runtime"
    echo "  bash build-libcxxabi.sh  # uses clang from $INSTALL_DIR"
    echo "  bash build-libcxx.sh"
    echo ""
    echo "The runtime build scripts read headers from $SRC_DIR/libcxx/include"
    echo "Set LLVM_PROJECT=$SRC_DIR for the irix-cxx-libcxx wrapper."
}

# ─── Main ───
main() {
    # Parse args
    while [ $# -gt 0 ]; do
        case "$1" in
            --version)  LLVM_VERSION="$2"; shift 2 ;;
            --install-dir) INSTALL_DIR="$2"; shift 2 ;;
            --build-dir) BUILD_DIR="$2"; shift 2 ;;
            --jobs|-j) JOBS="$2"; shift 2 ;;
            --help)
                echo "Usage: $0 [--version VER] [--install-dir DIR] [--jobs N]"
                exit 0 ;;
            *) echo "Unknown option: $1"; exit 1 ;;
        esac
    done

    echo "LLVM version:  $LLVM_VERSION"
    echo "Install dir:   $INSTALL_DIR"
    echo "Build dir:     $BUILD_DIR"
    echo "Patch dir:     $PATCH_DIR"
    echo ""

    download_source
    apply_patches
    build_toolchain
    build_runtime_note
}

main "$@"
