# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-06
**Status**: Phase 3 in progress. gnupg2 2.4.4 fully working on IRIX (key gen, sign, verify). All crypto chain libraries installed. Next: gettext, coreutils, grep, sed, gawk.

---

## CRITICAL WARNINGS

**NEVER install packages directly to /usr/sgug on the live IRIX system.**

On 2026-01-27, installing directly to /usr/sgug replaced libz.so with zlib-ng, broke sshd, and locked out SSH access. System recovered from console using backup libs.

**Always use /opt/chroot for testing first.** Only touch /usr/sgug after:
1. Full testing in chroot passes
2. Explicit user approval
3. Console access is available as backup

**NEVER modify IRIX paths directly via SSH.** The chroot at `/opt/chroot` has its own `/usr/sgug/`. The base system's `/usr/sgug/` is separate. When running commands on IRIX, always be explicit about whether you're targeting the chroot or the base system. Let the user handle IRIX operations to avoid path confusion.

**NEVER CHEAT!** Work through problems converting packages. Don't copy from old SGUG-RSE. Don't fake Provides. Check the rules for how to handle being stuck.

**Question SGUG-RSE assumptions.** SGUG-RSE bootstrapped ON IRIX without most deps. They bundled things and skipped system libraries out of necessity. We cross-compile with a full sysroot — prefer system libraries over bundled copies. See `rules/methods/before-you-start.md`.

---

## Goal

Cross-compile Fedora 40 packages for IRIX using mogrix, a deterministic SRPM conversion engine. Phase 1 (bootstrap: rpm/tdnf) and Phase 2 (build tools: autoconf/automake/libtool) are complete. Phase 3 targets user-facing packages.

---

## Current Status

### Phase 1: Bootstrap (COMPLETE)

All 14 bootstrap packages cross-compiled and working on IRIX:

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

Verified working: `rpm -Uvh`, `rpm -qa`, `tdnf repolist`, `tdnf makecache`, `tdnf install`.

### Phase 2: Build Tools (COMPLETE)

| Package | Status | Notes |
|---------|--------|-------|
| m4 1.4.19 | INSTALLED | - |
| perl 5.38.2 | INSTALLED | Monolithic build, explicit Provides, 652 .pm modules |
| autoconf 2.71 | INSTALLED | Generates configure + autom4te cache |
| automake 1.16.5 | INSTALLED | shebang + FindBin fixes |
| bash 5.2.26 | INSTALLED | System readline, libtinfo for termcap |
| libtool 2.4.7 | INSTALLED | shebang + drop_requires fixes |

### Phase 3: User-Facing Packages (IN PROGRESS)

| Package | Version | Status | Notes |
|---------|---------|--------|-------|
| pkgconf | 2.1.0 | INSTALLED | Static build, %zu→%u fix, working on IRIX |
| readline | 8.2-8 | INSTALLED | Uses system ncurses |
| libgpg-error | 1.48-1 | INSTALLED | lock-obj-pub generated on IRIX, strdup/strndup compat |
| libgcrypt | 1.10.3-3 | INSTALLED | FIPS removed, ASM disabled |
| libassuan | 2.5.7-1 | INSTALLED | unsetenv decl fix, SCM_RIGHTS defined |
| libksba | 1.6.6-1 | INSTALLED | Clean build |
| npth | 1.7-1 | INSTALLED | pthread_atfork in unistd.h (IRIX POSIX1C) |
| gnupg2 | 2.4.4-1 | INSTALLED | Key gen + sign + verify working on IRIX |

**gnupg2 SIGSEGV root cause**: `wipememory()` in common/mischelp.c uses
`static void *(*volatile memset_ptr)(...) = (void *)memset;` — a volatile function pointer
with a static initializer. IRIX rld doesn't handle the relocation for this static initializer
correctly in our cross-compiled binaries, so the function pointer contains garbage → SIGSEGV.
Fix: provide `explicit_bzero` in mogrix-compat + set `ac_cv_func_explicit_bzero: "yes"`.
When HAVE_EXPLICIT_BZERO is defined, wipememory uses it instead of the volatile function pointer.
The explicit_bzero compat uses `volatile unsigned char *` pointer (safe on IRIX).

