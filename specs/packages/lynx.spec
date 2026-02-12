Name:           lynx
Version:        2.9.2
Release:        1%{?dist}
Summary:        Text-mode web browser
License:        GPLv2
URL:            https://lynx.invisible-island.net/
Source0:        lynx-2.9.2.tar.gz

BuildRequires:  openssl-devel
BuildRequires:  ncurses-devel
BuildRequires:  zlib-devel

%description
Lynx is a text-based Web browser. Lynx does not display any images,
but it does support frames, tables, and most other HTML tags. One
advantage Lynx has over graphical browsers is speed; Lynx starts and
exits quickly and swiftly displays web pages.

%prep
%autosetup -n lynx2.9.2

%build
%configure \
    --with-screen=ncurses \
    --with-ssl \
    --with-zlib \
    --disable-nls \
    --disable-ipv6 \
    --disable-rpath-hack \
    --enable-default-colors
%make_build

%install
make install DESTDIR=$RPM_BUILD_ROOT

%files
%{_bindir}/lynx
%{_mandir}/man1/lynx.1*
%config(noreplace) %{_sysconfdir}/lynx.cfg
%config(noreplace) %{_sysconfdir}/lynx.lss
%doc AUTHORS CHANGES COPYING README
