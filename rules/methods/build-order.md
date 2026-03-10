# Mogrix Build Order

Dependency-phased build order for all current mogrix packages.
Waves are defined by dependency depth: Wave 0 has no mogrix-built deps,
Wave N depends only on packages from Waves 0 through N-1.

Based on `readelf -d` analysis of all built RPMs (March 2026).

## IRIX System Libraries (always available)

These are in the IRIX sysroot and don't need to be built:

libc.so.1, libm.so, libgen.so, libdl.so, libpthread.so, libnsl.so, libsocket.so,
libcrypt.so, librt.so, libxnet.so, libmx.so, libaudio.so,
libX11.so.1, libXext.so, libXt.so, libXmu.so, libXm.so.2, libGL.so, libGLU.so

## Toolchain Libraries (always available)

libgcc_s.so.1, libstdc++.so.6, libmogrix_compat.so

---

## Wave 0 — No Mogrix Dependencies

Only needs IRIX system libs and toolchain.

### Core Compression
| Package | Provides |
|---------|----------|
| zlib-ng-compat | libz.so |
| bzip2 | libbz2.so.1 |
| xz | liblzma.so.5 |
| libzstd | libzstd.so.1 |
| lz4 | liblz4.so.1 |
| lzo | liblzo2.so.2 |
| brotli | libbrotlicommon.so.1, libbrotlidec.so.1, libbrotlienc.so.1 |

### Core Text/String
| Package | Provides |
|---------|----------|
| pcre2 | libpcre2-8.so.0, libpcre2-16.so.0, libpcre2-32.so.0, libpcre2-posix.so.3 |
| gettext | libintl.so.8 |
| libunistring | libunistring.so.5 |
| fribidi | libfribidi.so.0 |
| oniguruma | libonig.so.5 |

### Core Libraries
| Package | Provides |
|---------|----------|
| libffi | libffi.so.8 |
| libtasn1 | libtasn1.so.6 |
| nettle | libnettle.so.8, libhogweed.so.6 |
| expat | libexpat.so.1 |
| gmp | libgmp.so.10 |
| libxml2 | libxml2.so.2 |
| libyaml | libyaml-0.so.2 |
| jansson | libjansson.so.4 |
| json-c | libjson-c.so.5 |
| popt | libpopt.so.0 |
| libev | libev.so.4 |
| libpipeline | libpipeline.so.1 |
| libtool-ltdl | libltdl.so.7 |
| pkgconf | (tool, no shared lib) |
| hyphen | libhyphen.so.0 |

### Graphics/Media (no mogrix deps)
| Package | Provides |
|---------|----------|
| pixman | libpixman-1.so.0 |
| libepoxy | libepoxy.so.0 |
| libjpeg-turbo | libjpeg.so.62 |
| lcms2 | liblcms2.so.2 |
| giflib | libgif.so.7 |
| SDL2 | libSDL2-2.0.so.0 |
| libogg | libogg.so.0 |
| opus | libopus.so.0 |
| double-conversion | libdouble-conversion.so.3 |
| mpg123-libs | libmpg123.so.0, libout123.so.0, libsyn123.so.0 |
| libao | libao.so.4 |

### Terminal
| Package | Provides |
|---------|----------|
| ncurses | libtinfo.so, libncurses.so, libncursesw.so, libtic.so |
| slang | libslang.so.2 |

### X11 Thin Wrappers (only need IRIX X11)
| Package | Provides |
|---------|----------|
| libICE | libICE.so.6 |
| libXrender | libXrender.so.1 |
| libXfixes | libXfixes.so.3 |
| libXcomposite | libXcomposite.so.1 |
| libXi | libXi.so.6 |
| libXinerama | libXinerama.so.1 |
| libxcb | libxcb.so.1, libxcb-*.so.* |
| libptytty | libptytty.so |

### Pure Executables (no mogrix lib deps)
banner, bomtool, chrpath, dash, diffstat, diffutils, dos2unix, ed, figlet,
findutils, gawk, grep, gzip, lolcat, lrzsz, m4, make, mksh, patch, pwgen,
sed, symlinks, tar, time, tree, which, xxd, zip

---

## Wave 1 — Depends on Wave 0 Only

