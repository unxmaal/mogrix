# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-21 (session 101)
**Status**: WebKit MiniBrowser RUNNING on IRIX! Root cause found and fixed: unnamed socketpair sockets caused GLib g_socket_new_from_fd() to fail (no SO_DOMAIN on IRIX).

---

## Post-Compaction Checklist (READ THIS FIRST)

1. **Mogrix invocation**: `uv run mogrix <command>` — not `mogrix`, not `python -m mogrix`
2. **Grep rules/INDEX.md** before attempting ANY fix — the answer is probably already there
3. **When you make a mistake or hit an error**: grep INDEX.md BEFORE retrying
4. **Read GENERIC_SUMMARY.md** before starting any new package
5. **Paths are different things**:
   - `/opt/sgug-staging/` = cross-compilation sysroot (on Linux build host)
   - `/opt/chroot` = IRIX test chroot (on IRIX host, accessed via MCP tools)
6. **IRIX access**: Use MCP tools. NEVER raw SSH. `irix_copy_to` targets CHROOT not host — chain with `irix_host_exec "cp ..."` to reach host.
7. **Testing**: Use `mogrix-test` MCP tools. NOT ad-hoc irix_exec calls.
8. **Large builds**: Use haiku sub-agents (Rule 6 in task-tracking.md). NEVER background Bash.
9. **Deploy irix-ld after editing**: `cp cross/bin/irix-ld /opt/sgug-staging/usr/sgug/bin/irix-ld`
10. **`mogrix build` takes a FILE PATH** not a package name: `mogrix build ~/mogrix_outputs/SRPMS/<pkg>-converted/<pkg>.src.rpm --cross`

---

## Current State

### WebKit MiniBrowser — WORKING

**Root cause found and fixed (session 101):**
Our compat `socketpair()` in `compat/sys/socketpair.c` created one unnamed socket (the connecting end). On IRIX, `getsockname()` on an unnamed AF_UNIX socket returns `addrlen=0`. GLib's `g_socket_new_from_fd()` then needs `SO_DOMAIN` to determine the address family — but **IRIX has no SO_DOMAIN**. Result: `g_socket_new_from_fd()` returned NULL, subprocesses had broken IPC sockets.

**Fix:** Bind BOTH sockets in `socketpair()` to named AF_UNIX paths (`/tmp/.msp_<pid>_<cnt>s` and `/tmp/.msp_<pid>_<cnt>c`). Now `getsockname()` returns valid AF_UNIX addresses for both ends. Fix deployed in `libmogrix_compat.so`.

**Confirmed working:** MiniBrowser renders `about:blank`, subprocesses (WebProcess + NetworkProcess) stay alive with working IPC.

**Diagnostic tooling still in place (TEMPORARY — remove when stable):**
- `webkit_subprocess_diag.h` in webkitgtk.yaml `add_source` + prep_commands
- Diag logs: `$MOGRIX_CRASH_DIR/mogrix_diag_<pid>.log`
- Crash handler: `mogrix_init/crash/exit_<pid>.log` via `MOGRIX_CRASH_DEBUG=1`

**Key files:**
- `compat/sys/socketpair.c` — the fix (named both socket ends)
- `patches/packages/webkitgtk/webkit_subprocess_diag.h` — diagnostic header
- `patches/shared/mogrix_crash_handler.c` — signal/atexit handler
- IRIX bundle: `/usr/people/edodd/apps/libwebkit2gtk-2.42.5-1-irix-bundle.0221261724`

### Sed Elimination — COMPLETE (session 99)

314 sed→safepatch across 80 packages. All validated vs baseline 1f2994a.

---

## Next Steps (prioritized)

1. **Test MiniBrowser with real URLs** — try loading an actual webpage, check for rendering issues
2. **Clean up temporary diagnostics** — remove `webkit_subprocess_diag.h` from webkitgtk.yaml once stable
3. **Rebuild WebKit without diag code** for a clean production bundle
4. **New package work** — See smallweb_plan.md

---

## Key Commands

```sh
# Convert + build (two steps, use correct SRPM path)
uv run mogrix convert ~/mogrix_inputs/SRPMS/<pkg>*.src.rpm
uv run mogrix build ~/mogrix_outputs/SRPMS/<pkg>-converted/<pkg>.src.rpm --cross

# Bundle + deploy
uv run mogrix bundle <pkg>
# Then: irix_copy_to → irix_host_exec cp

# Build with haiku sub-agent (large packages)
# See rules/methods/task-tracking.md Rule 6
# IMPORTANT: pass exact SRPM path, not package name
```

---

## Session 101 Key Findings

- **ROOT CAUSE FOUND: unnamed socketpair sockets + missing SO_DOMAIN on IRIX.** Our compat `socketpair()` left one socket unnamed → `getsockname()` returns addrlen=0 → GLib can't determine AF_UNIX family → `g_socket_new_from_fd()` returns NULL → IPC broken → subprocesses die.
- **Fix: bind both socketpair ends to named paths.** `compat/sys/socketpair.c` updated. `libmogrix_compat.so` rebuilt and deployed.
- **MiniBrowser running on IRIX!** Screenshot confirms: window opens, toolbar renders, `about:blank` loads. Both WebProcess and NetworkProcess alive with working IPC.
- **Crash handler deployed and documented** in `rules/methods/irix-testing.md`, `INDEX.md`, `compat-functions.md`.
- **`_exit()` interposition is IMPOSSIBLE on IRIX** — rld doesn't set $t9/GP for internal libc→interposed calls. SIGBUS (BUS_ADRALN). Added to INDEX.md anti-patterns.
- **Diagnostic write() approach works well** — `webkit_subprocess_diag.h` with `MOGRIX_DIAG("stage")` pinpointed the exact failure in `g_socket_new_from_fd()`. Pattern reusable for future debugging.
