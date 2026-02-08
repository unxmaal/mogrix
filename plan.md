# Mogrix: SRPM Conversion Engine for IRIX

Mogrix is a deterministic SRPM-to-RSE-SRPM conversion engine that transforms Fedora SRPMs into IRIX-compatible packages. It centralizes all platform knowledge required to adapt Linux build intent for IRIX reality.

## Current Status (2026-02-08)

**Phase 5: IN PROGRESS — 63 source packages cross-compiled for IRIX. Tier 0-2 building.**

All phases through 4c complete (41 packages). Phase 5 (library foundation toward aterm) is underway with 22 additional packages: 15 installed, 7 staged. New this session: gettext, zstd, fontconfig installed and verified on IRIX. Key systemic fix: PKG_CONFIG_SYSROOT_DIR added to rpmmacros.irix. Cairo 1.18.0 blocked (uses meson). aterm's direct deps identified: libAfterImage-devel, X11 libs (likely in sysroot), make, chrpath.

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

### Phase 4a: User-Facing Utilities (COMPLETE)

| Package | Status | Notes |
|---------|--------|-------|
| less 643 | ✅ Verified on IRIX | Skip fsync AC_TRY_RUN patch, ac_cv override |
| which 2.21 | ✅ Verified on IRIX | getopt_long compat only |
| gzip 1.13 | ✅ Verified on IRIX | Source renumbering, gnulib-tests removal, nls-disabled |
| diffutils 3.10 | ✅ Verified on IRIX | gnulib select override, %td fix |
| patch 2.7.6 | ✅ Verified on IRIX | posix_spawn compat (full impl), skip SELinux patch |

### Phase 4b: Build/System Utilities (COMPLETE)

| Package | Status | Notes |
|---------|--------|-------|
| make 4.4.1 | ✅ Verified on IRIX | --disable-load (--export-dynamic crashes rld), --without-guile |
| findutils 4.9.0 | ✅ Verified on IRIX | Out-of-tree build (_configure macro fix), gnulib-tests post-autoreconf |
| tar 1.35 | ✅ Verified on IRIX | --disable-year2038, %zu fixes, brace expansion fix |

### Phase 4c: Core Utilities (COMPLETE)

| Package | Status | Notes |
|---------|--------|-------|
| coreutils 9.4 | ✅ Verified on IRIX | Source renumbering, pthread_sigmask, strcasestr compat, seq disabled |

Skipped utilities: kill, uptime, stdbuf, pinky, who, users, seq (seq: IRIX printf can't handle `%Lg` long double format).

### Phase 5: Library Foundation (IN PROGRESS — 22 packages)

Derived from `mogrix roadmap aterm` analysis. Building in tiers of increasing complexity. Tier 0 + Tier 1 complete. Tier 2 in progress.

**Completed:** pcre2, symlinks, tree-pkg, oniguruma, libffi, tcl, flex, chrpath, libpng, bison, libunistring, gettext, zstd, fontconfig, freetype (15 installed/built).
**Staged:** expat, nettle, libtasn1, fribidi, libjpeg-turbo, pixman, uuid (7 staged).

#### Current blockers
- **FC40 GNOME stack uses meson**: cairo 1.18, harfbuzz, pango, glib2, p11-kit all need `%meson`. We have `cross/meson-irix-cross.ini` but no `%meson` RPM macro. Options: set up meson macros, find autotools versions, or skip.
- **libAfterImage**: aterm's key dep. Not in Fedora repos. Need to find source.

#### Remaining Tier 2-3 targets (autotools)
| Package | Build System | Unblocks |
|---------|-------------|----------|
| gd | autotools | graphics, emacs |
| pcre | autotools | glib2, grep (already have pcre2) |
| gnutls | autotools | libarchive, git, p11-kit |
| elfutils | autotools | binutils, debuginfo |
| groff | autotools | man pages |
| libarchive | autotools | cmake, rpm |
| gtk2 | autotools | GUI apps |

### Phase 6: Development Tools (future)

| Package | Status | Notes |
|---------|--------|-------|
| binutils | Blocked on tier 2 | Assembler, linker, objdump |
| gcc | Blocked on binutils | Native compiler for IRIX |

### Long-Term: Modern Browser

Target: WebKitGTK 2.38.x with Epiphany or Surf browser.

---

## Roadmap Enhancements (TODO)

The `mogrix roadmap` command generates dependency graphs but currently requires manual analysis to produce actionable build plans. The following analysis was done by hand for Phase 5 and should be automated:

1. **Ready-to-build detection**: For each HAVE_RULES package, check if all its BuildRequires are satisfied (built or have rules). These are immediately buildable.

2. **Blocker impact scoring**: Count how many HAVE_RULES (or NEED_RULES) packages each unsatisfied dependency blocks. Rank by impact — `gettext` blocks 12, `meson` blocks 9, `bison` blocks 8, etc.

3. **Tier classification**: Automatically group packages into tiers:
   - Tier 0: Have rules + all deps satisfied → build now
   - Tier 1: Leaf packages (only need themselves, no new transitive deps)
   - Tier 2: Small chains (1-9 new deps) ranked by unblock impact
   - Tier 3: Heavy chains (10+ new deps)

4. **Glob pattern drops for impossible ecosystems**: Already implemented (rust-*, golang-*, ghc-*). Will need periodic refinement as we encounter more impossible-on-IRIX packages during real porting.

5. **Build plan export**: `mogrix roadmap <target> --plan` could emit a structured plan file with tiers, ready-to-build lists, and fetch commands, instead of the flat numbered list.

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
| Roadmap: transitive dep graph + topo sort (`mogrix roadmap`) | Done |
| Roadmap: glob pattern drops for impossible ecosystems | Done |
| 80+ package rules + 1 class rule | Done |
| Rule auditing (`mogrix audit-rules`) | Done |
| Rule scoring (`mogrix score-rules`) | Done |
| 153 tests, all passing | Done |
| Bootstrap tarball (`scripts/bootstrap-tarball.sh`) | Done |
| MCP-based IRIX testing (no SSH) | Done |
| 63 source packages cross-compiled for IRIX | Done |
| Full GNU userland (coreutils, findutils, tar, make) | Done |
| Package manager (tdnf) functional on IRIX | Done |

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
| Disable `--export-dynamic` features | IRIX rld crashes with large dynamic symbol tables (468+ entries) |
| `-lpthread` for pthread_sigmask | IRIX has it in libpthread, not libc; gnulib replacement needs it |

---

## Success Criteria

1. **Functional:** Can convert and build tdnf and all dependencies ✓
2. **Self-contained:** No libdicl dependency; all compat code is bundled ✓
3. **No bootstrap required:** Pure cross-compilation from source ✓
4. **Auditable:** Every transformation is logged and explainable ✓
5. **Extensible:** Adding new rules requires only YAML ✓
6. **Reproducible:** Same input SRPM + rules = identical output SRPM ✓
7. **Package manager on IRIX:** tdnf works ✓
8. **Build tools on IRIX:** autoconf/automake/libtool/make ✓
9. **Full GNU userland:** coreutils/findutils/tar/sed/gawk/grep ✓
10. **Crypto stack:** gnupg2 key gen + sign + verify ✓

---

## Reference Documents

| Document | Purpose |
|----------|---------|
| `HANDOFF.md` | Session continuity, current blockers |
| `rules/INDEX.md` | Rules reference, per-package problem guide |
| `rules/generic.yaml` | Universal rules for all packages |
| `rules/methods/before-you-start.md` | Checklist + SGUG-RSE assumptions warning |
| `compat/catalog.yaml` | Compat function registry |
