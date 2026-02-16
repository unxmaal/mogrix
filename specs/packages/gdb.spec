Name:           gdb
Epoch:          2
Version:        14.2
Release:        1%{?dist}
Summary:        GNU Debugger for IRIX
License:        GPL-3.0-or-later
URL:            https://www.gnu.org/software/gdb/
Source0:        gdb-%{version}.tar.xz

BuildRequires:  expat-devel

%description
GDB, the GNU Debugger, allows you to debug programs written in C, C++,
and other languages. Cross-compiled for SGI IRIX 6.5.

%prep
%autosetup -n gdb-%{version} -p1

%build
# Configure GDB for IRIX cross-compilation
# Disable everything optional — just core debugging functionality

# Set cross-tool variables so configure finds the right tools
export AR=/opt/cross/bin/llvm-ar
export RANLIB=/opt/cross/bin/llvm-ranlib
export NM=/opt/cross/bin/llvm-nm
export STRIP=/opt/cross/bin/llvm-strip
export OBJCOPY=/opt/cross/bin/llvm-objcopy
export OBJDUMP=/opt/cross/bin/llvm-objdump
export READELF=/opt/cross/bin/llvm-readelf

# Link against compat library
# Include strnlen.o directly (not just from archive) to beat link order issues:
# GDB's CLIBS puts @LIBS@ before $(LIBGNU), so archive members needed by
# gnulib code won't be pulled in. Direct .o files are always linked.
COMPAT_DIR=$(pwd)/mogrix-compat
export LIBS="${COMPAT_DIR}/strnlen.o -L${COMPAT_DIR} -lmogrix-compat"

# Force configure detection of headers/functions that fail during cross-compilation
# IRIX has these but libiberty's cross-compile configure misdetects them
export ac_cv_header_fcntl_h=yes
export ac_cv_func_gettimeofday=yes
export ac_cv_func_strstr=yes
export ac_cv_func_strncmp=yes
export ac_cv_func_strerror=yes
export ac_cv_func_memmove=yes
export ac_cv_func_memset=yes
export ac_cv_func_memcmp=yes
export ac_cv_func_memchr=yes
export ac_cv_func_strchr=yes
export ac_cv_func_strrchr=yes
export ac_cv_func_strtol=yes
export ac_cv_func_strtoul=yes
export ac_cv_func_strtod=yes
export ac_cv_func_tmpnam=yes
export ac_cv_func_waitpid=yes
export ac_cv_func_vfork_works=yes
export ac_cv_func_getrlimit=yes
export ac_cv_func_sbrk=yes
export ac_cv_func_on_exit=no
export ac_cv_func_psignal=no

# Tell gnulib NOT to replace functions that work fine on IRIX
# Prevents gnulib wrapper headers from conflicting with IRIX system headers
export gl_cv_func_gettimeofday_posix_signature=yes
export gl_cv_func_localtime_r_inline=no
export gl_cv_func_malloc_posix=yes
export gl_cv_func_select_supports0=yes
export gl_cv_header_sys_select_h_selfcontained=yes
export gl_cv_func_free_preserves_errno=yes

# Prevent gnulib from creating rpl_malloc/rpl_realloc wrappers.
# gnulib's malloc.m4 hardcodes gl_cv_func_malloc_posix=no for irix*,
# and the ptrdiff_t safety check fails for 32-bit MIPS N32.
# rpl_malloc calls libc malloc through a GOT entry that IRIX rld
# fails to resolve (R_MIPS_REL32 in local GOT slot), causing SIGSEGV.
export gl_cv_malloc_ptrdiff=yes

# Disable dlmalloc — debugging C++ static init crash
export MOGRIX_NO_DLMALLOC=1

mkdir -p _build && cd _build

CC="%{__cc}" \
CXX="%{__cxx}" \
CFLAGS="%{optflags}" \
CXXFLAGS="%{optflags} -Drestrict=__restrict__" \
LDFLAGS="-L/opt/sgug-staging/usr/sgug/lib32 -L/opt/irix-sysroot/usr/lib32 -L${COMPAT_DIR} -lmogrix-compat" \
../configure \
  --host=mips-sgi-irix6.5 \
  --prefix=%{_prefix} \
  --libdir=%{_libdir} \
  --sysconfdir=%{_sysconfdir} \
  --mandir=%{_mandir} \
  --infodir=%{_infodir} \
  --enable-build-with-cxx \
  --enable-64-bit-bfd \
  --disable-sim \
  --disable-rpath \
  --disable-libmcheck \
  --disable-tui \
  --disable-werror \
  --disable-source-highlight \
  --disable-unit-tests \
  --disable-inprocess-agent \
  --enable-libsframe \
  --without-python \
  --without-guile \
  --without-babeltrace \
  --without-debuginfod \
  --without-intel-pt \
  --without-xxhash \
  --without-rpm \
  --without-libunwind \
  --without-auto-load-safe-path \
  --with-expat \
  --with-system-zlib \
  --with-system-readline \
  --with-lzma

# LOADLIBES ensures compat archive is scanned LAST — after gnulib's LIBGNU
# (gnulib detects strnlen as "available" via LIBS during configure, generates
# code calling it, but LIBGNU comes after @LIBS@ in CLIBS link order)
make V=1 %{?_smp_mflags} LOADLIBES="-L${COMPAT_DIR} -lmogrix-compat"

%install
cd _build
make install DESTDIR=%{buildroot}

# Remove files we don't need on IRIX
rm -rf %{buildroot}%{_infodir}
rm -rf %{buildroot}%{_datadir}/gdb/python
rm -rf %{buildroot}%{_datadir}/gdb/syscalls
rm -rf %{buildroot}%{_includedir}
rm -rf %{buildroot}%{_libdir}/*.a
rm -rf %{buildroot}%{_libdir}/*.la
# Remove binutils components (we don't need them)
rm -f %{buildroot}%{_bindir}/addr2line
rm -f %{buildroot}%{_bindir}/ar
rm -f %{buildroot}%{_bindir}/as
rm -f %{buildroot}%{_bindir}/c++filt
rm -f %{buildroot}%{_bindir}/elfedit
rm -f %{buildroot}%{_bindir}/gprof
rm -f %{buildroot}%{_bindir}/ld*
rm -f %{buildroot}%{_bindir}/nm
rm -f %{buildroot}%{_bindir}/objcopy
rm -f %{buildroot}%{_bindir}/objdump
rm -f %{buildroot}%{_bindir}/ranlib
rm -f %{buildroot}%{_bindir}/readelf
rm -f %{buildroot}%{_bindir}/size
rm -f %{buildroot}%{_bindir}/strings
rm -f %{buildroot}%{_bindir}/strip
rm -rf %{buildroot}%{_mandir}/man1/[a-f]* %{buildroot}%{_mandir}/man1/[h-z]*
rm -rf %{buildroot}%{_datadir}/locale

%files
%license COPYING COPYING.LIB COPYING3 COPYING3.LIB
%{_bindir}/gdb
%{_bindir}/gdb-add-index
%{_mandir}/man1/gdb*
%{_mandir}/man5/gdbinit.5*
%{_datadir}/gdb/
