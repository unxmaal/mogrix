# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-01-25 21:00
**Status**: IRIX RUNTIME TESTING - bzip2 validated working on IRIX!

---

## Session 2026-01-25 Evening: IRIX Runtime Validation

### What Was Done
1. Created bootstrap scripts (`scripts/bootstrap-tarball.sh`, `scripts/irix-chroot-testing.sh`, `scripts/irix-bootstrap.sh`)
2. Built all 12 packages and created bootstrap tarball
3. Tested on IRIX - discovered binaries segfaulted
4. Diagnosed issue: wrong linker selection (GNU ld vs LLD)
5. Fixed `irix-ld` wrapper: GNU ld for shared libs, LLD for executables
6. **Validated bzip2 runs on IRIX!**

### What Remains
- Rebuild all 12 packages with fixed linker
- Regenerate bootstrap tarball
- Test rpm and tdnf on IRIX
- Complete bootstrap process

### Key Files Modified
- `/opt/sgug-staging/usr/sgug/bin/irix-ld` - Linker wrapper with correct GNU ld / LLD selection
- `scripts/bootstrap-tarball.sh` - Creates tarball in `tmp/` directory
- `scripts/irix-chroot-testing.sh` - Safe chroot testing (no auto-delete)
- `scripts/irix-bootstrap.sh` - Real installation script

---

## Goal

Validate mogrix workflow by building tdnf dependency chain one package at a time.

**Critical rule**: Use FC40 exclusively. Never mix Fedora releases - the whole point is that Fedora solved dependencies for us.

---

## Build Order (FC40)

| # | Package | Version | Status | Notes |
|---|---------|---------|--------|-------|
| 1 | **zlib-ng** | 2.1.6 | **COMPLETE** | FC40 replaced zlib with zlib-ng; cmake-based |
| 2 | bzip2 | 1.0.8 | **COMPLETE** | Built first try |
| 3 | popt | 1.19 | **COMPLETE** | Built first try (stpcpy already in mogrix) |
| 4 | openssl | 3.2.1 | **COMPLETE** | Built first try |
| 5 | libxml2 | 2.12.5 | **COMPLETE** | Built first try |
| 6 | curl | 8.6.0 | **COMPLETE** | Built first try |
| 7 | xz | 5.4.6 | **COMPLETE** | Built first try |
| 8 | lua | 5.4.6 | **COMPLETE** | Built first try |
| 9 | file | 5.45 | **COMPLETE** | Required posix_spawn_file_actions_* functions |
| 10 | rpm | 4.19.1.1 | **COMPLETE** | Built first try (after file compat fix) |
| 11 | libsolv | 0.7.28 | **COMPLETE** | Built first try |
| 12 | **tdnf** | **3.5.14** | **COMPLETE** | **TARGET ACHIEVED** - from Photon OS SRPM |

---

## Completed: zlib-ng

### Root Cause
zlib-ng's `zbuild.h` defines `_POSIX_SOURCE` and `_POSIX_C_SOURCE` which breaks IRIX's feature macro chain. This makes `_NO_POSIX` false, which makes `_SGIAPI` false, which prevents snprintf/vsnprintf from being declared.

### Solution
Add `-D_XOPEN_SOURCE=600` to CMAKE_C_FLAGS. This makes `_XOPEN5` true, which exposes snprintf/vsnprintf regardless of POSIX macros.

Full flag set needed:
```
-DCMAKE_C_FLAGS="-I/usr/sgug/include/mogrix-compat/generic -D_SGI_SOURCE -D__EXTENSIONS__ -D_XOPEN_SOURCE=600"
```

### Additional Fixes Required
1. **llabs declaration**: IRIX libc doesn't have `llabs()`. Added extern declaration in compat/stdlib.h that Clang resolves via builtin.
2. **Versioned library symlinks**: CMake doesn't create versioned .so files for IRIX. Added post-install commands to create `libz-ng.so.2.1.6`, `libz-ng.so.2` symlinks, and similar for compat `libz.so.1`, `libz.so.1.3.0.zlib-ng`.

### Files Changed
- `rules/packages/zlib-ng.yaml` - complete cmake cross-compilation setup
- `compat/include/mogrix-compat/generic/stdlib.h` - llabs declaration
- `compat/include/mogrix-compat/generic/stdint.h` - UINT64_C etc.

---

