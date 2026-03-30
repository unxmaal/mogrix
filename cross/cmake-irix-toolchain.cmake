# CMake toolchain file for IRIX N32 cross-compilation
# Used by mogrix to build cmake-based packages

# Use Linux as system name — cmake doesn't know IRIX, and Generic doesn't
# support shared libraries. Our irix-cc/irix-cxx wrappers handle the actual
# IRIX ELF details, so Linux's shared lib model (gcc -shared) works fine.
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR mips)

set(CMAKE_C_COMPILER $ENV{MOGRIX_STAGING}/bin/irix-cc)
set(CMAKE_CXX_COMPILER $ENV{MOGRIX_STAGING}/bin/irix-cxx)
set(CMAKE_AR /opt/cross/bin/llvm-ar)
set(CMAKE_RANLIB /opt/cross/bin/llvm-ranlib)
set(CMAKE_STRIP /opt/cross/bin/llvm-strip)

set(CMAKE_SYSROOT $ENV{MOGRIX_STAGING_ROOT})
# Both paths needed: staging root re-roots pkg-config hints correctly
# (e.g. /opt/mogrix/lib32 → <staging_root>/opt/mogrix/lib32), while
# <staging>/opt/mogrix provides the default lib32/ search.
# MOGRIX_STAGING: mogrix-built libraries (primary)
# MOGRIX_STAGING_ROOT: sysroot for pkg-config re-rooting
# /opt/irix-sysroot: IRIX native system libraries (libXt, libX11, etc.)
set(CMAKE_FIND_ROOT_PATH $ENV{MOGRIX_STAGING_ROOT} $ENV{MOGRIX_STAGING} /opt/irix-sysroot)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_INSTALL_PREFIX /opt/mogrix)
set(CMAKE_INSTALL_LIBDIR lib32)

# Tell cmake to search lib32 (IRIX n32 ABI) in addition to default lib/
# Also /usr/lib32 for IRIX native libraries (libXt, libX11, etc. from irix-sysroot)
set(CMAKE_SYSTEM_LIBRARY_PATH /opt/mogrix/lib32 /usr/lib32)
# CMAKE_LIBRARY_PATH helps find_library search lib32 under sysroot
list(APPEND CMAKE_LIBRARY_PATH $ENV{MOGRIX_STAGING}/lib32 /opt/irix-sysroot/usr/lib32)
# Also search IRIX native includes
set(CMAKE_SYSTEM_PREFIX_PATH $ENV{MOGRIX_STAGING} /opt/irix-sysroot/usr)

# Tell pkg-config where to find .pc files for cross-compilation
# PKG_CONFIG_SYSROOT_DIR is needed so cmake IMPORTED_TARGET include dirs
# exist on disk (e.g. <staging>/include/gtk-3.0).
# This can cause double-prefixing in find_library with ONLY mode — for
# affected packages, pre-set cache variables (e.g. -DHarfBuzz_LIBRARY=...).
set(ENV{PKG_CONFIG_PATH} "")
set(ENV{PKG_CONFIG_LIBDIR} "$ENV{MOGRIX_STAGING}/lib32/pkgconfig:$ENV{MOGRIX_STAGING}/share/pkgconfig")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "$ENV{MOGRIX_STAGING_ROOT}")

# -mxgot: Use 32-bit GOT offsets (lui/addiu pairs) instead of 16-bit.
# Without this, LLD creates "secondary GOTs" for large libraries (>16K entries).
# IRIX rld does NOT displace secondary GOT entries, causing SIGSEGV at runtime.
# -mxgot eliminates secondary GOTs so all entries are in the primary GOT that rld handles.
set(CMAKE_C_FLAGS_INIT "-I$ENV{MOGRIX_STAGING}/include/mogrix-compat/generic -Wno-macro-redefined -Dalloca=__builtin_alloca -mxgot")
set(CMAKE_CXX_FLAGS_INIT "-I$ENV{MOGRIX_STAGING}/include/mogrix-compat/generic -Wno-macro-redefined -Dalloca=__builtin_alloca -mxgot")
