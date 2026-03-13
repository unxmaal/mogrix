#!/bin/bash
# Build libstdc++.so.6 from GCC 9.5.0 source using clang cross-compiler
#
# Compiles GCC 9's libstdc++-v3 source with our clang 16 for MIPS n32 IRIX.
# Uses the existing c++config.h from cross/include/c++/9/ with overrides
# for functions IRIX libc doesn't actually have (via libstdcxx_compat.h).
#
# IRIX OS config files extracted from SGUG-RSE's gcc.sgifixes.patch.
#
# Usage: ./build-libstdcxx.sh [--clean]
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
MOGRIX_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

# Source locations
GCC_SRC="$SCRIPT_DIR/gcc-9.5.0"
LIBSTDCXX="$GCC_SRC/libstdc++-v3"
GCC_CFG="$SCRIPT_DIR/gcc-config"

# Cross-compiler settings
CLANG="/opt/cross/bin/clang++"
SYSROOT="${IRIX_SYSROOT:-/opt/irix-sysroot}"
STAGING="${SGUG_STAGING:-/opt/sgug-staging/usr/sgug}"
IRIX_LD="$STAGING/bin/irix-ld"

# Base flags matching irix-cxx
CXX_FLAGS="--target=mips-sgi-irix6.5 --sysroot=$SYSROOT -mabi=n32 -march=mips3 -mxgot"
CXX_FLAGS="$CXX_FLAGS -O2 -fPIC -std=gnu++11"
CXX_FLAGS="$CXX_FLAGS -U_MIPS_ISA -D_MIPS_ISA=_MIPS_ISA_MIPS3"
CXX_FLAGS="$CXX_FLAGS -nostdlibinc -nostdinc++"
# Force-include our compat header FIRST (handles __restrict, wchar overrides, SGI namespace macros)
CXX_FLAGS="$CXX_FLAGS -include $GCC_CFG/libstdcxx_compat.h"
# Force-include stdarg.h for va_list compat
CXX_FLAGS="$CXX_FLAGS -include $STAGING/include/dicl-clang-compat/stdarg.h"
# Force-include time.h for struct timespec
CXX_FLAGS="$CXX_FLAGS -include $SYSROOT/usr/include/time.h"
# Header search order (same as irix-cxx)
CXX_FLAGS="$CXX_FLAGS -isystem $STAGING/include/c++/9"
CXX_FLAGS="$CXX_FLAGS -isystem $STAGING/include/c++/9/mips-sgi-irix6.5"
CXX_FLAGS="$CXX_FLAGS -isystem $STAGING/include/mogrix-compat/generic"
CXX_FLAGS="$CXX_FLAGS -isystem $STAGING/include/dicl-clang-compat"
CXX_FLAGS="$CXX_FLAGS -isystem $STAGING/include"
CXX_FLAGS="$CXX_FLAGS -isystem $SYSROOT/usr/include"
# IRIX compatibility defines
CXX_FLAGS="$CXX_FLAGS -D_SGI_SOURCE -D_SGI_MP_SOURCE -D_SGI_REENTRANT_FUNCTIONS"
CXX_FLAGS="$CXX_FLAGS -Dunix=1 -D__unix__=1 -D__unix=1"
CXX_FLAGS="$CXX_FLAGS -D__sgi=1 -D_LANGUAGE_C_PLUS_PLUS=1 -D_LONGLONG=1"
CXX_FLAGS="$CXX_FLAGS -D_BOOL -D_WCHAR_T_IS_KEYWORD -D_WCHAR_T"
CXX_FLAGS="$CXX_FLAGS -D__ELF__"
# C99 math support (same as irix-cxx)
CXX_FLAGS="$CXX_FLAGS -D_GLIBCXX_USE_C99_MATH=1"
CXX_FLAGS="$CXX_FLAGS -D_GLIBCXX_USE_C99_MATH_TR1=1"
# IRIX ABI
CXX_FLAGS="$CXX_FLAGS -fno-use-cxa-atexit -fno-use-init-array"

