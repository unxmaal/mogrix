Name:           gmi100
Version:        3.2
Release:        1%{?dist}
Summary:        Gemini CLI client in 100 lines of ANSI C
License:        Unlicense
URL:            https://github.com/ir33k/gmi100
Source0:        gmi100-3.2.tar.gz

BuildRequires:  openssl-devel

%description
Gemini protocol client for the terminal, written in 100 lines of ANSI C.
Supports persistent browsing history, link navigation, and search.

%prep
%autosetup -n gmi100-3.2

%build
%{__cc} %{optflags} -o gmi100 gmi100.c -lssl -lcrypto $LIBS

%install
install -Dpm 0755 gmi100 %{buildroot}%{_bindir}/gmi100

%files
%{_bindir}/gmi100
