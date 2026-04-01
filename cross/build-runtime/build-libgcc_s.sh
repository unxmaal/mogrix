#!/bin/bash
# Build libgcc_s.so.1 from LLVM compiler-rt (builtins) + GCC 9 (unwind/EH)
#
# This replaces the SGUG-RSE libgcc_s.so.1 with our own build:
# - Builtins (soft-float, integer ops, etc.) from LLVM compiler-rt
# - DWARF2 unwind/EH from GCC 9.5.0 source (compiled with clang)
# - negtf2 (128-bit float negate, needed by libstdc++) - local implementation
#
# Usage: ./build-libgcc_s.sh [--clean]
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
MOGRIX_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Source locations
LLVM_PROJECT="${LLVM_PROJECT:-$HOME/projects/github/llvm-project}"
COMPILER_RT="$LLVM_PROJECT/compiler-rt/lib/builtins"
GCC_SRC="$SCRIPT_DIR/gcc-9.5.0"
GCC_CFG="$SCRIPT_DIR/gcc-config"

# Cross-compiler settings (match irix-cc)
CLANG="/opt/cross/bin/clang"
SYSROOT="${IRIX_SYSROOT:-/opt/irix-sysroot}"
STAGING="${MOGRIX_STAGING:-/opt/sgug-staging/usr/sgug}"
IRIX_LD="$STAGING/bin/irix-ld"

# Base flags matching irix-cc's target/ABI settings
BASE_CFLAGS="--target=mips-sgi-irix6.5 --sysroot=$SYSROOT -mabi=n32 -march=mips3 -mxgot"
BASE_CFLAGS="$BASE_CFLAGS -O2 -fPIC -fno-builtin -fvisibility=hidden"
# IRIX compatibility defines (from irix-cc)
BASE_CFLAGS="$BASE_CFLAGS -D_LONGLONG=1 -D_SGI_SOURCE -D_SGI_MP_SOURCE"
BASE_CFLAGS="$BASE_CFLAGS -D_SGI_REENTRANT_FUNCTIONS -Dsgi=1 -D__sgi=1"
BASE_CFLAGS="$BASE_CFLAGS -D_COMPILER_VERSION=730 -D_LANGUAGE_C=1"
# Force-include sgidefs.h for __int64_t, __uint32_t, etc.
BASE_CFLAGS="$BASE_CFLAGS -include $SYSROOT/usr/include/sgidefs.h"
# ELF target (for COMPILER_RT_ALIAS support) and C99 (for IRIX stdint.h)
BASE_CFLAGS="$BASE_CFLAGS -std=c99 -D__ELF__ -D__c99"

# compiler-rt builtins: include compiler-rt internal headers
# Drop -fvisibility=hidden for builtins — they need to be exported from libgcc_s
RT_CFLAGS="${BASE_CFLAGS/-fvisibility=hidden/} -I$COMPILER_RT -I$COMPILER_RT/../"

# GCC unwind: use gnu99, our config headers, suppress clang's unwind.h
UNWIND_CFLAGS="--target=mips-sgi-irix6.5 --sysroot=$SYSROOT -mabi=n32 -march=mips3 -mxgot"
UNWIND_CFLAGS="$UNWIND_CFLAGS -O2 -fPIC -fvisibility=hidden -std=gnu99 -D__ELF__ -D__c99"
UNWIND_CFLAGS="$UNWIND_CFLAGS -D_LONGLONG=1 -D_SGI_SOURCE -D_SGI_MP_SOURCE"
UNWIND_CFLAGS="$UNWIND_CFLAGS -D_SGI_REENTRANT_FUNCTIONS -Dsgi=1 -D__sgi=1"
UNWIND_CFLAGS="$UNWIND_CFLAGS -D_COMPILER_VERSION=730 -D_LANGUAGE_C=1"
UNWIND_CFLAGS="$UNWIND_CFLAGS -include $SYSROOT/usr/include/sgidefs.h"
UNWIND_CFLAGS="$UNWIND_CFLAGS -include $GCC_CFG/stdarg_shim.h"
UNWIND_CFLAGS="$UNWIND_CFLAGS -I$GCC_CFG -I$GCC_SRC/libgcc -I$GCC_SRC/gcc -I$GCC_SRC/include -I$GCC_SRC/libgcc/config"
UNWIND_CFLAGS="$UNWIND_CFLAGS -Wno-attributes -Wno-builtin-macro-redefined"
# Prevent clang's unwind.h from conflicting with GCC's unwind types
UNWIND_CFLAGS="$UNWIND_CFLAGS -D__CLANG_UNWIND_H"

