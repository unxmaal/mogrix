# Mogrix

TransMOGrifies Linux SRPMs into IRIX-compatible packages.

## Overview

Mogrix is a complete IRIX cross-compilation system that transforms Fedora SRPMs into working IRIX packages. It handles the entire pipeline from SRPM fetch through cross-compilation to deployable RPMs.

**Target Platform:** SGI IRIX 6.5.x running on MIPS processors (O2, Octane, Origin, Fuel, Tezro). Builds use the N32 ABI (MIPS III instruction set).

**Background:** SGI's IRIX was discontinued in 2006, leaving users with aging software. [SGUG-RSE](https://github.com/sgidevnet/sgug-rse) (SGI Users Group Rebuilt Software Environment) provides modern open-source packages for IRIX, but its build process requires native IRIX compilation. Mogrix enables cross-compilation from Linux, making it faster and more accessible to build IRIX packages.

**Core Capabilities:**

- **IRIX Cross-Compilation** - Full toolchain integration with clang/LLD targeting MIPS N32 ABI. The `mogrix build --cross` command invokes rpmbuild with IRIX-specific compiler wrappers, header paths, and linker settings.

- **SRPM Conversion Engine** - YAML-based rules transform spec files for IRIX compatibility: drops unavailable dependencies, injects compat functions, applies platform-specific patches, and configures autoconf/cmake for cross-compilation.

- **Compat Library** - Drop-in implementations of missing POSIX/C99 functions (openat family, posix_spawn, vasprintf, funopen, etc.) automatically injected into packages that need them.

- **Staging System** - Cross-compiled packages are staged to provide headers and libraries for dependent builds, enabling complex dependency chains like the 13-package tdnf stack.

- **Bootstrap Tarball** - Self-contained tarball generator for deploying the complete tdnf package manager to IRIX without requiring any tools on the target system.

## Installation

```bash
# Clone the repository
git clone https://github.com/unxmaal/mogrix.git
cd mogrix

# Create virtual environment and install
make venv
source .venv/bin/activate
make install
```

## Quick Start

### Standard SRPM Workflow (Recommended)

The recommended workflow operates on SRPMs directly. Direct spec file editing should be avoided.

```bash
# 1. One-time setup of cross-compilation environment
mogrix setup-cross

# 2. Fetch SRPM from Fedora (downloads to current directory by default)
mogrix fetch popt

# 3. Convert SRPM (creates -converted/ directory next to input)
mogrix convert popt-1.19-6.fc40.src.rpm

# 4. Cross-compile for IRIX (outputs to ~/rpmbuild/RPMS/mips/)
mogrix build popt-1.19-6.fc40.src-converted/popt-*.src.rpm --cross

# 5. Stage for dependent builds
mogrix stage ~/rpmbuild/RPMS/mips/popt*.rpm
```

#### Alternative: Organize in ~/rpmbuild/SRPMS/

```bash
# Fetch to ~/rpmbuild/SRPMS/
mogrix fetch popt -o ~/rpmbuild/SRPMS/

# Convert (creates ~/rpmbuild/SRPMS/popt-*-converted/)
mogrix convert ~/rpmbuild/SRPMS/popt-1.19-6.fc40.src.rpm

# Build and stage
mogrix build ~/rpmbuild/SRPMS/popt-1.19-6.fc40.src-converted/popt-*.src.rpm --cross
mogrix stage ~/rpmbuild/RPMS/mips/popt*.rpm
```

### Analyze an SRPM

```bash
# See what rules would apply to a package
mogrix analyze package-1.0-1.fc40.src.rpm
```

### Batch Conversion

```bash
# Convert multiple SRPMs at once
mogrix batch srpms_directory/ output_directory/
```

### Build a Converted Package

```bash
# Cross-compile for IRIX (primary use case)
mogrix build converted-package.src.rpm --cross

# Dry-run to see cross-compilation command
mogrix build converted-package.src.rpm --cross --dry-run

# Native rpmbuild (for testing on Linux)
mogrix build converted-package.src.rpm
```

### Stage Cross-Compiled RPMs

After cross-compiling packages, stage them to make their headers and libraries available for building dependent packages:

```bash
# Stage RPMs to /opt/sgug-staging
mogrix stage ~/rpmbuild/RPMS/mips/popt-*.rpm

# Stage to custom location
mogrix stage ~/rpmbuild/RPMS/mips/*.rpm --staging-dir /my/staging

# List what's staged
mogrix stage --list

# Clean staging area
mogrix stage --clean
```

