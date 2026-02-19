# CMake toolchain file for IRIX N32 cross-compilation
# Used by mogrix to build cmake-based packages

# Use Linux as system name — cmake doesn't know IRIX, and Generic doesn't
# support shared libraries. Our irix-cc/irix-cxx wrappers handle the actual
# IRIX ELF details, so Linux's shared lib model (gcc -shared) works fine.
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR mips)

set(CMAKE_C_COMPILER /opt/sgug-staging/usr/sgug/bin/irix-cc)
set(CMAKE_CXX_COMPILER /opt/sgug-staging/usr/sgug/bin/irix-cxx)
set(CMAKE_AR /opt/cross/bin/llvm-ar)
set(CMAKE_RANLIB /opt/cross/bin/llvm-ranlib)
set(CMAKE_STRIP /opt/cross/bin/llvm-strip)

set(CMAKE_SYSROOT /opt/sgug-staging)
# Both paths needed: /opt/sgug-staging re-roots pkg-config hints correctly
# (e.g. /usr/sgug/lib32 → /opt/sgug-staging/usr/sgug/lib32), while
# /opt/sgug-staging/usr/sgug provides the default lib32/ search.
set(CMAKE_FIND_ROOT_PATH /opt/sgug-staging /opt/sgug-staging/usr/sgug)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_INSTALL_PREFIX /usr/sgug)
set(CMAKE_INSTALL_LIBDIR lib32)

# Tell cmake to search lib32 (IRIX n32 ABI) in addition to default lib/
set(CMAKE_SYSTEM_LIBRARY_PATH /usr/sgug/lib32)
# CMAKE_LIBRARY_PATH helps find_library search lib32 under sysroot
list(APPEND CMAKE_LIBRARY_PATH /opt/sgug-staging/usr/sgug/lib32)

# Tell pkg-config where to find .pc files for cross-compilation
# PKG_CONFIG_SYSROOT_DIR prefixes -I/-L paths so cmake IMPORTED_TARGET
# include dirs exist on disk. find_library still works via CMAKE_SYSTEM_LIBRARY_PATH.
set(ENV{PKG_CONFIG_PATH} "")
set(ENV{PKG_CONFIG_LIBDIR} "/opt/sgug-staging/usr/sgug/lib32/pkgconfig:/opt/sgug-staging/usr/sgug/share/pkgconfig")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "/opt/sgug-staging")

set(CMAKE_C_FLAGS_INIT "-I/opt/sgug-staging/usr/sgug/include/mogrix-compat/generic -Wno-macro-redefined -Dalloca=__builtin_alloca")
set(CMAKE_CXX_FLAGS_INIT "-I/opt/sgug-staging/usr/sgug/include/mogrix-compat/generic -Wno-macro-redefined -Dalloca=__builtin_alloca")
