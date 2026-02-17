Name:           libXi
Version:        1.7.10
Release:        1%{?dist}
Summary:        X Input extension library
License:        MIT
URL:            https://www.x.org
Source0:        libXi-%{version}.tar.gz

%description
X Input extension library.

%package devel
Summary:        Development files for %{name}
Requires:       %{name} = %{version}-%{release}

%description devel
libXi development package.

%prep
%autosetup -n libXi-%{version}

%build
# Use shipped configure (autoreconf with host autoconf 2.71 breaks cross-compilation)
%configure --disable-static

make V=1 %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}
rm -f %{buildroot}%{_libdir}/*.la

%files
%{_libdir}/libXi.so.*

%files devel
%{_libdir}/libXi.so
%{_includedir}/X11/extensions/XInput.h
%{_includedir}/X11/extensions/XInput2.h
%{_libdir}/pkgconfig/xi.pc
%{_mandir}/man3/*
