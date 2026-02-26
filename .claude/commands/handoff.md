Update the knowledge DB and HANDOFF.md so the next agent with fresh context can continue this work.

## Step 1: Update Knowledge DB (`.claude/knowledge.db`)

Insert any new knowledge from this session:

```bash
sqlite3 .claude/knowledge.db "INSERT INTO ..."
```

- **findings**: New things learned (topic, finding, source, project)
- **errors_seen**: Errors encountered and their fixes (pattern, root_cause, fix, file_path)
- **decisions**: Design choices made (topic, decision, rationale, alternatives_rejected)
- **boundaries**: ABI boundary maps created (system, transition, register_or_state, ...)
- **tasks**: Update status of completed tasks, add new active tasks
- **sessions**: Insert a session summary row

## Step 2: Update HANDOFF.md (thin pointer only)

1. Read HANDOFF.md first (if it exists)
2. **Keep it under 50 lines.** Details live in the knowledge DB, not here.
3. **Don't duplicate DB content.** If it's in the DB, don't repeat it in HANDOFF.md.
4. **Never reference Claude Code task IDs** — they're ephemeral.

## HANDOFF.md Structure

```markdown
# Project Handoff
**Last Updated**: <date>
**Status**: <one-line summary>

## Current Task
What's in progress. What was the user's last request.

## Next Steps
1. First priority
2. Second priority

## Active Blockers
Any decisions needed or obstacles.

## Post-Compaction Checklist (READ THIS FIRST)
1. **Query the knowledge DB**: `sqlite3 .claude/knowledge.db "SELECT * FROM tasks WHERE status='active'"`
2. **Mogrix invocation**: `uv run mogrix <command>`
3. **Grep rules/INDEX.md** before attempting ANY fix
4. **Read GENERIC_SUMMARY.md** before starting any new package
5. **Paths**: `/opt/sgug-staging/` = Linux sysroot, `/opt/chroot` = IRIX chroot (MCP tools)
6. **IRIX access**: Use MCP tools. NEVER raw SSH.
7. **Go port source**: `~/projects/golang-irix/`
8. **Go cross-compile**: `cd ~/projects/golang-irix && GOOS=irix GOARCH=mips64 ./bin/go build ...`
9. **Test Go binaries**: `test_binary host_mode=true`. Deploy via `irix_copy_to host_path=true owner=edodd`.
```

Save as HANDOFF.md in the project root and tell the user the file path.
