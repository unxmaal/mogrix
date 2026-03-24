# Mogrix Roadmap

## Current Status

**v14 rebuild**: 234+ packages cross-compiled for IRIX 6.5 (500+ RPMs). 161+ app bundles as self-extracting `.run` installers. Full GNU userland, crypto stack, X11 GUI apps, Qt5, GTK3, WebKit browser (ir8).

**This session**: 58 bundles + 8 suites created. Tree-sitter AST transform engine added. Bundler now rules-aware. MCP hardening complete.

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
- [ ] LLD: suppress GNU version section OUTPUT (keep internal objects alive)
- [ ] .eh_frame: move to RW LOAD segment (needed for C++ shared libraries on IRIX)
- [ ] Fresh sysroot from actual IRIX machine (current one may be from a different SGI)

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
