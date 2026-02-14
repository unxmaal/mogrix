# Mogrix

## Overview

Mogrix is a complete IRIX cross-compilation system that transforms C/C++ software into working IRIX packages. It supports Fedora 40 SRPMs, upstream git repos, and tarball downloads — handling the entire pipeline from source fetch through cross-compilation to deployable RPMs and self-contained app bundles.

**Current Status:** 105+ source packages cross-compiled for IRIX (290+ RPMs), including a full GNU userland (coreutils, findutils, tar, make, sed, gawk, grep), build tools (autoconf, automake, libtool, perl, bash), crypto stack (gnupg2), a complete package management system (rpm + tdnf), library foundation packages (fontconfig, freetype, gettext, pcre2, libffi, libpng, and more), **gnutls** (TLS library with CA trust store), **openssh** (SSH server), **groff** (document formatting system), **nano** (text editor), **weechat** (IRC client with TLS verified on IRIX), **rsync** (file sync), **tmux** (terminal multiplexer), **vim** (text editor), **wget2** (HTTP/HTTPS downloader), **st** (X11 terminal), **bitlbee** (IM gateway with Discord), **Qt5** (5.15.13), and **aterm** — the first X11 graphical application running on the IRIX GUI. C++ cross-compilation is fully operational using clang++ with GCC 9 libstdc++. **23 app bundles** (`mogrix bundle`) create optimized, self-contained tarballs that coexist with SGUG-RSE — no `/usr/sgug` replacement needed.

**Target Platform:** SGI IRIX 6.5.x running on MIPS processors (O2, Octane, Origin, Fuel, Tezro). Builds use the N32 ABI (MIPS III instruction set).

**Background:** SGI's IRIX was discontinued in 2006, leaving users with aging software. [SGUG-RSE](https://github.com/sgidevnet/sgug-rse) (SGI Users Group Rebuilt Software Environment) provides modern open-source packages for IRIX, but its build process requires native IRIX compilation. Mogrix enables cross-compilation from Linux, making it faster and more accessible to build IRIX packages.

**Core Capabilities:**

- **IRIX Cross-Compilation** - Full toolchain integration with clang/LLD targeting MIPS N32 ABI. The `mogrix build --cross` command invokes rpmbuild with IRIX-specific compiler wrappers, header paths, and linker settings.

- **SRPM Conversion Engine** - YAML-based rules transform spec files for IRIX compatibility: drops unavailable dependencies, injects compat functions, applies platform-specific patches, and configures autoconf/cmake for cross-compilation.

- **Compat Library** - Drop-in implementations of 35+ missing POSIX/C99 functions (openat family, posix_spawn, vasprintf, funopen, pselect, openpty, open_memstream, explicit_bzero, etc.) automatically injected into packages that need them. dlmalloc (mmap-based allocator) is linked into executables only by the linker wrapper.

- **Source Analysis** - Scans source tarballs for known IRIX-incompatible patterns (`%zu` format strings, `__thread` TLS, volatile function pointers, epoll/inotify usage) using ripgrep. Patterns defined in YAML rules — adding a new check requires no code changes.

- **Validation** - Spec validation (specfile library) and RPM linting (rpmlint with IRIX-specific config) integrated into the conversion pipeline.

- **Staging System** - Cross-compiled packages are staged to provide headers and libraries for dependent builds, enabling complex dependency chains like the 13-package tdnf stack.

- **Bootstrap Tarball** - Self-contained tarball generator for deploying the complete tdnf package manager to IRIX without requiring any tools on the target system.

- **App Bundles** - `mogrix bundle` creates optimized, self-contained app tarballs for IRIX that coexist with SGUG-RSE. Resolves dependencies via ELF scanning, prunes unused libraries, trims terminfo, and generates Flatpak-style install scripts. Extract anywhere, run `./install`, add one directory to PATH.

## Installation

```bash
# Clone the repository
git clone https://github.com/unxmaal/mogrix.git
cd mogrix

# Install uv (Python package manager) if not already installed
curl -LsSf https://astral.sh/uv/install.sh | sh

# Install dependencies and run
uv sync
uv run mogrix --help
```

## Quick Start

### Standard SRPM Workflow (Recommended)

The recommended workflow operates on SRPMs directly. Direct spec file editing should be avoided.

#### Directory Conventions

