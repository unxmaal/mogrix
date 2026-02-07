# Mogrix: SRPM Conversion Engine for IRIX

Mogrix is a deterministic SRPM-to-RSE-SRPM conversion engine that transforms Fedora SRPMs into IRIX-compatible packages. It centralizes all platform knowledge required to adapt Linux build intent for IRIX reality.

## Current Status (2026-02-07)

**Phase 3: COMPLETE — Phase 4 next**

Phase 1 (bootstrap: 14 packages), Phase 1.5 (system libs: 2), Phase 2 (build tools: 6), Phase 3a (crypto: 7), Phase 3b (GNU text tools: 3) — all complete. 41 packages installed on clean `/opt/chroot` via bootstrap tarball + MCP. Next: coreutils, gettext, and other user-facing packages.

---

## Architecture Overview

```
FC40 SRPM  ─────────────────────────────────────────────────────────────┐
   │                                                                     │
   │  (extract)                                                          │
   ▼                                                                     │
┌──────────────────────────────────────────────────────────────────┐    │
│                           MOGRIX                                  │    │
│                                                                   │    │
│  ┌─────────────┐    ┌─────────────┐    ┌─────────────┐          │    │
│  │   Parser    │───▶│ Rule Engine │───▶│   Emitter   │          │    │
│  │ (spec+src)  │    │             │    │ (SRPM+report)│          │    │
│  └─────────────┘    └─────────────┘    └─────────────┘          │    │
│                            │                                      │    │
│            ┌───────────────┼───────────────┐                     │    │
│            ▼               ▼               ▼                     │    │
│     ┌──────────┐    ┌──────────┐    ┌──────────┐                │    │
│     │  Rules   │    │  Header  │    │  Compat  │                │    │
│     │ (YAML)   │    │ Overlays │    │ Sources  │                │    │
│     └──────────┘    └──────────┘    └──────────┘                │    │
│                      (stdarg.h,      (strdup.c,                  │    │
│                       stdio.h...)     getline.c...)              │    │
│                                                                   │    │
│  ┌─────────────────────────────────────────────────────────────┐ │    │
│  │                  Dependency Resolution                       │ │    │
│  │  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐      │ │    │
│  │  │   Resolver   │  │ FedoraRepo   │  │   Fetcher    │──────┼─┼────┘
│  │  │(parse errors)│  │(search/find) │  │  (download)  │      │ │
│  │  └──────────────┘  └──────────────┘  └──────────────┘      │ │
│  │  Parses rpmbuild errors → Finds SRPM URLs → Downloads deps  │ │
│  └─────────────────────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────────────┘
   │
   ▼
RSE SRPM (self-contained, no libdicl dependency)
   │
   ├── Modified spec file
   ├── Original sources + patches
   ├── Injected compat headers (bundled)
   └── Injected compat sources (bundled)
```

**Key principle:** Each converted SRPM contains everything needed to build on IRIX. No external compatibility library required. Automatic linker selection ensures shared libraries work correctly.

---

## Core Concept

**Mogrix is not:** A distro, a ports tree, a replacement RPM system, or a separate compatibility library.

**Mogrix is:** A semantic adapter between Linux SRPM intent and IRIX reality. A deterministic conversion engine with auditable, testable behavior. A single choke point for all IRIX platform knowledge. Self-contained: all compatibility fixes are injected per-package, not linked from a shared library.

**Why no separate libdicl?** libdicl's true value was the institutional memory of how to turn Linux intent into IRIX reality. Mogrix absorbs that knowledge and applies it directly to each package, eliminating the need for a pre-compiled compatibility library.

---

## Build Phases

### Phase 1: Bootstrap (COMPLETE)

All 14 packages cross-compiled and verified on IRIX:

| # | Package | Version | RPMs |
|---|---------|---------|------|
| 1 | zlib-ng | 2.1.6 | 5 |
| 2 | bzip2 | 1.0.8 | 4 |
| 3 | popt | 1.19 | 3 |
| 4 | openssl | 3.2.1 | 4 |
| 5 | libxml2 | 2.12.5 | 3 |
| 6 | curl | 8.6.0 | 1 |
| 7 | xz | 5.4.6 | 5 |
| 8 | lua | 5.4.6 | 4 |
| 9 | file | 5.45 | 4 |
| 10 | sqlite | 3.45.1 | 3 |
| 11 | rpm | 4.19.1.1 | 14 |
| 12 | libsolv | 0.7.28 | 4 |
| 13 | tdnf | 3.5.14 | 8 |
| 14 | sgugrse-release | 0.0.7beta | 2 |

Validated: `rpm -Uvh`, `rpm -qa`, `tdnf repolist`, `tdnf makecache`, `tdnf install`.

### Phase 2: Build Tools (COMPLETE)

| Package | Status | Notes |
|---------|--------|-------|
| m4 1.4.19 | ✅ Verified on IRIX | - |
| perl 5.38.2 | ✅ Verified on IRIX | image-base fix, explicit Provides, -z separate-code |
| autoconf 2.71 | ✅ Verified on IRIX | Generates configure + autom4te cache |
| automake 1.16.5 | ✅ Verified on IRIX | shebang + FindBin fixes |
| bash 5.2.26 | ✅ Verified on IRIX | System readline, libtinfo for termcap, wchar.h overlay |
| libtool 2.4.7 | ✅ Verified on IRIX | shebang + drop_requires fixes |

