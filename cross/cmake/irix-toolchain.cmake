# CMake Toolchain File for IRIX N32 Cross-Compilation
#
# This is a template toolchain file for cross-compiling to IRIX 6.5 N32 ABI.
# Copy and customize for your specific package needs.
#
# Key discoveries that make this work:
# - Shared libraries: Use GNU ld (produces correct 2-LOAD segment layout)
# - Executables: Use LLD with --dynamic-linker=/lib32/rld
# - The interpreter MUST be /lib32/rld, not /usr/lib32/libc.so.1
#
# Usage:
#   cmake -DCMAKE_TOOLCHAIN_FILE=/path/to/irix-toolchain.cmake ..
#

set(CMAKE_SYSTEM_NAME IRIX)
set(CMAKE_SYSTEM_VERSION 6.5)
set(CMAKE_SYSTEM_PROCESSOR mips)

# Cross-compiler settings
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# Skip compiler tests since we're cross-compiling for IRIX
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# =============================================================================
# Path Configuration
# =============================================================================

# IRIX sysroot - pristine IRIX system headers and libraries
set(IRIX_SYSROOT /opt/irix-sysroot)

# Staging directory - cross-compiled packages are installed here
set(STAGING_DIR /opt/sgug-staging/usr/sgug)

# GNU ld for shared libraries (critical for IRIX compatibility)
# LLD produces 3-LOAD-segment layout that crashes IRIX rld
# GNU ld produces correct 2-LOAD-segment layout
set(GNU_LD /opt/cross/bin/mips-sgi-irix6.5-ld.bfd)

# LLD for executables (faster, works fine for non-shared)
set(LLD /opt/cross/bin/ld.lld-irix)

# =============================================================================
# Compiler Flags
# =============================================================================

# C flags for IRIX N32 ABI
# -target mips-sgi-irix6.5: Target IRIX
# --sysroot: Use IRIX headers/libs
# -mabi=n32 -mips4: N32 ABI with MIPS IV instructions
# -fPIC: Position independent code (required for shared libs)
# -D__unix__ etc: IRIX platform defines
# -include irix-compat.h: Include compatibility header for all sources
# -fno-stack-protector: IRIX doesn't support stack canaries
# -D_XOPEN_SOURCE=500: Enable POSIX/XSI extensions
set(CMAKE_C_FLAGS_INIT "-target mips-sgi-irix6.5 --sysroot=${IRIX_SYSROOT} -mabi=n32 -mips4 -fPIC -D__unix__ -D__sgi__ -D__SGI_IRIX__ -D__irix__ -D_SGI_MP_SOURCE -D_LANGUAGE_C -DIRIX -I${STAGING_DIR}/include -include irix-compat.h -fno-stack-protector -D_XOPEN_SOURCE=500")
set(CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT}")

# =============================================================================
# Linker Configuration
# =============================================================================

# Shared library creation using GNU ld
# -melf32bmipn32: N32 ABI ELF format
# -shared: Create shared library
# -lpthread -lc: Link against pthread and libc
set(CMAKE_C_CREATE_SHARED_LIBRARY "${GNU_LD} -melf32bmipn32 -shared <CMAKE_SHARED_LIBRARY_SONAME_C_FLAG><TARGET_SONAME> -o <TARGET> <OBJECTS> <LINK_LIBRARIES> -L${STAGING_DIR}/lib32 -L${STAGING_DIR}/lib -L${IRIX_SYSROOT}/usr/lib32 -L${IRIX_SYSROOT}/lib32 -lpthread -lc")
set(CMAKE_CXX_CREATE_SHARED_LIBRARY "${CMAKE_C_CREATE_SHARED_LIBRARY}")

# Static library creation
set(CMAKE_C_CREATE_STATIC_LIBRARY "<CMAKE_AR> rcs <TARGET> <OBJECTS>")
set(CMAKE_CXX_CREATE_STATIC_LIBRARY "${CMAKE_C_CREATE_STATIC_LIBRARY}")

