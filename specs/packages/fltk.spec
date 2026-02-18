Name:           fltk
Version:        1.3.11
Release:        1%{?dist}
Summary:        C++ user interface toolkit
License:        LGPLv2+
URL:            https://www.fltk.org/
Source0:        fltk-1.3.11.tar.gz

%description
FLTK (pronounced "fulltick") is a cross-platform C++ GUI toolkit.
It provides modern GUI functionality without the bloat, and supports
3D graphics via OpenGL and its built-in GLUT emulation.

%prep
%autosetup -p1 -n fltk-1.3.11

%build
%configure
make %{?_smp_mflags}

%install
%make_install

%files
%{_bindir}/fltk-config
%{_libdir}/libfltk*.so*
%{_libdir}/libfltk*.a
%{_includedir}/FL
%{_includedir}/Fl