**Next packages**:
- gettext, coreutils, grep, sed, gawk (user-facing tools)

---

## What Worked This Session

### RPM OS Tag + Libtool Triplet Decoupling
- `--target mips-sgi-irix6.5` gave RPM `OS=irix6.5` — IRIX rpm rejects this (expects `OS=irix`)
- `--target mips-sgi-irix` gives RPM `OS=irix` but `--host=mips-irix` (no version) breaks libtool
- Fix: `--target mips-sgi-irix` in cli.py (for RPM), `--host=mips-irix6.5` hardcoded in rpmmacros.irix (for libtool)
- `_target_os` in rpmmacros set to `irix` (was `irix6.5`)

### pkgconf %zu Format Specifier Fix (Root Cause Found)
- pkgconf crashed with SIGSEGV inside `pkgconf_client_init()` → `pkgconf_trace()`
- Root cause: IRIX libc does NOT support the C99 `%zu` printf format specifier
- `%zu` corrupts varargs parsing — subsequent `%s` reads garbage pointer → SIGSEGV
- pkgconf's `SIZE_FMT_SPECIFIER` macro defaulted to `"%zu"` on non-Windows platforms
- Fix: patch `libpkgconf/stdinc.h` to use `"%u"` (size_t = 4 bytes on MIPS n32)
- **This is a generic IRIX issue**: any package using `%zu`/`%zd`/`%zx` in printf-family calls will break
- Also still using `--disable-shared` (no other package needs libpkgconf.so)
- Debugging methodology: systematically narrowed crash location using write() syscall markers
  (fprintf was unsafe — it uses printf internally which has the same bug)

### gnupg2 2.4.4 — wipememory / explicit_bzero Fix
- gnupg2 built and installed, but gpg-agent crashed (SIGSEGV) during key generation
- Crash narrowed to `nvc_release(pk)` → `nve_release` → `wipememory(entry->value, strlen(...))`
- `wipememory()` uses `static volatile` function pointer to `memset` — a common anti-optimization trick
- IRIX rld doesn't handle the static initializer relocation for the memset address correctly
- The function pointer contains garbage → calling it SIGSEGVs
- NOT an allocator mismatch (tested with and without dlmalloc — same crash)
- Fix: `explicit_bzero` compat function (byte-by-byte volatile zero) + `ac_cv_func_explicit_bzero: "yes"`
- gnupg2's `wipememory` has `#elif defined(HAVE_EXPLICIT_BZERO)` path that avoids the volatile pointer
- **This is a generic IRIX issue**: any code using `static volatile` function pointers with initializers may crash
- Debugging methodology: binary search with `write(2, ...)` syscall markers to find exact crash location

### IRIX MCP Server
- Added `tools/irix-mcp-server.py` — MCP server for safe chroot access from Claude Code
- Registered in `.mcp.json` — tools: `irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`
- All commands wrapped in `chroot /opt/chroot /usr/sgug/bin/sgug-exec /bin/sh -c '...'`
- Safety blocklist for destructive commands (rm -rf /, reboot, etc.)

### PKG_CONFIG_LIBDIR in rpmmacros.irix
- Packages needing other staging sysroot libraries (e.g., libgcrypt → libgpg-error) couldn't find them
- Added `PKG_CONFIG_LIBDIR` and `PATH` exports to `%configure` macro in `cross/rpmmacros.irix`

### Source Numbering Fix
- Compat sources (dlmalloc, strdup, etc.) use Source100+
- Extra sources (`add_source` in package yaml) also started at Source100 → conflict
- Fix: extra sources now start at Source200 in `mogrix/cli.py:420`

### libgpg-error lock-obj-pub
- Cross-compilation needs platform-specific `lock-obj-pub.<host_os>.h`
- Generated on IRIX, stored at `compat/libgpg-error/lock-obj-pub.irix6.5.h`
- Added as Source200 via `add_source`, copied during prep

### IRIX POSIX1C Guards
- `pthread_atfork` in IRIX `unistd.h` is behind `#if _POSIX1C`
- `_POSIX1C` is enabled by `_SGI_REENTRANT_FUNCTIONS` (set in irix-cc) or by including `pthread.h` first
- Include order matters: `pthread.h` before `unistd.h`

