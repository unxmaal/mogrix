# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-14 (session 37, continued)
**Status**: 100+ source packages cross-compiled (280+ RPMs). Weechat TLS WORKING on IRIX. All 23 bundles rebuilt with dlmalloc fix + gnutls CA trust store. wget2 strnlen fix done.

---

## Critical Warnings

**NEVER install packages directly to /usr/sgug on the live IRIX system.** Always use `/opt/chroot` for testing.

**NEVER use raw SSH to IRIX.** Always use MCP tools (`irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`).

**NEVER CHEAT.** Don't copy from SGUG-RSE. Don't fake Provides. Every FC40 package must be properly cross-compiled.

---

## Current State

**Weechat TLS**: SOLVED. Connected to irc.libera.chat:6697 on real IRIX hardware. See session 37 notes for the dlmalloc fix.

**dlmalloc architecture change**: dlmalloc is now linked into executables only (via `irix-ld`). Shared libraries leave malloc/free undefined so rld resolves them to the executable's single allocator. This eliminates cross-heap corruption when library A allocates and library B frees.

**All bundles rebuilt** (session 37 continued): All 23 bundles (16 individual + 5 suites + weechat + gmi100) re-bundled with dlmalloc-free shared libraries. gnutls rebuilt with `--with-default-trust-store-file`. wget2 rebuilt with strnlen compat fix. Key bundles deployed to IRIX host.

**Bundles deployed on IRIX** (`/usr/people/edodd/apps/`):

| Bundle | Key Apps | Tests |
|--------|----------|-------|
| mogrix-smallweb | telescope, gmi100, lynx, snownews | 34/34 |
| mogrix-essentials | nano, grep, sed, gawk, less, coreutils, findutils, diffutils, tar, tree | 134/134 |
| mogrix-net | curl, rsync, gnupg2 | 41/41 |
| mogrix-fun | cmatrix, figlet, sl | 16/16 |
| weechat | IRC client (TLS working!) | 19/19 + TLS verified |
| groff | Document formatter | 12/12 |
| st | Suckless terminal (Xft + Iosevka font) | 7/7 |
| bitlbee | IRC gateway + discord plugin | 6/6 |
| telescope | Gemini browser | 6/6 |
| gmi100 | Gemini client | 6/6 |
| lynx | Web browser | 6/6 |
| snownews | RSS reader | 6/6 |
| tinc | VPN daemon | 8/6 (2 non-critical fails) |
| man-db | man, whatis, mandb, lexgrog | NEW — not yet in a deployed bundle |

---

## Next Steps

1. **Test X11 bundles from real terminal**: dmenu, rxvt-unicode, st
2. **Clean up old bundles on IRIX**: Remove stale weechat bundles (0214260152, 0214260152-real, 0213262143, 0213262214, old tmux/vim bundles)
3. **Pending tasks:**
   - Build qtermwidget5 + qterminal
   - Add convert-time lint for duplicated rules
   - Implement mogrix elevate command
   - htop: BLOCKED (needs IRIX /proc platform backend)
4. **Low priority**: apropos segfault

---

## Recent Work

### Session 37: SOLVED weechat TLS ABORT — dlmalloc cross-heap corruption

**Root cause**: `-Bsymbolic-functions` + dlmalloc linked into every .so = each shared library gets its own private malloc heap. When library A allocates memory and library B calls free() on it, dlmalloc detects a foreign pointer and calls abort(). This is exactly what happens during TLS: gnutls allocates, nettle frees (or vice versa).

**Fix — dlmalloc moved to executables only**:
- `mogrix/rules/engine.py` — removed automatic dlmalloc injection into all builds; actively strips dlmalloc from link commands
- `cross/bin/irix-ld` — now adds `dlmalloc.o` only when linking executables (not shared libraries). Pre-compiled object at `/opt/sgug-staging/usr/sgug/lib32/dlmalloc.o`
- `cross/crt/crtbeginS.S` — `.hidden` on `__do_global_ctors_aux`, `_init`
- `cross/crt/crtendS.S` — `.hidden` on `__CTOR_END__`, `__DTOR_END__`
- `rules/packages/gnutls.yaml` — added `--with-default-trust-store-file=/usr/sgug/etc/pki/tls/certs/ca-bundle.crt`

**Batch rebuild**: All 12 shared library packages rebuilt without dlmalloc in .so files: libgpg-error, gmp, libtasn1, libunistring, xz, zstd, ncurses, nettle, gettext, openssl, curl, gnutls. Then weechat rebuilt on top. Then ALL 23 bundles re-created. gnutls rebuilt again with CA trust store fix. wget2 rebuilt with strnlen compat.

**Result**: Weechat connected to irc.libera.chat:6697 with TLS on real IRIX hardware. The longest-standing bug is fixed.

**IRIX tar limitation**: IRIX native `/bin/tar` silently drops files with long paths (>100 chars). Use gtar from the chroot (`gtar xzf`) or pipe through host tar for short-path archives. For deployment: symlink from `/usr/people/edodd/apps/` to `/opt/chroot/tmp/<bundle>` when host tar fails.

### Session 36: Bundle testing + weechat TLS debugging

**Chroot curl fix**: Chroot had stale libcurl linked against IRIX OpenSSL 0.9.7 (not our 3.x). Force-reinstalled correct RPM.

**TLS loopback test**: `gnutls_loopback.c` — full TLS 1.3 handshake passes in chroot. Bundle still ABORTed on host during real TLS, which led to the dlmalloc discovery in session 37.

**Chroot bundle tests**: vim, tmux, jq, bc, man-db pass. wget2 fails (missing strnlen).

### Session 35: weechat gnutls + bundle fixes

**gnutls priority fix** (`rules/packages/gnutls.yaml`): Removed `--with-system-priority-file`, changed default to "NORMAL".

**Bundle wrapper fix** (`mogrix/bundle.py`): LD_LIBRARYN32_PATH set fresh (not appended). Tarball format changed to ustar (later: removed `-h` flag to fix broken symlink errors in groff/dmenu bundles).

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
