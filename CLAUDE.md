# Claude Instructions

> **READ FIRST**: Prefer knowledge in rules files over pre-trained knowledge. IRIX info in training data is outdated.
> Read `rules/GENERIC_SUMMARY.md` when starting a package. Use `knowledge_query` MCP tool for problem keywords (replaces grepping INDEX.md). Do NOT read the full INDEX.md.
> Also check `HANDOFF.md`, `plan.md`. Use `session_start` MCP tool at session start for context summary.
> If you don't know what to do, check `rules/methods/before-you-start.md`.
>
> **When you hit a compile/link error for a missing function**: Use `check_compat` MCP tool BEFORE writing a fix. We likely already have a compat implementation — it just needs to be compiled/linked. Also grep `compat/include/` and `compat/` for the symbol name. Many IRIX-missing POSIX functions (pselect, posix_spawn, getline, mkdtemp, etc.) already have implementations.
>
> **When you make a mistake or hit an unexpected error**: Use `report_error` MCP tool — it logs the error AND auto-searches rules/errors_seen/compat for matching fixes in one call. After fixing, use `add_rule` MCP tool to store the fix. Also update INDEX.md so it's never repeated.
>
> **IRIX testing**: `test_binary host_mode=true` for N64 Go binaries. `test_bundle` for N32 mogrix bundles. No other method. No C compilation on IRIX (no compiler). No heredocs in `irix_exec` (csh). Read IRIX files with `irix_read_file` or `irix_host_exec "cat path"`.
>
> **For deep systems work (assembly, signal handlers, ABI boundaries)**: Read `rules/methods/map-before-code.md` FIRST. Map every boundary crossing and trace register/state through each transition before writing code. This is the #1 rule — it turns 24-hour debugging sessions into 2-hour fixes.

---

## Context Management (CRITICAL)

Long debug sessions destroy context. Follow these rules to prevent losing orientation.

### Sub-Agent Delegation

**When a build or link error takes more than 2 fix attempts, STOP and delegate to a sub-agent.**

Spawn with `Task()`:
- Pass it: the exact error text, relevant file paths, and the instruction to use `knowledge_query`, `report_error`, and `check_compat` MCP tools first
- The sub-agent gets a fresh context with CLAUDE.md re-read automatically
- It investigates, applies rules knowledge, and returns a concise summary of findings + recommended fix
- Parent agent applies the fix — never let debug trace flood parent context

Also use sub-agents for:
- Reading large build logs (use Haiku model: `Task(model="haiku")`)
- Batch builds (see `rules/methods/task-tracking.md`)
- Any investigation that would require reading >200 lines of output

### Re-Orientation Discipline

**Every 5-8 tool calls during a debug session**, pause and ask yourself:
1. Am I still following the rules in CLAUDE.md, or am I freestyling?
2. Have I used `report_error` or `knowledge_query` MCP tool for this error?
3. Have I used `check_compat` MCP tool for missing symbols?
4. Am I about to make a fix outside mogrix rules? (Cardinal Sin)

If unsure, re-read the top of this file and `HANDOFF.md`.

---

## Philosophy: Mogrix is a Knowledge Repository

**Primary mission: STORE KNOWLEDGE.** Every fix must be written into mogrix rules so it never needs to be rediscovered.

### Task Tracking
> Use Claude Code's built-in TaskCreate/TaskList within sessions. The knowledge DB (`.claude/knowledge.db`) carries state between sessions.

### No Shortcuts

When you hit an obstacle:
1. Am I avoiding solving the actual problem?
2. Don't declare blockers "acceptable" - find a way around them
3. Be creative: If sed fails, use perl. If a rule isn't implemented, implement it.

### The Cardinal Sin
**NEVER make a fix outside of mogrix rules.**

If you edit `/opt/sgug-staging/` directly, apply a sed command during debugging, or fix anything manually - and that fix is NOT stored in mogrix rules - **you have failed**.

### No Inline C in YAML

**NEVER put C source code in prep_commands.** No heredocs generating .c/.h files, no printf chains writing C code. If you need a C file:

