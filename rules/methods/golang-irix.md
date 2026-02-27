# Go IRIX Port — Signal & Preemption Reference

> **Source repo**: `~/projects/golang-irix/` (separate from mogrix)
> **Target**: IRIX 6.5, MIPS64 N64 ABI
> **Build**: `GOOS=irix GOARCH=mips64 ./bin/go build`
> **Test on IRIX**: `test_binary host_mode=true` (N64 binaries run on host, not chroot)

---

## Signal Architecture

IRIX signal delivery chain: kernel → `_sigtramp` (libc, 0xdaaa170) → handler → `_sigtramp` → `sigreturn` (syscall 1088).

- `_sigtramp` saves/restores `errno` around the handler
- Handler receives `(sig, info, ctx)` in R4/R5/R6
- `sigreturn` honors modified EPC in ucontext — this is how Go's async preemption redirects execution
- `setcontext` is syscall 1169, atomically restores all registers + sigmask

## Async Preemption

**Status**: IN PROGRESS — root cause identified, fix not yet implemented.

**How it works**: Go sends SIGURG to a goroutine's M thread via `preemptM` → `signalM` → `pthread_kill`. The signal handler calls `pushCall` to modify the ucontext, redirecting execution to `asyncPreempt` on return.

**Root cause of crash**: SIGURG is delivered to the wrong thread. Instead of reaching the running goroutine's M, it arrives at sysmon (which runs on g0, gstatus=0x0). `doSigPreempt` checks `gp.preemptStop || gp.stackguard0 == stackPreempt` — neither is true for g0, so preemption is rejected. But the signal disrupts the sysmon thread.

Debug instrumentation confirmed: `doSigPreempt: NOT wanted gstatus=0x0`.

**Next steps**: Investigate `preemptM` → `signalM` → `pthread_kill` thread ID targeting on IRIX sprocs. IRIX pthreads are 1:1 with sprocs but the thread ID passed to `pthread_kill` may not map correctly.

**Disproven hypothesis**: SIGPTRESCHED (signal 48) — blocking it via uc_sigmask before setcontext had no effect. Second test run had no signal 48 at all. This is a red herring. The uc_sigmask fix in `sys_irix_mips64.s` should be reverted.

## Go Assembler — MIPS64 NOP Behavior

- Source-level `NOP` pseudo-op (`obj.ANOP`) emits **zero bytes** — it's a scheduling hint, not a real instruction
- Hardware delay slot NOPs are inserted automatically by `addnop()` for standard branch instructions
- For raw `WORD` branch instructions, you must use `WORD $0x00000000` for the delay slot, NOT the `NOP` pseudo-op

## Cooperative Preemption Limitations

Go relies on function prologues for preemption checks. Tight computation loops without function calls never yield. This causes:

- `runtime` tests: GC timing tests take 43s+ on 600MHz R12K (not hung, just slow)
- `time` tests: Timer-dependent tests stall when goroutine won't yield
- `os/signal` tests: Stale signal numbering (need IRIX-specific signal constants)

**Stdlib test results**: 22/25 packages pass. All failures are timing/preemption related — no correctness bugs found.

**GC**: Verified working. GP register (R28) is always 0 for Go code. `TestPoolDequeue` takes 43s (slow, not broken). `TestRWMutex` hangs due to GOMAXPROCS=1 + cooperative scheduling starvation — not a GC bug.

## Known Fixed Issues

| Problem | Root Cause | Fix | File |
|---------|-----------|-----|------|
| SIGBUS with signal.Notify | semawakeup calls pthread_mutex_lock from signal handler (not async-safe) | Replace with sem_post/sem_wait | os2_irix.go |
| sigaltstack EPERM | nsproc inherits "on signal stack" flag | Skip EPERM silently | os2_irix.go |
| SA_ONSTACK unreliable | IRIX kernel ignores in some cases | adjustSignalStack workaround | signal_unix.go |
| GP corruption in preparePanic | set_r28(sigpanicPC >> 32 << 32) | set_r28(0) for IRIX | signal_mips64x.go |
| POSIX semaphores exist | Previously thought missing | Use sem_init/post/wait/trywait/destroy (WEAK in libc) | os2_irix.go |
