Name:           cmake
Version:        3.28.2
Release:        1%{?dist}
Summary:        Cross-platform make system
License:        BSD-3-Clause AND MIT-open-group AND Zlib AND Apache-2.0
URL:            http://www.cmake.org
Source0:        cmake-3.28.2.tar.gz

# Bundled libraries (bootstrap mode - no system deps needed)
Provides:       bundled(cppdap)
Provides:       bundled(curl) = 8.4.0
Provides:       bundled(expat) = 2.5.0
Provides:       bundled(jsoncpp)
Provides:       bundled(libarchive) = 3.7.2
Provides:       bundled(libuv) = 1.44.2
Provides:       bundled(librhash)
Provides:       bundled(nghttp2) = 1.56.0
Provides:       bundled(zlib) = 1.3
Provides:       bundled(bzip2) = 1.0.8

%description
CMake is used to control the software compilation process using simple
platform and compiler independent configuration files. CMake generates
native makefiles and workspaces that can be used in the compiler
environment of your choice.

%package        data
Summary:        CMake data files (modules, templates, etc.)
BuildArch:      noarch

%description    data
CMake data files including modules, templates, and platform definitions.

%prep
%autosetup -n cmake-%{version}

# Create IRIX toolchain file for cross-compilation
cat > irix-toolchain.cmake << 'TOOLCHAIN_EOF'
# IRIX 6.5 cross-compilation toolchain
set(CMAKE_SYSTEM_NAME IRIX)
set(CMAKE_SYSTEM_PROCESSOR mips)
set(UNIX 1)

set(CMAKE_C_COMPILER   /opt/sgug-staging/usr/sgug/bin/irix-cc)
set(CMAKE_CXX_COMPILER /opt/sgug-staging/usr/sgug/bin/irix-cxx)
set(CMAKE_AR           /opt/cross/bin/llvm-ar)
set(CMAKE_RANLIB       /opt/cross/bin/llvm-ranlib)
set(CMAKE_LINKER       /opt/sgug-staging/usr/sgug/bin/irix-ld)

set(CMAKE_FIND_ROOT_PATH /opt/sgug-staging/usr/sgug)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Pthreads
set(CMAKE_HAVE_LIBC_PTHREAD OFF)
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(THREADS_PREFER_PTHREAD_FLAG OFF)
set(CMAKE_USE_PTHREADS_INIT 1)

# Cross-compile try_run results
set(HAVE_H_ERRNO_ASSIGNABLE_EXITCODE 0 CACHE STRING "")
TOOLCHAIN_EOF

# Create Platform/IRIX.cmake in source tree so host cmake can find it
cat > Modules/Platform/IRIX.cmake << 'PLATFORM_EOF'
# IRIX 6.5 platform definition
set(UNIX 1)
set(CMAKE_DL_LIBS "")
set(CMAKE_SHARED_LIBRARY_PREFIX "lib")
set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
set(CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS "-shared")
set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG "-Wl,-rpath,")
set(CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP ":")
set(CMAKE_SHARED_LIBRARY_SONAME_C_FLAG "-Wl,-soname,")
set(CMAKE_EXE_EXPORTS_C_FLAG "")
PLATFORM_EOF

# Patch libuv for IRIX compatibility
_libuv=Utilities/cmlibuv/src/unix/fs.c

# 1. Fix UV_CONST_DIRENT: IRIX scandir uses non-const dirent pointers
perl -pi -e 's/^#if defined\(__APPLE__\) && !defined\(MAC_OS_X_VERSION_10_8\)/#if (defined(__APPLE__) \&\& !defined(MAC_OS_X_VERSION_10_8)) || defined(__sgi)/' $_libuv

# 2. Add __sgi to platforms using statvfs.h (include block)
perl -0pi -e 's/(defined\(__QNX__\)\n# include <sys\/statvfs\.h>)/defined(__QNX__)    || \\\n      defined(__sgi)\n# include <sys\/statvfs.h>/' $_libuv

# 3. Add __sgi to platforms using statvfs() in uv__fs_statfs
perl -0pi -e 's/(defined\(__QNX__\)\n  struct statvfs buf;)/defined(__QNX__)    || \\\n    defined(__sgi)\n  struct statvfs buf;/' $_libuv

# 4. Add __sgi to platforms where f_type is not supported
perl -0pi -e 's/(defined\(__QNX__\)\n  stat_fs->f_type = 0;)/defined(__QNX__)      || \\\n    defined(__sgi)\n  stat_fs->f_type = 0;/' $_libuv

