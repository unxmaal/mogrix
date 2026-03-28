# Mogrix Roadmap

## Current Status

**v14 rebuild**: 234+ packages cross-compiled for IRIX 6.5 (500+ RPMs). 161+ app bundles as self-extracting `.run` installers. Full GNU userland, crypto stack, X11 GUI apps, Qt5, GTK3, WebKit browser (ir8).

**This session**: 58 bundles + 8 suites created. Tree-sitter AST transform engine added. Bundler now rules-aware. MCP hardening complete.

---

## Highest Priority: Prefix Change + libc++ Cutover

### 1. Change install prefix from /usr/sgug to /opt/mogrix

**Status**: Planned. See `sgug_prefix_change.md` for full details.

`/usr/sgug` is an SGUG-RSE convention. Staging accumulated SGUG-RSE artifacts (GCC 9 C++ headers) that conflicted with LLVM 22 libc++. Changing to `/opt/mogrix` gives clean separation and eliminates the contamination vector.

- [ ] Mechanical sed across ~550 references in YAML rules, Python, shell scripts, cmake
- [ ] Update rpmmacros.irix, compiler wrappers, cli.py, bundle.py
- [ ] New staging path: `/opt/mogrix-staging/` (flat, no nested prefix)
- [ ] Full rebuild-all after prefix change

### 2. libc++ Migration (DONE — integration remaining)

**Completed**:
- [x] LLVM 22.1.2 clang + LLD built and installed (`/opt/cross/`)
- [x] libc++ 22 built with iostream, locale, filesystem, ranges (56/56 sources, 875 exports)
- [x] libc++abi 22 built with exceptions (libgcc_s unwinder, not LLVM libunwind)
- [x] C++ exceptions working on IRIX (6/6 tests pass)
- [x] btop built and running on IRIX with libc++ + LLVM 22
- [x] Locale stubs (xlocale_irix.h) for IRIX
- [x] LLD IRIX patches ported to LLVM 22 (11 patches, `001-lld-irix-support.patch`)
- [x] `irix-cxx-libcxx` compiler wrapper
- [x] `strip-eh-relocs` post-link tool

**Remaining** (after prefix change):
- [ ] Make `irix-cxx-libcxx` the default `%{__cxx}` in rpmmacros
- [ ] Rebuild all C++ packages with libc++
- [ ] Remove libstdc++ build infrastructure, GCC 9 source, polyfill headers
- [ ] Retire `irix-cxx` (libstdc++ wrapper)

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