The staging area extracts RPM contents so that subsequent builds can find headers (`-I/opt/sgug-staging/usr/sgug/include`) and libraries (`-L/opt/sgug-staging/usr/sgug/lib32`).

### Cross-Compilation Setup

The `--cross` flag enables IRIX cross-compilation. This section describes setting up the build environment on Ubuntu.

#### Prerequisites

Ubuntu 22.04+ with build dependencies:

```bash
sudo apt install -y \
    build-essential gcc g++ make patch \
    bzip2 xz-utils git wget curl file \
    pkg-config flex bison texinfo \
    python3 python3-setuptools python3-venv \
    libgmp-dev libmpc-dev libmpfr-dev \
    cmake ninja-build \
    libz-dev libxml2-dev libedit-dev libncurses-dev \
    rpm
```

#### Directory Structure

```bash
sudo mkdir -p /opt/cross/bin /opt/irix-sysroot /opt/sgug-staging/usr/sgug/{bin,lib32,include}
sudo chown -R $USER:$USER /opt/cross /opt/irix-sysroot /opt/sgug-staging
```

| Path | Purpose |
|------|---------|
| `/opt/cross/bin/` | Cross-compilation toolchain |
| `/opt/irix-sysroot/` | IRIX system headers and libraries |
| `/opt/sgug-staging/usr/sgug/` | Staging area for cross-compiled packages |

#### 1. IRIX Sysroot

Extract IRIX system files from an IRIX 6.5.30 installation:

```bash
# From a tarball of IRIX root filesystem
tar xjf irix-root.6.5.30.tar.bz2 -C /opt/irix-sysroot

# The sysroot should contain:
# /opt/irix-sysroot/usr/include/  - System headers
# /opt/irix-sysroot/usr/lib32/    - System libraries
# /opt/irix-sysroot/lib32/        - Core libraries (libc, rld, etc.)
```

#### 2. LLVM/Clang from vvuk's Fork

Vladimir Vukicevic's LLVM fork includes IRIX/MIPS support for both clang and LLD:

```bash
# Clone vvuk's LLVM with IRIX support
git clone https://github.com/vvuk/llvm-project.git
cd llvm-project
git checkout irix/14.x

# Build clang and LLD
mkdir build && cd build
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DLLVM_ENABLE_PROJECTS="clang;lld" \
    -DLLVM_TARGETS_TO_BUILD="Mips;X86" \
    -DCMAKE_INSTALL_PREFIX=/opt/cross \
    ../llvm

ninja
ninja install
```

This provides in `/opt/cross/bin/`:
- `clang`, `clang++` - C/C++ compiler with MIPS/IRIX target
- `llvm-ar`, `llvm-ranlib`, `llvm-strip`, `llvm-nm`, `llvm-objdump`
- `lld`, `ld.lld` - LLVM linker with IRIX support

Rename the IRIX-capable LLD:
```bash
cp /opt/cross/bin/ld.lld /opt/cross/bin/ld.lld-irix
```

#### 3. GNU Binutils for IRIX

Build binutils 2.23.2 targeting `mips-sgi-irix6.5`:

```bash
# Download and patch binutils
wget https://ftp.gnu.org/gnu/binutils/binutils-2.23.2.tar.bz2
tar xjf binutils-2.23.2.tar.bz2
cd binutils-2.23.2
# Apply IRIX patches if needed

# Configure and build
mkdir build && cd build
../configure \
    --prefix=/opt/cross \
    --target=mips-sgi-irix6.5 \
    --disable-werror \
    --with-sysroot=/opt/irix-sysroot

make -j$(nproc)
make install
```

This provides:
- `mips-sgi-irix6.5-ld.bfd` - GNU ld (critical for shared library layout)
- `mips-sgi-irix6.5-ar`, `mips-sgi-irix6.5-ranlib`, etc.

#### 4. Fix IRIX CRT Files

IRIX's `crt1.o` contains sections that LLD can't handle. Strip them:

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

#### 5. Deploy Mogrix Cross-Compilation Environment

```bash
cd mogrix
source .venv/bin/activate
mogrix setup-cross
```

This deploys:
- `irix-cc`, `irix-ld` wrapper scripts to `/opt/sgug-staging/usr/sgug/bin/`
- Compat headers to `/opt/sgug-staging/usr/sgug/include/`
- RPM macros to `/opt/sgug-staging/rpmmacros.irix`