## Fetch Command Update

`mogrix fetch` now defaults to FC40 and shows the full archive URL:
```
Fetching SRPMs from Fedora 40 archives
https://archives.fedoraproject.org/pub/archive/fedora/linux/releases/40/Everything/source/tree/Packages/
```

---

## Cleanup

Use `./cleanup.sh` for a complete reset. The script:
1. Cleans staging lib32/ and include/
2. Restores compat headers
3. Builds runtime libraries (libsoft_float_stubs.a, libatomic.a)
4. Cleans rpmbuild directories

---

## Per-Package Workflow

```bash
source .venv/bin/activate
mogrix fetch <package>
mogrix convert ~/rpmbuild/SRPMS/<package>-*.src.rpm
mogrix build ~/rpmbuild/SRPMS/<package>-*-converted/<package>-*.src.rpm --cross
mogrix stage ~/rpmbuild/RPMS/mips/<package>*.rpm
```

---

## Key Discoveries This Session

### IRIX Feature Macro Chain
IRIX uses a complex macro chain in `<standards.h>`:
- `_NO_POSIX` = true only if neither `_POSIX_SOURCE` nor `_POSIX_C_SOURCE` defined
- `_SGIAPI` = `(_SGI_SOURCE && _NO_POSIX && _NO_XOPEN4 && _NO_XOPEN5) || ...`
- `_XOPEN5` = `(_XOPEN_SOURCE >= 500) || ...`
- `_NO_ANSIMODE` = `__EXTENSIONS__ || ...`

Functions like snprintf/vsnprintf require:
```c
#if defined(__c99) || ((_XOPEN5 || _SGIAPI) && _NO_ANSIMODE)
```

**Key insight**: Many packages define POSIX macros for compatibility, breaking `_SGIAPI`. Use `_XOPEN_SOURCE=600` to get `_XOPEN5` instead.

### CMake Packages Need Macro Expansion
Fedora uses `%cmake`, `%cmake_build`, `%cmake_install` macros that aren't available in our build environment. Must replace with explicit cmake commands in spec_replacements.

### CMake Doesn't Handle IRIX Library Versioning
CMake creates unversioned .so files for unknown platforms. Need manual symlink creation for proper versioned libraries.

### Libtool Refuses Shared Libraries for IRIX
Libtool detects IRIX as unknown and sets `build_libtool_libs=no`. Need post-configure sed commands:
```bash
sed -i 's/build_libtool_libs=no/build_libtool_libs=yes/g' libtool
sed -i 's/deplibs_check_method="unknown"/deplibs_check_method="pass_all"/g' libtool
sed -i 's/^version_type=none$/version_type=linux/g' libtool
sed -i 's/^soname_spec=""$/soname_spec="\\$libname\\${shared_ext}\\$major"/g' libtool
sed -i 's/^library_names_spec=""$/library_names_spec="\\$libname\\${shared_ext}\\$versuffix \\$libname\\${shared_ext}\\$major \\$libname\\${shared_ext}"/g' libtool
```

### New Compat Function: stpcpy
Added `stpcpy()` to libcompat.a - copies string and returns pointer to end.
- Implementation: `compat/runtime/stpcpy.c`
- Declaration: `compat/include/mogrix-compat/generic/string.h`
- Link with: `-lcompat` (added to rpmmacros.irix LDFLAGS)

### Clang Builtins Fill Gaps
IRIX libc is missing some C99 functions (llabs), but Clang provides them as builtins. Just need the declaration; Clang handles the implementation.

