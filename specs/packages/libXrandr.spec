Name:           libXrandr
Version:        1.5.2
Release:        1%{?dist}
Summary:        X RandR extension library
License:        MIT
URL:            https://www.x.org
Source0:        libXrandr-%{version}.tar.gz

%description
X RandR extension library.

%package devel
Summary:        Development files for %{name}
Requires:       %{name} = %{version}-%{release}

%description devel
libXrandr development package.

%prep
%autosetup -n libXrandr-%{version}

%build
# Use shipped configure (autoreconf with host autoconf 2.71 breaks cross-compilation)
%configure --disable-static

make V=1 %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}
rm -f %{buildroot}%{_libdir}/*.la

%files
%{_libdir}/libXrandr.so.*

%files devel
%{_libdir}/libXrandr.so
%{_includedir}/X11/extensions/Xrandr.h
%{_libdir}/pkgconfig/xrandr.pc
%{_mandir}/man3/*
