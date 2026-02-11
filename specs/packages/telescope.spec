Name:           telescope
Version:        0.11
Release:        1%{?dist}
Summary:        Gemini, Gopher and Finger browser for the terminal
License:        ISC
URL:            https://github.com/omar-polo/telescope
Source0:        telescope-0.11.tar.gz

BuildRequires:  ncurses-devel
BuildRequires:  libretls-devel
BuildRequires:  openssl-devel
BuildRequires:  bison
BuildRequires:  automake autoconf

%description
Telescope is a w3m-like browser for Gemini, Gopher and Finger.
Features include tabs, bookmarks, client certificates, and
an Emacs-inspired UI.

%prep
%autosetup -n telescope-0.11

%build
# Pass CPPFLAGS so telescope finds our compat/endian.h
export CPPFLAGS="-I$(pwd)/compat $CPPFLAGS"
autoreconf -fi
# HOSTCC is used for pagebundler (build-time code generator)
%configure --with-default-editor=vi HOSTCC=gcc HOSTCFLAGS=""
%make_build

%install
%make_install

%files
%{_bindir}/telescope
%{_bindir}/telescope-identity
%{_mandir}/man1/telescope.1*
%{_mandir}/man1/telescope-identity.1*