### New Compat Functions Added
| Function | Header | Purpose |
|----------|--------|---------|
| `llabs()` | stdlib.h | long long absolute value (C99) - extern decl for Clang builtin |
| `UINT64_C()` etc | stdint.h | Integer constant macros (C99) |
| `stpcpy()` | string.h | Copy string returning pointer to end |
| `posix_spawn()` | spawn.h | Spawn process (POSIX.1-2008) - fork+exec implementation |
| `posix_spawnp()` | spawn.h | Spawn process with PATH search |
| `posix_spawn_file_actions_init()` | spawn.h | Initialize file actions for spawn |
| `posix_spawn_file_actions_destroy()` | spawn.h | Destroy file actions |
| `posix_spawn_file_actions_addclose()` | spawn.h | Add close action for spawn |
| `posix_spawn_file_actions_adddup2()` | spawn.h | Add dup2 action for spawn |
| `openat()` | fcntl.h | Open file relative to directory fd |
| `fstatat()` | sys/stat.h | Stat file relative to directory fd |
| `mkdirat()` | sys/stat.h | Create directory relative to directory fd |
| `fchmodat()` | sys/stat.h | Chmod relative to directory fd |
| `utimensat()` | sys/stat.h | Set timestamps relative to directory fd |
| `futimens()` | sys/stat.h | Set timestamps via fd |
| `faccessat()` | unistd.h | Access check relative to directory fd |
| `fchownat()` | unistd.h | Chown relative to directory fd |
| `unlinkat()` | unistd.h | Unlink relative to directory fd |
| `renameat()` | unistd.h | Rename relative to directory fds |
| `symlinkat()` | unistd.h | Symlink relative to directory fd |
| `readlinkat()` | unistd.h | Readlink relative to directory fd |
| `linkat()` | unistd.h | Hard link relative to directory fds |

### New Compat Constants Added
| Constant | Header | Purpose |
|----------|--------|---------|
| `O_NOFOLLOW` | fcntl.h | Don't follow symlinks (defined as 0 - no-op) |
| `O_CLOEXEC` | fcntl.h | Close-on-exec (defined as 0 - use fcntl instead) |
| `AT_FDCWD` | fcntl.h | Current working directory for *at() functions |
| `AT_SYMLINK_NOFOLLOW` | fcntl.h | Don't follow symlinks in *at() |
| `AT_REMOVEDIR` | fcntl.h | Remove directory instead of file |
| `AT_SYMLINK_FOLLOW` | fcntl.h | Follow symlinks in linkat() |
| `AT_EACCESS` | fcntl.h | Use effective IDs in faccessat() |
| `_SC_NPROCESSORS_ONLN` | unistd.h | Maps to IRIX's _SC_NPROC_ONLN |
| `_SC_PHYS_PAGES` | unistd.h | Physical memory pages (stub) |

### irix-cc Wrapper Must Handle Clang Diagnostic Options
Libtool and configure scripts call the compiler with diagnostic flags like `-print-search-dirs`, `-print-prog-name`, etc. The irix-cc wrapper was incorrectly passing these to the linker (because no .c files = link mode). Fixed by adding special case handling at the start of irix-cc:
```bash
case "$1" in
    -print-search-dirs|-print-prog-name*|-print-file-name*|-print-libgcc-file-name)
        exec $CLANG $CLANG_FLAGS "$@"
        ;;
    --version|-v|-dumpversion|-dumpmachine|-print-multiarch)
        exec $CLANG $CLANG_FLAGS "$@"
        ;;
esac
```

### irix-ld Wrapper Must Handle GNU ld Options
Similarly, the irix-ld wrapper must pass options like `-print-search-dirs` to GNU ld.bfd instead of LLD:
```bash
case "$1" in
    -print-search-dirs|-print-prog-name*|-v|--version)
        exec $GNULD "$@"
        ;;
esac
```

### Clang __mips64 Macro Breaks OpenSSL Multiarch Headers
Clang defines `__mips64=1` even for N32 ABI. OpenSSL's multiarch `configuration.h` checks `__mips64` before `__mips`, so it tries to include `configuration-mips64.h` instead of `configuration-mips.h`. Fixed by adding to irix-cc:
```bash
CLANG_FLAGS="$CLANG_FLAGS -U__mips64 -U__mips64__ -D__mips=1"
```

---

## Environment

| Purpose | Path |
|---------|------|
| Mogrix project | `/home/edodd/projects/github/unxmaal/mogrix/` |
| Cross toolchain | `/opt/cross/bin/` |
| Staging area | `/opt/sgug-staging/usr/sgug/` |
| IRIX sysroot | `/opt/irix-sysroot/` |
| Python venv | `.venv/bin/activate` |

---

## Previous Round Summary

The previous validation round successfully built all 12 packages, but required many fixes along the way. Those fixes have now been captured in:

- Package YAML files (`rules/packages/*.yaml`)
- Compat headers (`compat/include/mogrix-compat/generic/`)
- Compat runtime (`compat/runtime/`)
- Toolchain wrappers (`/opt/cross/bin/irix-cc`, `irix-ld`)

This fresh validation round will verify that all fixes are properly captured and builds succeed from a clean state.

