#!/bin/bash
#
# build-runtime-objects.sh — Build ALL runtime objects needed in staging
#
# This script builds every object file and library that must exist in
# $MOGRIX_STAGING/lib32/ before any package can be cross-compiled.
#
# Prerequisites:
#   - irix-cc deployed to staging (run: uv run mogrix setup-cross)
#   - llvm-ar available in /opt/cross/bin/
#   - Source files present in repo (cross/crt/, cross/lib/, compat/)
#
# Usage:
#   ./scripts/build-runtime-objects.sh
#
# What it builds:
#
#   FROM cross/crt/:
#     crtbeginS.o   — PIC constructor support (shared libs)
#     crtendS.o     — PIC destructor support (shared libs)
#     crtbeginT.o   — Non-PIC constructor support (executables)
#     crtendT.o     — Non-PIC destructor support (executables)
#
#   FROM cross/lib/:
#     dso_handle.o  — __dso_handle for DSO-local operations
#     safe_mem.o    — Byte-safe mem/string functions (prevents overread SIGSEGV)
#
#   FROM compat/malloc/:
#     dlmalloc.o    — mmap-based allocator (executables only, never in .so)
#
#   FROM compat/runtime/:
#     libsoft_float_stubs.a  — 128-bit soft float stubs
#     libatomic.a            — Atomic operation stubs
#     libcompat.a            — Compat functions (stpcpy, posix_spawn, etc.)
#
#   FROM compat/ (multiple dirs):
#     libmogrix_compat.so   — Preloaded override for buggy libc functions
#
#   FROM cross/lib/:
#     irix-shared.lds       — BFD ld linker script (copied, not built)
#

set -euo pipefail

# --- Configuration ---

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
MOGRIX_DIR="$(dirname "$SCRIPT_DIR")"
STAGING="$MOGRIX_STAGING"
CROSS="/opt/cross/bin"
CC="${STAGING}/bin/irix-cc"
AR="${CROSS}/llvm-ar"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

PASS=0
FAIL=0
TOTAL=0

log_ok()   { echo -e "  ${GREEN}[OK]${NC} $*"; PASS=$((PASS + 1)); TOTAL=$((TOTAL + 1)); }
log_fail() { echo -e "  ${RED}[FAIL]${NC} $*"; FAIL=$((FAIL + 1)); TOTAL=$((TOTAL + 1)); }
log_info() { echo -e "  ${YELLOW}[..]${NC} $*"; }

# --- Prerequisites ---

echo "=== Mogrix Runtime Objects Builder ==="
echo ""

errors=0
if [[ ! -x "$CC" ]]; then
    echo -e "${RED}ERROR:${NC} irix-cc not found at $CC"
    echo "       Run 'uv run mogrix setup-cross' first."
    errors=1
fi
if [[ ! -x "$AR" ]]; then
    echo -e "${RED}ERROR:${NC} llvm-ar not found at $AR"
    errors=1
fi
if [[ ! -d "$STAGING/lib32" ]]; then
    echo -e "${RED}ERROR:${NC} Staging lib32 directory missing: $STAGING/lib32"
    echo "       Run: sudo mkdir -p $STAGING/lib32 && sudo chown \$USER $STAGING/lib32"
    errors=1
fi
if [[ $errors -ne 0 ]]; then
    exit 1
fi

TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

echo "Sources:  $MOGRIX_DIR"
echo "Staging:  $STAGING/lib32/"
echo "Compiler: $CC"
echo "Archiver: $AR"
echo ""

# --- CRT Objects (from assembly) ---

echo "[1/7] CRT objects..."

# crtbeginS.o — PIC, for shared libraries
# Uses version script to hide CRT symbols (prevents cross-library interposition)
if "$CC" -c -fPIC \
    "${MOGRIX_DIR}/cross/crt/crtbeginS.S" \
    -o "${TMPDIR}/crtbeginS.o" 2>/dev/null; then
    cp "${TMPDIR}/crtbeginS.o" "${STAGING}/lib32/crtbeginS.o"
    log_ok "crtbeginS.o"
else
    log_fail "crtbeginS.o"
fi

# crtendS.o — PIC, for shared libraries
if "$CC" -c -fPIC \
    "${MOGRIX_DIR}/cross/crt/crtendS.S" \
    -o "${TMPDIR}/crtendS.o" 2>/dev/null; then
    cp "${TMPDIR}/crtendS.o" "${STAGING}/lib32/crtendS.o"
    log_ok "crtendS.o"
else
    log_fail "crtendS.o"
fi

# crtbeginT.o — Non-PIC, for executables
if "$CC" -c \
    "${MOGRIX_DIR}/cross/crt/crtbeginT.S" \
    -o "${TMPDIR}/crtbeginT.o" 2>/dev/null; then
    cp "${TMPDIR}/crtbeginT.o" "${STAGING}/lib32/crtbeginT.o"
    log_ok "crtbeginT.o"
