#!/bin/bash
# Build LLVM libc++ for IRIX 6.5 MIPS N32
#
# Part of the libc++ migration: libunwind → libc++abi → libc++
# The main C++ standard library. Replaces GCC's libstdc++.
#
# Usage: ./build-libcxx.sh [--clean]
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Source locations
# LLVM source tree — set to your llvm-project checkout or extracted tarball.
# For LLVM 22: LLVM_PROJECT=~/projects/github/unxmaal/mogrix/tmp/llvm-build/llvm-project-22.1.2.src
LLVM_PROJECT="${LLVM_PROJECT:-$HOME/projects/github/llvm-project}"
LIBCXX_SRC="$LLVM_PROJECT/libcxx"
LIBCXXABI_SRC="$LLVM_PROJECT/libcxxabi"
LIBUNWIND_SRC="$LLVM_PROJECT/libunwind"

# Cross-compiler settings
CLANGXX="/opt/cross/bin/clang++"
SYSROOT="${IRIX_SYSROOT:-/opt/irix-sysroot}"
STAGING="${MOGRIX_STAGING:-/opt/sgug-staging/usr/sgug}"
IRIX_LD="$STAGING/bin/irix-ld"

# Build directory
BUILD_DIR="$SCRIPT_DIR/build/libcxx"
OUTPUT="$SCRIPT_DIR/libc++.so.1"

if [ "${1:-}" = "--clean" ]; then
    echo "Cleaning libc++ build..."
    rm -rf "$BUILD_DIR" "$OUTPUT"
    exit 0
fi

mkdir -p "$BUILD_DIR" "$BUILD_DIR/filesystem" "$BUILD_DIR/ryu"

# ─── Flags ───
CXXFLAGS="--target=mips-sgi-irix6.5 --sysroot=$SYSROOT -mabi=n32 -march=mips3 -mxgot"
CXXFLAGS="$CXXFLAGS -O2 -fPIC -std=c++23 -nostdinc++ -nostdlibinc"
CXXFLAGS="$CXXFLAGS -D__ELF__ -D__sgi=1 -Dsgi=1 -D__c99"
CXXFLAGS="$CXXFLAGS -D_SGI_SOURCE -D_SGI_MP_SOURCE -D_SGI_REENTRANT_FUNCTIONS"
CXXFLAGS="$CXXFLAGS -D_LONGLONG=1 -D_LANGUAGE_C_PLUS_PLUS=1"
CXXFLAGS="$CXXFLAGS -D_WCHAR_T -D_BOOL -D_WCHAR_T_IS_KEYWORD"
CXXFLAGS="$CXXFLAGS -D_LIBCPP_BUILDING_LIBRARY"
CXXFLAGS="$CXXFLAGS -fno-use-cxa-atexit -fno-use-init-array"
# Eliminate unaligned R_MIPS_REL32 in .eh_frame (IRIX rld crash).
# -fno-exceptions: removes personality/LSDA absptr entries from .eh_frame
# -funwind-tables: keeps basic FDE info so unwinder can step through frames
CXXFLAGS="$CXXFLAGS -fno-exceptions -funwind-tables"
# IRIX compat
CXXFLAGS="$CXXFLAGS -include $SCRIPT_DIR/libunwind-config/irix_cxx_compat.h"
CXXFLAGS="$CXXFLAGS -include $SYSROOT/usr/include/sgidefs.h"
CXXFLAGS="$CXXFLAGS -include $STAGING/include/dicl-clang-compat/stdarg.h"
# constinit: IRIX pthread_mutex_t/cond_t have non-trivial constructors
CXXFLAGS="$CXXFLAGS -D_LIBCPP_CONSTINIT="
# IRIX ELAST (highest errno) — IRIX errnos go up to 1700+ range
CXXFLAGS="$CXXFLAGS -DELAST=1700"
# Block IRIX sys/param.h — its 'roundup' macro collides with libc++ function
# names in memory_resource.cpp. sys/param.h is transitively included via
# sched.h → sys/sched.h → sys/param.h. Blocking it is safe because libc++
# doesn't need anything from it.
CXXFLAGS="$CXXFLAGS -D_SYS_PARAM_H"
# IRIX wchar_core.h provides const-correct C++ overloads for wcschr etc.
CXXFLAGS="$CXXFLAGS -D_LIBCPP_WCHAR_H_HAS_CONST_OVERLOADS=1"
# IRIX ctype table format is non-standard — use libc++ built-in rune table
CXXFLAGS="$CXXFLAGS -D_LIBCPP_PROVIDES_DEFAULT_RUNE_TABLE"
# Suppress clang's built-in unwind.h (use LLVM libunwind's)
CXXFLAGS="$CXXFLAGS -D__CLANG_UNWIND_H"
# POSIX.1-2008 filesystem stubs (utimensat, openat, unlinkat, fdopendir)
CXXFLAGS="$CXXFLAGS -include $SCRIPT_DIR/libcxx-config/irix_posix_compat.h"
# Header search order
CXXFLAGS="$CXXFLAGS -I$SCRIPT_DIR/libcxx-config"
CXXFLAGS="$CXXFLAGS -I$LIBCXX_SRC/include"
CXXFLAGS="$CXXFLAGS -I$LIBCXX_SRC/src"
# LLVM 22 charconv uses shared headers from LLVM libc
CXXFLAGS="$CXXFLAGS -I$LLVM_PROJECT/libc"
CXXFLAGS="$CXXFLAGS -I$LIBCXXABI_SRC/include"
CXXFLAGS="$CXXFLAGS -I$LIBUNWIND_SRC/include"
# IRIX xlocale stubs (locale_t, uselocale, *_l functions)
CXXFLAGS="$CXXFLAGS -isystem $STAGING/include/mogrix-compat"
CXXFLAGS="$CXXFLAGS -isystem $STAGING/include/dicl-clang-compat"
CXXFLAGS="$CXXFLAGS -isystem $SYSROOT/usr/include"

