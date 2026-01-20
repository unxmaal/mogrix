Name:           gnutls
Version:        3.8.0
Release:        1%{?dist}
Summary:        GNU TLS library

License:        LGPL-2.1-or-later
URL:            https://www.gnutls.org/
Source0:        https://www.gnupg.org/ftp/gcrypt/gnutls/v3.8/gnutls-%{version}.tar.xz

BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  pkgconfig
BuildRequires:  nettle-devel
BuildRequires:  libtasn1-devel
BuildRequires:  p11-kit-devel
BuildRequires:  libdicl-devel
BuildRequires:  trousers-devel
BuildRequires:  systemtap-sdt-devel
Requires:       libdicl

%description
GnuTLS is a secure communications library implementing TLS, SSL protocols.

%package devel
Summary:        Development files for GnuTLS
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description devel
Development files for GnuTLS.

%package debuginfo
Summary:        Debug information for GnuTLS
AutoReqProv:    no

%description debuginfo
Debug information for GnuTLS.

%prep
%setup -q

%build
export CPPFLAGS="-I%{_includedir}/libdicl-0.1"
export LIBS="-ldicl-0.1"
%configure --enable-ktls --with-default-trust-store-pkcs11

make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}

%files
%{_libdir}/lib*.so.*

%files devel
%{_includedir}/*
%{_libdir}/lib*.so
%{_libdir}/pkgconfig/*.pc

%files debuginfo
%{_prefix}/lib/debug/*
