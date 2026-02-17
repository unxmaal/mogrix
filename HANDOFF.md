# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-17 (session 59)
**Status**: All 28 bundles rebuilt with fixed self-extracting template. GTK3 verified on IRIX.

---

## Critical Warnings

**NEVER install packages directly to /usr/sgug on the live IRIX system.** Always use `/opt/chroot` for testing.

**NEVER use raw SSH to IRIX.** Always use MCP tools (`irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`).

**NEVER CHEAT.** Don't copy from SGUG-RSE. Don't fake Provides. Every FC40 package must be properly cross-compiled.

**CHECK USEFULNESS FIRST.** Before porting a package, verify it will actually work on IRIX. See `rules/methods/before-you-start.md` section 0.

**SYNC STAGING -> CHROOT.** After rebuilding any library, compare sizes and redeploy to chroot. Mismatched versions cause silent SIGABRT. See INDEX.md "Staging/chroot library mismatch".

---

## Current State

### Bundles: All 28 rebuilt and working

Self-extracting `.run` bundles now use full absolute paths (`/sbin/tar`, `/sbin/mkdir`, `/usr/sbin/gzcat`, `/bin/tail`, `/bin/pwd`) to avoid PATH shadowing from DIDBS/SGUG-RSE on user's IRIX host. `cd "$dest"` instead of `-C` flag. All bundles in `~/mogrix_outputs/bundles/`.

**21 individual apps:** bash, bc, bitlbee, cmatrix, dmenu, gmi100, groff, jq, lynx, man-db, nano, rxvt-unicode, snownews, st, tcsh, telescope, tinc, tmux, vim-enhanced, weechat, wget2

**7 suites:**
- mogrix-essentials: nano grep sed gawk less coreutils findutils diffutils tar tree-pkg
- mogrix-extras: bc jq man-db tmux vim-enhanced
- mogrix-fun: cmatrix figlet sl
- mogrix-funtools: cowsay banner
- mogrix-clitools: dtach diffstat pwgen rlwrap ncdu
- mogrix-net: curl rsync gnupg2
- mogrix-smallweb: telescope gmi100 lynx snownews

### GTK3 on IRIX: WORKING

`test_gtk3` renders on IRIX 6.5 console. Runtime requirements for non-chroot GTK3 apps: `FONTCONFIG_FILE` override needed (see INDEX.md "FONTCONFIG_FILE for non-chroot apps").

### Native glib tools provisioned

GTK3 meson build requires native (x86_64) tools in `/tmp/` -- re-extract if cleared:
```
apt-get download libglib2.0-dev-bin && dpkg-deb -x libglib2.0-dev-bin*.deb /tmp/glib-dev-bin
apt-get download libxml2-utils && dpkg-deb -x libxml2-utils*.deb /tmp/xmllint-pkg
```

---

## Next Steps

1. **Build #205 GUI apps** -- hexchat, geany, pidgin, nedit (GTK3 verified)
2. **Build #204 IPC/heavy** -- dbus, dbus-glib, icu, cyrus-sasl
3. **Build #203 remaining** -- fossil, mercurial
4. **Rebuild remaining ~50 pre-fix libraries** in staging (exposed CRT, 3-LOAD segments from before fixes)
5. **Update `rules/methods/bundles.md`** -- document the self-extracting .run format (currently only covers .tar.gz)
6. **Low priority**: `drop_buildrequires_if_unavailable` engine feature

### Batch Progress

| Batch | Status |
|-------|--------|
| #191-#200 | COMPLETED |
| #201 GTK3 stack | DONE -- GUI verified working on IRIX |
| #202 Build tools | DONE (gdb skipped) |
| #203 Dev tools pt2 | re2c/yasm/quilt DONE; fossil/mercurial PENDING |
| #204 IPC/heavy | PENDING |
| #205 GUI apps | UNBLOCKED -- GTK3 verified |

### Blocked/Skipped Packages

| Package | Reason |
|---------|--------|
| gdb 14.2 | No IRIX native debug support in this version |
| ksh | ~3400-line custom build system |
| htop | Needs Linux /proc backend |
| openjpeg2 | No SRPM available |

---

## Recent Work

### Session 59: Bundle extraction fix + full rebuild

- **Root cause**: `.run` scripts used bare `tar`/`mkdir` without full paths. User's IRIX `.bash_profile` had ancient DIDBS environment (`/usr/didbs/current/bin` first in PATH), shadowing system commands. Bare `tar` resolved to DIDBS tar instead of `/sbin/tar`.
- **Fix**: `SELF_EXTRACTING_TEMPLATE` in `bundle.py` now uses `/sbin/tar`, `/sbin/mkdir`, `/usr/sbin/gzcat`, `/bin/tail`, `/bin/pwd`. Uses `cd "$dest"` instead of `-C` flag. `$0` resolved to absolute path before `cd`.
- **Also fixed**: Staging-only libs (libgcc_s, libstdc++) no longer trigger misleading "may need SGUG-RSE" warning.
- All 28 bundles (21 individual + 7 suites) rebuilt and verified.
- INDEX.md updated with "Shell scripts must use full paths" invariant.
- User removed ancient DIDBS `.bash_profile` from IRIX host.

### Session 58: GTK3 verified working + staging/chroot sync fix

- 12 libraries mismatched between staging and chroot: pango x4, cairo x2, gio, bz2, jpeg, pixman, zlib, intl. Redeployed all.
- test_gtk3 renders successfully. FONTCONFIG_FILE override for non-chroot apps.
- INDEX.md updated with staging/chroot mismatch pattern.

### Session 57: 3-LOAD-segment crash + PHDRS FILEHDR fix

- Fixed irix-shared.lds with PHDRS block (IRIX rld needs exactly 2 LOAD segments).
- Rebuilt 6 packages: pixman, bzip2, zlib-ng, pango, cairo, libjpeg-turbo.

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

---

## Post-Compaction Checklist (READ THIS FIRST)

You just woke up from a context compaction. You WILL get basic things wrong unless you re-anchor now.

1. **Mogrix invocation**: `uv run mogrix <command>` — not `mogrix`, not `python -m mogrix`
2. **Grep INDEX.md** before attempting ANY fix: `Grep pattern="keyword" path="rules/INDEX.md"` — the answer is probably already there
3. **Read GENERIC_SUMMARY.md** before starting any new package
4. **Paths are different things**:
   - `/opt/sgug-staging/` = cross-compilation sysroot (on Linux build host)
   - `/opt/chroot` = IRIX test chroot (on IRIX host, accessed via MCP tools)
   - These are NOT the same. Don't mix them up.
5. **Chroot is root-owned**. Everything in `/opt/chroot` is owned by root. Don't be surprised by permission errors.
6. **IRIX access**: Use MCP tools (`irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`). NEVER raw SSH.
7. **Compat functions**: Grep `compat/catalog.yaml` before writing a new one — it probably already exists.