# libstdc++ internal include paths
CXX_FLAGS="$CXX_FLAGS -I$LIBSTDCXX/src"
CXX_FLAGS="$CXX_FLAGS -I$LIBSTDCXX/src/c++11"
CXX_FLAGS="$CXX_FLAGS -I$LIBSTDCXX/src/c++98"
CXX_FLAGS="$CXX_FLAGS -I$LIBSTDCXX/src/filesystem"
CXX_FLAGS="$CXX_FLAGS -I$LIBSTDCXX/libsupc++"
# GCC internal headers (for gthr.h etc.)
CXX_FLAGS="$CXX_FLAGS -I$GCC_SRC/libgcc"
CXX_FLAGS="$CXX_FLAGS -I$GCC_SRC/include"
# Use IRIX-specific config from SGUG-RSE patches (ctype, os_defines, atomicity)
CXX_FLAGS="$CXX_FLAGS -I$LIBSTDCXX/config/os/irix/irix6.5"
CXX_FLAGS="$CXX_FLAGS -I$LIBSTDCXX/config/os/irix"
CXX_FLAGS="$CXX_FLAGS -I$LIBSTDCXX/config/locale/generic"
CXX_FLAGS="$CXX_FLAGS -I$LIBSTDCXX/config"

# Backward-compat headers (strstream etc.)
CXX_FLAGS="$CXX_FLAGS -I$LIBSTDCXX/include/backward"
# gcc-config directory (bits/largefile-config.h for filesystem)
CXX_FLAGS="$CXX_FLAGS -I$GCC_CFG"
# Suppress noisy warnings
CXX_FLAGS="$CXX_FLAGS -Wno-deprecated -Wno-attributes -Wno-unused-function"
CXX_FLAGS="$CXX_FLAGS -Wno-missing-exception-spec"

# Output
BUILD_DIR="$SCRIPT_DIR/build/libstdcxx"
OUTPUT="$SCRIPT_DIR/libstdc++.so.6"

if [ "${1:-}" = "--clean" ]; then
    rm -rf "$BUILD_DIR" "$OUTPUT"
    echo "Cleaned."
    exit 0
fi

mkdir -p "$BUILD_DIR/c++98" "$BUILD_DIR/c++11" "$BUILD_DIR/c++17" "$BUILD_DIR/filesystem" "$BUILD_DIR/supc++" "$BUILD_DIR/locale"

# Check GCC source
if [ ! -d "$LIBSTDCXX/src" ]; then
    echo "ERROR: libstdc++-v3 source not found at $LIBSTDCXX"
    exit 1
fi

# Ensure gthr-default.h exists
[ ! -e "$GCC_SRC/libgcc/gthr-default.h" ] && cp "$GCC_SRC/libgcc/gthr-posix.h" "$GCC_SRC/libgcc/gthr-default.h"

echo "=== Phase 1: Compile C++98 sources ==="

# C++98 sources (compiled with -std=gnu++98)
# snprintf_lite/streambuf-inst/wlocale-inst/wstring-inst are in c++11, not c++98
# locale_init.cc and localename.cc use char16_t/char32_t — compile as C++11 (see below)
CXX98_SOURCES=(
    allocator-inst.cc
    bitmap_allocator.cc
    codecvt.cc
    complex_io.cc
    concept-inst.cc
    cow-istream-string.cc
    ext-inst.cc
    globals_io.cc
    hash_tr1.cc
    hashtable_tr1.cc
    ios_failure.cc
    ios_init.cc
    ios_locale.cc
    istream-string.cc
    istream.cc
    list.cc
    list-aux.cc
    list-aux-2.cc
    list_associated.cc
    list_associated-2.cc
    locale.cc
    locale_facets.cc
    math_stubs_float.cc
    math_stubs_long_double.cc
    misc-inst.cc
    mt_allocator.cc
    parallel_settings.cc
    pool_allocator.cc
    stdexcept.cc
    streambuf.cc
    strstream.cc
    tree.cc
    valarray.cc
)

# Compatibility sources for C++98
CXX98_COMPAT_SOURCES=(
    compatibility.cc
    compatibility-debug_list.cc
    compatibility-debug_list-2.cc
)

CXX98_FLAGS="${CXX_FLAGS/-std=gnu++11/-std=gnu++98}"

