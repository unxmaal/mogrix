# IRIX Testing Methods

> **For AI agents**: READ THIS before running commands on IRIX.
> IRIX shell and environment differ significantly from Linux.

---

## Connection

| Property | Value |
|----------|-------|
| SSH | `ssh root@192.168.0.81` |
| Hostname | `blue` |
| IRIX version | 6.5.30 |
| Machine | SGI Octane (IP30) |
| Chroot | `/opt/chroot` |
| SGUG-RSE | `/usr/sgug` (existing rpm 4.15.0) |

---

## Shell Rules

### IRIX Default Shell is csh

When you SSH in, you're in csh (C shell). **Always use `/bin/sh` for scripts.**

```bash
# CORRECT - explicitly invoke /bin/sh:
ssh root@192.168.0.81 "/bin/sh -c 'export FOO=bar; echo \$FOO'"

# WRONG - relies on csh which doesn't understand export:
ssh root@192.168.0.81 "export FOO=bar"
```

### Bashisms That Don't Work

| Bashism | POSIX Alternative |
|---------|-------------------|
| `pushd/popd` | `(cd dir && cmd)` subshell |
| `${VAR}{a,b}` brace expansion | `${VAR}a ${VAR}b` explicit |
| `[[ ]]` | `[ ]` |
| `source file` | `. file` |
| `<<<` here-string | `echo "str" \|` |
| `echo -e` | `printf` |
| `function name()` | `name()` without function keyword |

---

## Library Path

**Use `LD_LIBRARYN32_PATH`, not `LD_LIBRARY_PATH`.**

SGUG-RSE uses n32 ABI (32-bit pointers, 64-bit registers).

| Variable | Purpose |
|----------|---------|
| `LD_LIBRARYN32_PATH` | **Use this** - IRIX n32 ABI |
| `LD_LIBRARY_PATH` | Old 32-bit ABI - not for SGUG |
| `LD_LIBRARY64_PATH` | 64-bit ABI - not for SGUG |

---

## Running Binaries

### Method 1: sgug-exec (Recommended)

```bash
# Sets up LD_LIBRARYN32_PATH and PATH automatically
/usr/sgug/bin/sgug-exec /usr/sgug/bin/tdnf --version
```

### Method 2: Manual Library Path

```bash
# For chroot binaries from base system:
LD_LIBRARYN32_PATH=/opt/chroot/usr/sgug/lib32:/usr/sgug/lib32 \
  /opt/chroot/usr/sgug/bin/rpm --version
```

### Method 3: Interactive sgugshell

```bash
/usr/sgug/bin/sgugshell
# Now SGUG commands work without prefixes
rpm --version
tdnf --help
```

---

## Chroot Behavior

### CRITICAL: IRIX chroot Does NOT Fully Isolate

Unlike Linux:
- Process CAN still see `/etc`, `/usr` from base system
- Library loading (`rld`) still searches base system paths
- Symlinks pointing outside chroot are followed

**This means:**
- `/opt/chroot/tmp/foo.txt` is NOT accessible as `/tmp/foo.txt` inside chroot
- Must use full paths or set up symlinks on base system

### Required Symlinks on Base System

```bash
# On base IRIX (not in chroot):
mkdir -p /etc/yum.repos.d
mkdir -p /etc/tdnf/pluginconf.d
ln -sf /opt/chroot/usr/sgug/lib32/rpm /usr/sgug/lib32/rpm
```

---

## Test Patterns

### Pattern 1: Quick Single Command

```bash
ssh root@192.168.0.81 "/bin/sh -c '/opt/chroot/usr/sgug/bin/sgug-exec /opt/chroot/usr/sgug/bin/rpm --version'"
```

### Pattern 2: Script-Based Test

```bash
# Create test script locally
cat > /tmp/test.sh << 'EOF'
#!/bin/sh
/opt/chroot/usr/sgug/bin/sgug-exec /opt/chroot/usr/sgug/bin/tdnf -c /opt/chroot/etc/tdnf/tdnf.conf --installroot=/opt/chroot repolist
echo "Exit code: $?"
EOF

# Copy and run
scp /tmp/test.sh root@192.168.0.81:/tmp/
ssh root@192.168.0.81 "/bin/sh /tmp/test.sh"
```

### Pattern 3: Interactive Chroot

```bash
ssh root@192.168.0.81
chroot /opt/chroot /bin/sh
export LD_LIBRARYN32_PATH=/usr/sgug/lib32
export PATH=/usr/sgug/bin:$PATH
rpm --version
```

