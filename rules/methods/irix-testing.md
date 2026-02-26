# IRIX Testing Methods

> **For AI agents**: READ THIS before running commands on IRIX.
> IRIX shell and environment differ significantly from Linux.

---

## Connection

| Property | Value |
|----------|-------|
| Hostname | `blue` / `192.168.0.81` |
| IRIX version | 6.5.30 |
| Machine | SGI Octane (IP30) |
| Chroot | `/opt/chroot` |
| SGUG-RSE | `/usr/sgug` (existing rpm 4.15.0) |

### Always Use MCP Tools

**NEVER use raw SSH to IRIX.** Use MCP tools for all IRIX interaction:

| Tool | Purpose |
|------|---------|
| `irix_exec "command"` | Run a command in the chroot |
| `irix_host_exec "command"` | Run a safe command on the live host (outside chroot) |
| `irix_copy_to local remote` | Copy a file to IRIX chroot |
| `irix_read_file path` | Read a file from IRIX chroot |
| `irix_par "command"` | Run par (syscall trace) on a command |

`irix_host_exec` has a command allowlist (ls, tar, cp, mkdir, etc.) and blocks writes to system paths. Use it for bundle extraction, inspecting the live system, or managing user directories.

**Fallback** (if MCP is down): `tools/irix-exec.sh "command"` — uses the same chroot+sgug-exec pattern via SSH.

Test MCP at session start: `irix_exec "echo ok"`

### mogrix-test MCP Server (Test Harness)

A separate MCP server (`tools/mogrix-test-server.py`) provides structured testing tools. Results are stored as JSON in `test-results/<package>.json`.

| Tool | Purpose |
|------|---------|
| `test_bundle bundle_path` | Deploy a `.run` bundle to IRIX, run all tests (auto `--version` + YAML smoke_tests), return pass/fail report |
| `test_binary binary [args]` | Run a single binary on IRIX. Returns exit code, stdout, stderr, crash/signal info |
| `check_deps path` | Check library dependencies WITHOUT running. Reports each NEEDED soname and resolution status |
| `par_trace command` | Run command under `par` (syscall tracer) with intelligent output parsing |
| `test_report [package]` | Return stored test results. If package given, full results; if omitted, summary of all |
| `screenshot [delay]` | Capture the IRIX X11 display as PNG. Use Read tool to view the image |

**Key differences from irix MCP tools:**

- `test_bundle` deploys to `/tmp/mogrix-test/` on the IRIX **host** (not chroot). Bundle wrappers set their own `LD_LIBRARYN32_PATH`, so running through chroot would pollute the environment.
- `test_binary` supports both chroot mode (default, for installed packages) and `host_mode` (for bundle binaries).
- `screenshot` captures the X11 display via `xwd` on IRIX, transfers via SCP, converts to PNG on Linux.

**Typical testing workflow:**

```bash
# 1. Full bundle test (auto-discovers binaries, runs smoke tests from YAML)
test_bundle {"bundle_path": "/path/to/package.run"}

# 2. Quick ad-hoc binary check
test_binary {"binary": "/usr/sgug/bin/grep", "args": "--version"}

# 3. Check library deps before deploying
check_deps {"path": "/path/to/bundle_dir"}

# 4. Debug a crash with parsed syscall trace
par_trace {"command": "/usr/sgug/bin/failing-binary"}

# 5. GUI app testing: launch + screenshot
test_binary {"binary": "/path/to/gui-app", "args": "-display :0", "env": {"DISPLAY": ":0"}, "timeout": 5, "host_mode": true}
screenshot {"delay": 3}

# 6. View all test results
test_report {}
```

**Configuration:** Defined in `.mcp.json` under `mogrix-test`. Uses its own SSH connection (separate from the irix MCP server).

---

## Shell Rules

### IRIX Default Shell is csh

The `irix_exec` MCP tool runs commands through the chroot shell, which may be csh. **Do not use heredocs, `export`, or bash syntax in `irix_exec` commands.** Use `setenv VAR val` instead of `export VAR=val`. For writing files, write locally to `/tmp/` then use `irix_copy_to` + `irix_host_exec "cp ..."`.

**`irix_copy_to` copies into the chroot, NOT the host.** To get a file onto the live IRIX filesystem, use `irix_copy_to` with `host_path=true` and `owner=edodd`:

```
irix_copy_to local_path=/tmp/file remote_path=/usr/people/edodd/file host_path=true owner=edodd
```

**NEVER SCP/copy files as root then expect edodd to run them.** Root-owned files may not be executable by edodd. Always transfer as edodd or set owner=edodd.

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

### Pattern 1: Quick Single Command (MCP)

