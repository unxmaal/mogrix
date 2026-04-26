# Decision Log: Mogrix

## DEC-001 — Cross-compile via clang/LLD rather than native IRIX compilation

**What:** Use a Linux-hosted clang 16+ / LLD 18 toolchain targeting MIPS N32 ABI instead of compiling natively on IRIX.

**When:** Project inception (unknown exact date).

**Why:** Documented in README: "SGUG-RSE's build process requires native IRIX compilation. Mogrix enables cross-compilation from Linux, making it faster and more accessible." SGI IRIX was discontinued in 2006; native hardware is slow and scarce.

**Alternatives considered:** Native compilation on IRIX (SGUG-RSE approach) — explicitly cited as the prior art and the alternative being replaced.

**Status:** In effect. The `--cross` flag is the primary recommended workflow; native rpmbuild is described only as "for testing on Linux."

---

## DEC-002 — Fork and patch LLD 18 for IRIX support

**What:** Rather than using an unmodified upstream LLD, a patched LLD 18 is built from source via `scripts/build-lld-irix.sh`. The patch directory `lld-fixes/` (6 files, 657 lines) contains the modifications.

**When:** Unknown. Introduced when LLD was adopted as the linker.

**Why:** Upstream LLD does not support IRIX ELF targets. The patch was necessary to produce working MIPS N32 binaries.

**Critical constraint documented in source:** `lld-fixes/build-lld-irix.sh:106`:
> `CRITICAL: Must use if (skipPhdr) {} else if (isMain) — NOT !skipPhdr && isMain.`

This comment records a subtle boolean-logic bug that was found and fixed in the LLD patch — the two forms are not equivalent under LLVM's control-flow, and the wrong form produced broken output.

**Alternatives considered:** GNU BFD ld — retained as a fallback (`GNU Binutils 2.41, targeting mips-sgi-irix6.5`) but LLD is primary. `lld-fixes/build-lld-irix.sh:121` also has a `TODO: suppress output emission instead`, indicating one lld-fixes workaround is provisional.

**Status:** In effect. LLD 18 with patches is the required linker.

---

## DEC-003 — YAML-based declarative rules engine instead of imperative scripts

**What:** All SRPM transformations are expressed as YAML rule files in `rules/packages/` and `rules/generic.yaml`. No per-package Python/shell code is written; new packages are added by writing YAML.

**When:** Unknown (project design decision).

**Why:** README states: "Patterns defined in YAML rules — adding a new check requires no code changes." The audit/score/validate-rules CLI commands and the 365-file `rules/` directory confirm this is the primary extension mechanism.

**Alternatives considered:** Not documented. The existence of `mogrix/rule_generator.py` (with a `TODO (manual review needed)` at line 358) suggests that rule generation from source analysis was considered as an automated step.

**Status:** In effect.

---

## DEC-004 — Compat library of 35+ POSIX/C99 functions injected at build time

**What:** Missing functions (openat family, posix_spawn, vasprintf, funopen, open_memstream, explicit_bzero, etc.) are implemented in `compat/` (80 files, 7,575 lines) and injected per-package via `inject_compat_functions` rules.

**When:** Unknown. Required early since nearly every modern package uses functions IRIX 6.5 lacks.

**Why:** IRIX 6.5's libc predates POSIX.1-2008. Functions like openat, posix_spawn, and open_memstream do not exist in the system libc.

**Alternatives considered:** Not documented. The README notes "Drop-in implementations … automatically injected into packages that need them," implying the prior approach was manual (likely per-package patching, now replaced).

**Status:** In effect. The `check_compat` MCP tool and CLAUDE.md mandate consulting the compat catalog before writing new implementations.

---

## DEC-005 — dlmalloc linked into executables only (not shared libraries)

**What:** dlmalloc, a mmap-based allocator, is linked into IRIX executables by the linker wrapper, but not into shared libraries.

**When:** Unknown.

**Why:** IRIX's native malloc does not use mmap, causing issues with modern software that expects mmap-backed allocation. Linking dlmalloc into shared libraries would cause allocator conflicts when multiple libraries are loaded. `scripts/build-runtime-objects.sh:253` documents a related constraint:
> `CRITICAL: No GLib/GTK deps allowed — this is preloaded into ALL binaries.`

**Alternatives considered:** Not documented.

**Status:** In effect. The linker wrapper handles injection automatically.

---

## DEC-006 — App bundles that coexist with SGUG-RSE (no /usr/sgug replacement)

**What:** `mogrix bundle` creates self-extracting `.run` installers that install to a user-chosen directory, with trampolines in `~/apps/bin/`. They do not replace or conflict with SGUG-RSE's `/usr/sgug`.

**When:** Unknown. Post-initial-build (bundles require working cross-compiled packages).

**Why:** README explicitly: "coexist with SGUG-RSE — no /usr/sgug replacement needed." SGUG-RSE users should not have to choose between the two environments.

**Alternatives considered:** Replacing SGUG-RSE (explicitly rejected). System-level RPM installation (available separately via the staged RPMs).

**Status:** In effect. 161+ app bundles produced.