### GNU ld Linker Script Fix (ncurses)
- Fedora ncurses creates `libcurses.so`, `libcursesw.so`, `libtermcap.so` as GNU ld scripts (`INPUT(-lfoo)`)
- IRIX rld can't load linker scripts — only real ELF .so files
- Symptom: `rld: Error: elfmap failed on /path/libfoo.so : read N bytes too short to be elf header`
- Fixed in `ncurses.yaml`: replace `echo "INPUT(...)"` with `ln -s` for all 3 files
- Also fixed staging sysroot (`/opt/sgug-staging/usr/sgug/lib32/`) — same linker scripts replaced with symlinks
- Also dropped `ncurses-c++-libs` (subpackage dropped) and `pkgconfig` requires from ncurses-devel
- **Any package that creates `INPUT()` style .so files needs the same fix**

### Bash 5.2.26 — System Readline (Not Bundled)
- SGUG-RSE used `--without-installed-readline` because they bootstrapped without readline available
- We have readline+ncurses in our sysroot — use `--with-installed-readline` instead
- Bundled readline linked `-lcurses` → `NEEDED: libcurses.so` (the linker script) → rld error
- SGUG-RSE ncurses splits termcap symbols (tputs, tgetent) into libtinfo.so
- Must set `bash_cv_termcap_lib: "libtinfo"` — configure's default search hits libcurses before libncurses
- Readline statically linked from libreadline.a (no .so in sysroot), only libtinfo.so needed at runtime

### Questioning SGUG-RSE Assumptions
- Added guidance to `rules/methods/before-you-start.md`: SGUG-RSE made decisions under bootstrap constraints that don't apply to us
- Rule: prefer system libraries over bundled copies when the dependency is available

### Perl Provides (Approach A — Explicit Tags)
- `rpmmacros.irix` sets `__find_provides: %{nil}` + `AutoProv: no` globally
- This CANNOT be overridden from the spec (`--macros` files > `%global` > `%define`)
- Solution: 4 explicit `Provides:` tags in `perl.yaml` spec_replacements

---

## What Failed (Don't Repeat)

### Perl Auto-Provides Approaches (All Failed)
1. `%define __find_provides /usr/lib/rpm/perl.prov` in spec — overridden by rpmmacros.irix
2. `%global __find_provides /usr/lib/rpm/perl.prov` in spec — still lower priority than `--macros` files
3. `AutoProv: yes` before `Name:` in spec — RPM ignores preamble tags before `Name:`
4. `AutoProv: yes` after `Name:` in spec — rpmmacros.irix's `AutoProv: no` wins

### Bash Bundled Readline (Wrong Approach)
- `--without-installed-readline` produces binary with `NEEDED: libcurses.so` (linker script, rld fails)
- `bash_cv_termcap_lib: "libncurses"` → link error: termcap symbols are in libtinfo, not libncurses
- Must use `--with-installed-readline` + `bash_cv_termcap_lib: "libtinfo"`

### Modifying Live IRIX System
- Agent modified `/usr/sgug/lib32/libcurses.so` on the base system instead of the chroot
- No harm done (base system already had a proper symlink), but reinforces: let user handle IRIX operations

### Fedora Multilib libdir Hacks
- libgpg-error and libgcrypt specs hardcode `libdir="/usr/lib"` via sed in %prep for multilib compat
- This breaks our lib32 setup — must remove via spec_replacements

### libgcrypt FIPS Cross-Compilation
- Fedora's `__spec_install_post` override uses readelf/objcopy on target binaries — fails cross-compiling
- Must remove entire FIPS block via spec_replacements

### Key Lesson
Package-level `rpm_macros` overrides are NOT supported in `engine.py`. Use `spec_replacements` to inject `Provides:` tags directly.

---

## Key Architectural Decisions

