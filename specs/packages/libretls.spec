Name:           libretls
Version:        3.8.1
Release:        1%{?dist}
Summary:        Port of libtls from LibreSSL to OpenSSL
License:        ISC
URL:            https://git.causal.agency/libretls
Source0:        libretls-3.8.1.tar.gz

BuildRequires:  openssl-devel
BuildRequires:  automake autoconf libtool

%description
LibreTLS is a port of libtls from LibreSSL to OpenSSL. It provides
the libtls API for simplified TLS programming.

%package devel
Summary:        Development files for libretls
Requires:       %{name} = %{version}-%{release}

%description devel
Headers and pkg-config file for libretls.

%prep
%autosetup -n libretls-3.8.1

%build
autoreconf -fi
%configure
%make_build

%install
%make_install

%files
%{_libdir}/libtls.so.*

%files devel
%{_includedir}/tls.h
%{_libdir}/libtls.so
%{_libdir}/libtls.a
%{_libdir}/pkgconfig/libtls.pc
%{_mandir}/man3/*.3*