| Directory | Purpose |
|-----------|---------|
| `~/mogrix_inputs/SRPMS/` | Original Fedora 40 SRPMs (fetched, read-only) |
| `~/mogrix_outputs/SRPMS/` | Converted SRPMs (mogrix convert output) |
| `~/mogrix_outputs/RPMS/` | Built MIPS RPMs (known-good outputs) |
| `~/rpmbuild/` | Ephemeral rpmbuild workspace |
| `/opt/sgug-staging/` | Cross-compilation staging area |

```bash
# 1. One-time setup of cross-compilation environment
uv run mogrix setup-cross

# 2. Fetch SRPM from Fedora
uv run mogrix fetch popt -y

# 3. Convert SRPM (applies rules, injects compat)
uv run mogrix convert ~/mogrix_inputs/SRPMS/popt-1.19-6.fc40.src.rpm

# 4. Cross-compile for IRIX
uv run mogrix build ~/mogrix_outputs/SRPMS/popt-1.19-6.fc40.src-converted/popt-1.19-6.src.rpm --cross

# 5. Stage for dependent builds
uv run mogrix stage ~/rpmbuild/RPMS/mips/popt*.rpm

# 6. Archive built RPMs
cp ~/rpmbuild/RPMS/mips/popt*.rpm ~/mogrix_outputs/RPMS/
```

### Upstream Source Workflow (Git Repos & Tarballs)

For packages not in Fedora, add an `upstream:` block to the package YAML and use `create-srpm`:

```bash
# 1. Create package rules with upstream: block
cat > rules/packages/gmi100.yaml << 'EOF'
package: gmi100
upstream:
  url: https://github.com/shtanton/gmi100
  version: "1.0"
  build_system: makefile
  license: MIT
  summary: "Minimalist Gemini client"
rules:
  inject_compat_functions:
    - getline
EOF

# 2. Generate SRPM from upstream source
uv run mogrix create-srpm gmi100

# 3. Normal pipeline from here
uv run mogrix convert ~/mogrix_inputs/SRPMS/gmi100-1.0-1.src.rpm
uv run mogrix build ~/mogrix_outputs/SRPMS/.../gmi100-1.0-1.src.rpm --cross
```

Supports `autoconf`, `cmake`, `meson`, and `makefile` build systems. See `rules/methods/upstream-packages.md` for full documentation.

### Analyze an SRPM

```bash
# See what rules would apply to a package
uv run mogrix analyze package-1.0-1.fc40.src.rpm
```

### Batch Build

```bash
# Build multiple packages from a list (user controls order)
uv run mogrix batch-build --from-list packages.txt

# Build all dependencies for a target package (topological order)
uv run mogrix batch-build --target gdb

# Preview what would be built
uv run mogrix batch-build --from-list packages.txt --dry-run

# Generate JSON report
uv run mogrix batch-build --from-list packages.txt --output-report report.json
```

Packages without rules get candidate YAML generated in `rules/candidates/` for human review. The batch always moves on — never blocks on failures.

### App Bundles

Create self-contained app tarballs for IRIX that coexist with SGUG-RSE — no `/usr/sgug` replacement needed.

```bash
# Single app bundle
uv run mogrix bundle nano

# Suite bundle (multiple apps, shared libs)
uv run mogrix bundle telescope snownews lynx --name mogrix-smallweb

# Include extra subpackages
uv run mogrix bundle groff --include groff-perl

# Just build directory, no tarball
uv run mogrix bundle nano --no-tarball
```

Bundles resolve dependencies via ELF `readelf -d` NEEDED scanning, include only the shared libraries actually needed, trim terminfo to common terminals, and strip docs. The nano bundle is under 1MB compressed.

Install on IRIX:

```sh
# Extract anywhere
tar xzf nano-7.2-6-irix-bundle.tar.gz -C /opt/mogrix-apps/

# Run the install script (creates trampolines in ../bin/)
cd /opt/mogrix-apps/nano-7.2-6-irix-bundle
./install

# Add to PATH once (put in ~/.profile for permanent)
PATH=/opt/mogrix-apps/bin:$PATH; export PATH

# Run like any other command
nano
```

Multiple bundles share the same `bin/` directory — one PATH entry covers all bundles. Each bundle includes `./uninstall` for clean removal.

### Dependency Roadmap