---

## Debugging with par

`par` is IRIX's strace equivalent:

```bash
# Trace system calls (inside chroot)
/usr/sgug/bin/sgug-exec par -s /usr/sgug/bin/tdnf repolist > /tmp/par_out.txt 2>&1

# Copy trace to Linux for analysis
scp root@192.168.0.81:/opt/chroot/tmp/par_out.txt /tmp/
```

### Reading par Output

```
4004mS: open("popt.d", O_RDONLY, ...) = 14      # fd 14 returned
4004mS: fchown(14, 0, 0) OK                      # success
4004mS: utimets("/dev/fd/14", ...) errno = 2    # ENOENT - failed!
```

### Common Errors in par Output

| Error | errno | Meaning | Likely Cause |
|-------|-------|---------|--------------|
| `EINVAL` | 22 | Invalid argument | Unsupported flag passed to syscall (e.g., O_NOFOLLOW) |
| `ENOENT` | 2 | No such file | Missing file/directory, or symlink target doesn't exist |
| `ENOSYS` | 89 | Function not implemented | Syscall not available on IRIX |
| `ENODEV` | 6 | No such device | /dev/fd issue or missing device |

### Patterns to Look For

**O_NOFOLLOW / EINVAL pattern:**
```
open("usr", O_RDONLY, ...) errno = 22 (Invalid argument)
```
IRIX doesn't support O_NOFOLLOW. If you see EINVAL on open(), check if the caller is passing O_NOFOLLOW. Fix: strip the flag in openat-compat.c.

**/dev/fd issues:**
```
utimets("/dev/fd/14", ...) errno = 2 (No such file or directory)
```
IRIX /dev/fd doesn't work the same as Linux. Our futimens() tries to use /dev/fd paths but IRIX utimes() can't handle them. Fix: use filename-based approach instead of fd-based.

**Missing syscall result:**
```
179mS: gettimeofday(0x80c78)
183mS: close(12) OK
```
If a syscall shows no result (no "OK" or "errno"), something crashed or hung. The unusual address (0x80c78 instead of stack address 0x7ffd...) suggests memory corruption.

**ensureDir only goes partway:**
If you see open("/"), open("usr") but never open("sgug") for a path like /usr/sgug/etc/, check for EINVAL on the directory opens - likely O_NOFOLLOW issue.

---

## NEVER Do

1. **NEVER install to /usr/sgug directly** - can break sshd, lock you out
2. **NEVER assume bash syntax** - use POSIX sh
3. **NEVER forget LD_LIBRARYN32_PATH** - binaries won't find libraries
4. **NEVER run rpmbuild on IRIX** - build on Linux, copy RPMs to IRIX
5. **NEVER replace libraries that running services use** - sshd uses libz, libssl
6. **NEVER put files in /tmp on Linux** - put files in /home/edodd/projects/github/unxmaal/mogrix/tmp
7. **NEVER reference paths outside /usr/sgug** - mogrix-converted packages must be fully self-contained under /usr/sgug. Config files must not reference /etc, /var, /lib, or any path outside /usr/sgug. IRIX is old and fragile; reinstalling is painful. We never touch the base IRIX OS filesystem.

### Path Rooting Rule

All paths in mogrix-converted packages must be rooted at `/usr/sgug`:

| Linux Path | SGUG Path |
|------------|-----------|
| `/etc/pki/CA` | `/usr/sgug/etc/pki/CA` |
| `/etc/ssl` | `/usr/sgug/etc/pki/tls` |
| `/var/lib/rpm` | `/usr/sgug/var/lib/rpm` |
| `/etc/tdnf` | `/usr/sgug/etc/tdnf` |

When converting packages, check config files for hardcoded paths and fix them. 

---

## Setting Up a Fresh Chroot

### Creating the tdnf Bundle

On Linux, create a tarball with all required packages:

