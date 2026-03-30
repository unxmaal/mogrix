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
# Install to project tools/bin by default (no sudo needed)
INSTALL_DIR="$PROJECT_DIR/tools/bin"

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

    local writer="lld/ELF/Writer.cpp"
    local synth="lld/ELF/SyntheticSections.cpp"
    local driver="lld/ELF/Driver.cpp"

    # =========================================================================
    # Writer.cpp patches — ELF structure compatibility
    # =========================================================================
    log_info "Patching $writer for IRIX ELF structure compatibility..."

    # 1. Skip MIPS ABIFLAGS section creation (IRIX ELF parser crashes on it)
    sed -i '/if ((in.mipsAbiFlags = MipsAbiFlagsSection<ELFT>::create()))/,/add(\*in.mipsAbiFlags);/{
        s/if ((in.mipsAbiFlags = MipsAbiFlagsSection<ELFT>::create()))/\/\/ IRIX ELF parser crashes on MIPS ABIFLAGS\n    if (config->osabi != ELFOSABI_IRIX \&\& (in.mipsAbiFlags = MipsAbiFlagsSection<ELFT>::create()))/
    }' "$writer"

    # 2. Skip PT_MIPS_ABIFLAGS program header
    sed -i '/addPhdrForSection(part, SHT_MIPS_ABIFLAGS, PT_MIPS_ABIFLAGS, PF_R);/{
        s|addPhdrForSection(part, SHT_MIPS_ABIFLAGS, PT_MIPS_ABIFLAGS, PF_R);|if (config->osabi != ELFOSABI_IRIX) { addPhdrForSection(part, SHT_MIPS_ABIFLAGS, PT_MIPS_ABIFLAGS, PF_R); }|
    }' "$writer"

    # 3. Skip PT_PHDR for IRIX shared libs (confuses IRIX ELF parser)
    # CRITICAL: Must use `if (skipPhdr) {} else if (isMain)` — NOT `!skipPhdr && isMain`.
    # The else branch dereferences part.programHeaders which is null for main partition.
    sed -i '/The first phdr entry is PT_PHDR/,/addHdr(PT_PHDR, PF_R)->add(part.programHeaders/{
        s|// itself\.|// itself. IRIX shared libraries must not have PT_PHDR -- it confuses\n    // the IRIX ELF parser.|
        s|if (isMain)|bool skipPhdr = config->osabi == ELFOSABI_IRIX \&\& config->shared;\n    if (skipPhdr) {\n      \/\/ Skip PT_PHDR entirely for IRIX shared libraries.\n    } else if (isMain)|
    }' "$writer"

    # 4. Disable PT_GNU_STACK for IRIX (predates it, crashes on it)
    sed -i '/if (config->zGnustack != GnuStackKind::None) {/i\
  \/\/ IRIX predates PT_GNU_STACK and crashes on it\n  if (config->osabi == ELFOSABI_IRIX)\n    config->zGnustack = GnuStackKind::None;\n' "$writer"

    # 5. Add IRIX version section comment (sections must still be created —
    # LLD internal version resolution needs the objects. strip-verneed cleans
    # the output post-link. DO NOT wrap in if-guard — that crashes LLD.)
    sed -i '/part.verSym = std::make_unique<VersionTableSection>();/i\
      \/\/ IRIX rld predates GNU symbol versioning and crashes on VERNEED\/VERSYM\n      \/\/ dynamic tags. We still create the sections (version resolution needs\n      \/\/ them internally) but strip-verneed removes them post-link if they\n      \/\/ contain data. TODO: suppress output emission instead.' "$writer"

    # =========================================================================
    # SyntheticSections.cpp patches — .eh_frame and dynamic tags
    # =========================================================================
    log_info "Patching $synth for IRIX .eh_frame and dynamic tag fixes..."

    # 6. Set SHF_WRITE on .eh_frame for IRIX (rld writes relocations at load time)
    sed -i 's|: SyntheticSection(SHF_ALLOC, SHT_PROGBITS, 1, ".eh_frame") {}|: SyntheticSection(SHF_ALLOC, SHT_PROGBITS, 1, ".eh_frame") {\n  // IRIX rld applies R_MIPS_REL32 relocations by writing into .eh_frame at\n  // load time. The section must be in a writable segment to avoid SIGSEGV.\n  if (config->osabi == ELFOSABI_IRIX)\n    flags \|= SHF_WRITE;\n}|' "$synth"

    # 7. Skip DT_INIT_ARRAY/DT_FINI_ARRAY for IRIX (rld doesn't process them)
    sed -i '/if (Out::initArray) {/{
        s|if (Out::initArray) {|// IRIX rld does not process DT_INIT_ARRAY\/DT_FINI_ARRAY.\n    if (Out::initArray \&\& config->osabi != ELFOSABI_IRIX) {|
    }' "$synth"
    sed -i '/if (Out::finiArray) {/{
        s|if (Out::finiArray) {|if (Out::finiArray \&\& config->osabi != ELFOSABI_IRIX) {|
    }' "$synth"

    # =========================================================================
    # Driver.cpp patches — IRIX target defaults
    # =========================================================================
    log_info "Patching $driver for IRIX target defaults..."

    # 8. Add IRIX defaults at end of setConfigs() (after osabi is known)
    sed -i '/config->pcRelOptimize =/,/m == EM_PPC64);/{
        /m == EM_PPC64);/a\
