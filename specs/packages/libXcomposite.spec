Name:           libXcomposite
Version:        0.4.4
Release:        1%{?dist}
Summary:        X Composite extension library
License:        MIT
URL:            https://www.x.org
Source0:        libXcomposite-%{version}.tar.gz

%description
X Composite extension library.

%package devel
Summary:        Development files for %{name}
Requires:       %{name} = %{version}-%{release}

%description devel
libXcomposite development package.

%prep
%autosetup -n libXcomposite-%{version}

%build
# Use shipped configure (autoreconf with host autoconf 2.71 breaks cross-compilation)
%configure --disable-static

make V=1 %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}
rm -f %{buildroot}%{_libdir}/*.la

%files
%{_libdir}/libXcomposite.so.*

%files devel
%{_libdir}/libXcomposite.so
%{_includedir}/X11/extensions/Xcomposite.h
%{_libdir}/pkgconfig/xcomposite.pc
%{_mandir}/man3/*
