# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-08 (session 3)
**Status**: Phase 5 (Library Foundation) IN PROGRESS. Tier 0-2 building. 63 source packages cross-compiled. gettext, zstd, fontconfig INSTALLED on IRIX.

---

## CRITICAL WARNINGS

**NEVER install packages directly to /usr/sgug on the live IRIX system.**

On 2026-01-27, installing directly to /usr/sgug replaced libz.so with zlib-ng, broke sshd, and locked out SSH access. System recovered from console using backup libs.

**Always use /opt/chroot for testing.** This is our clean chroot bootstrapped via `scripts/bootstrap-tarball.sh`. The old `/opt/chroot_0206` has the full SGUG-RSE base (backup only).

**NEVER CHEAT!** Work through problems converting packages. Don't copy from old SGUG-RSE. Don't fake Provides. Check the rules for how to handle being stuck.

**Question SGUG-RSE assumptions.** SGUG-RSE bootstrapped ON IRIX without most deps. They bundled things and skipped system libraries out of necessity. We cross-compile with a full sysroot — prefer system libraries over bundled copies.

---

## Goal

Cross-compile Fedora 40 packages for IRIX using mogrix. Phases 1-4c complete (41 packages). Now building Phase 5: library foundation packages toward aterm (X11 terminal emulator). 63 total source packages cross-compiled.

---

## IMMEDIATE NEXT: aterm Path

aterm is a simple Xlib terminal emulator (FC39: 1.0.1). Direct BuildRequires:
- `libAfterImage-devel >= 1.07` — NOT in Fedora, need separate source
- `libXt-devel` — X11 toolkit, likely in SGUG-RSE sysroot
- `libXext-devel` — X11 extensions, likely in SGUG-RSE sysroot
- `mesa-libGL-devel` — OpenGL, may skip
- `make` ✅, `chrpath` ✅, `gcc` (cross-compiler)

**Key question: Is libAfterImage already in the SGUG-RSE sysroot, or does it need building?**

### Autotools Packages Ready to Build
These use `%configure`/`%make_build` and have rules or are straightforward:
- **gd** — graphics library, autotools
- **jq** — JSON processor, autotools (needs select() declaration fix)
- **pcre** — older regex library, autotools
- **gnutls** — TLS library, autotools
- **groff** — text formatting, autotools
- **libarchive** — archive library, autotools
- **elfutils** — ELF handling, autotools
- **gtk2** — GTK toolkit, autotools

### Meson Packages (BLOCKED — need meson cross-compilation)
FC40 GNOME stack has migrated to meson:
- **cairo** 1.18.0 — `%meson` (1.16.x was autotools)
- **harfbuzz** — `%meson`
- **pango** — `%meson`
- **glib2** — `%meson`
- **p11-kit** — `%meson`

**Options**: (1) Set up meson cross-compilation with `cross/meson-irix-cross.ini`, (2) Find older autotools-based versions, (3) Skip if not needed for aterm.

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

### Tier 2: Small Chains (IN PROGRESS)
| Package | Version | Status | Key Fixes |
|---------|---------|--------|-----------|
| libunistring | 1.1 | BUILT | Clean build |
| gettext | 0.22.5 | INSTALLED | %files substring collision fix, common-devel dep drop |
| zstd | 1.5.5 | INSTALLED | Makefile CC/AR/RANLIB exports, pthread detection, LDFLAGS preservation |
| fontconfig | 2.15.0 | INSTALLED | Host gperf pre-generation, expat backend, __sync atomics, test dir removal |
| freetype | 2.13.2 | INSTALLED | Rebuilt with explicit Provides (mips-32, libfreetype.so.6) |
| expat | 2.6.0 | STAGED | prep_commands touch for man page, --without-docbook |
| nettle | 3.9.1 | STAGED | --enable-mini-gmp, FIPS disabled, LD export |
| libtasn1 | 4.19.0 | STAGED | GTKDOCIZE=true, gl_cv_header_working_stdint_h, docs disabled |
| fribidi | 1.0.13 | STAGED | autotools path via %if 1, utime rename, getopt_long compat |
| libjpeg-turbo | 3.0.2 | STAGED | raw cmake, SIMD off, setenv compat, lib32 hardcode |
| pixman | 0.43.0 | STAGED | meson cross file, tests/demos disabled |
| uuid | 1.6.2 | STAGED | ac_cv_va_copy=C99, perl disabled, libtool override removed |

---

## What Was Fixed This Session (Session 3)

### 1. PKG_CONFIG_SYSROOT_DIR (systemic fix)
pkg-config returns paths like `/usr/sgug/include/freetype2` but that doesn't exist on the build host. Added `export PKG_CONFIG_SYSROOT_DIR="/opt/sgug-staging"` to `%configure` in `cross/rpmmacros.irix`. Affects ALL packages using pkg-config.

### 2. dicl-clang-compat/limits.h overlay
IRIX `limits.h` guards C99 long long limits (LLONG_MIN, LLONG_MAX, ULLONG_MAX) behind `__c99` (MIPSpro flag). Created overlay with `#include_next` + explicit defines.

### 3. Fontconfig gperf CPP regeneration
Cross-compiler CPP injects `typedef __builtin_va_list va_list;` into gperf input, corrupting it. Fix: pre-generate `fcobjshash.gperf` and `fcobjshash.h` using HOST cpp+gperf before make runs. The tarball does NOT ship these files — they MUST be generated during build.

