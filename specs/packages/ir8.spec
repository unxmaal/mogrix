Name:           ir8
Version:        1.0
Release:        1%{?dist}
Summary:        Web browser for IRIX
License:        BSD-2-Clause
URL:            https://github.com/unxmaal/mogrix

Source0:        ir8-%{version}.tar.gz

BuildRequires:  pkgconfig(webkit2gtk-4.0)
BuildRequires:  pkgconfig(gtk+-3.0)
BuildRequires:  glib2-devel

%description
ir8 is a tabbed web browser for SGI IRIX, built on WebKit2GTK.
Features tabbed browsing, bookmarks, search, downloads, and
IRIX-appropriate defaults.

%prep
%autosetup -n ir8-%{version}

%build
export CC="%{__cc}"
export CXX="%{__cxx}"
export PKG_CONFIG_LIBDIR="$MOGRIX_STAGING/lib32/pkgconfig:$MOGRIX_STAGING/share/pkgconfig"
export PKG_CONFIG_SYSROOT_DIR="$MOGRIX_STAGING_ROOT"
%make_build

%install
%make_install PREFIX=%{_prefix} DESTDIR=%{buildroot}

%files
%{_bindir}/ir8
