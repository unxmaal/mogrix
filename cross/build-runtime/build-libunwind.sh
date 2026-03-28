#!/bin/bash
# Build LLVM libunwind for IRIX 6.5 MIPS N32
#
# Part of the libc++ migration: libunwind → libc++abi → libc++
# LLVM source at $LLVM_PROJECT already has IRIX patches (_LIBUNWIND_USE_IRIX_RLD).
#
# Usage: ./build-libunwind.sh [--clean]
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
MOGRIX_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Source locations
LLVM_PROJECT="${LLVM_PROJECT:-$HOME/projects/github/llvm-project}"
LIBUNWIND_SRC="$LLVM_PROJECT/libunwind"
LIBCXX_SRC="$LLVM_PROJECT/libcxx"

# Cross-compiler settings
CLANG="/opt/cross/bin/clang"
CLANGXX="/opt/cross/bin/clang++"
SYSROOT="${IRIX_SYSROOT:-/opt/irix-sysroot}"
STAGING="${SGUG_STAGING:-/opt/sgug-staging/usr/sgug}"
IRIX_LD="$STAGING/bin/irix-ld"

# Build directory
BUILD_DIR="$SCRIPT_DIR/build/libunwind"
OUTPUT="$SCRIPT_DIR/libunwind.so.1"

if [ "${1:-}" = "--clean" ]; then
    echo "Cleaning libunwind build..."
    rm -rf "$BUILD_DIR" "$OUTPUT"
    exit 0
fi

mkdir -p "$BUILD_DIR"

# ─── Base flags (matching irix-cc) ───
BASE_CFLAGS="--target=mips-sgi-irix6.5 --sysroot=$SYSROOT -mabi=n32 -march=mips3 -mxgot"
BASE_CFLAGS="$BASE_CFLAGS -O2 -fPIC"
BASE_CFLAGS="$BASE_CFLAGS -D__ELF__ -D__sgi=1 -Dsgi=1"
BASE_CFLAGS="$BASE_CFLAGS -D_SGI_SOURCE -D_SGI_MP_SOURCE -D_SGI_REENTRANT_FUNCTIONS"
BASE_CFLAGS="$BASE_CFLAGS -D_LONGLONG=1 -D_LANGUAGE_C=1 -D__c99"
# sgidefs.h provides __int64_t etc. — but DON'T include for .S files
C_INCLUDE_SGIDEFS="-include $SYSROOT/usr/include/sgidefs.h"

# libunwind-specific flags
UNW_FLAGS="$BASE_CFLAGS"
UNW_FLAGS="$UNW_FLAGS -I$LIBUNWIND_SRC/include"
UNW_FLAGS="$UNW_FLAGS -I$LIBUNWIND_SRC/src"
UNW_FLAGS="$UNW_FLAGS -isystem $SYSROOT/usr/include"
# Enable __register_frame_info() GCC-compatible API (for eh_frame_reg.c)
# Source has this guarded by arch check (x86/ppc only) — we add MIPS via -D
UNW_FLAGS="$UNW_FLAGS -D_LIBUNWIND_SUPPORT_FRAME_APIS"
# IRIX compat: PRIdPTR (C) and __restrict/restrict (C++) fixes
UNW_FLAGS="$UNW_FLAGS -include $SCRIPT_DIR/libunwind-config/irix_cxx_compat.h"
# Build as native-only (MIPS N32, skip other arches)
UNW_FLAGS="$UNW_FLAGS -D_LIBUNWIND_IS_NATIVE_ONLY"
# Don't hide symbols — we need exports
UNW_FLAGS="$UNW_FLAGS -fvisibility=default"

# C++ flags for libunwind.cpp
UNWCXX_FLAGS="$UNW_FLAGS $C_INCLUDE_SGIDEFS -D_LANGUAGE_C_PLUS_PLUS=1 -std=c++14 -nostdinc++ -nostdlibinc"
# IRIX header compat: wchar_t keyword, __restrict→empty (clang C++ treats restrict
# as keyword, IRIX headers use __restrict macro which expands to 'restrict', causing
# duplicate parameter name errors), stdarg va_list
UNWCXX_FLAGS="$UNWCXX_FLAGS -D_WCHAR_T -D_BOOL -D_WCHAR_T_IS_KEYWORD"
# Force-include IRIX C++ compat FIRST — fixes __restrict → empty in C++ mode
# (IRIX sgimacros.h defines __restrict → restrict, causing 'redefinition of
# parameter' errors when clang C++ sees duplicate restrict qualifiers)
UNWCXX_FLAGS="$UNWCXX_FLAGS -include $SCRIPT_DIR/libunwind-config/irix_cxx_compat.h"
UNWCXX_FLAGS="$UNWCXX_FLAGS -include $STAGING/include/dicl-clang-compat/stdarg.h"
UNWCXX_FLAGS="$UNWCXX_FLAGS -isystem $STAGING/include/dicl-clang-compat"
# libc++ headers need __config_site — provide our IRIX version
UNWCXX_FLAGS="$UNWCXX_FLAGS -I$SCRIPT_DIR/libcxx-config"
UNWCXX_FLAGS="$UNWCXX_FLAGS -I$LIBCXX_SRC/include"
# Suppress clang's built-in unwind.h
UNWCXX_FLAGS="$UNWCXX_FLAGS -D__CLANG_UNWIND_H"