```bash
# Show full transitive build-dependency graph
uv run mogrix roadmap aterm

# JSON output for programmatic use
uv run mogrix roadmap aterm --json

# Rich tree widget
uv run mogrix roadmap aterm --tree
```

### Batch Conversion

```bash
# Convert multiple SRPMs at once
uv run mogrix batch srpms_directory/ output_directory/
```

### Build a Converted Package

```bash
# Cross-compile for IRIX (primary use case)
uv run mogrix build converted-package.src.rpm --cross

# Dry-run to see cross-compilation command
uv run mogrix build converted-package.src.rpm --cross --dry-run

# Native rpmbuild (for testing on Linux)
uv run mogrix build converted-package.src.rpm
```

### Stage Cross-Compiled RPMs

After cross-compiling packages, stage them to make their headers and libraries available for building dependent packages:

```bash
# Stage RPMs to /opt/sgug-staging
uv run mogrix stage ~/rpmbuild/RPMS/mips/popt-*.rpm

# Stage to custom location
uv run mogrix stage ~/rpmbuild/RPMS/mips/*.rpm --staging-dir /my/staging

# List what's staged
uv run mogrix stage --list

# Clean staging area
uv run mogrix stage --clean
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
uv run mogrix setup-cross
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

All commands use `uv run mogrix` (or activate the venv first with `source .venv/bin/activate`).

| Command | Description |
|---------|-------------|
| `mogrix setup-cross` | One-time cross-compilation environment setup |
| `mogrix fetch <packages...>` | Fetch SRPMs from Fedora repositories |
| `mogrix convert <srpm>` | Convert SRPM (applies IRIX rules, repackages) |
| `mogrix build <srpm> --cross` | Cross-compile converted SRPM for IRIX |
| `mogrix stage <rpms...>` | Stage cross-compiled RPMs for dependency resolution |
| `mogrix analyze <srpm>` | Analyze rules + scan source for IRIX issues |
| `mogrix sync-headers` | Sync compat headers to staging after edits |
| `mogrix batch <dir> <out>` | Convert multiple SRPMs in batch |
| `mogrix batch-build` | Automated multi-package build pipeline |
| `mogrix roadmap <package>` | Show transitive build-dependency graph |
| `mogrix lint <rpms...>` | Lint RPMs/specs with IRIX-specific rpmlint config |
| `mogrix validate-spec <spec>` | Validate spec file structure |
| `mogrix list-rules` | List available package rules |
| `mogrix validate-rules` | Validate all rule files |
| `mogrix audit-rules` | Scan package rules for duplicates and class candidates |
| `mogrix score-rules` | Score package rules by complexity |
| `mogrix bundle <package>` | Create self-contained IRIX app bundle |

## Dependency Handling

When building a converted package, mogrix detects missing dependencies and offers to fetch them:

```bash
# Build will detect missing deps and offer to fetch them
uv run mogrix build converted-package.src.rpm

# Or manually fetch SRPMs from Fedora archives
uv run mogrix fetch popt zlib curl

# Search shows matches and prompts for confirmation
uv run mogrix fetch zlib
# Found 2 matching packages:
#   [1] zlib-ada-1.4-0.37.fc40.src.rpm
#   [2] zlib-ng-2.1.6-2.fc40.src.rpm
# Select [1]:

# Auto-confirm single matches with -y
uv run mogrix fetch popt -y

# Fetch from Photon OS repos (for tdnf, etc.)
uv run mogrix fetch tdnf --base-url photon5

# Available presets: photon5, photon4, photon3
# Or use a custom URL (ending with / for flat directories)
uv run mogrix fetch popt --base-url http://my-mirror.example.com/srpms/
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
| `open_memstream` | Dynamic memory buffer stream (POSIX.1-2008) |
| `dprintf` / `vdprintf` | Formatted print to file descriptor |
| `explicit_bzero` | Zero memory that won't be optimized away |
| `secure_getenv` | Secure environment variable access |
| `strerror_r` | Thread-safe strerror |
| `strnlen` | Length of fixed-size string |
| `strcasestr` | Case-insensitive substring search |
| `strsep` | Token extraction from strings |
| `basename` | Extract filename from path |
| `setenv` / `unsetenv` | Environment variable manipulation |
| `qsort_r` | Reentrant quicksort |
| `timegm` | Inverse of gmtime |
| `mkdtemp` | Create temporary directory |
| `dlmalloc` | mmap-based malloc — linked into executables only by irix-ld |
| `strtof` | String to float conversion |
| `getopt_long` | GNU long option parsing |
| `reallocarray` | Overflow-checked array allocation |
| `openpty` | Open pseudo-terminal (wraps IRIX `_getpty()`) |
| `pselect` | Synchronous I/O multiplexing with signal mask |
| `err` / `errx` / `warn` / `warnx` | BSD error reporting functions |
| `sqlite3_stub` | Stub sqlite3 for optional features |

