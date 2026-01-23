# Mogrix

TransMOGrifies Linux SRPMs into IRIX-compatible packages.

## Overview

Mogrix is a deterministic SRPM-to-RSE-SRPM conversion engine that transforms Fedora SRPMs into IRIX-compatible packages. It centralizes all platform knowledge required to adapt Linux build intent for IRIX reality.

**Key Features:**
- YAML-based rule engine for spec file transformations
- Compat source injection (replaces libdicl dependency)
- Header overlay system for clang/IRIX compatibility
- Batch conversion for multiple packages
- SRPM extraction and packaging

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

# 2. Fetch SRPM from Fedora
mogrix fetch popt

# 3. Convert SRPM (applies IRIX rules)
mogrix convert popt-1.19-6.fc40.src.rpm -o converted/

# 4. Cross-compile for IRIX
mogrix build converted/popt-1.19-6.rse.src.rpm --cross

# 5. Stage for dependent builds
mogrix stage rpmbuild/RPMS/mipsn32/*.rpm
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
mogrix stage rpmbuild/RPMS/mipsn32/popt-*.rpm

# Stage to custom location
mogrix stage rpmbuild/RPMS/mipsn32/*.rpm --staging-dir /my/staging

# List what's staged
mogrix stage --list

# Clean staging area
mogrix stage --clean
```

The staging area extracts RPM contents so that subsequent builds can find headers (`-I/opt/sgug-staging/usr/sgug/include`) and libraries (`-L/opt/sgug-staging/usr/sgug/lib32`).

### Cross-Compilation Setup

The `--cross` flag enables IRIX cross-compilation. This requires:

**IRIX Sysroot** at `/opt/irix-sysroot/`:
- Pristine IRIX system headers and libraries
- Obtained from an IRIX 6.5.30 installation

**Cross-Toolchain** at `/opt/cross/bin/`:
- `irix-cc-bootstrap` - Clang wrapper for IRIX cross-compilation
- `irix-ld-lld` - vvuk's LLD with IRIX/MIPS N32 support
- `mips-sgi-irix6.5-ar`, `mips-sgi-irix6.5-ranlib` - binutils

**RPM Macros** at `/opt/sgug-staging/rpmmacros.irix`:
- Defines toolchain paths, CFLAGS, LDFLAGS for cross-compilation

**Staging Area** at `/opt/sgug-staging/`:
- Where cross-compiled packages are extracted for dependency resolution

See `plan.md` for detailed toolchain setup instructions.

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
| `getline` | Read a line from stream |
| `asprintf` | Print to allocated string |
| `vasprintf` | Print to allocated string (va_list) |
| `reallocarray` | Safe array reallocation |
| `strerror_r` | Thread-safe strerror |

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
├── compat/             # Compat source files
│   ├── catalog.yaml    # Function catalog
│   └── string/         # String function implementations
├── tests/              # Test suite
└── plan.md             # Architecture documentation
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

32 package rules covering the tdnf dependency chain:

- **Security/Crypto**: popt, libgpg-error, libgcrypt, libassuan, libksba, nettle, libtasn1, p11-kit, gnutls, gpgme, gnupg2, openssl
- **Python3/GLib**: python3, glib2, gobject-introspection, pcre, pcre2, fribidi, cairo
- **Package Management**: elfutils, file, libarchive, rpm, libsolv, zchunk, expat, libxml2, lua
- **Network**: curl, libmetalink, ca-certificates
- **Target**: tdnf

## Architecture

See `plan.md` for detailed architecture documentation, including:
- Rule engine design
- Header overlay system
- Compat source injection
- Build order for tdnf dependencies

## License

See LICENSE file.
