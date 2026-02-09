# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-09 (session 9)
**Status**: Phase 5 + OpenSSH + tdnf WORKING. 77 source packages cross-compiled (207 RPMs). Batch-build + rule promotion workflow operational.

---

## CRITICAL WARNINGS

**NEVER install packages directly to /usr/sgug on the live IRIX system.**

On 2026-01-27, installing directly to /usr/sgug replaced libz.so with zlib-ng, broke sshd, and locked out SSH access. System recovered from console using backup libs.

**Always use /opt/chroot for testing.** This is our clean chroot bootstrapped via `scripts/bootstrap-tarball.sh`. The old `/opt/chroot_0206` has the full SGUG-RSE base (backup only).

**NEVER CHEAT!** Work through problems converting packages. Don't copy from old SGUG-RSE. Don't fake Provides. Check the rules for how to handle being stuck.

**Question SGUG-RSE assumptions.** SGUG-RSE bootstrapped ON IRIX without most deps. They bundled things and skipped system libraries out of necessity. We cross-compile with a full sysroot — prefer system libraries over bundled copies.

---

## Goal

Cross-compile Fedora 40 packages for IRIX using mogrix. Phases 1-4c complete (41 packages). Phase 5 complete: library foundation + aterm (first X11 app). OpenSSH 9.6p1 cross-compiled and working. 65 total source packages cross-compiled. System ready for /usr/sgug swap.

---

## What Was Done This Session (Session 7)

### `mogrix batch-build` command — IMPLEMENTED

New command that automates the fetch → convert → build pipeline for multiple packages:

```bash
mogrix batch-build --from-list packages.txt          # List mode
mogrix batch-build --target gdb                       # Roadmap mode (dep-ordered)
mogrix batch-build --from-list t.txt --dry-run        # Preview without building
mogrix batch-build --from-list t.txt --output-report r.json  # JSON report
```

**New files:**
- `mogrix/batch_build.py` — BatchBuilder orchestrator, failure classifier, Rich report
- `mogrix/rule_generator.py` — Generates candidate YAML rule files from source analysis

**Features:**
- Auto-fetches SRPMs from Fedora archives
- For packages without rules: scans source with SourceAnalyzer, generates candidate rules in `rules/candidates/`
- Conservative auto-detection (>90% confidence): `inject_compat_functions`, `drop_buildrequires` (known Linux deps), `classes: [nls-disabled]`
- Build timeout (default 600s) prevents hangs
- Always moves on — never blocks on failures
- Rich table output + JSON report
- Skips already-built packages by default

**Bug fix:** `batch.py` was missing `get_extra_files()` for compat injection — `dlmalloc-src.inc` wasn't being copied into SRPMs. Fixed.

**Test results:**
- dry-run: correctly identifies built/rules/no-rules packages
- candidate generation: detected strdup, getopt_long, texinfo in gmp/bc/time
- full pipeline: pcre2 convert+build → 7 RPMs in 63s
- failure classification: gperf build failure correctly tagged as spec_error
- JSON report: clean output with per-package status, duration, findings

---

## What Was Done This Session (Session 8)

### nano 7.2 — COMPLETE
- **gnulib stdint.h conflict**: Elevated `gl_cv_header_working_stdint_h: "yes"` to `generic.yaml` (was duplicated across 5 packages)
- **IRIX libgen.so**: `dirname()`/`basename()` in `/usr/lib32/libgen.so`, not libc. Fixed with `LIBS="$LIBS -lgen"` before `%configure`
- **spec_replacements ordering**: Replacements run BEFORE configure_flags — must match original `%configure`, not `%configure --disable-nls`
- **%files fixes**: Brace expansions (`{,r}nano`), texinfo removal, `%files -f build/` path, doc cleanup
- Verified: `nano --version` (7.2, UTF-8), `nano --help` — working on IRIX

### rsync 3.2.7 — COMPLETE
- **Conditional %prep bug**: Emitter injected compat copies into dead `%if %isprerelease` branch. Fixed by collapsing to `%if 0` and re-injecting after `%patch 2`
- **daemon subpackage**: Dropped systemd units, scriptlets via spec_replacements
- **Long double crash (`__extenddftf2`)**: rsync's bundled snprintf uses `LDOUBLE long double` for `%f` formatting. IRIX MIPS doesn't support 128-bit long double. Fix: `ac_cv_type_long_double_wider: "no"` → `LDOUBLE` falls back to `double`
- Verified: `rsync --version`, `rsync -v`, `rsync --stats`, `rsync --info=progress2` — all working

