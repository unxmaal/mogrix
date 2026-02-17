Name:           xorg-x11-proto-devel
Version:        2019.1
Release:        1%{?dist}
Summary:        X.Org X11 Protocol headers
License:        MIT
URL:            https://www.x.org
Source0:        xorg-x11-proto-devel-%{version}.tar.gz

%description
X.Org X11 Protocol headers

%prep
%autosetup -n xorgproto-%{version}

%build
# Headers-only package — nothing to compile

%install
# Install headers directly
mkdir -p %{buildroot}%{_includedir}/GL
mkdir -p %{buildroot}%{_includedir}/GL/internal
mkdir -p %{buildroot}%{_includedir}/X11
mkdir -p %{buildroot}%{_includedir}/X11/dri
mkdir -p %{buildroot}%{_includedir}/X11/extensions
mkdir -p %{buildroot}%{_includedir}/X11/fonts

cp -a include/GL/*.h %{buildroot}%{_includedir}/GL/
cp -a include/GL/internal/*.h %{buildroot}%{_includedir}/GL/internal/
cp -a include/X11/*.h %{buildroot}%{_includedir}/X11/
cp -a include/X11/dri/*.h %{buildroot}%{_includedir}/X11/dri/
cp -a include/X11/extensions/*.h %{buildroot}%{_includedir}/X11/extensions/
cp -a include/X11/fonts/*.h %{buildroot}%{_includedir}/X11/fonts/

# Remove conflicting glxtokens
rm -f %{buildroot}%{_includedir}/GL/glxtokens.h

# Trim platform-specific headers
for i in apple windows trap ; do
    rm -f %{buildroot}%{_includedir}/X11/extensions/*${i}*
done

# Install pkg-config files with correct prefix
mkdir -p %{buildroot}%{_datadir}/pkgconfig
for pc in *.pc.in ; do
    pcname=$(basename "$pc" .pc.in)
    sed -e 's|@prefix@|%{_prefix}|g' \
        -e 's|@datarootdir@|%{_datadir}|g' \
        -e 's|@datadir@|%{_datadir}|g' \
        -e 's|@includedir@|%{_includedir}|g' \
        -e 's|@PACKAGE_VERSION@|%{version}|g' \
        "$pc" > %{buildroot}%{_datadir}/pkgconfig/${pcname}.pc
done
# Remove platform-specific pc files
for i in apple windows trap ; do
    rm -f %{buildroot}%{_datadir}/pkgconfig/*${i}*
done

%files
%dir %{_includedir}/GL
%{_includedir}/GL/*.h
%{_includedir}/GL/internal/
%dir %{_includedir}/X11
%{_includedir}/X11/*.h
%dir %{_includedir}/X11/dri/
%{_includedir}/X11/dri/*.h
%dir %{_includedir}/X11/extensions/
%{_includedir}/X11/extensions/*.h
%dir %{_includedir}/X11/fonts/
%{_includedir}/X11/fonts/*.h
%{_datadir}/pkgconfig/*.pc
