Name:           libXcursor
Version:        1.1.15
Release:        1%{?dist}
Summary:        X Cursor management library
License:        MIT
URL:            https://www.x.org
Source0:        libXcursor-%{version}.tar.gz

%description
X Cursor management library.

%package devel
Summary:        Development files for %{name}
Requires:       %{name} = %{version}-%{release}

%description devel
libXcursor development package.

%prep
%autosetup -n libXcursor-%{version}

%build
# Use shipped configure (autoreconf with host autoconf 2.71 breaks cross-compilation)
%configure --disable-static

make V=1 %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}
rm -f %{buildroot}%{_libdir}/*.la

%files
%{_libdir}/libXcursor.so.*

%files devel
%{_libdir}/libXcursor.so
%{_includedir}/X11/Xcursor/Xcursor.h
%{_libdir}/pkgconfig/xcursor.pc
%{_mandir}/man3/*
