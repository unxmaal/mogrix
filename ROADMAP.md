# Mogrix Roadmap

## Current Status

**v14 rebuild**: 234+ packages cross-compiled for IRIX 6.5 (500+ RPMs). 161+ app bundles as self-extracting `.run` installers. Full GNU userland, crypto stack, X11 GUI apps, Qt5, GTK3, WebKit browser (ir8).

**This session**: 58 bundles + 8 suites created. Tree-sitter AST transform engine added. Bundler now rules-aware. MCP hardening complete.

---

## Highest Priority: libc++ Migration

Replace GCC's libstdc++ with LLVM's libc++ as the C++ standard library.

**Why**: We're fighting an endless stream of bugs from compiling GCC's runtime library with clang — ODR violations, visibility mismatches, broken exceptions, broken rebasing, dual-ABI headaches, GCC 9.2 vs 9.5 header mismatches. Every fix exposes another issue. The fundamental problem is using compiler A to compile compiler B's runtime.

**Why now**: We don't need SGUG-RSE ABI compatibility. Our bundles are self-contained. There's no technical reason to use GCC's libstdc++ — it was an inherited assumption from the SGUG-RSE project that we never questioned.

**What libc++ gives us**:
- Compiled by clang, for clang — no compiler mismatch
- C++20/C++23 features are first-class (eliminates most polyfill headers)
- Single ABI — no cow/SSO dual-ABI, no ODR violations
- Exception handling may work (libc++ + LLVM libunwind is a tested stack)
- Rebasing should work (no GCC-specific hidden assumptions)
- Smaller, cleaner codebase

**Work required**:
- [ ] Port libc++ to MIPS N32 IRIX (platform headers, threading via pthreads, locale)
- [ ] Port LLVM libunwind to IRIX (signal handling, stack walking, MIPS N32 register context)
- [ ] Build both with clang 16 cross-compiler
- [ ] Add `-stdlib=libc++` to irix-cxx wrapper
- [ ] Rebuild all packages (full v16 rebuild)
- [ ] Remove GCC 9.5 source tree, build-libstdcxx.sh, cow-fs_* workarounds, polyfill headers that libc++ natively supports
- [ ] Fix or remove mrqs (may not be needed if libc++ rebases cleanly)

**Risk**: libc++ on MIPS N32 may need porting work for IRIX-specific threading (sproc/pthread model), locale (IRIX locale APIs), and possibly missing POSIX functions. But these are well-defined problems, unlike the open-ended "why does clang compile GCC's code wrong" investigation.

**Bridge**: Current libstdc++ (with cow-fs_* exclusion + skip_mrqs_rebase) works for all packages. The migration can be incremental — build libc++ alongside libstdc++, test packages one at a time.

---

## Near-Term (Next 1-2 Sessions)

### Unblock Motif Apps
- [ ] Copy Motif headers from IRIX to sysroot (`/usr/include/Xm/` → `/opt/irix-sysroot/usr/include/Xm/`)
- [ ] Copy libXm.so from IRIX to sysroot
- [ ] Build **nedit** (classic Motif text editor)
- [ ] Build **xnedit** (XNEdit Motif editor with Xt constructor workaround)
- [ ] Build **emacs** (needs Motif or X toolkit)
- **Sysroot script fixed**: `create-irix-sysroot.sh` now follows symlinks from `/usr/include/` to external dirs (e.g., `/usr/Motif-1.2/`)

### Rust TUI Debugging
- [ ] Revert sysroot `realpath` patch (causes SIGSEGV in termusic-server)
- [ ] Build minimal `realpath` test binary to isolate the crash
- [ ] Fix `realpath(path, NULL)` for IRIX — either sysroot patch or per-app workaround
- [ ] Build complete Rust compat archive (all compat .c files, not cherry-picked)
- [ ] Find a simple Rust TUI app to validate crossterm/ratatui independently
- **Bugs fixed this session**: FIONBIO/FIONREAD/FIOCLEX ioctl encoding, O_CLOEXEC/O_DIRECTORY/O_NOFOLLOW flag collisions

