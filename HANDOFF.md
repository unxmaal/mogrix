# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-01-25 12:30
**Status**: STARTING FRESH - Clean validation with strict stop-on-error

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
| 1 | zlib | 1.2.13 | PENDING | |
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

```bash
# Clean staging (keep toolchain wrappers in bin/)
sudo rm -rf /opt/sgug-staging/usr/sgug/lib32/*
sudo rm -rf /opt/sgug-staging/usr/sgug/include/*

# Restore compat headers from mogrix
cp -r /home/edodd/projects/github/unxmaal/mogrix/cross/include/* /opt/sgug-staging/usr/sgug/include/
cp -r /home/edodd/projects/github/unxmaal/mogrix/compat/include/* /opt/sgug-staging/usr/sgug/include/

# Clean rpmbuild
rm -rf ~/rpmbuild/BUILD/* ~/rpmbuild/BUILDROOT/* ~/rpmbuild/RPMS/mips/*

# Verify
ls /opt/sgug-staging/usr/sgug/lib32/   # Should be empty
ls ~/rpmbuild/RPMS/mips/               # Should be empty
```

---

## Per-Package Workflow

```bash
# 1. Fetch SRPM (if not already present)
mogrix fetch <package>

# 2. Convert (MUST succeed without errors)
mogrix convert ~/rpmbuild/SRPMS/<package>-*.fc40.src.rpm

# 3. Build
rpmbuild --rebuild ~/rpmbuild/SRPMS/<package>-*-converted/<package>-*.src.rpm \
    --define "_topdir $HOME/rpmbuild" \
    --target mips-sgi-irix \
    --define "__cc /opt/sgug-staging/usr/sgug/bin/irix-cc" \
    --define "__ld /opt/sgug-staging/usr/sgug/bin/irix-ld" \
    --define "_prefix /usr/sgug" \
    --define "_libdir /usr/sgug/lib32" \
    --define "_arch mips" \
    --nocheck \
    --nodeps

# 4. Stage for next package
rpm2cpio ~/rpmbuild/RPMS/mips/<package>*.rpm | cpio -idmv -D /opt/sgug-staging
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
