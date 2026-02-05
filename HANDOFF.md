# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-05
**Status**: Phase 2 build tools nearly complete. All 6 packages INSTALLED on IRIX (m4, perl, autoconf, automake, bash). libtool RPM built, ready to install.

---

## CRITICAL WARNING

**NEVER install packages directly to /usr/sgug on the live IRIX system.**

On 2026-01-27, installing directly to /usr/sgug replaced libz.so with zlib-ng, broke sshd, and locked out SSH access. System recovered from console using backup libs.

**Always use /opt/chroot for testing first.** Only touch /usr/sgug after:
1. Full testing in chroot passes
2. Explicit user approval
3. Console access is available as backup

**NEVER CHEAT!** Work through problems converting packages. Don't copy from old SGUG-RSE. Don't fake Provides. Check the rules for how to handle being stuck.

---

## Goal

Get **rpm** and **tdnf** working on IRIX so packages can be installed via the package manager. Then build **Phase 2 tools** (autoconf, automake, libtool) to enable self-hosting.

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

### Phase 2: Build Tools (IN PROGRESS)

| Package | Status | Notes |
|---------|--------|-------|
| m4 1.4.19 | INSTALLED | - |
| perl 5.38.2 | INSTALLED (upgraded with Provides) | Monolithic build, 652 .pm modules |
| autoconf 2.71 | INSTALLED | Awaiting smoke test |
| automake 1.16.5 | INSTALLED (shebang+FindBin fixes) | Awaiting smoke test |
| libtool 2.4.7 | RPM built, ready to install | Has shebang + drop_requires fixes |
| bash 5.2.26 | INSTALLED | System readline, libtinfo for termcap, HAVE_PSELECT fixes |

---

## What Worked This Session

### Perl Provides (Approach A — Explicit Tags)
- `rpmmacros.irix` sets `__find_provides: %{nil}` + `AutoProv: no` globally
- This CANNOT be overridden from the spec (`--macros` files > `%global` > `%define`)
- Solution: 4 explicit `Provides:` tags in `perl.yaml` spec_replacements
- Tags: `perl-interpreter`, `perl(File::Compare)`, `perl(Thread::Queue)`, `perl(threads)`
- All modules physically exist in the monolithic perl RPM (verified with rpm -qpl)

### Bash 5.2.26 Cross-Compilation
- 17 rules in `bash.yaml`: configure flags, HAVE_PSELECT fixes, doc cleanup, /bin/sh compat
- `--with-installed-readline` uses system readline (not bundled — SGUG-RSE bundled out of bootstrap necessity)
- `bash_cv_termcap_lib: "libtinfo"` — SGUG-RSE ncurses splits termcap into libtinfo.so
- Created `compat/include/mogrix-compat/generic/wchar.h` overlay for C99 multibyte functions
- `ac_cv_header_sys_random_h: "no"` avoids getrandom() conflict with dicl-clang-compat
- `prep_commands` fix lib/sh/input_avail.c HAVE_PSELECT bugs
- `install_cleanup` removes doc files from dropped doc subpackage
- Verified on IRIX: `bash --version`, brace expansion, for loops all working

### ncurses Linker Script Fix
- Fedora ncurses creates `libcurses.so`, `libcursesw.so`, `libtermcap.so` as GNU ld scripts (`INPUT(-lfoo)`)
- IRIX rld can't load linker scripts — only real ELF .so files
- Fixed in `ncurses.yaml`: replace `echo "INPUT(...)"` with `ln -s` for all 3 files
- Also dropped `ncurses-c++-libs` (subpackage dropped) and `pkgconfig` requires from ncurses-devel

### Rules Cleanup
- Removed duplicate rules from ~30 package YAMLs (ac_cv_overrides, skip_check, libdicl, drop_buildrequires)
- Updated `rules/INDEX.md` with "Check generic.yaml First" guidance
- plan.md compacted from 1348 to ~170 lines

### SRPM Preservation
- `cli.py` now copies converted SRPMs to `~/rpmbuild/SRPMS/` after successful builds

## What Failed (Don't Repeat)

### Perl Auto-Provides Approaches (All Failed)
1. `%define __find_provides /usr/lib/rpm/perl.prov` in spec — overridden by rpmmacros.irix
2. `%global __find_provides /usr/lib/rpm/perl.prov` in spec — still lower priority than `--macros` files
3. `AutoProv: yes` before `Name:` in spec — RPM ignores preamble tags before `Name:`
4. `AutoProv: yes` after `Name:` in spec — rpmmacros.irix's `AutoProv: no` wins
5. `rewrite_paths` transforms build-host paths — use `/usr/./lib` to avoid `/usr/lib` matching

