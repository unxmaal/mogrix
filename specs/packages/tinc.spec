Name:           tinc
Version:        1.0.36
Release:        1%{?dist}
Summary:        Virtual Private Network daemon
License:        GPLv2+
URL:            https://www.tinc-vpn.org/
Source0:        tinc-1.0.36.tar.gz

BuildRequires:  openssl-devel
BuildRequires:  zlib-devel
BuildRequires:  automake autoconf

%description
tinc is a Virtual Private Network (VPN) daemon that uses tunnelling
and encryption to create a secure private network between hosts on
the Internet.

%prep
%autosetup -n tinc-1.0.36

%build
autoreconf -fi
%configure \
    --disable-uml \
    --disable-lzo \
    --with-openssl=yes
%make_build

%install
%make_install

%files
%{_sbindir}/tincd
%{_mandir}/man5/tinc.conf.5*
%{_mandir}/man8/tincd.8*
%{_infodir}/tinc.info*
%doc AUTHORS COPYING COPYING.README NEWS README THANKS