```bash
cd /tmp && rm -rf tdnf-chroot-bundle && mkdir tdnf-chroot-bundle && cd tdnf-chroot-bundle

# Extract all required packages
for rpm in \
  ~/rpmbuild/RPMS/mips/zlib-ng-compat-*.mips.rpm \
  ~/rpmbuild/RPMS/mips/bzip2-libs-*.mips.rpm \
  ~/rpmbuild/RPMS/mips/popt-*.mips.rpm \
  ~/rpmbuild/RPMS/mips/openssl-libs-*.mips.rpm \
  ~/rpmbuild/RPMS/mips/libxml2-*.mips.rpm \
  ~/rpmbuild/RPMS/mips/libcurl-*.mips.rpm \
  ~/rpmbuild/RPMS/mips/xz-libs-*.mips.rpm \
  ~/rpmbuild/RPMS/mips/lua-libs-*.mips.rpm \
  ~/rpmbuild/RPMS/mips/file-libs-*.mips.rpm \
  ~/rpmbuild/RPMS/mips/sqlite-libs-*.mips.rpm \
  ~/rpmbuild/RPMS/mips/rpm-4.19.*.mips.rpm \
  ~/rpmbuild/RPMS/mips/rpm-libs-*.mips.rpm \
  ~/rpmbuild/RPMS/mips/libsolv-*.mips.rpm \
  ~/rpmbuild/RPMS/mips/tdnf-3.*.mips.rpm \
  ~/rpmbuild/RPMS/mips/tdnf-cli-libs-*.mips.rpm \
  ~/rpmbuild/RPMS/noarch/sgugrse-release-*.noarch.rpm \
; do
  rpm2cpio "$rpm" | cpio -idm 2>/dev/null
done

# Create required directories and config
mkdir -p usr/sgug/etc/{tdnf/vars,dnf/vars,yum/vars,yum.repos.d,rpm}
mkdir -p usr/sgug/var/cache/tdnf
mkdir -p usr/sgug/lib/sysimage/rpm
mkdir -p var/lib
cd var/lib && ln -s ../../usr/sgug/lib/sysimage/rpm rpm && cd ../..

# Create sqlite backend macro
echo "%_db_backend sqlite" > usr/sgug/etc/rpm/macros.sqlite

# Create unversioned library symlinks (needed by libsolvext.so)
cd usr/sgug/lib32
ln -sf libz.so.1 libz.so
cd ../../..

# Create tdnf.conf
cat > usr/sgug/etc/tdnf/tdnf.conf << 'EOF'
[main]
gpgcheck=0
installonly_limit=3
clean_requirements_on_remove=1
repodir=/usr/sgug/etc/yum.repos.d
cachedir=/usr/sgug/var/cache/tdnf
EOF

# Create tarball
tar cvf ../tdnf-chroot-bundle.tar usr var
gzip ../tdnf-chroot-bundle.tar
scp ../tdnf-chroot-bundle.tar.gz root@192.168.0.81:/tmp/
```

### Installing on IRIX

```bash
# Remove old chroot
rm -rf /opt/chroot
mkdir -p /opt/chroot

# Extract base SGUG sysroot (your base image)
# ... your base extraction command ...

# Extract the tdnf bundle
cd /opt/chroot
gunzip -c /tmp/tdnf-chroot-bundle.tar.gz | tar xvf -

# Initialize rpm database (from inside chroot)
chroot /opt/chroot /bin/sh
export LD_LIBRARYN32_PATH=/usr/sgug/lib32
/usr/sgug/bin/rpm --initdb
exit
```

### Testing

Inside the chroot:
```bash
/usr/sgug/bin/sgug-exec /usr/sgug/bin/rpm -qa
/usr/sgug/bin/sgug-exec /usr/sgug/bin/rpm -Uvh --nodeps /tmp/some-package.rpm
/usr/sgug/bin/sgug-exec /usr/sgug/bin/tdnf repolist
```

---

## Creating Repos for IRIX

**IMPORTANT**: Always use `--simple-md-filenames` with createrepo_c.

IRIX tar doesn't support GNU long filename extensions. Without this flag, filenames like `a05ec0b1a1165abf978441a5654b86afd1bd380d19a2dede505e1a7dfd561a02-primary.xml.gz` get corrupted during extraction (the file mode gets appended to the filename).

```bash
# CORRECT - simple filenames that IRIX tar can handle
createrepo_c --simple-md-filenames /path/to/repo

# WRONG - long hash-prefixed filenames will be corrupted on IRIX
createrepo_c /path/to/repo
```

---

## Quick Reference

```bash
# Connect
ssh root@192.168.0.81

# Enter SGUG environment
/usr/sgug/bin/sgugshell

# Run chroot binary from base
/opt/chroot/usr/sgug/bin/sgug-exec /opt/chroot/usr/sgug/bin/CMD

# Copy file to chroot
scp file root@192.168.0.81:/opt/chroot/tmp/

# Check library dependencies
elfdump -Dl /path/to/binary
```