## IRIX Bootstrap

After building all required packages, create a self-contained bootstrap tarball for IRIX:

```bash
# Create bootstrap tarball (includes all base packages)
./scripts/bootstrap-tarball.sh

# Copy to IRIX and extract into chroot
scp tmp/irix-bootstrap.tar.gz root@irix-host:/tmp/
# On IRIX:
cd /opt/chroot
gzcat /tmp/irix-bootstrap.tar.gz | tar xvf -
chroot /opt/chroot /bin/sh
export LD_LIBRARYN32_PATH=/usr/sgug/lib32
/usr/sgug/bin/rpm --initdb
cd /tmp/bootstrap-rpms
/usr/sgug/bin/rpm -Uvh --nodeps sgugrse-release*.noarch.rpm
/usr/sgug/bin/rpm -Uvh *.rpm
```

Additional packages (Phase 2+) are installed individually via `rpm -Uvh` after copying to the chroot. The MCP-based workflow uses `irix_copy_to` and `irix_exec` tools instead of SSH.

The bootstrap tarball includes:
- All Phase 1 dependency chain packages (extracted to filesystem)
- RPM files for database registration (`/tmp/bootstrap-rpms/`)

See `HANDOFF.md` for detailed IRIX setup instructions and current package status.

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
│   ├── bundle.py       # Self-contained IRIX app bundle generator
│   ├── analyzers/      # Source-level static analysis
│   ├── validators/     # Spec file validation
│   └── patches/        # Patch catalog
├── rules/              # Rule definitions
│   ├── generic.yaml    # Universal rules (applied to ALL packages)
│   ├── classes/        # Class rules (nls-disabled, etc.)
│   ├── packages/       # Package-specific rules (145 packages)
│   └── source_checks.yaml  # IRIX source pattern definitions
├── headers/            # Header overlay files
│   └── generic/        # Clang/IRIX compat headers
├── compat/             # Compat library
│   ├── catalog.yaml    # Function registry + source patterns
│   ├── include/        # Compat headers (mogrix-compat/)
│   │   └── mogrix-compat/
│   │       └── generic/  # stdlib.h, string.h, fcntl.h, spawn.h, etc.
│   └── (source dirs)   # string/, stdio/, stdlib/, dicl/, malloc/, etc.
├── rpmlint.toml        # IRIX-specific rpmlint configuration
├── scripts/            # Helper scripts
├── tests/              # Test suite (170 tests)
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
uv run mogrix validate-rules