# 5. Fix pthread_condattr_setclock: IRIX doesn't have it or CLOCK_MONOTONIC
perl -pi -e 's/^#if !defined\(__hpux\)$/#if !defined(__hpux) \&\& !defined(__sgi)/' \
  Utilities/cmlibuv/src/unix/thread.c

# 6. Fix cfmakeraw: IRIX doesn't have it, use manual implementation
perl -pi -e 's/^#if defined __sun \|\| defined __MVS__ \|\| defined __hpux$/#if defined __sun || defined __MVS__ || defined __hpux || defined __sgi/' \
  Utilities/cmlibuv/src/unix/tty.c

# 7. Fix SSM (Source-Specific Multicast): IRIX doesn't have ip_mreq_source etc.
perl -pi -e 's/!defined\(__GNU__\)/!defined(__GNU__) \&\&                                        \\\n    !defined(__sgi)/' \
  Utilities/cmlibuv/src/unix/udp.c

# 8. Don't use -D_XOPEN_SOURCE=600 on IRIX - it hides BSD types (u_short,
#    u_char) that IRIX netinet/tcp.h uses unconditionally, and also triggers
#    UB in standards.h macro evaluation with clang. IRIX's default SGI mode
#    already provides all the POSIX features these libraries need.
#    Match lines with the exact "BSD|Darwin|Windows") pattern, add IRIX.
for f in Utilities/cmbzip2/CMakeLists.txt Utilities/cmcurl/CMakeLists.txt Utilities/cmlibarchive/CMakeLists.txt Utilities/cmliblzma/CMakeLists.txt Utilities/cmlibuv/CMakeLists.txt; do
  perl -pi -e 's/MATCHES "BSD\|Darwin\|Windows"\)/MATCHES "BSD|Darwin|Windows|IRIX")/' "$f"
done

# 9. Don't define _POSIX_C_SOURCE on IRIX - it makes _NO_POSIX FALSE, which
#    disables _SGIAPI, hiding C99 functions (lldiv_t, snprintf, etc.) that
#    GCC 9's libstdc++ C++ wrappers need via using-declarations.
perl -pi -e 's/!defined\(__OpenBSD__\)/!defined(__OpenBSD__) \&\& !defined(__sgi)/' \
  Source/cmStandardLexer.h

# 10. Undef IRIX stat.h macros that break libuv's uv_stat_t struct definition.
#     IRIX defines st_blocks as a macro expanding to union member access.
perl -pi -e 'if (/^typedef struct \{/ .. /^\} uv_stat_t;/) {
  if (/uint64_t st_blocks/) {
    print "#ifdef st_blocks\n#undef st_blocks\n#endif\n";
  }
}' Utilities/cmlibuv/include/uv.h

# 11. Add IRIX platform support to libuv CMakeLists.txt
#     IRIX uses poll(), has no fsevents, no proctitle, and needs CLOCK_SGI_CYCLE
#     for high-res timer instead of CLOCK_MONOTONIC.
perl -pi -e 'if (/^if\(CMAKE_SYSTEM_NAME STREQUAL "QNX"\)/) {
  print qq{if(CMAKE_SYSTEM_NAME STREQUAL "IRIX")\n};
  print qq{  list(APPEND uv_headers\n};
  print qq{    include/uv/posix.h\n};
  print qq{    )\n};
  print qq{  list(APPEND uv_sources\n};
  print qq{    src/unix/posix-hrtime.c\n};
  print qq{    src/unix/posix-poll.c\n};
  print qq{    src/unix/no-fsevents.c\n};
  print qq{    src/unix/no-proctitle.c\n};
  print qq{    )\n};
  print qq{endif()\n\n};
}' Utilities/cmlibuv/CMakeLists.txt

# 12. Add __sgi to libuv platform detection in uv/unix.h to include posix.h
#     This provides UV_LOOP_PRIVATE_FIELDS (poll_fds, etc.)
perl -pi -e 's/^#elif defined\(__hpux\)/#elif defined(__sgi)\n# include "posix.h"\n#elif defined(__hpux)/' \
  Utilities/cmlibuv/include/uv/unix.h