# Output
BUILD_DIR="$SCRIPT_DIR/build"
OUTPUT="$SCRIPT_DIR/libgcc_s.so.1"

if [ "${1:-}" = "--clean" ]; then
    rm -rf "$BUILD_DIR" "$OUTPUT"
    echo "Cleaned."
    exit 0
fi

mkdir -p "$BUILD_DIR/builtins" "$BUILD_DIR/unwind"

echo "=== Phase 1: Compile compiler-rt builtins ==="

# All builtin .c files we need (mapped from SGUG-RSE libgcc_s symbol table)
BUILTIN_SOURCES=(
    # Integer arithmetic
    absvdi2.c absvsi2.c absvti2.c
    addvdi3.c addvsi3.c addvti3.c
    ashlti3.c ashrti3.c
    clzsi2.c clzdi2.c clzti2.c cmpti2.c ctzsi2.c ctzdi2.c ctzti2.c
    divmodti4.c divti3.c
    ffsdi2.c ffsti2.c
    lshrti3.c
    modti3.c multi3.c
    mulvdi3.c mulvsi3.c mulvti3.c
    negdi2.c negti2.c negvdi2.c negvsi2.c negvti2.c
    paritydi2.c parityti2.c
    popcountdi2.c popcountti2.c
    subvdi3.c subvsi3.c subvti3.c
    ucmpti2.c udivmodti4.c udivti3.c umodti3.c

    # Byte swap
    bswapdi2.c bswapsi2.c

    # Soft-float: double precision
    adddf3.c subdf3.c muldf3.c divdf3.c negdf2.c
    comparedf2.c  # exports __cmpdf2, __eqdf2, __ledf2, __ltdf2, __nedf2, __gedf2, __gtdf2, __unorddf2
    extendsfdf2.c truncdfsf2.c
    fixdfdi.c fixdfsi.c fixdfti.c
    fixunsdfdi.c fixunsdfsi.c fixunsdfti.c
    floatdidf.c floatsidf.c floattidf.c
    floatundidf.c floatunsidf.c floatuntidf.c
    powidf2.c divdc3.c muldc3.c

    # Soft-float: single precision
    addsf3.c subsf3.c mulsf3.c divsf3.c negsf2.c
    comparesf2.c  # exports __cmpsf2, __eqsf2, etc.
    fixsfdi.c fixsfsi.c fixsfti.c
    fixunssfdi.c fixunssfsi.c fixunssfti.c
    floatdisf.c floatsisf.c floattisf.c
    floatundisf.c floatunsisf.c floatuntisf.c
    powisf2.c divsc3.c mulsc3.c

    # Soft-float: quad precision (128-bit / long double on MIPS)
    addtf3.c subtf3.c multf3.c divtf3.c
    comparetf2.c  # exports __cmptf2, __eqtf2, etc.
    extenddftf2.c extendsftf2.c trunctfdf2.c trunctfsf2.c
    fixtfdi.c fixtfsi.c fixtfti.c
    fixunstfdi.c fixunstfsi.c fixunstfti.c
    floatditf.c floatsitf.c floattitf.c
    floatunditf.c floatunsitf.c floatuntitf.c
    powitf2.c divtc3.c multc3.c

    # Misc
    clear_cache.c
    enable_execute_stack.c
    emutls.c
    # gcc_personality_v0.c — provided by GCC's unwind-c.c instead

    # Internal helpers (referenced by builtins at runtime)
    int_util.c          # __compilerrt_abort_impl
    fp_mode.c           # __fe_getround, __fe_raise_inexact
)

