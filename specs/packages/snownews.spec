Name:           snownews
Version:        1.11
Release:        1%{?dist}
Summary:        Console RSS reader
License:        GPLv3
URL:            https://sourceforge.net/projects/snownews/
Source0:        snownews-1.11.tar.gz

BuildRequires:  ncurses-devel
BuildRequires:  libxml2-devel
BuildRequires:  libcurl-devel
BuildRequires:  openssl-devel

%description
Snownews is a text mode RSS/RDF newsreader. It depends on ncurses
for the user interface and uses libxml2 for XML parsing. Feed
updates are fetched using libcurl.

%prep
%autosetup -n snownews-1.11

%build
export PKG_CONFIG_SYSROOT_DIR="/opt/sgug-staging"
export PKG_CONFIG_PATH="/opt/sgug-staging/usr/sgug/lib32/pkgconfig"

# Create local libintl.h stub to shadow system gettext
# (our libintl exports bindtextdomain, not libintl_bindtextdomain)
mkdir -p nls-stub
cat > nls-stub/libintl.h << 'STUBEOF'
#ifndef _LIBINTL_H
#define _LIBINTL_H
#define bindtextdomain(d,p) ((void)0)
#define textdomain(d) ((void)0)
#define gettext(s) (s)
#define dgettext(d,s) (s)
#define dcgettext(d,s,c) (s)
#define ngettext(s,p,n) ((n)==1?(s):(p))
#define dngettext(d,s,p,n) ((n)==1?(s):(p))
#define bind_textdomain_codeset(d,c) ((void)0)
#define _(s) (s)
#define N_(s) (s)
#endif
STUBEOF

# Set CC for configure to detect our cross-compiler
export CC="%{__cc}"
# Extra include paths: NLS stub (shadows system libintl.h) + mogrix compat headers
export CFLAGS="-I$(pwd)/nls-stub -I/usr/sgug/include/mogrix-compat/generic"

./configure --prefix=/usr/sgug

# Fix generated Config.mk for IRIX cross-compilation
COMPAT_DIR=$(pwd)/mogrix-compat
# Replace -std=c11 with -std=gnu11 for better compat
sed -i 's/-std=c11/-std=gnu11/' Config.mk
# Append compat library to the libs line
sed -i "s|^libs.*|& -L$COMPAT_DIR -lmogrix-compat|" Config.mk
# Disable NLS build (no msgfmt during cross-build)
: > po/Module.mk

make %{?_smp_mflags}

%install
make install DESTDIR=$RPM_BUILD_ROOT PREFIX=/usr/sgug

%files
%{_bindir}/snownews
%{_mandir}/man1/snownews.1*
%{_mandir}/*/man1/snownews.1*
%doc LICENSE.md README.md