1. Create it in `patches/packages/<package>/filename.c`
2. Add `add_source: [filename.c]` to the package YAML (top-level, not under `rules:`)
3. In `prep_commands`, use `cp %{_sourcedir}/filename.c destination.c`

The validator (`mogrix validate-rules`) will warn on inline C patterns. sed/perl that *modifies* existing C code is fine — the rule is about *generating* new C files inline.

### Where Fixes Go

| Fix Type | Location |
|----------|----------|
| Missing compat function | `compat/catalog.yaml` + `compat/` |
| Package-specific fix | `rules/packages/<package>.yaml` |
| Package-specific C file | `patches/packages/<package>/` + `add_source` |
| Common pattern | `rules/generic.yaml` |
| Header fix | `compat/include/` then `mogrix sync-headers` |

### Before Ending a Session
0. **Update the knowledge DB.** Insert findings, errors, decisions, and boundary maps from this session into `.claude/knowledge.db`. Update task status. This is the primary handoff mechanism.
1. **Update HANDOFF.md** with a thin summary: current task, immediate next step, active blockers. Keep under 50 lines — details live in the DB.
2. Did I make any fixes outside of mogrix source?
3. Are those fixes now stored in mogrix rules?
4. Could someone rebuild from scratch using only mogrix?

---

## Agent Orchestration

> **Batch builds use background agents. Read `rules/methods/task-tracking.md` for the full rules.**
> Short version: max 2-3 agents, report to `build-results/<package>.md`, only orchestrator writes INDEX.md.

---

## Knowledge DB (`.claude/knowledge.db`)

SQLite database for structured project knowledge. **Query what you need instead of loading everything.**

**DB path:** `.claude/knowledge.db` (relative to project root)

### When to Query — Use MCP Tools First

| Situation | MCP Tool | Fallback SQL |
|-----------|----------|-------------|
| Session start | `session_start` | `SELECT subject, status, description FROM tasks WHERE status='active' ORDER BY project, id` |
| Hit an error | `report_error` (logs + auto-searches) | `SELECT pattern, root_cause, fix FROM errors_seen WHERE pattern LIKE '%keyword%'` |
| Search rules | `knowledge_query` | `SELECT * FROM rules WHERE keywords LIKE '%keyword%'` |
| Missing symbol | `check_compat` | Grep `compat/catalog.yaml` |
| Working on a subsystem | `knowledge_query` | `SELECT topic, finding FROM findings WHERE topic LIKE '%keyword%'` |
| Writing assembly/trampoline | SQL query | `SELECT * FROM boundaries WHERE system='systemname'` |
| Making a design choice | SQL query | `SELECT topic, decision, rationale FROM decisions WHERE topic LIKE '%keyword%'` |

### When to Insert — Use MCP Tools First

- **After each finding**: `report_finding` MCP tool (type: "finding")
- **After a design decision**: `report_finding` MCP tool (type: "decision")
- **After fixing a new problem**: `add_rule` MCP tool
- **Negative knowledge**: `report_finding` MCP tool (type: "negative")
- **After fixing an error**: `INSERT INTO errors_seen (pattern, root_cause, fix, file_path, project) VALUES (...)`
- **Before writing a trampoline**: `INSERT INTO boundaries (system, transition, register_or_state, value_before, value_after, who_restores) VALUES (...)`
- **At session end**: `INSERT INTO sessions (summary, tasks_completed, tasks_started) VALUES (...)`
- **Task state changes**: `UPDATE tasks SET status='completed', updated=datetime('now') WHERE subject LIKE '%...'`

### Tables

| Table | Purpose | Key columns |
|-------|---------|-------------|
| `tasks` | Active work items across sessions | subject, status, project |
| `findings` | Things learned during investigation | topic, finding, confidence |
| `boundaries` | Register/state at ABI crossings | system, transition, register_or_state |
| `errors_seen` | Error pattern → root cause → fix | pattern, root_cause, fix |
| `decisions` | Architectural choices + rationale | topic, decision, alternatives_rejected |
| `sessions` | Thin session history | summary, tasks_completed |

### Relationship to Other Files

