Name:           atk
Version:        2.36.0
Release:        1%{?dist}
Summary:        Interfaces for accessibility support
License:        LGPL-2.1-or-later
URL:            https://download.gnome.org/sources/atk/2.36/
Source0:        atk-%{version}.tar.gz

BuildRequires:  glib2-devel
BuildRequires:  gettext

%description
The ATK library provides a set of interfaces for adding accessibility
support to applications and graphical user interface toolkits.

%package devel
Summary:        Development files for the ATK accessibility toolkit
Requires:       %{name} = %{version}-%{release}

%description devel
Development files for ATK.

%prep
%autosetup -n atk-%{version}

%build
# Add glib build tools (Python scripts only, not IRIX binaries) to PATH
export PATH=/home/edodd/projects/github/unxmaal/mogrix/cross/native-tools:$PATH

MOGRIX_CROSS=/home/edodd/projects/github/unxmaal/mogrix/cross/meson-irix-cross.ini
/home/edodd/.local/bin/meson setup _build \
  --cross-file=$MOGRIX_CROSS \
  --prefix=%{_prefix} \
  --libdir=%{_libdir} \
  -Dintrospection=false \
  -Ddocs=false \
  --wrap-mode=nodownload

/home/edodd/.local/bin/meson compile -C _build

%install
DESTDIR=%{buildroot} /home/edodd/.local/bin/meson install -C _build --no-rebuild

rm -rf %{buildroot}%{_datadir}/locale

%files
%license COPYING
%{_libdir}/libatk-1.0.so.*

%files devel
%{_libdir}/libatk-1.0.so
%{_includedir}/atk-1.0/
%{_libdir}/pkgconfig/atk.pc
