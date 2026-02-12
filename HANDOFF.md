# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-12 (session 30)
**Status**: 96+ source packages cross-compiled (270+ RPMs). 7 suite bundles shipped and verified on live IRIX. `mogrix test` command validates bundles automatically.

---

## Critical Warnings

**NEVER install packages directly to /usr/sgug on the live IRIX system.** Always use `/opt/chroot` for testing.

**NEVER use raw SSH to IRIX.** Always use MCP tools (`irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`).

**NEVER CHEAT.** Don't copy from SGUG-RSE. Don't fake Provides. Every FC40 package must be properly cross-compiled.

---

## Current State

All primary deliverables are working. No active blockers.

**Bundles deployed on IRIX** (`/usr/people/edodd/apps/`):

| Bundle | Size | Key Apps | Tests |
|--------|------|----------|-------|
| mogrix-smallweb | 11.1 MB | telescope, gmi100, lynx, snownews | 34/34 |
| mogrix-essentials | 24 MB | nano, grep, sed, gawk, less, coreutils, findutils, diffutils, tar, tree | 134/134 |
| mogrix-net | 20.8 MB | curl, rsync, gnupg2 | 41/41 |
| mogrix-fun | 0.6 MB | cmatrix, figlet, sl | 16/16 |
| weechat | 21.7 MB | IRC client (TLS verified on Libera.Chat) | — |
| st | 10.2 MB | Suckless terminal (Xft + Iosevka font) | — |
| bitlbee | — | IRC gateway + discord plugin | — |

**Key capabilities**: `mogrix fetch/convert/build/stage/bundle/test/create-srpm`, suite bundles, upstream git/tarball packages, MCP server v2.1.0 with `irix_host_exec`.

---

## Next Steps

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

### Session 30: Housekeeping

- Cleaned up dead Claude Code task IDs from HANDOFF.md
- Updated plan.md status (91 → 96+ packages)
- Merged 3 memory files into rules/INDEX.md (11 new invariant rows)
- Slimmed MEMORY.md from 94 → 52 lines
- Rewrote handoff command + precompact hook with 200-line limit and trim rules

### Session 29: `mogrix test` + Three New Bundles

**`mogrix test <bundle-dir>`** — automated bundle smoke testing on IRIX:
- Auto-generates `--version` test for every wrapper script
- Runs YAML `smoke_test` entries (23 packages have them)
- Detects: unresolved symbols, SIGSEGV, SIGBUS, missing data files
- All 4 suite bundles pass (225 total tests)
- Files: `mogrix/test.py` (new), `mogrix/cli.py` (added test command)

**bundle.py fixes:**
- Symlink crash in chmod loop (broken symlinks in `os.walk`)
- FIGLET_FONTDIR auto-detection for figlet font path

**Bundles created**: mogrix-fun, mogrix-essentials, mogrix-net (see table above)

### Session 28: Telescope Fixes + Tinc Guide

- Telescope `%zu` SIGSEGV — real fix (session 27 was false positive). Sed all `%zu/%zd` → `%u/%d`
- Telescope UTF-8 glyphs → ASCII equivalents for IRIX terminals
- libretls getentropy fix — stripped `if HOST_LINUX` conditional
- Bundle ownership (`--owner=0`), permissions (world-readable), tar format fixes
- Created `docs/tinc-irix-guide.txt` with full IRIX setup guide

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