### unzip 6.0, zip 3.0 — COMPLETE (previous session continuation)
- Simple builds with minimal rules

### Pattern: IRIX long double crash
Same class as coreutils `seq` issue. IRIX MIPS n32 doesn't have 128-bit long double support. `__extenddftf2` (double→long double conversion) crashes with SIGSEGV. Fix: disable `HAVE_LONG_DOUBLE` via `ac_cv_type_long_double_wider: "no"` or equivalent configure cache variable.

---

## What Was Done This Session (Session 9)

### Batch-build 68 packages + rule promotion
- Ran `mogrix batch-build` on 68 packages from roadmap. 53 candidates generated, 6 build failures identified.
- Promoted 14 candidates to `rules/packages/`: figlet, lolcat, sl, time, bc, hyphen, xclip, cmatrix, gmp, mpfr, libevent, libxslt, libstrophe, giflib
- Cleaned up 4 stale candidate files (nano, rsync, unzip, zip already had rules)

### 8 new packages built successfully
- **figlet** — Makefile build with CC injection
- **sl** — Makefile build with ncurses
- **time** — autotools, drop texinfo/gnupg2, gpgverify removal
- **cmatrix** — autotools, drop help2man/console-setup/x11-fonts
- **gmp** — autotools, strdup compat, git autosetup → p1
- **mpfr** — autotools, `--disable-thread-safe` (no __thread TLS on IRIX)
- **hyphen** — autotools, pushd/popd → cd fix
- **libevent** — autotools, strsep compat, test dir removal, doxygen disabled, doc mv fix

### Failure investigation: groff and git
- **groff**: Fixed brace expansion in %prep. BLOCKED by C++ — irix-cxx uses `clang` not `clang++`, configure fails "header files do not support C++"
- **git**: Fixed 3 brace expansions in spec. Needs pcre2 headers staged, doc build disabled (no asciidoc), extensive rules work for full build
- Both had stale `add_patch` directives (from SGUG-RSE era patches for older versions) that were removed

### Packages that need more work
- **lolcat** — needs `err.h` header + wide char I/O (wprintf/fwprintf) support
- **bc** — bootstrap problem: `./fbc` is MIPS binary needed at build time
- **xclip** — XA_UTF8_STRING not in IRIX Xmu headers (too old)
- **giflib** — needs `%cmake` macro for cross-build
- **libxslt** — python-3.12 pkg-config dependency
- **libstrophe** — res_query() configure test failure

### Patterns discovered
- **Brace expansion in specs**: `{a,b,c}` is bash-only, rpmbuild uses /bin/sh. Expand manually via spec_replacements. Found in groff, git.
- **pushd/popd**: Also bash-only. Replace with `cd`/`cd -`.
- **`%{__python3}` in %prep**: python3 pathfix.py not available, remove for cross-build
- **`%(c=%{commit}; echo ${c:0:7})`**: Bash substring in RPM macro, hardcode value
- **Compat header conflicts**: Our mogrix-compat headers declare functions (strsep, setenv, etc.) but configure says "no" → package provides its own static version → "static follows non-static" error. Fix: ac_cv override to say "yes" + inject compat if needed.

---

## IMMEDIATE NEXT: /usr/sgug Swap + Choose Next Target

OpenSSH is COMPLETE (working on IRIX in debug mode). The system is now ready for the /usr/sgug swap — see "Swap Procedure" section below.

After swap, possible next package targets:

### Autotools Packages Ready to Build
- **gd** — graphics library, autotools
- **jq** — JSON processor, autotools (needs select() declaration fix)
- **pcre** — older regex library, autotools
- **gnutls** — TLS library, autotools
- **groff** — text formatting, autotools
- **libarchive** — archive library, autotools
- **elfutils** — ELF handling, autotools
- **gtk2** — GTK toolkit, autotools

### Meson Packages (require meson cross-compilation)
FC40 GNOME stack has migrated to meson:
- **cairo** 1.18.0 — `%meson` (1.16.x was autotools)
- **harfbuzz** — `%meson`
- **pango** — `%meson`
- **glib2** — `%meson`
- **p11-kit** — `%meson`

