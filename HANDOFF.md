# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-15 (session 42)
**Status**: 116+ source packages cross-compiled (310+ RPMs). All 35 bundle packages rebuilt with dlmalloc spin locks + high-fd fix. 20 bundles (.run installers) created. Essentials, bash, and extras verified on IRIX.

---

## Critical Warnings

**NEVER install packages directly to /usr/sgug on the live IRIX system.** Always use `/opt/chroot` for testing.

**NEVER use raw SSH to IRIX.** Always use MCP tools (`irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`).

**NEVER CHEAT.** Don't copy from SGUG-RSE. Don't fake Provides. Every FC40 package must be properly cross-compiled.

---

## Current State

### Session 42: Alpine email client (completed)

**Completed this session** (built, MIPS N32 verified, staged, copied to outputs):
- **alpine** 2.26 — console email client with built-in IMAP c-client library.
  11 distinct build issues resolved iteratively. Key fixes in `rules/packages/alpine.yaml`:
  - CC_FOR_BUILD LIBS cross-contamination (sed $LIBS out of ac_link)
  - IRIX case added to host detection in configure
  - c-client target slx with flocksim.c instead of flocklnx.c (IRIX has no fstatfs)
  - scandir/alphasort prototypes guarded with `#if !_SGIAPI` (not `#ifndef` -- _SGIAPI is a macro expression)
  - HOSTCC pattern for help_h_gen/help_c_gen code generators
  - vfork -> fork define (pith/osdep/pipe.c), %zu -> %u, mkdtemp compat
  - os_slx.h patched with flocksim.h + ustat.h includes

### Session 41: Batch building

**Completed** (built, MIPS N32 verified, staged, copied to outputs):
- **recode** 3.7.14 — character set converter. -lgen for dirname, Python/NLS disabled.
- **mpg123** 1.32.6 — MPEG audio player. Dummy output only, network disabled.
- **lame** 3.100 — MP3 encoder. -ltinfo for termcap, nasm/ix86 blocks removed.
- **libpsl** 0.21.5 — Public Suffix List library. libidn2 backend (no ICU), bundled PSL data.
- **opus** 1.5.1 — audio codec. In-tree build (no pushd/popd), libtool IRIX fix.
- **libssh2** 1.11.0 — SSH2 client library. openssl+zlib backend, libtool fix.
- **lcms2** 2.16 — Color management engine. Meson build, replaced %meson macros with raw meson commands.
- **imlib2** 1.11.1 — Image rendering library. Autotools, memmem compat, CLOCK_MONOTONIC disabled.
- **mksh** 59c — MirBSD Korn Shell. Custom Build.sh, TARGET_OS=IRIX. Verified on IRIX.
- **libao** 1.2.0, **libsndfile** 1.2.2, **libcaca** 0.99-beta20, **ksh** 1.0.8 — agent-built.

### Batch Progress

| Batch | Task | Status | Details |
|-------|------|--------|---------|
| #191 Quick-win CLI | COMPLETED | screen, zsh, ed, dtach, pwgen, rlwrap, units, diffstat, most |
| #192 Fun/trivial | COMPLETED | cowsay, banner, neofetch, screenfetch, boxes, sl |
| #193 Sysadmin | COMPLETED | dos2unix, ncdu, dash, lrzsz |
| #194 Dev tools pt1 | COMPLETED | ctags, patchutils, enscript, mandoc, help2man, recode |
| #195 Alt shells | IN_PROGRESS | mksh DONE (verified on IRIX), ksh checking build sys |
| #196 Foundation libs | IN_PROGRESS | slang/jansson/libyaml/libev done; json-c BLOCKED(cmake); libcaca building |
| #197 Compression/net | COMPLETING | lz4/lzo done; brotli BLOCKED(cmake); libpsl+libssh2 DONE this session |
| #199 Audio | IN_PROGRESS | libogg/libvorbis/flac done; opus+lame+mpg123 DONE this session; libao+libsndfile building |
| #198 User apps | IN_PROGRESS | alpine DONE; mc, irssi, joe, vile, frotz pending |
| #200 Image libs | IN_PROGRESS | libtiff done; lcms2+imlib2 DONE this session; libwebp BLOCKED(cmake); openjpeg2 BLOCKED(cmake) |
| #201 GTK3 stack | PENDING | blocked on image libs |
| #202 Build tools | PENDING | cmake, ninja, meson, gdb, doxygen |
| #203 Dev tools pt2 | PENDING | fossil, mercurial, quilt, re2c, yasm |
| #204 IPC/heavy | PENDING | dbus, dbus-glib, icu, cyrus-sasl |
| #205 GUI apps | PENDING | hexchat, geany, pidgin, nedit |

### Blocked Packages

| Package | Reason |
|---------|--------|
| json-c | cmake build system |
| brotli | cmake build system |
| libwebp | cmake build system |
| ninja-build | cmake build system |
| SDL2 | cmake build system |
| p11-kit | meson build system (now unblocked — lcms2 proved meson works) |
| htop | needs IRIX /proc backend |

---

## Previous State (session 40)

**All bundles rebuilt**: 35 packages rebuilt, 20 bundles created with hardened dlmalloc.
**Deployed on IRIX**: mogrix-essentials, bash, mogrix-extras verified.
**Trampoline exclusions working**: bash bundle correctly excludes sh and builtins.

---

## Reference

| What | Where |
|------|-------|
| Technical patterns & fixes | `rules/INDEX.md` |
| Package rules | `rules/packages/<name>.yaml` |
| Project architecture | `plan.md` |
| Build methods | `rules/methods/*.md` |
| Compat functions | `compat/catalog.yaml` |
| Workspace paths & workflow | `CLAUDE.md` |
| dlmalloc test suite | `tests/dlmalloc-test.c` |
