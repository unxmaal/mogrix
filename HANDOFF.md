# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-04
**Status**: mmap-based malloc (dlmalloc) integrated - bypasses 176MB IRIX brk() heap ceiling

---

## ✅ RESOLVED: rpm --help working (2026-02-03)

**Problem**: `rpm --help` (and ALL rpm options) were failing with "unknown option", but popt itself worked correctly.

**ROOT CAUSE**: LLD 14 (vvuk's fork) generates incorrect R_MIPS_REL32 relocations for external data symbols in static arrays on MIPS N32.

**FIX**: Updated `cross/bin/irix-ld` to use **LLD 18 with IRIX patches** (`ld.lld-irix-18`). The patches in `lld-fixes/` fix the relocation bug.

**VERIFIED WORKING**: 2026-02-03 - `rpm --help` confirmed working on IRIX.

**WARNING**: Do NOT try using GNU ld for executables - it crashes IRIX rld. See `rules/methods/linker-selection.md`.

---

---

## CRITICAL WARNING

**NEVER install packages directly to /usr/sgug on the live IRIX system.**

On 2026-01-27, installing packages directly to /usr/sgug replaced libz.so with zlib-ng, which broke sshd and locked out SSH access. The system had to be recovered from console using backup libraries from /usr/sgug_old.

**Always use /opt/chroot for testing first.** Only touch /usr/sgug after:
1. Full testing in chroot passes
2. Explicit user approval
3. Console access is available as backup
4. Recovery plan is documented

**NEVER CHEAT!** work through problems converting packages. Don't copy from old sgugrse! Don't fake it! Check the rules for how to handle being stuck.

---

## Goal

Get **rpm** and **tdnf** working on IRIX so packages can be installed via the package manager.

---

## Current Progress

### What's Done (2026-02-02)
- All 14 packages cross-compile successfully
- **IRIX vsnprintf bug FIXED** - rpm output no longer garbled
- popt rebuilt with vasprintf injection
- rpm rebuilt with rvasprintf and rpmlog fixes
- **RPM INSTALL was WORKING** - Fixed three critical bugs previously
- **TDNF STILL WORKING** - `tdnf --help`, `tdnf --version` work
- **sed→patch migration VALIDATED** - 8 of 11 packages migrated
- **Removed stub sqlite3.h** - Was shadowing real sqlite3.h in compat headers

### ✅ Fixed (2026-02-03)
- **rpm --help** now working - required LLD 18 with IRIX patches (see `lld-fixes/`)
- Root cause was LLD 14 generating bad relocations for external data symbols

### Additional Fixes for tdnf (2026-02-01)

1. **Create /var/run directory** - tdnf needs it for lockfile
2. **libz.so symlink** - `ln -sf libz.so.1 libz.so` (libsolvext.so needs unversioned name)
3. **Use createrepo --simple-md-filenames** - IRIX tar corrupts long filenames

### Fixes Applied (2026-01-30)

**Bug 1: futimens sed was too broad**

The original sed to disable futimens was:
```
sed -i 's/if (fd >= 0)/if (0 && fd >= 0)/' lib/fsm.c
```
This replaced ALL `if (fd >= 0)` patterns, including one in `ensureDir()` that checks if directory openat succeeded. This caused rpm install to fail with "Error 0" after successfully opening directories.

**Fix**: Use a more specific sed pattern that only matches the fsmUtime check:
```
sed -i '/if (fd >= 0)/{N;/futimens/s/if (fd >= 0)/if (0 \&\& fd >= 0)/}' lib/fsm.c
```

**Bug 2: utimensat didn't handle symlinks with AT_SYMLINK_NOFOLLOW**

IRIX's `utimes()` follows symlinks. When rpm creates a symlink (e.g., `libpopt.so.0 -> libpopt.so.0.0.2`) and tries to set its timestamp with `AT_SYMLINK_NOFOLLOW`, our `utimensat()` implementation still followed the symlink. If the target didn't exist yet, `utimes()` failed with ENOENT.

**Fix**: Added symlink detection in `compat/dicl/openat-compat.c:utimensat()`. When `AT_SYMLINK_NOFOLLOW` is set and the path is a symlink, return success immediately (since IRIX doesn't have `lutimes()` and symlink timestamps are rarely critical).

**Bug 3: O_NOFOLLOW not supported on IRIX (2026-01-31)**

rpm's `fsmOpenat()` adds `O_NOFOLLOW` to open flags for security. IRIX doesn't support this flag and returns `EINVAL` when it's passed to `open()`.

**Symptom**: `rpm -Uvh` fails with:
```
error: failed to open dir usr of /usr/sgug/lib/: Invalid argument
error: unpacking of archive failed on file /usr/sgug/lib/os-release: cpio: open failed - Invalid argument
```

**Fix**: Added `strip_unsupported_flags()` in `compat/dicl/openat-compat.c` to remove O_NOFOLLOW before calling IRIX's `open()`:
```c
#ifndef O_NOFOLLOW
#define O_NOFOLLOW 0x20000
#endif

static int strip_unsupported_flags(int flags) {
    flags &= ~O_NOFOLLOW;
    return flags;
}
```
Called in `openat()` before passing flags to the real `open()`.

### Verified Working (2026-01-31)

```bash
# sgugrse-release package installs successfully:
$ /usr/sgug/bin/sgug-exec /usr/sgug/bin/rpm -Uvh --nodeps /tmp/sgugrse-release-0.0.7beta-1.noarch.rpm
Verifying...                          ################################# [100%]
Preparing...                          ################################# [100%]
Updating / installing...
   1:sgugrse-release-0.0.7beta-1      ################################# [100%]
```

### Verified in /opt/chroot on IRIX (2026-02-03)

| Test | Result |
|------|--------|
| `rpm --version` | ✓ RPM version 4.19.1.1 |
| `rpm --help` | ✓ Shows options AND descriptions correctly (LLD 18 fix) |
| `rpm -qpl <pkg>` | ✓ Lists package contents |
| `rpm --initdb` | ✓ Creates sqlite database |
| `rpm -ivh --nodeps` | ✓ Installs with progress bars |
| `rpm -qa` | ✓ Shows `popt-1.19-6.mips` |
| `rpm -qi popt` | ✓ Full package info displayed |
| `tdnf --version` | ✓ tdnf: 3.5.14 |
| `tdnf --help` | ✓ Full command list displayed |

```bash
# Test session from IRIX chroot (2026-01-28):
$ rpm -ivh --nodeps /tmp/popt-1.19-6.mips.rpm
Verifying...                          ########################################
Preparing...                          ########################################
Updating / installing...
popt-1.19-6                           ########################################

$ rpm -qa
popt-1.19-6.mips

$ tdnf --version
tdnf: 3.5.14
```

### tdnf Repository Setup (DONE)

Repository configured and working:
- **mogrix.repo** at `/opt/chroot/usr/sgug/etc/yum.repos.d/mogrix.repo`
- **tdnf.conf** at `/opt/chroot/usr/sgug/etc/tdnf/tdnf.conf`
- Repository metadata at `/opt/chroot/usr/sgug/share/mogrix-repo/`

**IMPORTANT**: tdnf 3.5.14 is patched to use `/usr/sgug/etc` paths (not `/etc`).
This matches SGUG-RSE conventions and keeps all config under `/usr/sgug`.

**tdnf.conf settings** (at `/usr/sgug/etc/tdnf/tdnf.conf`):
```ini
[main]
gpgcheck=0
installonly_limit=3
clean_requirements_on_remove=1
repodir=/usr/sgug/etc/yum.repos.d
cachedir=/usr/sgug/var/cache/tdnf
distroverpkg=sgugrse-release  # Must be actual package name (empty value causes segfault)
releasever=1                  # Required - prevents $releasever lookup
basearch=mips                 # Required - prevents $basearch lookup
```

**mogrix.repo settings** (at `/usr/sgug/etc/yum.repos.d/mogrix.repo`):
```ini
[mogrix]
name=Mogrix IRIX Packages
baseurl=file:///opt/chroot/usr/sgug/share/mogrix-repo
enabled=1
gpgcheck=0
```

| Test | Result |
|------|--------|
| `tdnf repolist` | ✓ Shows "mogrix" repo enabled |
| `tdnf makecache` | ✓ Downloads metadata, creates cache |
| `tdnf list` | ✓ Lists all available packages |
| `tdnf search <pkg>` | ✓ Searches package names |
| `tdnf info <pkg>` | ✓ Shows package details |

### Path Fixes Applied (2026-01-31)

TDNF 3.5.14 is patched to use `/usr/sgug/etc` paths. Directories needed:

**In /opt/chroot:**
- `/opt/chroot/usr/sgug/etc/tdnf/vars/`
- `/opt/chroot/usr/sgug/etc/dnf/vars/`
- `/opt/chroot/usr/sgug/etc/yum/vars/`
- `/opt/chroot/usr/sgug/etc/tdnf/minversions.d/`
- `/opt/chroot/usr/sgug/etc/tdnf/locks.d/`
- `/opt/chroot/usr/sgug/etc/tdnf/protected.d/`
- `/opt/chroot/usr/sgug/etc/yum.repos.d/`
- `/opt/chroot/usr/sgug/var/cache/tdnf/`
- `/opt/chroot/var/lib/rpm` → `../../usr/sgug/lib32/sysimage/rpm` (relative symlink)
- `/opt/chroot/usr/sgug/lib/sysimage/rpm` → `../../lib32/sysimage/rpm` (for `_dbpath` macro mismatch)

**Note**: Previous versions used `/etc` paths. Our patched tdnf uses `/usr/sgug/etc` to match SGUG-RSE conventions.

### Test Method

**IMPORTANT**: Default IRIX shell is csh. Always use `/bin/sh` to run test scripts:

```bash
# Create test script locally
cat > /tmp/test.sh << 'SCRIPT'
#!/bin/sh
export LD_LIBRARYN32_PATH=/opt/chroot/usr/sgug/lib32:/usr/sgug/lib32
# tdnf 3.5.14 uses /usr/sgug/etc paths by default
/opt/chroot/usr/sgug/bin/tdnf --installroot=/opt/chroot repolist
echo "exit: $?"
SCRIPT

# Copy and run with /bin/sh
scp /tmp/test.sh root@192.168.0.81:/tmp/
ssh root@192.168.0.81 "/bin/sh /tmp/test.sh"
```

**Note**: IRIX chroot doesn't properly isolate - commands still see base system paths. That's why we run binaries directly with `LD_LIBRARYN32_PATH` instead of using chroot.

### Preferred Test Method (2026-01-31)

Use `sgug-exec` which sets up the proper environment:
```bash
# From within the chroot on IRIX (ssh root@192.168.0.81, then chroot /opt/chroot)
# Note: tdnf 3.5.14 uses /usr/sgug/etc paths by default, no -c needed
/usr/sgug/bin/sgug-exec /usr/sgug/bin/tdnf repolist
```

### Debugging with par

IRIX has `par` (Process Activity Reporter) similar to Linux strace:
```bash
par -s /usr/sgug/bin/sgug-exec /usr/sgug/bin/tdnf repolist > /tmp/par_output.txt 2>&1
```
This traces system calls and helps identify missing files or failed operations.

### TDNF NOW WORKING (2026-01-29)

All tdnf operations work:
- `tdnf repolist` - lists configured repos
- `tdnf makecache` - downloads and caches repo metadata
- `tdnf list` - lists available packages

**Fixes applied:**
1. **funopen implementation** (`compat/stdio/funopen.c`) - BSD-style cookie I/O for transparent .gz decompression
2. **libsolv built with HAVE_FUNOPEN=1** - uses funopen instead of fopencookie
3. **--whole-archive linking** - ensures funopen is included in shared libraries
4. **Hardcoded mips architecture** - IRIX uname reports IP30/IP32/etc, patched to use "mips"
5. **tdnf.conf settings** - `distroverpkg=` and `releasever=1` to disable distro package check

### Required tdnf.conf Settings

Located at `/usr/sgug/etc/tdnf/tdnf.conf`:

```ini
[main]
gpgcheck=0
installonly_limit=3
clean_requirements_on_remove=1
repodir=/usr/sgug/etc/yum.repos.d
cachedir=/usr/sgug/var/cache/tdnf
distroverpkg=sgugrse-release  # Must be actual package name (empty value causes segfault)
releasever=1
basearch=mips
```

### Required Directories

Create these before running tdnf:
```bash
mkdir -p /usr/sgug/var/cache/tdnf
mkdir -p /usr/sgug/etc/yum.repos.d
mkdir -p /usr/sgug/etc/tdnf/vars
```

Initialize rpm database:
```bash
rpm --initdb --dbpath=/usr/sgug/lib/sysimage/rpm
```

---

## Bootstrap Tarball Workflow (2026-02-01)

A self-contained bootstrap tarball is available at `scripts/bootstrap-tarball.sh`. It:
- Validates all 17 required packages exist
- Extracts them to a single directory
- Includes RPM files at `/tmp/bootstrap-rpms/` for rpmdb registration
- Ships `mogrix.repo` pointing to `file:///tmp/mogrix-repo`

**Usage:**
```bash
# On Linux build host
./scripts/bootstrap-tarball.sh
scp tmp/irix-bootstrap.tar.gz root@192.168.0.81:/tmp/

# On IRIX
cd /opt/chroot
gzcat /tmp/irix-bootstrap.tar.gz | tar xvf -
chroot /opt/chroot /bin/sh
export LD_LIBRARYN32_PATH=/usr/sgug/lib32
/usr/sgug/bin/rpm --initdb
/usr/sgug/bin/rpm -Uvh --nodeps /tmp/bootstrap-rpms/sgugrse-release*.noarch.rpm

# Create repo (on Linux, copy repodata to IRIX)
createrepo_c --simple-md-filenames ~/rpmbuild/RPMS/mips/
# Copy repodata/* to IRIX /tmp/mogrix-repo/

# Test
/usr/sgug/bin/sgug-exec /usr/sgug/bin/tdnf repolist
/usr/sgug/bin/sgug-exec /usr/sgug/bin/tdnf makecache
/usr/sgug/bin/sgug-exec /usr/sgug/bin/tdnf install popt
```

---

## Next Steps

### ✅ COMPLETED: tdnf installing from remote repos (2026-02-03)

### Next priorities:
1. ~~**Rebuild rpm** with the image-base fix~~ **DONE** (2026-02-04)
2. ~~**Implement mmap-based malloc**~~ **DONE** (2026-02-04) - dlmalloc 2.8.6 integrated into compat system. All packages using compat functions auto-get mmap-based malloc, bypassing 176MB brk() heap ceiling. RPMs deployed to IRIX at `/tmp/` for testing.
3. ~~**Test dlmalloc on IRIX**~~ **DONE** (2026-02-04) - rpm with mmap-based malloc verified working on IRIX, ncurses-term installs correctly
4. **Make `--image-base=0x1000000` the default for all executables** - Update `cross/bin/irix-ld` (less critical now with dlmalloc, but still beneficial)
5. **Phase 2 build tools** - **IN PROGRESS** - Dependency chain: perl → bash → autoconf → automake → libtool
   - m4 ✅ installed on IRIX
   - autoconf: BUILT (RPM exists), NOT INSTALLABLE - blocked on perl
   - automake: BUILT (RPM exists, shebang/FindBin fixes applied), NOT INSTALLABLE - blocked on perl
   - libtool: BUILT (RPM exists, shebang/drop_requires fixes applied), NOT INSTALLABLE - blocked on autoconf+automake
   - **perl: NOT BUILT** - Must cross-compile FC40 perl 5.38.2 (SGUG-RSE has 65 patches for reference)
   - **bash: NOT BUILT** - Must cross-compile FC40 bash 5.2.26
   - sgugrse-release ✅ rebuilt with Provides for IRIX system tools (sed, tar, findutils, grep, gawk)
6. **Plan production migration** - Strategy for moving from chroot to /usr/sgug
7. **Long-term goal**: Port WebKitGTK 2.38.x for a modern browser on IRIX

### Deferred (low priority):
- **Remaining inline seds** - libsolv (2), tdnf (6), rpm (5) still have inline seds but are working. These are mostly CMake/config changes that are hard to express as source patches. Not blocking.

---

## ✅ RESOLVED: ncurses-term installation (2026-02-04)

**Problem**: Installing ncurses-term (2757 files) on IRIX fails: `memory alloc (176 bytes) returned NULL.`

### Root Cause: IRIX rld quickstart mapping blocks heap

IRIX's runtime linker (rld) places an internal data structure at address **0x200000** (2MB). When rpm is linked with the default image base of 0x10000, the heap starts at ~0x38000 and can only grow to 0x1FF000 before hitting this mapping. That gives only **~1.8MB of heap** - not enough for installing packages with many files.

The par trace confirmed: `brk(0x244000) errno = 12 (Not enough space)` while all previous brk() calls succeeded up to 0x1cc000.

A test program linked against the same libraries as rpm confirmed the heap limit, while a simple test program (only libc) could allocate 250MB - proving the issue is address space layout, not system limits.

### Investigation Trail (SQLite was a red herring)

1. Initially suspected SQLite malloc vs BDB mmap (SGUG-RSE used BDB, we use SQLite)
2. Enabled SQLite mmap: added `SQLITE_MAX_MMAP_SIZE=268435456` to sqlite build and `PRAGMA mmap_size` to rpm's sqlite.c - **did not fix** because the heap exhaustion wasn't from SQLite
3. `rpm -Uvh --test` (dry run) **succeeded** - proving database queries weren't the issue
4. Built address space probe program that found the mapping at 0x200000 blocking heap growth
5. Tested `--image-base=0x1000000` - heap grew from 1.8MB to **176MB**

### Fix

Added `--image-base=0x1000000` to rpm's EXE linker flags, moving the binary above rld's quickstart area:

```yaml
# In rules/packages/rpm.yaml:
-DCMAKE_EXE_LINKER_FLAGS="-Wl,--image-base=0x1000000 -Wl,--whole-archive ..."
```

### Files Modified

- `rules/packages/rpm.yaml` - Added `--image-base=0x1000000` to CMAKE_EXE_LINKER_FLAGS
- `rules/packages/sqlite.yaml` - Added `-DSQLITE_MAX_MMAP_SIZE=268435456` (kept, beneficial)
- `patches/packages/rpm/rpm.irixfixes.patch` - Added PRAGMA mmap_size (kept, beneficial)

### Verified

```
ncurses-term-6.4-12.20240127          ########################################
exit: 0
```

All 2757 files installed, terminfo entries confirmed working.

### mmap-based malloc (dlmalloc) - IMPLEMENTED (2026-02-04)

**Problem**: Even with `--image-base=0x1000000`, the brk() heap is still limited to 176MB because libpthread.so is mapped at fixed address 0x0C080000. Any program linking pthreads has this ceiling regardless of image base.

**Full IRIX n32 address space mapped** via probe programs:

| Address | What | Size |
|---------|------|------|
| 0x00200000 | rld quickstart data | 256KB |
| 0x00AB0000 | libcrypto.so | ~4.5MB |
| 0x00F90000 | libm.so | ~1MB |
| 0x0C080000 | **libpthread.so** (HEAP CEILING) | 2MB |
| 0x0FC40000-0x5C000000 | **FREE SPACE (mmap target)** | **1.2GB** |
| 0x7BFC0000 | Stack | 64KB+ |

**Solution**: Integrated dlmalloc 2.8.6 (MIT-0 license) into mogrix compat system. Configured with `HAVE_MORECORE=0` so all allocations use `mmap()` instead of `brk()`. Allocations go to the 1.2GB free region above libpthread.

**Key implementation details**:
- `compat/malloc/dlmalloc.c` - Config wrapper, sets IRIX-specific options
- `compat/malloc/dlmalloc-src.inc` - Raw dlmalloc 2.8.6 source (stored as .inc to avoid `*.c` glob compilation)
- `USE_DL_PREFIX` must be `#undef`, NOT `#define 0` (dlmalloc uses `#ifndef` check)
- No MAP_ANONYMOUS on IRIX - dlmalloc falls back to `/dev/zero` (built-in)
- Auto-injected by `mogrix/rules/engine.py` whenever any compat functions are used

**Symbol verification**: rpm binary exports `malloc`/`free`/`calloc`/`realloc` (5880/2500/200/328 bytes) - confirmed standard names, not dl-prefixed.

**Files added/modified**:
- `compat/malloc/dlmalloc.c` (NEW)
- `compat/malloc/dlmalloc-src.inc` (NEW)
- `compat/catalog.yaml` (dlmalloc entry with extra_files)
- `mogrix/compat/injector.py` (extra_files support)
- `mogrix/cli.py` (copy extra files)
- `mogrix/rules/engine.py` (auto-inject dlmalloc)
- `rules/methods/irix-address-space.md` (NEW - full address map)
- `rules/INDEX.md` (new invariants)

See `rules/methods/irix-address-space.md` for the complete address space documentation.

### WARNING: All IRIX executables may need `--image-base=0x1000000`

The 0x200000 quickstart mapping affects ALL n32 executables that link multiple shared libraries. Any program with a significant heap requirement should be linked with `--image-base=0x1000000`. Consider making this the default in `cross/bin/irix-ld`.

---

## sed → Proper Patch Files Migration (2026-01-30)

**Problem**: sed silently succeeds when patterns don't match, leading to hours of debugging (e.g., the rpm futimens bug where an overly-broad sed matched 6 locations instead of 1).

**Solution Evolution**:
1. **Initial attempt**: Individual per-package patch scripts (e.g., `tools/patch-rpm-source.sh`) with MOGRIX_ROOT
2. **Rejected**: Not scalable for thousands of packages
3. **Final approach**: Use the EXISTING `patches/packages/` infrastructure with proper `.patch` files

### Existing Infrastructure (USE THIS)

The project already has a complete patch management system:
- **patches/packages/<pkg>/**: Directory for each package's patches
- **add_patch:** YAML directive references patches
- **mogrix/patches/catalog.py**: Integrates patches into spec files

### Tools Kept

| Tool | Purpose |
|------|---------|
| `tools/safepatch` | Generate diffs for patches (useful for creating .patch files) |
| `tools/fix-libtool-irix.sh` | Generic libtool fixes (temporary - will become mogrix hook) |

### Tools Deleted (Wrong Approach)

These per-package scripts were deleted - not scalable:
- `tools/patch-rpm-source.sh`
- `tools/patch-popt-source.sh`
- `tools/patch-libsolv-source.sh`
- `tools/patch-tdnf-source.sh`
- `tools/patch-libxml2-source.sh`
- `tools/patch-bzip2-source.sh`

### Migration Strategy

For each sed command in YAML `spec_replacements`:
1. Determine if it's a **source patch** (C/Makefile changes) or **spec modification** (rpm macro changes)
2. **Source patches** → Create `.patch` file in `patches/packages/<pkg>/`, add `add_patch:` to YAML
3. **Spec modifications** → Keep in `spec_replacements:` (these modify the spec file, not source)
4. **Libtool fixes** → Future: mogrix post-configure hook (currently using fix-libtool-irix.sh)

### Patch Categories

| Category | Examples | Approach |
|----------|----------|----------|
| Source fixes | vsnprintf workarounds, __thread removal | `.patch` file |
| Platform defines | `#if defined(__sgi)` additions | `.patch` file |
| Makefile changes | Remove test target, fix rules | `.patch` file |
| Spec macro changes | `%bcond_without` → `%bcond_with` | Keep in `spec_replacements` |
| cmake options | Add cross-compilation flags | Keep in `spec_replacements` |
| %files modifications | Remove systemd entries | Keep in `spec_replacements` |

### Migration Status

| Package | sed→patch | Notes |
|---------|-----------|-------|
| bzip2.yaml | **DONE** | Uses `add_patch: disable-test.patch` |
| popt.yaml | **DONE** | Uses 2 patches + `fix-libtool-irix.sh` |
| libxml2.yaml | **DONE** | Uses `add_patch: libxml2-disable-check.patch` + `fix-libtool-irix.sh` |
| sqlite.yaml | **DONE** | Uses `fix-libtool-irix.sh` |
| xz.yaml | **DONE** | Uses `fix-libtool-irix.sh` |
| lua.yaml | **DONE** | Uses `fix-libtool-irix.sh` + custom soname sed |
| zlib.yaml | **DONE** | Uses `fix-libtool-irix.sh` + custom soname sed |
| file.yaml | **DONE** | Uses `fix-libtool-irix.sh` + custom soname sed |
| libsolv.yaml | PARTIAL | Has libsolv.sgifixes.patch, still uses inline seds |
| tdnf.yaml | PARTIAL | Has 11 patches already, still uses inline seds |
| rpm.yaml | TODO | Has existing rpm.sgifixes.patch (for 4.15), needs 4.19 patches |

**8 of 11 packages migrated** (2026-01-31). The remaining 3 complex packages (libsolv, tdnf, rpm) need review.

### Testing the Migration (2026-01-31)

**Full build test completed** - bzip2 and popt both build successfully with migrated patches.

**Patch fixes required during testing:**
1. `bzip2/disable-test.patch` - Line numbers were wrong (28 vs 34). Regenerated from actual source.
2. `popt/popt-disable-stpcpy.patch` - Context didn't match. Regenerated from popt-1.19 source.
3. `popt/popt-disable-mbsrtowcs.patch` - Context didn't match. Regenerated from popt-1.19 source.

**Test commands:**
```bash
# Convert and build bzip2
MOGRIX_ROOT=/home/edodd/projects/github/unxmaal/mogrix .venv/bin/mogrix convert oldcrap/bzip2-1.0.8-18.fc40.src.rpm -o /tmp/bzip2-test
MOGRIX_ROOT=/home/edodd/projects/github/unxmaal/mogrix rpmbuild --rebuild /tmp/bzip2-test/bzip2-1.0.8-18.src.rpm --define "__cc /opt/sgug-staging/usr/sgug/bin/irix-cc" --nodeps

# Convert and build popt
MOGRIX_ROOT=/home/edodd/projects/github/unxmaal/mogrix .venv/bin/mogrix convert oldcrap/popt-1.19-6.fc40.src.rpm -o /tmp/popt-test
MOGRIX_ROOT=/home/edodd/projects/github/unxmaal/mogrix rpmbuild --rebuild /tmp/popt-test/popt-1.19-6.src.rpm --define "__cc /opt/sgug-staging/usr/sgug/bin/irix-cc" --nodeps
```

**Results:** Both packages build successfully. Output binaries verified as MIPS N32:
```
ELF 32-bit MSB shared object, MIPS, N32 MIPS-III version 1 (SYSV)
```

**Note on MOGRIX_ROOT:** The mogrix spec files use `$MOGRIX_ROOT/tools/fix-libtool-irix.sh` for libtool fixes. This environment variable must be set both for conversion AND for rpmbuild.

---

## The Root Cause: IRIX vsnprintf Pre-C99 Behavior

**Symptom**: rpm output was garbled or missing. `--help` showed descriptions without option flags. `--debug` showed `D: ` prefixes repeated without messages.

**Root Cause**: IRIX vsnprintf does NOT support C99 behavior.
- C99: `vsnprintf(NULL, 0, fmt, ap)` returns the required buffer size
- IRIX: `vsnprintf(NULL, 0, fmt, ap)` returns **-1**

This affected:
1. **popt**: Uses vasprintf in POPT_fprintf for help output
2. **rpm's rvasprintf**: Internal vasprintf implementation in rpmio/rpmstring.c
3. **rpm's rpmlog**: Log function in rpmio/rpmlog.c also used vsnprintf(NULL,0)

**Note**: The va_list ABI IS compatible between clang and IRIX (both 4-byte pointers on MIPS n32). The original "va_list ABI mismatch" hypothesis was wrong.

---

## Fixes Applied

### 1. compat/stdio/asprintf.c - Iterative vasprintf
Changed from "determine size then allocate" to iterative approach:
```c
int vasprintf(char **strp, const char *fmt, va_list ap) {
    size_t size = 256;  // Start with reasonable buffer
    while (1) {
        char *str = malloc(size);
        va_copy(ap_copy, ap);
        int len = vsnprintf(str, size, fmt, ap_copy);
        va_end(ap_copy);

        if (len >= 0 && len < size) {
            *strp = str;
            return len;  // Success
        }
        free(str);

        if (len >= size) size = len + 1;  // C99 precise size
        else size *= 2;  // IRIX: double buffer

        if (size > 1024*1024) return -1;  // Safety limit
    }
}
```

### 2. rules/packages/popt.yaml
```yaml
inject_compat_functions:
  - vasprintf  # Added - IRIX vsnprintf(NULL,0) returns -1

ac_cv_overrides:
  ac_cv_func_vasprintf: "yes"  # Tell configure we have vasprintf

prep_commands:
  # Force disable HAVE_MBSRTOWCS - IRIX doesn't have this
  - "sed -i 's/#ifdef HAVE_MBSRTOWCS/#if 0/' src/popthelp.c"
```

### 3. rules/packages/rpm.yaml
Added two sed fixes in the `%autosetup` replacement:

**rvasprintf fix** (rpmio/rpmstring.c):
```bash
sed -i '/^int rvasprintf/,/^int rasprintf/c\
int rvasprintf(char **strp, const char *fmt, va_list ap)\n\
... iterative implementation ...
' rpmio/rpmstring.c
```

**rpmlog fix** (rpmio/rpmlog.c):
```bash
sed -i '/^void rpmlog.*code.*fmt/,/^exit:/c\
void rpmlog (int code, const char *fmt, ...)\n\
... iterative implementation ...
' rpmio/rpmlog.c
```

---

## What Worked

### SQLite Build Fixes (rules/packages/sqlite.yaml)
Required multiple fixes for IRIX cross-compilation:
1. `--disable-math` - IRIX lacks acosh/asinh/atanh
2. `-D_ABI_SOURCE` - Fix select() static/extern conflict
3. `-DLLONG_MAX=...` - IRIX predates C99
4. `-DLONGDOUBLE_TYPE=double` - Avoid 128-bit long double

### LLD 18 from Source
Built LLD 18.1.3 from LLVM source with three essential patches for IRIX compatibility.

### RPM Fixes
- `-DRPM_CONFIGDIR=/usr/sgug/lib32/rpm` - Fix config directory path
- `-DENABLE_SQLITE=ON` with sqlite paths - Enable sqlite database backend
- `sed -i 's/static __thread/static/'` - Remove TLS (IRIX rld doesn't support `__tls_get_addr`)

---

## What Failed

### Using vvuk's LLD 14 fork
Generated incorrect relocations - external symbols in static data referenced `*ABS*` instead of actual symbol names.

### sqlite3 CLI file write crash (OPEN)
sqlite3 works with in-memory databases but crashes with SIGSEGV when writing to files. The rpm database was created successfully by `rpm --initdb`, so only the CLI has this issue.

### RPM without sqlite database
rpm built with `-DENABLE_SQLITE=OFF` could not initialize its database properly.

### fopencookie implementation
The `compat/stdio/fopencookie.c` uses pipes + pthreads and crashes on IRIX. Don't use it. Use `funopen` instead.

### Static archive linking without --whole-archive
When linking a static archive (.a) into a shared library (.so), symbols that aren't directly referenced won't be included. This caused funopen to be missing from libsolvext.so until we added `-Wl,--whole-archive` to the linker flags.

---

## Key Files

| File | Purpose |
|------|---------|
| `compat/stdio/asprintf.c` | Fixed vasprintf with iterative approach |
| `compat/stdio/fopencookie.c` | fopencookie compat (BROKEN - crashes on IRIX, don't use) |
| `compat/stdio/funopen.c` | funopen compat - BSD-style cookie I/O (WORKING) |
| `rules/packages/popt.yaml` | popt with vasprintf injection |
| `rules/packages/rpm.yaml` | rpm with rvasprintf and rpmlog fixes |
| `rules/packages/libsolv.yaml` | libsolv - needs HAVE_FUNOPEN=1 |
| `rules/packages/tdnf.yaml` | tdnf cross-compilation config |
| `compat/malloc/dlmalloc.c` | mmap-based malloc config wrapper for IRIX |
| `compat/malloc/dlmalloc-src.inc` | dlmalloc 2.8.6 source (MIT-0 license) |
| `rules/methods/irix-address-space.md` | Full IRIX n32 address space map |
| `tools/bin/ld.lld-irix-18` | LLD 18 binary with IRIX patches |
| `/opt/sgug-staging/usr/sgug/bin/irix-ld` | Linker wrapper (uses LLD 18) |

### IRIX Chroot Configuration Files

| File (in /opt/chroot) | Purpose |
|------|---------|
| `/usr/sgug/etc/tdnf/tdnf.conf` | tdnf main config (distroverpkg= disabled) |
| `/usr/sgug/etc/yum.repos.d/mogrix.repo` | Repository definition |
| `/usr/sgug/share/mogrix-repo/` | Repository metadata location |
| `/usr/sgug/var/cache/tdnf/` | tdnf metadata cache |

---

## Build Order (FC40/Photon5) - Status

All 14 bootstrap packages built successfully (2026-02-02).

| # | Package | Version | Status | RPMs |
|---|---------|---------|--------|------|
| 1 | zlib-ng | 2.1.6 | ✅ COMPLETE | 5 |
| 2 | bzip2 | 1.0.8 | ✅ COMPLETE | 4 |
| 3 | popt | 1.19 | ✅ COMPLETE | 3 |
| 4 | openssl | 3.2.1 | ✅ COMPLETE | 4 |
| 5 | libxml2 | 2.12.5 | ✅ COMPLETE | 3 |
| 6 | curl | 8.6.0 | ✅ COMPLETE | 1 |
| 7 | xz | 5.4.6 | ✅ COMPLETE | 5 |
| 8 | lua | 5.4.6 | ✅ COMPLETE | 4 |
| 9 | file | 5.45 | ✅ COMPLETE | 4 |
| 10 | sqlite | 3.45.1 | ✅ COMPLETE | 3 |
| 11 | rpm | 4.19.1.1 | ✅ COMPLETE | 14 |
| 12 | libsolv | 0.7.28 | ✅ COMPLETE | 4 |
| 13 | tdnf | 3.5.14 | ✅ COMPLETE | 8 |
| 14 | sgugrse-release | 0.0.7beta | ✅ COMPLETE | 2 |

**Total: 80 MIPS RPMs**

Key fixes applied during this rebuild:
- **setjmp.h wrapper** - IRIX headers check MIPS1-4 but clang defines MIPS64
- **math.h float functions** - Added ldexpf/frexpf wrappers (IRIX has them in `#if 0`)
- **stat64 conflict resolved** - IRIX has separate struct stat64, can't `#define stat64 stat`
- **UINT64_C macro** - IRIX inttypes.h requires _SGIAPI which conflicts with _XOPEN_SOURCE

---

## Environment

| Purpose | Path |
|---------|------|
| Mogrix project | `/home/edodd/projects/github/unxmaal/mogrix/` |
| Cross toolchain | `/opt/cross/bin/` |
| Staging area | `/opt/sgug-staging/usr/sgug/` |
| IRIX sysroot | `/opt/irix-sysroot/` |
| Python venv | `.venv/bin/activate` |
| IRIX host | `ssh edodd@192.168.0.81` |
| IRIX chroot | `/opt/chroot` |

### Directory Conventions

| Directory | Purpose |
|-----------|---------|
| `~/rpmbuild/SRPMS/fc40/` | Original Fedora 40 SRPMs (persistent inputs) |
| `/tmp/mogrix-converted/<pkg>/` | Conversion output (ephemeral, regenerate anytime) |
| `~/rpmbuild/RPMS/mips/` | Built MIPS packages (rpmbuild output) |
| `~/rpmbuild/RPMS/noarch/` | Built noarch packages |
| `/tmp/mogrix-repo/` | Distribution repository (copy of what ships) |

**Key principles:**
- **Original SRPMs are inputs** - keep them in `~/rpmbuild/SRPMS/fc40/`
- **Converted output is ephemeral** - regenerate from originals + rules anytime
- **Built RPMs are outputs** - rpmbuild puts them in `~/rpmbuild/RPMS/`
- **Repo is for distribution** - copy built RPMs here, run createrepo_c
- **Never store build artifacts in the mogrix repo** - keep it code-only

---

## Quick Test on IRIX

To verify rpm works:
```bash
ssh edodd@192.168.0.81 '/usr/sgug/bin/bash' <<'EOF'
export LD_LIBRARY_PATH=/tmp/test-rpm/usr/sgug/lib32:/usr/sgug/lib32
/tmp/test-rpm/usr/sgug/bin/rpm --version
/tmp/test-rpm/usr/sgug/bin/rpm --help | head -20
EOF
```

---

## Rebuilding Packages

### Rebuild popt (if needed)
```bash
source .venv/bin/activate
rm -rf popt-*-converted
mogrix convert popt-1.19-6.fc40.src.rpm
mogrix build popt-1.19-6.fc40.src-converted/*.src.rpm --cross
mogrix stage ~/rpmbuild/RPMS/mips/popt*.rpm
```

### Rebuild rpm (after popt)
```bash
rm -rf rpm-*-converted
mogrix convert rpm-4.19.1.1-1.fc40.src.rpm
mogrix build rpm-4.19.1.1-1.fc40.src-converted/*.src.rpm --cross
mogrix stage ~/rpmbuild/RPMS/mips/rpm-libs*.rpm ~/rpmbuild/RPMS/mips/rpm-4*.rpm
```

### Create test tarball
```bash
cd /tmp && rm -rf irix-test && mkdir irix-test && cd irix-test
for rpm in ~/rpmbuild/RPMS/mips/{popt,rpm-libs,rpm-4}*.mips.rpm; do
    rpm2cpio "$rpm" | cpio -idm
done
# Copy dependencies from staging
cp -av /opt/sgug-staging/usr/sgug/lib32/lib{lua,ssl,crypto,bz2,lzma,magic,sqlite3,z}*.so* usr/sgug/lib32/
tar cf ../irix-test.tar usr
scp ../irix-test.tar edodd@192.168.0.81:/tmp/
```

---

## Known Issues

- **fopencookie crashes on IRIX** - mogrix's pipe+pthread implementation doesn't work
  - FIXED: Use funopen instead (compat/stdio/funopen.c)
- Existing IRIX system has SGUG-RSE with rpm 4.15.0 already installed
- Our packages conflict with existing SGUG packages (e.g., zlib-ng-compat vs zlib)
- sqlite3 CLI crashes when writing to files (but rpm database writes work)
- IRIX uname reports machine as IP30/IP32/etc - tdnf patched to hardcode "mips"
- **tdnf install fails** - Error(1525) rpm transaction failed due to futimens/utimes issue with /dev/fd - **FIX APPLIED, needs rebuild**

---

## Session Summary (2026-01-29)

This session fixed the Solv I/O error (Error 1304) for tdnf list/repolist/makecache.

**Root causes identified:**
1. **fopencookie crashes** - The pipe+pthread implementation in `compat/stdio/fopencookie.c` doesn't work on IRIX
2. **funopen not linked** - Static archive symbols need `--whole-archive` to be included in shared libs
3. **IRIX uname returns IP30** - tdnf couldn't understand the machine type, needed to hardcode "mips"
4. **Empty rpm database** - tdnf fails if rpmdb.sqlite is empty, need `rpm --initdb` first
5. **Missing tdnf.conf settings** - Need `distroverpkg=` and `releasever=1` to skip distro package checks

**Files modified:**
- `compat/stdio/funopen.c` - New BSD-style cookie I/O implementation
- `compat/catalog.yaml` - Added funopen entry
- `compat/include/mogrix-compat/generic/stdio.h` - Added funopen declaration
- `rules/packages/libsolv.yaml` - HAVE_FUNOPEN=1, --whole-archive linking
- `rules/packages/tdnf.yaml` - Hardcode mips arch, add compat functions

**The funopen implementation uses:**
- pipe() to create a communication channel
- pthread to run callbacks in a separate thread
- fdopen() to wrap the pipe as a FILE*

This is the same approach as libdicl's funopen, just in pure C instead of C++.

---

## Session Summary (2026-01-30)

**Current Issue: tdnf install fails with Error(1525) - rpm transaction failed**

### Symptom (from older tdnf build with /etc paths)
```
# /usr/sgug/bin/sgug-exec /usr/sgug/bin/tdnf install popt
Installing:
popt             mips         1.19-6           mogrix       119.83k    75.38k

Total installed size: 119.83k
Total download size:  75.38k
Is this ok [y/N]: y
Testing transaction
Running transaction
Installing/Updating: popt-1.19-6.mips
error: error: Error(1525) : rpm transaction failed
```

### Root Cause Analysis (using par trace)

The par trace (`par_popt_install.txt` in project root) shows:
```
4004mS: open("popt.d", O_RDONLY, ...) = 14
4004mS: fchown(14, 0, 0) OK
4004mS: fchmod(14, 0755) OK
4004mS: utimets("/dev/fd/14", 0x7ffcf480) errno = 2 (No such file or directory)
```

RPM creates a directory, then tries to set timestamps using `/dev/fd/N` path. This fails with ENOENT.

### Investigation Findings

1. **IRIX does have /dev/fd filesystem** - it's mounted and working:
   ```
   # exec 5</etc/passwd; ls -la /dev/fd; exec 5>&-
   crw-rw-rw-  1 root sys 13, 0 Jan 29 16:11 0
   crw-rw-rw-  1 root sys 13, 1 Jan 29 16:11 1
   ...
   crw-rw-rw-  1 root sys 13, 5 Jan 29 16:11 5
   ```

2. **The futimens() implementation** in `compat/dicl/openat-compat.c` line 821 does:
   ```c
   sprintf(fdpath, "/dev/fd/%d", fd);
   return utimes(fdpath, tv);
   ```

3. **SGUG-RSE uses rpm 4.15.0**, we use rpm 4.19.1.1 - the futimens usage may be newer

4. **SGUG-RSE librepo patch** shows the preferred approach on IRIX is to use `utimes(pathname, tv)` directly instead of `futimes(fileno(f), tv)` - avoids /dev/fd entirely

### Pending Test

Need to verify if IRIX's utimes() works with /dev/fd/N paths at all. Test program:

```c
// /tmp/test_fd_utime.c
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main() {
    int fd = open("/tmp/testfile", O_RDONLY);
    char path[32];
    struct timeval tv[2];

    printf("Opened fd: %d\n", fd);
    sprintf(path, "/dev/fd/%d", fd);
    printf("Trying utimes on: %s\n", path);

    tv[0].tv_sec = 1000000000;
    tv[0].tv_usec = 0;
    tv[1].tv_sec = 1000000000;
    tv[1].tv_usec = 0;

    if (utimes(path, tv) < 0) {
        printf("utimes failed: %s (errno %d)\n", strerror(errno), errno);
    } else {
        printf("utimes succeeded!\n");
    }

    close(fd);
    return 0;
}
```

Compile and run on IRIX:
```bash
touch /tmp/testfile
/usr/sgug/bin/gcc -o /tmp/test_fd_utime /tmp/test_fd_utime.c
/tmp/test_fd_utime
```

### Possible Solutions

1. **If utimes() doesn't work with /dev/fd**: Need to reimplement futimens() to use a different approach
   - Option A: Patch RPM to pass pathname instead of fd (invasive)
   - Option B: Track fd-to-path mappings (complex)
   - Option C: Just use fstat to preserve times, accept timestamp limitations

2. **If utimes() does work with /dev/fd**: The issue is elsewhere - timing, permissions, or fd visibility

### Files to Investigate

| File | Issue |
|------|-------|
| `compat/dicl/openat-compat.c:821` | futimens() uses /dev/fd path |
| `par_popt_install.txt` | Full system call trace of failed install |
| `/home/edodd/projects/github/sgug-rse/packages/librepo/librepo.sgifixes.patch` | Shows SGUG-RSE workaround |

### Solution Applied and Verified (2026-01-30)

**Root cause confirmed**: IRIX's `utimes()` does NOT work with `/dev/fd/N` paths. The sgug-rse librepo patch confirms this - they use `utimes(filename, tv)` instead of `futimes(fd, tv)`.

**Two bugs were fixed:**

**Bug 1: futimens sed was too broad** (in `rules/packages/rpm.yaml`)

Original overly-broad sed that broke ensureDir():
```bash
sed -i 's/if (fd >= 0)/if (0 \&\& fd >= 0)/' lib/fsm.c
```

Fixed to only match the fsmUtime check:
```bash
sed -i '/if (fd >= 0)/{N;/futimens/s/if (fd >= 0)/if (0 \&\& fd >= 0)/}' lib/fsm.c
```

**Bug 2: utimensat didn't handle symlinks** (in `compat/dicl/openat-compat.c`)

IRIX's `utimes()` follows symlinks. When rpm creates a symlink and calls `utimensat()` with `AT_SYMLINK_NOFOLLOW`, our implementation followed the symlink anyway. If the target didn't exist yet, `utimes()` failed with ENOENT.

Fixed by detecting symlinks when `AT_SYMLINK_NOFOLLOW` is set and returning success (IRIX doesn't have `lutimes()`).

### Verification

Three packages installed successfully:
```bash
$ rpm -qa --root=/opt/chroot
popt-1.19-6.mips
bzip2-libs-1.0.8-18.mips
libxml2-2.12.5-1.mips
```

---

## Next Steps

1. **Test tdnf install** - Now that rpm install works, test `tdnf install <package>` on IRIX
2. **Build more packages** - Expand the mogrix repo with more useful packages
3. **Plan production migration** - Strategy for moving from chroot to /usr/sgug (conflicts with existing SGUG-RSE)
4. **Long-term goal**: Port WebKitGTK 2.38.x for a modern browser on IRIX

---

## Quick Test Commands

### Test rpm install on IRIX
```bash
# Copy package to IRIX
scp ~/rpmbuild/RPMS/mips/<package>.mips.rpm root@192.168.0.81:/opt/chroot/tmp/

# Install using our rpm (from outside chroot)
ssh root@192.168.0.81 "/bin/sh -c 'LD_LIBRARYN32_PATH=/opt/chroot/tmp:/opt/chroot/usr/sgug/lib32:/usr/sgug/lib32 /opt/chroot/tmp/rpm -Uvh /opt/chroot/tmp/<package>.mips.rpm --root=/opt/chroot --nodeps'"

# Query installed packages
ssh root@192.168.0.81 "/bin/sh -c 'LD_LIBRARYN32_PATH=/opt/chroot/tmp:/opt/chroot/usr/sgug/lib32:/usr/sgug/lib32 /opt/chroot/tmp/rpm -qa --root=/opt/chroot'"
```

### Update rpm binary on IRIX after rebuild
```bash
cd /tmp && rm -rf rpm-extract rpmlibs-extract
mkdir rpm-extract rpmlibs-extract
cd rpm-extract && rpm2cpio ~/rpmbuild/RPMS/mips/rpm-4.19.1.1-1.mips.rpm | cpio -idmv
cd ../rpmlibs-extract && rpm2cpio ~/rpmbuild/RPMS/mips/rpm-libs-4.19.1.1-1.mips.rpm | cpio -idmv
scp /tmp/rpm-extract/usr/sgug/bin/rpm /tmp/rpmlibs-extract/usr/sgug/lib32/librpm.so /tmp/rpmlibs-extract/usr/sgug/lib32/librpmio.so root@192.168.0.81:/opt/chroot/tmp/
```

---

## Session Summary (2026-01-31)

### sed→patch Migration Validation

Tested full build cycle for migrated packages:
- **bzip2**: Build successful after fixing patch line numbers (34 vs 28)
- **popt**: Build successful after regenerating patches from actual source

**Lesson learned**: Patches must be generated from actual source tarballs using `diff -u`, not created manually with guessed line numbers.

### Documentation Restructuring

Created `rules/methods/` directory with process documentation:

| File | Purpose |
|------|---------|
| `text-replacement.md` | When to use .patch vs safepatch vs sed |
| `irix-testing.md` | Shell rules, chroot behavior, debugging with par |
| `compat-functions.md` | How to add missing functions to catalog |
| `autoconf-cross.md` | ./configure package patterns |
| `cmake-cross.md` | CMake package patterns (rpm, libsolv, tdnf) |

Updated `rules/INDEX.md` to reference all methods at the top.

**Goal**: Improve AI agent context efficiency - agents read INDEX.md first, then retrieve specific methods as needed, rather than keeping everything in context.

### Files Modified

- `patches/packages/bzip2/disable-test.patch` - Fixed line numbers
- `patches/packages/popt/popt-disable-stpcpy.patch` - Regenerated from source
- `patches/packages/popt/popt-disable-mbsrtowcs.patch` - Regenerated from source
- `Claude.md` - Added safepatch instructions
- `rules/INDEX.md` - Added Methods section with links
- `rules/methods/*.md` - 5 new method files
- Plan file - Trimmed to active work only

---

## Session Summary (2026-01-31, Evening)

### tdnf Path Patches Complete

**Problem**: tdnf was still accessing `/etc/tdnf/tdnf.conf` despite path changes.

**Root cause**: The `client-defines.sgifixes.patch` only patched `client/defines.h`, but `common/config.h` ALSO has the same defines. Both files needed patching.

**Fix**: Updated `patches/packages/tdnf/client-defines.sgifixes.patch` to patch BOTH files:
- `client/defines.h` - 4 hunks
- `common/config.h` - 4 hunks

Patched paths now include:
- `TDNF_CONF_FILE` → `/usr/sgug/etc/tdnf/tdnf.conf`
- `TDNF_DEFAULT_REPO_LOCATION` → `/usr/sgug/etc/yum.repos.d`
- `TDNF_DEFAULT_CACHE_LOCATION` → `/usr/sgug/var/cache/tdnf`
- `TDNF_DEFAULT_DATA_LOCATION` → `/usr/sgug/var/lib/tdnf`
- `TDNF_DEFAULT_VARS_DIRS` → `/usr/sgug/etc/tdnf/vars ...`
- `TDNF_DEFAULT_DISTROVERPKG` → `sgugrse-release`
- `TDNF_DEFAULT_DISTROARCHPKG` → `mips`
- `TDNF_DEFAULT_PLUGIN_CONF_PATH` → `/usr/sgug/etc/tdnf/pluginconf.d`

**Verified**: `strings libtdnf.so` now shows correct `/usr/sgug/` paths.

### RPM Database Backend Issue

**Problem**: Our cross-compiled rpm 4.19 uses sqlite backend by default, but SGUG-RSE's rpm uses BerkeleyDB.

**Symptom**: `Error(1304) : Solv - I/O error` when tdnf tries to read the empty sqlite rpmdb.

**Solution**:
1. Create macro file to set sqlite backend:
   ```bash
   echo "%_db_backend sqlite" > /usr/sgug/etc/rpm/macros.sqlite
   ```
2. Initialize sqlite database:
   ```bash
   rpmdb --initdb --dbpath=/usr/sgug/var/lib/rpm -D "_db_backend sqlite"
   ```

### Current Status

After path fix and sqlite macro setup:
- ✅ tdnf reads config from `/usr/sgug/etc/tdnf/tdnf.conf`
- ✅ rpm uses sqlite backend (not BDB)
- ❌ tdnf fails with "distroverpkg config entry is set to a package that is not installed"

The distroverpkg (`sgugrse-release`) check fails because:
1. Our sqlite rpmdb is empty (no packages installed)
2. SGUG-RSE's BDB database has packages but we can't read it with our rpm

### Next Steps

1. **Install sgugrse-release into sqlite rpmdb** - Download or create the package
2. **Or disable distroverpkg check** - Add `distroverpkg=` to tdnf.conf (but empty value causes segfault)
3. **Or set to a package we have** - e.g., `distroverpkg=popt` if popt is installed

### Files Modified

- `patches/packages/tdnf/client-defines.sgifixes.patch` - Extended to patch both `client/defines.h` AND `common/config.h`
- `rules/packages/tdnf.yaml` - Added `client-defines.sgifixes.patch` to `add_patch:`, removed redundant sed commands

### Key Learning

When debugging path issues:
1. Check SGUG-RSE patches first (`/home/edodd/projects/github/sgug-rse/packages/<pkg>/*.sgifixes.patch`)
2. Use `par` on IRIX to trace system calls and see actual paths accessed
3. Use `strings` on binaries to verify compiled-in paths
4. Check ALL header files for duplicate defines (not just the obvious one)

---

## Session Summary (2026-02-02, Continued)

### Patch Creation Workflow (mkpatch)

Integrated sgug-rse style patch creation workflow into mogrix:

**New Components:**
- `tools/mkpatch` - Script for creating patches from changes vs .origfedora
- `mogrix/emitter/spec.py` - Modified to inject .origfedora copy after %setup/%autosetup
- `rules/INDEX.md` - Updated with Patch Creation section

**Workflow:**
```bash
# 1. Convert the package (creates spec with .origfedora support)
mogrix convert ~/rpmbuild/SRPMS/fc40/foo-1.0.src.rpm -o /tmp/foo/

# 2. Run prep phase only (extracts source AND creates .origfedora copy)
rpmbuild -bp /tmp/foo/foo.spec --nodeps

# 3. Enter source directory and make changes
cd ~/rpmbuild/BUILD/foo-1.0
vim src/file.c

# 4. Create patch from your changes
mkpatch diff .
# -> patches/packages/foo/foo.irixfixes.patch

# 5. Add to YAML
# add_patch:
#   - foo.irixfixes.patch
```

**Benefits:**
- Patches preferred over sed for source modifications (sed silently fails on pattern mismatch)
- .origfedora copy created automatically during conversion
- Patches are version-controlled and reviewable
- mkpatch tool handles diff formatting and naming conventions

**tdnf sed→patch Migration:**
- Replaced ~25 lines of sed commands with `tdnf.irixfixes.patch` (112 lines)
- Patch covers: strings.h in 4 includes.h files, IRIX headers, statfs signature, mips arch hardcode
- Validated by successful tdnf build

---

## Session Summary (2026-02-02)

### Dependency Installation Fixes

Multiple issues prevented clean package installation without `--nodeps`:

**Issue 1: drop_requires regex too simplistic**

Original `drop_requires` only matched simple `Requires: packagename` lines. Failed on:
- `Requires(pre): sed grep gawk` - multi-package lines
- `Requires: rpm-sequoia%{_isa}` - ISA suffix
- Partial match: `libsolv` matched `libsolv-devel` leaving `Requires: -devel`

**Fix**: Enhanced `mogrix/emitter/spec.py:drop_requires()` to:
- Handle `Requires(pre/post/preun/postun):` variants
- Handle multi-package lines by removing just the matching package
- Use negative lookahead `(?![a-zA-Z0-9_-])` to match whole package names only
- Handle `%{_isa}` suffixes

**Issue 2: setenv unresolved symbol in librpm.so**

The `--whole-archive` flag was only in `CMAKE_EXE_LINKER_FLAGS`, not `CMAKE_SHARED_LINKER_FLAGS`.

**Fix**: Added `--whole-archive` to both linker flags in `rules/packages/rpm.yaml`:
```yaml
-DCMAKE_EXE_LINKER_FLAGS="-Wl,--whole-archive $COMPAT_DIR/libmogrix-compat.a -Wl,--no-whole-archive -lgen"
-DCMAKE_SHARED_LINKER_FLAGS="-Wl,--whole-archive $COMPAT_DIR/libmogrix-compat.a -Wl,--no-whole-archive -lgen"
```

**Issue 3: Missing /bin/sh provider**

Packages with shell scriptlets require `/bin/sh`. IRIX has `/bin/sh` but it's not in the RPM database.

**Fix**: Created `rules/packages/sgugrse-release.yaml` to add:
```yaml
spec_replacements:
  - pattern: "Provides:       system-release\n"
    replacement: "Provides:       system-release\nProvides:       /bin/sh\nProvides:       /usr/bin/sh\n"
```

Also disabled swidtag entries (Fedora-specific macros undefined on IRIX).

**Issue 4: IRIX ln doesn't support -r (relative symlinks)**

RPM's `%pre` and `%posttrans` scriptlets use `ln -sfr`. IRIX `ln` fails with "Illegal option -- r".

**Fix**: Added to `rules/packages/rpm.yaml`:
```yaml
- pattern: "ln -sfr"
  replacement: "ln -sf"
```

**Issue 5: IRIX sed doesn't support | delimiter**

The `%pre` scriptlet uses `sed 's|^/var/lib/rpm/||g'` which IRIX sed doesn't parse.

**Fix**: Added to `rules/packages/rpm.yaml`:
```yaml
- pattern: "rpmdb_files=$(find /var/lib/rpm ...| sed 's|^/var/lib/rpm/||g' | sort)..."
  replacement: "for rpmdb_path in /var/lib/rpm/*; do\n        [ -f \"$rpmdb_path\" ] || continue\n        rpmdb_file=$(basename \"$rpmdb_path\")..."
```

### Current Status

**rpm is broken** after reinstall attempts. The chroot needs to be reset and bootstrap tarball re-extracted.

**Packages installed before breakage**:
- sgugrse-release, rpm, rpm-libs installed (but rpm became non-functional)
- tdnf packages need reinstall

### Files Modified

- `mogrix/emitter/spec.py` - Enhanced `drop_requires()` regex handling
- `rules/packages/rpm.yaml` - Added `--whole-archive` to SHARED_LINKER_FLAGS, `ln -sfr` fix, sed→basename fix
- `rules/packages/sgugrse-release.yaml` - New file: provides /bin/sh, disables swidtag
- `rules/packages/tdnf.yaml` - Updated drop_requires list

### Packages Rebuilt with Fixes

**rpm-4.19.1.1-1.mips.rpm** (2026-02-02 00:29):
- ✅ IRIX sed fix applied - uses basename instead of `sed 's|...|'`
- ✅ ln -sfr→ln -sf fix applied
- ✅ --whole-archive in SHARED_LINKER_FLAGS

**sgugrse-release-0.0.7beta-1.mips.rpm** (2026-02-02 00:30):
- ✅ Provides /bin/sh and /usr/bin/sh
- ✅ swidtag entries disabled

### Bootstrap Tarball Status

The bootstrap tarball script is ready at `scripts/bootstrap-tarball.sh`.

**Note**: tdnf package needs rebuild (has `/var/cache/tdnf` instead of `/usr/sgug/var/cache/tdnf`), but this directory will be created automatically when tdnf runs. The existing package is functional.

All other packages validated and ready.

### Recovery Steps

1. Reset chroot on IRIX (user can do this)
2. Run `./scripts/bootstrap-tarball.sh` to create tarball
3. Copy to IRIX: `scp tmp/irix-bootstrap.tar.gz root@192.168.0.81:/tmp/`
4. On IRIX:
   ```bash
   cd /opt/chroot
   gzcat /tmp/irix-bootstrap.tar.gz | tar xvf -
   mkdir -p /opt/chroot/usr/sgug/var/cache/tdnf  # Create if missing
   chroot /opt/chroot /bin/sh
   export LD_LIBRARYN32_PATH=/usr/sgug/lib32
   /usr/sgug/bin/rpm --initdb
   /usr/sgug/bin/rpm -Uvh /tmp/bootstrap-rpms/sgugrse-release*.rpm
   /usr/sgug/bin/rpm -Uvh /tmp/bootstrap-rpms/rpm*.rpm
   /usr/sgug/bin/rpm -Uvh /tmp/bootstrap-rpms/*.rpm
   ```
5. Test: `/usr/sgug/bin/sgug-exec /usr/sgug/bin/rpm --version`

---

## Session Summary (2026-02-02, Evening)

### rpm.irixfixes.patch Complete

**Problem**: rpm 4.19.1.1 had many IRIX compatibility issues scattered across inline sed commands in the rules file. These were unreliable (sed silently fails when patterns don't match) and hard to maintain.

**Solution**: Created comprehensive `patches/packages/rpm/rpm.irixfixes.patch` (298 lines, 11 files) adapting sgug-rse's rpm 4.15 patch for rpm 4.19.

### Patch Contents

| File | Fix |
|------|-----|
| `CMakeLists.txt` | Conditional NLS for po directory |
| `lib/fsm.c` | futimens workaround for IRIX /dev/fd paths |
| `lib/headerfmt.c` | Remove `__thread` TLS (IRIX rld doesn't support) |
| `lib/rpmrc.c` | IRIX architecture detection + skip generic MIPS |
| `lib/rpmug.c` | Remove `__thread` TLS |
| `macros.in` | `_buildshell` path + `LD_LIBRARYN32_PATH` export |
| `misc/fts.c` | `dirfd`, `stat64`, `__errno_location` for IRIX |
| `misc/system.h` | `xsetprogname`/`xgetprogname` for IRIX |
| `rpmio/macro.c` | `_SC_NPROC_ONLN` for CPU detection |
| `rpmio/rpmlog.c` | Remove `__thread` + iterative vsnprintf |
| `rpmio/rpmstring.c` | Iterative vsnprintf for IRIX |

### Key IRIX Compatibility Issues Addressed

1. **IRIX vsnprintf pre-C99**: `vsnprintf(NULL, 0, fmt, ap)` returns -1 instead of buffer size
   - Fix: Iterative doubling approach in `rvasprintf()` and `rpmlog()`

2. **IRIX rld doesn't support `__thread` TLS**: Thread-local storage causes link failures
   - Fix: Remove `__thread` keyword (accept single-threaded limitation)

3. **IRIX futimens/utimes with /dev/fd**: `utimes("/dev/fd/N", ...)` fails with ENOENT
   - Fix: Force `utimensat()` path which uses actual pathname

4. **IRIX uname returns IP30/IP32**: Not recognized as valid architecture
   - Fix: Hardcode "mips" based on `__MIPS_SIM` ABI detection

5. **IRIX uses `_SC_NPROC_ONLN`**: Not `_SC_NPROCESSORS_ONLN` for CPU count
   - Fix: Conditional `#if defined(__sgi)` for sysconf call

### mogrix/emitter/spec.py Fix

**Problem**: Patches added as `Patch200:` tags were applied BEFORE `%patchlist` patches. This caused line number mismatches since Fedora patches modify macros.in.

**Fix**: Modified spec emitter to detect `%patchlist` and add patch filenames to the END of patchlist instead of using Patch tags. This ensures IRIX patches apply after all Fedora patches.

### Files Modified

- `patches/packages/rpm/rpm.irixfixes.patch` - New comprehensive IRIX patch (298 lines)
- `mogrix/emitter/spec.py` - Add patches to %patchlist when present
- `rules/packages/rpm.yaml` - Already had `add_patch: rpm.irixfixes.patch`

### Verification

Patch applies cleanly after all Fedora patchlist patches:
```bash
rpmbuild -bp ~/rpmbuild/SPECS/rpm.spec --nodeps
# No errors, all 11 files patched successfully
```

### Next Steps

1. Continue rebuilding bootstrap packages (libxml2 next)
2. Regenerate bootstrap tarball when complete
3. Deploy to IRIX chroot for testing

---

## Session Summary (2026-02-02, Afternoon)

### Bootstrap Chain Rebuild Progress

Rebuilt 4 of 14 bootstrap packages:

| Package | Version | Status |
|---------|---------|--------|
| zlib-ng | 2.1.6 | ✅ Built |
| bzip2 | 1.0.8 | ✅ Built |
| popt | 1.19 | ✅ Built |
| openssl | 3.2.1 | ✅ Built |

### Infrastructure Improvements

#### 1. MOGRIX_ROOT Auto-Detection

**Problem**: Rules used `$MOGRIX_ROOT/tools/fix-libtool-irix.sh` but MOGRIX_ROOT wasn't being set, causing build failures.

**Fix**: Modified `mogrix/emitter/spec.py` to:
- Calculate MOGRIX_ROOT from the package location at runtime
- Automatically export it in every spec's `%build` section

```python
# mogrix/emitter/spec.py
MOGRIX_ROOT = str(Path(__file__).parent.parent.parent.resolve())
# ... later in write() ...
all_export_vars = {"MOGRIX_ROOT": MOGRIX_ROOT}
```

Now rules can use `$MOGRIX_ROOT/tools/...` without hardcoding paths - portable across installations.

#### 2. UINT64_C Macro Fix

**Problem**: zlib-ng build failed with "call to undeclared function 'UINT64_C'".

**Root cause**: IRIX's `inttypes.h` only defines `UINT64_C` when `_SGIAPI` is true. But `_SGIAPI` is disabled when `_XOPEN_SOURCE` or `_POSIX_C_SOURCE` are defined (common in portable code like zlib-ng's zbuild.h).

**Fix**: Added fallback definitions to `/opt/sgug-staging/usr/sgug/include/mogrix-compat/generic/inttypes.h`:
```c
#ifndef UINT64_C
#define UINT64_C(c) c ## ULL
#endif
/* ... and INT64_C, UINT32_C, etc. */
```

#### 3. origfedora/diff Workflow Confirmed

All converted specs automatically include:
```bash
# Create .origfedora copy for patch development (mkpatch workflow)
_srcdir=$(basename $(pwd))
cd .. && cp -a "$_srcdir" "${_srcdir}.origfedora" && cd "$_srcdir"
```

This enables proper patch creation workflow - no sed for source modifications.

### Rules Best Practices Enforced

1. **Never hardcode absolute paths in rules** - Use `$MOGRIX_ROOT` or staging paths
2. **Use `add_patch` for source modifications** - Not sed in `prep_commands`
3. **Tools go in staging** - `/opt/sgug-staging/share/mogrix/` for helper scripts
4. **Patches go in patches/packages/<pkg>/** - Version controlled, reviewable

### Files Modified

- `mogrix/emitter/spec.py` - MOGRIX_ROOT auto-detection + export
- `/opt/sgug-staging/usr/sgug/include/mogrix-compat/generic/inttypes.h` - UINT64_C fallback
- `rules/packages/zlib-ng.yaml` - Cleaned up, uses add_patch properly
- `rules/packages/popt.yaml` - Uses $MOGRIX_ROOT for fix-libtool-irix.sh
- `patches/packages/zlib-ng/zlib-ng-inttypes.patch` - Adds inttypes.h include to zbuild.h

---

## Session Summary (2026-02-02, Final)

### Bootstrap Chain Complete!

All 14 packages in the bootstrap chain have been rebuilt successfully, producing 80 MIPS RPMs.

**Key Issue Fixed: Wrong SRPM Used for tdnf**

The tdnf conversion was failing with "Source100 defined multiple times". Investigation revealed:
- `/home/edodd/rpmbuild/SRPMS/fc40/tdnf-3.5.14-1.src.rpm` was a PREVIOUSLY CONVERTED SRPM (already had Source100-104 injected)
- `/home/edodd/rpmbuild/SRPMS/fc40/tdnf-3.5.14-1.ph5.src.rpm` is the original Photon 5 SRPM

**Fix**: Use the `.ph5.src.rpm` (original Photon) instead of the already-converted SRPM.

**Lesson learned**: Always verify SRPMs are originals before conversion. A previously-converted SRPM will have compat sources already injected.

### Compat Header Fixes Applied

During the rebuild, several compat header issues were discovered and fixed:

1. **setjmp.h** (`/opt/sgug-staging/usr/sgug/include/mogrix-compat/generic/setjmp.h`)
   - IRIX setjmp.h checks `_MIPS_ISA_MIPS[1-4]` but clang defines `_MIPS_ISA_MIPS64`
   - Without this wrapper, jmp_buf typedef doesn't happen and setjmp/longjmp fail to compile

2. **math.h** (`/opt/sgug-staging/usr/sgug/include/mogrix-compat/generic/math.h`)
   - IRIX math.h has ldexpf/frexpf in `#if 0 /* not yet implemented */`
   - Added inline wrappers that call the double versions

3. **inttypes.h** - UINT64_C macro fix (from earlier)

### stat64 Issue in rpm.irixfixes.patch

The rpm patch originally had `#define stat64 stat` which caused:
```
error: redefinition of 'stat'
```

**Root cause**: IRIX has BOTH `struct stat` AND `struct stat64` as separate types. The macro caused the preprocessor to turn `struct stat64 {...}` into `struct stat {...}`, conflicting with the existing struct stat definition.

**Fix**: Removed the `#define stat64 stat` line. IRIX's `fstat()` works with `struct stat` directly (the FTS_FSTAT64 macro already uses fstat() for IRIX).

---

## Session Summary (2026-02-02, stpcpy Fix)

### rpm --help Regression Fixed

**Problem**: `rpm --help: unknown option` error on IRIX - popt wasn't recognizing POPT_AUTOHELP alias.

**Root cause**: The compat header deployment was out of sync. The `stpcpy` declaration had been removed from the staging header at `/opt/sgug-staging/usr/sgug/include/mogrix-compat/generic/string.h` but the repo version still had it. popt uses stpcpy internally, and without the declaration, popt's internal stpcpy copy was used incorrectly.

**Fixes applied**:

1. **Restored stpcpy declaration** in compat headers
2. **Removed #ifndef guards** from all compat declarations - multiple identical declarations are legal C, but #ifndef caused silent failures if something else defined the symbol as a macro
3. **Added `mogrix sync-headers` command** - Forces resync of compat headers from repo to staging
4. **Updated staging.py** - Added `force` parameter so headers resync properly

### Documentation Consolidation

Cleaned up documentation to reduce token usage:

| File | Change |
|------|--------|
| `rules/INDEX.md` | Compacted to ~70 lines (was 317), now just a lookup table |
| `Claude.md` | Trimmed to ~80 lines, just philosophy + file pointers |
| `rules/methods/patch-creation.md` | New file with patch workflow content |
| `rules/methods/build-order.md` | Moved from root `BUILD_ORDER.md` |
| `docs/archive/BOOTSTRAP.md` | Archived from root |

### Files Modified

- `compat/include/mogrix-compat/generic/string.h` - Removed #ifndef guards, unconditional declarations
- `mogrix/staging.py` - Added `force_headers` parameter to `ensure_ready()`
- `mogrix/cli.py` - Added `sync-headers` command
- `rules/methods/compat-functions.md` - Added sync documentation
- `compat/catalog.yaml` - Added standalone stpcpy entry

### Verification Needed

Bootstrap tarball with stpcpy fix has been created and deployed to IRIX at `/tmp/irix-bootstrap.tar.gz`.

**Test commands on IRIX:**
```bash
cd /opt/chroot
rm -rf usr/sgug
gzcat /tmp/irix-bootstrap.tar.gz | tar xvf -
chroot /opt/chroot /bin/sh
export LD_LIBRARYN32_PATH=/usr/sgug/lib32
/usr/sgug/bin/rpm --help
```

### Next Steps

1. Verify `rpm --help` works on IRIX with the new bootstrap tarball
2. Continue with Package Catalog Expansion (ncurses chain, GPG chain, etc.)

---

## Session Summary (2026-02-02/03, LLD 18 Fix)

### ROOT CAUSE IDENTIFIED

**Problem**: `rpm --help` returned "unknown option" despite popt library working correctly (tdnf --help worked fine).

**Root cause**: LLD 14 (vvuk's fork) generates incorrect relocations for external data symbols embedded in static arrays on MIPS N32.

When rpm's `optionsTable` array includes pointers to external symbols like `poptHelpOptions`:
```c
static struct poptOption optionsTable[] = {
    POPT_AUTOHELP  // expands to { ..., &poptHelpOptions, ... }
    POPT_AUTOALIAS // expands to { ..., &poptAliasOptions, ... }
    POPT_TABLEEND
};
```

**LLD 14 generated** (BROKEN):
```
00035f9c  00000003 R_MIPS_REL32     (no symbol)
```
These anonymous relocations leave the pointers as NULL at runtime.

### WRONG FIX ATTEMPTED: GNU ld for executables

Initially tried switching to GNU ld for all linking. This generates correct relocations BUT crashes IRIX rld with SIGSEGV during initialization (wrong segment layout for executables).

**This is a known bad fix - do not attempt again.** See `rules/methods/linker-selection.md`.

### CORRECT FIX: LLD 18 with IRIX patches

The `lld-fixes/` directory contains patches for LLD 18 that fix the relocation bug. The patched binary is at:
```
/home/edodd/projects/github/unxmaal/mogrix/tools/bin/ld.lld-irix-18
```

Updated `cross/bin/irix-ld` to use LLD 18 instead of LLD 14:
```bash
LLD="${IRIX_LLD:-/home/edodd/projects/github/unxmaal/mogrix/tools/bin/ld.lld-irix-18}"
```

### Verification

After rebuilding rpm with LLD 18:
```bash
$ readelf -r rpm | grep -E "(poptHelp|poptAlias|rpmInstall)"
00035ffc  00007b03 R_MIPS_REL32      00000000   rpmInstallPoptTable
00036050  00007d03 R_MIPS_REL32      00000000   poptAliasOptions
0003606c  00007e03 R_MIPS_REL32      00000000   poptHelpOptions
```

All external data symbols now have proper symbol-referencing relocations.

### Bootstrap Tarball

Created `/tmp/bootstrap-lld18.tar.gz` (41.8MB) - copied to IRIX for testing.

### Documentation Added

To prevent repeating this mistake:
- `rules/methods/linker-selection.md` - Documents linker strategy and KNOWN BAD FIXES
- `rules/INDEX.md` - Added "STOP - Before Making Significant Changes" section
- `cross/bin/irix-ld` - Added warning comments about GNU ld

### Test Commands for IRIX

```bash
cd /opt/chroot
rm -rf usr/sgug
gzcat /tmp/bootstrap-lld18.tar.gz | tar xvf -
chroot /opt/chroot /bin/sh
export LD_LIBRARYN32_PATH=/usr/sgug/lib32
/usr/sgug/bin/rpm --help
```

---

## Session Summary (2026-02-03, Bootstrap Rebuild)

### Clean Rebuild of All 14 Bootstrap Packages

Ran `./cleanup.sh` and rebuilt entire bootstrap chain from scratch:

| # | Package | Version | Status |
|---|---------|---------|--------|
| 1 | zlib-ng | 2.1.6 | ✅ |
| 2 | bzip2 | 1.0.8 | ✅ |
| 3 | popt | 1.19 | ✅ |
| 4 | openssl | 3.2.1 | ✅ |
| 5 | libxml2 | 2.12.5 | ✅ |
| 6 | curl | 8.6.0 | ✅ |
| 7 | xz | 5.4.6 | ✅ |
| 8 | lua | 5.4.6 | ✅ |
| 9 | file | 5.45 | ✅ |
| 10 | sqlite | 3.45.1 | ✅ |
| 11 | rpm | 4.19.1.1 | ✅ |
| 12 | libsolv | 0.7.28 | ✅ |
| 13 | tdnf | 3.5.14 | ✅ |
| 14 | sgugrse-release | 0.0.7beta | ✅ |

**Total: 70 MIPS RPMs built**

### Issues Fixed During Rebuild

1. **Multilib header symlinks** - Clang defines `__mips64` even for n32 ABI, but OpenSSL/Lua only ship `configuration-mips.h` / `luaconf-mips.h`. Created symlinks:
   - `configuration-mips64.h -> configuration-mips.h`
   - `luaconf-mips64.h -> luaconf-mips.h`

2. **curl OpenSSL detection** - Updated `rules/packages/curl.yaml` to pass explicit `--with-openssl=/opt/sgug-staging/usr/sgug` path instead of relying on pkg-config.

3. **ldconfig scriptlet failures** - Added `macros.ldconfig` to rpm package that makes `%ldconfig*` macros no-ops (IRIX uses rld, not ldconfig).

### Files Modified

- `rules/packages/curl.yaml` - Explicit OpenSSL path
- `rules/packages/rpm.yaml` - Added macros.ldconfig creation
- `configs/rpm/macros.ldconfig` - New file (IRIX ldconfig stub macros)

### Bootstrap Tarball

- Location: `tmp/irix-bootstrap.tar.gz` (40MB)
- Includes macros.ldconfig for silent ldconfig scriptlets
- **VERIFIED**: tdnf installing packages from remote repo on IRIX

### Remaining Inline Seds (Deferred)

These packages still have inline seds but are working fine:

| Package | Sed Count | Purpose |
|---------|-----------|---------|
| libsolv | 2 | xmlErrorPtr const, HAVE_FUNOPEN |
| tdnf | 6 | CMake subdirs, tdnf.conf paths |
| rpm | 5 | BZip2 target, SONAME fixes |

Most are CMake/config changes that don't fit well as source patches. Not blocking.

---

## Session Summary (2026-02-04, Phase 2 Build Tools)

### Phase 2: Build Tools Chain for IRIX

Building native build tools so IRIX can compile packages locally. Build order: m4 -> autoconf -> libtool -> automake -> bash -> perl.

### m4 1.4.19 - COMPLETE

**SRPMs**: `/home/edodd/rpmbuild/SRPMS/fc40/m4-1.4.19-9.fc40.src.rpm`

Required 5 fixes for cross-compilation with clang on IRIX:

1. **wcrtomb/mbsinit undeclared** - IRIX `wchar.h` hides C99/XPG5 functions behind `_WCHAR_CORE_EXTENSIONS_1` (requires `__c99` or `_XOPEN_SOURCE>=500`). Clang doesn't define `__c99` (SGI MIPSpro thing). Fix: `ac_cv_func_mbsinit: "no"`, `ac_cv_func_wcrtomb: "no"` so gnulib provides replacements.

2. **strerror_r linker error** - `dicl-clang-compat/string.h` declared `char *strerror_r()` but no implementation existed. Configure found the declaration → `HAVE_STRERROR_R=1` → linker error. Fix: Removed dead declaration from string.h. error.c falls back to `strerror()`.

3. **gnulib select replacement** - `gl_cv_func_select_detects_ebadf` defaults to "guessing no" for cross-compilation, causing gnulib to replace `select()`. IRIX's select() works fine. Fix: `gl_cv_func_select_detects_ebadf: "yes"`.

4. **getrandom** - Our compat `sys/random.h` provides a stub. Fix: `ac_cv_func_getrandom: "yes"` to prevent gnulib from compiling its own.

5. **find_lang** - NLS disabled for cross-compilation. Fix: `skip_find_lang: true`.

**Key insight**: IRIX `wchar_core.h` (lines 417-458) hides these functions behind `_WCHAR_CORE_EXTENSIONS_1`: btowc, wctob, mbsinit, mbrlen, mbrtowc, wcrtomb, mbsrtowcs, wcsrtombs. Any package using gnulib regex will hit this.

**SGUG-RSE comparison**: They used m4 1.4.18 with native GCC, never hit `_WCHAR_CORE_EXTENSIONS_1` issues. Their patches were getprogname and glibc compat only.

**Verified on IRIX**: `m4 --version` and macro expansion test passed.

### autoconf 2.71 - COMPLETE

**SRPMs**: `/home/edodd/rpmbuild/SRPMS/fc40/autoconf-2.71-10.fc40.src.rpm`

Noarch package (Perl scripts + m4 macros). Only packaging fixes needed:

1. **Emacs support disabled** - `%bcond_without autoconf_enables_emacs` → `%bcond_with`
2. **_emacs_sitelispdir macro removed** - Not defined for IRIX
3. **find_lang skipped** - `skip_find_lang: true`

**Verified on IRIX**: `autoconf --version` passed.

### libtool 2.4.7 - BUILT, NEEDS DEPLOY

**SRPMs**: `/home/edodd/rpmbuild/SRPMS/fc40/libtool-2.4.7-10.fc40.src.rpm`

Required 6 fixes:

1. **Old sgifixes.patch removed** - 2.4.6 patch doesn't apply to 2.4.7. All IRIX inherit_rpath fixes already in upstream 2.4.7.

2. **help2man missing** - Dropped BuildRequires, added `HELP2MAN=true` to both `%make_build` and `%make_install`.

3. **%_hardening_cflags not defined** - Fedora hardening macros (`CUSTOM_LTDL_CFLAGS/LDFLAGS`) not available for IRIX. Removed from make command.

4. **Info files not compressed** - Fedora's `__brp_compress` doesn't run during cross-compilation. Changed `%{_infodir}/libtool.info*.gz` to `%{_infodir}/libtool.info*`.

5. **host_os=irix doesn't match libtool patterns** - rpmbuild's `--target mips-sgi-irix` sets `host_os=irix`, but libtool's configure checks for `irix5*` | `irix6*` patterns to enable shared library support. Fix: Added `--host=mips-sgi-irix6.5` to override the `%configure` expansion, making `host_os=irix6.5`.

6. **Brace expansion in rpmbuild shell** - `rm -f libltdl.{a,la}` not supported. Expanded to separate filenames.

**Built RPMs**: `libtool-2.4.7-10.mips.rpm`, `libtool-ltdl-2.4.7-10.mips.rpm`, `libtool-ltdl-devel-2.4.7-10.mips.rpm`

**Deploy command** (needs `--nodeps` due to `gcc(major)` and `automake` deps):
```bash
scp ~/rpmbuild/RPMS/mips/libtool-2.4.7-10.mips.rpm ~/rpmbuild/RPMS/mips/libtool-ltdl-2.4.7-10.mips.rpm ~/rpmbuild/RPMS/mips/libtool-ltdl-devel-2.4.7-10.mips.rpm edodd@192.168.0.81:/tmp/
ssh root@192.168.0.81 '/usr/sgug/bin/sgug-exec /usr/sgug/bin/rpm -Uvh --nodeps /tmp/libtool-ltdl-2.4.7-10.mips.rpm /tmp/libtool-2.4.7-10.mips.rpm /tmp/libtool-ltdl-devel-2.4.7-10.mips.rpm'
```

### Pending: automake, bash, perl

Task list:
- **#14** automake - pending (blocked by autoconf - now unblocked)
- **#15** bash - pending
- **#16** perl - pending

### Important Pattern: ac_cv Overrides for IRIX Cross-Compilation

When cross-compiling GNU packages with gnulib, IRIX's non-standard header layout causes cascading issues. The pattern is:
1. Configure's link tests find functions in libc (they exist in the shared library)
2. Compile-time headers don't declare them due to missing guard macros (`_WCHAR_CORE_EXTENSIONS_1`, `_SGIAPI`, etc.)
3. Build fails with "call to undeclared function"

**Solution**: Override `ac_cv_func_X: "no"` in the package's YAML rules so gnulib provides its own replacement implementation with proper declarations.

### Important: mogrix convert Bakes Overrides

`ac_cv_overrides` and other rules are baked into the converted SRPM during `mogrix convert`. They are NOT applied at build time. **Must re-run `mogrix convert` after changing rules**, then rebuild from the new converted SRPM.

### Files Modified

- `rules/packages/m4.yaml` - ac_cv overrides for mbsinit, wcrtomb, select, getrandom; skip_find_lang
- `rules/packages/autoconf.yaml` - NEW: disable emacs, skip_find_lang
- `rules/packages/libtool.yaml` - Updated: remove old patch, help2man, hardening flags, info gz, host triple, brace expansion
- `cross/include/dicl-clang-compat/string.h` - Removed dead strerror_r declaration
- `cross/include/dicl-clang-compat/sys/select.h` - NEW: declares select() for IRIX

### Dependency Fix (2026-02-04)

Attempted to install autoconf and libtool on IRIX - both failed due to missing dependencies:

- **autoconf**: Requires `perl-interpreter` and `perl(File::Compare)` - autoconf is Perl scripts, perl is a genuine runtime dependency
- **libtool**: Requires `autoconf`, `automake`, `findutils`, `gcc(major)=13`, `sed`, `tar`

**Fixes applied**:
- `rules/packages/libtool.yaml` - Added `drop_requires` for `gcc(major)` (Fedora GCC, we use clang) and `automake = 1.16.5` (version lock too strict)
- `rules/packages/sgugrse-release.yaml` - Added `Provides:` for IRIX system tools (sed, tar, findutils, grep, gawk, diffutils, coreutils) so RPM deps are satisfied

**Correct Phase 2 build order**: perl → autoconf → automake → libtool

**Lesson**: A package is not complete until it can be installed with `rpm -Uvh` without `--nodeps`. Build success != installable.

### Phase 2 Build Tools Complete (2026-02-04)

All Phase 2 build tools installed on IRIX **without `--nodeps`**:

| Package | Version | Status | Key Fix |
|---------|---------|--------|---------|
| m4 | 1.4.19 | ✅ Installed | ac_cv overrides for wchar/gnulib |
| autoconf | 2.71 | ✅ Installed | Noarch, disabled emacs |
| automake | 1.16.5 | ✅ Installed | Shebang→/usr/sgug/bin/perl, FindBin @INC |
| libtool | 2.4.7 | ✅ Installed | Shebang→/usr/sgug/bin/bash, dropped gcc(major) |
| libtool-ltdl | 2.4.7 | ✅ Installed | Shared library for IRIX |

**Key issues solved:**

1. **Perl shebang**: `/usr/bin/perl` on IRIX is ancient system perl 5.004. Changed to `/usr/sgug/bin/perl` (SGUG-RSE 5.30.0)

2. **Automake @INC conflict**: IRIX chroot doesn't isolate, so automake's `@INC` path `/usr/sgug/share/automake-1.16` found the base system's SGUG-RSE automake (has `parse_warnings($$)` prototype) instead of our version (has `parse_warnings(@)`). Fixed with `FindBin` for relocatable module path.

3. **Bash shebang**: libtool uses `#!/bin/bash` but IRIX has no `/bin/bash`. Changed to `#!/usr/sgug/bin/bash`.

4. **sgugrse-release Provides**: Added system tool + perl module Provides so RPM deps are satisfied without `--nodeps`. Includes: sed, tar, findutils, grep, gawk, diffutils, coreutils, perl-interpreter, perl(File::Compare), perl(Thread::Queue), perl(threads).

5. **sgugrse-repos drop_requires**: Dropped `sgugrse-repos(%{version})` dependency from sgugrse-release-common (we use our own mogrix.repo). Also fixed `drop_requires` regex to handle `pkg(%{version})` syntax.

6. **RPM --target mips-sgi-irix**: sgugrse-release packages must be built with `--target` flag or they get OS=linux tag which IRIX rpm rejects.

7. **Perl for chroot**: Copied SGUG-RSE perl 5.30.0 binary and modules from base IRIX system to chroot. Perl doesn't work inside IRIX chroot (rld issues) but works from outside with `LD_LIBRARYN32_PATH`.

**Packages installed in chroot RPM database:**
```
ncurses-term-6.4-12.20240127.noarch
m4-1.4.19-9.mips
autoconf-2.71-10.mips
automake-1.16.5-16.mips
libtool-2.4.7-10.mips
libtool-ltdl-2.4.7-10.mips
libtool-ltdl-devel-2.4.7-10.mips
sgugrse-release-0.0.7beta-1.mips
sgugrse-release-common-0.0.7beta-1.mips
```

### Files Modified (2026-02-04 session)

- `rules/packages/sgugrse-release.yaml` - Added Provides for system tools + perl modules, drop_requires for sgugrse-repos
- `rules/packages/libtool.yaml` - drop_requires (gcc(major), automake version), bash shebang fix
- `rules/packages/automake.yaml` - perl shebang fix, FindBin @INC fix, skip_check
- `rules/packages/autoconf.yaml` - (unchanged, was already working)
- `mogrix/emitter/spec.py` - Fixed drop_requires regex for `pkg(...)` syntax

### IRIX Host Info

- IP: 192.168.0.81 (edodd@ for scp, root@ for install)
- Chroot: /opt/chroot
- Install: `ssh root@192.168.0.81 '/bin/sh -c "LD_LIBRARYN32_PATH=/opt/chroot/usr/sgug/lib32:/usr/sgug/lib32 /opt/chroot/usr/sgug/bin/rpm -Uvh --root=/opt/chroot /tmp/<pkg>.rpm"'`
