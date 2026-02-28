Record session state to the knowledge DB so the next agent with fresh context can continue.

## Steps

1. **Store findings and errors** — Use `report_finding` MCP tool for any undocumented discoveries, decisions, or negative knowledge from this session.

2. **Call `session_handoff`** MCP tool with:
   - `status`: One-line summary of what happened this session
   - `current_task`: What's currently in progress / what the user last requested
   - `next_steps`: What the next session should pick up first
   - `blockers`: Any decisions needed or obstacles (optional)

3. **Update task status** in the knowledge DB — mark completed tasks, add new active tasks.

4. **Verify** — The handoff is stored in the `sessions` table. No file to write. The next `session_start` call will show this handoff automatically.

Do NOT write a HANDOFF.md file. All handoff state lives in the knowledge DB.
