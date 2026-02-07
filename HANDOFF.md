# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-07 (session 2)
**Status**: Phase 5 (Library Foundation) IN PROGRESS. Tier 0 + Tier 1 + 8 Tier 2 packages complete (18 new packages, 59 total). Moving to Tier 2 chains.

---

## CRITICAL WARNINGS

**NEVER install packages directly to /usr/sgug on the live IRIX system.**

On 2026-01-27, installing directly to /usr/sgug replaced libz.so with zlib-ng, broke sshd, and locked out SSH access. System recovered from console using backup libs.

**Always use /opt/chroot for testing.** This is our clean chroot bootstrapped via `scripts/bootstrap-tarball.sh`. The old `/opt/chroot_0206` has the full SGUG-RSE base (backup only).

**NEVER CHEAT!** Work through problems converting packages. Don't copy from old SGUG-RSE. Don't fake Provides. Check the rules for how to handle being stuck.

**Question SGUG-RSE assumptions.** SGUG-RSE bootstrapped ON IRIX without most deps. They bundled things and skipped system libraries out of necessity. We cross-compile with a full sysroot — prefer system libraries over bundled copies.

---

## Goal

Cross-compile Fedora 40 packages for IRIX using mogrix. Phases 1-4c complete (41 packages). Now building Phase 5: library foundation packages toward aterm (X11 terminal emulator). 59 total packages built.

---

## IMMEDIATE NEXT: Tier 2 Packages

Tier 0 and Tier 1 are complete. Next targets from the aterm roadmap:

### Tier 2: Small Chains (high impact)
- **gettext** (6 new deps) → unblocks 12 packages (biggest single blocker)
- **zstd** (4 new deps) → unblocks elfutils, gnutls, libarchive, git
- **fontconfig** (5 new deps) → unblocks emacs, cairo, pango, gd
- **libX11** (8 new deps) → unblocks emacs, freetype, tk, gd — CRITICAL for aterm
- **texinfo** (9 new deps) → unblocks emacs, gnutls, dejagnu, binutils

Start with packages that have the fewest new dependencies.

---

## Phase 5 Progress

