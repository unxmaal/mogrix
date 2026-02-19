Name:           webkitgtk
Version:        2.42.5
Release:        1%{?dist}
Summary:        GTK port of the WebKit rendering engine
License:        LGPLv2+
URL:            https://webkitgtk.org
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.20
BuildRequires:  gcc-c++
BuildRequires:  make
BuildRequires:  perl

%description
WebKitGTK is a full-featured port of the WebKit rendering engine,
suitable for projects requiring any kind of web integration, from
hybrid HTML/CSS applications to full-fledged web browsers.

Cross-compiled for IRIX 6.5 with CLoop JavaScript interpreter
(no JIT), minimal feature set.

%package -n libwebkit2gtk
Summary:        WebKit2 GTK library
Provides:       webkit2gtk4.0(mips-32) = %{version}-%{release}
Provides:       libwebkit2gtk-4.0.so.37

%description -n libwebkit2gtk
WebKit2 GTK shared library for IRIX.

%package devel
Summary:        WebKitGTK development files
Requires:       libwebkit2gtk = %{version}-%{release}

%description devel
Development headers and pkgconfig files for WebKitGTK.

%package -n libjavascriptcoregtk
Summary:        JavaScriptCore GTK library
Provides:       javascriptcoregtk4.0(mips-32) = %{version}-%{release}
Provides:       libjavascriptcoregtk-4.0.so.18

%description -n libjavascriptcoregtk
JavaScriptCore shared library for IRIX.

%package jsc
Summary:        JavaScriptCore shell
Requires:       libjavascriptcoregtk = %{version}-%{release}

%description jsc
JSC is a command-line utility that evaluates JavaScript.

%prep
%autosetup -n webkitgtk-%{version}

# === IRIX Compatibility Patches ===

# 1. Fix %zu/%zd/%zx format specifiers — IRIX libc is pre-C99
find . -name '*.c' -o -name '*.cpp' -o -name '*.h' | \
  xargs sed -i 's/%%zu/%%u/g; s/%%zd/%%d/g; s/%%zx/%%x/g' 2>/dev/null || :

# 2. Provide unifdef, ruby, and other host tools in PATH
export PATH=/home/edodd/projects/github/unxmaal/mogrix/cross/native-tools:$PATH
export LD_LIBRARY_PATH=/tmp/ruby-pkg/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH

%build
# Cross-compile WebKitGTK for IRIX using our cmake toolchain
# Disable everything we don't need/can't support on IRIX
export PATH=/home/edodd/projects/github/unxmaal/mogrix/cross/native-tools:$PATH
export LD_LIBRARY_PATH=/tmp/ruby-pkg/usr/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH

# Reference the compat archive built by mogrix converter (mogrix-compat/ in BUILD dir)
COMPAT_DIR=$(pwd)/mogrix-compat