compiled=0
failed=0
for src in "${CXX98_SOURCES[@]}" "${CXX98_COMPAT_SOURCES[@]}"; do
    obj="$BUILD_DIR/c++98/${src%.cc}.o"
    srcpath="$LIBSTDCXX/src/c++98/$src"
    if [ ! -f "$srcpath" ]; then
        echo "MISSING: c++98/$src"
        failed=$((failed + 1))
        continue
    fi
    if $CLANG $CXX98_FLAGS -c "$srcpath" -o "$obj" 2>"$BUILD_DIR/c++98/${src%.cc}.err"; then
        compiled=$((compiled + 1))
    else
        echo "FAILED: c++98/$src"
        head -5 "$BUILD_DIR/c++98/${src%.cc}.err"
        failed=$((failed + 1))
    fi
done
# locale_init.cc and localename.cc use char16_t/char32_t — must compile as C++11
for src in locale_init.cc localename.cc; do
    obj="$BUILD_DIR/c++98/${src%.cc}.o"
    srcpath="$LIBSTDCXX/src/c++98/$src"
    if $CLANG $CXX_FLAGS -c "$srcpath" -o "$obj" 2>"$BUILD_DIR/c++98/${src%.cc}.err"; then
        compiled=$((compiled + 1))
    else
        echo "FAILED: c++98/$src (as C++11)"
        head -5 "$BUILD_DIR/c++98/${src%.cc}.err"
        failed=$((failed + 1))
    fi
done
echo "C++98: $compiled compiled, $failed failed"

echo ""
echo "=== Phase 2: Compile C++11 sources ==="

# C++11 sources
CXX11_SOURCES=(
    chrono.cc
    codecvt.cc
    condition_variable.cc
    cow-fstream-inst.cc
    cow-locale_init.cc
    cow-shim_facets.cc
    cow-sstream-inst.cc
    cow-stdexcept.cc
    cow-string-inst.cc
    cow-string-io-inst.cc
    cow-wstring-inst.cc
    cow-wstring-io-inst.cc
    ctype.cc
    cxx11-hash_tr1.cc
    cxx11-ios_failure.cc
    cxx11-locale-inst.cc
    cxx11-shim_facets.cc
    cxx11-stdexcept.cc
    cxx11-wlocale-inst.cc
    debug.cc
    ext11-inst.cc
    fstream-inst.cc
    functexcept.cc
    functional.cc
    futex.cc
    future.cc
    hash_c++0x.cc
    hashtable_c++0x.cc
    ios.cc
    ios-inst.cc
    iostream-inst.cc
    istream-inst.cc
    limits.cc
    locale-inst.cc
    mutex.cc
    ostream-inst.cc
    placeholders.cc
    random.cc
    regex.cc
    shared_ptr.cc
    snprintf_lite.cc
    sso_string.cc
    sstream-inst.cc
    streambuf-inst.cc
    string-inst.cc
    string-io-inst.cc
    system_error.cc
    thread.cc
    wlocale-inst.cc
    wstring-inst.cc
    wstring-io-inst.cc
)

# Compatibility sources
CXX11_COMPAT_SOURCES=(
    compatibility-c++0x.cc
    compatibility-atomic-c++0x.cc
    compatibility-thread-c++0x.cc
    compatibility-chrono.cc
    compatibility-condvar.cc
)

compiled11=0
failed11=0
for src in "${CXX11_SOURCES[@]}" "${CXX11_COMPAT_SOURCES[@]}"; do
    obj="$BUILD_DIR/c++11/${src%.cc}.o"
    srcpath="$LIBSTDCXX/src/c++11/$src"
    if [ ! -f "$srcpath" ]; then
        echo "MISSING: c++11/$src"
        failed11=$((failed11 + 1))
        continue
    fi
    if $CLANG $CXX_FLAGS -c "$srcpath" -o "$obj" 2>"$BUILD_DIR/c++11/${src%.cc}.err"; then
        compiled11=$((compiled11 + 1))
    else
        echo "FAILED: c++11/$src"
        head -5 "$BUILD_DIR/c++11/${src%.cc}.err"
        failed11=$((failed11 + 1))
    fi
done
echo "C++11: $compiled11 compiled, $failed11 failed"

