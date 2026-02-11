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
set(CMAKE_FIND_ROOT_PATH /opt/sgug-staging/usr/sgug)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_INSTALL_PREFIX /usr/sgug)
set(CMAKE_INSTALL_LIBDIR lib32)

set(CMAKE_C_FLAGS_INIT "-I/opt/sgug-staging/usr/sgug/include/mogrix-compat/generic -Wno-macro-redefined -Dalloca=__builtin_alloca")
set(CMAKE_CXX_FLAGS_INIT "-I/opt/sgug-staging/usr/sgug/include/mogrix-compat/generic -Wno-macro-redefined -Dalloca=__builtin_alloca")