```bash
# Via MCP tool (preferred)
irix_exec "rpm --version"

# Via fallback script
tools/irix-exec.sh "rpm --version"
```

### Pattern 2: Copy File and Test (MCP)

```bash
# Copy RPM to IRIX chroot
irix_copy_to /path/to/package.rpm /tmp/package.rpm

# Install and test
irix_exec "rpm -Uvh --nodeps /tmp/package.rpm"
irix_exec "/usr/sgug/bin/some-binary --version"
```

### Pattern 3: Read Files from IRIX (MCP)

```bash
# Read a config file or log
irix_read_file /usr/sgug/etc/tdnf/tdnf.conf
```

### Pattern 4: Syscall Tracing (MCP)

```bash
# Trace a command with par
irix_par "/usr/sgug/bin/some-binary --version"
```

Note: All MCP tools run inside the chroot with sgug-exec — LD_LIBRARYN32_PATH and PATH are set automatically.

---

## Debugging with par

`par` is IRIX's strace equivalent:

```bash
# Trace system calls (via MCP)
irix_par "/usr/sgug/bin/tdnf repolist"

# Or manually with output redirect:
irix_exec "par -s /usr/sgug/bin/tdnf repolist > /tmp/par_out.txt 2>&1"

# Read trace output on Linux for analysis
irix_read_file /tmp/par_out.txt
```

### Par Trace File Locations

When par traces are run (by user or agent), outputs are stored at predictable locations:

| Location | Path |
|----------|------|
| **IRIX host** | `~edodd/par_<test_app_name>.txt` |
| **Linux build host** | `/home/edodd/projects/github/unxmaal/mogrix/par_<test_app_name>.txt` |

The user may run par manually and SCP the output to the Linux path. Always check for existing par files with `Read` before requesting a new trace.

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

## Debugging with Crash Handler (mogrix_crash_handler)

For crashes that produce no stderr output (common when subprocesses are launched by GSubprocess, which may swallow stderr), use the built-in crash diagnostic handler. It writes crash reports to **log files** — not just stderr.

### How It Works

`mogrix_crash_handler.c` in `patches/shared/` installs signal handlers via `__attribute__((constructor))`. It's compiled into `libmogrix_compat.so`, which every bundle preloads via `_RLDN32_LIST`. When a caught signal fires, it writes a detailed crash report (signal, PC, RA, registers, resolved stack backtrace via `_rld_new_interface`) to a file.

### Signals Caught

SIGSEGV, SIGBUS, SIGABRT, SIGFPE, **SIGPIPE**, SIGILL, SIGTRAP

### Enabling

```bash
export MOGRIX_CRASH_DEBUG=1
export MOGRIX_CRASH_DIR=/usr/people/edodd  # optional, default: cwd

# Then run your bundle normally
$BUNDLE/libexec/webkit2gtk-4.0/MiniBrowser about:blank
```

All subprocesses (WebProcess, NetworkProcess, etc.) inherit the env vars and get crash diagnostics automatically.

### Output Files

| File | Meaning |
|------|---------|
| `mogrix_init_<pid>.log` | Handler loaded and installed — confirms preload worked |
| `mogrix_crash_<pid>.log` | **CRASH** — full diagnostic: signal, PC, RA, registers, stack backtrace |
| `mogrix_exit_<pid>.log` | Clean exit (no signal) — process called `exit()` |
| (no file for PID) | Library never loaded, or process killed by SIGKILL (uncatchable) |

### Reading a Crash Log

```
Signal: SIGPIPE (broken pipe)
PID: 12345
-- Crash Location (resolved via rld) -----------------
  PC : 0x0fa12340 in libgio-2.0.so [base 0x0fa00000, offset +0x00012340]
         nearest symbol: g_output_stream_write at 0x0fa12300 (+64)
  RA : 0x10abcde0 in libwebkit2gtk-4.0.so [...]
```

Key fields: **Signal** (what killed it), **PC** (where it crashed), **RA** (who called the crashing function), **Stack Backtrace** (resolved call chain).

### Updating libmogrix_compat.so

After editing `patches/shared/mogrix_crash_handler.c`:

```bash
irix-cc -shared -fPIC -o /tmp/libmogrix_compat.so \
    compat/stdlib/bsearch.c \
    compat/sys/socketpair.c \
    compat/sys/mincore.c \
    compat/runtime/muloti4.c \
    compat/runtime/divti3.c \
    compat/stdlib/mkdtemp.c \
    compat/error/strerror_r.c \
    compat/string/memmem.c \
    patches/shared/mogrix_crash_handler.c \
    -I compat/include -I patches/shared

cp /tmp/libmogrix_compat.so /opt/sgug-staging/usr/sgug/lib32/libmogrix_compat.so
```