else
    log_fail "crtbeginT.o"
fi

# crtendT.o — Non-PIC, for executables
if "$CC" -c \
    "${MOGRIX_DIR}/cross/crt/crtendT.S" \
    -o "${TMPDIR}/crtendT.o" 2>/dev/null; then
    cp "${TMPDIR}/crtendT.o" "${STAGING}/lib32/crtendT.o"
    log_ok "crtendT.o"
else
    log_fail "crtendT.o"
fi

# --- Linker support objects ---

echo ""
echo "[2/7] Linker support objects..."

# dso_handle.o — __dso_handle symbol for __cxa_atexit
if "$CC" -c \
    "${MOGRIX_DIR}/cross/lib/dso_handle.c" \
    -o "${TMPDIR}/dso_handle.o" 2>/dev/null; then
    cp "${TMPDIR}/dso_handle.o" "${STAGING}/lib32/dso_handle.o"
    log_ok "dso_handle.o"
else
    log_fail "dso_handle.o"
fi

# safe_mem.o — Byte-safe memcmp/strcmp/strncmp/_Hash_bytes (prevents overread SIGSEGV)
# Uses raw clang (not irix-cc) because safe_mem.c is freestanding — it has its own
# size_t typedef that conflicts with the sysroot headers irix-cc force-includes.
RAW_CLANG="${CROSS}/clang"
if "$RAW_CLANG" --target=mips-sgi-irix6.5 -mabi=n32 -march=mips3 -w -c \
    "${MOGRIX_DIR}/cross/lib/safe_mem.c" \
    -o "${TMPDIR}/safe_mem.o" 2>/dev/null; then
    cp "${TMPDIR}/safe_mem.o" "${STAGING}/lib32/safe_mem.o"
    log_ok "safe_mem.o"
else
    log_fail "safe_mem.o"
fi

# --- dlmalloc ---

echo ""
echo "[3/7] dlmalloc (mmap-based allocator)..."

# dlmalloc.o — NEVER link into shared libraries, executables only
# Uses spin locks (MIPS ll/sc atomics) for thread safety
if "$CC" -c -O2 \
    -DHAVE_MORECORE=0 -DHAVE_MMAP=1 \
    -DUSE_LOCKS=1 -DUSE_SPIN_LOCKS=1 \
    -DMMAP_CLEARS=1 -Dmalloc_getpagesize=16384 \
    "${MOGRIX_DIR}/compat/malloc/dlmalloc.c" \
    -o "${TMPDIR}/dlmalloc.o" 2>/dev/null; then
    cp "${TMPDIR}/dlmalloc.o" "${STAGING}/lib32/dlmalloc.o"
    log_ok "dlmalloc.o"
else
    log_fail "dlmalloc.o"
fi

# --- Static archives ---

echo ""
echo "[4/7] Static archives..."

# libsoft_float_stubs.a — 128-bit soft float stubs
if "$CC" -c \
    "${MOGRIX_DIR}/compat/runtime/soft_float_stubs.c" \
    -o "${TMPDIR}/soft_float_stubs.o" 2>/dev/null; then
    "$AR" rcs "${STAGING}/lib32/libsoft_float_stubs.a" "${TMPDIR}/soft_float_stubs.o"
    log_ok "libsoft_float_stubs.a"
else
    log_fail "libsoft_float_stubs.a"
fi

# libatomic.a — Atomic operation stubs
if [[ -f "${MOGRIX_DIR}/compat/runtime/libatomic_stub.c" ]]; then
    if "$CC" -c \
        "${MOGRIX_DIR}/compat/runtime/libatomic_stub.c" \
        -o "${TMPDIR}/libatomic_stub.o" 2>/dev/null; then
        "$AR" rcs "${STAGING}/lib32/libatomic.a" "${TMPDIR}/libatomic_stub.o"
        log_ok "libatomic.a"
    else
        log_fail "libatomic.a"
    fi
else
    log_info "libatomic_stub.c not found, skipping libatomic.a"
fi

# libcompat.a — All remaining compat/runtime/*.c files
COMPAT_OBJS=""
compat_ok=true
for src in "${MOGRIX_DIR}/compat/runtime/"*.c; do
    base=$(basename "$src" .c)
    # Skip files that go into their own archives
    if [[ "$base" == "soft_float_stubs" || "$base" == "libatomic_stub" ]]; then
        continue
    fi
    if "$CC" -c "$src" -o "${TMPDIR}/${base}.o" 2>/dev/null; then
        COMPAT_OBJS="${COMPAT_OBJS} ${TMPDIR}/${base}.o"
    else
        log_fail "libcompat.a (failed to compile ${base}.c)"
        compat_ok=false
    fi
done
if [[ -n "$COMPAT_OBJS" ]] && $compat_ok; then
    "$AR" rcs "${STAGING}/lib32/libcompat.a" $COMPAT_OBJS
    log_ok "libcompat.a"
