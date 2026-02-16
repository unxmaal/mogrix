Name:           libXinerama
Version:        1.1.4
Release:        1%{?dist}
Summary:        X Xinerama extension library
License:        MIT
URL:            https://www.x.org
Source0:        libXinerama-%{version}.tar.gz

%description
X Xinerama extension library.

%package devel
Summary:        Development files for %{name}
Requires:       %{name} = %{version}-%{release}

%description devel
libXinerama development package.

%prep
%autosetup -n libXinerama-%{version}

%build
# Use shipped configure (autoreconf with host autoconf 2.71 breaks cross-compilation)
%configure --disable-static

make V=1 %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}
rm -f %{buildroot}%{_libdir}/*.la

%files
%{_libdir}/libXinerama.so.*

%files devel
%{_libdir}/libXinerama.so
%{_includedir}/X11/extensions/Xinerama.h
%{_includedir}/X11/extensions/panoramiXext.h
%{_libdir}/pkgconfig/xinerama.pc
%{_mandir}/man3/*
