# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-01-28
**Status**: VSNPRINTF FIX VERIFIED - rpm --help and debug output now work correctly on IRIX

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
- `rpm --version` works: shows "RPM version 4.19.1.1"
- `rpm --help` works: shows option flags AND descriptions correctly
- Debug output works: shows "D: Exit status: 1" etc. (not empty)
- popt rebuilt with vasprintf injection
- rpm rebuilt with rvasprintf and rpmlog fixes

### Verified on IRIX
```bash
# Test results from IRIX:
$ rpm --version
RPM version 4.19.1.1

$ rpm --help | head -10
Usage: rpm [OPTION...]

Query/Verify package selection options:
  -a, --all                          query/verify all packages
  -f, --file                         query/verify package(s) owning installed
                                     file
```

### What's Next
1. Copy complete bootstrap tarball to IRIX chroot
2. Test `rpm -qpl <package>` to verify package queries work
3. Initialize rpm database: `rpm --initdb`
4. Test package installation: `rpm -ivh --nodeps <package>`
5. Test tdnf

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
| `rules/packages/popt.yaml` | popt with vasprintf injection |
| `rules/packages/rpm.yaml` | rpm with rvasprintf and rpmlog fixes |
| `tools/bin/ld.lld-irix-18` | LLD 18 binary with IRIX patches |
| `/opt/sgug-staging/usr/sgug/bin/irix-ld` | Linker wrapper (uses LLD 18) |

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
| 12 | libsolv | 0.7.28 | COMPLETE |
| 13 | tdnf | 3.5.14 | Ready to test |

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

- Existing IRIX system has SGUG-RSE with rpm 4.15.0 already installed
- Our packages conflict with existing SGUG packages (e.g., zlib-ng-compat vs zlib)
- sqlite3 CLI crashes when writing to files (but rpm database writes work)

---

## Long-Term Goal

After tdnf works: port **WebKitGTK 2.38.x** for a modern browser on IRIX.