We have `cross/meson-irix-cross.ini` and pixman was successfully built with meson.

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
| aterm | 1.0.1 | INSTALLED | X11 sysroot autodetect, drop libAfterImage/chrpath/desktop, -rdynamic filter |
| openssh | 9.6p1 | INSTALLED | R_MIPS_REL32 dispatch functions, ensure_minimum_time bypass, debug-mode only |

---

## What Was Fixed This Session (Session 6)

### 1. tdnf Error(1602) ROOT CAUSE FOUND AND FIXED
`tdnf repolist` failed with `Error(1602) : No such file or directory`. Error code 1602 = `ERROR_TDNF_SYSTEM_BASE (1600) + ENOENT (2)`.

**Root cause**: `TDNFAllocateStringPrintf()` in `common/strings.c` uses `vsnprintf(&chDstTest, 1, fmt, args)` to measure output length. On IRIX, the non-XPG5 `vsnprintf` returns 0 (not total chars needed) when output is truncated. The code then checks `if(nSize <= 0)` and reads `errno`, which contains stale ENOENT from earlier operations → error 1602.

**Verified on IRIX** with test binary confirming:
- `snprintf(buf,1,"%s-%s","mogrix","abcd1234")` returns `ret=0` (C99 expects 15)
- `snprintf(NULL,0,"%s","hello")` returns `ret=-1` (C99 expects 5)
- Stale errno is preserved across calls

**Fix**: `tdnf-vsnprintf-irix.sgifixes.patch` — replaces C99-dependent `vsnprintf(&buf,1,...)` measuring approach with iterative buffer-growing (start at 256 bytes, double on truncation). Works on both C99 and pre-C99 systems.

### 2. cmake cross-compilation regression fixes
New Phase 4/5 packages in staging (`make`, `pkg-config`) caused cmake to find MIPS binaries instead of host tools. Fixed with:
- `-DCMAKE_MAKE_PROGRAM=/usr/bin/make` — prevent cmake from using cross-compiled gmake
- `-DPKG_CONFIG_EXECUTABLE=/usr/bin/pkg-config` — prevent cmake from using cross-compiled pkg-config

### 3. SQLite WAL fix from previous session confirmed working
`rpm -qa` lists 83 packages. rpmdb uses DELETE journal mode (SQLITE_OMIT_WAL).

### tdnf verified operations on IRIX:
- `tdnf --version` → `tdnf: 3.5.14`
- `tdnf repolist` → shows mogrix repo enabled
- `tdnf makecache` → "Metadata cache created."
- `tdnf list` → 269 entries (installed + available)
- `tdnf reinstall tree` → full download/install/remove cycle works

---

## What Was Fixed in Session 5

