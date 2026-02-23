# X.org Extension Libraries (Cross-Compilation)

**Problem: `xorg_cv_malloc0_returns_null` configure test fails during cross-compilation**
X.org libraries use their own `XORG_CHECK_MALLOC_ZERO` macro (not standard autoconf `ac_cv_func_malloc_0_nonnull`). It tries to run a test program, which fails when cross-compiling.
**Fix:** Add `ac_cv_overrides: { xorg_cv_malloc0_returns_null: "no" }` to the package YAML. Already applied to: libXrender, libXft, libXi, libXinerama, libXrandr.

**Problem: autoconf 2.71 `cannot make irix-cc report undeclared builtins`**
Host autoconf 2.71's new configure check fails with our cross-compiler. Affects all packages using autoreconf.
**Fix:** Don't use autoreconf for X11 libs — use the shipped configure script. For headers-only packages (xorgproto), skip configure entirely.

**Problem: `_XEatDataWords` undeclared (IRIX sysroot Xlibint.h)**
IRIX's native libX11 is pre-1.6, missing `_XEatDataWords`. It's just `_XEatData(dpy, n * 4)`.
**Fix:** Add patch with `#ifndef _XEatDataWords` / `static inline void _XEatDataWords(...)` shim. Needed by: libXfixes, libXi, libXinerama, libXrandr.

**Problem: Missing `Xge.h` (Generic Event Extension) for libXi**
IRIX's X11 predates X11R7.5. libXi's XI2 code needs `XGenericEvent`/`XGenericEventCookie` types and functions (`XESetWireToEventCookie`, `XESetCopyEventCookie`, `_XUnknownNativeEvent`).
**Fix:** Create stub `Xge.h` header + `xge_compat.c` with safe no-op implementations (IRIX X server doesn't generate Generic Events). See `patches/packages/libXi/`.

## Meson Cross-Builds (GTK3 and similar)

**Problem: irix-cc doesn't expand response files (@file) from meson**
Meson uses response files for large link commands. Our `irix-cc` shell wrapper doesn't expand `@file` syntax, so `-Wl,` flags and `-shared` flag are invisible to the wrapper's filtering logic.
**Fix:** Added response file expansion to `irix-cc` — detects `@file` args, reads and expands their contents (stripping single quotes), then re-sets positional params. See `cross/bin/irix-cc`.

**Problem: IRIX XEvent union lacks `xcookie` member (pre-X11R7.5)**
GTK3's X11 backend accesses `xevent->xcookie` extensively (gdkdevicemanager-xi2.c, gdkdisplay-x11.c, gdkeventsource.c, gdkwindow-x11.c, gtkdnd.c). IRIX's XEvent union predates GenericEvent.
**Fix:** prep_commands: (1) add `#include <X11/extensions/Xge.h>` to each file, (2) replace `&var->xcookie` → `(XGenericEventCookie*)var`, (3) replace `var->xcookie.member` → `((XGenericEventCookie*)var)->member`, (4) replace standalone `var->xcookie` → `(*((XGenericEventCookie*)var))`. Xge.h also provides inline stubs for `XGetEventData`/`XFreeEventData`/`XSetIMValues`.

**Problem: No atk-bridge-2.0 (AT-SPI2 accessibility) on IRIX**
GTK3 unconditionally includes `atk-bridge.h` under `#ifdef GDK_WINDOWING_X11`.
**Fix:** (1) Make dep `required: false` in meson.build, (2) Guard pkgconfig reference, (3) sed out the include and `atk_bridge_adaptor_init` call.

**Problem: GTK3 requires native glib tools (glib-compile-resources, gdbus-codegen, xmllint)**
These are build-machine tools not available on the cross-compile host.
**Fix:** Extract from Ubuntu debs using `apt-get download` + `dpkg-deb -x` to `/tmp/`. Symlink/copy to `cross/native-tools/`. gdbus-codegen needs its Python module path patched to point to the extracted location.