# ─── Source files ───
# C sources
C_SOURCES=(
    UnwindLevel1.c
    UnwindLevel1-gcc-ext.c
    Unwind-sjlj.c
)

# C++ sources
CXX_SOURCES=(
    libunwind.cpp
    Unwind_AppleExtras.cpp
)

# Assembly sources
ASM_SOURCES=(
    UnwindRegistersSave.S
    UnwindRegistersRestore.S
)

# ─── Compile ───
echo "=== Building libunwind for IRIX MIPS N32 ==="
ALL_OBJECTS=()

for src in "${C_SOURCES[@]}"; do
    obj="$BUILD_DIR/${src%.c}.o"
    echo "  CC  $src"
    $CLANG $UNW_FLAGS $C_INCLUDE_SGIDEFS -std=c99 -c "$LIBUNWIND_SRC/src/$src" -o "$obj" 2>"$BUILD_DIR/${src%.c}.err" || {
        echo "FAILED: $src"
        cat "$BUILD_DIR/${src%.c}.err"
        exit 1
    }
    ALL_OBJECTS+=("$obj")
done

for src in "${CXX_SOURCES[@]}"; do
    obj="$BUILD_DIR/${src%.cpp}.o"
    echo "  CXX $src"
    # Unwind_AppleExtras.cpp compiles to empty on non-Apple — that's fine
    $CLANGXX $UNWCXX_FLAGS -c "$LIBUNWIND_SRC/src/$src" -o "$obj" 2>"$BUILD_DIR/${src%.cpp}.err" || {
        echo "FAILED: $src"
        cat "$BUILD_DIR/${src%.cpp}.err"
        exit 1
    }
    ALL_OBJECTS+=("$obj")
done

for src in "${ASM_SOURCES[@]}"; do
    obj="$BUILD_DIR/${src%.S}.o"
    echo "  AS  $src"
    $CLANG $UNW_FLAGS -c "$LIBUNWIND_SRC/src/$src" -o "$obj" 2>"$BUILD_DIR/${src%.S}.err" || {
        echo "FAILED: $src"
        cat "$BUILD_DIR/${src%.S}.err"
        exit 1
    }
    ALL_OBJECTS+=("$obj")
done

# IRIX-specific: findUnwindSections implementation via __rld_obj_head
IRIX_SRC="$SCRIPT_DIR/libunwind-config/irix_find_unwind.cpp"
echo "  CXX irix_find_unwind.cpp (findUnwindSections for IRIX)"
$CLANGXX $UNWCXX_FLAGS -c "$IRIX_SRC" -o "$BUILD_DIR/irix_find_unwind.o" 2>"$BUILD_DIR/irix_find_unwind.err" || {
    echo "FAILED: irix_find_unwind.cpp"
    cat "$BUILD_DIR/irix_find_unwind.err"
    exit 1
}
ALL_OBJECTS+=("$BUILD_DIR/irix_find_unwind.o")

echo ""
echo "Compiled ${#ALL_OBJECTS[@]} objects"

# ─── Link ───
echo "  LD  libunwind.so.1"
$IRIX_LD \
    -shared \
    -soname libunwind.so.1 \
    -o "$OUTPUT" \
    "${ALL_OBJECTS[@]}" \
    -L"$SYSROOT/usr/lib32" \
    -lc -lpthread \
    2>"$BUILD_DIR/link.err" || {
    echo "LINK FAILED"
    cat "$BUILD_DIR/link.err"
    exit 1
}

echo ""
echo "=== libunwind.so.1 built ==="
ls -lh "$OUTPUT"
echo ""

# Quick sanity check
echo "Exported symbols:"
nm -D "$OUTPUT" 2>/dev/null | grep -c " T " || true
echo "NEEDED:"
readelf -d "$OUTPUT" 2>/dev/null | grep NEEDED || true
echo ""
echo "Key APIs:"
nm -D "$OUTPUT" 2>/dev/null | grep "T.*__register_frame_info\|T.*_Unwind_RaiseException\|T.*_Unwind_Resume\|T.*unw_init_local\|T.*unw_step" || echo "(check nm output)"