To hot-deploy to an existing bundle without rebundling:
```bash
irix_copy_to /tmp/libmogrix_compat.so /tmp/libmogrix_compat.so
irix_host_exec "cp /opt/chroot/tmp/libmogrix_compat.so <bundle>/_lib32/libmogrix_compat.so"
```

### Alternative: Compile Into a Specific Package

For packages where you want the crash handler even without preload:

```yaml
add_source:
  - mogrix_crash_handler.c
  - mogrix_crash_handler.h
prep_commands:
  - "cp %{_sourcedir}/mogrix_crash_handler.c src/"
  - "cp %{_sourcedir}/mogrix_crash_handler.h src/"
  # Add to the package's build system (meson, cmake, etc.)
```

See `rules/packages/gtkterm.yaml` for a working example.

---

## NEVER Do

1. **NEVER install to /usr/sgug directly** - can break sshd, lock you out
2. **NEVER assume bash syntax** - use POSIX sh
3. **NEVER forget LD_LIBRARYN32_PATH** - binaries won't find libraries
4. **NEVER run rpmbuild on IRIX** - build on Linux, copy RPMs to IRIX
5. **NEVER replace libraries that running services use** - sshd uses libz, libssl
6. **NEVER put files in /tmp on Linux** - put files in /home/edodd/projects/github/unxmaal/mogrix/tmp
7. **NEVER reference paths outside /usr/sgug** - mogrix-converted packages must be fully self-contained under /usr/sgug. Config files must not reference /etc, /var, /lib, or any path outside /usr/sgug. IRIX is old and fragile; reinstalling is painful. We never touch the base IRIX OS filesystem.
8. **NEVER write ad-hoc wrapper scripts for IRIX testing** — use the mogrix-test MCP tools (`test_bundle`, `test_binary`, `par_trace`, `screenshot`). They handle permissions, paths, environment, and output capture correctly. Writing one-off scripts leads to permission errors, wrong paths, and wasted debugging time.
9. **NEVER assume `/tmp` is writable on IRIX** — the `edodd` user cannot write to `/tmp`. Use `/usr/people/edodd/tmp/` for user-writable temp files on the IRIX host.
10. **NEVER forget bundle file permissions** — extracted bundles may be owned by root and unreadable by `edodd`. Use `chown -Rh edodd` (NOT `-R`) because bundles contain symlinks and `-R` follows them, failing on broken symlink targets with ENOENT. The mogrix-test MCP server handles this correctly; ad-hoc testing does not.
11. **NEVER copy files to IRIX as root then expect edodd to run them** — use `irix_copy_to` with `host_path=true, owner=edodd` to transfer directly to edodd's home with correct ownership. Or SCP as edodd: `scp file edodd@192.168.0.81:/usr/people/edodd/file`.

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

# Copy to IRIX (via MCP or fallback)
irix_copy_to ../tdnf-chroot-bundle.tar.gz /tmp/tdnf-chroot-bundle.tar.gz
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

## IRIX tar Limitations

IRIX tar is NOT GNU tar. Critical differences:

| Feature | IRIX tar | GNU tar |
|---------|----------|---------|
| `-C dir` (change directory) | **Not supported** — silently ignored | Works |
| `-z` (gzip) | **Not supported** | Works |
| GNU/pax extensions | **Silently fails** — extracts nothing | Works |
| Long filenames (>100 chars) | Corrupts (appends mode to name) | Works |

### Creating tarballs for IRIX

Always use `--format=v7` when creating tarballs on Linux for IRIX:

```bash
# CORRECT — v7 format that IRIX tar understands
tar --format=v7 -cf bundle.tar bundle-dir/
gzip bundle.tar

# On IRIX — must gunzip separately, no -C flag
gunzip bundle.tar.gz
tar xf bundle.tar           # extracts to cwd!
```

### Creating repos for IRIX

Always use `--simple-md-filenames` with createrepo_c:

```bash
# CORRECT - simple filenames that IRIX tar can handle
createrepo_c --simple-md-filenames /path/to/repo

# WRONG - long hash-prefixed filenames will be corrupted on IRIX
createrepo_c /path/to/repo
```

---

## Quick Reference

```bash
# Run a command (MCP — preferred)
irix_exec "command"

# Copy a file to IRIX (MCP)
irix_copy_to local_path /remote/path

# Read a file from IRIX (MCP)
irix_read_file /path/on/irix

# Syscall trace (MCP)
irix_par "command"

# Fallback (if MCP is down)
tools/irix-exec.sh "command"

# Check library dependencies
irix_exec "elfdump -Dl /path/to/binary"
```
