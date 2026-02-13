# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-12 (session 31)
**Status**: 96+ source packages cross-compiled (270+ RPMs). 13 bundles rebuilt and verified on IRIX. 536 tests, 534 passed (99.6%).

---

## Critical Warnings

**NEVER install packages directly to /usr/sgug on the live IRIX system.** Always use `/opt/chroot` for testing.

**NEVER use raw SSH to IRIX.** Always use MCP tools (`irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`).

**NEVER CHEAT.** Don't copy from SGUG-RSE. Don't fake Provides. Every FC40 package must be properly cross-compiled.

---

## Current State

All 13 bundles rebuilt from scratch and tested on IRIX. No active blockers.

**Bundles deployed on IRIX** (`/usr/people/edodd/apps/`):

| Bundle | Key Apps | Tests |
|--------|----------|-------|
| mogrix-smallweb | telescope, gmi100, lynx, snownews | 34/34 |
| mogrix-essentials | nano, grep, sed, gawk, less, coreutils, findutils, diffutils, tar, tree | 134/134 |
| mogrix-net | curl, rsync, gnupg2 | 41/41 |
| mogrix-fun | cmatrix, figlet, sl | 16/16 |
| weechat | IRC client (TLS) | 19/19 |
| groff | Document formatter | 12/12 |
| st | Suckless terminal (Xft + Iosevka font) | 7/7 |
| bitlbee | IRC gateway + discord plugin | 6/6 |
| telescope | Gemini browser | 6/6 |
| gmi100 | Gemini client | 6/6 |
| lynx | Web browser | 6/6 |
| snownews | RSS reader | 6/6 |
| tinc | VPN daemon | 8/6 (2 non-critical fails) |

**2 non-critical test failures**: groff/gdiffmk (needs diff3 in PATH), bitlbee/gapplication (glib2 utility SIGSEGV).

---

## Next Steps

**Engine improvements needed** (from full-rebuild evaluation):
1. **`drop_subpackages` should handle `%post/%postun` scriptlets** — currently leaves orphaned scriptlets for dropped subpackages (cmatrix workaround: spec_replacements)
2. **Emitter should inject compat Sources outside conditionals** — rsync compat sources land inside dead `%if` branch (workaround: spec_replacement re-injection)
3. **ncurses post-stage .pc cleanup** — build-time `-L.../mogrix-compat` paths baked into staged .pc files. Fixed manually but no rule prevents recurrence.

**Redundant per-package rules to clean up** (now handled by generic.yaml):
- xz, fontconfig, gettext, gnutls: `%files -f *.lang` spec_replacements (generic skip_find_lang handles them)
- Several packages: locale-related spec_replacements (generic install_cleanup removes locale dir)

**Pending ultralist tasks:**
- **#37**: Build cairo (meson, deps: pixman+freetype+fontconfig+libpng — all ready)
- **#38**: Build pango (meson, deps: cairo+harfbuzz+fribidi+fontconfig)
- **#46**: Build qtermwidget5 + qterminal (Qt5 verified on IRIX)

**Optional builds (no ultralist task yet):**
- gmid (Gemini server), gophernicus (Gopher server)
- vim, tmux, man-db, jq
- Ship bitlbee + weechat + st tarballs to community

---

## Recent Work

### Session 31: Full Rebuild of All 13 Bundles

**Rebuilt all packages in 6 tiers** (0-5) from scratch using `mogrix batch-build`. Fixed 12 build failures discovered during Tier 4:

**Generic fixes (promoted to generic.yaml):**
- `strnlen` added to inject_compat_functions (unblocked 5+ packages)
- Locale cleanup added to install_cleanup
- `skip_find_lang: true` added (all packages, no locale files shipped)

**Platform discoveries (new cross/compat headers):**
- C++ `va_list` fix in `stdarg.h` — IRIX declares `va_list` as `char*`, C++ needs matching type
- Static `select()` wrapper in `sys/time.h` — IRIX provides static select() causing duplicate symbols in static archives
- `SIG_ATOMIC_MAX/MIN` in compat `stdint.h` — gnulib include_next chain loses these definitions
- Bundle `dirname` fork bomb — wrappers must use `/bin/dirname` (absolute paths) to avoid infinite loops

**Engine fixes:**
- `spec.py`: `remove_conditionals` now runs before Source injection (fixes nettle)
- `batch.py`: Now passes `export_vars`, `skip_find_lang`, `skip_check`, `install_cleanup`, `spec_replacements` to writer (was silently ignoring these rules)

**Per-package fixes**: figlet (alloca, bash-ism, prefix, LD=), sl (direct compile), cmatrix (setenv, AC_CHECK_FILES, %post scriptlets), gnupg2/libgcrypt/libassuan/libksba (--with-lib*-prefix), rsync (compat source loop), findutils (gnulib stdint.in.h), gmp (FIPS/assembly), p11-kit (new prep_commands), fontconfig (symlink, locale), nettle (remove_conditionals), xz (--disable-nls), popt (locale cleanup)

**Results**: All 13 bundles generated. 536 IRIX tests, 534 passed (99.6%).

### Session 30: Housekeeping

- Cleaned up dead Claude Code task IDs from HANDOFF.md
- Updated plan.md status (91 → 96+ packages)
- Merged 3 memory files into rules/INDEX.md (11 new invariant rows)
- Slimmed MEMORY.md from 94 → 52 lines

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
