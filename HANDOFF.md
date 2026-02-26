# Project Handoff
**Last Updated**: 2026-02-26
**Status**: Go 1.24.1 IRIX port — Phase 3 (HTTPS/DNS) COMPLETE. Knowledge DB live.

## Current Task
User paused to make adjustments. Plan.md and HANDOFF.md updated.

## Next Steps (Go Port)
1. **os/exec test** — subprocess spawning via fork/exec
2. **File I/O stress test** — large files, concurrent access
3. **Build a real Go application** for IRIX
4. Investigate async preemption crash (low priority)

## Next Steps (Mogrix)
1. Verify jenna's ir8 fix
2. Rebuild 217 pre-Feb-20 libs with `-Bsymbolic`

## Active Blockers
None.

## Key Decision
User always handles GitHub pushes. Never push repos autonomously.

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