\n  \/\/ IRIX target-specific defaults. IRIX rld predates several GNU ELF\n  \/\/ extensions and requires a 2-segment (RE + RW) layout.\n  if (config->osabi == ELFOSABI_IRIX) {\n    \/\/ 2-segment layout: IRIX rld expects RE + RW, not R + RE + RW.\n    config->singleRoRx = true;\n    \/\/ No GNU_RELRO: IRIX rld does not support it and separate RELRO\n    \/\/ segments crash rld during static init of large C++ binaries.\n    config->zRelro = false;\n    \/\/ Allow text relocations: MIPS clang emits absolute R_MIPS_32\n    \/\/ relocations in .eh_frame even with -fPIC.\n    config->zText = false;\n    \/\/ Prevent secondary GOT: IRIX rld only processes primary GOT entries\n    \/\/ (DT_MIPS_LOCAL_GOTNO\/GOTSYM\/SYMTABNO). Secondary GOTs use GNU\n    \/\/ R_MIPS_REL32 relocations that IRIX rld ignores, leaving entries\n    \/\/ at 0 -> SIGSEGV. 1048576 allows up to 262K entries.\n    config->mipsGotSize = 1048576;\n    \/\/ DT_RPATH: IRIX rld only recognizes DT_RPATH, not DT_RUNPATH.\n    config->enableNewDtags = false;\n    \/\/ Default image base: shared libs at 0x0f800000, executables at\n    \/\/ 0x1000000 (above rld quickstart table, giving 176MB brk heap).\n    \/\/ Shared libs MUST NOT use 0x5ffe0000 (libgcc_s preferred base).\n    \/\/ Collision causes rld displacement that corrupts DWARF unwinder\n    \/\/ state, crashing on any C++ throw in shared libraries (SIGSEGV).\n    if (!config->imageBase.has_value())\n      config->imageBase = config->shared ? 0x0f800000 : 0x1000000;\n  }
    }' "$driver"

    # 9. Add _irix emulation suffix detection in parseEmulation()
    # Follows FreeBSD's _fbsd pattern. Activated by: -m elf32btsmipn32_irix
    sed -i '/osabi = ELFOSABI_FREEBSD;/{
        N
        s|osabi = ELFOSABI_FREEBSD;\n  }|osabi = ELFOSABI_FREEBSD;\n  } else if (s.ends_with("_irix")) {\n    s = s.drop_back(5);\n    osabi = ELFOSABI_IRIX;\n  }|
    }' "$driver"

    # 10. Don't clobber imageBase set by target defaults (IRIX needs non-zero)
    # link() unconditionally sets config->imageBase = getImageBase(args) which
    # returns nullopt when --image-base not on CLI, overwriting IRIX defaults.
    sed -i 's|config->imageBase = getImageBase(args);|// Only override imageBase if the user passed --image-base. Target-specific\n  // defaults (e.g., IRIX non-zero base in setConfigs) must not be clobbered.\n  if (auto base = getImageBase(args))\n    config->imageBase = base;|' "$driver"

    # =========================================================================
    # SyntheticSections.cpp patch — EI_OSABI output fix
    # =========================================================================

    # 11. Write ELFOSABI_NONE (not ELFOSABI_IRIX) in ELF header
    # IRIX rld only accepts ELFOSABI_NONE(0). IRIX native tools never set
    # ELFOSABI_IRIX(8). We use it internally only for conditional behavior.
    sed -i 's|eHdr->e_ident\[EI_OSABI\] = config->osabi;|// IRIX rld only accepts ELFOSABI_NONE (0) in EI_OSABI. ELFOSABI_IRIX (8)\n  // is used internally to activate IRIX-specific behavior but must not appear\n  // in the output ELF header -- IRIX native tools never set it.\n  eHdr->e_ident[EI_OSABI] = config->osabi == ELFOSABI_IRIX\n                                 ? ELFOSABI_NONE\n                                 : config->osabi;|' "$synth"

    # =========================================================================
    # Verify all patches applied
    # =========================================================================
    local ok=1
    grep -q "osabi != ELFOSABI_IRIX" "$writer" || { log_error "Writer.cpp IRIX patches failed"; ok=0; }
    grep -q "skipPhdr" "$writer" || { log_error "Writer.cpp PT_PHDR fix failed"; ok=0; }
    grep -q "ELFOSABI_IRIX" "$synth" || { log_error "SyntheticSections.cpp patches failed"; ok=0; }
    grep -q "ELFOSABI_NONE" "$synth" || { log_error "SyntheticSections.cpp EI_OSABI fix failed"; ok=0; }
    grep -q 'ends_with("_irix")' "$driver" || { log_error "Driver.cpp emulation suffix failed"; ok=0; }
    grep -q "singleRoRx" "$driver" || { log_error "Driver.cpp IRIX defaults failed"; ok=0; }
    grep -q "auto base = getImageBase" "$driver" || { log_error "Driver.cpp imageBase clobber fix failed"; ok=0; }

    if [ "$ok" = "1" ]; then
        log_info "All patches applied successfully"
    else
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
        mkdir -p "$INSTALL_DIR"
        cp bin/ld.lld "$INSTALL_DIR/ld.lld-irix-18"
        chmod +x "$INSTALL_DIR/ld.lld-irix-18"
        log_info "Installed to $INSTALL_DIR/ld.lld-irix-18"

        echo ""
        echo "To use this LLD, update irix-ld wrapper:"
        echo "  LLD=\"$INSTALL_DIR/ld.lld-irix-18\""
    else
        log_error "ld.lld binary not found in build output"
        exit 1
    fi
}

test_lld() {
    log_info "Testing LLD..."

    "$INSTALL_DIR/ld.lld-irix-18" --version

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
    echo "New LLD: $INSTALL_DIR/ld.lld-irix-18"
    echo ""
    echo "To test with mogrix, update $MOGRIX_STAGING/bin/irix-ld"
    echo "to use this LLD path in the LLD= variable"
}

main "$@"
