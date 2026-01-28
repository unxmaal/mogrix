# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-01-28
**Status**: RPM WORKING, TDNF ROOT CAUSE IDENTIFIED - funopen implementation needed

---

## CRITICAL WARNING

**NEVER install packages directly to /usr/sgug on the live IRIX system.**

On 2026-01-27, installing packages directly to /usr/sgug replaced libz.so with zlib-ng, which broke sshd and locked out SSH access. The system had to be recovered from console using backup libraries from /usr/sgug_old.

**Always use /opt/chroot for testing first.** Only touch /usr/sgug after:
1. Full testing in chroot passes
2. Explicit user approval
3. Console access is available as backup
4. Recovery plan is documented

---

## Goal

Get **rpm** and **tdnf** working on IRIX so packages can be installed via the package manager.

---

## Current Progress

### What's Done (2026-01-28)
- All 13 packages cross-compile successfully
- **IRIX vsnprintf bug FIXED** - rpm output no longer garbled
- popt rebuilt with vasprintf injection
- rpm rebuilt with rvasprintf and rpmlog fixes
- **Full chroot testing PASSED** - all rpm and tdnf operations verified

### Verified in /opt/chroot on IRIX

| Test | Result |
|------|--------|
| `rpm --version` | ✓ RPM version 4.19.1.1 |
| `rpm --help` | ✓ Shows options AND descriptions correctly |
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
- **mogrix.repo** at `/opt/chroot/etc/yum.repos.d/mogrix.repo`
- **tdnf.conf** at `/opt/chroot/etc/tdnf/tdnf.conf`
- Repository metadata at `/opt/chroot/usr/sgug/share/mogrix-repo/`

**tdnf.conf settings**:
```ini
[main]
gpgcheck=0
installonly_limit=3
clean_requirements_on_remove=1
repodir=/etc/yum.repos.d
cachedir=/var/cache/tdnf
distroverpkg=        # Disables distro package check
releasever=1         # Required - prevents $releasever lookup
basearch=mips        # Required - prevents $basearch lookup
```

