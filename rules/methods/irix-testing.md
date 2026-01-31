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
ssh root@192.168.0.81 "/bin/sh -c 'LD_LIBRARYN32_PATH=/opt/chroot/usr/sgug/lib32 /opt/chroot/usr/sgug/bin/rpm --version'"
```

### Pattern 2: Script-Based Test

```bash
# Create test script locally
cat > /tmp/test.sh << 'EOF'
#!/bin/sh
export LD_LIBRARYN32_PATH=/opt/chroot/usr/sgug/lib32:/usr/sgug/lib32
/opt/chroot/usr/sgug/bin/tdnf -c /opt/chroot/etc/tdnf/tdnf.conf --installroot=/opt/chroot repolist
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
# Trace system calls
par -s /usr/sgug/bin/sgug-exec /usr/sgug/bin/tdnf repolist > /tmp/par_out.txt 2>&1

# Copy trace to Linux for analysis
scp root@192.168.0.81:/opt/chroot/tmp/par_out.txt /tmp/
```

### Reading par Output

```
4004mS: open("popt.d", O_RDONLY, ...) = 14      # fd 14 returned
4004mS: fchown(14, 0, 0) OK                      # success
4004mS: utimets("/dev/fd/14", ...) errno = 2    # ENOENT - failed!
```

---

## NEVER Do

1. **NEVER install to /usr/sgug directly** - can break sshd, lock you out
2. **NEVER assume bash syntax** - use POSIX sh
3. **NEVER forget LD_LIBRARYN32_PATH** - binaries won't find libraries
4. **NEVER run rpmbuild on IRIX** - build on Linux, copy RPMs to IRIX
5. **NEVER replace libraries that running services use** - sshd uses libz, libssl

---

## Quick Reference

```bash
# Connect
ssh root@192.168.0.81

# Enter SGUG environment
/usr/sgug/bin/sgugshell

# Run chroot binary from base
LD_LIBRARYN32_PATH=/opt/chroot/usr/sgug/lib32 /opt/chroot/usr/sgug/bin/CMD

# Copy file to chroot
scp file root@192.168.0.81:/opt/chroot/tmp/

# Check library dependencies
elfdump -Dl /path/to/binary
```