### Key Lesson
Package-level `rpm_macros` overrides are NOT supported in `engine.py`. The `_apply_package_rules()` method doesn't handle `rpm_macros`. Use `spec_replacements` to inject `Provides:` tags directly.

---

## Next Steps

1. **Install libtool** — copy `libtool-2.4.7-10.mips.rpm` + `libtool-ltdl-2.4.7-10.mips.rpm` to IRIX
2. **Plan production migration** — Strategy for moving from chroot to /usr/sgug
3. **Begin Phase 3** — user-facing packages (ncurses, coreutils, etc.)

### Deferred
- Remaining inline seds in libsolv/tdnf/rpm (working, not blocking)
- Long-term: WebKitGTK 2.38.x for modern browser on IRIX

---

## Perl 5.38.2 Cross-Compilation

### Approach: Monolithic Package

Fedora perl has 188 subpackages. For cross-compilation, we build a single monolithic package:
- `drop_subpackages: ["[A-Za-z]*"]` comments out all subpackages
- `%files -f perl-filelist.txt` uses an auto-generated file manifest
- `%global dual_life 1` keeps all core modules
- `remove_lines` strips orphaned Requires from dropped subpackages
- Original FC40 SRPM: `~/rpmbuild/SRPMS/fc40/perl-5.38.2-506.fc40.src.rpm`

### RPM Contents
- 3556 files, 652 .pm modules, 23 .so extensions (XS modules)
- All critical modules present (Carp, Exporter, strict, MakeMaker, etc.)
- ELF 32-bit MSB executable, MIPS, N32 MIPS-III

### Cross-Compilation Strategy
1. Pre-generated `config.sh` (from SGUG-RSE) tells Perl about IRIX target
2. `Configure -S` reads config.sh without probing
3. Native miniperl built on host (gcc) runs build scripts
4. `CC=irix-cc` compiles C code for IRIX via clang cross-compiler

### What Failed (Don't Repeat)
- `drop_subpackages: ["*"]` matches `-f` in `%files -f`. Use `["[A-Za-z]*"]`.
- Build without `dual_life 1` removes dual-life modules in %install.
- Converting a previously-converted SRPM causes duplicate Source100. Always use original FC40 SRPM.
- `-I. -Ilib` order breaks buildcustomize.pl. Must be `-Ilib -I.`.
- Configure -S runs `make depend` internally. Must clean stale .o/makefile before spec's `make`.

---

## Key Architectural Decisions

### Linker Selection
- **LLD 18** with IRIX patches for executables (LLD 14 has MIPS relocation bugs)
- **GNU ld** for shared libraries (LLD exe layout crashes IRIX rld)
- See `rules/methods/linker-selection.md`

### IRIX vsnprintf
- Pre-C99: `vsnprintf(NULL, 0)` returns -1 (not buffer size)
- Fixed with iterative vasprintf in `compat/stdio/asprintf.c`

### Memory Allocation
- IRIX brk() heap limited to 176MB (libpthread at 0x0C080000 blocks growth)
- dlmalloc uses mmap instead, accessing 1.2GB of free address space
- Auto-injected when any compat functions are used

### Cookie I/O
- fopencookie crashes on IRIX — use funopen instead (`compat/stdio/funopen.c`)

---

## Environment

| Purpose | Path |
|---------|------|
| Mogrix project | `/home/edodd/projects/github/unxmaal/mogrix/` |
| Cross toolchain | `/opt/cross/bin/` |
| Staging area | `/opt/sgug-staging/usr/sgug/` |
| IRIX sysroot | `/opt/irix-sysroot/` |
| IRIX host | `ssh edodd@192.168.0.81` |
| IRIX chroot | `/opt/chroot` |
| Original FC40 SRPMs | `~/rpmbuild/SRPMS/fc40/` |
| Built MIPS RPMs | `~/rpmbuild/RPMS/mips/` |

### Directory Principles
- **Original SRPMs are inputs** — keep in `~/rpmbuild/SRPMS/fc40/`
- **Converted output is ephemeral** — regenerate from originals + rules anytime
- **Built RPMs are outputs** — rpmbuild puts them in `~/rpmbuild/RPMS/`
- **Never store build artifacts in the mogrix git repo**

---

## Known Issues

- fopencookie crashes on IRIX — use funopen instead
- sqlite3 CLI crashes when writing to files (rpm database writes work fine)
- IRIX chroot doesn't fully isolate — processes see base system paths
- Existing SGUG-RSE packages conflict with our FC40 packages (zlib-ng-compat vs zlib)