echo ""
echo "=== Phase 3: Compile C++17 sources ==="

# C++17 sources
# Skip: floating_from/to_chars (GCC 9.5 doesn't have them)
# Note: memory_resource.cc is replaced by pmr_shim.cc (Phase 4c) —
# GCC's version fails with clang (out-of-line virtual dtor = default)
CXX17_SOURCES=(
    cow-string-inst.cc
    fs_dir.cc
    fs_ops.cc
    fs_path.cc
    cow-fs_dir.cc
    cow-fs_ops.cc
    cow-fs_path.cc
    ostream-inst.cc
    string-inst.cc
)

# -fsized-deallocation: clang 16 disables it by default; needed for operator delete(void*, size_t)
CXX17_FLAGS="${CXX_FLAGS/-std=gnu++11/-std=gnu++17} -fsized-deallocation"

compiled17=0
failed17=0
for src in "${CXX17_SOURCES[@]}"; do
    obj="$BUILD_DIR/c++17/${src%.cc}.o"
    srcpath="$LIBSTDCXX/src/c++17/$src"
    if [ ! -f "$srcpath" ]; then
        echo "MISSING: c++17/$src"
        failed17=$((failed17 + 1))
        continue
    fi
    # fs_path.cc and cow-fs_path.cc: patch noexcept mismatch between GCC 9.5.0 source
    # and staging headers (SGUG-RSE GCC 9.2.0). Source adds noexcept to _List::begin/end
    # but staging header doesn't have it — clang rejects the mismatch.
    compile_src="$srcpath"
    extra_flags=""
    if [[ "$src" == "fs_path.cc" ]]; then
        # Patch fs_path.cc and put it where cow-fs_path.cc can also find it
        patched="$BUILD_DIR/c++17/fs_path.cc"
        sed 's/::begin() noexcept/::begin()/g; s/::end() noexcept/::end()/g; s/::begin() const noexcept/::begin() const/g; s/::end() const noexcept/::end() const/g' "$srcpath" > "$patched"
        compile_src="$patched"
    elif [[ "$src" == "cow-fs_path.cc" ]]; then
        # cow-fs_path.cc is just: #define _GLIBCXX_USE_CXX11_ABI 0 + #include "fs_path.cc"
        # Compile patched fs_path.cc with cow ABI flag instead
        compile_src="$BUILD_DIR/c++17/fs_path.cc"
        extra_flags="-D_GLIBCXX_USE_CXX11_ABI=0"
    fi
    if $CLANG $CXX17_FLAGS $extra_flags -c "$compile_src" -o "$obj" 2>"$BUILD_DIR/c++17/${src%.cc}.err"; then
        compiled17=$((compiled17 + 1))
    else
        echo "FAILED: c++17/$src"
        head -5 "$BUILD_DIR/c++17/${src%.cc}.err"
        failed17=$((failed17 + 1))
    fi
done
echo "C++17: $compiled17 compiled, $failed17 failed"

# Skip Phase 3b: Filesystem TS v1 (experimental::filesystem)
# GCC 9 headers have noexcept mismatch that clang rejects as hard error.
# These provide deprecated std::experimental::filesystem — deferring until needed.
compiledfs=0
failedfs=0

echo ""
echo "=== Phase 4: Compile libsupc++ ==="

# libsupc++ — C++ support library (exception handling, RTTI, new/delete)
# cp-demangle is from libiberty — compile as C
SUPCXX_SOURCES=(
    array_type_info.cc
    atexit_arm.cc
    atexit_thread.cc
    bad_alloc.cc
    bad_array_length.cc
    bad_array_new.cc
    bad_cast.cc
    bad_typeid.cc
    class_type_info.cc
    del_op.cc
    del_ops.cc
    del_opnt.cc
    del_opv.cc
    del_opvs.cc
    del_opvnt.cc
    dyncast.cc
    eh_alloc.cc
    eh_arm.cc
    eh_aux_runtime.cc
    eh_call.cc
    eh_catch.cc
    eh_exception.cc
    eh_globals.cc
    eh_personality.cc
    eh_ptr.cc
    eh_term_handler.cc
    eh_terminate.cc
    eh_throw.cc
    eh_tm.cc
    eh_type.cc
    eh_unex_handler.cc
    enum_type_info.cc
    function_type_info.cc
    fundamental_type_info.cc
    guard.cc
    guard_error.cc
    hash_bytes.cc
    nested_exception.cc
    new_handler.cc
    new_op.cc
    new_opnt.cc
    new_opv.cc
    new_opvnt.cc
    pbase_type_info.cc
    pmem_type_info.cc
    pointer_type_info.cc
    pure.cc
    si_class_type_info.cc
    tinfo.cc
    tinfo2.cc
    vec.cc
    vmi_class_type_info.cc
    vterminate.cc
    vtv_stubs.cc
)