| Package | Provides | Key Mogrix Deps |
|---------|----------|-----------------|
| openssl | libcrypto.so.3, libssl.so.3 | zlib |
| libpng | libpng16.so.16 | zlib |
| readline | libreadline.so.8, libhistory.so.8 | ncurses |
| sqlite | libsqlite3.so.0 | zlib |
| libgpg-error | libgpg-error.so.0 | gettext |
| p11-kit | libp11-kit.so.0 | libffi |
| libidn2 | libidn2.so.0 | libunistring |
| glib2 | libglib-2.0.so.0, libgio-2.0.so.0, libgobject-2.0.so.0, libgmodule-2.0.so.0 | libffi, gettext, pcre2 |
| libSM | libSM.so.6 | libICE |
| libvorbis | libvorbis.so.0, libvorbisenc.so.2, libvorbisfile.so.3 | libogg |
| flac-libs | libFLAC.so.12 | libogg |
| lame | libmp3lame.so.0 | ncurses |
| npth | libnpth.so.0 | (only libc, libpthread) |
| mpfr | libmpfr.so.6 | gmp |
| file | libmagic.so.1 | bzip2, xz, zlib, libzstd |
| libtiff | libtiff.so.6 | libjpeg-turbo, xz, zlib, libzstd |
| libXcursor | libXcursor.so.1 | libXrender, libXfixes |
| libXdamage | libXdamage.so.1 | libXfixes |
| libXrandr | libXrandr.so.2 | libXrender |
| libxkbcommon | libxkbcommon.so.0, libxkbcommon-x11.so.0 | libxcb, libxml2 |
| xcb-util | libxcb-util.so.1 | libxcb |
| xcb-util-image | libxcb-image.so.0 | libxcb, xcb-util |
| xcb-util-keysyms | libxcb-keysyms.so.1 | libxcb |
| xcb-util-renderutil | libxcb-render-util.so.0 | libxcb |
| xcb-util-wm | libxcb-wm.so.0 | libxcb |

### Wave 1 Executables
| Package | Key Mogrix Deps |
|---------|-----------------|
| bison | (only system) — could be W0 but listed here for grouping |
| bc | readline, ncurses |
| less | ncurses |
| cmatrix | ncurses |
| sl | ncurses |
| frotz | ncurses |
| nano | ncurses |
| ncdu | ncurses |
| joe | ncurses |
| vile | ncurses |
| tcsh | ncurses |
| screen | ncurses |
| zsh | ncurses |
| vim | ncurses, gettext |
| enscript | gettext |
| yasm | gettext |
| flex | (system only — could be W0) |
| re2c | (system only — could be W0) |
| gperf | (system only — could be W0) |
| patchutils | pcre2 |
| boxes | pcre2, ncurses, libunistring |
| jq | oniguruma |
| unzip | bzip2 |
| xclip | (IRIX X11 only — could be W0) |
| aterm | (IRIX X11 only — could be W0) |
| nedit | (IRIX X11 only — could be W0) |

---

## Wave 2 — Depends on Waves 0-1

| Package | Provides | Key Mogrix Deps |
|---------|----------|-----------------|
| freetype | libfreetype.so.6 | libpng (W1), bzip2 (W0), zlib (W0) |
| libgcrypt | libgcrypt.so.20 | libgpg-error (W1) |
| libevent | libevent_core-2.1.so.7 | openssl (W1) |
| libpsl | libpsl.so.5 | libidn2 (W1), libunistring (W0), gettext (W0) |
| atk | libatk-1.0.so.0 | glib2 (W1), gettext (W0) |
| libstrophe | libstrophe.so.0 | openssl (W1), expat (W0) |
| libretls | libtls.so.28 | openssl (W1) |
| libssh2 | libssh2.so.1 | openssl (W1), zlib (W0) |
| libksba | libksba.so.8 | libgpg-error (W1) |
| libassuan | libassuan.so.0 | libgpg-error (W1) |
| gdbm | libgdbm.so.6, libgdbm_compat.so.4 | readline (W1), gettext (W0) |
| SDL2_image | libSDL2_image-2.0.so.0 | SDL2 (W0), libpng (W1), libjpeg-turbo (W0) |
| libsndfile | libsndfile.so.1 | flac-libs (W1), libogg (W0), opus (W0), libvorbis (W1) |
| woff2 | libwoff2*.so.* | brotli (W0) |
| lua | liblua-5.4.so | readline (W1), ncurses (W0) |
| libwebp | libwebp.so.7, libwebpmux.so.3, libwebpdemux.so.2, libsharpyuv.so.0 | (system only — could be W0) |
| recode | librecode.so.3 | (system only — could be W0) |

