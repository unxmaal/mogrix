# Constraints and Invariants

*Source: mogrix — IRIX cross-compilation system (1009 files, 111307 lines). Derived from marker extraction: CRITICAL, TODO, HACK, FIXME, WORKAROUND, SAFETY, INVARIANT annotations across all source files.*

---

## 1. LLD / Linker Patching

### C1.1 — Conditional expression form in phdr skip logic

**What:** The IRIX LLD patch must use the two-branch form `if (skipPhdr) {} else if (isMain)`, not the logically equivalent `!skipPhdr && isMain`.

**Where:** `lld-fixes/build-lld-irix.sh:106`

**What breaks:** The short-circuit form apparently misfires under LLVM's IR optimization or the specific LLD code path being patched — the semantic difference is invisible in C but matters in the LLVM AST or surrounding state machine. The original comment marks this as CRITICAL, implying the linker produces wrong output or crashes silently.

**Enforcement:** Convention only (comment). No assertion or test guards the conditional form; a maintainer editing the patch could rewrite it to the "equivalent" form without triggering any failure at patch-apply time.

---

### C1.2 — Output-emission suppression (deferred)

**What:** A TODO at `lld-fixes/build-lld-irix.sh:121` flags that suppressing output emission has not yet been implemented.

**Where:** `lld-fixes/build-lld-irix.sh:121`

**What breaks:** Unknown; the TODO gives no blast radius. The current implementation presumably emits output that a caller does not expect to receive.

**Enforcement:** Convention only (TODO comment). No blocking guard exists.

---

## 2. Runtime Preload Objects

### C2.1 — No GLib/GTK dependencies in preloaded objects

**What:** The runtime objects built by `build-runtime-objects.sh` must have zero GLib or GTK link dependencies because they are preloaded (`LD_PRELOAD` or equivalent) into **every** binary on the system.

**Where:** `scripts/build-runtime-objects.sh:253`

**What breaks:** Any GLib/GTK symbol pulled in at preload time would force GTK's initialization into every process, including daemons and command-line tools that have never heard of GTK. The result is likely crashes or hangs in processes that do not initialize GLib's main loop.

**Enforcement:** Convention only (comment). The build script does not run `ldd` or `readelf -d` on its outputs to assert the absence of `libglib` or `libgtk` in `NEEDED` entries. Enforcement would require a post-build ELF scan guard.

---

## 3. Terminal / PTY (tmux on IRIX)

### C3.1 — Slave PTY must be re-opened without O_NOCTTY after setsid()

**What:** In the forkpty implementation for IRIX, after calling `setsid()` the code must re-open the slave PTY without the `O_NOCTTY` flag so the slave becomes the process's controlling terminal.

**Where:** `patches/packages/tmux/forkpty-irix.c:7`

**What breaks:** Without this, the child process has no controlling terminal. `SIGINT`/`SIGQUIT`/`SIGHUP` delivery from the terminal will not work, causing tmux to be unable to signal foreground processes or detect terminal hang-up. The session will appear to hang or misbehave under job control.

**Enforcement:** Convention only (CRITICAL comment in the source patch). The constraint is in a vendored patch file; any future re-sync from upstream tmux would silently lose it.

---

## 4. Build Quality Gates

### C4.1 — CRITICAL gate must block the pipeline

**What:** The gate system in `mogrix/gates.py` recognizes a `CRITICAL` severity level (lines 212, 226, 276) and treats it as a pipeline-blocking failure.

**Where:** `mogrix/gates.py:212`, `mogrix/gates.py:226`, `mogrix/gates.py:276`

**What breaks:** If CRITICAL findings are not surfaced as hard errors, broken RPMs or ELF defects are silently staged as if they were known-good outputs, potentially poisoning dependent builds in the 13-package tdnf stack or propagating to installed bundles on IRIX.

**Enforcement:** Code-enforced (the gate emits `CRITICAL` strings that are tested at lines 212 and 226; the overall result at line 276 presumably non-zero-exits). Strength depends on whether the caller checks the exit code — not verifiable from the excerpts alone.

---

## 5. Defect Scanning (ELF Image Base)

### C5.1 — ELF image base must be 0x{GOOD_IMAGE_BASE:x}

**What:** Every cross-compiled MIPS binary must have the correct image base address; any binary with the old (incorrect) base is flagged CRITICAL by `scan-defects.py`.

**Where:** `tools/scan-defects.py:56`, `tools/scan-defects.py:121`, `tools/scan-defects.py:127`

**What breaks:** An incorrect image base causes the IRIX dynamic linker to load the binary at the wrong virtual address, producing immediate SIGSEGV or silent memory corruption depending on ASLR absence and segment overlap.

**Enforcement:** Code-enforced (automated scan; CRITICAL results are counted and reported at line 127). Whether the scan is gated into CI or run manually is not stated in the excerpts — the `.github_disabled/` directory suggests CI is currently disabled.

---

## 6. ELF Analyzer

### C6.1 — Translation field identification is incomplete

**What:** The ELF analyzer at `mogrix/analyzers/elf.py:476` has an unresolved TODO: it does not yet identify which translations field to use.

**Where:** `mogrix/analyzers/elf.py:476`

**What breaks:** Analysis results that depend on this field will be silently incomplete or incorrect. Downstream rule generation or defect reports may miss findings.

**Enforcement:** Convention only (TODO comment). No assertion guards the missing code path.

---

### C6.2 — NULL-check and field assignment stubs are unimplemented

**What:** `mogrix/analyzers/elf.py:496` notes that NULL-check plus assignment code for each field has not been written.

**Where:** `mogrix/analyzers/elf.py:496`

**What breaks:** Without NULL guards, the analyzer is susceptible to attribute errors or silent `None` propagation when processing ELF objects that have missing or malformed fields.