compiledsupc=0
failedsupc=0
for src in "${SUPCXX_SOURCES[@]}"; do
    obj="$BUILD_DIR/supc++/${src%.cc}.o"
    srcpath="$LIBSTDCXX/libsupc++/$src"
    if [ ! -f "$srcpath" ]; then
        echo "MISSING: libsupc++/$src"
        failedsupc=$((failedsupc + 1))
        continue
    fi
    if $CLANG $CXX_FLAGS -I$LIBSTDCXX/libsupc++ -c "$srcpath" -o "$obj" 2>"$BUILD_DIR/supc++/${src%.cc}.err"; then
        compiledsupc=$((compiledsupc + 1))
    else
        echo "FAILED: libsupc++/$src"
        head -5 "$BUILD_DIR/supc++/${src%.cc}.err"
        failedsupc=$((failedsupc + 1))
    fi
done

# cp-demangle.c from libiberty (compiled as C)
CLANG_C="/opt/cross/bin/clang"
CP_DEMANGLE_SRC="$GCC_SRC/libiberty/cp-demangle.c"
if [ -f "$CP_DEMANGLE_SRC" ]; then
    C_FLAGS="--target=mips-sgi-irix6.5 --sysroot=$SYSROOT -mabi=n32 -march=mips3 -mxgot"
    C_FLAGS="$C_FLAGS -O2 -fPIC -std=c99 -D__ELF__"
    C_FLAGS="$C_FLAGS -DIN_GLIBCPP_V3 -DHAVE_STDLIB_H -DHAVE_STRING_H -D_LANGUAGE_C=1"
    C_FLAGS="$C_FLAGS -I$GCC_SRC/include"
    C_FLAGS="$C_FLAGS -D_LONGLONG=1 -D_SGI_SOURCE"
    C_FLAGS="$C_FLAGS -include $SYSROOT/usr/include/sgidefs.h"
    C_FLAGS="$C_FLAGS -include $SYSROOT/usr/include/stdio.h"
    C_FLAGS="$C_FLAGS -isystem $SYSROOT/usr/include"
    if $CLANG_C $C_FLAGS -c "$CP_DEMANGLE_SRC" -o "$BUILD_DIR/supc++/cp-demangle.o" 2>"$BUILD_DIR/supc++/cp-demangle.err"; then
        compiledsupc=$((compiledsupc + 1))
    else
        echo "FAILED: libiberty/cp-demangle.c"
        head -5 "$BUILD_DIR/supc++/cp-demangle.err"
        failedsupc=$((failedsupc + 1))
    fi
else
    echo "MISSING: $CP_DEMANGLE_SRC"
    failedsupc=$((failedsupc + 1))
fi
echo "libsupc++: $compiledsupc compiled, $failedsupc failed"

echo ""
echo "=== Phase 4b: Compile C++17 aligned new/delete (libsupc++, needs -std=gnu++17) ==="

# Aligned new/delete operators use std::align_val_t (C++17)
SUPCXX17_SOURCES=(
    del_opa.cc
    del_opant.cc
    del_opsa.cc
    del_opva.cc
    del_opvant.cc
    del_opvsa.cc
    new_opa.cc
    new_opant.cc
    new_opva.cc
    new_opvant.cc
)

SUPCXX17_FLAGS="${CXX_FLAGS/-std=gnu++11/-std=gnu++17}"