### Bundle Deployment
- [ ] Deploy all 58 bundles to IRIX via `deploy-test.sh`
- [ ] Run `test_bundle` on each to verify
- [ ] Fix remaining bundle failures (vim-enhanced naming resolved, tree-pkg resolved)

---

## Medium-Term (Next Month)

### Blocked Packages (12 remaining)
| Package | Blocker | Priority |
|---------|---------|----------|
| nedit | Motif headers (fix in progress) | High |
| xnedit | Motif headers (fix in progress) | High |
| emacs | Motif/X toolkit | High |
| python3 | FC40 uses python3.12, no matching SRPM | High (unblocks gobject-introspection) |
| gobject-introspection | Needs python3 | Medium |
| createrepo_c | Needs rpm-devel + other deps | Medium |
| expect | Needs tcl-devel | Low |
| gd | Needs fontconfig-devel chain | Low |
| openjade | Missing opensp-devel | Low |
| libdb | Build system issues | Low |
| binutils | Circular dep / needs special handling | Low |
| elfutils | Circular dep with rpm | Low |

### QuickJS on IRIX
- [ ] Investigate QuickJS as a lightweight JavaScript engine for IRIX
- [ ] QuickJS is pure C, no JIT, no V8, no libuv — compiles with irix-cc
- [ ] Could enable: lightweight web browser (QuickJS + minimal HTML renderer), Node.js-like scripting, possibly run bundled TypeScript apps
- [ ] Evaluate: can OpenCode's TypeScript be bundled to run on QuickJS?
- [ ] Evaluate: QuickJS + dillo = JS-capable browser lighter than ir8/WebKit?

### C++20 Polyfill Header (DONE — bridge until libc++ migration)
- [x] Created systemic polyfill headers at `compat/include/mogrix-compat/generic/`
- [x] Implements: ranges, semaphore, source_location, utility_polyfill.h (to_string, cmp_*)
- [x] All guarded with `__cpp_lib_*` feature test macros
- [x] Force-included by `irix-cxx` wrapper
- [x] Unblocked btop (first C++20 app on IRIX)
- [ ] **Will be eliminated by libc++ migration** — libc++ has native C++20 support

### btop for IRIX (DONE)
- [x] Platform backend: `patches/packages/btop/btop_collect.cpp` (sysmp, PIOCPSINFO, statvfs, getpwuid_r)
- [x] Rules: `rules/packages/btop.yaml` (8 prep_commands fixes)
- [x] Fixes applied: cow-fs_* ODR exclusion, graph_symbols safe find, size clamping, to_string %f, tty_mode defaults
- [x] Production bundle deployed and verified on IRIX 6.5 IP30 (screenshot confirmed)
- [x] `skip_mrqs_rebase: true` (workaround until libc++ migration)

### Toolchain Gaps (from assumptions audit)

**dlmalloc: opt-in, not opt-out**
- [ ] Investigate: does IRIX libc malloc actually cause problems? Profile with/without dlmalloc.
- [ ] If no evidence of problems: flip default to OFF, make `MOGRIX_USE_DLMALLOC=1` opt-in
- [ ] dlmalloc causes cross-heap crashes with IRIX native Motif (nedit), and mmap(0) returning NULL with 77+ loaded libraries required a custom fix. The blanket usage creates more problems than it solves unless IRIX malloc is demonstrably broken.

**-mxgot: per-package, not global**
- [ ] Add `xgot: false` as a per-package rules flag (default: off)
- [ ] Only enable for libraries with large GOTs (WebKit, libicudata, libstdc++)
- [ ] Remove `-mxgot` from `irix-cc` global flags
- [ ] -mxgot generates 3-instruction GOT access (vs 1), produces 4.6x more GOT entries, increases code size. Most packages don't need it.

**Post-link fixups → LLD patches**
- [ ] Move fix-anon-relocs logic into LLD source (anonymous R_MIPS_REL32 repointing)
- [ ] Move strip-verneed logic into LLD source (GNU version section suppression)
- [ ] These are fragile Python scripts that swallowed errors silently for months (77 staged libs went unfixed). They should be in the linker where errors are caught at link time.

