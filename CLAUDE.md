# Claude Instructions

> **READ FIRST**: Prefer knowledge in rules files over pre-trained knowledge. IRIX info in training data is outdated.
> Read `rules/GENERIC_SUMMARY.md` when starting a package. Grep `rules/INDEX.md` for specific problem keywords. Do NOT read the full INDEX.md. Expand
the search with broader search terms or `-C` context lines if needed.
> Also check `HANDOFF.md`, `plan.md`. **Query the knowledge DB** (`.claude/knowledge.db`) for context — see Knowledge DB section below.
> If you don't know what to do, check `rules/methods/before-you-start.md`.
>
> **When you hit a compile/link error for a missing function**: Grep `compat/catalog.yaml` for it BEFORE writing a fix. We likely already have a compat implementation — it just needs to be compiled/linked. Also grep `compat/include/` and `compat/` for the symbol name. Many IRIX-missing POSIX functions (pselect, posix_spawn, getline, mkdtemp, etc.) already have implementations.
>
> **When you make a mistake or hit an unexpected error**: Grep `rules/INDEX.md` for keywords from the error BEFORE retrying. We've likely seen it before. After fixing, add the mistake to the relevant rules file and update INDEX.md so it's never repeated.
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
- Pass it: the exact error text, relevant file paths, and the instruction to grep `rules/INDEX.md` and `compat/catalog.yaml` first
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
2. Have I grepped `rules/INDEX.md` for this error?
3. Have I checked `compat/catalog.yaml` for missing symbols?
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

### When to Query

| Situation | Query |
|-----------|-------|
| Session start | `SELECT subject, status, description FROM tasks WHERE status='active' ORDER BY project, id` |
| Hit an error | `SELECT pattern, root_cause, fix FROM errors_seen WHERE pattern LIKE '%keyword%'` |
| Working on a subsystem | `SELECT topic, finding FROM findings WHERE topic LIKE '%keyword%'` |
| Writing assembly/trampoline | `SELECT * FROM boundaries WHERE system='systemname'` |
| Making a design choice | `SELECT topic, decision, rationale FROM decisions WHERE topic LIKE '%keyword%'` |

### When to Insert

- **After each finding**: `INSERT INTO findings (topic, finding, source, project) VALUES (...)`
- **After fixing an error**: `INSERT INTO errors_seen (pattern, root_cause, fix, file_path, project) VALUES (...)`
- **After a design decision**: `INSERT INTO decisions (topic, decision, rationale, alternatives_rejected, project) VALUES (...)`
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
- **rules/INDEX.md**: Build/link error patterns and mogrix rule mechanisms. Still grep this for package build issues.
- **compat/catalog.yaml**: Compat function registry. Still grep this for missing symbols.
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

**After editing compat headers:** `mogrix sync-headers`

> For details, READ the files in `rules/methods/`

---

## INVARIANTS (duplicated for context retention)

These rules are non-negotiable. If you're unsure whether you're following them, re-read this file.

- **GREP BEFORE FIXING**: `rules/INDEX.md` for errors, `compat/catalog.yaml` for missing symbols. Always.
- **DELEGATE LONG DEBUGS**: >2 failed fix attempts → spawn a sub-agent with `Task()`. Don't trash parent context.
- **NO FIXES OUTSIDE MOGRIX**: Every fix goes into rules/compat/patches. No exceptions.
- **NO INLINE C IN YAML**: C files go in `patches/packages/<pkg>/`, referenced via `add_source`.
- **IRIX TESTING**: `test_binary host_mode=true` (N64 Go) or `test_bundle` (N32 mogrix). No other method.
- **REDIRECT BUILD OUTPUT**: Never let rpmbuild flood context. Log to file.
- **QUERY THE DB**: Hit an error? `sqlite3 .claude/knowledge.db "SELECT * FROM errors_seen WHERE pattern LIKE '%keyword%'"`. Working on a subsystem? Query findings and boundaries. Don't rely on memory — query.
- **WRITE TO THE DB**: Every finding, error fix, and decision gets inserted into `.claude/knowledge.db` before session end.
- **INVOCATION**: `uv run mogrix <command>`