/usr/bin/cmake -B _build -S . \
  -DCMAKE_TOOLCHAIN_FILE=/home/edodd/projects/github/unxmaal/mogrix/cross/cmake-irix-toolchain.cmake \
  -DPORT=GTK \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=%{_prefix} \
  -DCMAKE_INSTALL_LIBDIR=%{_libdir} \
  -DCMAKE_C_FLAGS="%{optflags}" \
  -DCMAKE_CXX_FLAGS="%{optflags}" \
  -DCMAKE_EXE_LINKER_FLAGS="-L/opt/sgug-staging/usr/sgug/lib32 -L$COMPAT_DIR -lpthread -lm -lmogrix_compat -lmogrix-compat" \
  -DCMAKE_SHARED_LINKER_FLAGS="-L/opt/sgug-staging/usr/sgug/lib32 -lpthread -lm" \
  \
  -DUSE_GTK4=OFF \
  -DUSE_SOUP2=ON \
  \
  -DENABLE_JIT=OFF \
  -DENABLE_C_LOOP=ON \
  -DENABLE_FTL_JIT=OFF \
  -DENABLE_DFG_JIT=OFF \
  -DENABLE_SAMPLING_PROFILER=OFF \
  -DENABLE_WEBASSEMBLY=OFF \
  \
  -DENABLE_X11_TARGET=ON \
  -DENABLE_WAYLAND_TARGET=OFF \
  -DENABLE_QUARTZ_TARGET=OFF \
  \
  -DUSE_OPENGL_OR_ES=OFF \
  -DUSE_GBM=OFF \
  -DENABLE_WEBGL=OFF \
  \
  -DENABLE_DOCUMENTATION=OFF \
  -DENABLE_INTROSPECTION=OFF \
  -DENABLE_JOURNALD_LOG=OFF \
  -DENABLE_GAMEPAD=OFF \
  -DENABLE_SPELLCHECK=OFF \
  -DENABLE_SPEECH_SYNTHESIS=OFF \
  -DENABLE_WEB_CRYPTO=OFF \
  -DENABLE_WEBDRIVER=OFF \
  -DENABLE_TOUCH_EVENTS=OFF \
  -DENABLE_BUBBLEWRAP_SANDBOX=OFF \
  -DENABLE_PDFJS=OFF \
  -DENABLE_MEDIA_STREAM=OFF \
  -DENABLE_WEB_RTC=OFF \
  -DENABLE_MEDIA_RECORDER=OFF \
  -DENABLE_ENCRYPTED_MEDIA=OFF \
  -DENABLE_GPU_PROCESS=OFF \
  \
  -DUSE_LIBSECRET=OFF \
  -DUSE_LIBHYPHEN=OFF \
  -DUSE_OPENJPEG=OFF \
  -DUSE_WOFF2=OFF \
  -DUSE_AVIF=OFF \
  -DUSE_JPEGXL=OFF \
  -DUSE_LCMS=OFF \
  \
  -DENABLE_UNIFIED_BUILDS=OFF \
  -DUSE_SYSTEM_MALLOC=ON \
  -DENABLE_MINIBROWSER=ON \
  -DENABLE_API_TESTS=OFF \
  -DENABLE_LAYOUT_TESTS=OFF \
  -DENABLE_XSLT=ON \
  -DENABLE_DRAG_SUPPORT=ON \
  \
  -DUSE_GSTREAMER=OFF \
  -DUSE_GSTREAMER_TRANSCODER=OFF \
  -DENABLE_VIDEO=OFF \
  -DENABLE_WEB_AUDIO=OFF \
  \
  -DRuby_EXECUTABLE=/home/edodd/projects/github/unxmaal/mogrix/cross/native-tools/ruby \
  -DIMPORT_EXECUTABLES= \
  -DCMAKE_CROSSCOMPILING=ON

cd _build && make %{?_smp_mflags} VERBOSE=1

%install
cd _build && make install DESTDIR=%{buildroot}

# Clean up files we don't need
rm -rf %{buildroot}%{_datadir}/locale
rm -rf %{buildroot}%{_datadir}/gir-1.0
rm -rf %{buildroot}%{_libdir}/girepository-1.0

%files -n libwebkit2gtk
%{_libdir}/libwebkit2gtk-4.0.so.37*
%{_libdir}/webkit2gtk-4.0/
%dir %{_libexecdir}/webkit2gtk-4.0
%{_libexecdir}/webkit2gtk-4.0/WebKitWebProcess
%{_libexecdir}/webkit2gtk-4.0/WebKitNetworkProcess
%{_libexecdir}/webkit2gtk-4.0/MiniBrowser

%files devel
%{_includedir}/webkitgtk-4.0/
%{_libdir}/libwebkit2gtk-4.0.so
%{_libdir}/libjavascriptcoregtk-4.0.so
%{_libdir}/pkgconfig/webkit2gtk-4.0.pc
%{_libdir}/pkgconfig/webkit2gtk-web-extension-4.0.pc
%{_libdir}/pkgconfig/javascriptcoregtk-4.0.pc

%files -n libjavascriptcoregtk
%{_libdir}/libjavascriptcoregtk-4.0.so.18*

%files jsc
%{_libexecdir}/webkit2gtk-4.0/jsc