compiledsupc17=0
failedsupc17=0
for src in "${SUPCXX17_SOURCES[@]}"; do
    obj="$BUILD_DIR/supc++/${src%.cc}.o"
    srcpath="$LIBSTDCXX/libsupc++/$src"
    if [ ! -f "$srcpath" ]; then
        echo "MISSING: libsupc++/$src"
        failedsupc17=$((failedsupc17 + 1))
        continue
    fi
    if $CLANG $SUPCXX17_FLAGS -I$LIBSTDCXX/libsupc++ -c "$srcpath" -o "$obj" 2>"$BUILD_DIR/supc++/${src%.cc}.err"; then
        compiledsupc17=$((compiledsupc17 + 1))
    else
        echo "FAILED: libsupc++/$src (C++17)"
        head -5 "$BUILD_DIR/supc++/${src%.cc}.err"
        failedsupc17=$((failedsupc17 + 1))
    fi
done
echo "libsupc++ C++17: $compiledsupc17 compiled, $failedsupc17 failed"

echo ""
echo "=== Phase 4c: Compile std::pmr shim (C++17) ==="

# GCC 9's memory_resource.cc fails with clang due to out-of-line "= default"
# virtual dtor and 3-arg operator delete. This shim provides just the symbols
# Qt5 needs: get_default_resource, _M_new_buffer, _M_release_buffers.
PMR_SHIM="$SCRIPT_DIR/pmr_shim.cc"
PMR_OBJ="$BUILD_DIR/c++17/pmr_shim.o"
if $CLANG $SUPCXX17_FLAGS -I$LIBSTDCXX/include -I$LIBSTDCXX/src -c "$PMR_SHIM" -o "$PMR_OBJ" 2>"$BUILD_DIR/c++17/pmr_shim.err"; then
    echo "pmr_shim.o: OK"
else
    echo "FAILED: pmr_shim.cc"
    cat "$BUILD_DIR/c++17/pmr_shim.err"
    # Non-fatal — only Qt5 needs it
fi

echo ""
echo "=== Phase 5: Compile I/O backend + locale backend (generic) ==="

# basic_file_stdio.cc — provides std::__basic_file<char> (file I/O for fstream)
BASIC_FILE_SRC="$LIBSTDCXX/config/io/basic_file_stdio.cc"
if [ -f "$BASIC_FILE_SRC" ]; then
    if $CLANG $CXX98_FLAGS -I$LIBSTDCXX/config/io -c "$BASIC_FILE_SRC" -o "$BUILD_DIR/locale/basic_file_stdio.o" 2>"$BUILD_DIR/locale/basic_file_stdio.err"; then
        echo "basic_file_stdio.cc: OK"
    else
        echo "FAILED: basic_file_stdio.cc"
        head -5 "$BUILD_DIR/locale/basic_file_stdio.err"
    fi
else
    echo "MISSING: config/io/basic_file_stdio.cc"
fi

LOCALE_SOURCES=(
    c_locale.cc
    codecvt_members.cc
    collate_members.cc
    ctype_members.cc
    messages_members.cc
    monetary_members.cc
    numeric_members.cc
    # time_members.cc — IRIX wcsftime(3rd arg=const char*) conflicts with C++ (const wchar_t*)
    # Compile separately below with a local fix
)

# IRIX-specific ctype backend
IRIX_LOCALE_SOURCES=(
    ctype_configure_char.cc
)

compiledloc=0
failedloc=0
for src in "${LOCALE_SOURCES[@]}"; do
    obj="$BUILD_DIR/locale/${src%.cc}.o"
    srcpath="$LIBSTDCXX/config/locale/generic/$src"
    if [ ! -f "$srcpath" ]; then
        echo "MISSING: locale/generic/$src"
        failedloc=$((failedloc + 1))
        continue
    fi
    if $CLANG $CXX98_FLAGS -c "$srcpath" -o "$obj" 2>"$BUILD_DIR/locale/${src%.cc}.err"; then
        compiledloc=$((compiledloc + 1))
    else
        echo "FAILED: locale/generic/$src"
        head -5 "$BUILD_DIR/locale/${src%.cc}.err"
        failedloc=$((failedloc + 1))
    fi
