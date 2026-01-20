# Mogrix

SRPM conversion engine for IRIX. Transforms Fedora SRPMs into IRIX-compatible packages.

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

### Analyze a Spec or SRPM

```bash
# See what rules would apply to a package
mogrix analyze path/to/package.spec
mogrix analyze package-1.0-1.fc40.src.rpm
```

### Convert an SRPM (Full Workflow)

```bash
# Extract, convert, and repackage as new SRPM
mogrix convert package-1.0-1.fc40.src.rpm -o output_dir/
```

This will:
1. Extract the SRPM
2. Convert the spec file using rules
3. Copy all sources to the output directory
4. Build a new converted SRPM

### Convert a Spec File Only

```bash
# Convert just a spec file (outputs to stdout)
mogrix convert path/to/package.spec

# Save to file
mogrix convert path/to/package.spec -o output.spec
```

### Batch Conversion

```bash
# Convert multiple SRPMs at once
mogrix batch srpms_directory/ output_directory/
```

### Build a Converted Package

```bash
# Build using rpmbuild (dry-run to see what would happen)
mogrix build converted-package.src.rpm --dry-run

# Actually build (native rpmbuild)
mogrix build converted-package.src.rpm

# Cross-compile for IRIX (requires cross-toolchain setup)
mogrix build converted-package.src.rpm --cross

# Dry-run to see cross-compilation command
mogrix build converted-package.src.rpm --cross --dry-run
```

### Cross-Compilation for IRIX

The `--cross` flag enables IRIX cross-compilation:

```bash
mogrix build converted-package.src.rpm --cross
```

This requires the following setup:
- **IRIX sysroot** at `/opt/irix-sysroot/`
- **Cross-toolchain** at `/opt/cross/bin/`:
  - `irix-cc-bootstrap` - C compiler wrapper
  - `irix-ld-lld` - Linker wrapper (vvuk's LLD with IRIX support)
- **RPM macros** at `/opt/sgug-staging/rpmmacros.irix`

See `/src/plan.md` for detailed toolchain setup instructions.

## CLI Commands

| Command | Description |
|---------|-------------|
| `mogrix analyze <spec\|srpm>` | Analyze and show what rules would apply |
| `mogrix convert <spec\|srpm>` | Convert spec/SRPM (SRPM: full workflow with repackaging) |
| `mogrix batch <dir> <out>` | Convert multiple SRPMs in batch |
| `mogrix build <spec\|srpm>` | Build a converted spec or SRPM with rpmbuild |
| `mogrix fetch <packages...>` | Fetch SRPMs from Fedora repositories |
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
