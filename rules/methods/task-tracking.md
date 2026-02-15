# Task Tracking & Agent Orchestration

> Use Claude Code's built-in TaskCreate/TaskList for in-session task tracking.
> HANDOFF.md carries state between sessions.
> Batch builds use background agents — follow the orchestration rules below to avoid context exhaustion.

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

---

## Agent Orchestration for Batch Builds

When building multiple packages, use background Task agents. These rules prevent context exhaustion (which has killed two sessions in a row).

### Rule 1: Max 2-3 Concurrent Agents

Never launch more than 3 agents at once. Structure work as waves:

1. Pick 2-3 packages from the queue
2. Launch agents (one package per agent)
3. Collect results from disk
4. Update INDEX.md (orchestrator only)
5. Assess context — compact if past 60%
6. Repeat

### Rule 2: Agents Report to Disk, Not Conversation

Agents write results to `build-results/<package>.md`. The agent prompt must include:

```
**Reporting rules:** When done, write your result to build-results/<package>.md:

package: <name>
version: <version>
status: SUCCESS | FAILED | BLOCKED
rpms: <list of RPM filenames produced>
staged: yes | no
summary: <one line — what fix was applied or why it failed>
rule_file: rules/packages/<name>.yaml

Do NOT include build logs, configure output, or multi-line explanations.
Do NOT update rules/INDEX.md — the orchestrator handles that.
```

The agent's conversational return to the orchestrator should be equally brief — just "done, see build-results/foo.md". The orchestrator reads the file when ready, not from the agent's output.

### Rule 3: Only the Orchestrator Writes INDEX.md

Agents create/update `rules/packages/<name>.yaml` (that's their job). But `rules/INDEX.md` is written only by the orchestrator after reading agent results from disk. This eliminates concurrent edit conflicts and context duplication.

### Rule 4: Compaction Checkpoints

After collecting each wave's results:
1. Read `build-results/*.md` for the wave
2. Update INDEX.md and HANDOFF.md batch table
3. **Assess context usage.** If past 60%, write HANDOFF.md and either compact or end the session
4. Never launch new agents when context is tight

### Why These Rules Exist

The failure mode: 5+ agents each return large results → main context fills with accumulated build details → no room for orchestration or handoff → session dies without recovery. The fix: agents are workers that write to disk, the orchestrator is a coordinator that reads from disk. Conversation context only holds the current wave's summary.