done
# IRIX ctype config
for src in "${IRIX_LOCALE_SOURCES[@]}"; do
    obj="$BUILD_DIR/locale/${src%.cc}.o"
    srcpath="$LIBSTDCXX/config/os/irix/irix6.5/$src"
    if [ -f "$srcpath" ]; then
        if $CLANG $CXX98_FLAGS -c "$srcpath" -o "$obj" 2>"$BUILD_DIR/locale/${src%.cc}.err"; then
            compiledloc=$((compiledloc + 1))
        else
            echo "FAILED: os/irix/$src"
            head -5 "$BUILD_DIR/locale/${src%.cc}.err"
            failedloc=$((failedloc + 1))
        fi
    fi
done
# time_members.cc needs special handling: IRIX wcsftime(format=const char*)
# differs from C++ standard (format=const wchar_t*). Fix: patch a copy to
# call _xpg5_wcsftime (IRIX's XPG5-compliant version with wchar_t format).
TIME_SRC="$LIBSTDCXX/config/locale/generic/time_members.cc"
if [ -f "$TIME_SRC" ]; then
    TIME_PATCHED="$BUILD_DIR/locale/time_members_patched.cc"
    {
        echo 'extern "C" size_t _xpg5_wcsftime(wchar_t *, size_t, const wchar_t *, const struct tm *);'
        sed 's/wcsftime(__s, __maxlen, __format, __tm)/_xpg5_wcsftime(__s, __maxlen, __format, __tm)/' \
            "$TIME_SRC"
    } > "$TIME_PATCHED"
    if $CLANG $CXX98_FLAGS -c "$TIME_PATCHED" -o "$BUILD_DIR/locale/time_members.o" 2>"$BUILD_DIR/locale/time_members.err"; then
        compiledloc=$((compiledloc + 1))
    else
        echo "FAILED: time_members.cc (patched)"
        head -5 "$BUILD_DIR/locale/time_members.err"
        failedloc=$((failedloc + 1))
    fi
fi
echo "Locale: $compiledloc compiled, $failedloc failed"

echo ""
echo "=== Phase 5b: Compile old-ABI (cow) locale members ==="
# Dual ABI: the locale members above compile with _GLIBCXX_USE_CXX11_ABI=1 (default),
# producing std::__cxx11::numpunct etc. For backward compat we also need the old ABI
# symbols (std::numpunct etc.) — same source, compiled with _GLIBCXX_USE_CXX11_ABI=0.
COW_LOCALE_SOURCES=(
    collate_members.cc
    messages_members.cc
    monetary_members.cc
    numeric_members.cc
)
CXX98_COW_FLAGS="$CXX98_FLAGS -D_GLIBCXX_USE_CXX11_ABI=0 -fimplicit-templates"

compiledcow=0
failedcow=0
for src in "${COW_LOCALE_SOURCES[@]}"; do
    cowobj="$BUILD_DIR/locale/cow_${src%.cc}.o"
    srcpath="$LIBSTDCXX/config/locale/generic/$src"
    if [ ! -f "$srcpath" ]; then
        echo "MISSING: locale/generic/$src (cow)"
        failedcow=$((failedcow + 1))
        continue
    fi
    if $CLANG $CXX98_COW_FLAGS -c "$srcpath" -o "$cowobj" 2>"$BUILD_DIR/locale/cow_${src%.cc}.err"; then
        compiledcow=$((compiledcow + 1))
    else
        echo "FAILED: cow locale/generic/$src"
        head -5 "$BUILD_DIR/locale/cow_${src%.cc}.err"
        failedcow=$((failedcow + 1))
    fi
done
# time_members.cc: __timepunct is NOT in __cxx11 namespace, so the
# regular compilation already provides both ABIs. No cow version needed.
echo "Cow locale: $compiledcow compiled, $failedcow failed"

echo ""
echo "=== Summary ==="
total_compiled=$((compiled + compiled11 + compiled17 + compiledfs + compiledsupc + compiledsupc17 + compiledloc + compiledcow))
total_failed=$((failed + failed11 + failed17 + failedfs + failedsupc + failedsupc17 + failedloc + failedcow))
echo "Total: $total_compiled compiled, $total_failed failed"

if [ "$total_failed" -gt 0 ]; then
    echo ""
    echo "Some files failed. Review errors above."
    echo "Error logs in: $BUILD_DIR/"
    exit 1
