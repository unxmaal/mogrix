# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-23 (session 118)
**Status**: WebKit HTTPS WORKING on IRIX. HTTP+HTTPS both verified. Ready for surf browser build.

---

## Post-Compaction Checklist (READ THIS FIRST)

1. **Mogrix invocation**: `uv run mogrix <command>`
2. **Grep rules/INDEX.md** before attempting ANY fix
3. **Read GENERIC_SUMMARY.md** before starting any new package
4. **Paths**: `/opt/sgug-staging/` = Linux sysroot, `/opt/chroot` = IRIX test chroot (MCP tools)
5. **IRIX access**: Use MCP tools. NEVER raw SSH. `irix_copy_to` supports `host_path=true` + `owner="edodd"`.
6. **Large builds**: Use haiku sub-agents. NEVER background Bash.
7. **WebKit rebuild**: `uv run mogrix convert ~/mogrix_inputs/SRPMS/webkitgtk-2.42.5-1.src.rpm && uv run mogrix build ~/mogrix_outputs/SRPMS/webkitgtk-2.42.5-1.src-converted/webkitgtk-2.42.5-1.src.rpm --cross && uv run mogrix stage ~/mogrix_outputs/RPMS/libwebkit2gtk*.rpm && uv run mogrix bundle libwebkit2gtk` (fast with ccache)
8. **Bundle deploy**: `irix_copy_to` with `host_path=true, owner="edodd"` to `/usr/people/edodd/apps/`. Extract with `sh <bundle>.run`, then `mv` + `chown -Rh edodd` to user apps dir.

---

## Current State

### WebKit HTTPS — WORKING (session 118)

Bundle `0223262218` renders HTTPS pages on IRIX. Verified with `https://example.com/` — full TLS handshake, HTTP 200, page rendered with title, body text, and links.

Key fixes that enabled HTTPS:
1. **DEVELOPER_MODE bypass** for `WEBKIT_TLS_CAFILE_PEM` in SoupNetworkSession.cpp (prep_commands in webkitgtk.yaml)
2. **WEBKIT_TLS_CAFILE_PEM export** in bundle wrapper (bundle.py) — points GnuTLS to bundle's CA certs
3. **Bundle pruning fix** for GIO TLS module deps (bundle.py) — `libgnutls.so.30` symlink target was being pruned

### WebKit HTTP — WORKING (session 116)

Bundle `0223261927` renders HTTP pages. All 5 silent blockers bypassed.

### 5 HTTP Bypasses in webkitgtk.yaml

| # | What | Fix |
|---|------|-----|
| 1 | Cookie domain check (11 instances) | Bypass `allowsFirstPartyForCookies` |
| 2 | SW import gate | `!server.isImportCompleted()` → `false` |
| 3 | SW load routing | `startWithServiceWorker()` → `start()` |
| 4 | HTTP disk cache | `canUseCache()` → always `false` |
| 5 | GNetworkMonitor NULL | NULL-check before signal connect |

---

## Next Steps

1. **Build surf browser** — now that HTTPS works, surf is a lightweight WebKit GTK browser
2. **Test more HTTPS sites** — verify beyond example.com (sites with JS, CSS, images)
3. **Consider disabling DIAG tags** for production bundles (set MOGRIX_DIAG env var gate already handles this)

---

## Recent Work (Sessions 117-118)

### Session 118
- Built + deployed WebKit with HTTPS instrumentation + fixes
- Fixed `soup_session_get_tls_database` → soup2-compatible `g_object_get` with "tls-database"
- Fixed **bundle pruning bug**: GIO module deps (libgnutls.so.30 symlink target + deps-of-deps) were being pruned. Updated `bundle.py` to resolve symlinks and protect transitive deps
- **HTTPS confirmed working**: TLS_HANDSHAKING → TLS_HANDSHAKED → status=200 → full data delivery
- Screenshot captured: MiniBrowser rendering https://example.com/ on IRIX

### Session 117
- Added `MOGRIX_DIAG` env var gate to webkit_subprocess_diag.h (zero overhead when disabled)
- Added 11 TLS DIAG tags (TLS-1 to TLS-11) in webkitgtk.yaml
- Removed DEVELOPER_MODE guard from WEBKIT_TLS_CAFILE_PEM code
- Added WEBKIT_TLS_CAFILE_PEM export to bundle.py wrapper

---

## Key Files

- **WebKit rules**: `rules/packages/webkitgtk.yaml` (all bypasses + 33 HTTP + 11 TLS DIAG tags)
- **DIAG header**: `patches/packages/webkitgtk/webkit_subprocess_diag.h` (env var gated)
- **Bundle wrapper**: `mogrix/bundle.py` (WEBKIT_TLS_CAFILE_PEM + SSL_CERT_FILE + pruning fix)
- **Latest working bundle**: `0223262218` (HTTP+HTTPS, deployed at `/usr/people/edodd/apps/`)
- **Test scripts**: `/usr/people/edodd/https_test.sh`, `/usr/people/edodd/https_test2.sh` on IRIX
- **HTTPS plan**: `.claude/plans/misty-beaming-patterson.md` (full problem space map + decision table)
- **Silent blockers guide**: `rules/methods/webkit-silent-blockers.md`