# Executable linking using LLD
# CRITICAL: --dynamic-linker=/lib32/rld is required!
# Without this, executables segfault because LLD defaults to /usr/lib32/libc.so.1
# --allow-shlib-undefined: Don't error on undefined symbols in shared libs
# -rpath: Runtime library search path on target system
# -z notext: Allow text relocations (needed for some IRIX code)
# crt1.o/crtn.o: C runtime startup/cleanup files
set(CMAKE_C_LINK_EXECUTABLE "${LLD} --sysroot=${IRIX_SYSROOT} --allow-shlib-undefined --dynamic-linker=/lib32/rld -L${STAGING_DIR}/lib32 -L${STAGING_DIR}/lib -L${IRIX_SYSROOT}/usr/lib32 -L${IRIX_SYSROOT}/lib32 -rpath /usr/sgug/lib32:/usr/lib32:/lib32 -z notext ${IRIX_SYSROOT}/usr/lib32/mips3/fixed/crt1.o <OBJECTS> -o <TARGET> <LINK_LIBRARIES> -lpthread -lm -lc ${IRIX_SYSROOT}/usr/lib32/mips3/fixed/crtn.o")
set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_C_LINK_EXECUTABLE}")

# SONAME flag for shared libraries
set(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "--soname=")
set(CMAKE_SHARED_LIBRARY_SONAME_CXX_FLAG "--soname=")

# Disable --as-needed since we're using GNU ld directly
set(CMAKE_LINK_WHAT_YOU_USE OFF)
set(CMAKE_SHARED_LINKER_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "")

# =============================================================================
# Find* Configuration
# =============================================================================

# Include and library paths for find_* commands
set(CMAKE_FIND_ROOT_PATH ${STAGING_DIR} ${IRIX_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Pkg-config paths
set(ENV{PKG_CONFIG_PATH} "${STAGING_DIR}/lib32/pkgconfig:${STAGING_DIR}/lib/pkgconfig")
set(ENV{PKG_CONFIG_LIBDIR} "${STAGING_DIR}/lib32/pkgconfig:${STAGING_DIR}/lib/pkgconfig")
set(ENV{PKG_CONFIG_SYSROOT_DIR} "")

# Set paths for find_* commands
set(CMAKE_PREFIX_PATH ${STAGING_DIR})

# Cross-compiling flag
set(CMAKE_CROSSCOMPILING TRUE)

# =============================================================================
# Package-Specific Library Paths (customize as needed)
# =============================================================================

# Common library locations - uncomment and modify as needed for your package:

# zlib
#set(ZLIB_LIBRARY "${STAGING_DIR}/lib32/libz.so" CACHE FILEPATH "zlib")
#set(ZLIB_INCLUDE_DIR "${STAGING_DIR}/include" CACHE PATH "zlib include")

# OpenSSL
#set(OPENSSL_ROOT_DIR ${STAGING_DIR})
#set(OPENSSL_INCLUDE_DIR "${STAGING_DIR}/include" CACHE PATH "OpenSSL include")
#set(OPENSSL_CRYPTO_LIBRARY "${STAGING_DIR}/lib32/libcrypto.so" CACHE FILEPATH "OpenSSL crypto")
#set(OPENSSL_SSL_LIBRARY "${STAGING_DIR}/lib32/libssl.so" CACHE FILEPATH "OpenSSL SSL")

# CURL
#set(CURL_INCLUDE_DIR "${STAGING_DIR}/include" CACHE PATH "CURL include")
#set(CURL_LIBRARY "${STAGING_DIR}/lib32/libcurl.so" CACHE FILEPATH "CURL library")

# libxml2
#set(LIBXML2_LIBRARY "${STAGING_DIR}/lib32/libxml2.so" CACHE FILEPATH "libxml2")
#set(LIBXML2_INCLUDE_DIR "${STAGING_DIR}/include/libxml2" CACHE PATH "libxml2 include")

# libsolv
#set(LibSolv_INCLUDE_DIRS "${STAGING_DIR}/include")
#set(LIBSOLV_LIBRARY "${STAGING_DIR}/lib/libsolv.so" CACHE FILEPATH "libsolv")
#set(LIBSOLV_EXT_LIBRARY "${STAGING_DIR}/lib/libsolvext.so" CACHE FILEPATH "libsolvext")

# RPM
#set(LIBRPM_LIBRARY "${STAGING_DIR}/lib32/librpm.so" CACHE FILEPATH "librpm")
#set(LIBRPMIO_LIBRARY "${STAGING_DIR}/lib32/librpmio.so" CACHE FILEPATH "librpmio")