### Wave 2 Executables
| Package | Key Mogrix Deps |
|---------|-----------------|
| bash | readline (W1), ncurses (W0) |
| gdb | readline (W1), gmp (W0), mpfr (W1), expat (W0), xz (W0), zlib (W0), libzstd (W0), gettext (W0) |
| pcre2-tools | pcre2 (W0), readline (W1) |
| rlwrap | readline (W1) |
| units | readline (W1) |
| irssi | openssl (W1), glib2 (W1), gettext (W0) |
| gmi100 | openssl (W1) |
| openssh | openssl (W1), zlib (W0) |
| tinc | openssl (W1), zlib (W0) |
| coreutils | openssl (W1), gettext (W0) |
| alpine | openssl (W1), ncurses (W0), gettext (W0) |
| lynx | openssl (W1), ncurses (W0), libidn2 (W1), bzip2 (W0), zlib (W0) |
| most | slang (W0) |
| mc | glib2 (W1), slang (W0), gettext (W0) |
| rsync | openssl (W1), popt (W0), libzstd (W0) |
| ctags | jansson (W0), pcre2 (W0), libxml2 (W0), libyaml (W0) |
| git | openssl (W1), pcre2 (W0), expat (W0), zlib (W0) |
| perl | gdbm (hmm, gdbm is W2 — perl should be W3) |
| tcl | zlib (W0) |
| lame (app) | ncurses (W0), lame-libs (W1) |
| lcms2-utils | lcms2 (W0), libjpeg-turbo (W0), libtiff (W1) |
| libpng-tools | libpng (W1), zlib (W0) |
| openssl (app) | openssl-libs (W1) |
| sqlite (app) | sqlite-libs (W1) |

---

## Wave 3 — Depends on Waves 0-2

| Package | Provides | Key Mogrix Deps |
|---------|----------|-----------------|
| fontconfig | libfontconfig.so.1 | freetype (W2), expat (W0), gettext (W0) |
| libcurl | libcurl.so.4 | openssl (W1), brotli (W0), libidn2 (W1), libpsl (W2) |
| gnutls | libgnutls.so.30 | nettle (W0), libtasn1 (W0), p11-kit (W1), gmp (W0), libidn2 (W1), libunistring (W0) |
| gdk-pixbuf2 | libgdk_pixbuf-2.0.so.0 | glib2 (W1), libjpeg-turbo (W0), libpng (W1) |
| gnupg2 | (executables) | libgcrypt (W2), libassuan (W2), libgpg-error (W1), npth (W1) |
| imlib2 | libImlib2.so.1 | freetype (W2), libpng (W1), libjpeg-turbo (W0), libtiff (W1), giflib (W0), libwebp (W2), bzip2 (W0), xz (W0), zlib (W0) |
| libxslt | libxslt.so.1, libexslt.so.0 | libxml2 (W0) |
| perl | libperl.so | gdbm (W2), zlib (W0) |

### Wave 3 Executables
| Package | Key Mogrix Deps |
|---------|-----------------|
| tmux | libevent (W2), ncurses (W0) |
| telescope | openssl (W1), libretls (W2), ncurses (W0) |
| decker | SDL2 (W0), SDL2_image (W2) |
| man-db | gdbm (W2), libpipeline (W0), gettext (W0), zlib (W0) |

---

## Wave 4 — Depends on Waves 0-3

| Package | Provides | Key Mogrix Deps |
|---------|----------|-----------------|
| harfbuzz | libharfbuzz.so.0 | freetype (W2), glib2 (W1), gettext (W0) |
| cairo | libcairo.so.2, libcairo-gobject.so.2 | freetype (W2), fontconfig (W3), libpng (W1), pixman (W0), glib2 (W1) |
| libXft | libXft.so.2 | fontconfig (W3), freetype (W2), libXrender (W0) |
| curl (app) | (executable) | libcurl (W3), openssl (W1), brotli (W0), libidn2 (W1), libpsl (W2) |
| libsoup | libsoup-2.4.so.1 | glib2 (W1), brotli (W0), libpsl (W2), sqlite (W1), libxml2 (W0) |
| GraphicsMagick | libGraphicsMagick-Q16.so.3 | freetype (W2), libpng (W1), libjpeg-turbo (W0), libtiff (W1), lcms2 (W0), libwebp (W2), libxml2 (W0), bzip2 (W0), xz (W0), zlib (W0), libzstd (W0) |
| fltk | libfltk.so | libjpeg-turbo (W0), libpng (W1), zlib (W0) |
| rpm | librpm.so, librpmio.so | popt (W0), lua (W2), file (W1), openssl (W1), bzip2 (W0), xz (W0), zlib (W0), libzstd (W0), sqlite (W1) |

