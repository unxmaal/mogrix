Name:           decker
Version:        1.64
Release:        1%{?dist}
Summary:        Multimedia platform for creating and sharing interactive documents
License:        MIT
URL:            https://github.com/JohnEarnest/Decker
Source0:        decker-1.64.tar.gz

BuildRequires:  SDL2-devel
BuildRequires:  SDL2_image-devel
BuildRequires:  vim-common

%description
Decker is a multimedia platform for creating and sharing interactive documents,
with sound, images, hypertext, and scripted behavior. A HyperCard-like authoring
tool that runs as both a native desktop application and in web browsers.

%prep
%autosetup -n decker-1.64

%build
# Generate resources header (needs xxd from vim-common)
chmod +x scripts/resources.sh
./scripts/resources.sh examples/decks/tour.deck

# Build lilt (standalone scripting interpreter, no SDL deps)
%{__cc} %{optflags} ./c/lilt.c -o lilt -lm -DVERSION="\"1.64\"" $LIBS

# Build decker (SDL2 GUI application)
# Use pkg-config for SDL2 flags (works in cross-compile with PKG_CONFIG_PATH set)
%{__cc} %{optflags} ./c/decker.c -o decker \
  -I/opt/sgug-staging/usr/sgug/include/SDL2 -lSDL2 -lSDL2_image \
  -lm -DVERSION="\"1.64\"" $LIBS

%install
install -Dpm 0755 decker %{buildroot}%{_bindir}/decker
install -Dpm 0755 lilt %{buildroot}%{_bindir}/lilt

# Install the example tour deck
install -Dpm 0644 examples/decks/tour.deck %{buildroot}%{_datadir}/decker/tour.deck

%files
%license LICENSE.txt
%{_bindir}/decker
%{_bindir}/lilt
%{_datadir}/decker/tour.deck
