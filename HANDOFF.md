# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-21 (session 95)
**Status**: WebKit IPC debug logging built and bundled. Bundle ready to deploy to IRIX. Need to identify which `connectionDidClose()` path fires when MiniBrowser reports "WebProcess CRASHED".

---

## Post-Compaction Checklist (READ THIS FIRST)

1. **Mogrix invocation**: `uv run mogrix <command>` — not `mogrix`, not `python -m mogrix`
2. **Grep rules/INDEX.md** before attempting ANY fix — the answer is probably already there
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
12. **NEVER install packages directly to /usr/sgug on the live IRIX system.** Always use `/opt/chroot`.

---

## Current State

### WebKit IPC Debugging

**Bundle ready to deploy**: `~/mogrix_outputs/bundles/libwebkit2gtk-2.42.5-1-irix-bundle.0221260057.run` (103MB)

**What we know:**
- MiniBrowser window appears, but WebKitWebProcess "crashes" (IPC connection drops) after 25-47 seconds
- "WebProcess CRASHED" is misleading — WebProcess receives SIGTERM from MiniBrowser cleanup, NOT SIGSEGV/SIGBUS
- All IRIX IPC primitives work correctly (SOCK_STREAM, SCM_RIGHTS, poll, shm_open — all tested)
- GOT overflow is fixed (`-Bsymbolic`, global GOT 4661→3515)
- All 70+ libraries load successfully

**What's new in this bundle:** 12 IPC_LOG injection points in ConnectionUnix.cpp write to `/usr/people/edodd/ipc_<pid>.log`. Both MiniBrowser and WebProcess sides log. See INDEX.md "WebKit IPC Connection Debugging" section.

**To test:** Deploy bundle → run MiniBrowser → collect `ipc_*.log` files → identify which `CLOSE:*` path fired.

**Previous bundle on IRIX**: `/usr/people/edodd/apps/libwebkit2gtk-2.42.5-1-irix-bundle.0220262009/`
(has signal_catcher wrapper as WebKitWebProcess — must restore original binary or use new bundle)

### Key Artifacts

| What | Where |
|------|-------|
| IPC debug header | `patches/packages/webkitgtk/ipc_debug_log.h` |
| rld analysis harness | `tools/rld-harness.py` |
| rld decompilation | `docs/rld/` (4 files) |
| Signal catcher source | `/tmp/signal_catcher.c` (on build host) |
| IPC test program | `/tmp/test_ipc.c` (on build host) |

---

## Next Steps (prioritized)

1. **Deploy new bundle to IRIX and run MiniBrowser** — collect `ipc_<pid>.log` files from `/usr/people/edodd/`. These will show exactly which close path fires (EOF, ECONNRESET, GIO_HUP, MSG_CTRUNC, send error, etc.) and whether any messages were successfully exchanged first.

2. **Based on IPC logs, fix the root cause** — Likely scenarios:
   - `CLOSE:EOF` → WebProcess closed its socket (initialization failure, unhandled error)
   - `CLOSE:GIO_HUP` → Socket hangup (process died, fd leaked/closed)
   - `CLOSE:ECONNRESET` → Connection reset (OS-level issue)
   - `MSG_CTRUNC` → SCM_RIGHTS ancillary data truncated (buffer too small for FDs)
   - No CLOSE log at all → crash in unlogged code path (expand logging)

3. **Once IPC works**, remove temporary debug logging from webkitgtk.yaml.

4. **Fix 5 remaining bundle failures** (independent of MiniBrowser):
   - cwebp/dwebp/gif2webp: signal 127
   - gdk-pixbuf-query-loaders/gtk-query-immodules-3.0: rld fatal loading .so modules

---

## Recent Work (session 94-95)

- **Determined WebProcess is NOT crashing** — signal catcher (C fork-based wrapper with setpgid, fd cleanup, signal ignoring) confirmed all WebProcess deaths are SIGTERM from MiniBrowser
- **Tested all IRIX IPC primitives** — comprehensive test_ipc.c: SOCK_STREAM, SCM_RIGHTS, poll, non-blocking — all work (SIGBUS in test 8 was test code bug: unaligned MIPS access)
- **Tested shm_open** — works in both chroot and host
- **Added IPC debug logging** — 12 injection points in ConnectionUnix.cpp via `ipc_debug_log.h` macros
- **Fixed YAML/RPM issues** — `%` in YAML causes parse errors (moved format strings to .h macros); `#` in RPM spec prep commands eaten as comments (use perl not sed)
- **Built and bundled** — `libwebkit2gtk-2.42.5-1-irix-bundle.0221260057.run` ready for deployment
- **Updated INDEX.md** with extensive negative knowledge (what's NOT the problem)

## Recent Work (session 93)

- Created `tools/rld-harness.py` — reusable bundle analysis tool
- Fixed global GOT overflow: `-Bsymbolic-functions` → `-Bsymbolic` in irix-ld

---

## Key Commands

```sh
# Bundle + test
uv run mogrix bundle libwebkit2gtk

# rld analysis harness
python3 tools/rld-harness.py analyze ~/mogrix_outputs/bundles/<bundle-dir>/
python3 tools/rld-harness.py got-inspect /path/to/lib.so

# Copy to IRIX host (two-step: chroot then host)
irix_copy_to local_path /opt/chroot/tmp/file
irix_host_exec "cp /opt/chroot/tmp/file /usr/people/edodd/file"
```