# Clean build artifacts
make clean
```

## Package Rules Status

105+ source packages cross-compiled for IRIX across multiple phases:

### Phase 1: Bootstrap (14 packages)

| # | Package | Version | Description |
|---|---------|---------|-------------|
| 1 | zlib-ng | 2.1.6 | Compression library (replaces zlib in FC40) |
| 2 | bzip2 | 1.0.8 | Block-sorting compression |
| 3 | popt | 1.19 | Command-line option parsing |
| 4 | openssl | 3.2.1 | TLS/SSL and crypto library |
| 5 | libxml2 | 2.12.5 | XML parsing library |
| 6 | curl | 8.6.0 | URL transfer library |
| 7 | xz | 5.4.6 | LZMA compression |
| 8 | lua | 5.4.6 | Scripting language (for rpm) |
| 9 | file | 5.45 | File type detection |
| 10 | sqlite | 3.45.1 | Embedded SQL database |
| 11 | rpm | 4.19.1.1 | Package manager |
| 12 | libsolv | 0.7.28 | Dependency solver |
| 13 | tdnf | 3.5.14 | Package manager CLI |
| 14 | sgugrse-release | 0.0.7beta | Distribution release package |

### Phase 1.5: System Libraries (2 packages)

| # | Package | Version | Description |
|---|---------|---------|-------------|
| 15 | ncurses | 6.4 | Terminal handling library |
| 16 | readline | 8.2 | Line editing library |

### Phase 2: Build Tools (6 packages)

| # | Package | Version | Description |
|---|---------|---------|-------------|
| 17 | m4 | 1.4.19 | Macro processor |
| 18 | perl | 5.38.2 | Perl interpreter (monolithic build) |
| 19 | bash | 5.2.26 | Bourne-Again Shell |
| 20 | autoconf | 2.71 | Configure script generator |
| 21 | automake | 1.16.5 | Makefile generator |
| 22 | libtool | 2.4.7 | Shared library support |

### Phase 3a: Crypto Stack (7 packages)

| # | Package | Version | Description |
|---|---------|---------|-------------|
| 23 | pkgconf | 2.1.0 | Package configuration tool |
| 24 | libgpg-error | 1.48 | GnuPG error library |
| 25 | libgcrypt | 1.10.3 | GnuPG crypto library |
| 26 | libassuan | 2.5.7 | GnuPG IPC library |
| 27 | libksba | 1.6.6 | GnuPG X.509 library |
| 28 | npth | 1.7 | GnuPG threading library |
| 29 | gnupg2 | 2.4.4 | GNU Privacy Guard |

### Phase 3b: GNU Text Tools (3 packages)

| # | Package | Version | Description |
|---|---------|---------|-------------|
| 30 | sed | 4.9 | Stream editor |
| 31 | gawk | 5.3.0 | Pattern processing language |
| 32 | grep | 3.11 | Pattern matching |

### Phase 4a: User-Facing Utilities (5 packages)

| # | Package | Version | Description |
|---|---------|---------|-------------|
| 33 | less | 643 | File pager |
| 34 | which | 2.21 | Command locator |
| 35 | gzip | 1.13 | Compression utility |
| 36 | diffutils | 3.10 | File comparison |
| 37 | patch | 2.7.6 | Patch application |

### Phase 4b: Build/System Utilities (3 packages)

| # | Package | Version | Description |
|---|---------|---------|-------------|
| 38 | make | 4.4.1 | Build automation |
| 39 | findutils | 4.9.0 | File search (find, xargs) |
| 40 | tar | 1.35 | Archive handling |

### Phase 4c: Core Utilities (1 package)

| # | Package | Version | Description |
|---|---------|---------|-------------|
| 41 | coreutils | 9.4 | GNU core utilities (ls, cp, mv, cat, head, sort, etc.) |

### Phase 5: Library Foundation + Apps (23 packages)

| # | Package | Version | Description |
|---|---------|---------|-------------|
| 42 | pcre2 | 10.42 | Perl-compatible regex library |
| 43 | symlinks | 1.7 | Symlink maintenance utility |
| 44 | tree-pkg | 2.1.0 | Directory listing tree |
| 45 | oniguruma | 6.9.9 | Regular expression library |
| 46 | libffi | 3.4.4 | Foreign function interface |
| 47 | tcl | 8.6.13 | Tool Command Language |
| 48 | flex | 2.6.4 | Lexical analyzer generator |
| 49 | chrpath | 0.16 | Rpath editor |
| 50 | libpng | 1.6.40 | PNG image library |
| 51 | bison | 3.8.2 | Parser generator |
| 52 | libunistring | 1.1 | Unicode string library |
| 53 | gettext | 0.22.5 | Internationalization framework |
| 54 | zstd | 1.5.5 | Zstandard compression |
| 55 | fontconfig | 2.15.0 | Font configuration library |
| 56 | freetype | 2.13.2 | Font rendering engine |
| 57 | expat | 2.6.0 | XML parser (staged) |
| 58 | nettle | 3.9.1 | Crypto library (staged) |
| 59 | libtasn1 | 4.19.0 | ASN.1 library (staged) |
| 60 | fribidi | 1.0.13 | Bidi algorithm (staged) |
| 61 | libjpeg-turbo | 3.0.2 | JPEG library (staged) |
| 62 | pixman | 0.43.0 | Pixel manipulation (staged) |
| 63 | uuid | 1.6.2 | UUID library (staged) |
| 64 | aterm | 1.0.1 | X11 terminal emulator (first GUI app!) |

### Sessions 5-11: Additional Packages (17 packages)

| # | Package | Version | Description |
|---|---------|---------|-------------|
| 65 | openssh | 9.6p1 | SSH server/client (first network service!) |
| 66 | unzip | 6.0 | ZIP decompression |
| 67 | zip | 3.0 | ZIP compression |
| 68 | nano | 7.2 | Text editor |
| 69 | rsync | 3.2.7 | File synchronization |
| 70 | figlet | 2.2.5 | ASCII art text |
| 71 | sl | 5.02 | Steam locomotive animation |
| 72 | time | 1.9 | Command timing |
| 73 | cmatrix | 2.0 | Matrix screen effect |
| 74 | gmp | 6.2.1 | GNU multiprecision arithmetic |
| 75 | mpfr | 4.2.1 | Multiple-precision floating-point |
| 76 | hyphen | 2.8.8 | Hyphenation library |
| 77 | libevent | 2.1.12 | Event notification library |
| 78 | libxslt | 1.1.39 | XSLT processing library |
| 79 | giflib | 5.2.2 | GIF image library |
| 80 | libstrophe | 0.13.1 | XMPP client library |
| 81 | groff | 1.23.0 | Document formatting system (first C++ package!) |

### Sessions 12-20: X11, Qt5, and More Apps (24 packages)

| # | Package | Version | Description |
|---|---------|---------|-------------|
| 82 | gdbm | 1.23 | GNU database manager |
| 83 | glib2 | 2.80.0 | GLib core library (first meson package) |
| 84 | harfbuzz | 8.3.0 | Text shaping engine |
| 85 | xcb-proto | 1.16.0 | X11 protocol definitions |
| 86 | libxcb | 1.16 | X protocol C binding |
| 87 | xcb-util | 0.4.1 | XCB utility library |
| 88 | xcb-util-wm | 0.4.2 | XCB window manager helpers |
| 89 | xcb-util-image | 0.4.1 | XCB image helpers |
| 90 | xcb-util-keysyms | 0.4.1 | XCB keysym helpers |
| 91 | xcb-util-renderutil | 0.3.10 | XCB render helpers |
| 92 | libxkbcommon | 1.6.0 | XKB keymap library |
| 93 | double-conversion | 3.3.0 | Binary-decimal conversion |
| 94 | qt5-qtbase | 5.15.13 | Qt5 framework (Core+Gui+Widgets+XcbQpa) |
| 95 | libXrender | 0.9.11 | X Render extension |
| 96 | libXft | 2.3.8 | X FreeType library |
| 97 | cairo | 1.18.0 | 2D graphics library |
| 98 | pango | 1.52.0 | Text layout engine |
| 99 | libptytty | 2.0 | PTY/TTY library |
| 100 | dmenu | 5.2 | Dynamic X11 menu |
| 101 | rxvt-unicode | 9.31 | Unicode terminal emulator |
| 102 | st | 0.9 | Suckless terminal (Xft + Iosevka font) |
| 103 | bitlbee | 3.6 | IRC gateway with Discord plugin |
| 104 | xclip | 0.13 | X11 clipboard tool |
| 105 | lolcat | 1.5 | Rainbow text colorizer |

### Sessions 21-37: Networking, TLS, and User Apps (16 packages)

| # | Package | Version | Description |
|---|---------|---------|-------------|
| 106 | gnutls | 3.8.3 | TLS library (CA trust store configured) |
| 107 | libtasn1 | 4.19.0 | ASN.1 library |
| 108 | nettle | 3.9.1 | Crypto library |
| 109 | libidn2 | 2.3.7 | Internationalized domain names |
| 110 | libretls | 3.8.1 | LibreTLS (libtls API) |
| 111 | libpipeline | 1.5.7 | Pipeline manipulation library |
| 112 | telescope | 0.11 | Gemini browser |
| 113 | gmi100 | 3.2 | Gemini client |
| 114 | lynx | 2.9.2 | Web browser |
| 115 | snownews | 1.11 | RSS reader |
| 116 | weechat | 4.3.1 | IRC client (TLS verified on real IRIX) |
| 117 | tmux | 3.5a | Terminal multiplexer |
| 118 | vim | 9.1.158 | Text editor |
| 119 | wget2 | 2.1.0 | HTTP/HTTPS downloader |
| 120 | man-db | 2.12.0 | Manual page system |
| 121 | tinc | 1.0.36 | VPN daemon |

Additional packages: bc, jq.

145 package rule files exist, with 40+ pending future builds.

## Known Limitations

**IRIX Platform:**
- **fopencookie** - The glibc-style cookie I/O crashes on IRIX. Use `funopen` (BSD-style) instead.
- **O_NOFOLLOW** - IRIX doesn't support this flag; stripped in openat-compat.c.
- **vsnprintf(NULL, 0)** - Returns -1 on IRIX (not buffer size as in C99). Packages using this pattern need iterative vasprintf.
- **Thread-local storage** - IRIX rld doesn't support `__tls_get_addr`. Packages using `__thread` need patching.
- **`%zu`/`%zd`/`%td` format specifiers** - IRIX libc is pre-C99; these corrupt varargs → SIGSEGV or literal letter output. Use `%u`/`%d` instead (32-bit on n32).
- **`%Lg` (long double printf)** - IRIX printf doesn't handle long double formatting. `seq` disabled due to this.
- **Volatile function pointer initializers** - `static volatile fptr = memset;` crashes on IRIX rld due to relocation issues. Use `explicit_bzero` compat.
- **GNU ld linker scripts** - IRIX rld can only load ELF .so files. `INPUT(-lfoo)` text files crash rld. Replace with symlinks.
- **`--export-dynamic`** - Causes IRIX rld to SIGSEGV when dynamic symbol table is very large (468+ entries). Disable features that require it.
- **IRIX native tar** - Silently drops files with paths >100 characters. Doesn't support `-z` or pax headers. Always use `gtar` from the chroot for extraction. For deployment, use tar pipes (`tar cf - | tar xf -`) or symlinks.
- **IRIX `cp -r`** - Breaks on relative symlinks (dereferences them, fails if target doesn't exist yet). Use tar pipe instead.
- **dlmalloc in shared libraries** - NEVER link dlmalloc into .so files. With `-Bsymbolic-functions`, each library gets its own private heap. Cross-library free() calls abort(). dlmalloc is linked into executables only by irix-ld.
- **`-z separate-code`** - Produces 3 LOAD segments in shared libraries, corrupting IRIX rld internal state on dlopen. Use custom linker script for standard 2-segment (RE+RW) layout.
- **GNU symbol versioning** - `--version-script` creates `.gnu.version` tags that crash IRIX rld (predates GNU versioning). Filtered by irix-ld.
- **pthread_sigmask** - In libpthread, not libc. Must explicitly link `-lpthread` when gnulib provides a replacement.
- **C++ cmath/specfun** - GCC 9 c++config.h enables C99 math TR1 and specfun, but IRIX libm lacks these. Must disable in c++config.h.
- **wchar_t in C++ mode** - IRIX stdlib.h tries to typedef wchar_t, but it's a C++ keyword. Fix: `-D_WCHAR_T`.
- **R_MIPS_REL32 relocations** - Function pointers in static data crash IRIX rld. Replace with dispatch functions (switch/strcmp).
- **sockaddr_storage** - Hidden when `_XOPEN_SOURCE` is set in compat headers. Define explicitly in socket.h overlay.
- **autoreconf overwrites prep_commands** - Patches to `configure` in `%prep` are undone by `autoreconf` in `%build`. Use spec_replacements to inject fixes after autoreconf.
- **update-alternatives** - Doesn't exist on IRIX. Drop `Requires(post/preun/postun)` for packages that use it.
- **ncurses ext-colors terminfo corruption** - ABI 6 auto-enables `--enable-ext-colors`, causing terminfo reader to interpret 16-bit numbers as 32-bit. Fix: `--disable-ext-colors`.

**Build Environment:**
- Cross-compiled binaries cannot be tested on the Linux host (use IRIX chroot for testing).
- Some packages require libtool fixes for IRIX shared library versioning.
- Packages with assembly code need `--disable-asm` or MIPS-specific implementations.
- Indented `%configure` (inside for loops) is not matched by `configure_flags:add` — use `spec_replacements` instead.
- `LIBS="value"` on `%configure` line **replaces** the exported `LIBS`. Always use `LIBS="$LIBS value"` to append.

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

Also check for `--export-dynamic` in link commands — large dynamic symbol tables crash rld. And volatile function pointer static initializers (`static volatile fptr = memset;`) cause rld relocation failures.

**Binary crashes during normal operation (not at startup)**

Likely dlmalloc/libc allocator mismatch. libc functions like `strdup`, `asprintf`, `getline` allocate with libc malloc, but our code frees with dlmalloc. Inject compat `strdup`/`strndup` etc. via `inject_compat_functions`.

**ABORT during TLS or cross-library calls**

If dlmalloc was accidentally linked into a shared library, each .so gets its own private heap (due to `-Bsymbolic-functions`). When library A allocates and library B frees, dlmalloc detects a foreign pointer and calls `abort()`. Fix: ensure dlmalloc is only in executables (check with `readelf -s foo.so | grep malloc` — should show 0 malloc exports in .so files).

**Undefined symbol: pthread_sigmask**

IRIX has `pthread_sigmask` in libpthread, not libc. Add `LIBS="$LIBS -lpthread"` to configure environment and set `ac_cv_func_pthread_sigmask: "yes"` in ac_cv_overrides.

## Integration with Agentic AI

Mogrix is designed for effective collaboration with AI coding assistants like Claude Code. The project includes structured documentation (`rules/INDEX.md`, `HANDOFF.md`, `rules/methods/*.md`) that AI agents can reference to understand patterns and make informed decisions.

**Important:** The `rules/` directory serves a dual purpose:
- **mogrix** loads these YAML files programmatically via `RuleLoader` to transform spec files during conversion
- **AI agents** read the same files as reference documentation to understand patterns and create new rules

This means the rules are both executable configuration *and* human/AI-readable documentation. When an agent reads `rules/packages/popt.yaml`, it sees exactly what mogrix will apply - making it straightforward to understand patterns and create similar rules for new packages.

### How to Use AI Effectively

**Start with context:** Point the agent to `rules/INDEX.md` first. This index links to method documentation and package rules, allowing the agent to retrieve specific information as needed rather than hallucinating patterns.

**Be specific about the task:** "Port package X" is too vague. Better: "Create a rule file for package X. It uses autoconf. Check similar packages like popt.yaml for patterns."

**Iterate incrementally:** Don't ask the agent to port 10 packages at once. Port one, test it on IRIX, fix issues, then proceed. Each package teaches lessons that apply to the next.

**Use the existing workflow:**
```bash
uv run mogrix fetch <package> -y
uv run mogrix convert ~/mogrix_inputs/SRPMS/<package>-*.src.rpm
uv run mogrix build ~/mogrix_outputs/SRPMS/<package>-*-converted/<package>-*.src.rpm --cross
# Test on IRIX before proceeding
```

### LLMs Are Not Magic

AI assistants are powerful tools, but they have fundamental limitations:

**They guess plausibly, not correctly.** An LLM will confidently generate a sed pattern that looks right but matches zero lines, or six lines instead of one. The rpm futimens bug (documented in HANDOFF.md) cost hours of debugging because a sed pattern was too broad - it "worked" but broke unrelated code.

**Output must be tested.** Every change must be verified:
- Does the package build? (`mogrix build --cross`)
- Does it run on IRIX? (test in chroot)
- Does it break other packages? (rebuild dependents)

**Small mistakes are multiplicative.** A wrong `ac_cv_override` silently produces a broken binary. A missing compat function causes a runtime crash. A bad sed creates a package that builds but segfaults. These compound: package A builds wrong → package B links against it → package C inherits the bug → hours of debugging to find the root cause.

### Best Practices

1. **Read before writing.** Always have the agent read existing files before modifying them. "Check popt.yaml, then create a similar rule for package X."

2. **Prefer patches over sed.** Sed silently succeeds when patterns don't match. Use `.patch` files or `safepatch` (fails loudly on mismatch).

3. **Document the why.** When the agent fixes something, have it document why in HANDOFF.md or the rule file. Future sessions need this context.

4. **Verify on real hardware.** Cross-compilation hides runtime issues. Always test on actual IRIX before declaring success.

5. **Use the handoff pattern.** When ending a session, update HANDOFF.md with current state, pending issues, and next steps. The next session (human or AI) starts informed.

### Project Files for AI Context

| File | Purpose |
|------|---------|
| `rules/INDEX.md` | Package lookup, method links - **read first** |
| `HANDOFF.md` | Current state, recent fixes, known issues |
| `rules/methods/*.md` | Process documentation (text replacement, IRIX testing, etc.) |
| `rules/packages/*.yaml` | Existing package rules - patterns to follow |
| `compat/catalog.yaml` | Available compat functions |

The goal is augmented intelligence: human judgment directing AI capability, with verification at every step.

## License

See LICENSE file.
