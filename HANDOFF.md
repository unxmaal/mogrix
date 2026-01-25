# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-01-25 13:45
**Status**: Validation in progress - zlib complete

---

## Goal

Validate mogrix workflow by building tdnf dependency chain one package at a time.

**Critical expectation**: `mogrix convert` should work perfectly for each package. If it doesn't, that's a bug in mogrix rules that must be fixed before proceeding.

## Approach

1. Clean build environment completely
2. For each package in order:
   - Fetch fresh FC40 SRPM
   - Run `mogrix convert`
   - Run `rpmbuild --rebuild`
   - **STOP immediately if either step fails**
   - **NOTIFY user on success** before proceeding
3. Install successful builds to staging before next package

---

## Build Order

| # | Package | Version | Status | Notes |
|---|---------|---------|--------|-------|
| 1 | zlib | 1.2.13 | **DONE** | Libtool fix for minizip |
| 2 | bzip2 | 1.0.8 | PENDING | |
| 3 | popt | 1.19 | PENDING | |
| 4 | openssl | 3.2.1 | PENDING | |
| 5 | libxml2 | 2.12.5 | PENDING | |
| 6 | curl | 8.6.0 | PENDING | |
| 7 | xz | 5.4.6 | PENDING | |
| 8 | lua | 5.4.6 | PENDING | |
| 9 | file | 5.45 | PENDING | |
| 10 | rpm | 4.19.1.1 | PENDING | Known blocker: spawn.h |
| 11 | libsolv | 0.7.28 | PENDING | |
| 12 | **tdnf** | **3.5.14** | PENDING | **TARGET** |

---

## Known Issues from Previous Attempt

These were encountered in today's earlier run. Some may need to be added to mogrix:

| Issue | Description | Fix Location |
|-------|-------------|--------------|
| `__mips` undefined | OpenSSL multiarch headers need it | `cross/bin/irix-cc` (DONE) |
| configuration-mips64.h | Symlink needed in staging | Manual - needs automation |
| luaconf-mips64.h | Symlink needed in staging | Manual - needs automation |
| `_SC_NPROCESSORS_ONLN` | IRIX uses `_SC_NPROC_ONLN` | `compat/.../unistd.h` (DONE) |
| spawn.h missing | rpm 4.19 needs posix_spawn | NOT FIXED |

---

## Cleanup Commands

Use the `cleanup.sh` script for a complete reset:

```bash
./cleanup.sh
```

This script:
1. Cleans staging lib32/ and include/
2. Restores compat headers from mogrix
3. **Builds and installs runtime libraries** (libsoft_float_stubs.a, libatomic.a)
4. Cleans rpmbuild directories
5. Verifies the cleanup

**Note**: `mogrix convert` and `mogrix build` now automatically ensure staging is ready via the `staging.py` module. Manual cleanup is only needed for a complete reset.

---

## Per-Package Workflow

```bash
# 1. Fetch SRPM (if not already present)
mogrix fetch <package>

# 2. Convert (ensures staging is ready automatically)
mogrix convert ~/rpmbuild/SRPMS/<package>-*.src.rpm

# 3. Build (verifies staging as backstop)
mogrix build ~/rpmbuild/SRPMS/<package>-*-converted/<package>-*.src.rpm --cross

# 4. Stage for next package
mogrix stage ~/rpmbuild/RPMS/mips/<package>*.rpm
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

## Key Fixes This Session

### Staging Workflow Improvements

**Problem**: mogrix assumed staging was pre-configured but didn't ensure it.

**Solution**: Added `mogrix/staging.py` module that:
- Checks if staging has required resources (headers, runtime libs)
- Creates missing resources automatically
- Integrated into `mogrix convert` (after conversion) and `mogrix build` (as backstop)

**Required staging resources**:
- `lib32/libsoft_float_stubs.a` - compiler-rt long double stubs
- `lib32/libatomic.a` - atomic operations stubs
- `include/dicl-clang-compat/` - IRIX header compatibility
- `include/mogrix-compat/` - Function compatibility
- `include/irix-compat.h`, `include/getopt.h`

### Libtool Cross-Compilation Fix

**Problem**: Libtool doesn't recognize IRIX and refuses to build shared libraries.

**Solution**: After `%configure`, inject sed commands to force libtool settings:
```bash
sed -i 's|^build_libtool_libs=no|build_libtool_libs=yes|g' libtool
sed -i 's|^deplibs_check_method=.*|deplibs_check_method=pass_all|g' libtool
sed -i 's|^soname_spec=.*|soname_spec="\\$libname\\$shared_ext.1"|g' libtool
sed -i 's|^library_names_spec=.*|library_names_spec="\\$libname\\$shared_ext.1.0.0 \\$libname\\$shared_ext.1 \\$libname\\$shared_ext"|g' libtool
```

**Packages with this fix**: zlib (minizip), lua, file

**Analysis for generic implementation**:
- Can't easily make this a spec_replacement rule because injection point varies by package
- Potential solutions:
  1. Post-configure hook in mogrix engine that patches any `libtool` files found
  2. Wrapper script that patches libtool before make runs
  3. Rule class that packages can inherit
- **Decision**: Keep as per-package rule for now. The pattern is documented and easy to copy.
- **Future**: Consider implementing option 1 (post-configure hook) in mogrix engine

---

## Reference: Compat Functions in Mogrix

| Function | Used By | File |
|----------|---------|------|
| strdup/strndup | file, xz, libsolv, tdnf, curl | string/strdup.c |
| getline | libsolv, tdnf | stdio/getline.c |
| asprintf/vasprintf | tdnf | stdio/asprintf.c |
| fopencookie | libsolv | stdio/fopencookie.c |
| openat family | rpm, libsolv | dicl/openat-compat.c |
| getprogname | rpm | unistd/getprogname.c |
| qsort_r | libsolv, tdnf | stdlib/qsort_r.c |
| getopt_long | file, tdnf | functions/getopt_long.c |
| sqlite3_stub | tdnf | sqlite/sqlite3_stub.c |