**mogrix.repo settings** (note: full path required for file:// URLs):
```ini
[mogrix]
name=Mogrix IRIX Packages
baseurl=file:///opt/chroot/usr/sgug/share/mogrix-repo
enabled=1
gpgcheck=0
```

| Test | Result |
|------|--------|
| `tdnf repolist` | ✓ Shows "mogrix" repo enabled (exit 0) |
| `tdnf makecache` | Downloads metadata to cache but exits 1 |
| `tdnf list` | No output, exits 1 |
| `tdnf count` | No output, exits 1 |
| `tdnf clean all` | Crashes with SIGSEGV (exit 139) |

### Path Fixes Applied (2026-01-28)

TDNF requires many directories to exist. Created on IRIX:

**In /opt/chroot:**
- `/opt/chroot/etc/tdnf/vars/`
- `/opt/chroot/etc/dnf/vars/`
- `/opt/chroot/etc/yum/vars/`
- `/opt/chroot/etc/tdnf/minversions.d/`
- `/opt/chroot/etc/tdnf/locks.d/`
- `/opt/chroot/etc/tdnf/protected.d/`
- `/opt/chroot/var/lib/rpm` → `../../usr/sgug/lib32/sysimage/rpm` (relative symlink)
- `/opt/chroot/usr/sgug/lib/sysimage/rpm` → `../../lib32/sysimage/rpm` (for `_dbpath` macro mismatch)

**On base IRIX system** (required because IRIX chroot doesn't isolate properly):
- `/etc/yum.repos.d/`
- `/etc/tdnf/pluginconf.d/`
- `/usr/sgug/lib32/rpm` → `/opt/chroot/usr/sgug/lib32/rpm` (symlink)

### Test Method

**IMPORTANT**: Default IRIX shell is csh. Always use `/bin/sh` to run test scripts:

```bash
# Create test script locally
cat > /tmp/test.sh << 'SCRIPT'
#!/bin/sh
export LD_LIBRARYN32_PATH=/opt/chroot/usr/sgug/lib32:/usr/sgug/lib32
/opt/chroot/usr/sgug/bin/tdnf -c /opt/chroot/etc/tdnf/tdnf.conf --installroot=/opt/chroot repolist
echo "exit: $?"
SCRIPT

# Copy and run with /bin/sh
scp /tmp/test.sh root@192.168.0.81:/tmp/
ssh root@192.168.0.81 "/bin/sh /tmp/test.sh"
```

**Note**: IRIX chroot doesn't properly isolate - commands still see base system paths. That's why we run binaries directly with `LD_LIBRARYN32_PATH` instead of using chroot.

### Current Issue: Error(1304) - Solv I/O Error

**ROOT CAUSE IDENTIFIED (2026-01-28):**

All tdnf repo operations (list, count, info, search, makecache) fail with:
```
Error(1304) : Solv - I/O error
```

**Definitive proof:**
```bash
# Test with COMPRESSED input - FAILS (exit 1)
/opt/chroot/usr/sgug/bin/rpmmd2solv < /opt/chroot/var/cache/tdnf/mogrix-fe7d8a84/repodata/primary.xml.gz

# Test with DECOMPRESSED input - WORKS (exit 0)
gzip -dc /opt/chroot/var/cache/tdnf/mogrix-fe7d8a84/repodata/primary.xml.gz | /opt/chroot/usr/sgug/bin/rpmmd2solv
```

**The problem is cookie I/O for transparent .gz decompression:**
- libsolv's `solv_xfopen()` uses either `fopencookie` (GNU) or `funopen` (BSD)
- mogrix provides `fopencookie` via `compat/stdio/fopencookie.c`
- This implementation uses pipes + pthreads and **crashes on IRIX**
- SGUG-RSE solves this by providing `funopen` via `libdiclfunopen`

**Code path:**
```
solv_xfopen() → cookieopen() → fopencookie() → CRASH
                            ↘ funopen() → would work (if available)
```

---

## SOLUTION: Implement funopen in mogrix

**Mogrix philosophy**: Self-contained - incorporate all IRIX compatibility functions rather than depending on external libraries like libdicl.

### Required Changes

**1. Add funopen to compat system** (`compat/stdio/funopen.c`)
   - Port funopen implementation from libdicl or write new one
   - funopen is simpler than fopencookie (single callback vs struct of callbacks)
   - BSD signature: `FILE *funopen(void *cookie, int (*readfn)(), int (*writefn)(), fpos_t (*seekfn)(), int (*closefn)())`

**2. Update libsolv build** (`rules/packages/libsolv.yaml`)
   - Change from `HAVE_FOPENCOOKIE=1` to `HAVE_FUNOPEN=1`
   - Link against mogrix-compat with funopen
   - libsolv cmake checks: `ext/solv_xfopen.c` lines 29-52

**3. Rebuild and test**
   - Rebuild libsolv with funopen support
   - Rebuild tdnf (links against libsolv)
   - Test: `tdnf list`, `tdnf makecache`, `tdnf info popt`

### Implementation Reference

SGUG-RSE's approach (from `sgug-rse/packages/libsolv/libsolv.spec`):
```
export LDFLAGS="-ldicl-0.1 -ldiclfunopen-0.1"
```

libdicl funopen source: https://github.com/danielhams/dicl

### libsolv Cookie I/O Code

From `libsolv-0.7.28/ext/solv_xfopen.c`:
```c
#ifndef WITHOUT_COOKIEOPEN
static FILE *cookieopen(void *cookie, const char *mode,
    ssize_t (*cread)(void *, char *, size_t),
    ssize_t (*cwrite)(void *, const char *, size_t),
    int (*cclose)(void *))
{
#ifdef HAVE_FUNOPEN
  if (!cwrite)
    return funopen(cookie, (int (*)(void *, char *, int))cread, NULL, NULL, cclose);
  return funopen(cookie, NULL, (int (*)(void *, const char *, int))cwrite, NULL, cclose);
#elif defined(HAVE_FOPENCOOKIE)
  // ... fopencookie implementation
#endif
}
#endif
```

### Secondary Issue: Segfault in clean

`tdnf clean all` crashes after partial output:
```
cleaning mogrix: metadata dbcache packages keys expire-cache
Memory fault(coredump)
```
- Likely related to fopencookie crash when cleaning cached .solv files
- Should be fixed by funopen implementation
- If not, debug with `dbx /opt/chroot/usr/sgug/bin/tdnf /opt/chroot/core`

### Other Tasks (Lower Priority)
- Plan migration strategy for production /usr/sgug (conflicts with existing SGUG-RSE)
- Build more packages via mogrix

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

---

## Key Files

| File | Purpose |
|------|---------|
| `compat/stdio/asprintf.c` | Fixed vasprintf with iterative approach |
| `compat/stdio/fopencookie.c` | fopencookie compat (BROKEN - crashes on IRIX) |
| `compat/stdio/funopen.c` | **TODO**: funopen compat (needed for libsolv) |
| `rules/packages/popt.yaml` | popt with vasprintf injection |
| `rules/packages/rpm.yaml` | rpm with rvasprintf and rpmlog fixes |
| `rules/packages/libsolv.yaml` | libsolv - needs HAVE_FUNOPEN=1 |
| `rules/packages/tdnf.yaml` | tdnf cross-compilation config |
| `tools/bin/ld.lld-irix-18` | LLD 18 binary with IRIX patches |
| `/opt/sgug-staging/usr/sgug/bin/irix-ld` | Linker wrapper (uses LLD 18) |

### IRIX Chroot Configuration Files

| File (in /opt/chroot) | Purpose |
|------|---------|
| `/etc/tdnf/tdnf.conf` | tdnf main config (distroverpkg= disabled) |
| `/etc/yum.repos.d/mogrix.repo` | Repository definition |
| `/usr/sgug/share/mogrix-repo/` | Repository metadata location |
| `/var/cache/tdnf/` | tdnf metadata cache |

---

## Build Order (FC40) - Status

| # | Package | Version | Status |
|---|---------|---------|--------|
| 1 | zlib-ng | 2.1.6 | COMPLETE |
| 2 | bzip2 | 1.0.8 | COMPLETE |
| 3 | popt | 1.19 | **REBUILT with vasprintf** |
| 4 | openssl | 3.2.1 | COMPLETE |
| 5 | libxml2 | 2.12.5 | COMPLETE |
| 6 | curl | 8.6.0 | COMPLETE |
| 7 | xz | 5.4.6 | COMPLETE |
| 8 | lua | 5.4.6 | COMPLETE |
| 9 | file | 5.45 | COMPLETE |
| 10 | sqlite | 3.45.1 | COMPLETE |
| 11 | rpm | 4.19.1.1 | **REBUILT with vsnprintf fixes** |
| 12 | libsolv | 0.7.28 | **NEEDS REBUILD with funopen** |
| 13 | tdnf | 3.5.14 | Builds but repo ops fail (waiting on libsolv fix) |

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
  - SOLUTION: Implement funopen instead (see "SOLUTION" section above)
- Existing IRIX system has SGUG-RSE with rpm 4.15.0 already installed
- Our packages conflict with existing SGUG packages (e.g., zlib-ng-compat vs zlib)
- sqlite3 CLI crashes when writing to files (but rpm database writes work)

---

## Long-Term Goal

After tdnf works: port **WebKitGTK 2.38.x** for a modern browser on IRIX.
