# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-18 (session 75)
**Status**: ~161 bundles shipping. gtkterm now runs successfully from self-contained bundle on IRIX. GTK3 apps unblocked.

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

### gtkterm: FULLY WORKING (session 75)

Screenshot-confirmed running on IRIX from self-contained bundle. GTK3 UI, menus, and VTE terminal widget all rendering correctly.

Key fixes this session:
1. **IRIX bsearch crash (GTK3 blocker)** — IRIX libc bsearch() crashes when nmemb=0 (underflows nmemb-1 to 0xFFFFFFFF). Root cause of GTK3 SIGSEGV in `_gtk_style_context_peek_style_property` → bsearch on empty property_cache. Created `compat/stdlib/bsearch.c` (POSIX-compliant replacement), added to generic.yaml `inject_compat_functions` so ALL packages get it.
2. **IRIX rld does NOT preempt exe symbols** — Linking bsearch.o into the executable is insufficient. IRIX rld resolves shared lib calls through NEEDED chains only, never checking the executable's .dynsym (unlike Linux). Fix: built `libmogrix_compat.so` (in staging at `/usr/sgug/lib32/`), preloaded via `_RLDN32_LIST=libmogrix_compat.so:DEFAULT` in bundle wrappers. Bundler auto-includes this .so and sets the env var.
3. **Bundler zlib issue (3 bugs in bundle.py)** — resolve_deps() now checks mogrix-built RPMs BEFORE IRIX sysroot; new `_create_soname_symlinks()` creates missing unversioned .so symlinks; fixed `_prune_unused_libs()` to walk symlink chains step-by-step.

### xnedit 1.6.1: FULLY WORKING (session 74)

Three bugs fixed in sessions 72-74. All in `rules/packages/xnedit.yaml` + `patches/packages/xnedit/fix_class_recs.c`:
1. **BadMatch on X_CreateWindow** — forced default 24-bit visual
2. **SIGBUS in create_image** — misaligned `uint32_t*` cast, fixed with memcpy
3. **SIGSEGV at PC=0 in VertLayout** — ClassRec `_XtInherit` not resolved by rld, fixed via pointer arithmetic

### Tool: `mogrix check-elf`

Detects R_MIPS_REL32 relocation issues BEFORE deploying to IRIX. Run on any Xt/Motif package after build.

```bash
uv run mogrix check-elf ~/mogrix_outputs/RPMS/<package>.rpm
uv run mogrix check-elf --generate-fix <rpm>
```

---

## Next Steps

1. **Build more GTK3 apps** — bsearch fix is generic, GTK3 apps should work now
2. **Run `mogrix check-elf` on any new Xt/Motif package** before deploying to IRIX
3. **check-elf plan** exists in `.claude/plans/` — wasn't worked on this session
4. **Find more buildable packages** — check `packages_plan.md`
5. **Investigate dash SIGSEGV, rsync SIGABRT, cwebp/dwebp crashes**

### Dropped/Blocked

- dbus, dbus-glib, icu, cyrus-sasl (pointless on IRIX)
- fastfetch, htop (too many Linux-specific deps)
- mupdf (too many missing deps)
- gdb 14.2 (binary crashes)
- GraphicsMagick, SDL2 (stopped — too complex)

---

## Recent Work

### Session 75: gtkterm Working + bsearch Fix + Bundler Fixes

- Discovered IRIX libc bsearch() crashes on nmemb=0 (underflows to 0xFFFFFFFF)
- Created `compat/stdlib/bsearch.c`, added to `compat/catalog.yaml` and `rules/generic.yaml` inject_compat_functions
- Discovered IRIX rld does NOT preempt exe symbols for shared lib calls — built `libmogrix_compat.so` and preloaded via `_RLDN32_LIST`
- Fixed 3 bugs in `mogrix/bundle.py`: dep resolution priority, soname symlink creation, pruner symlink chain handling
- Added `_RLDN32_LIST` support to bundle wrapper templates + auto-include `libmogrix_compat.so`
- gtkterm verified running on IRIX via screenshot — GTK3 UI fully functional from self-contained bundle

### Session 74: xnedit SIGBUS/SIGSEGV Fixed + check-elf Tool

- Fixed SIGBUS (misaligned pointer in create_image) and SIGSEGV (NULL preLayoutProc via _XtInherit)
- Built `mogrix check-elf` tool to detect ClassRec relocation issues proactively
- Files: `mogrix/analyzers/elf.py`, CLI in `mogrix/cli.py`

### Session 73: Batch Build Wave — 7 Packages

Built screen, lua, feh, vim, scrot, cmake, p11-kit. All bundled and tested. INDEX.md updated with 10 new patterns.

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