### 1. OpenSSH 9.6p1 cross-compilation (first network service!)
FC40 package, portable SSH server/client. Key approach: drop ALL ~40 Fedora patches (GSSAPI, SELinux, audit, systemd, FIPS, PKCS#11), use clean upstream with OpenSSH's own `openbsd-compat/` portability layer. No mogrix compat injection needed.

**Critical fixes:**
- **R_MIPS_REL32 relocation elimination**: IRIX rld fails on R_MIPS_REL32 dynamic relocations for function pointers in static data arrays. Eliminated ALL 20 relocations:
  - **cipher.c (9 EVP relocations)**: Added `ssh_cipher_evptype()` dispatch function (strcmp-based switch returning EVP_*() call results). Replaced `(*cipher->evptype)()` indirect call. NULLed out EVP pointers in ciphers[] array initializers.
  - **charclass.h (11 ctype relocations)**: Replaced entire header. Removed `isctype` function pointer field from struct. Added `cclass_isctype()` dispatch function (strcmp → isalnum/isalpha/etc). Fixed glob.c and fnmatch.c call sites via sed.
  - **digest-openssl.c**: Switch dispatch for EVP_MD functions (from previous session)
  - **explicit_bzero.c**: Volatile char loop replacing volatile function pointer (from previous session)
- **`ensure_minimum_time_since()` bypass**: Crashes privsep child after authentication (only for non-"none" methods). Contains monotime_double/SHA512/nanosleep — root cause unclear. Bypassed with `if(0)`. Timing side-channel protection disabled (acceptable for IRIX LAN).
- **BROKEN_SNPRINTF**: IRIX vsnprintf(NULL,0) returns -1. Forced via `-DBROKEN_SNPRINTF` in CFLAGS.
- **Fork-mode sshd failure**: Connection socket returns EOF (SSH_ERR_CONN_CLOSED) after privsep child exits. Likely IRIX kernel socket refcount bug after fork. Debug mode (`-d`, no fork) works perfectly.
  - **Workaround**: Run `while true; do /usr/sgug/sbin/sshd -d -e -p 22; done` for persistent service.
- **perl -i -e slurp mode bug**: `perl -i -e 'my @lines = <>; ... print $content;'` WIPES the file. Must use explicit `open(F, "<file"); ... open(F, ">file"); print F $content;` instead.

### 2. SSH login verified on IRIX
```
$ ssh root@192.168.0.81 -p 9992
OpenSSH_9.6p1, OpenSSL 3.2.1 30 Jan 2024
IRIX64 blue 6.5 07202013 IP30 mips Irix
```
Full session: command execution, clean disconnect. Ed25519 and RSA keys work.

---

## /usr/sgug Swap Procedure

**Safety invariant**: At least one working sshd must be listening at all times.

### Pre-swap checklist
- [x] openssh tested and working in chroot on alternate port
- [ ] Physical/serial console access available as fallback
- [ ] Backup of live /usr/sgug exists

### Swap sequence
```bash
# 1. From existing SSH session on port 22 (keep alive!):
cd /opt && tar cf sgug-live-backup.tar /usr/sgug

# 2. Start test sshd from chroot on alternate port:
chroot /opt/chroot /bin/sh -c 'LD_LIBRARYN32_PATH=/usr/sgug/lib32 /usr/sgug/sbin/sshd -d -e -p 2222'
# Note: must use -d (debug/no-fork mode) — fork mode has IRIX socket bug

# 3. Verify port 2222 works from another machine

# 4. From the port 2222 session, do the swap:
mv /usr/sgug /usr/sgug.old
cp -a /opt/chroot/usr/sgug /usr/sgug

# 5. Start new sshd on port 2223 (from new /usr/sgug):
LD_LIBRARYN32_PATH=/usr/sgug/lib32 /usr/sgug/sbin/sshd -d -e -p 2223

# 6. If port 2223 works, set up persistent sshd on port 22
```

### Rollback (if swap fails)
```bash
# From physical console:
mv /usr/sgug /usr/sgug.new && mv /usr/sgug.old /usr/sgug
LD_LIBRARYN32_PATH=/usr/sgug/lib32 /usr/sgug/sbin/sshd
```

---

## What Was Fixed in Session 4

### 1. aterm cross-compilation (first X11 app!)
FC39 package, simple autotools VT102 terminal emulator. Key fixes:
- Remove `--x-includes`/`--x-libraries` flags — sysroot autodetects X11 headers/libs
- Drop libAfterImage (optional, configure skips gracefully), chrpath, desktop-file-utils
- Drop `xorg-x11-fonts-misc` Requires (IRIX has native X11 fonts)
- Disable CJK font support (`--disable-kanji --disable-big5 --disable-greek`) — k14/taipei16 fonts not available on IRIX
- Man page `.gz` suffix removal (brp scripts disabled in rpmmacros.irix)
- Links: libXt.so, libX11.so.1, libXext.so, libc.so.1 (all IRIX native)

### 2. X11 testing from chroot
- UNIX sockets (`/tmp/.X11-unix/X0`) don't cross chroot boundaries — `DISPLAY=:0` won't work
- SSH X forwarding auth cookies not available in chroot — `DISPLAY=localhost:13.0` won't work
- **TCP display works**: `DISPLAY=192.168.0.81:0` (machine's own IP) bypasses socket/chroot issues
- Must run `xhost +` from a **local GUI terminal** first (not from SSH — `xhost` needs DISPLAY set)
- Launch pattern: `DISPLAY=:0 chroot /opt/chroot /bin/sh -c 'DISPLAY=192.168.0.81:0 /usr/sgug/bin/sgug-exec /usr/sgug/bin/aterm'`

### 3. `-rdynamic`/`--export-dynamic` filter in irix-ld
LLD doesn't support `-rdynamic`. Also dangerous for IRIX rld (SIGSEGV on large dynamic symbol tables). Added to filter in both `cross/bin/irix-ld` and staging copy.

### 4. tdnf repo setup (FIXED in session 6)
- `createrepo_c --simple-md-filenames` creates repo metadata on Linux
- Tar and copy to chroot, extract to `/tmp/mogrix-repo`
- Session 5 fixed SQLite WAL locking (SQLITE_OMIT_WAL + rpmdb conversion)
- Session 6 fixed Error(1602) (IRIX pre-C99 vsnprintf in TDNFAllocateStringPrintf)
- tdnf now fully working: repolist, makecache, list, reinstall all verified

### 5. sgugshell shebang fix (live system)
- Mogrix bash links libtinfo.so, SGUG-RSE's original bash didn't
- `#!/usr/sgug/bin/bash` in sgugshell loads bash before `LD_LIBRARYN32_PATH` is set → rld can't find libtinfo.so
- Fix: changed shebang to `#!/bin/ksh` (native IRIX shell, like sgug-exec uses)

### 6. Roadmap performance fix
`_build_exclusion_index()` scanned all binary_provides rows (millions) in the 1GB Fedora sqlite DB, pegging CPU and crashing. Removed entirely — lazy `_check_roadmap_drop()` after sqlite lookups achieves the same filtering.

### 7. Expanded roadmap_config.yaml and sysroot_provides.yaml
- Added gcc subpackage provides (cpp, libgcc, libstdc++, etc.)
- Added ~50 more drop patterns (impossible ecosystems, desktop frameworks, linux-specific)
- Reduced NEED_RULES from 2804 to 2137 (24% reduction)

---

## What Was Fixed in Session 3

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

**Total: 77 source packages cross-compiled (207 RPMs).**

### Phases 1-4c: 41 packages (ALL INSTALLED)
Bootstrap (14) + system libs (2) + build tools (6) + crypto (7) + text tools (3) + utilities (9).

### Phase 5: Library Foundation + aterm + openssh (24 packages)
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
| aterm | 1.0.1 | INSTALLED | X11 sysroot autodetect, CJK disabled, -rdynamic filter, first X11 app |
| openssh | 9.6p1 | INSTALLED | R_MIPS_REL32 dispatch, ensure_minimum_time bypass, debug-mode only |
| unzip | 6.0 | INSTALLED | Simple build |
| zip | 3.0 | INSTALLED | Simple build |
| nano | 7.2 | INSTALLED | -lgen for dirname/basename, gnulib stdint.h, brace expansion %files |
| rsync | 3.2.7 | INSTALLED | Long double crash fix, conditional %prep compat injection, daemon dropped |
| figlet | 2.2.5 | BUILT | Makefile CC injection |
| sl | 5.02 | BUILT | Makefile CC injection with ncurses |
| time | 1.9 | BUILT | Drop texinfo/gnupg2, gpgverify removal |
| cmatrix | 2.0 | BUILT | Drop help2man/console-setup/x11-fonts |
| gmp | 6.2.1 | BUILT | strdup compat, git autosetup → p1 |
| mpfr | 4.2.1 | BUILT | --disable-thread-safe (no __thread TLS) |
| hyphen | 2.8.8 | BUILT | pushd/popd → cd, drop valgrind |
| libevent | 2.1.12 | BUILT | strsep compat, test removal, doxygen disabled |

---

## Environment

| Purpose | Path |
|---------|------|
| Mogrix project | `/home/edodd/projects/github/unxmaal/mogrix/` |
| Cross toolchain | `/opt/cross/bin/` |
| Staging area | `/opt/sgug-staging/usr/sgug/` |
| IRIX sysroot | `/opt/irix-sysroot/` |
| IRIX host | `192.168.0.81` (use MCP, not SSH) |
| IRIX chroot (active) | `/opt/chroot` (bootstrapped, ~58 source packages installed) |
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
- **tdnf install dependency resolution** — some packages have unresolvable deps due to missing `Provides:` in cross-compiled packages (need explicit Provides for all .so files)
- **Man page .gz in %files** — brp scripts disabled, man pages stay uncompressed. Fix: `%{name}.1.gz` → `%{name}.1*`
- **CJK fonts not on IRIX** — aterm's k14/taipei16/greek fonts don't exist. Disable with `--disable-kanji --disable-big5 --disable-greek`
- **R_MIPS_REL32 relocations crash IRIX rld** — function pointers in static/const data arrays. Fix: dispatch functions (switch/strcmp). Affects openssh cipher.c, charclass.h, digest-openssl.c, explicit_bzero.c
- **openssh fork-mode broken** — connection socket EOF after privsep child exit. Use `-d` (debug/no-fork mode) in a loop
- **openssh ensure_minimum_time_since crash** — privsep child dies after non-"none" auth. Bypassed with `if(0)`

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