#### 6. Build Runtime Libraries

```bash
./cleanup.sh
```

This script resets the staging environment and rebuilds all runtime libraries:
- Cleans `staging/lib32/` and `staging/include/`
- Restores compat headers from `cross/include/` and `compat/include/`
- Compiles runtime libraries:
  - `libsoft_float_stubs.a` - Soft float ABI stubs
  - `libatomic.a` - Atomic operation stubs
  - `libcompat.a` - Missing POSIX functions (stpcpy, posix_spawn, etc.)
- Cleans `~/rpmbuild/BUILD/`, `BUILDROOT/`, and `RPMS/mips/`

Run this before starting a fresh build chain or when you need to reset the staging environment.

#### Verify Setup

```bash
# Check toolchain
/opt/cross/bin/clang --version
/opt/cross/bin/mips-sgi-irix6.5-ld.bfd --version
/opt/cross/bin/ld.lld-irix --version

# Check sysroot
ls /opt/irix-sysroot/usr/include/stdio.h
ls /opt/irix-sysroot/usr/lib32/libc.so

# Check staging
ls /opt/sgug-staging/usr/sgug/bin/irix-cc
ls /opt/sgug-staging/usr/sgug/lib32/libsoft_float_stubs.a

# Test cross-compilation
echo 'int main() { return 0; }' > /tmp/test.c
/opt/sgug-staging/usr/sgug/bin/irix-cc /tmp/test.c -o /tmp/test
file /tmp/test  # Should show: ELF 32-bit MSB executable, MIPS
```

## CLI Commands

| Command | Description |
|---------|-------------|
| `mogrix setup-cross` | One-time cross-compilation environment setup |
| `mogrix fetch <packages...>` | Fetch SRPMs from Fedora repositories |
| `mogrix convert <srpm>` | Convert SRPM (applies IRIX rules, repackages) |
| `mogrix build <srpm> --cross` | Cross-compile converted SRPM for IRIX |
| `mogrix stage <rpms...>` | Stage cross-compiled RPMs for dependency resolution |
| `mogrix analyze <srpm>` | Analyze and show what rules would apply |
| `mogrix batch <dir> <out>` | Convert multiple SRPMs in batch |
| `mogrix list-rules` | List available package rules |
| `mogrix validate-rules` | Validate all rule files |

## Dependency Handling

When building a converted package, mogrix detects missing dependencies and offers to fetch them:

```bash
# Build will detect missing deps and offer to fetch them
mogrix build converted-package.src.rpm

# Or manually fetch SRPMs from Fedora archives
mogrix fetch popt zlib curl

# Search shows matches and prompts for confirmation
mogrix fetch zlib
# Found 2 matching packages:
#   [1] zlib-ada-1.4-0.37.fc40.src.rpm
#   [2] zlib-ng-2.1.6-2.fc40.src.rpm
# Select [1]:

# Auto-confirm single matches with -y
mogrix fetch popt -y

# Fetch from Photon OS repos (for tdnf, etc.)
mogrix fetch tdnf --base-url photon5

# Available presets: photon5, photon4, photon3
# Or use a custom URL (ending with / for flat directories)
mogrix fetch popt --base-url http://my-mirror.example.com/srpms/
```

The build command will:
1. Detect missing build dependencies
2. Categorize them (system packages, packages with rules, unsupported)
3. Offer to fetch packages that have mogrix conversion rules
4. Download SRPMs and show next steps for converting them

## Creating Package Rules

Package rules are YAML files in `rules/packages/`. Each rule file defines transformations for a specific package.

### Example Rule File

```yaml
# rules/packages/mypackage.yaml
package: mypackage

rules:
  # Inject compat functions (replaces libdicl)
  inject_compat_functions:
    - strdup
    - strndup
    - getline

  # Remove unavailable dependencies
  drop_buildrequires:
    - systemd-devel
    - libselinux-devel

  # Remove runtime dependencies
  drop_requires:
    - systemd

  # Disable configure features
  configure_disable:
    - systemd
    - selinux

  # Set autoconf cache values for cross-compilation
  ac_cv_overrides:
    ac_cv_func_malloc_0_nonnull: "yes"
    ac_cv_func_realloc_0_nonnull: "yes"

  # Remove specific lines from the spec
  remove_lines:
    - 'export LIBS="-lsystemd"'
```

