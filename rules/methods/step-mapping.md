# Step-Mapping Analysis

A systematic method for debugging failures in multi-step processes where you can't attach a debugger. Especially valuable for cross-platform porting, IPC debugging, distributed systems, and any scenario where a process works in one configuration but fails in another.

## The Problem It Solves

You have a system that works in scenario A but fails in scenario B. Both scenarios share large portions of the same code path. You don't know where the failure occurs because the intermediate steps are unobservable ("dark territory"). Randomly adding instrumentation wastes rebuild cycles and risks never finding the divergence point.

## The Method

### 1. Start in Known Territory

Never start your map in the dark. Find an execution path that is **provably working** and that shares code with the broken path. This is your baseline.

Example: If a brokered IPC connection fails but an inherited IPC connection works, and both go through `platformInitialize()` → `platformOpen()` → `readyReadHandler()`, then the working connection IS your known territory.

### 2. Map Every Step

Write out every step from the working path through to the failure symptom. For each step, record:

| Column | Purpose |
|--------|---------|
| **Step #** | Sequential numbering |
| **What happens** | Concrete action (function call, syscall, state change) |
| **Code location** | File:line or function name |
| **Instrumented?** | What logging/tracing exists today |
| **Status** | MAPPED / PARTIAL / DARK / ASSUMED / CONFIRMED |

Status definitions:
- **CONFIRMED**: We have log output proving this step executes correctly
- **MAPPED**: We have instrumentation that would show success or failure
- **PARTIAL**: We log some outcomes (e.g. errors) but not others (e.g. success)
- **ASSUMED**: We believe it works because a downstream step works, but we've never verified
- **DARK**: No instrumentation. We have no idea what happens here.

### 3. Identify the Dark Zone

The dark zone is the contiguous range of steps between the last CONFIRMED/MAPPED step and the first step where you observe the failure symptom.

```
CONFIRMED → CONFIRMED → DARK → DARK → DARK → DARK → SYMPTOM
                         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^
                         This is where the bug lives
```

### 4. Instrument the Shared Path, Not Just the Broken One

This is the critical insight: **instrument the code that both the working and broken paths share**. When you run the test, the working path produces a "golden log" showing what normal looks like. The broken path produces a log you can diff against it.

If you only instrument the broken path, you're guessing at what "correct" looks like. If you instrument the shared path, the working execution tells you.

### 5. Light Up From Both Ends

Add instrumentation at the **last confirmed step** (extending into the dark zone) and at the **first symptom step** (extending backward into the dark zone). This narrows the dark zone from both sides simultaneously.

```
Before:  CONFIRMED → DARK → DARK → DARK → DARK → SYMPTOM
After:   CONFIRMED → MAPPED → DARK → DARK → MAPPED → SYMPTOM
                              ^^^^^^^^^^^^^^
                              Smaller dark zone
```

### 6. One Rebuild, Maximum Coverage

Before rebuilding, ask: "Can I foresee needing more instrumentation after this?" Each rebuild is expensive. Plan all your DIAG points in one pass. It's better to have 15 log lines you don't need than to do 5 rebuild cycles.

### 7. Iterate: Narrow the Dark Zone

After each instrumented test run, update the map. Promote DARK steps to MAPPED or CONFIRMED. The dark zone shrinks. Repeat until the dark zone is 1-2 steps wide — then you've found your bug.

## Anti-Patterns

- **Starting in the dark**: Adding instrumentation to the broken path without first confirming the baseline. You don't know what "normal" looks like.
- **Assuming "it works" means "it's mapped"**: Just because the end result is correct doesn't mean you understand the intermediate steps. Those ASSUMED steps may hide the divergence point.
- **Instrumenting only error paths**: If you only log failures, you can't distinguish "this step never executed" from "this step executed but the log doesn't show it."
- **One DIAG point per rebuild**: Maximally wasteful. Plan ahead.
- **Skipping the map**: Jumping straight to "I think the bug is in X" without mapping the territory. You end up debugging your hypothesis instead of the system.

## Template

```markdown
# [System] — Step Flow Map

**Created**: [date]
**Purpose**: [one line]

## Context
[What works, what's broken, what's shared between them]

## Phase A: Known Working Path (BASELINE)
| # | Step | Code Location | Instrumented? | Status |
|---|------|---------------|---------------|--------|
| 1 | ... | ... | ... | CONFIRMED |

## Phase B: Divergence Point
| # | Step | Code Location | Instrumented? | Status |
|---|------|---------------|---------------|--------|
| N | ... | ... | ... | DARK |

## Phase C: Failure Zone
...

## Coverage Summary
[ASCII diagram showing CONFIRMED/MAPPED/DARK distribution]

## Planned Instrumentation
[Table of DIAG points, which steps they cover, what they reveal]
```

## When to Use This

- Cross-platform port: same code, different OS behavior
- IPC / network debugging: sender works, receiver doesn't
- Multi-process systems: one process path works, another doesn't
- Regression hunting: worked before, broken now, same code path
- Any "it should work but doesn't" where printf-debugging is your only option
