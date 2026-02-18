# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-18 (session 76)
**Status**: ~161 bundles shipping. Dillo web browser with working HTTPS on IRIX. GTK3, FLTK apps all functional.

---

## Critical Warnings

**NEVER install packages directly to /usr/sgug on the live IRIX system.** Always use `/opt/chroot` for testing.

**NEVER use raw SSH to IRIX.** Always use MCP tools (`irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`) or `mogrix-test` MCP tools.

**NEVER CHEAT.** Don't copy from SGUG-RSE. Don't fake Provides. Every FC40 package must be properly cross-compiled.

**CHECK USEFULNESS FIRST.** Before porting a package, verify it will actually work on IRIX. See `rules/methods/before-you-start.md` section 0.

**SYNC STAGING -> CHROOT.** After rebuilding any library, compare sizes and redeploy to chroot. Mismatched versions cause silent SIGABRT.

**ALWAYS use full mogrix pipeline** (`create-srpm` → `convert` → `build --cross`). Manual linking/ad-hoc compilation introduces dirty state.

**IRIX interactive testing**: Write instructions to `~edodd/instructions.txt` on the IRIX host (NOT chroot). User stores results in `~edodd/output.txt`. Use `irix_host_exec cp` from chroot path to host path.

---

## Current State

### Dillo 3.0.5: FULLY WORKING with HTTPS (session 76)

User-confirmed browsing HTTPS sites (google.com, news.ycombinator.com, mogrix.unxmaal.com) on IRIX. Three fixes required:

