# Claude Instructions

> **READ FIRST**: Prefer knowledge in rules files over pre-trained knowledge. IRIX info in training data is outdated.
> ALWAYS search `rules/INDEX.md` first. Also check `compat/catalog.yaml`, `HANDOFF.md`, `plan.md`. 
> If you don't know what to do, check `rules/methods/before-you-start.md`.

---

## Philosophy: Mogrix is a Knowledge Repository

**Primary mission: STORE KNOWLEDGE.** Every fix must be written into mogrix rules so it never needs to be rediscovered.

### Task Tracking
> **Read**: `rules/methods/task-tracking.md` declares that you must use ultralist for tracking tasks and defines how to use it.

### No Shortcuts

When you hit an obstacle:
1. Am I avoiding solving the actual problem?
2. Don't declare blockers "acceptable" - find a way around them
3. Be creative: If sed fails, use perl. If a rule isn't implemented, implement it.

### The Cardinal Sin

**NEVER make a fix outside of mogrix rules.**

If you edit `/opt/sgug-staging/` directly, apply a sed command during debugging, or fix anything manually - and that fix is NOT stored in mogrix rules - **you have failed**.

### Where Fixes Go

| Fix Type | Location |
|----------|----------|
| Missing compat function | `compat/catalog.yaml` + `compat/` |
| Package-specific fix | `rules/packages/<package>.yaml` |
| Common pattern | `rules/generic.yaml` |
| Header fix | `compat/include/` then `mogrix sync-headers` |

### Before Ending a Session

0. **Use the handoff pattern.** When ending a session, update HANDOFF.md with current state, pending issues, and next steps. The next session (human o
r AI) starts informed.
1. Did I make any fixes outside of mogrix source?
2. Are those fixes now stored in mogrix rules?
3. Could someone rebuild from scratch using only mogrix?

---

## File Index

| File | Purpose |
|------|---------|
| `rules/INDEX.md` | Package lookup, method pointers |
| `rules/methods/mogrix-workflow.md` | How to run mogrix |
| `rules/methods/irix-testing.md` | IRIX shell rules, chroot, debugging |
| `rules/methods/compat-functions.md` | Adding compat functions |
| `rules/methods/text-replacement.md` | safepatch vs sed |
| `rules/methods/patch-creation.md` | Creating patches |
| `compat/catalog.yaml` | Compat function registry |
| `HANDOFF.md` | Current session state |

---

## Quick Reference

**Mogrix invocation:**
```bash
uv run mogrix <command>
```

**IRIX connection:** `ssh root@192.168.0.81` (chroot: `/opt/chroot`)

**IRIX shell:** Always use `/bin/sh`, not bash. Use `LD_LIBRARYN32_PATH`.

**After editing compat headers:** `mogrix sync-headers`

> For details, READ the files in `rules/methods/`