compiled=0
failed=0
for src in "${BUILTIN_SOURCES[@]}"; do
    obj="$BUILD_DIR/builtins/${src%.c}.o"
    srcpath="$COMPILER_RT/$src"
    if [ ! -f "$srcpath" ]; then
        echo "MISSING: $srcpath"
        failed=$((failed + 1))
        continue
    fi
    if $CLANG $RT_CFLAGS -c "$srcpath" -o "$obj" 2>"$BUILD_DIR/builtins/${src%.c}.err"; then
        compiled=$((compiled + 1))
    else
        echo "FAILED: $src"
        cat "$BUILD_DIR/builtins/${src%.c}.err"
        failed=$((failed + 1))
    fi
done

# Build our negtf2 (not in compiler-rt)
echo "Building negtf2.c (local)..."
if $CLANG $RT_CFLAGS -c "$SCRIPT_DIR/negtf2.c" -o "$BUILD_DIR/builtins/negtf2.o" 2>"$BUILD_DIR/builtins/negtf2.err"; then
    compiled=$((compiled + 1))
else
    echo "FAILED: negtf2.c"
    cat "$BUILD_DIR/builtins/negtf2.err"
    failed=$((failed + 1))
fi

echo "Builtins: $compiled compiled, $failed failed"
if [ "$failed" -gt 0 ]; then
    echo "ERROR: Some builtins failed to compile. Check errors above."
    exit 1
fi

echo ""
echo "=== Phase 2: Compile GCC 9 unwind/EH ==="

# Check GCC source is available
if [ ! -d "$GCC_SRC/libgcc" ]; then
    echo "ERROR: GCC 9.5.0 source not found at $GCC_SRC"
    echo "Run: cd $SCRIPT_DIR && curl -sL https://ftp.gnu.org/gnu/gcc/gcc-9.5.0/gcc-9.5.0.tar.xz | tar xJ --wildcards 'gcc-9.5.0/libgcc/*' 'gcc-9.5.0/gcc/tsystem.h' 'gcc-9.5.0/gcc/coretypes.h' 'gcc-9.5.0/gcc/defaults.h' 'gcc-9.5.0/include/*'"
    exit 1
fi

# Ensure unwind.h symlink exists (GCC's unwind-generic.h)
[ ! -e "$GCC_SRC/libgcc/unwind.h" ] && ln -sf unwind-generic.h "$GCC_SRC/libgcc/unwind.h"
# Ensure gthr-default.h exists (copy of gthr-posix.h)
[ ! -e "$GCC_SRC/libgcc/gthr-default.h" ] && cp "$GCC_SRC/libgcc/gthr-posix.h" "$GCC_SRC/libgcc/gthr-default.h"

# GCC unwind C sources
UNWIND_SOURCES=(
    unwind-dw2.c        # Core DWARF2 unwinder (_Unwind_RaiseException, etc.)
    unwind-dw2-fde.c    # Frame Description Entry registration (__register_frame, etc.)
)

# unwind-c.c needs VISIBLE __gcc_personality_v0, so compile WITHOUT -fvisibility=hidden
UNWIND_VISIBLE_SOURCES=(
    unwind-c.c          # C language personality routine (__gcc_personality_v0)
)

unwind_compiled=0
unwind_failed=0
for src in "${UNWIND_SOURCES[@]}"; do
    obj="$BUILD_DIR/unwind/${src%.c}.o"
    if $CLANG $UNWIND_CFLAGS -c "$GCC_SRC/libgcc/$src" -o "$obj" 2>"$BUILD_DIR/unwind/${src%.c}.err"; then
        echo "  OK: $src"
        unwind_compiled=$((unwind_compiled + 1))
    else
        echo "  FAILED: $src"
        cat "$BUILD_DIR/unwind/${src%.c}.err"
        unwind_failed=$((unwind_failed + 1))
    fi
