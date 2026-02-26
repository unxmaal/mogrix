# Project Handoff
**Last Updated**: 2026-02-26
**Status**: Go 1.24.1 IRIX port — Phase 2 signal handling COMPLETE. Knowledge DB created.

## Current Task
Knowledge infrastructure — created `.claude/knowledge.db` (SQLite) to replace HANDOFF.md as the primary cross-session knowledge store. All findings, boundary maps, errors, decisions, and tasks are now queryable.

## Next Steps
1. **HTTPS client test** — TLS/crypto should work now that signals are fixed
2. **DNS resolution test** — net.LookupHost
3. **Push golang-irix to GitHub**
4. Verify jenna's ir8 fix
5. Rebuild 217 pre-Feb-20 libs with `-Bsymbolic`

## Active Blockers
None.

## Post-Compaction Checklist (READ THIS FIRST)
1. **Query the knowledge DB**: `sqlite3 .claude/knowledge.db "SELECT subject, status FROM tasks WHERE status='active'"`
2. **Query for context**: `sqlite3 .claude/knowledge.db "SELECT topic, finding FROM findings WHERE project='golang-irix' ORDER BY id DESC LIMIT 10"`
3. **Mogrix invocation**: `uv run mogrix <command>`
4. **Grep rules/INDEX.md** before attempting ANY fix
5. **Read GENERIC_SUMMARY.md** before starting any new package
6. **Paths**: `/opt/sgug-staging/` = Linux sysroot, `/opt/chroot` = IRIX chroot (MCP tools)
7. **IRIX access**: Use MCP tools. NEVER raw SSH.
8. **Go port source**: `~/projects/golang-irix/`
9. **Go cross-compile**: `cd ~/projects/golang-irix && GOOS=irix GOARCH=mips64 ./bin/go build ...`
10. **Test Go binaries**: `test_binary host_mode=true`. Deploy via `irix_copy_to host_path=true owner=edodd`.