**Enforcement:** Convention only (TODO comment).

---

## 7. Rule Generator

### C7.1 — Generated rules require manual review before use

**What:** The rule generator at `mogrix/rule_generator.py:358` emits a `TODO (manual review needed)` annotation on certain generated candidates.

**Where:** `mogrix/rule_generator.py:358`

**What breaks:** If generated candidates in `rules/candidates/` are promoted to `rules/packages/` without review, incorrect or overfitted rules may silently alter spec files for all future builds of that package.

**Enforcement:** Convention only (TODO text embedded in generated output). The pipeline does not block on unreviewed candidates — the `batch-build` command explicitly "never blocks on failures" and moves on.

---

## 8. Source Patches (Third-Party Code)

### C8.1 — Virtual directory handler must use worker filetypes (deferred)

**What:** `patches/packages/worker/nwc_virtualdir.cc:84` defers type-checking to a generic fallback instead of using the worker filetype system.

**Where:** `patches/packages/worker/nwc_virtualdir.cc:84`

**What breaks:** Files that should be handled by a specific virtual-file handler may be processed by the wrong handler, producing incorrect search results or file metadata.

**Enforcement:** Convention only (TODO comment in a vendored patch).

---

### C8.2 — Search thread is not UTF-8 safe

**What:** `patches/packages/worker/searchthread.cc:248` acknowledges that the search implementation is not safe for UTF-8 input.

**Where:** `patches/packages/worker/searchthread.cc:248`

**What breaks:** Byte-level operations on multi-byte UTF-8 sequences can split characters, producing garbled results, missed matches, or out-of-bounds reads when filenames or file contents contain non-ASCII characters.

**Enforcement:** Convention only (TODO comment in a vendored patch). The constraint is inherited from upstream, not introduced by mogrix.

---

## 9. C++ Standard Library (Vendored GCC 9 Headers)

### C9.1 — String conversion results should be constructed in-place (deferred)

**What:** `cross/include/c++/9/ext/string_conversions.h:105` contains an `XXX` note that the result should eventually be constructed in-place rather than via the current allocation strategy.

**Where:** `cross/include/c++/9/ext/string_conversions.h:105`

**What breaks:** The current approach incurs an extra copy or allocation. This is a performance issue, not a correctness constraint. No functional breakage is implied.

**Enforcement:** Convention only (XXX comment in vendored GCC 9 headers). These headers are not modified by mogrix — the annotation originates from the GCC project itself.

---

## 10. Test / CI Filtering

### C10.1 — GLib/GObject/Pango warnings must not fail IRIX integration tests

**What:** `patches/packages/ir8/ir8_test.sh:184` uses a grep filter to suppress `GLib-GObject-(WARNING|CRITICAL)` and `Pango-WARNING` messages from test failure detection.

**Where:** `patches/packages/ir8/ir8_test.sh:184`

**What breaks:** If this filter is removed, the test suite will fail on GLib/Pango warnings that are expected and benign on IRIX (likely missing fontconfig paths or Pango module search failures). Removing the filter would produce false-negative test results.

**Enforcement:** Code-enforced (shell regex filter in test script). The filtering is active and automatic. The risk is that genuinely unexpected CRITICAL messages from GLib would also be suppressed.

---

## Implicit Constraints (Inferred from Architecture)

### IC.1 — Input SRPMs are read-only

The README designates `~/mogrix_inputs/SRPMS/` as read-only ("fetched, read-only"). No code assertion enforces this, but the entire pipeline is designed to write outputs only to `~/mogrix_outputs/` and `~/rpmbuild/`. Violating this by editing input SRPMs directly would silently produce non-reproducible builds.

**Enforcement:** Convention only (directory naming and README documentation).

---

### IC.2 — Staging area must be populated before dependent builds

Cross-compiled packages must be staged to `/opt/sgug-staging/` before any package that depends on them is built. The `mogrix stage` command is the only mechanism for this; skipping it causes the next build to search for headers and libraries that do not exist at the expected paths, producing linker errors that appear to be source-level bugs.

**Enforcement:** Convention only (documented workflow in README). No automatic dependency resolution enforces staging order.

---

### IC.3 — Compat functions must not be injected without checking the catalog first

The MCP server instructions (CLAUDE.md) mandate calling `check_compat` before writing compat implementations to avoid duplicating functions already present in `compat/`. This is a process constraint on the developer workflow, not enforced by mogrix itself.

**Enforcement:** Convention only (CLAUDE.md process rules).

---

## Potential Contradictions

| # | Tension |
|---|---------|
| T1 | **C10.1 vs. C4.1**: The IR8 test filter suppresses GLib `CRITICAL` messages (C10.1), but the gate system (C4.1) treats `CRITICAL` as a pipeline-blocking severity. If a GLib CRITICAL in a test ever reflected a real defect, it would be invisible to both the test harness and the gate. |
| T2 | **C7.1 vs. batch-build "never blocks"**: The rule generator marks candidates as needing manual review (C7.1), but the batch-build pipeline explicitly moves past failures without blocking. There is no enforcement point between "candidate generated" and "candidate promoted to production rules." |
| T3 | **C2.1 and absence of ELF scan**: The no-GLib preload constraint (C2.1) is documented as CRITICAL but has no automated verification. The defect scanner (C5.1) checks image base but there is no evidence it checks `NEEDED` entries for forbidden libraries. These two mechanisms do not cover each other's blind spot. |

---

*6 code-enforced constraints (C4.1, C5.1, C10.1, and partial C3.1 via patch application). 8 convention-only constraints. 3 implicit architectural invariants. 2 open TODOs with unknown blast radius (C6.1, C6.2). CI is currently disabled (`.github_disabled/`), meaning no constraint is continuously verified on commit.*

