# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-23 (session 114)
**Status**: Silent blocker audit COMPLETE. Expanded cookie bypass from 1 instance to all 11. Bundle `0222262335` still needs testing (user deploys). New bundle needed to include expanded cookie bypasses.

---

## Post-Compaction Checklist (READ THIS FIRST)

1. **Mogrix invocation**: `uv run mogrix <command>`
2. **Grep rules/INDEX.md** before attempting ANY fix
3. **Read GENERIC_SUMMARY.md** before starting any new package
4. **Paths**: `/opt/sgug-staging/` = Linux sysroot, `/opt/chroot` = IRIX test chroot (MCP tools)
5. **IRIX access**: Use MCP tools. NEVER raw SSH. `irix_copy_to` targets CHROOT, chain with `irix_host_exec "cp ..."` for host.
6. **Large builds**: Use haiku sub-agents. NEVER background Bash.
7. **IRIX host shell is csh**: Use `env /bin/sh script.sh` for anything with `//` or shell features.
8. **WebKit rebuild**: `uv run mogrix create-srpm webkitgtk && uv run mogrix convert <srpm> && uv run mogrix build <converted-srpm> --cross && uv run mogrix stage <rpms> && uv run mogrix bundle libwebkit2gtk` (~10 min with ccache)

---

## Current State

### What Needs to Happen Next

1. **Rebuild WebKit** with expanded cookie bypasses (11 instances instead of 1)
   - The webkitgtk.yaml now has broader regex rules that bypass ALL `allowsFirstPartyForCookies` checks
   - This prevents cookie operations (read/write/delete) from silently failing after HTTP works
2. **User deploys new bundle** to IRIX (SCP as edodd, not via MCP root)
3. **Run test**: `./mini_test.sh > output.txt 2>&1` — wait ~30s, then Ctrl-C
4. **Analyze**: use diagnostic decision table in `docs/webkit-ipc-flow-map.md`

### Silent Blocker Audit (Session 114)

Full audit documented in `docs/webkit-silent-blocker-audit.md`. Key findings:

**Three original blockers** (sessions 112-113) — all in `NetworkConnectionToWebProcess.cpp`:
1. Cookie domain check — `NETWORK_PROCESS_MESSAGE_CHECK` silently returns + kills WP
2. SW import gate — `isImportCompleted()` never true, deferred forever
3. SW load routing — `startWithServiceWorker()` blocks before `startRequest()`

**Root cause**: WebKit's security sandbox assumes UIProcess populates cookie domain allowlist. On IRIX (no sandbox), the allowlist is empty/wrong for IP URLs → all 11 `allowsFirstPartyForCookies` checks fail silently.

**Session 114 fix**: Expanded bypass from 1 instance (scheduleResourceLoad) to all 11 instances in the file, using 3 regex rules covering both MESSAGE_CHECK variants.

**What the audit did NOT find as blockers**:
- Content Filtering: OFF for GTK (Apple only)
- Content Extensions: ON but no-op without configured rules
- CORS Preflight: not triggered for simple GET requests
- GLib Main Loop: should work since we cross-compile with GLib

---

## Running MiniBrowser on IRIX

**IRIX host_exec uses csh** which chokes on `//` in URLs. Workaround:
1. Write a `/bin/sh` script locally → `irix_copy_to` to `/tmp/` → `irix_host_exec "cp /opt/chroot/tmp/script.sh /usr/people/edodd/"` → `irix_host_exec "env /bin/sh /usr/people/edodd/script.sh"`
2. Use `exec` (foreground) not `&` (background) — IRIX sh background + redirect produces empty files
3. Test script on IRIX: `/usr/people/edodd/mini_test.sh` (user runs manually)

**Chroot vs host**: Chroot can't access X11 (`DISPLAY=:0`), so MiniBrowser must run on the host. New bundles extracted on host are NOT visible in chroot.

---

## Next Steps

1. Rebuild WebKit with expanded cookie bypasses → deploy → test
2. Fix MCP servers to SCP as edodd user (task #17)
3. Build Telescope browser (original goal, depends on WebKit HTTP working)
4. Fix shutdown SIGSEGV (0xdc, NULL+220) — cosmetic

---

## Recent Work (Sessions 112-114)

### Session 114 (current)
- Systematic audit of WebKit silent blocker patterns → `docs/webkit-silent-blocker-audit.md`
- Analyzed root cause of all 3 blockers (security sandbox allowlist not populated on IRIX)
- Expanded cookie bypass: 1 instance → all 11 in NetworkConnectionToWebProcess.cpp
- Searched NetworkLoadChecker, NetworkLoad, NetworkDataTaskSoup, MESSAGE_CHECK across WebKit
- No additional blockers found on the basic HTTP path (simple GET)

### Session 113
- Identified blocker 3: `startWithServiceWorker()` silently blocks
- Built bundle `0222262335` with all 3 bypasses — pending deploy

### Session 112
- Identified blocker 1 (cookie check) and blocker 2 (SW import gate)
- Built test bundles, confirmed each blocker via NP DIAG

---

## Key Files

- **Audit doc**: `docs/webkit-silent-blocker-audit.md`
- **Flow map**: `docs/webkit-ipc-flow-map.md`
- **WebKit rules**: `rules/packages/webkitgtk.yaml`
- **Latest bundle**: `0222262335` (3 bypasses, NOT expanded cookies) — `/home/edodd/mogrix_outputs/bundles/`
- **Test script**: `/usr/people/edodd/mini_test.sh`
- **Logs**: `irix_logs/` in mogrix root