---

## Validation Complete!

All 12 packages in the tdnf dependency chain have been successfully built for IRIX cross-compilation.

**Built RPMs available in:** `~/rpmbuild/RPMS/mips/`

Key packages:
- `tdnf-3.5.14-1.mips.rpm` - The target package manager
- `rpm-4.19.1.1-1.mips.rpm` - RPM package manager
- `libsolv-0.7.28-1.mips.rpm` - SAT solver for package dependencies

**Note**: tdnf is not in Fedora - it's from VMware's Photon OS. The SRPM is in `srpms/tdnf-3.5.14-1.ph5.src-converted/`.

---

## What Worked This Session

1. **All 12 packages built successfully** - mogrix rules captured all necessary fixes
2. **Direct wget downloads** work better than `mogrix fetch` for reliability
3. **Existing Photon OS SRPM** for tdnf in `srpms/` directory worked perfectly
4. **posix_spawn_file_actions_*** functions needed for `file` package - added to compat layer

---

## What Failed / Gotchas

1. **mogrix fetch sometimes puts SRPMs in wrong directory** (SOURCES instead of SRPMS)
2. **Previously-converted SRPMs may linger** - always clean up converted directories before converting
3. **"source 100 defined multiple times" error** means SRPM was already converted - delete and re-fetch fresh
4. **tdnf is NOT in Fedora** - it's from VMware Photon OS, use existing SRPM in `srpms/` directory
5. **file package needed posix_spawn file actions** - added `posix_spawn_file_actions_init/destroy/addclose/adddup2` to spawn.h/spawn.c

---

## CRITICAL: IRIX rld Compatibility Issues (2026-01-25)

Testing on IRIX revealed three critical linking issues. **ALL FIXED - bzip2 validated working!**

### Issue 1: MIPS_BASE_ADDRESS = 0 (FIXED)

**Symptom:** `DT_MIPS_BASE_ADDRESS missing or zero in <lib>.so. rld cannot continue`

**Cause:** IRIX's runtime linker (`rld`) requires shared libraries to have a non-zero `MIPS_BASE_ADDRESS` dynamic tag. LLD was setting it to 0.

**Fix:** Shared libraries now use GNU ld which sets proper base address via linker scripts.

### Issue 2: rpm libraries missing SONAME (FIXED)

**Symptom:** `Cannot Successfully map soname '../lib/librpm.so'`

**Cause:** cmake builds didn't pass `-soname` to the linker for unknown platforms.

**Fix:** Updated `irix-ld` wrapper to auto-generate `-soname` for `lib*.so*` outputs when not provided.

### Issue 3: Wrong linker for executables vs libraries (FIXED)

**Symptom:** Segmentation fault on any binary execution

**Cause:** Using GNU ld for executables produces binaries that segfault. Using LLD for shared libraries produces libraries missing IRIX ELF structure.

**Solution - CORRECT LINKER SELECTION:**

| Build Type | Linker | Why |
|------------|--------|-----|
| Shared libraries (`-shared`) | **GNU ld** | Produces correct IRIX ELF structure |
| Executables | **LLD** | With `--dynamic-linker=/lib32/rld` |

### Current irix-ld wrapper configuration (WORKING):

```bash
# Shared libraries: GNU ld
$GNULD --allow-shlib-undefined $auto_soname -L... $filtered_args

# Executables: LLD
$LLD --allow-shlib-undefined -dynamic-linker /lib32/rld $CRT1 $filtered_args -L... -lsoft_float_stubs -lc $CRTN
```

### Validation: bzip2 runs on IRIX!

```
$ /tmp/bzip2 --version
bzip2, a block-sorting file compressor.  Version 1.0.8, 13-Jul-2019.
```

### Required Action: REBUILD ALL PACKAGES

All 12 packages must be rebuilt with the fixed linker:

```bash
./cleanup.sh
# Rebuild each: zlib-ng bzip2 popt openssl libxml2 curl xz lua file rpm libsolv tdnf
./scripts/bootstrap-tarball.sh
```

---

## IRIX Bootstrap Process

The 12 packages are built but cannot be installed on IRIX without bootstrapping (rpm requires rpm to install). This is the bootstrap procedure:

### Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                    LINUX BUILD HOST                              │
│                                                                  │
│  .mips.rpm files ──► rpm2cpio ──► bootstrap tarball ──► scp ──► │
└─────────────────────────────────────────────────────────────────┘
                                                          │
                                                          ▼
┌─────────────────────────────────────────────────────────────────┐
│                      IRIX TARGET                                 │
│                                                                  │
│  tar xf ──► test in chroot ──► real install ──► rpm --initdb   │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

### Step 1: Create Bootstrap Tarball (Linux)

```bash
# On Linux build host:
./scripts/bootstrap-tarball.sh

# Creates (in project tmp/ directory):
#   tmp/irix-bootstrap.tar     (uncompressed)
#   tmp/irix-bootstrap.tar.gz  (compressed)
```

### Step 2: Copy Files to IRIX

```bash
# Copy the tarball
scp tmp/irix-bootstrap.tar.gz edodd@192.168.0.81:/tmp/

# Copy the IRIX bootstrap scripts
scp scripts/irix-chroot-testing.sh scripts/irix-bootstrap.sh edodd@192.168.0.81:/tmp/

# Copy RPM files (for proper database registration later)
ssh edodd@192.168.0.81 'mkdir -p /tmp/rpms'
scp ~/rpmbuild/RPMS/mips/*.mips.rpm edodd@192.168.0.81:/tmp/rpms/
```

### Step 3: Test in Chroot (Manual)

The chroot testing script extracts the tarball into an existing chroot for manual testing.
It does NOT auto-delete or auto-test - you control the testing process.

```bash
# On IRIX as root:
cd /tmp
chmod +x irix-chroot-testing.sh irix-bootstrap.sh

# Ensure your chroot exists (script will NOT create or delete it)
# If you need to restore your chroot from backup first, do that now.

# Extract bootstrap into chroot
./irix-chroot-testing.sh /tmp/irix-bootstrap.tar.gz /opt/chroot

# Enter chroot and test manually
chroot /opt/chroot /bin/sh
. /usr/sgug/bin/chroot-shell.sh    # Set up environment
rpm --version                        # Test rpm
tdnf --version                       # Test tdnf
```

### Step 4: Real Installation (DANGEROUS)

```bash
# On IRIX as root:
cd /tmp

# This modifies the real filesystem!
./irix-bootstrap.sh /tmp/irix-bootstrap.tar.gz /tmp/rpms

# Confirm with "yes" when prompted
```

### What the Bootstrap Does

1. **Extracts tarball** to `/usr/sgug/` - puts binaries and libraries in place
2. **Verifies rpm works** - tests the binary can run with shared libraries
3. **Initializes RPM database** - creates `/usr/sgug/lib32/sysimage/rpm/`
4. **Creates symlink** - `/var/lib/rpm` → `/usr/sgug/lib32/sysimage/rpm`
5. **Installs RPMs** - registers packages in database for proper tracking

### Post-Bootstrap

```bash
# Add to shell startup (.profile or .cshrc):

# For sh/bash:
export LD_LIBRARY_PATH=/usr/sgug/lib32:$LD_LIBRARY_PATH
export PATH=/usr/sgug/bin:$PATH

# For csh/tcsh:
setenv LD_LIBRARY_PATH /usr/sgug/lib32:$LD_LIBRARY_PATH
setenv PATH /usr/sgug/bin:$PATH

# Verify:
rpm --version
tdnf --version
rpm -qa  # List installed packages
```

### Scripts Reference

| Script | Runs On | Purpose |
|--------|---------|---------|
| `scripts/bootstrap-tarball.sh` | Linux | Creates bootstrap tarball from RPMs |
| `irix-chroot-testing.sh` | IRIX | **SAFE** - Tests in /opt/chroot (copy to /tmp first) |
| `irix-bootstrap.sh` | IRIX | **DANGER** - Real installation to / (copy to /tmp first) |

### Troubleshooting

**rpm: cannot open Packages database**
- Run `rpm --initdb --dbpath /usr/sgug/lib32/sysimage/rpm`

**rpm: /lib32/rld: cannot properly exec**
- Library path not set: `export LD_LIBRARY_PATH=/usr/sgug/lib32:$LD_LIBRARY_PATH`

**rpm: symbol not found**
- Missing library - check `elfdump -Dl /usr/sgug/bin/rpm` for NEEDED entries

**Cleanup chroot test:**
```bash
rm -rf /opt/chroot
```