### Tier 0: Ready to Build ✅ COMPLETE
| Package | Version | Status | Key Fixes |
|---------|---------|--------|-----------|
| pcre2 | 10.42 | INSTALLED | JIT/sealloc blocks, `--disable-percent-zt`, inttypes.h C99 fix |
| symlinks | 1.7 | INSTALLED | CC override for raw Makefile |
| tree-pkg | 2.1.0 | INSTALLED | Clean build |
| oniguruma | 6.9.9 | INSTALLED | fix-libtool-irix.sh |
| libffi | 3.4.4 | INSTALLED | CCASFLAGS `-fno-integrated-as` (LLVM #52785), sgidefs.h overlay |
| tcl | 8.6.13 | INSTALLED | `tcl_cv_sys_version=IRIX64-6.5`, SHLIB_LD=`$CC -shared`, symlink fixes |

### Tier 1: Leaf Blockers ✅ COMPLETE
| Package | Version | Status | Key Fixes |
|---------|---------|--------|-----------|
| flex | 2.6.4 | INSTALLED | nls-disabled, inline basename (bootstrap HOST binary issue) |
| chrpath | 0.16 | INSTALLED | byteswap.h compat, doc cleanup |
| libpng | 1.6.40 | INSTALLED | pngcp/pngfix stubs (header conflicts + missing zlib features) |
| bison | 3.8.2 | INSTALLED | gl_cv_header_working_stdint_h override, doc cleanup |

---

## What Was Fixed This Session

### 1. configure_flags:remove regex bug
`spec.py:222` now uses `(?=[=\s\\]|$)` lookahead to prevent `--enable-jit` matching inside `--enable-jit-sealloc`.

### 2. Duplicate add_patch/add_source engine bug
Fixed in `engine.py` — when yaml has no `rules:` subsection, `rules = pkg_rules.get("rules", pkg_rules)` made `rules` the SAME dict, causing double processing.

### 3. C99 inttypes.h compliance
IRIX `inttypes.h` doesn't include `stdint.h` (violating C99). Fixed by adding `#include <stdint.h>` to `mogrix-compat/generic/inttypes.h`.

### 4. LLVM/Clang 16 assembler bug (MIPS .cpsetup)
Clang 16 integrated assembler generates wrong relocations for MIPS N32 `.cpsetup` (R_MIPS_HI16 against `__gnu_local_gp` instead of GP-relative). Fix: `-fno-integrated-as` to use GNU as. Tracked as LLVM #52785.

### 5. __ASSEMBLER__ guards for IRIX headers
IRIX `sgidefs.h` has C typedefs under `_LANGUAGE_C` guard but clang defines this even in assembler-with-cpp mode. Fixed with `dicl-clang-compat/sgidefs.h` overlay.

### 6. byteswap.h compat header
Created `dicl-clang-compat/byteswap.h` using `__builtin_bswap{16,32,64}`.

### 7. basename compat function
IRIX has `basename()` in `libgen.so`, not libc. Created `compat/string/basename.c` and added to catalog.

### 8. gnulib stdint.h conflict
gnulib generates its own `int_fast32_t` (as `long`) conflicting with our `dicl-clang-compat/stdint.h` (as `int`). Fix: `gl_cv_header_working_stdint_h: "yes"` override.

### 9. tcl cross-compilation detection
`tcl.m4` uses `uname -s` to detect platform — returns `Linux` during cross-compilation. Fix: `tcl_cv_sys_version: "IRIX64-6.5"` override.

---

## Current Status

### Full Phased Rebuild (COMPLETE - 2026-02-07)

All packages rebuilt from clean state. Bootstrap tarball deployed to bare `/opt/chroot`, all packages installed via MCP (no SSH), verified working. 51 packages in rpm database (41 from phases 1-4c + 10 from phase 5).

### Phase 1: Bootstrap (14 packages)

| # | Package | Version | Status |
|---|---------|---------|--------|
| 1 | zlib-ng | 2.1.6 | INSTALLED |
| 2 | bzip2 | 1.0.8 | INSTALLED |
| 3 | popt | 1.19 | INSTALLED |
| 4 | xz | 5.4.6 | INSTALLED |
| 5 | openssl | 3.2.1 | INSTALLED |
| 6 | libxml2 | 2.12.5 | INSTALLED |
| 7 | curl | 8.6.0 | INSTALLED |
| 8 | lua | 5.4.6 | INSTALLED |
| 9 | file | 5.45 | INSTALLED |
| 10 | sqlite | 3.45.1 | INSTALLED |
| 11 | rpm | 4.19.1.1 | INSTALLED |
| 12 | libsolv | 0.7.28 | INSTALLED |
| 13 | tdnf | 3.5.14 | INSTALLED |
| 14 | sgugrse-release | 0.0.7beta | INSTALLED |

### Phase 1.5: System Libraries

| Package | Version | Status |
|---------|---------|--------|
| ncurses | 6.4 | INSTALLED |
| readline | 8.2 | INSTALLED |

### Phase 2: Build Tools (6 packages)

| Package | Version | Status |
|---------|---------|--------|
| m4 | 1.4.19 | INSTALLED |
| perl | 5.38.2 | INSTALLED |
| bash | 5.2.26 | INSTALLED |
| autoconf | 2.71 | INSTALLED |
| automake | 1.16.5 | INSTALLED |
| libtool | 2.4.7 | INSTALLED |

### Phase 3a: Crypto Stack (7 packages)

| Package | Version | Status |
|---------|---------|--------|
| pkgconf | 2.1.0 | INSTALLED |
| libgpg-error | 1.48 | INSTALLED |
| libgcrypt | 1.10.3 | INSTALLED |
| libassuan | 2.5.7 | INSTALLED |
| libksba | 1.6.6 | INSTALLED |
| npth | 1.7 | INSTALLED |
| gnupg2 | 2.4.4 | INSTALLED |

### Phase 3b: GNU Text Tools (3 packages)

| Package | Version | Status |
|---------|---------|--------|
| sed | 4.9 | INSTALLED |
| gawk | 5.3.0 | INSTALLED |
| grep | 3.11 | INSTALLED |

### Phase 4a-c: Utilities (9 packages)

| Package | Version | Status |
|---------|---------|--------|
| less | 643 | INSTALLED |
| which | 2.21 | INSTALLED |
| gzip | 1.13 | INSTALLED |
| diffutils | 3.10 | INSTALLED |
| patch | 2.7.6 | INSTALLED |
| make | 4.4.1 | INSTALLED |
| findutils | 4.9.0 | INSTALLED |
| tar | 1.35 | INSTALLED |
| coreutils | 9.4 | INSTALLED |

### Phase 5: Library Foundation (10 packages so far)

| Package | Version | Status | Tier |
|---------|---------|--------|------|
| pcre2 | 10.42 | INSTALLED | 0 |
| symlinks | 1.7 | INSTALLED | 0 |
| tree-pkg | 2.1.0 | INSTALLED | 0 |
| oniguruma | 6.9.9 | INSTALLED | 0 |
| libffi | 3.4.4 | INSTALLED | 0 |
| tcl | 8.6.13 | INSTALLED | 0 |
| flex | 2.6.4 | INSTALLED | 1 |
| chrpath | 0.16 | INSTALLED | 1 |
| libpng | 1.6.40 | INSTALLED | 1 |
| bison | 3.8.2 | INSTALLED | 1 |
| expat | 2.6.0 | STAGED | 2 |
| freetype | 2.13.2 | STAGED | 2 |
| nettle | 3.9.1 | STAGED | 2 |
| libtasn1 | 4.19.0 | STAGED | 2 |
| fribidi | 1.0.13 | STAGED | 2 |
| libjpeg-turbo | 3.0.2 | STAGED | 2 |
| pixman | 0.43.0 | STAGED | 2 |
| uuid | 1.6.2 | STAGED | 2 |

**Total: 59 source packages cross-compiled (51 installed+verified on IRIX, 8 staged for next install batch).**

---

## Environment

| Purpose | Path |
|---------|------|
| Mogrix project | `/home/edodd/projects/github/unxmaal/mogrix/` |
| Cross toolchain | `/opt/cross/bin/` |
| Staging area | `/opt/sgug-staging/usr/sgug/` |
| IRIX sysroot | `/opt/irix-sysroot/` |
| IRIX host | `192.168.0.81` (use MCP, not SSH) |
| IRIX chroot (active) | `/opt/chroot` (bootstrapped, ~51 source packages) |
| IRIX chroot (backup) | `/opt/chroot_0206` (old SGUG-RSE base) |
| Original FC40 SRPMs | `~/mogrix_inputs/SRPMS/` |
| Converted SRPMs | `~/mogrix_outputs/SRPMS/` |
| Built MIPS RPMs | `~/mogrix_outputs/RPMS/` |
| rpmbuild workspace | `~/rpmbuild/` (ephemeral) |

### Workflow
```bash
# Build on Linux
uv run mogrix fetch <package> -y
uv run mogrix convert ~/mogrix_inputs/SRPMS/<pkg>-*.src.rpm
uv run mogrix build ~/mogrix_outputs/SRPMS/<pkg>-*-converted/<pkg>-*.src.rpm --cross
uv run mogrix stage ~/mogrix_outputs/RPMS/<pkg>*.rpm

# Install on IRIX via MCP (do NOT ssh as root)
# Use irix_copy_to to copy RPMs, irix_exec to run rpm -Uvh
```

---

## `mogrix roadmap` Command

Computes the full transitive build-dependency graph for any FC40 package. Usage:
```bash
uv run mogrix roadmap aterm           # full graph
uv run mogrix roadmap aterm --json    # JSON for programmatic use
uv run mogrix roadmap aterm --tree    # Rich tree widget
```

---

## Key Lessons Learned This Session (Session 2)

### Meson cross-compilation support
Installed meson 1.10.1 via `uv tool install meson`. Created `cross/meson-irix-cross.ini` cross file. Enables building meson-based packages (pixman, fribidi future use). Key issue: meson runs `--version` on compiler for detection — irix-cc wrapper was forwarding to linker. Fixed by adding `--version`/`-v`/`--help` handling to irix-cc.

### cmake cross-compilation (libjpeg-turbo)
Replace `%{cmake}`, `%cmake_build`, `%cmake_install` macros with raw cmake commands. Key: use `-DCMAKE_INSTALL_LIBDIR=lib32` (hardcoded, not `%{_lib}` which resolves to `lib64` on host). Pass compat library via `-DCMAKE_EXE_LINKER_FLAGS` and `-DCMAKE_SHARED_LINKER_FLAGS` (cmake doesn't use `LIBS`).

### irix-cc wrapper improvements
- Added `--version`/`-v`/`--help`/`-dumpversion`/`-dumpmachine` handling (forward to clang) for build system detection
- Added `-ggdb*`/`-gdwarf*`/`-gz*` to link-only filter (fixes nettle `-ggdb3` passed to linker)

### irix-ld --no-undefined fix
`--no-undefined` with shared libraries causes failures because IRIX libc has `__tls_get_addr` references. Fixed by filtering `--no-undefined` in irix-ld for shared library links. Shared libraries resolve symbols at load time via rld.

### FC31→FC40 patch staleness
SGUG-RSE sgifixes patches from FC31 often don't apply to FC40 sources. For expat, freetype — removed stale patches and applied fixes via mogrix rules instead.

### Meson packages with autotools fallback (fribidi)
Fribidi has `%if 0%{?rhel} && 0%{?rhel} <= 8` guards around autotools code. Replaced with `%if 1` to force autotools path. Also fixed `utime()` name clash (IRIX has `utime(2)` which conflicts with fribidi's `utime()` function).

### OSSP uuid cross-compilation
Old configure script has `va_copy()` test that tries to execute. Fix: `ac_cv_va_copy: "C99"`. Perl module build commands must be individually commented out (pushd/popd replacement only handles one line).

## Key Lessons Learned (Session 1)

### Bootstrap HOST binaries in cross-compilation
Packages like flex build HOST tools (stage1flex) before cross-compiling. `inject_compat_functions` and `LIBS` affect the HOST link too. Fix: inject compat code directly into cross-only source files via prep_commands.

### IRIX libgen.so
`basename()` and `dirname()` are in `/usr/lib32/libgen.so`, not libc. Copied to staging for cross-linker access. Also added `compat/string/basename.c` for packages where `-lgen` is problematic.

### tcl.m4 platform detection
Many packages with custom platform detection use `uname` directly instead of autoconf's `--host`. These always detect "Linux" during cross-compilation. Fix: override `tcl_cv_sys_version` or similar variables.

### gnulib stdint.h conflicts
gnulib's `gl_STDINT_H` generates replacement headers with different integer types. Fix: `gl_cv_header_working_stdint_h: "yes"` tells gnulib not to generate its own.

### Unpackaged doc files pattern
`make install` puts docs in `$(pkgdocdir)` but rpmbuild's `%doc` macro handles them separately. When both install docs, you get "unpackaged files" errors. Fix: `install_cleanup` to remove the make install copies.

---

## Known Issues

- **`%zu` format specifier crashes IRIX libc** — use `%u` for size_t
- **Volatile function pointer static initializers crash** — provide `explicit_bzero`
- **IRIX chroot doesn't fully isolate** — processes see base system paths
- fopencookie crashes on IRIX — use funopen instead
- sqlite3 CLI crashes when writing to files
- pkgconf installed with `--nodeps` (needs rebuild with `drop_requires: libpkgconf`)
- coreutils `seq` disabled — IRIX printf doesn't handle `%Lg`
- **`--export-dynamic` crashes IRIX rld** — large dynamic symbol tables cause SIGSEGV

---

## Reference Documents

| Document | Purpose |
|----------|---------|
| `rules/INDEX.md` | Rules reference, per-package problem guide |
| `rules/methods/before-you-start.md` | Checklist before debugging |
| `rules/generic.yaml` | Universal rules for all packages |
| `compat/catalog.yaml` | Compat function registry |
| `plan.md` | Project plan and architecture |
| `aterm_roadmap.md` | Phase 5 roadmap toward aterm |