fi

echo ""
echo "=== Phase 5c: Compile .eh_frame registration objects ==="

# libstdc++.so needs explicit .eh_frame registration so the DW2 unwinder
# can find FDEs for functions like __cxa_throw. Without this, exceptions
# always terminate() because the unwinder can't step through libstdc++ frames.
# GCC normally provides this via crtbeginS.o; we must do it manually.

FRAME_REG_C="$SCRIPT_DIR/libstdcxx_frame_reg.c"
FRAME_END_C="$SCRIPT_DIR/libstdcxx_frame_end.c"
CLANG_C="/opt/cross/bin/clang"
FRAME_C_FLAGS="--target=mips-sgi-irix6.5 --sysroot=$SYSROOT -mabi=n32 -march=mips3 -mxgot -O2 -fPIC"
FRAME_C_FLAGS="$FRAME_C_FLAGS -isystem $STAGING/include -isystem $SYSROOT/usr/include"

if $CLANG_C $FRAME_C_FLAGS -c "$FRAME_REG_C" -o "$BUILD_DIR/frame_reg.o" 2>"$BUILD_DIR/frame_reg.err"; then
    echo "frame_reg.o: OK"
else
    echo "FAILED: libstdcxx_frame_reg.c"
    cat "$BUILD_DIR/frame_reg.err"
    exit 1
fi

if $CLANG_C $FRAME_C_FLAGS -c "$FRAME_END_C" -o "$BUILD_DIR/frame_end.o" 2>"$BUILD_DIR/frame_end.err"; then
    echo "frame_end.o: OK"
else
    echo "FAILED: libstdcxx_frame_end.c"
    cat "$BUILD_DIR/frame_end.err"
    exit 1
fi

echo ""
echo "=== Phase 6: Link libstdc++.so.6 ==="

# Collect all .o files
ALL_OBJECTS=()
for obj in "$BUILD_DIR"/c++98/*.o "$BUILD_DIR"/c++11/*.o "$BUILD_DIR"/c++17/*.o "$BUILD_DIR"/filesystem/*.o "$BUILD_DIR"/supc++/*.o "$BUILD_DIR"/locale/*.o; do
    [ -f "$obj" ] && ALL_OBJECTS+=("$obj")
done

echo "Linking ${#ALL_OBJECTS[@]} object files + frame registration..."

# LLD 18 with IRIX patches handles .eh_frame placement natively:
# - Sets SHF_WRITE on .eh_frame (IRIX rld writes relocations at load time)
# - Suppresses GNU version sections (VERNEED/VERSYM crash rld)
# - Uses IRIX target defaults (2-segment layout, no RELRO, etc.)
# No BFD ld fallback or fix-eh-frame preprocessing needed.
#
# irix-ld automatically adds crtbeginS.o FIRST and crtendS.o LAST for
# shared libraries. crtbeginS.o provides _init → __do_global_ctors_aux
# which walks .ctors in reverse. crtendS.o provides __CTOR_END__ sentinel.
#
# frame_reg.o puts our frame registration function in .ctors so it runs
# during DSO load. frame_end.o is no longer needed (LLD adds .eh_frame
# terminator automatically).
#
# IRIX rld does NOT process .init_array, so __attribute__((constructor))
# doesn't work — we must use .ctors.

$IRIX_LD \
    -shared \
    -soname libstdc++.so.6 \
    -o "$OUTPUT" \
    "$BUILD_DIR/frame_reg.o" \
    "${ALL_OBJECTS[@]}" \
    -L"$SYSROOT/usr/lib32" \
    -L"$STAGING/lib32" \
    -lgcc_s \
    -lc -lm -lpthread \
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
    echo "=== Comparison with SGUG-RSE ==="
    sgug_count=$(/opt/cross/bin/mips-sgi-irix6.5-nm -D "$STAGING/lib32/libstdc++.so.6" 2>/dev/null | grep -c " T " || true)
    echo "SGUG-RSE exported: $sgug_count"
    echo "Our build exported: $exported"
else
    echo "LINK FAILED"
    cat "$BUILD_DIR/link.err"
    exit 1
fi