1. **SNI support** — Dillo 3.0.5 never sent SNI. Added `SSL_set_tlsext_host_name()` before `SSL_connect()`. Without this, CDN-hosted sites return wrong cert or reject.
2. **DPI daemon path fix** — dpid couldn't find DPI plugins (hardcoded `/usr/sgug/lib32/dillo/dpi/`). Bundle wrapper now creates `~/.dillo/dpidrc` pointing to bundle's DPI dir, plus `~/.dillo/dpid` wrapper script with `LD_LIBRARYN32_PATH` (dillo fork+exec's dpid directly, bypassing bundle wrapper).
3. **SSL cert loading** — Replaced hardcoded `/etc/ssl/certs/` dir with `SSL_CERT_FILE`/`SSL_CERT_DIR` env vars + `SSL_CTX_set_default_verify_paths()` fallback. Bundle sets `SSL_CERT_FILE`.

Also: static FLTK linking (avoids C++ vtable R_MIPS_REL32 SIGSEGV), `-fcommon`, auto-generated `dillorc` with IRIX bitmap fonts.

All fixes stored in rules: `rules/packages/dillo.yaml`, `patches/packages/dillo/dillo.irixfixes.patch`, `mogrix/bundle.py`.

### Test harness: deploy path changed

`tools/mogrix-test-server.py` now deploys bundles to `/usr/people/edodd/apps/` instead of `/tmp/mogrix-test/`. **Restart the MCP server for this to take effect.**

### Tool: `mogrix check-elf`

Detects R_MIPS_REL32 relocation issues BEFORE deploying to IRIX. Run on any Xt/Motif package after build.

```bash
uv run mogrix check-elf ~/mogrix_outputs/RPMS/<package>.rpm
uv run mogrix check-elf --generate-fix <rpm>
```

---

## Next Steps

1. **Build more packages** — check `packages_plan.md` for candidates
2. **Build more GTK3 apps** — bsearch fix is generic, GTK3 apps should work now
3. **Run `mogrix check-elf`** on any new Xt/Motif package before deploying to IRIX
4. **check-elf plan** exists in `.claude/plans/` — implements `--generate-fix` scaffold
5. **Investigate dash SIGSEGV, rsync SIGABRT, cwebp/dwebp crashes**

### Dropped/Blocked

- dbus, dbus-glib, icu, cyrus-sasl (pointless on IRIX)
- fastfetch, htop (too many Linux-specific deps)
- mupdf (too many missing deps)
- gdb 14.2 (binary crashes)
- GraphicsMagick, SDL2 (stopped — too complex)

---

## Recent Work

### Session 76: Dillo + FLTK (web browser with HTTPS)

- Built FLTK 1.3.11 as upstream package (`rules/packages/fltk.yaml`, `specs/packages/fltk.spec`)
- Built Dillo 3.0.5 as upstream package with OpenSSL support
- Fixed C++ vtable R_MIPS_REL32 SIGSEGV via static FLTK linking (`-Wl,-Bstatic`)
- Added SNI, SSL cert path flexibility, dpid bundle support to `dillo.irixfixes.patch`
- Added dillo-specific wrapper logic to `mogrix/bundle.py` (dpidrc, dpid wrapper, dillorc)
- Changed test harness deploy path from `/tmp/mogrix-test` to `~edodd/apps/`
- Added 5 new patterns to `rules/INDEX.md` (C++ vtable, -fcommon, fltk-config, fonts, SNI, dpid)

### Session 75: gtkterm Working + bsearch Fix + Bundler Fixes

- Discovered IRIX libc bsearch() crashes on nmemb=0 — created `compat/stdlib/bsearch.c`
- Built `libmogrix_compat.so` preloaded via `_RLDN32_LIST` (IRIX rld doesn't preempt exe symbols)
- Fixed 3 bugs in `mogrix/bundle.py`: dep resolution, soname symlinks, pruner chains
- gtkterm verified running on IRIX — GTK3 UI fully functional

### Session 74: xnedit SIGBUS/SIGSEGV Fixed + check-elf Tool

- Fixed SIGBUS (misaligned pointer) and SIGSEGV (NULL _XtInherit in ClassRec)
- Built `mogrix check-elf` tool: `mogrix/analyzers/elf.py`, CLI in `mogrix/cli.py`

---

## Reference

| What | Where |
|------|-------|
| Generic rules summary | `rules/GENERIC_SUMMARY.md` (read this first) |
| Problem lookup | `rules/INDEX.md` (grep, don't read whole file) |
| Test harness | `tools/mogrix-test-server.py` + `.mcp.json` |
| ELF relocation checker | `mogrix/analyzers/elf.py` — run `mogrix check-elf` |
| Package plan & tiers | `packages_plan.md` |
| Agent orchestration | `rules/methods/task-tracking.md` |
| Package rules | `rules/packages/<name>.yaml` |
| Build methods | `rules/methods/*.md` |
| Compat functions | `compat/catalog.yaml` |
| Workspace paths & workflow | `CLAUDE.md` |

---

## Post-Compaction Checklist (READ THIS FIRST)

1. **Mogrix invocation**: `uv run mogrix <command>` — not `mogrix`, not `python -m mogrix`
2. **Grep INDEX.md** before attempting ANY fix — the answer is probably already there
3. **Read GENERIC_SUMMARY.md** before starting any new package
4. **Paths are different things**:
   - `/opt/sgug-staging/` = cross-compilation sysroot (on Linux build host)
   - `/opt/chroot` = IRIX test chroot (on IRIX host, accessed via MCP tools)
5. **IRIX access**: Use MCP tools. NEVER raw SSH.
6. **Testing**: Use `mogrix-test` MCP tools. NOT ad-hoc irix_exec calls.
7. **Compat functions**: Grep `compat/catalog.yaml` before writing a new one.
8. **Always use full mogrix pipeline** for builds — manual compilation creates dirty state.
9. **Deploy irix-ld after editing**: `cp cross/bin/irix-ld /opt/sgug-staging/usr/sgug/bin/irix-ld`
10. **GSettings schemas**: Run `glib-compile-schemas` in chroot after installing GTK3 apps.
11. **Run `mogrix check-elf`** on Xt/Motif packages after build to catch ClassRec relocation issues.