### Wave 4 Executables
| Package | Key Mogrix Deps |
|---------|-----------------|
| snownews | libcurl (W3), ncurses (W0), libxml2 (W0), openssl (W1) |
| wget2 | gnutls (W3), brotli (W0), libidn2 (W1), pcre2 (W0), libpsl (W2), zlib (W0), libzstd (W0), gettext (W0) |
| weechat | gnutls (W3), libgcrypt (W2), libcurl (W3), ncurses (W0), zlib (W0), libzstd (W0) |
| profanity | libcurl (W3), glib2 (W1), readline (W1), ncurses (W0), sqlite (W1), libstrophe (W2), gettext (W0) |
| feh | imlib2 (W3), libcurl (W3), libpng (W1) |
| scrot | imlib2 (W3), libXcomposite (W0), libXfixes (W0), libXinerama (W0) |
| dmenu | libXft (W4) |
| st | libXft (W4), fontconfig (W3), freetype (W2) |
| xnedit | libXft (W4), fontconfig (W3), freetype (W2) |
| rxvt-unicode | libXft (W4), fontconfig (W3), freetype (W2), libptytty (W0) |

---

## Wave 5 — Depends on Waves 0-4

| Package | Provides | Key Mogrix Deps |
|---------|----------|-----------------|
| pango | libpango-1.0.so.0, libpangocairo-1.0.so.0, libpangoft2-1.0.so.0, libpangoxft-1.0.so.0 | harfbuzz (W4), cairo (W4), fontconfig (W3), freetype (W2), fribidi (W0), glib2 (W1), libXft (W4) |
| qt5-qtbase | libQt5Core.so.5, libQt5Gui.so.5 | openssl (W1), double-conversion (W0), pcre2 (W0), zlib (W0), libzstd (W0), fontconfig (W3), freetype (W2), harfbuzz (W4), libjpeg-turbo (W0), libpng (W1), libxcb (W0), libxkbcommon (W1), xcb-util-* (W1) |

### Wave 5 Executables
| Package | Key Mogrix Deps |
|---------|-----------------|
| dillo | libXcursor (W1), openssl (W1), fltk (W4), libjpeg-turbo (W0), libpng (W1), zlib (W0) |
| sox | libao (W0), libsndfile (W2), libogg (W0), libvorbis (W1), flac-libs (W1), lame-libs (W1), file (W1), libtool-ltdl (W0) |
| libwebp-tools | libwebp (W2), libjpeg-turbo (W0), libpng (W1), libtiff (W1), giflib (W0) |

---

## Wave 6 — Depends on Waves 0-5

| Package | Provides | Key Mogrix Deps |
|---------|----------|-----------------|
| gtk3 | libgtk-3.so.0, libgdk-3.so.0 | pango (W5), cairo (W4), gdk-pixbuf2 (W3), atk (W2), glib2 (W1), libepoxy (W0), fribidi (W0), fontconfig (W3), harfbuzz (W4), libXcomposite (W0), libXcursor (W1), libXdamage (W1), libXfixes (W0), libXi (W0), libXinerama (W0), libXrandr (W1) |
| vte291 | libvte-2.91.so.0 | gtk3 (W6), pango (W5), cairo (W4), glib2 (W1), gnutls (W3), gdk-pixbuf2 (W3), fribidi (W0), atk (W2), pcre2 (W0) |

---

## Wave 7 — Depends on Waves 0-6

### Apps
| Package | Key Mogrix Deps |
|---------|-----------------|
| gtkterm | gtk3 (W6), vte291 (W6), glib2 (W1), gdk-pixbuf2 (W3), pango (W5), gettext (W0) |
| ir8 | gtk3 (W6), webkitgtk (W7+), glib2 (W1), cairo (W4), harfbuzz (W4), atk (W2), gdk-pixbuf2 (W3), pango (W5) |

---

## Notes

### Circular Dependencies
- **harfbuzz ↔ freetype**: harfbuzz can use freetype, freetype can use harfbuzz. Build freetype first (W2) without harfbuzz, then harfbuzz (W4), then optionally rebuild freetype with harfbuzz support.

### WebKitGTK
webkitgtk is a special case — it depends on nearly everything (gtk3, cairo, harfbuzz-icu, ICU, libsoup, libxslt, libwebp, sqlite, glib2, etc.). It's effectively Wave 7+ and should be built last.

### Packages Not in Wave Structure
Some packages (autoconf, automake, cmake, doxygen, groff, help2man, ninja-build, quilt, stow, etc.) are build tools that don't produce shared libraries consumed by other packages. They can be built at any time.