---

## DEC-007 — N32 ABI (MIPS III instruction set) as the sole target ABI

**What:** All IRIX packages are built for the N32 ABI, not O32 or N64.

**When:** Project inception.

**Why:** README: "Builds use the N32 ABI (MIPS III instruction set)." N32 is the standard modern ABI for IRIX 6.5 on MIPS R4000+ hardware (O2, Octane, Origin, Fuel, Tezro). N64 has limited IRIX library support; O32 is the legacy 32-bit ABI.

**Alternatives considered:** O32 and N64 — not pursued.

**Status:** In effect.

---

## DEC-008 — Staging area at /opt/sgug-staging for dependency chaining

**What:** Cross-compiled RPMs are extracted to `/opt/sgug-staging/` so that dependent packages can find headers (`-I/opt/sgug-staging/usr/sgug/include`) and libraries (`-L/opt/sgug-staging/usr/sgug/lib32`) at build time.

**When:** Unknown. Required once the first multi-package dependency chain was needed (tdnf stack, 13 packages, is the documented example).

**Why:** rpmbuild cannot resolve MIPS RPM dependencies on a Linux host. Staging extracts the built artifacts into a well-known sysroot-like location that compiler wrappers point to.

**Alternatives considered:** Not documented.

**Status:** In effect.

---

## DEC-009 — forkpty on IRIX requires re-opening slave without O_NOCTTY after setsid()

**What:** The tmux compat patch (`patches/packages/tmux/forkpty-irix.c:7`) records a required ordering: after `setsid()`, the slave PTY must be re-opened *without* `O_NOCTTY` to acquire a controlling terminal.

**When:** Unknown (discovered during tmux port).

**Why:** Comment:
> `CRITICAL: After setsid(), must re-open slave WITHOUT O_NOCTTY so it [acquires controlling terminal]`

IRIX's PTY semantics differ from Linux: the `O_NOCTTY` flag on the initial open prevents terminal assignment, but POSIX requires the first open of a terminal after `setsid()` to set the controlling terminal. The re-open without the flag is the workaround.

**Alternatives considered:** Not documented.

**Status:** In effect in the tmux patch.

---

## DEC-010 — Python + uv + hatchling as the build and package management system

**What:** Mogrix is a Python project managed by `uv` with `hatchling` as the build backend. All commands are invoked as `uv run mogrix`.

**When:** Unknown, but pyproject.toml structure is consistent with a 2023–2024 project using current Python tooling.

**Why:** Not explicitly documented. `uv` is the modern fast Python package manager; `hatchling` is the standard PEP 517/518 backend.

**Alternatives considered:** Not documented.

**Status:** In effect.

---

## DEC-011 — mcm-engine as a local file dependency

**What:** `pyproject.toml` references `mcm-engine` as `file:///home/edodd/projects/github/unxmaal/mcm-engine` — an absolute local path, not a PyPI or git dependency.

**When:** Unknown.

**Why:** Not documented. The absolute path to `/home/edodd/` suggests this is a co-developed local library not yet published.

**Alternatives considered:** Publishing to PyPI or using a git URL reference.

**Status:** In effect, but a portability concern — other contributors cannot use this path. The `allow-direct-references = true` setting in `[tool.hatch.metadata]` is required to permit this reference.

---

## DEC-012 — CI disabled (.github_disabled/)

**What:** The `.github_disabled/` directory (1 file, 101 lines) contains a CI configuration that is not active. No CI system is detected in the inventory.

**When:** Unknown — the directory name implies it was explicitly disabled rather than never created.

**Why:** Not documented. Likely due to the complexity of the cross-compilation environment (requires a patched LLD, IRIX sysroot, and binutils not available on standard CI runners).

**Alternatives considered:** Not documented.

**Status:** CI remains disabled.

---

## DEC-013 — tree-sitter AST parsing as an optional dependency group

**What:** `pyproject.toml` defines an `ast` optional dependency group with tree-sitter and grammars for TypeScript, JavaScript, Python, C, C++, Go, and Rust. It is not in the default `dependencies` list.

**When:** Unknown. Likely added when `mogrix transform` or source analysis features were developed.

**Why:** tree-sitter and its grammars are large and slow to install. Most users of the SRPM conversion pipeline do not need AST-level source analysis.

**Alternatives considered:** Making tree-sitter a required dependency — rejected in favor of keeping the base install lightweight.

**Status:** In effect as an opt-in group.

---

## DEC-014 — General-purpose source transform engine (not IRIX-specific)

**What:** `mogrix transform` applies YAML-declared transforms to *any* source tree, not just IRIX packages. The documented first non-IRIX use case is stripping telemetry from the `opencode` CLI tool for internal corporate deployment.

**When:** Unknown. Added after the IRIX pipeline was established (the README presents it as a secondary capability).

**Why:** The rules engine was already general-purpose; exposing it as a standalone command required minimal additional work. The `opencode` use case is explicitly called out as the first application.

**Alternatives considered:** Keeping transforms IRIX-only. Rejected because the underlying mechanism is not IRIX-specific.

**Status:** In effect.

