# Task Tracking Method

> Use Claude Code's built-in TaskCreate/TaskList for in-session task tracking.
> HANDOFF.md carries state between sessions.

---

## Tool Hierarchy

| Tool | Purpose | When to Update |
|------|---------|----------------|
| **TaskCreate/TaskList** | In-session task tracking with progress, deps | During work |
| **HANDOFF.md** | Current state, pending tasks, session context | End of session |
| **plan.md** | High-level goals, architecture | When goals change |

---

## Session Workflow

### At Session Start

1. Read HANDOFF.md for current state and pending tasks
2. Create tasks with TaskCreate for the work items
3. Pick one and start working

### During Work

- Use TaskUpdate to mark tasks in_progress/completed
- Create new tasks as discovered during work
- Use dependencies (addBlockedBy) when tasks depend on each other

### At Session End

1. Run `/handoff` to update HANDOFF.md with:
   - What was completed
   - What's still pending (so next session can recreate tasks)
   - Any blockers or decisions needed
