# Mogrix Setup Guide

Complete walkthrough from a fresh Ubuntu VM to building your first IRIX package.

**Time estimate:** 1-2 hours (mostly waiting for LLVM to compile).

## Table of Contents

1. [Linux VM Setup](#1-linux-vm-setup)
2. [Claude Code Setup](#2-claude-code-setup)
3. [IRIX Sysroot](#3-irix-sysroot)
4. [Cross-Compilation Toolchain](#4-cross-compilation-toolchain)
5. [Fix IRIX CRT Files](#5-fix-irix-crt-files)
6. [Deploy Cross Environment](#6-deploy-cross-environment)
7. [Build Runtime Objects](#7-build-runtime-objects)
8. [Verify Setup](#8-verify-setup)
9. [IRIX Connection & Testing](#9-irix-connection--testing)

---

## 1. Linux VM Setup

### System Requirements

- Ubuntu 22.04+ (or 24.04)
- 4+ CPU cores
- 16GB+ RAM (LLVM builds are memory-hungry)
- 100GB+ disk (LLVM source + build trees are large)
- Network access to an IRIX machine (default: 192.168.0.81)

### Install System Packages

```bash
sudo apt update
sudo apt install -y \
    build-essential gcc g++ make patch \
    bzip2 xz-utils git wget curl file \
    pkg-config flex bison texinfo \
    python3 python3-setuptools python3-venv python3-pip \
    libgmp-dev libmpc-dev libmpfr-dev \
    cmake ninja-build \
    libz-dev libxml2-dev libedit-dev libncurses-dev \
    rpm rpm2cpio cpio \
    createrepo-c \
    clang-18 \
    jq sqlite3
```

### Install uv (Python Package Manager)

```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
# Restart shell or source ~/.bashrc to get uv in PATH
```

### Create Directory Structure

```bash
# Cross-compilation toolchain
sudo mkdir -p /opt/cross/bin
sudo mkdir -p /opt/irix-sysroot
sudo mkdir -p $MOGRIX_STAGING/{bin,lib32,include}
sudo chown -R $USER:$USER /opt/cross /opt/irix-sysroot $MOGRIX_STAGING_ROOT

# Claude Code jail temp directory
sudo mkdir -p /opt/faketemp
sudo chown $USER:$USER /opt/faketemp

# Mogrix input/output directories
mkdir -p ~/mogrix_inputs/SRPMS
mkdir -p ~/mogrix_outputs/{SRPMS,RPMS,converted,bundles}

# rpmbuild workspace (ephemeral, disposable)
mkdir -p ~/rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS}
mkdir -p ~/rpmbuild/RPMS/mips
```

### Clone Mogrix

```bash
cd ~/projects/github/unxmaal  # or wherever you keep repos
git clone https://github.com/unxmaal/mogrix.git
cd mogrix
uv sync
```

### SSH Keys

Set up SSH key-based auth to the IRIX box:

```bash
# Generate key if needed
ssh-keygen -t ed25519 -f ~/.ssh/irix_key

# Copy to IRIX (as both root and your user)
ssh-copy-id -i ~/.ssh/irix_key root@192.168.0.81
ssh-copy-id -i ~/.ssh/irix_key edodd@192.168.0.81

# Verify
ssh root@192.168.0.81 'uname -a'
```

---

## 2. Claude Code Setup

Mogrix is designed to be driven by Claude Code running inside a systemd sandbox. The sandbox restricts filesystem writes to specific paths while giving Claude full tool access within those boundaries.

### Install Claude Code

Follow the official install instructions at https://docs.anthropic.com/en/docs/claude-code/getting-started

### Create the Sandbox Launcher

Save this as `~/bin/mogrix-claude.sh` (or anywhere on your PATH):

```bash
#!/bin/bash
# mogrix-claude.sh — Launch Claude Code in a systemd sandbox
SESSION="${1:-$(claude sessions list --json 2>/dev/null | jq -r '.[0].id // empty')}"
RESUME_FLAG=""
[[ -n "$SESSION" ]] && RESUME_FLAG="--resume $SESSION"

cd /home/$USER/projects/github/unxmaal/mogrix || exit 1
exec systemd-run --user --pty \
  --property=ProtectSystem=strict \
  --property=ProtectHome=no \
  --property=ReadWritePaths=/src \
  --property=ReadWritePaths=/opt/cross \
  --property=ReadWritePaths=/opt/libdicl \
  --property=ReadWritePaths=$MOGRIX_STAGING_ROOT \
  --property=ReadWritePaths=/tmp \
  --property=PrivateDevices=no \
  --property=ProtectKernelTunables=yes \
  --property=ProtectKernelModules=yes \
  --property=ProtectControlGroups=yes \
  --property=LockPersonality=yes \
  --property=MemoryDenyWriteExecute=no \
  --property=RestrictRealtime=yes \
  --property=SystemCallArchitectures=native \
  --property=BindPaths=/opt/faketemp:/tmp \
  --property=UMask=077 \
  --property=WorkingDirectory="$(pwd)" \
  --setenv=HOME="$HOME" \
  claude --dangerously-skip-permissions $RESUME_FLAG
```

```bash
chmod +x ~/bin/mogrix-claude.sh
```

#### What Each Property Does

| Property | Effect |
|----------|--------|
| `ProtectSystem=strict` | Root filesystem is read-only — Claude can't modify `/usr`, `/etc`, etc. |
| `ProtectHome=no` | Home directory is writable (needed for rpmbuild, mogrix_outputs) |
| `ReadWritePaths=...` | Explicitly whitelist directories Claude needs to write to |
| `BindPaths=/opt/faketemp:/tmp` | Isolates `/tmp` — Claude sees `/opt/faketemp` as `/tmp`, can't access host temp files |
| `--dangerously-skip-permissions` | Gives Claude full tool access inside the sandbox (safe because the sandbox restricts what it can actually do) |

### MCP Server Configuration

The `.mcp.json` file in the mogrix repo root configures three MCP servers that Claude uses to interact with IRIX and the knowledge database:

```json
{
  "mcpServers": {
    "irix": {
      "command": "python3",
      "args": ["tools/irix-mcp-server.py"],
      "env": {
        "IRIX_HOST": "192.168.0.81",
        "IRIX_USER": "root",
        "IRIX_CHROOT": "/opt/chroot",
        "IRIX_LOG": "/tmp/irix-mcp.log"
      }
    },
    "mogrix-test": {
      "command": "python3",
      "args": ["tools/mogrix-test-server.py"],
      "env": {
        "IRIX_HOST": "192.168.0.81",
        "IRIX_USER": "root",
        "IRIX_CHROOT": "/opt/chroot",
        "MOGRIX_TEST_LOG": "/tmp/mogrix-test-mcp.log"
      }
    },
    "knowledge": {
      "command": "python3",
      "args": ["tools/knowledge-server.py"],
      "env": {
        "KNOWLEDGE_LOG": "/tmp/knowledge-mcp.log"
      }
    }
  }
}
```

Change `IRIX_HOST` if your IRIX machine is at a different IP address.

### Claude Code Settings

The `.claude/settings.local.json` is minimal:

```json
{
  "spinnerTipsEnabled": false
}
```

---

## 3. IRIX Sysroot

The sysroot contains IRIX system headers and libraries needed for cross-compilation.

### Create the Sysroot Tarball (on IRIX)

Copy `scripts/create-irix-sysroot.sh` to the IRIX machine and run it:

```bash
# On the IRIX machine
sh create-irix-sysroot.sh
# Creates /tmp/irix-sysroot.tar.gz
```

### Transfer and Extract (on Linux)

```bash
scp root@192.168.0.81:/tmp/irix-sysroot.tar.gz /tmp/

cd /opt/irix-sysroot
tar xzf /tmp/irix-sysroot.tar.gz
```

### Verify

```bash
# These must exist:
ls /opt/irix-sysroot/usr/include/stdio.h
ls /opt/irix-sysroot/usr/lib32/libc.so
ls /opt/irix-sysroot/lib32/rld
```

---

## 4. Cross-Compilation Toolchain

Three tools are needed:

1. **Clang** — the C/C++ compiler (any version with MIPS target support)
2. **LLD 18** — the linker (patched for IRIX ELF quirks)
3. **GNU Binutils** — objcopy, readelf, BFD ld fallback

### Clang (the Compiler)

Mogrix uses upstream clang targeting `mips-sgi-irix6.5`. The IRIX-specific behavior is in the `irix-cc` wrapper script, not in clang itself.

**Current working version:** clang 16.0.6 (built from upstream LLVM 16 source).

**Option A: Build from source** (recommended — guarantees MIPS target is enabled):

```bash
# Download LLVM 16 source
wget https://github.com/llvm/llvm-project/releases/download/llvmorg-16.0.6/llvm-project-16.0.6.src.tar.xz
tar xf llvm-project-16.0.6.src.tar.xz
cd llvm-project-16.0.6.src

mkdir build && cd build
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_PROJECTS="clang" \
    -DLLVM_TARGETS_TO_BUILD="Mips;X86" \
    -DCMAKE_INSTALL_PREFIX=/opt/cross \
    ../llvm

ninja
sudo ninja install
```

This installs `clang`, `clang++`, `llvm-ar`, `llvm-nm`, `llvm-objdump`, etc. to `/opt/cross/bin/`.

**Option B: System package** (if your distro provides clang with MIPS support):

```bash
# Check if the system clang supports MIPS
clang-16 --print-targets 2>/dev/null | grep -i mips

# If yes, symlink it
sudo ln -sf /usr/bin/clang-16 /opt/cross/bin/clang
sudo ln -sf /usr/bin/clang++-16 /opt/cross/bin/clang++
```

You also need `llvm-ar` in `/opt/cross/bin/`:

```bash
sudo ln -sf /usr/bin/llvm-ar-16 /opt/cross/bin/llvm-ar
```

### LLD 18 (the Linker)

LLD 18 is patched to handle IRIX ELF specifics (no MIPS_ABIFLAGS, no PT_GNU_STACK, correct shared library phdrs). A build script is provided:

```bash
cd mogrix
./scripts/build-lld-irix.sh
```

Prerequisites: `cmake`, `ninja-build`, `clang-18` (system package, for building LLD itself).

After the build completes:

```bash
# Create the default symlink
sudo ln -sf ld.lld-irix-18 /opt/cross/bin/ld.lld-irix
```

### GNU Binutils (objcopy, readelf, BFD ld)

Build binutils 2.41 targeting `mips-sgi-irix6.5`:

```bash
wget https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.xz
tar xf binutils-2.41.tar.xz
cd binutils-2.41

mkdir build && cd build
../configure \
    --prefix=/opt/cross \
    --target=mips-sgi-irix6.5 \
    --disable-werror

make -j$(nproc)
sudo make install
```

This provides:
- `mips-sgi-irix6.5-objcopy` — used to fix IRIX CRT files (next step)
- `mips-sgi-irix6.5-readelf` — ELF inspection
- `mips-sgi-irix6.5-ld.bfd` — fallback linker for shared libraries needing 2-segment layout

---

## 5. Fix IRIX CRT Files

IRIX's `crt1.o` contains `.MIPS.events*` sections that LLD can't process. Strip them:

```bash
mkdir -p /opt/irix-sysroot/usr/lib32/mips3/fixed

/opt/cross/bin/mips-sgi-irix6.5-objcopy \
    -R .MIPS.events.text \
    -R .MIPS.events.init \
    -R .MIPS.events \
    /opt/irix-sysroot/usr/lib32/mips3/crt1.o \
    /opt/irix-sysroot/usr/lib32/mips3/fixed/crt1.o

cp /opt/irix-sysroot/usr/lib32/mips3/crtn.o \
   /opt/irix-sysroot/usr/lib32/mips3/fixed/crtn.o
```

---

## 6. Deploy Cross Environment

```bash
cd mogrix
uv run mogrix setup-cross
```

This deploys:
- `irix-cc`, `irix-ld`, `irix-cxx` wrapper scripts to `$MOGRIX_STAGING/bin/`
- Compat headers to `$MOGRIX_STAGING/include/`
- RPM macros to `$MOGRIX_STAGING_ROOT/rpmmacros.irix`

It does NOT build the runtime objects — that's the next step.

---

## 7. Build Runtime Objects

```bash
./scripts/build-runtime-objects.sh
```

This builds all runtime objects that must exist in staging before any package can be cross-compiled:

| Object | Source | Purpose |
|--------|--------|---------|
| `crtbeginS.o` | `cross/crt/crtbeginS.S` | PIC C++ constructor support (shared libs) |
| `crtendS.o` | `cross/crt/crtendS.S` | PIC C++ destructor support (shared libs) |
| `crtbeginT.o` | `cross/crt/crtbeginT.S` | Non-PIC constructor support (executables) |
| `crtendT.o` | `cross/crt/crtendT.S` | Non-PIC destructor support (executables) |
| `dso_handle.o` | `cross/lib/dso_handle.c` | `__dso_handle` for `__cxa_atexit` |
| `safe_mem.o` | `cross/lib/safe_mem.c` | Byte-safe memcmp/strcmp (prevents overread SIGSEGV) |
| `dlmalloc.o` | `compat/malloc/dlmalloc.c` | mmap-based allocator (executables only) |
| `libsoft_float_stubs.a` | `compat/runtime/soft_float_stubs.c` | 128-bit soft float stubs |
| `libatomic.a` | `compat/runtime/libatomic_stub.c` | Atomic operation stubs |
| `libcompat.a` | `compat/runtime/*.c` | Missing POSIX functions (stpcpy, posix_spawn, etc.) |
| `libmogrix_compat.so` | `compat/` (multiple) | Preloaded override for buggy IRIX libc functions |
| `irix-shared.lds` | `cross/lib/irix-shared.lds` | BFD ld linker script (2-segment layout) |
| `crt-hide.ver` | `cross/crt/crt-hide.ver` | Version script hiding CRT symbols |

All objects are deployed to `$MOGRIX_STAGING/lib32/`.

---

## 8. Verify Setup

### Check Toolchain

```bash
/opt/cross/bin/clang --version
/opt/cross/bin/mips-sgi-irix6.5-ld.bfd --version
/opt/cross/bin/ld.lld-irix --version
```

### Check Sysroot

```bash
ls /opt/irix-sysroot/usr/include/stdio.h
ls /opt/irix-sysroot/usr/lib32/libc.so
```

### Check Staging

```bash
ls $MOGRIX_STAGING/bin/irix-cc
ls $MOGRIX_STAGING/lib32/libsoft_float_stubs.a
ls $MOGRIX_STAGING/lib32/dlmalloc.o
ls $MOGRIX_STAGING/lib32/libmogrix_compat.so
```

### Test Cross-Compilation

```bash
echo 'int main() { return 0; }' > /tmp/test.c
$MOGRIX_STAGING/bin/irix-cc /tmp/test.c -o /tmp/test
file /tmp/test
# Expected: ELF 32-bit MSB executable, MIPS, N32 MIPS-III version 1 (SYSV), dynamically linked
```

### Build a Test Package

```bash
# Fetch and build popt (small, no dependencies)
uv run mogrix fetch popt -y
uv run mogrix convert ~/mogrix_inputs/SRPMS/popt-*.src.rpm
uv run mogrix build ~/mogrix_outputs/SRPMS/popt-*/popt-*.src.rpm --cross

# Check the output
file ~/rpmbuild/RPMS/mips/popt-*.rpm
```

---

## 9. IRIX Connection & Testing

### Network

The IRIX machine must be reachable from the Linux build host. Default address is `192.168.0.81`. Change the `IRIX_HOST` variable in `.mcp.json` if your machine is at a different address.

### SSH Access

- **Root access** is needed for the chroot environment (where cross-compiled N32 binaries run)
- **User access** (edodd) is used for running N64 Go binaries on the host

### MCP Tools

Three MCP servers provide IRIX access from within Claude Code:

| Server | Tools | Purpose |
|--------|-------|---------|
| `irix` | `irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par` | Execute commands in chroot, transfer files, read files, syscall tracing |
| `mogrix-test` | `test_bundle`, `test_binary`, `check_deps`, `par_trace`, `screenshot` | Test bundles, check binaries, verify dependencies |
| `knowledge` | `knowledge_query`, `report_error`, `check_compat`, `add_rule` | Knowledge DB lookup, error tracking, compat function catalog |

### Quick Test (from Claude Code)

```
# Via MCP (in a Claude session)
irix_exec "echo ok"

# Via CLI (outside Claude)
tools/irix-exec.sh "echo ok"
```

### Bundle Testing Workflow

```bash
# 1. Build the package
uv run mogrix build ~/mogrix_outputs/SRPMS/nano-*/nano-*.src.rpm --cross

# 2. Stage it
uv run mogrix stage ~/rpmbuild/RPMS/mips/nano-*.rpm

# 3. Create a bundle
uv run mogrix bundle nano

# 4. Test the bundle on IRIX (from Claude Code)
# test_bundle bundle_path=/path/to/nano-*.run
```

---

## Troubleshooting

### "irix-cc not found"

Run `uv run mogrix setup-cross` to deploy the compiler wrappers.

### "No such file: /opt/irix-sysroot/usr/include/stdio.h"

The IRIX sysroot hasn't been extracted. See [Section 3](#3-irix-sysroot).

### "ld.lld-irix: command not found"

LLD hasn't been built or symlinked. See [Section 4](#4-cross-compilation-toolchain).

### "undefined reference to __dso_handle / dlmalloc / safe_mem"

Runtime objects haven't been built. Run `./scripts/build-runtime-objects.sh`.

### Cross-compilation produces x86 binaries

Check that `irix-cc` is using the correct clang. Run:
```bash
$MOGRIX_STAGING/bin/irix-cc --version
```
It should show `Target: mips-sgi-irix6.5`.
