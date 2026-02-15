# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-15 (session 48)
**Status**: Fixed `__rld_obj_head` crash — all executables loading libC.so.2/libGLcore.so now work. XScreenSaver GL hacks rebuilt and redeployed. Awaiting user testing with X display.

---

## Critical Warnings

**NEVER install packages directly to /usr/sgug on the live IRIX system.** Always use `/opt/chroot` for testing.

**NEVER use raw SSH to IRIX.** Always use MCP tools (`irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`).

**NEVER CHEAT.** Don't copy from SGUG-RSE. Don't fake Provides. Every FC40 package must be properly cross-compiled.

---

## Current State

### cmake ecosystem is DONE

- **cmake 3.28.2**: Built, staged, verified on IRIX. Tarball at `mogrix_outputs/rpms/cmake/cmake-3.28.2-irix.tar.gz`.
- **json-c 0.17**: Built, staged to sysroot, installed on IRIX. RPMs at `~/rpmbuild/RPMS/mips/json-c-*.rpm`.
- **brotli 1.1.0**: Built, staged to sysroot, installed on IRIX. RPMs at `~/mogrix_outputs/rpms/brotli/`.
- **libwebp 1.3.2**: Built, staged to sysroot, installed on IRIX. RPMs at `~/mogrix_outputs/rpms/libwebp/`.
- **ninja-build 1.11.1**: Built, installed on IRIX, `ninja --version` works. RPMs at `~/mogrix_outputs/rpms/ninja-build/`.

### Staging sysroot updated

`/opt/sgug-staging/usr/sgug/lib32/libmogrix_compat.a` now contains: mkdtemp.o, setenv.o, pselect.o, spawn.o.

New compat header: `compat/include/mogrix-compat/generic/sys/select.h` (pselect declaration).

### cmake build pattern established

Use `CMAKE_SYSTEM_NAME=Linux` (NOT IRIX). See INDEX.md "cmake %cmake/%cmake3 macro" and json-c.yaml / brotli.yaml / libwebp.yaml for the standard pattern.

---

## Next Steps

1. **Build meson** — needs ninja (now done). Unblocks many packages.
2. **Build remaining #202 tools** — gdb, doxygen.
3. **Build remaining dev tools** — fossil, mercurial.
4. **Build #201 GTK3 stack** — image libs now ready (libwebp, lcms2, libtiff, imlib2 all staged).
5. **Build #204 IPC/heavy** — dbus, dbus-glib, icu, cyrus-sasl.
6. **Low priority**: `drop_buildrequires_if_unavailable` engine feature, ksh build system.

### Batch Progress

| Batch | Status | Details |
|-------|--------|---------|
| #191 Quick-win CLI | COMPLETED | screen, zsh, ed, dtach, pwgen, rlwrap, units, diffstat, most |
| #192 Fun/trivial | COMPLETED | cowsay, banner, neofetch, screenfetch, boxes, sl |
| #193 Sysadmin | COMPLETED | dos2unix, ncdu, dash, lrzsz |
| #194 Dev tools pt1 | COMPLETED | ctags, patchutils, enscript, mandoc, help2man, recode |
| #195 Alt shells | COMPLETED | mksh verified on IRIX; ksh BLOCKED (complex build sys) |
| #196 Foundation libs | COMPLETED | slang/jansson/libyaml/libev/libcaca/json-c done |
| #197 Compression/net | COMPLETED | lz4/lzo/libpsl/libssh2/brotli done |
| #198 User apps | COMPLETED | alpine/vile/frotz/stow/sox/mc/irssi/joe done |
| #199 Audio | COMPLETED | libogg/libvorbis/flac/opus/lame/mpg123/libao/libsndfile done |
| #200 Image libs | COMPLETED | libtiff/lcms2/imlib2/libwebp done; openjpeg2 no SRPM |
| #201 GTK3 stack | PENDING | image libs now ready |
| #202 Build tools | MOSTLY DONE | cmake + ninja done; meson, gdb, doxygen pending |
| #203 Dev tools pt2 | MOSTLY DONE | re2c/yasm/quilt done; fossil/mercurial pending |
| #204 IPC/heavy | PENDING | dbus, dbus-glib, icu, cyrus-sasl |
| #205 GUI apps | PENDING | hexchat, geany, pidgin, nedit |

### Blocked Packages

| Package | Reason |
|---------|--------|
| ksh | ~3400-line custom build system |
| htop | needs IRIX /proc backend |
| openjpeg2 | no SRPM available |

---

## Recent Work

### Session 48: `__rld_obj_head` fix + GL hacks rebuild

- **Root cause of GL hacks crash**: `__rld_obj_head` missing from .dynsym. IRIX rld requires executables to export this symbol; libC.so.2 and libGLcore.so both reference it. crt1.o defines it as COMMON, but LLD doesn't export COMMON symbols to .dynsym.
- **Fix**: Added `--export-dynamic-symbol=__rld_obj_head` to LLD invocation in `cross/bin/irix-ld` (line ~197). Deployed to staging.
- **This fix applies to ALL executables** — not just GL hacks. Any binary loading libC.so.2 (C++ apps) or libGLcore.so (GL apps) now works.
- Rebuilt all 129 GL hack binaries, rebundled, deployed to IRIX
- **IRIX GL architecture documented**: libGL.so = GLX dispatch + `__glx_dispatch` table; libGLcore.so = all gl* functions as 12-byte stubs
- **Bundled libz.so** (zlib-ng 1.3.0): IRIX ships zlib 1.1.4, too old for libpng 1.6. "libpng error: bad parameters to zlib" → abort. Fixed by adding libz.so to bundle `_lib32/`.
- **User confirmed working** — zlib fix resolved libpng crashes. Some GL hacks (glplanet) crash 4dwm — likely IRIX GL driver limits, not fixable from our side.
- Debugging tip: `par -a 300` needed to capture full rld error messages (default truncates)

### Session 47: XScreenSaver GL hacks compilation

- 129 GL hacks compiled from xscreensaver 6.08 (hand-built, not mogrix SRPM pipeline)
- Created GL/GLU stub libraries for cross-compilation
- Source patches: `<X11/keysym.h>`, `strcasestr` compat, disabled `glGenerateMipmap`, GLSL guards
- Added `strcasestr.o` to `libmogrix_compat.a`

### Session 46: cmake-blocked packages

- Built json-c, brotli, ninja-build, libwebp — all verified on IRIX

---

## Reference

| What | Where |
|------|-------|
| Generic rules summary | `rules/GENERIC_SUMMARY.md` (read this first) |
| Problem lookup | `rules/INDEX.md` (grep, don't read whole file) |
| Agent orchestration | `rules/methods/task-tracking.md` |
| Package rules | `rules/packages/<name>.yaml` |
| Project architecture | `plan.md` |
| Build methods | `rules/methods/*.md` |
| Compat functions | `compat/catalog.yaml` |
| Workspace paths & workflow | `CLAUDE.md` |
| safe_mem source | `cross/lib/safe_mem.c` |
