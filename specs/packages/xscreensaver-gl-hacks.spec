Name:           xscreensaver-gl-hacks
Version:        6.08
Release:        1%{?dist}
Summary:        OpenGL screensaver demos from XScreenSaver for IRIX
License:        MIT
URL:            https://www.jwz.org/xscreensaver/
Source0:        xscreensaver-6.08.tar.gz
# IRIX cross-compilation patches:
# - Add missing X11/keysym.h includes (3 files)
# - Disable glGenerateMipmap (GL 3.0, not on IRIX ~GL 1.2)
# - Guard cubocteversion GLSL code behind HAVE_GLSL
Patch0:         xscreensaver-irix.patch
# GL/GLU stub libraries for cross-compilation:
# IRIX native libGL/libGLU have IRIX-specific ELF extensions and UND symbols
# that our linker rejects. These stubs provide symbol definitions at link time;
# at runtime the real IRIX GL libraries are loaded via rld.
Source1:        gl_stub.c
Source2:        glu_stub.c

BuildRequires:  libpng-devel

%description
129 OpenGL screensaver demos extracted from XScreenSaver 6.08, cross-compiled
for SGI IRIX 6.5. Includes gears, pipes, lament, starwars, atlantis, molecule,
and many more. Requires IRIX with OpenGL hardware acceleration.

%prep
%autosetup -n xscreensaver-6.08 -p1
cp %{_sourcedir}/gl_stub.c .
cp %{_sourcedir}/glu_stub.c .

%build
# Build GL/GLU stub shared libraries for cross-link
%{__cc} %{optflags} -shared -o libGL.so gl_stub.c
%{__cc} %{optflags} -shared -o libGLU.so glu_stub.c

# Install stubs where the build can find them
STUBDIR=$(pwd)/stubs
mkdir -p $STUBDIR
cp libGL.so libGLU.so $STUBDIR/

# Configure xscreensaver for IRIX cross-compilation
# Only enable GL hacks — disable everything else (PAM, GTK, systemd, etc.)
# Use out-of-tree build
mkdir -p _build && cd _build

# xscreensaver's configure treats XInput2 and Xft as hard requirements:
# it exits non-zero (CONF_STATUS=1) if they're missing, even though
# Makefiles are generated. GL hacks don't need either feature.
# Force CONF_STATUS=0 at the end of configure to prevent the abort:
sed -i 's/^exit \$CONF_STATUS$/exit 0/' ../configure

CC="%{__cc}" \
LIBS="-L$STUBDIR $LIBS -lgen -lpng -lz" \
CFLAGS="%{optflags} -I/opt/irix-sysroot/usr/include" \
LDFLAGS="-L/opt/sgug-staging/usr/sgug/lib32 -L/opt/irix-sysroot/usr/lib32" \
../configure \
  --host=mips-sgi-irix6.5 \
  --prefix=%{_prefix} \
  --without-pam \
  --without-shadow \
  --without-kerberos \
  --without-setuid-hacks \
  --without-login-manager \
  --without-gtk \
  --without-systemd \
  --without-elogind \
  --without-record-animation \
  --without-xf86vmode-ext \
  --without-gle \
  --without-xft \
  --with-gl \
  --with-x \
  --x-includes=/opt/irix-sysroot/usr/include \
  --x-libraries=/opt/irix-sysroot/usr/lib32 \
  --with-jpeg \
  --with-png

# Verify Makefiles were actually created:
test -f hacks/glx/Makefile || { echo "FATAL: configure failed to generate Makefiles"; exit 1; }

# Patch Makefile to use our stubs and compat library
sed -i 's|HACK_POST     =.*|HACK_POST     = $(LIBS) $(X_LIBS) $(X_PRE_LIBS) $(XFT_LIBS) -lXt -lX11 -lXext $(X_EXTRA_LIBS) -lm -lmogrix-compat|' hacks/glx/Makefile
sed -i "s|-lGLU -lGL|-L$STUBDIR -lGLU -lGL|" hacks/glx/Makefile

# Build only utils, screenhack support, and GL hacks
make -C utils %{?_smp_mflags}
make -C hacks screenhack.o xlockmore.o fps.o %{?_smp_mflags}
# Build generated image headers first (avoids race condition with -jN)
make -C hacks/images %{?_smp_mflags}
make -C hacks/glx %{?_smp_mflags}

%install
# Install GL hack binaries
mkdir -p %{buildroot}%{_bindir}
cd _build/hacks/glx
for bin in $(file * | grep 'ELF 32-bit MSB executable' | cut -d: -f1); do
  install -m 755 "$bin" %{buildroot}%{_bindir}/
done

%files
%license README
%{_bindir}/*
