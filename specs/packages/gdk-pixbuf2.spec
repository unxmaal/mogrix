Name:           gdk-pixbuf2
Version:        2.42.10
Release:        1%{?dist}
Summary:        An image loading library

License:        LGPL-2.1-or-later
URL:            https://gitlab.gnome.org/GNOME/gdk-pixbuf
Source0:        gdk-pixbuf-%{version}.tar.xz

BuildRequires:  glib2-devel
BuildRequires:  gettext
BuildRequires:  libpng-devel
BuildRequires:  libjpeg-turbo-devel
BuildRequires:  libtiff-devel

%description
gdk-pixbuf is an image loading library that can be extended by loadable
modules for new image formats. It is used by toolkits such as GTK+ or
clutter.

%package devel
Summary:        Development files for gdk-pixbuf2
Requires:       %{name} = %{version}-%{release}
Requires:       glib2-devel

%description devel
Development files for gdk-pixbuf2.

%prep
%autosetup -n gdk-pixbuf-%{version}

%build
export PATH=/home/edodd/projects/github/unxmaal/mogrix/cross/native-tools:$PATH

MOGRIX_CROSS=/home/edodd/projects/github/unxmaal/mogrix/cross/meson-irix-cross.ini
/home/edodd/.local/bin/meson setup _build \
  --cross-file=$MOGRIX_CROSS \
  --prefix=%{_prefix} \
  --libdir=%{_libdir} \
  -Dintrospection=disabled \
  -Dgtk_doc=false \
  -Ddocs=false \
  -Dman=false \
  -Dtests=false \
  -Dinstalled_tests=false \
  -Dgio_sniffing=false \
  --wrap-mode=nodownload

/home/edodd/.local/bin/meson compile -C _build

%install
DESTDIR=%{buildroot} /home/edodd/.local/bin/meson install -C _build --no-rebuild

rm -rf %{buildroot}%{_datadir}/locale

%files
%license COPYING
%{_libdir}/libgdk_pixbuf-2.0.so.*
%dir %{_libdir}/gdk-pixbuf-2.0
%{_libdir}/gdk-pixbuf-2.0/2.10.0/
%{_bindir}/gdk-pixbuf-query-loaders
%{_bindir}/gdk-pixbuf-thumbnailer

%files devel
%{_libdir}/libgdk_pixbuf-2.0.so
%{_includedir}/gdk-pixbuf-2.0/
%{_libdir}/pkgconfig/gdk-pixbuf-2.0.pc