# 13. Patch posix-hrtime.c: IRIX has CLOCK_SGI_CYCLE, not CLOCK_MONOTONIC
perl -pi -e 's/^#elif defined\(__hpux\)/#elif defined(__sgi)\n\/* IRIX: no CLOCK_MONOTONIC, use CLOCK_SGI_CYCLE (hardware counter) *\/\n\n#include <stdint.h>\n#include <time.h>\n#include <sys\/ptimers.h>\n\n#undef NANOSEC\n#define NANOSEC ((uint64_t) 1e9)\n\nuint64_t uv__hrtime(uv_clocktype_t type) {\n  struct timespec ts;\n  clock_gettime(CLOCK_SGI_CYCLE, \&ts);\n  return (((uint64_t) ts.tv_sec) * NANOSEC + ts.tv_nsec);\n}\n\n#elif defined(__hpux)/' \
  Utilities/cmlibuv/src/unix/posix-hrtime.c

# 14. Add fallback CMAKE_ROOT resolution for bundle layout.
#     cmake resolves data dir relative to binary: strip "bin" suffix from exe_dir,
#     append "share/cmake-3.28". In mogrix bundles, binaries are in _bin/ not bin/,
#     so the standard suffix-strip produces "_" prefix → "_share/cmake-3.28" (wrong).
#     Add fallback: if standard resolution fails, try exe_dir/../share/cmake-3.28.
perl -pi -e 'if (/Build tree has.*bin.*config.*cmake/) {
  # Insert a fallback check before the build-tree check
  print "    // Mogrix bundle fallback: try <exe_dir>/../<CMAKE_DATA_DIR>\n";
  print "    std::string fallback_dir = cmSystemTools::GetFilenamePath(exe_dir);\n";
  print "    std::string fallback_root = cmStrCat(fallback_dir, \"/\", CMAKE_DATA_DIR);\n";
  print "    if (cmSystemTools::FileExists(cmStrCat(fallback_root, \"/Modules/CMake.cmake\"))) {\n";
  print "      cmSystemToolsCMakeRoot = fallback_root;\n";
  print "    }\n";
  print "  }\n";
  print "  if (cmSystemToolsCMakeRoot.empty() ||\n";
  print "      !cmSystemTools::FileExists(\n";
  print "        cmStrCat(cmSystemToolsCMakeRoot, \"/Modules/CMake.cmake\"))) {\n";
}' Source/cmSystemTools.cxx

%build
# Use host cmake to cross-configure for IRIX
# Bundle ALL third-party deps (no system libs) to avoid dependency issues
# Point CMAKE_MODULE_PATH to source Modules/ so host cmake finds Platform/IRIX.cmake
/usr/bin/cmake -B _build -S . \
  -DCMAKE_TOOLCHAIN_FILE=$(pwd)/irix-toolchain.cmake \
  -DCMAKE_MODULE_PATH=$(pwd)/Modules \
  -DCMAKE_INSTALL_PREFIX=%{_prefix} \
  -DCMAKE_INSTALL_LIBDIR=%{_libdir} \
  -DCMAKE_C_FLAGS="%{optflags}" \
  -DCMAKE_CXX_FLAGS="%{optflags}" \
  -DCMAKE_EXE_LINKER_FLAGS="-L/opt/sgug-staging/usr/sgug/lib32 -lpthread -lm -lmogrix_compat" \
  -DCMAKE_USE_SYSTEM_LIBRARIES=OFF \
  -DBUILD_CursesDialog=OFF \
  -DBUILD_QtDialog=OFF \
  -DBUILD_TESTING=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DENABLE_IPV6=OFF

cd _build && make %{?_smp_mflags} VERBOSE=1

%install
cd _build && make install DESTDIR=%{buildroot}

# Remove docs and extras we don't need on IRIX
rm -rf %{buildroot}%{_datadir}/doc
rm -rf %{buildroot}%{_prefix}/doc
rm -rf %{buildroot}%{_datadir}/bash-completion
rm -rf %{buildroot}%{_datadir}/emacs
rm -rf %{buildroot}%{_datadir}/vim

# Create version-suffixed symlinks
for f in cmake cpack ctest; do
  ln -sf $f %{buildroot}%{_bindir}/${f}3
done

%files
%license Copyright.txt
%{_bindir}/cmake
%{_bindir}/cmake3
%{_bindir}/cpack
%{_bindir}/cpack3
%{_bindir}/ctest
%{_bindir}/ctest3

%files data
%{_datadir}/cmake-3.28/
%{_datadir}/aclocal/cmake.m4
