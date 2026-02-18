Name:           dillo
Version:        3.0.5
Release:        1%{?dist}
Summary:        Very small and fast GUI web browser
License:        GPLv3+
URL:            https://www.dillo.org
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  autoconf
BuildRequires:  automake
BuildRequires:  fltk-devel >= 1.3.0
BuildRequires:  libjpeg-devel
BuildRequires:  libpng-devel >= 1.2.0
BuildRequires:  make
BuildRequires:  openssl-devel
BuildRequires:  zlib-devel

%description
Dillo is a very small and fast GUI web browser. It is designed to be
lightweight and fast, using the FLTK toolkit.

%prep
%setup -q
autoreconf -vif

%build
%configure --disable-dependency-tracking --enable-ssl --disable-xft
%make_build

%install
%make_install
rm -f doc/Makefile*
rm -fr %{buildroot}%{_datadir}/doc/%{name}

%files
%config(noreplace) %{_sysconfdir}/%{name}/
%{_bindir}/%{name}
%{_bindir}/%{name}-install-hyphenation
%{_bindir}/dpid
%{_bindir}/dpidc
%{_libdir}/%{name}/
%{_mandir}/man1/*.1*