**Sysroot provenance**
- [ ] Verify current `/opt/irix-sysroot` against the target IRIX machine
- [ ] Document differences (additional SGI packages installed, not ABI-breaking)
- [ ] Consider: regenerate sysroot from target machine with `create-irix-sysroot.sh`

### Infrastructure
- [ ] Rebuild staleness fix: use content hashes for cross-package inputs (generic.yaml touches invalidate ALL packages)
- [ ] Automate libmogrix_compat.so build in `setup-cross` (currently manual)
- [ ] Content-hash based SRPM cache invalidation
- [ ] `mogrix depmap` command for persistent dependency mapping

### Source Transform Evolution
- [ ] Tree-sitter policy model: define stripping policies (telemetry, analytics, tracking) that auto-derive transform rules
- [ ] More AST templates: `remove_decorator` (Python), `remove_feature_gate` (Rust), `remove_ifdef_block` (C/C++)
- [ ] `mogrix transform --diff` mode showing structural before/after
- [ ] Wire tree-sitter into RPM build pipeline for structural C/C++ source patching — replace brittle `safepatch` sed commands in `prep_commands` with AST queries that survive upstream refactoring (e.g., telescope `d_type` → `stat()` fix, `clock_gettime` → `gettimeofday` replacements)

---

## Long-Term (Exploration)

### New Language Runtimes on IRIX
- [ ] **QuickJS**: Pure C JS engine — lowest-hanging fruit for JS on IRIX
- [ ] **Lua 5.4**: Already cross-compiled (used by rpm). Could power scripting/game engines.
- [ ] **Rust on IRIX**: Working for CLI apps (ripgrep). TUI apps blocked on realpath + crossterm issues. Audio apps blocked on symphonia/SDL2 interaction.
- [ ] **Go on IRIX**: Working for servers (irix_httpd). Blocked on async preemption crash (SIGURG → SIGSEGV). Cooperative preemption works.

### New Apps
- [ ] **SDL2 audio backend**: IRIX dmedia (AL library) backend confirmed working. Enables music players, games.
- [ ] **ElectroPaint**: Decompiled + Rust rewrite in progress. Needs visual tuning to match original.
- [ ] **Games**: Angband, Crawl, chocolate-doom (all have rules, need testing)
- [ ] **Development tools**: GDB (crashes with libstdc++ issue), cmake (not built in v14)

### Toolchain
- [ ] **libc++ migration** (see Highest Priority above)
- [ ] LLD: suppress GNU version section OUTPUT (keep internal objects alive)
- [ ] .eh_frame: move to RW LOAD segment (may be resolved by libc++ + libunwind migration)
- [ ] Fresh sysroot from actual IRIX machine (current one may be from a different SGI)
- [ ] Fix mrqs rebase for new libstdc++ (or determine if libc++ rebases correctly, making this moot)

---

## Completed Milestones

- [x] v14 rebuild: 234+ packages (March 2026)
- [x] 161+ app bundles as self-extracting `.run` installers
- [x] First GTK3 GUI app on IRIX (gtkterm)
- [x] WebKit/ir8 browser rendering HTTP pages on IRIX
- [x] Qt5 (5.15.13) cross-compiled
- [x] `mogrix transform` — general-purpose declarative source transformation
- [x] Tree-sitter AST transform engine with 7 language grammars
- [x] opencode-strip: 22 telemetry targets stripped (hybrid AST+text)
- [x] ElectroPaint reverse engineered via Ghidra + Rust rewrite
- [x] MCP hardening: enforced search-first behavior, 1M context tuning
- [x] Bundler rules-aware: RPM name resolution, upstream source hints, blocked package messages
- [x] Go IRIX port: HTTP server, stdlib tests (500+ pass)
- [x] Rust IRIX port: ripgrep working, libc binding bugs fixed (FIONBIO, O_CLOEXEC)
- [x] Sysroot script fixed for Motif symlinks