### Available Rule Keys

| Key | Type | Description |
|-----|------|-------------|
| `inject_compat_functions` | list | Compat functions to inject (strdup, getline, etc.) |
| `drop_buildrequires` | list | BuildRequires to remove |
| `add_buildrequires` | list | BuildRequires to add |
| `drop_requires` | list | Requires to remove |
| `add_requires` | list | Requires to add |
| `configure_disable` | list | Features to disable (adds --disable-X) |
| `configure_enable` | list | Features to enable (adds --enable-X) |
| `ac_cv_overrides` | dict | Autoconf cache variable overrides |
| `remove_lines` | list | Lines to remove from spec |
| `comment_conditionals` | list | Conditionals to comment out |
| `header_overlays` | list | Header overlay directories to use |

### Available Compat Functions

| Function | Description |
|----------|-------------|
| `strdup` | Duplicate a string |
| `strndup` | Duplicate at most n bytes of a string |
| `stpcpy` | Copy string returning pointer to end |
| `getline` | Read a line from stream |
| `asprintf` | Print to allocated string |
| `vasprintf` | Print to allocated string (va_list) |
| `getprogname` | Get program name |
| `posix_spawn` | Spawn process (POSIX.1-2008) |
| `posix_spawnp` | Spawn process with PATH search |
| `posix_spawn_file_actions_*` | File actions for spawn |
| `openat` | Open file relative to directory fd |
| `fstatat` | Stat file relative to directory fd |
| `mkdirat` | Create directory relative to directory fd |
| `unlinkat` | Unlink relative to directory fd |
| `renameat` | Rename relative to directory fds |
| `readlinkat` | Readlink relative to directory fd |
| `symlinkat` | Symlink relative to directory fd |
| `linkat` | Hard link relative to directory fds |
| `fchmodat` | Chmod relative to directory fd |
| `fchownat` | Chown relative to directory fd |
| `faccessat` | Access check relative to directory fd |
| `utimensat` | Set timestamps relative to directory fd |
| `futimens` | Set timestamps via fd |
| `funopen` | BSD-style cookie I/O (for transparent decompression) |
| `qsort_r` | Reentrant quicksort |
| `timegm` | Inverse of gmtime |
| `mkdtemp` | Create temporary directory |

## IRIX Bootstrap

After building all required packages, create a self-contained bootstrap tarball for IRIX:

```bash
# Create bootstrap tarball (validates 17 required packages)
./scripts/bootstrap-tarball.sh

# Copy to IRIX
scp tmp/irix-bootstrap.tar.gz root@irix-host:/tmp/

# On IRIX, extract and initialize
cd /opt/chroot
gzcat /tmp/irix-bootstrap.tar.gz | tar xvf -
chroot /opt/chroot /bin/sh
export LD_LIBRARYN32_PATH=/usr/sgug/lib32
/usr/sgug/bin/rpm --initdb
/usr/sgug/bin/rpm -Uvh --nodeps /tmp/bootstrap-rpms/sgugrse-release*.noarch.rpm

# Test tdnf
/usr/sgug/bin/sgug-exec /usr/sgug/bin/tdnf repolist
```

The bootstrap tarball includes:
- All tdnf dependency chain packages (extracted)
- RPM files for database registration (`/tmp/bootstrap-rpms/`)
- Default repository config pointing to `/tmp/mogrix-repo`

See `HANDOFF.md` for detailed IRIX setup instructions.

## Project Structure

```
mogrix/
├── mogrix/              # Main Python package
│   ├── cli.py          # CLI commands
│   ├── parser/         # Spec and SRPM parsing
│   ├── rules/          # Rule loading and engine
│   ├── emitter/        # Spec and SRPM output
│   ├── headers/        # Header overlay management
│   ├── compat/         # Compat source injection
│   └── patches/        # Patch catalog
├── rules/              # Rule definitions
│   ├── generic.yaml    # Universal rules
│   └── packages/       # Package-specific rules
├── headers/            # Header overlay files
│   └── generic/        # Clang/IRIX compat headers
├── compat/             # Compat library (libcompat.a)
│   ├── include/        # Compat headers (mogrix-compat/)
│   │   └── mogrix-compat/
│   │       └── generic/  # stdlib.h, string.h, fcntl.h, spawn.h, etc.
│   └── runtime/        # Compat implementations
│       ├── strdup.c, strndup.c, stpcpy.c
│       ├── openat.c, fstatat.c, mkdirat.c, etc.
│       └── spawn.c     # posix_spawn implementation
├── scripts/            # Helper scripts
│   ├── bootstrap-tarball.sh   # Create IRIX bootstrap tarball
│   ├── irix-chroot-testing.sh # Safe chroot testing on IRIX
│   └── irix-bootstrap.sh      # Real IRIX installation
├── tests/              # Test suite
├── plan.md             # Architecture documentation
└── HANDOFF.md          # Session handoff notes
```

