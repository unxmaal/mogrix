# Mogrix: SRPM Conversion Engine for IRIX

Mogrix is a deterministic SRPM-to-RSE-SRPM conversion engine that transforms Fedora SRPMs into IRIX-compatible packages. It centralizes all platform knowledge required to adapt Linux build intent for IRIX reality.

## Current Status (2026-02-16)

**120+ source packages cross-compiled for IRIX (300+ RPMs). 20 bundles (5 suites + 15 individual) rebuilt from scratch and verified on IRIX.** Qt5 5.15.13 running — `qVersion()` returns "5.15.13". Weechat TLS verified on real IRIX hardware (irc.libera.chat:6697). dlmalloc hardened: exe-only linking, thread-safe spin locks (MIPS ll/sc), high-fd /dev/zero fix. Self-extracting .run bundles. `-z norelro` added to irix-ld for all executables (IRIX rld doesn't support GNU_RELRO).

All phases through 4c complete (41 packages). Phase 5+ complete with 60+ library/app packages including Qt5. `mogrix batch-build` automates multi-package build pipelines. `mogrix bundle` creates self-contained app bundles (.tar.gz or self-extracting .run) for IRIX. 145+ package rule files. Suites: mogrix-essentials, mogrix-extras, mogrix-net, mogrix-smallweb, mogrix-fun. Individual bundles: bash, bc, bitlbee, dmenu, groff, jq, man-db, rxvt-unicode, st, tcsh, tinc, tmux, vim-enhanced, weechat, wget2 — all tested on IRIX.

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
| perl 5.38.2 | ✅ Verified on IRIX | image-base fix, explicit Provides |
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

### Phase 5: Library Foundation + Apps (COMPLETE — 23 packages)

Derived from `mogrix roadmap aterm` analysis. Built in tiers of increasing complexity. All tiers complete. aterm is the first X11 graphical application, openssh is the first network service.

**Installed:** pcre2, symlinks, tree-pkg, oniguruma, libffi, tcl, flex, chrpath, libpng, bison, libunistring, gettext, zstd, fontconfig, freetype, aterm, openssh, unzip, zip, nano, rsync (21 installed).
**Staged:** expat, nettle, libtasn1, fribidi, libjpeg-turbo, pixman, uuid (7 staged).

### Sessions 7-11: Batch Build + New Packages (18 packages)

| Package | Version | Status | Notes |
|---------|---------|--------|-------|
| figlet | 2.2.5 | BUILT | Makefile CC injection |
| sl | 5.02 | BUILT | Makefile CC injection with ncurses |
| time | 1.9 | BUILT | Drop texinfo/gnupg2, gpgverify removal |
| cmatrix | 2.0 | BUILT | Drop help2man/console-setup/x11-fonts |
| gmp | 6.2.1 | BUILT | strdup compat, git autosetup → p1 |
| mpfr | 4.2.1 | BUILT | --disable-thread-safe (no __thread TLS) |
| hyphen | 2.8.8 | BUILT | pushd/popd → cd, drop valgrind |
| libevent | 2.1.12 | BUILT | strsep compat, test removal, doxygen disabled |
| libxslt | 1.1.39 | BUILT | Drop python bindings |
| giflib | 5.2.2 | INSTALLED | cmake cross-build |
| libstrophe | 0.13.1 | INSTALLED | XMPP client library, res_query compat |
| groff | 1.23.0 | INSTALLED | First C++ package! cmath/specfun, wchar_t, doc gen skip |
| openssh | 9.6p1 | INSTALLED | R_MIPS_REL32 dispatch, debug-mode only |
| unzip | 6.0 | INSTALLED | Simple build |
| zip | 3.0 | INSTALLED | Simple build |
| nano | 7.2 | INSTALLED | -lgen for dirname/basename |
| rsync | 3.2.7 | INSTALLED | Long double crash fix |
| tdnf | 3.5.14 | INSTALLED | vsnprintf-irix patch, cmake cross fixes |

### C++ Cross-Compilation (Session 10)

Full C++ cross-compilation using clang++ with GCC 9 libstdc++ from SGUG-RSE. Key components:
- `cross/bin/irix-cxx` — C++ compiler wrapper
- `cross/crt/crtbeginT.S` / `cross/crt/crtendT.S` — .ctors/.dtors CRT objects
- Staging c++config.h patches: disabled `_GLIBCXX_USE_C99_MATH_TR1` and `_GLIBCXX_USE_STD_SPEC_FUNCS`
- `-D_WCHAR_T` prevents IRIX wchar_t typedef conflict in C++ mode

#### Meson blocker resolved
glib2 (session 18), harfbuzz, libxkbcommon, pixman all built with raw meson commands replacing `%meson` RPM macros. Pattern documented in glib2.yaml.

### Phase 6: Qt5 (COMPLETE)

| Package | Version | Status | Notes |
|---------|---------|--------|-------|
| glib2 | 2.80.0 | INSTALLED | First meson package |
| harfbuzz | 8.3.0 | INSTALLED | Meson, freetype+glib2 deps |
| xcb-proto | 1.16.0 | INSTALLED | X11 protocol definitions |
| libxcb | 1.16 | INSTALLED | X protocol binding, added -lX11 for rld |
| xcb-util{,-wm,-image,-keysyms,-renderutil} | various | INSTALLED | XCB utility libs |
| libxkbcommon | 1.6.0 | INSTALLED | Meson, strnlen compat added |
| double-conversion | 3.3.0 | INSTALLED | cmake, small C++ |
| **qt5-qtbase** | **5.15.13** | **INSTALLED+VERIFIED** | **Qt5 running on IRIX!** |

Key Qt5 fixes:
- `-Bsymbolic-functions` reduces global GOT entries below rld ~4370 limit
- freetype rebuilt `--without-harfbuzz` to break circular NEEDED dependency
- Preload chain: dlopen Qt5Core + harfbuzz before Qt5Gui
- libxcb linked with `-lX11` for strict rld UND resolution
- Full mogrix pipeline: `mogrix convert` + `mogrix build --cross` → 3 RPMs

### Phase 7: Qt5 Apps (IN PROGRESS)

| Package | Build System | Unblocks | Status |
|---------|-------------|----------|--------|
| qtermwidget5 | cmake | qterminal | NEXT |
| qterminal | cmake | Community target | Blocked on qtermwidget5 |

### Batch Builds (Sessions 48+)

| Batch | Content | Status |
|-------|---------|--------|
| #191–#200 | 60+ packages (libs, utils, fun) | COMPLETED |
| #201 GTK3 stack | cairo, pango, gdk-pixbuf, gtk3 | PENDING (image libs ready) |
| #202 Build tools | cmake, ninja, doxygen, meson, gdb | DONE (gdb SKIPPED — no IRIX debug support in 14.2) |
| #203 Dev tools pt2 | re2c, yasm, quilt done; fossil, mercurial | PENDING |
| #204 IPC/heavy | dbus, dbus-glib, icu, cyrus-sasl | PENDING |
| #205 GUI apps | hexchat, geany, pidgin, nedit | PENDING |

Recent P5E packages (sessions 49-50): ed, dash, units, enscript, screen, mandoc, lrzsz, recode, zsh, mpg123, libpsl, opus, lame, mksh, libssh2, libao, libsndfile, libcaca, ksh, alpine, vile, re2c, yasm, quilt, lcms2, imlib2, frotz, stow, sox, doxygen.

### Skipped Packages

| Package | Reason |
|---------|--------|
| gdb 14.2 | All IRIX native debug support removed. SGUG-RSE used 7.6.2. |
| htop | Needs Linux /proc backend |
| openjpeg2 | No SRPM available |

See `rules/INDEX.md` "Skipped Packages" and `rules/methods/before-you-start.md` section 0 for the full triage checklist.

### Future targets

| Package | Build System | Unblocks |
|---------|-------------|----------|
| cairo | meson | pango, gtk2 |
| pango | meson | gtk2, GUI apps |
| gd | autotools | graphics, emacs |
| elfutils | autotools | binutils, debuginfo |
| libarchive | autotools | cmake, rpm |
| gtk2 | autotools | GUI apps |

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
| 145 package rules + 1 class rule | Done |
| Rule auditing (`mogrix audit-rules`) | Done |
| Rule scoring (`mogrix score-rules`) | Done |
| Batch build (`mogrix batch-build`) | Done |
| Dependency roadmap (`mogrix roadmap`) | Done |
| IRIX bundle tests passing across 20 bundles | Done |
| Bootstrap tarball (`scripts/bootstrap-tarball.sh`) | Done |
| MCP-based IRIX testing (no SSH) | Done |
| App bundles (`mogrix bundle` — .tar.gz and self-extracting .run) | Done |
| Bundle smoke testing (`mogrix test`) | Done |
| dlmalloc test suite (`tests/dlmalloc-test.c` — 29 tests) | Done |
| Upstream package support (`mogrix create-srpm`) | Done |
| Suite bundles (`mogrix bundle pkg1 pkg2 --name`) | Done |
| 105+ source packages cross-compiled for IRIX | Done |
| 20 bundles (5 suites + 15 individual) rebuilt and verified on IRIX | Done |
| Qt5 5.15.13 running on IRIX (Core+Gui+Widgets+XcbQpa) | Done |
| Full GNU userland (coreutils, findutils, tar, make) | Done |
| Package manager (tdnf) functional on IRIX | Done |
| C++ cross-compilation (clang++ + GCC 9 libstdc++) | Done |
| SSH server (openssh) on IRIX | Done |

---

## Key Technical Decisions

| Decision | Rationale |
|----------|-----------|
| LLD 18 for executables, GNU ld + custom linker script for shared libs | LLD for correct relocations; GNU ld with 2-segment (RE+RW) layout for rld |
| `--image-base=0x1000000` for all executables | Default 0x10000 gives only 1.8MB brk heap; 0x1000000 gives 176MB |
| `/lib32/rld` as dynamic linker | IRIX requires this interpreter, not `/usr/lib32/libc.so.1` |
| dlmalloc via mmap (exe-only, linked by irix-ld) | IRIX brk() heap limited to 176MB; mmap accesses 1.2GB. Never in .so files — causes cross-heap corruption with `-Bsymbolic-functions`. Thread-safe via spin locks (USE_LOCKS=1 + USE_SPIN_LOCKS=1, MIPS ll/sc atomics). /dev/zero fd duped to >=128 with FD_CLOEXEC |
| funopen instead of fopencookie | fopencookie crashes on IRIX |
| Iterative vasprintf | IRIX vsnprintf(NULL,0) returns -1 (pre-C99) |
| Per-package compat injection | No shared libdicl dependency; each SRPM is self-contained |
| Symlinks not linker scripts | IRIX rld can't load GNU ld scripts (`INPUT(-lfoo)`) |
| System libs over bundled | Full sysroot available; don't copy SGUG-RSE's bootstrap shortcuts |
| `%u` not `%zu` for size_t | IRIX libc is pre-C99; `%zu` corrupts varargs → SIGSEGV |
| `explicit_bzero` for wipememory | Volatile fptr static initializers crash on IRIX rld |
| Disable `--export-dynamic` features | IRIX rld crashes with large dynamic symbol tables (468+ entries) |
| `-lpthread` for pthread_sigmask | IRIX has it in libpthread, not libc; gnulib replacement needs it |
| X11 via IRIX native sysroot | Don't use `--x-includes`/`--x-libraries`; cross-compiler `--sysroot` finds X11 automatically |
| Filter `-rdynamic` in irix-ld | LLD doesn't support it; IRIX rld crashes on large dynamic symbol tables |
| clang++ with GCC 9 libstdc++ | IRIX GCC 9 headers + libs in staging; custom CRT for .ctors/.dtors |
| `-fno-use-cxa-atexit` | IRIX lacks __cxa_atexit; use atexit() for static destructors |
| `-fno-use-init-array` | IRIX rld doesn't process .init_array; use .ctors with custom CRT |
| `-Bsymbolic-functions` for all .so | Reduces global GOT entries by 50-65%, keeping under rld ~4370 limit |
| Break freetype↔harfbuzz cycle | `--without-harfbuzz` for freetype; rld crashes on circular NEEDED deps |
| Qt5 preload chain | dlopen Qt5Core + harfbuzz before Qt5Gui; rld can't resolve deep dep trees in one pass |
| libxcb links libX11 explicitly | IRIX rld strict UND resolution — won't search already-loaded DSOs without NEEDED edge |
| `-D_WCHAR_T` in C++ mode | Prevents IRIX stdlib.h from typedef-ing wchar_t (C++ keyword) |
| Disable C99 math TR1 in c++config.h | IRIX libm lacks exp2l, cbrt, fdim, etc. |
| R_MIPS_REL32 dispatch pattern | IRIX rld fails on function pointers in static data; use switch/strcmp dispatch |

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
11. **X11 graphical app:** aterm terminal emulator running on IRIX GUI ✓
12. **Network service:** openssh server accepting connections on IRIX ✓
13. **C++ packages:** groff cross-compiled using clang++ + GCC 9 libstdc++ ✓
14. **Batch automation:** `mogrix batch-build` automates multi-package pipelines ✓
15. **App bundles:** `mogrix bundle` creates optimized tarballs that coexist with SGUG-RSE ✓
16. **Qt5 on IRIX:** Qt5 5.15.13 cross-compiled and verified running (`qVersion()` = "5.15.13") ✓

---

## Reference Documents

| Document | Purpose |
|----------|---------|
| `HANDOFF.md` | Session continuity, current blockers |
| `rules/INDEX.md` | Rules reference, per-package problem guide |
| `rules/generic.yaml` | Universal rules for all packages |
| `rules/methods/before-you-start.md` | Checklist + SGUG-RSE assumptions warning |
| `compat/catalog.yaml` | Compat function registry |
