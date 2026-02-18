# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-18 (session 74)
**Status**: ~161 bundles shipping. xnedit fully working (3 bugs fixed). New `mogrix check-elf` tool detects R_MIPS_REL32 issues before deploying to IRIX.

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

### xnedit 1.6.1: FULLY WORKING

Three bugs fixed across sessions 72-74. All in `rules/packages/xnedit.yaml` + `patches/packages/xnedit/fix_class_recs.c`:

1. **BadMatch on X_CreateWindow** (session 72) — forced default 24-bit visual (IRIX offers 30-bit TrueColor that crashes Xft)
2. **SIGBUS in create_image** (session 74) — `const char*` cast to `uint32_t*` for icon data, misaligned on MIPS. Fixed with memcpy. Also added `__mips` to big-endian `#if` check.
3. **SIGSEGV at PC=0 in VertLayout** (session 74) — `xmlGridClassRec.grid_class.preLayoutProc` initialized to `_XtInherit` but rld didn't resolve it. Fixed in fix_class_recs.c via pointer arithmetic to reach custom class part.

Clean build at `xnedit-1.6.1-1-irix-bundle.0218260013.run`. File > Save As, File > Open all working.

### New Tool: `mogrix check-elf`

Detects R_MIPS_REL32 relocation issues BEFORE deploying to IRIX. Finds all `*ClassRec` symbols in MIPS ELF binaries and cross-references with R_MIPS_REL32 relocations targeting UNDEF symbols.

```bash
uv run mogrix check-elf ~/mogrix_outputs/RPMS/xnedit-1.6.1-1.mips.rpm   # finds 8 ClassRecs, 59 at-risk relocs
uv run mogrix check-elf --generate-fix <rpm>                              # scaffold fix_class_recs.c
```

Files: `mogrix/analyzers/elf.py`, CLI in `mogrix/cli.py`. Also checks TEXTREL and 3-LOAD-segment shared libs.

### gtkterm/VTE: Still Crashes

93/96 tests pass. VTE widget init crash remains. NOT related to pixman/TLS/rendering.

---

## Next Steps

1. **Run `mogrix check-elf` on any new Xt/Motif package** before deploying to IRIX
2. **Debug VTE crash** — possible causes: pty creation, GnuTLS init, VTE C++ internals
3. **Find more buildable packages** — check `packages_plan.md`
4. **Investigate dash SIGSEGV, rsync SIGABRT, cwebp/dwebp crashes**

### Dropped/Blocked

- dbus, dbus-glib, icu, cyrus-sasl (pointless on IRIX)
- fastfetch, htop (too many Linux-specific deps)
- mupdf (too many missing deps)
- gdb 14.2 (binary crashes)
- GraphicsMagick, SDL2 (stopped — too complex)

---

## Recent Work

### Session 74: xnedit SIGBUS/SIGSEGV Fixed + check-elf Tool

- Diagnosed SIGBUS via SIGBUS handler → PC in `create_image()` at misaligned address 0x10770d1
- Fixed alignment with memcpy, added `__mips` endianness check
- Diagnosed SIGSEGV via SIGSEGV+$ra handler → `VertLayout()` calling NULL `preLayoutProc`
- Fixed `xmlGridClassRec.grid_class.preLayoutProc` via pointer arithmetic in fix_class_recs.c
- Also patched Tree class grid_class fields (6 `_XtInherit` references)
- Built `mogrix check-elf` tool to detect these proactively — validated against xnedit (finds all 8 ClassRecs) and nano (zero false positives)

### Session 73: Batch Build Wave — 7 Packages

Built screen, lua, feh, vim, scrot, cmake, p11-kit. All bundled and tested. INDEX.md updated with 10 new patterns.

### Session 72: xnedit BadMatch Fixed

FindBestVisual() picks 30-bit TrueColor visual on SGI. Xft/Xrender libs cause BadMatch at depth 30. Fix: force default 24-bit visual.

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