**Build order**: perl → bash → autoconf → automake → libtool

### Phase 3: User-Facing Packages (COMPLETE)

With Phase 2 complete, IRIX has a full autotools chain.

| Package | Status | Notes |
|---------|--------|-------|
| ncurses 6.4 | ✅ Verified on IRIX | Linker script fix (symlinks not INPUT()) |
| readline 8.2 | ✅ Verified on IRIX | Staged in sysroot, uses system ncurses |
| pkgconf 2.1.0 | ✅ Installed on IRIX | Static build, %zu→%u fix |
| libgpg-error 1.48 | ✅ Installed on IRIX | lock-obj-pub generated on IRIX |
| libgcrypt 1.10.3 | ✅ Installed on IRIX | FIPS removed, ASM disabled |
| libassuan 2.5.7 | ✅ Installed on IRIX | unsetenv/SCM_RIGHTS fixes |
| libksba 1.6.6 | ✅ Installed on IRIX | Clean build |
| npth 1.7 | ✅ Installed on IRIX | POSIX1C pthread_atfork fix |
| gnupg2 2.4.4 | ✅ Installed on IRIX | Key gen + sign + verify working |
| sed 4.9 | ✅ Installed on IRIX | GNU regex bundled, gnulib-tests removed |
| gawk 5.3.0 | ✅ Installed on IRIX | MPFR disabled, git %prep replaced |
| grep 3.11 | ✅ Verified on IRIX | GNU regex bundled, PCRE2 disabled |

### Phase 4: User-Facing Packages (NOT STARTED)

| Package | Status | Notes |
|---------|--------|-------|
| gettext | Not started | Needed for i18n |
| coreutils | Not started | Core user tools (head, tail, cat, ls, etc.) |
| findutils | Not started | find, xargs |
| diffutils | Not started | diff, cmp |
| patch | Not started | Applying patches |
| tar | Not started | Archive handling |
| make | Not started | Build system |

### Long-Term: Modern Browser

Target: WebKitGTK 2.38.x with Epiphany or Surf browser.

---

## What Works Now

| Feature | Status |
|---------|--------|
| Spec file parsing, YAML rule loading, rule engine | Done |
| Header overlay system (16 generic headers) | Done |
| Compat source injection (30+ functions) | Done |
| SRPM extraction and repackaging | Done |
| CLI (analyze, convert, build, fetch, stage, lint, validate-spec) | Done |
| Dependency resolution + Fedora SRPM fetching | Done |
| Shared library cross-compilation (GNU ld) | Done |
| Dynamic executables (LLD 18 + /lib32/rld) | Done |
| Spec validation (specfile library, integrated into convert) | Done |
| RPM linting (rpmlint with IRIX-specific config) | Done |
| Source-level static analysis (ripgrep, rules-integrated) | Done |
| 79 package rules + 1 class rule | Done |
| Rule auditing (`mogrix audit-rules`) | Done |
| 127 tests, all passing | Done |
| Bootstrap tarball (`scripts/bootstrap-tarball.sh`) | Done |
| MCP-based IRIX testing (no SSH) | Done |

---

## Key Technical Decisions

| Decision | Rationale |
|----------|-----------|
| LLD 18 for executables, GNU ld + `-z separate-code` for shared libs | LLD for correct relocations; GNU ld with forced 3-segment layout for rld |
| `--image-base=0x1000000` for all executables | Default 0x10000 gives only 1.8MB brk heap; 0x1000000 gives 176MB |
| `/lib32/rld` as dynamic linker | IRIX requires this interpreter, not `/usr/lib32/libc.so.1` |
| dlmalloc via mmap (auto-injected) | IRIX brk() heap limited to 176MB; mmap accesses 1.2GB |
| funopen instead of fopencookie | fopencookie crashes on IRIX |
| Iterative vasprintf | IRIX vsnprintf(NULL,0) returns -1 (pre-C99) |
| Per-package compat injection | No shared libdicl dependency; each SRPM is self-contained |
| Symlinks not linker scripts | IRIX rld can't load GNU ld scripts (`INPUT(-lfoo)`) |
| System libs over bundled | Full sysroot available; don't copy SGUG-RSE's bootstrap shortcuts |
| `%u` not `%zu` for size_t | IRIX libc is pre-C99; `%zu` corrupts varargs → SIGSEGV |
| `explicit_bzero` for wipememory | Volatile fptr static initializers crash on IRIX rld |

---

## Success Criteria

1. **Functional:** Can convert and build tdnf and all dependencies ✓
2. **Self-contained:** No libdicl dependency; all compat code is bundled ✓
3. **No bootstrap required:** Pure cross-compilation from source ✓
4. **Auditable:** Every transformation is logged and explainable ✓
5. **Extensible:** Adding new rules requires only YAML ✓
6. **Reproducible:** Same input SRPM + rules = identical output SRPM ✓
7. **Package manager on IRIX:** tdnf works ✓
8. **Build tools on IRIX:** autoconf/automake/libtool ✓

---

## Reference Documents

| Document | Purpose |
|----------|---------|
| `HANDOFF.md` | Session continuity, current blockers |
| `rules/INDEX.md` | Rules reference, per-package problem guide |
| `rules/generic.yaml` | Universal rules for all packages |
| `rules/methods/before-you-start.md` | Checklist + SGUG-RSE assumptions warning |
| `compat/catalog.yaml` | Compat function registry |
