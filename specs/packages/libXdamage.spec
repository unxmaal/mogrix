Name:           libXdamage
Version:        1.1.4
Release:        1%{?dist}
Summary:        X Damage extension library
License:        MIT
URL:            https://www.x.org
Source0:        libXdamage-%{version}.tar.gz

%description
X Damage extension library.

%package devel
Summary:        Development files for %{name}
Requires:       %{name} = %{version}-%{release}

%description devel
libXdamage development package.

%prep
%autosetup -n libXdamage-%{version}

%build
# Use shipped configure (autoreconf with host autoconf 2.71 breaks cross-compilation)
%configure --disable-static

make V=1 %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}
rm -f %{buildroot}%{_libdir}/*.la

%files
%{_libdir}/libXdamage.so.*

%files devel
%{_libdir}/libXdamage.so
%{_includedir}/X11/extensions/Xdamage.h
%{_libdir}/pkgconfig/xdamage.pc
