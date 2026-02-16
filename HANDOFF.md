# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-16 (session 53)
**Status**: GTK3 3.24.41 BUILT and staged. All batch #201 dependencies complete. irix-cc now handles meson response files.

---

## Critical Warnings

**NEVER install packages directly to /usr/sgug on the live IRIX system.** Always use `/opt/chroot` for testing.

**NEVER use raw SSH to IRIX.** Always use MCP tools (`irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`).

**NEVER CHEAT.** Don't copy from SGUG-RSE. Don't fake Provides. Every FC40 package must be properly cross-compiled.

**CHECK USEFULNESS FIRST.** Before porting a package, verify it will actually work on IRIX. See `rules/methods/before-you-start.md` section 0.

---

## Current State

### Batch 201 GTK3 Stack: COMPLETE

All components built and staged to `/opt/sgug-staging`:
- atk, gdk-pixbuf2, libepoxy (with GLX), xorgproto, libXfixes
- libXcomposite, libXcursor, libXdamage, libXi, libXinerama, libXrandr
- **GTK3 3.24.41** — 4 RPMs: gtk3, gtk3-devel, gtk3-immodules, gtk-update-icon-cache

### irix-cc response file expansion (this session)

`cross/bin/irix-cc` now expands meson `@file` response files before processing flags. This was required for GTK3's large link commands where `-Wl,` flags and `-shared` were inside the file. Deployed to staging. See INDEX.md "Meson Cross-Builds" section.

### Xge.h expanded (this session)

`patches/packages/libXi/Xge.h` now includes inline stubs for `XGetEventData`, `XFreeEventData`, and `XSetIMValues` declaration. Installed at `/opt/sgug-staging/usr/sgug/include/X11/extensions/Xge.h`.

### Native glib tools provisioned

GTK3 meson build requires native (x86_64) tools. Extracted from Ubuntu debs without root:
- `glib-compile-resources` → `/tmp/glib-dev-bin/usr/bin/` (symlinked from `cross/native-tools/`)
- `gdbus-codegen` → `cross/native-tools/` (patched Python module path)
- `xmllint` → `/tmp/xmllint-pkg/usr/bin/` (symlinked from `cross/native-tools/`)

**WARNING**: `/tmp/` contents are ephemeral. If cleared, re-extract with:
```
apt-get download libglib2.0-dev-bin && dpkg-deb -x libglib2.0-dev-bin*.deb /tmp/glib-dev-bin
apt-get download libxml2-utils && dpkg-deb -x libxml2-utils*.deb /tmp/xmllint-pkg
```

---

## Next Steps

1. **Build #205 GUI apps** — hexchat, geany, pidgin, nedit (GTK3 is now available!)
2. **Build #204 IPC/heavy** — dbus, dbus-glib, icu, cyrus-sasl
3. **Build #203 remaining** — fossil, mercurial
4. **Test GTK3 on IRIX** — copy RPMs to chroot, verify libgtk-3.so loads
5. **Low priority**: `drop_buildrequires_if_unavailable` engine feature

### Batch Progress

| Batch | Status |
|-------|--------|
| #191–#200 | COMPLETED |
| #201 GTK3 stack | **DONE** |
| #202 Build tools | DONE (gdb skipped) |
| #203 Dev tools pt2 | re2c/yasm/quilt DONE; fossil/mercurial PENDING |
| #204 IPC/heavy | PENDING |
| #205 GUI apps | PENDING — can start now (GTK3 available) |

### Blocked/Skipped Packages

| Package | Reason |
|---------|--------|
| gdb 14.2 | No IRIX native debug support in this version |
| ksh | ~3400-line custom build system |
| htop | Needs Linux /proc backend |
| openjpeg2 | No SRPM available |

---

## Recent Work

### Session 53: GTK3 built (continuation of 52)

- Built GTK3 3.24.41 after 12 iterations fixing: XSetIMValues missing, xcookie union member (5 files), atk-bridge.h, sincos in tests, meson response file expansion, demo/example file list cleanup
- Key fixes: `irix-cc` response file expansion, expanded `Xge.h` with XGetEventData/XFreeEventData/XSetIMValues stubs, disabled demos/examples in meson
- Updated `rules/INDEX.md` with "Meson Cross-Builds" section

### Session 52: GTK3 stack dependencies + GTK3 prep

- Built 6 X11 extension libraries (libXcomposite, libXcursor, libXdamage, libXi, libXinerama, libXrandr)
- Rebuilt libepoxy with GLX support (`-Dglx=yes -Dx11=true`)
- Provisioned native glib tools for meson cross-builds
- Created comprehensive `rules/packages/gtk3.yaml` with ~160 lines of rules

### Session 51: GDB investigation + skip decision

- Investigated GDB 14.2 — no IRIX native debug support in this version
- Added `-z norelro` to irix-ld, `gl_cv_malloc_ptrdiff=yes` pattern

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
| SGUG RSE reference | `/home/edodd/projects/github/sgug-rse` |
