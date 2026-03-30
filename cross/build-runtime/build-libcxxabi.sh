#!/bin/bash
# Build LLVM libc++abi for IRIX 6.5 MIPS N32
#
# Part of the libc++ migration: libunwind → libc++abi → libc++
# Provides: C++ ABI (exception handling, RTTI, operator new/delete)
#
# Usage: ./build-libcxxabi.sh [--clean]
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
MOGRIX_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Source locations
LLVM_PROJECT="${LLVM_PROJECT:-$HOME/projects/github/llvm-project}"
LIBCXXABI_SRC="$LLVM_PROJECT/libcxxabi"
LIBCXX_SRC="$LLVM_PROJECT/libcxx"
LIBUNWIND_SRC="$LLVM_PROJECT/libunwind"

# Cross-compiler settings
CLANGXX="/opt/cross/bin/clang++"
SYSROOT="${IRIX_SYSROOT:-/opt/irix-sysroot}"
STAGING="${MOGRIX_STAGING:-/opt/sgug-staging/usr/sgug}"
IRIX_LD="$STAGING/bin/irix-ld"

# Build directory
BUILD_DIR="$SCRIPT_DIR/build/libcxxabi"
OUTPUT="$SCRIPT_DIR/libc++abi.so.1"

if [ "${1:-}" = "--clean" ]; then
    echo "Cleaning libc++abi build..."
    rm -rf "$BUILD_DIR" "$OUTPUT"
    exit 0
fi

mkdir -p "$BUILD_DIR"

# ─── Flags ───
CXXFLAGS="--target=mips-sgi-irix6.5 --sysroot=$SYSROOT -mabi=n32 -march=mips3 -mxgot"
CXXFLAGS="$CXXFLAGS -O2 -fPIC -std=c++23 -nostdinc++ -nostdlibinc"
CXXFLAGS="$CXXFLAGS -D__ELF__ -D__sgi=1 -Dsgi=1 -D__c99"
CXXFLAGS="$CXXFLAGS -D_SGI_SOURCE -D_SGI_MP_SOURCE -D_SGI_REENTRANT_FUNCTIONS"
CXXFLAGS="$CXXFLAGS -D_LONGLONG=1 -D_LANGUAGE_C_PLUS_PLUS=1"
CXXFLAGS="$CXXFLAGS -D_WCHAR_T -D_BOOL -D_WCHAR_T_IS_KEYWORD"
CXXFLAGS="$CXXFLAGS -D_LIBCXXABI_BUILDING_LIBRARY"
CXXFLAGS="$CXXFLAGS -D_LIBCPP_BUILDING_LIBRARY"
CXXFLAGS="$CXXFLAGS -fno-use-cxa-atexit -fno-use-init-array"
# libc++abi NEEDS exceptions (personality function uses try/throw).
# Unaligned .eh_frame relocs are handled by post-link strip_eh_relocs.py.
# IRIX compat: restrict keyword, format macros
CXXFLAGS="$CXXFLAGS -include $SCRIPT_DIR/libunwind-config/irix_cxx_compat.h"
CXXFLAGS="$CXXFLAGS -include $SYSROOT/usr/include/sgidefs.h"
# timespec_t is defined in irix_cxx_compat.h (fixes header ordering)
CXXFLAGS="$CXXFLAGS -include $STAGING/include/dicl-clang-compat/stdarg.h"
# Header search order
CXXFLAGS="$CXXFLAGS -I$SCRIPT_DIR/libcxx-config"
CXXFLAGS="$CXXFLAGS -I$LIBCXXABI_SRC/include"
CXXFLAGS="$CXXFLAGS -I$LIBCXXABI_SRC/src"
CXXFLAGS="$CXXFLAGS -I$LIBCXX_SRC/include"
CXXFLAGS="$CXXFLAGS -I$LIBCXX_SRC/src"
CXXFLAGS="$CXXFLAGS -I$LIBUNWIND_SRC/include"
CXXFLAGS="$CXXFLAGS -isystem $STAGING/include/dicl-clang-compat"
CXXFLAGS="$CXXFLAGS -isystem $SYSROOT/usr/include"
# IRIX libc lacks __cxa_thread_atexit_impl — use fallback
# IRIX libc lacks __cxa_thread_atexit_impl — do NOT define the macro.
# The #ifndef path in cxa_thread_atexit.cpp declares it as a weak symbol
# and provides a TLS-based fallback when the weak ref resolves to null.
# constinit: IRIX pthread_mutex_t/cond_t have non-trivial constructors,
# so GlobalStatic<LibcppMutex/CondVar> can't be constinit. Make it a no-op.
CXXFLAGS="$CXXFLAGS -D_LIBCPP_CONSTINIT="
# IRIX ELAST (highest errno) — IRIX errnos go up to 1700+ range
CXXFLAGS="$CXXFLAGS -DELAST=1700"
# Note: __STDCPP_THREADS__ is a clang builtin, no need to define manually
# IRIX wchar_core.h provides const-correct C++ overloads for wcschr etc.
# Tell libc++'s wchar.h wrapper to not generate its own (they fail due to
# IRIX's __SGI_LIBC_USING_FROM_STD namespace machinery).
CXXFLAGS="$CXXFLAGS -D_LIBCPP_WCHAR_H_HAS_CONST_OVERLOADS=1"
# Suppress clang's built-in unwind.h (use LLVM libunwind's)
CXXFLAGS="$CXXFLAGS -D__CLANG_UNWIND_H"
# IRIX lacks __tls_get_addr — we can't use thread_local or __thread.
# cxa_exception_storage.cpp checks __has_feature(cxx_thread_local) which
# we can't override. Instead, compile that file with _LIBCXXABI_HAS_NO_THREADS
# to force the single-threaded path. This is safe because IRIX sproc threads
# share address space (like Linux threads), so a single eh_globals works.
# Handled below in the special-case compilation section.
# IRIX lacks ELF TLS (__tls_get_addr). Disable __thread in cxa_thread_atexit.cpp
# by providing our own stub instead (see IRIX_SUPPORT section below).

