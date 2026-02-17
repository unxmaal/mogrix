Name:           libXfixes
Version:        5.0.3
Release:        1%{?dist}
Summary:        X Fixes library
License:        MIT
URL:            https://www.x.org
Source0:        libXfixes-%{version}.tar.gz

%description
X Fixes library.

%package devel
Summary:        Development files for %{name}
Requires:       %{name} = %{version}-%{release}

%description devel
libXfixes development package.

%prep
%autosetup -n libXfixes-%{version}

%build
# Use shipped configure (autoreconf with host autoconf 2.71 breaks cross-compilation)
%configure --disable-static

make V=1 %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}
rm -f %{buildroot}%{_libdir}/*.la

%files
%{_libdir}/libXfixes.so.*

%files devel
%{_libdir}/libXfixes.so
%{_includedir}/X11/extensions/Xfixes.h
%{_libdir}/pkgconfig/xfixes.pc
%{_mandir}/man3/Xfixes.3*