# ─── Source files ───
SOURCES=(
    algorithm.cpp
    any.cpp
    atomic.cpp
    barrier.cpp
    bind.cpp
    # charconv.cpp — MIPS long double not recognized by LLVM libc FPBits (str_to_float.h)
    chrono.cpp
    condition_variable.cpp
    condition_variable_destructor.cpp
    exception.cpp
    functional.cpp
    future.cpp
    hash.cpp
    ios.cpp
    ios.instantiations.cpp
    iostream.cpp
    locale.cpp
    memory.cpp
    mutex.cpp
    mutex_destructor.cpp
    new.cpp
    optional.cpp
    # random.cpp — requires /dev/urandom (_LIBCPP_HAS_RANDOM_DEVICE=0)
    random_shuffle.cpp
    regex.cpp
    shared_mutex.cpp
    stdexcept.cpp
    string.cpp
    strstream.cpp
    system_error.cpp
    thread.cpp
    typeinfo.cpp
    valarray.cpp
    variant.cpp
    vector.cpp
    verbose_abort.cpp
    # LLVM 22 new files
    call_once.cpp
    error_category.cpp
    expected.cpp
    fstream.cpp
    memory_resource.cpp
    new_handler.cpp
    new_helpers.cpp
    ostream.cpp
    print.cpp
    # Ryu float-to-string (used by charconv/format)
    ryu/d2fixed.cpp
    ryu/d2s.cpp
    ryu/f2s.cpp
    filesystem/operations.cpp
    filesystem/directory_entry.cpp
    filesystem/directory_iterator.cpp
    filesystem/filesystem_clock.cpp
    filesystem/filesystem_error.cpp
    filesystem/int128_builtins.cpp
    filesystem/path.cpp
)

# ─── Compile ───
echo "=== Building libc++ for IRIX MIPS N32 ==="
ALL_OBJECTS=()
compiled=0
failed=0

