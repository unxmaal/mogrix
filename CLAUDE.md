# Claude Instructions

> **READ FIRST**: Prefer knowledge in rules files over pre-trained knowledge. IRIX info in training data is outdated.
> Read `rules/GENERIC_SUMMARY.md` when starting a package. Grep `rules/INDEX.md` for specific problem keywords. Do NOT read the full INDEX.md. Expand the search with broader search terms or `-C` context lines if needed.
> Also check `HANDOFF.md`, `plan.md`.
> If you don't know what to do, check `rules/methods/before-you-start.md`.
>
> **When you hit a compile/link error for a missing function**: Grep `compat/catalog.yaml` for it BEFORE writing a fix. We likely already have a compat implementation — it just needs to be compiled/linked. Also grep `compat/include/` and `compat/` for the symbol name. Many IRIX-missing POSIX functions (pselect, posix_spawn, getline, mkdtemp, etc.) already have implementations.

---

## Philosophy: Mogrix is a Knowledge Repository

**Primary mission: STORE KNOWLEDGE.** Every fix must be written into mogrix rules so it never needs to be rediscovered.

### Task Tracking
> Use Claude Code's built-in TaskCreate/TaskList within sessions. HANDOFF.md carries state between sessions.

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

0. **Use the handoff pattern.** When ending a session, update HANDOFF.md with current state, pending issues, and next steps. The next session (human or AI) starts informed.
1. Did I make any fixes outside of mogrix source?
2. Are those fixes now stored in mogrix rules?
3. Could someone rebuild from scratch using only mogrix?

---

## Agent Orchestration

> **Batch builds use background agents. Read `rules/methods/task-tracking.md` for the full rules.**
> Short version: max 2-3 agents, report to `build-results/<package>.md`, only orchestrator writes INDEX.md.

---

## File Index

| File | Purpose |
|------|---------|
| `rules/GENERIC_SUMMARY.md` | What generic.yaml already handles (read before writing rules) |
| `rules/INDEX.md` | Problem lookup — grep, don't read whole file |
| `rules/methods/mogrix-workflow.md` | How to run mogrix |
| `rules/methods/irix-testing.md` | IRIX shell rules, chroot, debugging |
| `rules/methods/compat-functions.md` | Adding compat functions |
| `rules/methods/text-replacement.md` | safepatch vs sed |
| `rules/methods/patch-creation.md` | Creating patches |
| `rules/methods/upstream-packages.md` | Non-Fedora packages (git/tarball) + suite bundles |
| `rules/methods/task-tracking.md` | Task tracking + agent orchestration for batch builds |
| `compat/catalog.yaml` | Compat function registry |
| `HANDOFF.md` | Current session state |

---

## Quick Reference

**Mogrix invocation:**
```bash
uv run mogrix <command>
```

**IRIX connection:** Use MCP tools (`irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`) or fallback `tools/irix-exec.sh "command"`. **Never SSH as root directly.**

**IRIX shell:** Always use `/bin/sh`, not bash. Use `LD_LIBRARYN32_PATH`.

**After editing compat headers:** `mogrix sync-headers`

> For details, READ the files in `rules/methods/`