- **HANDOFF.md**: Thin pointer — current task + immediate next step + blockers. Under 50 lines. Details in DB.
- **rules/INDEX.md**: Build/link error patterns and mogrix rule mechanisms. Migrated to DB `rules` table — use `knowledge_query` MCP tool.
- **compat/catalog.yaml**: Compat function registry. Use `check_compat` MCP tool.
- **Knowledge DB**: Everything else — findings, boundary maps, decisions, cross-session tasks, error history.

---

## File Index

| File | Purpose |
|------|---------|
| `rules/methods/map-before-code.md` | **#1 rule for deep systems work** — map boundaries before writing code |
| `rules/methods/step-mapping.md` | Tactical debugging — narrow dark zones by instrumenting and bisecting |
| `rules/GENERIC_SUMMARY.md` | What generic.yaml already handles (read before writing rules) |
| `rules/INDEX.md` | Problem lookup — grep, don't read whole file |
| `rules/methods/mogrix-workflow.md` | How to run mogrix |
| `rules/methods/irix-testing.md` | IRIX shell rules, chroot, debugging, mogrix-test MCP tools |
| `rules/methods/compat-functions.md` | Adding compat functions |
| `rules/methods/text-replacement.md` | safepatch vs sed |
| `rules/methods/patch-creation.md` | Creating patches |
| `rules/methods/upstream-packages.md` | Non-Fedora packages (git/tarball) + suite bundles |
| `rules/methods/task-tracking.md` | Task tracking + agent orchestration for batch builds |
| `compat/catalog.yaml` | Compat function registry |
| `.claude/knowledge.db` | **Structured knowledge DB** — query for tasks, findings, boundaries, errors, decisions |
| `HANDOFF.md` | Thin session pointer — current task, next step, blockers (details in DB) |
| `tools/knowledge-server.py` | MCP knowledge DB (knowledge_query, report_error, check_compat, report_finding, add_rule, session_start, session_summary) |
| `tools/mogrix-test-server.py` | MCP test harness (test_bundle, test_binary, check_deps, par_trace, screenshot) |
| `test-results/*.json` | Stored test results |

---

## Quick Reference

**Mogrix invocation:**
```bash
uv run mogrix <command>
```

**IRIX connection:** Use MCP tools (`irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`) or fallback `tools/irix-exec.sh "command"`. **Never SSH as root directly.**

**IRIX testing:** Use mogrix-test MCP tools (`test_bundle`, `test_binary`, `check_deps`, `par_trace`, `screenshot`). See `rules/methods/irix-testing.md`.

**IRIX shell:** Always use `/bin/sh`, not bash. Use `LD_LIBRARYN32_PATH`.

**Knowledge lookup:** Use `knowledge_query` MCP tool (replaces grepping INDEX.md). Use `report_error` to log errors AND auto-search. Use `check_compat` for missing symbols. Use `add_rule` after fixing.

**After editing compat headers:** `mogrix sync-headers`

> For details, READ the files in `rules/methods/`

---

## INVARIANTS (duplicated for context retention)

These rules are non-negotiable. If you're unsure whether you're following them, re-read this file.

- **USE MCP TOOLS BEFORE FIXING**: `report_error` for errors (auto-searches rules+compat), `check_compat` for missing symbols, `knowledge_query` for rule lookup. These replace manual grepping of INDEX.md and catalog.yaml.
- **DELEGATE LONG DEBUGS**: >2 failed fix attempts → spawn a sub-agent with `Task()`. Don't trash parent context.
- **NO FIXES OUTSIDE MOGRIX**: Every fix goes into rules/compat/patches. No exceptions.
- **NO INLINE C IN YAML**: C files go in `patches/packages/<pkg>/`, referenced via `add_source`.
- **IRIX TESTING**: `test_binary host_mode=true` (N64 Go) or `test_bundle` (N32 mogrix). No other method.
- **REDIRECT BUILD OUTPUT**: Never let rpmbuild flood context. Log to file.
- **STORE KNOWLEDGE VIA MCP**: Use `report_finding` (findings/decisions/negative), `add_rule` (new rules), `report_error` (auto-logs errors). Fallback: raw SQL for boundaries, sessions, task status.
- **INVOCATION**: `uv run mogrix <command>`