### 4. Fontconfig test compilation during make all
Test programs use `setenv()` (not on IRIX) and compile during `make all` not just `make check`. Fix: `sed -i 's/ test$//' Makefile.am` before autoreconf.

### 5. Explicit Provides for cross-compiled packages
AutoProv is disabled in rpmmacros.irix. Packages need explicit `Provides:` for shared libraries and ISA-specific capabilities. Added to freetype (`libfreetype.so.6`, `freetype(mips-32)`) and fontconfig (`libfontconfig.so.1`, `fontconfig(mips-32)`).

### 6. drop_requires multi-package line bug
`drop_requires` in `emitter/spec.py` has a regex bug: Pattern 2 (multi-package Requires line) uses `.+(?:^|\s)dep` which fails when the dep is the FIRST package on the line (`.+` requires at least one char before `(?:^|\s)`). Workaround: use `spec_replacements` to replace the entire multi-package line.

### 7. Makefile-based cross-compilation (zstd)
Packages using plain Makefiles (not autotools) don't get CC from `%configure`. Must explicitly `export CC="%{__cc}"`, `export AR="%{__ar}"`, `export RANLIB="%{__ranlib}"`. Also: `LDFLAGS="$LDFLAGS $RPM_LD_FLAGS"` to APPEND (not overwrite mogrix LDFLAGS).

### 8. Gettext %files substring collision
Pattern `%{_libdir}/libgettextpo.so` matched inside `%{_libdir}/libgettextpo.so.0*`, corrupting filenames. Fix: use unique, non-overlapping anchor strings for each spec_replacement.

### 9. Cairo 1.18.0 uses meson (BLOCKED)
FC40 cairo uses `%meson` build system. SGUG-RSE patch for 1.16.0 doesn't apply. Options: meson cross-compilation, older autotools version, or skip.

---

## What Was Fixed in Session 2

### Meson cross-compilation support
Installed meson 1.10.1 via `uv tool install meson`. Created `cross/meson-irix-cross.ini` cross file. Key issue: irix-cc wrapper was forwarding `--version` to linker instead of compiler.

### cmake cross-compilation (libjpeg-turbo)
Replace `%{cmake}` macros with raw cmake commands. Use `-DCMAKE_INSTALL_LIBDIR=lib32` (hardcoded). Pass compat via `-DCMAKE_EXE_LINKER_FLAGS`.

### irix-cc/irix-ld wrapper improvements
- Added `--version`/`-v`/`--help`/`-dumpversion`/`-dumpmachine` handling
- Added `-ggdb*`/`-gdwarf*`/`-gz*` to link-only filter
- `--no-undefined` filtered for shared library links (IRIX libc has `__tls_get_addr` refs)

### Other session 2 fixes
- configure_flags:remove regex bug (lookahead for `--enable-jit` vs `--enable-jit-sealloc`)
- Duplicate add_patch/add_source engine bug
- C99 inttypes.h compliance, LLVM #52785, __ASSEMBLER__ guards, byteswap.h, basename compat
- gnulib stdint.h conflict, tcl cross-compilation detection

---

## Current Status

**Total: 63 source packages cross-compiled.**

### Phases 1-4c: 41 packages (ALL INSTALLED)
Bootstrap (14) + system libs (2) + build tools (6) + crypto (7) + text tools (3) + utilities (9).

### Phase 5: Library Foundation (22 packages)
| Package | Version | Status | Key Fixes |
|---------|---------|--------|-----------|
| pcre2 | 10.42 | INSTALLED | JIT/sealloc blocks, inttypes.h C99 |
| symlinks | 1.7 | INSTALLED | CC override for raw Makefile |
| tree-pkg | 2.1.0 | INSTALLED | Clean build |
| oniguruma | 6.9.9 | INSTALLED | fix-libtool-irix.sh |
| libffi | 3.4.4 | INSTALLED | -fno-integrated-as (LLVM #52785) |
| tcl | 8.6.13 | INSTALLED | tcl_cv_sys_version=IRIX64-6.5 |
| flex | 2.6.4 | INSTALLED | nls-disabled, inline basename |
| chrpath | 0.16 | INSTALLED | byteswap.h compat |
| libpng | 1.6.40 | INSTALLED | pngcp/pngfix stubs |
| bison | 3.8.2 | INSTALLED | gl_cv_header_working_stdint_h |
| libunistring | 1.1 | BUILT | Clean build |
| gettext | 0.22.5 | INSTALLED | %files substring collision, dep fixes |
| zstd | 1.5.5 | INSTALLED | Makefile CC/AR/RANLIB, pthread, LDFLAGS |
| fontconfig | 2.15.0 | INSTALLED | Host gperf, expat, __sync atomics |
| freetype | 2.13.2 | INSTALLED | Explicit Provides for ISA + .so |
| expat | 2.6.0 | STAGED | man page touch, --without-docbook |
| nettle | 3.9.1 | STAGED | --enable-mini-gmp |
| libtasn1 | 4.19.0 | STAGED | GTKDOCIZE=true |
| fribidi | 1.0.13 | STAGED | autotools path, utime rename |
| libjpeg-turbo | 3.0.2 | STAGED | raw cmake, SIMD off |
| pixman | 0.43.0 | STAGED | meson cross file |
| uuid | 1.6.2 | STAGED | ac_cv_va_copy=C99 |

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