| Decision | Rationale |
|----------|-----------|
| LLD 18 for executables, GNU ld + `-z separate-code` for shared libs | LLD for correct relocations; GNU ld with forced 3-segment layout for rld |
| `--image-base=0x1000000` for all executables | Default 0x10000 gives only 1.8MB brk heap; 0x1000000 gives 176MB |
| `/lib32/rld` as dynamic linker | IRIX requires this interpreter |
| dlmalloc via mmap (auto-injected) | IRIX brk() heap limited to 176MB; mmap accesses 1.2GB |
| funopen instead of fopencookie | fopencookie crashes on IRIX |
| Iterative vasprintf | IRIX vsnprintf(NULL,0) returns -1 (pre-C99) |
| Per-package compat injection | No shared libdicl dependency; each SRPM is self-contained |
| Symlinks not linker scripts | IRIX rld can't load GNU ld scripts (`INPUT(-lfoo)`) |
| System libs over bundled | We have a full sysroot; don't copy SGUG-RSE's bootstrap shortcuts |
| `%u` not `%zu` for size_t | IRIX libc is pre-C99; `%zu` corrupts varargs → SIGSEGV |
| `explicit_bzero` compat | IRIX rld breaks `static volatile` function pointer initializers → wipememory crashes |

---

## Environment

| Purpose | Path |
|---------|------|
| Mogrix project | `/home/edodd/projects/github/unxmaal/mogrix/` |
| Cross toolchain | `/opt/cross/bin/` |
| Staging area | `/opt/sgug-staging/usr/sgug/` |
| IRIX sysroot | `/opt/irix-sysroot/` |
| IRIX host | `ssh edodd@192.168.0.81` (root for installs) |
| IRIX chroot | `/opt/chroot` (testing environment) |
| Original FC40 SRPMs | `~/rpmbuild/SRPMS/fc40/` |
| Built MIPS RPMs | `~/rpmbuild/RPMS/mips/` |

### Directory Principles
- **Original SRPMs are inputs** — keep in `~/rpmbuild/SRPMS/fc40/`
- **Converted output is ephemeral** — regenerate from originals + rules anytime
- **Built RPMs are outputs** — rpmbuild puts them in `~/rpmbuild/RPMS/`
- **Never store build artifacts in the mogrix git repo**
- **mogrix convert bakes rules into the SRPM** — must re-convert after rule changes

### Workflow
```bash
uv run mogrix convert ~/rpmbuild/SRPMS/fc40/<package>.src.rpm
uv run mogrix build ~/rpmbuild/SRPMS/fc40/<package>-converted/<package>.src.rpm --cross
scp ~/rpmbuild/RPMS/mips/<package>*.rpm root@192.168.0.81:/opt/chroot/tmp/
# User installs in chroot: rpm -Uvh /tmp/<package>*.rpm
```

---

## Known Issues

- **`%zu` format specifier crashes IRIX libc** — use `%u` for size_t, `%lu` for unsigned long. IRIX printf doesn't understand the `z` length modifier (C99). Corrupts varargs → SIGSEGV when followed by `%s`.
- **Volatile function pointer static initializers crash** — `static volatile fptr = (void *)func;` doesn't get the right relocation from rld. Provide `explicit_bzero` for packages using `wipememory`-style patterns.
- fopencookie crashes on IRIX — use funopen instead
- sqlite3 CLI crashes when writing to files (rpm database writes work fine)
- IRIX chroot doesn't fully isolate — processes see base system paths
- Existing SGUG-RSE packages conflict with our FC40 packages (zlib-ng-compat vs zlib)
- `odump` and `elfdump` crash/fail on LLD-produced binaries (dynamic section format differs from GCC/IRIX ld output) — rld loads them fine, but IRIX analysis tools can't parse them
- rld may probe `.so` files in the library path even if not in NEEDED chain — any non-ELF `.so` file (linker scripts) will produce warnings

---

## Reference Documents

| Document | Purpose |
|----------|---------|
| `rules/INDEX.md` | Rules reference, per-package problem guide |
| `rules/methods/before-you-start.md` | Checklist before debugging (includes SGUG-RSE warning) |
| `rules/generic.yaml` | Universal rules for all packages |
| `compat/catalog.yaml` | Compat function registry |
| `plan.md` | Project plan and architecture |
| `rules/methods/mogrix-workflow.md` | How to run mogrix |
| `rules/methods/irix-testing.md` | IRIX shell rules, chroot, debugging |
