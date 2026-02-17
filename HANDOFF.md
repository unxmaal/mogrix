# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-17 (session 71)
**Status**: 154 bundles shipping. **Pixman TLS fix: cairo rendering fully works on IRIX.** GTK3 basic windows render correctly. gtkterm/VTE still crash during VTE widget initialization (separate issue from rendering).

---

## Critical Warnings

**NEVER install packages directly to /usr/sgug on the live IRIX system.** Always use `/opt/chroot` for testing.

**NEVER use raw SSH to IRIX.** Always use MCP tools (`irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`) or `mogrix-test` MCP tools.

**NEVER CHEAT.** Don't copy from SGUG-RSE. Don't fake Provides. Every FC40 package must be properly cross-compiled.

**CHECK USEFULNESS FIRST.** Before porting a package, verify it will actually work on IRIX. See `rules/methods/before-you-start.md` section 0.

**SYNC STAGING -> CHROOT.** After rebuilding any library, compare sizes and redeploy to chroot. Mismatched versions cause silent SIGABRT.

**ALWAYS use full mogrix pipeline** (`create-srpm` → `convert` → `build --cross`). Manual linking/ad-hoc compilation introduces dirty state.

---

## Current State

### GTK3 Rendering: FIXED

Cairo rendering on IRIX now works — paint, fill, stroke, arc, text all pass. Root cause was `__tls_get_addr` in pixman (IRIX has no TLS support). Fix in `rules/packages/pixman.yaml`: `-Dtls=disabled` + `PIXMAN_NO_TLS`.

Also disabled MIT-SHM in cairo as precaution (IRIX IPC_RMID semantics differ). Fix in `rules/packages/cairo.yaml`.

Verified with pure-cairo and GTK3 test programs on IRIX. See INDEX.md "GTK3/cairo rendering crash" entry.

### gtkterm/VTE: Crashes After GTK Init (NEW ISSUE)

GTK3 basic windows work. VTE terminal widget crashes during initialization (~2.5s in, well past gtk_init). NOT related to pixman/TLS/rld. Par trace shows crash after:
- Icon cache loading
- Cycle counter access (SGI_CYCLECNTR + /dev/mmem)
- Random seeding (/dev/urandom)
- Repeated getpid()/time() loop → SIGSEGV

Also discovered: **IRIX rld is sensitive to NEEDED library ordering** — VTE must come BEFORE GTK in the ELF NEEDED list. When VTE is after GTK, crash before main(). Gtkterm already has correct order (VTE first), so this isn't the current blocker.

### IRIX rld Library Ordering Bug

When VTE appears after GTK in NEEDED entries, crashes before main() during GLib's first memory allocation (g_slice_alloc). Same source code + same libraries work when VTE is listed first. Root cause unknown (not symbol conflicts, not SYMTABNO, not GOT overflow). Workaround: ensure VTE is linked before GTK.

### Key Blockers

| Package | Blocker |
|---------|---------|
| **gtkterm** (GUI) | VTE widget init crash (post-rendering-fix) |
| **xnedit** (GUI) | Needs rebuild with new pixman — may work now |
| **python3** | Boss fight — extensive patching needed |

---

## Next Steps

1. **Test xnedit GUI** — may work now that cairo rendering is fixed. Rebuild if needed.
2. **Debug VTE crash** — use the mogrix pipeline (NOT ad-hoc test binaries). Possible causes:
   - VTE pty creation (IRIX has `_getpty()` not `openpty()`)
   - GnuTLS initialization (crypto/random seeding)
   - VTE C++ internals accessing bad memory
3. **Rebuild gtkterm bundle** with new pixman to get updated test results
4. **Find buildable CLI packages** — 25 packages have rules but no RPMs. Check `packages_plan.md`.
5. **Investigate dash SIGSEGV, rsync SIGABRT, cwebp/dwebp crashes**

### Dropped/Blocked

- dbus, dbus-glib, icu, cyrus-sasl (pointless on IRIX)
- fastfetch, htop (too many Linux-specific deps)
- mupdf (too many missing deps)
- gdb 14.2 (binary crashes)

---

## Recent Work

### Session 71: Pixman TLS Fix + Cairo Rendering Works

**Root cause found**: `rld: Fatal Error: unresolvable symbol in libpixman-1.so.0: __tls_get_addr`. Pixman uses `__thread` TLS variables. Clang cross-compiles them fine, but IRIX rld can't resolve `__tls_get_addr` at runtime. Lazy resolution means library loading succeeds; crash on first TLS access.

**Fix**: `rules/packages/pixman.yaml` — `-Dtls=disabled` in meson + `#define PIXMAN_NO_TLS 1` → plain static variables.

**Also**: Disabled MIT-SHM in `rules/packages/cairo.yaml` (remove XShm.h checks from meson.build). Precautionary — IRIX IPC_RMID destroys segments immediately.

**Updated**: `rules/INDEX.md` — corrected GTK3 crash entry, added pixman TLS details.

**Discovered**: IRIX rld library ordering sensitivity. VTE must be before GTK in NEEDED. Added to INDEX.md.

### Session 70: VTE + gtkterm Built, rld Issues Solved

VTE 0.58.3 built. gtkterm 1.3.1 multi-session debugging: export_dynamic removal, libx11_xge_stubs.so for XGE, SYMTABNO limit discovery. All 51 libs load. See `rules/packages/gtkterm.yaml`, `compat/x11_xge_stubs.c`.

### Session 69: xnedit, packages_plan.md

xnedit 1.6.1 built. packages_plan.md rewritten (130+ apps, 21 suites).

---

## Reference

| What | Where |
|------|-------|
| Generic rules summary | `rules/GENERIC_SUMMARY.md` (read this first) |
| Problem lookup | `rules/INDEX.md` (grep, don't read whole file) |
| Test harness | `tools/mogrix-test-server.py` + `.mcp.json` |
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