## Development

```bash
# Run tests
make test

# Run linter
make lint

# Format code
make format

# Validate rules
mogrix validate-rules

# Clean build artifacts
make clean
```

## Package Rules Status

13 packages validated for the tdnf dependency chain:

| # | Package | Version | Source | Description |
|---|---------|---------|--------|-------------|
| 1 | zlib-ng | 2.1.6 | FC40 | Compression library (replaces zlib in FC40) |
| 2 | bzip2 | 1.0.8 | FC40 | Block-sorting compression |
| 3 | popt | 1.19 | FC40 | Command-line option parsing |
| 4 | openssl | 3.2.1 | FC40 | TLS/SSL and crypto library |
| 5 | libxml2 | 2.12.5 | FC40 | XML parsing library |
| 6 | curl | 8.6.0 | FC40 | URL transfer library |
| 7 | xz | 5.4.6 | FC40 | LZMA compression |
| 8 | lua | 5.4.6 | FC40 | Scripting language (for rpm) |
| 9 | file | 5.45 | FC40 | File type detection |
| 10 | sqlite | 3.45.1 | FC40 | Embedded SQL database |
| 11 | rpm | 4.19.1.1 | FC40 | Package manager |
| 12 | libsolv | 0.7.28 | FC40 | Dependency solver |
| 13 | tdnf | 3.5.14 | Photon5 | Package manager CLI (target) |

Additional supporting package:
- **sgugrse-release** - Distribution release package (noarch, provides distroverpkg)

## Known Limitations

**IRIX Platform:**
- **fopencookie** - The glibc-style cookie I/O crashes on IRIX. Use `funopen` (BSD-style) instead.
- **O_NOFOLLOW** - IRIX doesn't support this flag; stripped in openat-compat.c.
- **vsnprintf(NULL, 0)** - Returns -1 on IRIX (not buffer size as in C99). Packages using this pattern need iterative vasprintf.
- **Thread-local storage** - IRIX rld doesn't support `__tls_get_addr`. Packages using `__thread` need patching.
- **Long tar filenames** - IRIX tar corrupts GNU long filenames. Use `createrepo_c --simple-md-filenames`.

**Build Environment:**
- Cross-compiled binaries cannot be tested on the Linux host (use IRIX chroot for testing).
- Some packages require libtool fixes for IRIX shared library versioning.
- Packages with assembly code need `--disable-asm` or MIPS-specific implementations.

**Compatibility:**
- Built packages use `/usr/sgug` prefix (SGUG-RSE convention).
- May conflict with existing SGUG-RSE installations (different library versions).
- rpm 4.19 uses sqlite backend by default; incompatible with SGUG-RSE's BerkeleyDB databases.

## Architecture

See `plan.md` for detailed architecture documentation, including:
- Rule engine design
- Header overlay system
- Compat source injection
- Build order for tdnf dependencies

## Troubleshooting

**Build fails with "cannot find -lcompat"**
```bash
./cleanup.sh  # Rebuilds libcompat.a in staging
```

**"Invalid argument" errors on IRIX**

Usually caused by unsupported flags like O_NOFOLLOW. Check if the compat layer strips them.

**Garbled output from rpm/tdnf**

IRIX vsnprintf doesn't support C99 size calculation. The package needs vasprintf injection or iterative buffer sizing.

**"Solv - I/O error" from tdnf**

libsolv can't read repository metadata. Ensure:
1. `funopen` is linked (not fopencookie)
2. Repository created with `createrepo_c --simple-md-filenames`
3. rpm database initialized: `rpm --initdb`

**Shared library versioning errors**

Run `$MOGRIX_ROOT/tools/fix-libtool-irix.sh libtool` after configure. This patches libtool for IRIX .so versioning.

**Cross-compiled binary crashes immediately**

Check for TLS usage (`__thread` keyword). IRIX rld doesn't support it - patch to use static variables or pthreads.

## License

See LICENSE file.