# ─── Source files ───
SOURCES=(
    abort_message.cpp
    cxa_aux_runtime.cpp
    cxa_default_handlers.cpp
    cxa_demangle.cpp
    cxa_exception.cpp
    cxa_exception_storage.cpp
    cxa_guard.cpp
    cxa_handlers.cpp
    cxa_personality.cpp
    # cxa_thread_atexit.cpp — excluded: uses __thread (ELF TLS), IRIX lacks __tls_get_addr
    cxa_vector.cpp
    cxa_virtual.cpp
    fallback_malloc.cpp
    private_typeinfo.cpp
    stdlib_exception.cpp
    stdlib_new_delete.cpp
    stdlib_stdexcept.cpp
    stdlib_typeinfo.cpp
)
# Skip cxa_noexception.cpp — it's for -fno-exceptions builds only

# ─── Compile ───
echo "=== Building libc++abi for IRIX MIPS N32 ==="
ALL_OBJECTS=()

for src in "${SOURCES[@]}"; do
    obj="$BUILD_DIR/${src%.cpp}.o"
    EXTRA_FLAGS=""
    # cxa_exception_storage.cpp: force single-thread path to avoid __tls_get_addr
    if [ "$src" = "cxa_exception_storage.cpp" ]; then
        EXTRA_FLAGS="-D_LIBCXXABI_HAS_NO_THREADS"
    fi
    echo "  CXX $src"
    $CLANGXX $CXXFLAGS $EXTRA_FLAGS -c "$LIBCXXABI_SRC/src/$src" -o "$obj" 2>"$BUILD_DIR/${src%.cpp}.err" || {
        echo "FAILED: $src"
        cat "$BUILD_DIR/${src%.cpp}.err"
        exit 1
    }
    ALL_OBJECTS+=("$obj")
done

echo ""
echo "Compiled ${#ALL_OBJECTS[@]} upstream objects"

# ─── IRIX-specific sources ───
IRIX_SRC="$SCRIPT_DIR/libcxxabi-config/cxa_thread_atexit_irix.cpp"
echo "  CXX cxa_thread_atexit_irix.cpp (IRIX pthread_key stub)"
$CLANGXX $CXXFLAGS -c "$IRIX_SRC" -o "$BUILD_DIR/cxa_thread_atexit_irix.o" 2>"$BUILD_DIR/cxa_thread_atexit_irix.err" || {
    echo "FAILED: cxa_thread_atexit_irix.cpp"
    cat "$BUILD_DIR/cxa_thread_atexit_irix.err"
    exit 1
}
ALL_OBJECTS+=("$BUILD_DIR/cxa_thread_atexit_irix.o")

# ─── Link ───
echo "  LD  libc++abi.so.1"
$IRIX_LD \
    -shared \
    -soname libc++abi.so.1 \
    -o "$OUTPUT" \
    "${ALL_OBJECTS[@]}" \
    -L"$SCRIPT_DIR" \
    -L"$SYSROOT/usr/lib32" \
    -lgcc_s -lc -lpthread \
    2>"$BUILD_DIR/link.err" || {
    echo "LINK FAILED"
    cat "$BUILD_DIR/link.err"
    exit 1
}

# Strip unaligned R_MIPS_REL32 from .eh_frame in .rel.dyn.
# IRIX rld crashes (SIGBUS) on unaligned 32-bit writes. MIPS clang generates
# these because personality/LSDA fields in CIE/FDE are at non-4-byte offsets.
STRIP_EH="$SCRIPT_DIR/../bin/strip-eh-relocs"
if [ -f "$STRIP_EH" ]; then
    python3 "$STRIP_EH" "$OUTPUT" 2>&1
fi

echo ""
echo "=== libc++abi.so.1 built ==="
ls -lh "$OUTPUT"
echo ""
echo "Exported symbols:"
nm -D "$OUTPUT" 2>/dev/null | grep -c " T " || true
echo "NEEDED:"
readelf -d "$OUTPUT" 2>/dev/null | grep NEEDED || true
echo ""
echo "Key APIs:"
nm -D "$OUTPUT" 2>/dev/null | grep "T.*__gxx_personality_v0\|T.*__cxa_throw\|T.*__cxa_begin_catch\|T.*__cxa_end_catch\|T.*__dynamic_cast\|T.*_Znwj\|T.*_ZdlPv" || echo "(check nm output)"