done

# Compile sources that need default visibility (exported symbols)
UNWIND_VISIBLE_FLAGS="${UNWIND_CFLAGS/-fvisibility=hidden/}"
for src in "${UNWIND_VISIBLE_SOURCES[@]}"; do
    obj="$BUILD_DIR/unwind/${src%.c}.o"
    if $CLANG $UNWIND_VISIBLE_FLAGS -c "$GCC_SRC/libgcc/$src" -o "$obj" 2>"$BUILD_DIR/unwind/${src%.c}.err"; then
        echo "  OK: $src (visible)"
        unwind_compiled=$((unwind_compiled + 1))
    else
        echo "  FAILED: $src (visible)"
        cat "$BUILD_DIR/unwind/${src%.c}.err"
        unwind_failed=$((unwind_failed + 1))
    fi
done

echo "Unwind: $unwind_compiled compiled, $unwind_failed failed"
if [ "$unwind_failed" -gt 0 ]; then
    echo "ERROR: Some unwind sources failed to compile."
    exit 1
fi

echo ""
echo "=== Phase 2b: Compile standalone builtins ==="

# These are standalone implementations of functions that normally come from
# libgcc2.c, which can't be compiled with clang (uses GCC-specific machine modes).
STANDALONE_BUILTINS=(
    clrsbdi2.c    # __clrsbdi2 — count leading redundant sign bits (64-bit)
    clrsbti2.c    # __clrsbti2 — count leading redundant sign bits (128-bit)
)

standalone_compiled=0
standalone_failed=0
for src in "${STANDALONE_BUILTINS[@]}"; do
    obj="$BUILD_DIR/unwind/${src%.c}.o"
    if $CLANG $RT_CFLAGS -c "$SCRIPT_DIR/$src" -o "$obj" 2>"$BUILD_DIR/unwind/${src%.c}.err"; then
        echo "  OK: $src"
        standalone_compiled=$((standalone_compiled + 1))
    else
        echo "  FAILED: $src"
        cat "$BUILD_DIR/unwind/${src%.c}.err"
        standalone_failed=$((standalone_failed + 1))
    fi
done
echo "Standalone builtins: $standalone_compiled compiled, $standalone_failed failed"

echo ""
echo "=== Phase 3: Link libgcc_s.so.1 ==="

# Collect all .o files
ALL_OBJECTS=()
for obj in "$BUILD_DIR"/builtins/*.o "$BUILD_DIR"/unwind/*.o; do
    [ -f "$obj" ] && ALL_OBJECTS+=("$obj")
done

echo "Linking ${#ALL_OBJECTS[@]} object files..."

# Link using irix-ld to produce a shared library
# Use -soname libgcc_s.so.1 for compatibility
$IRIX_LD \
    -shared \
    -soname libgcc_s.so.1 \
    -o "$OUTPUT" \
    "${ALL_OBJECTS[@]}" \
    -L"$SYSROOT/usr/lib32" \
    -lc \
    2>"$BUILD_DIR/link.err"

if [ $? -eq 0 ] && [ -f "$OUTPUT" ]; then
    echo ""
    echo "=== SUCCESS ==="
    echo "Output: $OUTPUT"
    echo "Size: $(ls -lh "$OUTPUT" | awk '{print $5}')"
    echo ""
    echo "=== Symbol check ==="
    exported=$(/opt/cross/bin/mips-sgi-irix6.5-nm -D "$OUTPUT" 2>/dev/null | grep -c " T " || true)
    echo "Exported symbols: $exported"
    echo ""
    echo "Next steps:"
    echo "  1. Compare symbols: nm -D $OUTPUT vs nm -D $STAGING/lib32/libgcc_s.so.1"
    echo "  2. Test with overlayfs (Phase 0b)"
else
    echo "LINK FAILED"
    cat "$BUILD_DIR/link.err"
    exit 1
fi