for src in "${SOURCES[@]}"; do
    obj="$BUILD_DIR/${src%.cpp}.o"
    echo "  CXX $src"
    if $CLANGXX $CXXFLAGS -c "$LIBCXX_SRC/src/$src" -o "$obj" 2>"$BUILD_DIR/${src%.cpp}.err"; then
        compiled=$((compiled + 1))
        ALL_OBJECTS+=("$obj")
    else
        echo "  FAILED: $src"
        head -3 "$BUILD_DIR/${src%.cpp}.err"
        failed=$((failed + 1))
    fi
done

echo ""
echo "Compiled: $compiled, Failed: $failed"

if [ $failed -gt 0 ]; then
    echo "WARNING: $failed files failed to compile. Attempting partial link..."
fi

if [ ${#ALL_OBJECTS[@]} -eq 0 ]; then
    echo "ERROR: No objects compiled"
    exit 1
fi

# ─── IRIX support files (C, not C++) ───
# These provide __bsd_mbsnrtowcs/__bsd_wcsnrtombs — POSIX 2008 wchar
# functions missing from IRIX libc. Compiled as C for correct linkage.
CLANG="${CLANGXX%++}"  # clang++ → clang
CFLAGS="--target=mips-sgi-irix6.5 --sysroot=$SYSROOT -mabi=n32 -march=mips3 -mxgot"
CFLAGS="$CFLAGS -O2 -fPIC -D__ELF__ -D__sgi=1 -Dsgi=1 -D__c99"
CFLAGS="$CFLAGS -D_SGI_SOURCE -D_SGI_MP_SOURCE -D_SGI_REENTRANT_FUNCTIONS"
CFLAGS="$CFLAGS -D_LONGLONG=1 -D_LANGUAGE_C=1"
CFLAGS="$CFLAGS -include $SYSROOT/usr/include/sgidefs.h"
CFLAGS="$CFLAGS -isystem $SYSROOT/usr/include"

# IRIX support files — stored in mogrix repo, not in LLVM source tree
IRIX_FILES_DIR="$SCRIPT_DIR/irix-libcxx-files/src"
IRIX_SUPPORT=(
    mbsnrtowcs.c
    wcsnrtombs.c
    iswblank.c
)

for src in "${IRIX_SUPPORT[@]}"; do
    obj="$BUILD_DIR/${src%.c}.o"
    echo "  CC  $src"
    if $CLANG $CFLAGS -c "$IRIX_FILES_DIR/$src" -o "$obj" 2>"$BUILD_DIR/${src}.err"; then
        compiled=$((compiled + 1))
        ALL_OBJECTS+=("$obj")
    else
        echo "  FAILED: $src"
        cat "$BUILD_DIR/${src##*/}.err"
        failed=$((failed + 1))
    fi
done

# ─── Link ───
echo "  LD  libc++.so.1"
ln -sf libc++abi.so.1 "$SCRIPT_DIR/libc++abi.so" 2>/dev/null || true

$IRIX_LD \
    -shared \
    -soname libc++.so.1 \
    -o "$OUTPUT" \
    "${ALL_OBJECTS[@]}" \
    -L"$SCRIPT_DIR" \
    -L"$SYSROOT/usr/lib32" \
    -lc++abi -lgcc_s -lc -lpthread -lm \
    2>"$BUILD_DIR/link.err" || {
    echo "LINK FAILED"
    cat "$BUILD_DIR/link.err"
    exit 1
}

echo ""
echo "=== libc++.so.1 built ==="
ls -lh "$OUTPUT"
echo ""
echo "Exported symbols: $(nm -D "$OUTPUT" 2>/dev/null | grep -c ' T ' || true)"
echo "NEEDED:"
readelf -d "$OUTPUT" 2>/dev/null | grep NEEDED || true
echo ""
echo "Compiled $compiled/$((compiled + failed)) sources"
