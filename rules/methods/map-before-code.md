# Map the Territory Before Writing Code

> **This is the #1 rule for deep systems work.** Before writing any code that crosses ABI boundaries, OS interfaces, or calling conventions — map the full problem space. Know where the dark places are before you enter them.

## The Meta-Lesson

A 2-hour fix and a 24-hour fix often involve the same amount of actual code. The difference is knowing which rooms are dark before you open the door.

**Unknown unknowns kill you.** If you don't know a problem exists, you can't debug it — you can only stumble into it, misdiagnose the symptoms, chase red herrings, and eventually find it by exhaustion. The map turns unknown unknowns into known unknowns, which you can investigate systematically.

## The Method

### 1. Identify Every Boundary

Before writing code, list every point where control or data crosses between different domains:

- **ABI boundaries**: Go ↔ libc, signal handler ↔ userspace, kernel ↔ userspace
- **Calling conventions**: Which registers are callee-saved? Caller-saved? Have special meaning?
- **Ownership transitions**: Who owns the stack? Who owns GP? Who owns the g register?
- **State assumptions**: What does each side assume about register/memory state on entry and exit?

### 2. Trace State Through Every Boundary

For each boundary crossing, write out (pen and paper, or a markdown table):

| Transition | Register/State | Value Before | Value After | Who Restores? |
|------------|---------------|--------------|-------------|---------------|
| libc → sigtramp | GP (R28) | libc's DSO GP | ??? | ??? |
| sigtramp → Go code | GP (R28) | ??? | must be 0 | sigtramp zeros it |
| Go code → return to __sigtramp | GP (R28) | 0 | must be libc's GP | **nobody — BUG** |

If any cell says "???" or "nobody", you've found a dark place. That's where the bugs live.

### 3. Study How Others Solved It

Before implementing an OS subsystem, read how the 2-3 most architecturally similar ports handle it:

- **Go IRIX uses libc trampolines** → study Solaris (`tls_solaris_amd64.s`) and AIX (`tls_aix_ppc64.s`)
- **Writing a signal trampoline** → study how existing ports save/restore state across the handler boundary
- **Implementing TLS** → study what persistent per-thread storage the target OS provides

The similar ports have already solved the problems you're about to hit. Their code IS the map.

### 4. Test Primitives in Isolation

Before integrating a low-level mechanism (TLS, signal delivery, thread creation), validate it independently:

- Write the validation test **before** the implementation
- Prove each primitive works in isolation before combining them
- When combined code fails, you can trust the individual pieces and focus on the integration

Example: `test_prda` proved PRDA is per-thread before it was used for g storage. When the combined signal test still crashed, we knew the PRDA mechanism was correct and looked elsewhere (GP restore).

### 5. Mark the Dark Places

After mapping, you should have a list of things you're uncertain about. Write them down explicitly:

```
KNOWN: save_g stores g to PRDA (per-thread, verified)
KNOWN: sigtramp zeros GP for Go code
DARK:  What GP value does __sigtramp expect when we return?
DARK:  Does load_g work in signal context (no libc, no GP)?
DARK:  Can we access PRDA from gsignal stack?
```

Now you have a targeted investigation plan, not a "try things and see what happens" session.

## Anti-Patterns

### Micro-Focus Without the Map
Jumping straight to writing small test programs without understanding the full system. You end up with 20 test files that each prove one micro-fact, but you never assembled the big picture that would have revealed the bug in 10 minutes of thought.

### Printf Debugging in Constrained Environments
Using println/fmt in signal handlers, interrupt contexts, or tiny stacks. These environments have constraints (stack size, async-signal-safety, no allocation) that make printf debugging actively harmful — it introduces new crashes that mask the real bug.

**Instead:** Use assembly-level binary search. Start with a no-op handler, add code back piece by piece. Each iteration proves which instruction introduced the crash.

### Debugging Your Hypothesis Instead of the System
"I think the bug is in X" → spend 6 hours proving X works → discover the bug was in Y all along. The map prevents this by showing you ALL the dark places, not just the one you guessed.

### Session Fragmentation
Deep systems bugs require holding the full mental model in your head. Context compaction, session boundaries, and re-reading files all degrade this model. For problems that require deep understanding:

- Write a focused investigation document at the START
- Include: the boundary map, register state at each transition, what each test proves/disproves
- Keep it updated as you learn. This survives context compaction.

## Relationship to Step-Mapping

This method (`map-before-code`) is the **strategic** complement to `step-mapping.md`:

| | Map Before Code | Step Mapping |
|---|---|---|
| **When** | Before writing code | During debugging |
| **Focus** | System boundaries and state transitions | Execution paths and dark zones |
| **Goal** | Know where bugs WILL be | Find where bugs ARE |
| **Output** | Boundary/register state table | Step flow map with instrumentation plan |

Use **map-before-code** first. If bugs still occur (they will, but fewer), use **step-mapping** to hunt them down efficiently within the already-mapped territory.

## Case Study: Go IRIX Signal Handling (24 hours → 2 hours)

Two bugs, both visible from the boundary map:

**Bug 1: load_g was a no-op**
- Boundary map would show: "signal interrupts libc → R30 holds libc's frame pointer, not g"
- The Solaris/AIX ports both have real save_g/load_g implementations
- The question "what's our equivalent of their TLS mechanism?" reveals the no-op immediately

**Bug 2: GP not restored before returning to __sigtramp**
- Register state trace would show: "GP=0 when control returns to __sigtramp → __sigtramp uses GP-relative addressing → crash"
- The rule "every register you touch in a trampoline must be saved/restored" catches this before writing code
- 30 minutes with pen and paper vs. 12 hours of binary search

Both bugs are calling-convention violations visible from first principles. No debugging tools needed — just a complete map of the territory.