elif [[ -n "$COMPAT_OBJS" ]]; then
    "$AR" rcs "${STAGING}/lib32/libcompat.a" $COMPAT_OBJS
    log_ok "libcompat.a (partial — some sources failed)"
fi

# --- libmogrix_compat.so ---

echo ""
echo "[5/7] libmogrix_compat.so (preloaded libc overrides)..."

# Sources for the shared library — these override buggy IRIX libc functions
# that are called from shared libraries (not just executables).
# CRITICAL: No GLib/GTK deps allowed — this is preloaded into ALL binaries.
# CRITICAL: Do NOT include setenv.c or unsetenv — IRIX rld doesn't set up
# $t9/GP for interposed libc functions called internally, causing SIGSEGV.
# Packages needing setenv must use inject_compat_functions (static link) instead.
COMPAT_SO_SRCS=""
for src in \
    compat/stdlib/bsearch.c \
    compat/stdlib/posix_memalign.c \
    compat/sys/socketpair.c \
    compat/sys/shm_open.c \
    compat/sys/mincore.c \
    compat/runtime/muloti4.c \
    compat/runtime/divti3.c \
    compat/stdlib/mkdtemp.c \
    compat/error/strerror_r.c \
    compat/string/memmem.c \
    patches/shared/mogrix_crash_handler.c
do
    full="${MOGRIX_DIR}/${src}"
    if [[ -f "$full" ]]; then
        COMPAT_SO_SRCS="${COMPAT_SO_SRCS} ${full}"
    else
        log_fail "libmogrix_compat.so (missing source: $src)"
    fi
done

if [[ -n "$COMPAT_SO_SRCS" ]]; then
    # --as-needed: drop gratuitous libpthread.so NEEDED from irix-ld's default
    # -lgcc_s -lpthread. libmogrix_compat.so uses no pthread symbols. Having
    # libpthread as NEEDED causes "_RLD_PTHREADS_START invoked twice" when
    # preloaded via _RLDN32_LIST in bundles that also link libpthread (SDL2 apps).
    if "$CC" -shared -fPIC -Wl,--as-needed \
        -I "${MOGRIX_DIR}/compat/include" \
        -I "${MOGRIX_DIR}/patches/shared" \
        $COMPAT_SO_SRCS \
        -o "${TMPDIR}/libmogrix_compat.so" 2>/dev/null; then
        cp "${TMPDIR}/libmogrix_compat.so" "${STAGING}/lib32/libmogrix_compat.so"
        log_ok "libmogrix_compat.so"
    else
        log_fail "libmogrix_compat.so (compilation failed)"
    fi
fi

# --- Linker script ---

echo ""
echo "[6/7] Linker script..."

# irix-shared.lds — BFD ld linker script for standard 2-segment layout
# (Needed when BFD ld is used as fallback; -z separate-code crashes rld)
if [[ -f "${MOGRIX_DIR}/cross/lib/irix-shared.lds" ]]; then
    cp "${MOGRIX_DIR}/cross/lib/irix-shared.lds" "${STAGING}/lib32/irix-shared.lds"
    log_ok "irix-shared.lds"
else
    log_fail "irix-shared.lds (source not found)"
fi

# --- CRT version script ---

echo ""
echo "[7/7] CRT version script..."

# crt-hide.ver — Forces CRT symbols local to prevent cross-library interposition
if [[ -f "${MOGRIX_DIR}/cross/crt/crt-hide.ver" ]]; then
    cp "${MOGRIX_DIR}/cross/crt/crt-hide.ver" "${STAGING}/lib32/crt-hide.ver"
    log_ok "crt-hide.ver"
else
    log_fail "crt-hide.ver (source not found)"
fi

# --- Summary ---

echo ""
echo "=== Results ==="
echo ""
echo "  Total: $TOTAL  Passed: $PASS  Failed: $FAIL"
echo ""

# Verify all expected files exist
echo "Staging lib32 runtime objects:"
EXPECTED=(
    crtbeginS.o crtendS.o crtbeginT.o crtendT.o
    dso_handle.o safe_mem.o dlmalloc.o
    libsoft_float_stubs.a libatomic.a libcompat.a
    libmogrix_compat.so irix-shared.lds crt-hide.ver
)
missing=0
for f in "${EXPECTED[@]}"; do
    if [[ -f "${STAGING}/lib32/${f}" ]]; then
        size=$(stat -c%s "${STAGING}/lib32/${f}" 2>/dev/null || echo "?")
        echo -e "  ${GREEN}OK${NC}  ${f}  (${size} bytes)"
    else
        echo -e "  ${RED}MISSING${NC}  ${f}"
        missing=$((missing + 1))
    fi
done

echo ""
if [[ $FAIL -eq 0 && $missing -eq 0 ]]; then
    echo -e "${GREEN}All runtime objects built and deployed successfully.${NC}"
else
    echo -e "${YELLOW}Some objects failed or are missing. Check output above.${NC}"
    exit 1
fi
