# Mogrix Contributor Onboarding Guide

## 1. Prerequisites

### Language and Runtime

- **Python 3.11+** — enforced by `pyproject.toml` (`requires-python = ">=3.11"`). Older versions will be rejected at install time.
- **[uv](https://github.com/astral-sh/uv)** — the project's package manager. Do not use pip or poetry; the lockfile and workspace configuration assume uv.

```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
```

### Cross-Compilation Toolchain (only needed for IRIX builds)

If you plan to work on the cross-compilation pipeline (not required for rules or Python development):

| Tool | Version | Notes |
|------|---------|-------|
| Clang/LLVM | 16+, **upstream build with MIPS target** | Distro packages often lack the MIPS target; build from source or use a pre-built LLVM release |
| LLD | 18, **patched** | Use `scripts/build-lld-irix.sh`; the upstream LLD 18 has an IRIX-specific bug |
| GNU Binutils | 2.41, targeting `mips-sgi-irix6.5` | Provides `objcopy`, `readelf`, and BFD `ld` as fallback |
| IRIX Sysroot | From a live IRIX 6.5.x machine | System headers and libraries; cannot be synthesized |
| rpmbuild | any recent | From your distro (`rpm-build` package) |

### OS

Linux (Ubuntu or Fedora recommended). macOS is not supported for cross-compilation work; the compiler wrappers and rpmbuild integration assume a Linux environment.

### Local Dependency

The project has one non-PyPI dependency:

```toml
"mcm-engine @ file:///home/edodd/projects/github/unxmaal/mcm-engine"
```

This path is **hardcoded to the original developer's home directory**. Before running `uv sync`, either:
- Clone `mcm-engine` to that path, or
- Edit `pyproject.toml` to point to wherever you cloned it.

This is a known rough edge for new contributors.

---

## 2. Clone, Build, and Run

```bash
git clone https://github.com/unxmaal/mogrix.git
cd mogrix

# Edit mcm-engine path in pyproject.toml if your username is not edodd
uv sync

# Verify the install
uv run mogrix --help
```

There is no compilation step for the Python code. The `uv sync` command creates a `.venv/` in the project root and installs all dependencies.

To activate the venv manually (optional, for editor integration):

```bash
source .venv/bin/activate
mogrix --help
```

### First Run: Cross-Compilation Setup

If you want to do actual IRIX builds (not just rules development), run the one-time setup after installing the toolchain:

```bash
uv run mogrix setup-cross               # deploys compiler wrappers and headers
./scripts/build-runtime-objects.sh      # builds CRT, dlmalloc, compat libs
```

See `docs/setup-guide.md` for the complete walkthrough from a fresh Ubuntu VM.

---

## 3. Running Tests

```bash
uv run pytest
```

The test root is `tests/` (configured in `pyproject.toml` under `[tool.pytest.ini_options]`). Test files follow the `test_*.py` naming convention.

With coverage:

```bash
uv run pytest --cov=mogrix
```

The `tests/` directory contains 94 files (8,390 lines), making it one of the larger parts of the codebase — a good first place to understand expected behavior for any subsystem.

---

## 4. Project Structure

```
mogrix/          Python package — all application logic
tests/           pytest test suite (94 files)
rules/           YAML rule files (365 files, ~24K lines) — the main data layer
  packages/      Per-package conversion rules (one .yaml per package)
  generic.yaml   Rules that apply to all packages
  methods/       Markdown documentation for build methods
  transforms/    Source transform rule files
  candidates/    Auto-generated candidate rules awaiting human review
compat/          C source implementations of missing POSIX/C99 functions (80 files)
  catalog.yaml   Index of all compat functions — consult before implementing anything
cross/           IRIX cross-compilation toolchain (97 files)
  Compiler wrappers (irix-cc, irix-c++), linker scripts, sysroot config
patches/         Patch files applied during SRPM conversion (171 files)
headers/         IRIX-specific header overlays (18 files)
scripts/         Shell scripts for environment setup and toolchain builds (13 files)
  build-lld-irix.sh          Builds the patched LLD linker
  build-runtime-objects.sh   Builds CRT, dlmalloc, compat .o files
  cleanup.sh                 Resets the staging environment from scratch
docs/            Documentation (8 files, including setup-guide.md)
specs/           Generated specification documents (29 files — respec output)
analysis/        Tools and data for IRIX-incompatibility pattern analysis (6 files)
tools/           Miscellaneous utility scripts (46 files)
configs/         rpmlint configuration and similar build configs (4 files)
lld-fixes/       LLD patch files for IRIX linker compatibility (6 files)
```

### Runtime Directories (outside the repo)

| Path | Purpose |
|------|---------|
| `~/mogrix_inputs/SRPMS/` | Original Fedora 40 SRPMs (read-only inputs) |
| `~/mogrix_outputs/SRPMS/` | Converted SRPMs |
| `~/mogrix_outputs/RPMS/` | Known-good built RPMs |
| `~/rpmbuild/` | Ephemeral rpmbuild workspace — do not rely on its contents persisting |
| `/opt/sgug-staging/` | Cross-compilation staging area |
| `/opt/cross/bin/` | Cross toolchain binaries |
| `/opt/irix-sysroot/` | IRIX system headers and libraries |

---

## 5. Entry Point and Key Abstractions

### Entry Point

`mogrix/cli.py` — the `main()` function is the registered entry point (`mogrix = "mogrix.cli:main"` in `pyproject.toml`). Every CLI command is a Click command registered on the main group there.

### Main Pipeline (SRPM Workflow)

The core data flow for package conversion:

```
SRPM (Fedora 40)
  → mogrix fetch     # downloads from Fedora repo
  → mogrix convert   # applies rules from rules/packages/<pkg>.yaml
                     # injects compat functions from compat/
                     # applies patches from patches/
                     # → outputs a converted SRPM
  → mogrix build --cross   # invokes rpmbuild with cross-compilation wrappers
                            # compiler wrappers from cross/ are used
  → mogrix stage     # extracts RPM into /opt/sgug-staging for dependent builds
```

### Key Abstractions

**Rules** (`rules/packages/*.yaml`, `rules/generic.yaml`): The primary data layer. These YAML files declare what transformations to apply to each package — dropped dependencies, injected compat functions, configure flags, autoconf cache overrides, line removals. Adding support for a new package means writing a YAML file, not Python code.

**Compat Library** (`compat/`): Drop-in C implementations of 35+ POSIX/C99 functions missing from IRIX 6.5. `compat/catalog.yaml` is the index. Before writing a new compat function, check whether it already exists here.

**Compiler Wrappers** (`cross/`): Shell or C wrappers named `irix-cc`, `irix-c++`, etc. They intercept compiler invocations and inject IRIX-specific flags (`--target mips-sgi-irix6.5`, sysroot, header paths, linker options). rpmbuild uses these transparently.

**Source Analysis** (`analysis/`): YAML-defined ripgrep patterns that scan source tarballs for known IRIX-incompatible constructs (`%zu`, `__thread`, `epoll`, etc.). Adding a new incompatibility check requires no Python changes — add a pattern to the YAML.

**App Bundle System** (`mogrix bundle`): Resolves shared library dependencies via ELF `readelf -d` NEEDED scanning, creates self-contained tarballs with a `./install` script. The nano bundle is under 1 MB.

**Staging System** (`mogrix stage`): Extracts RPM contents into `/opt/sgug-staging/usr/sgug/` so that subsequent package builds find headers (`-I`) and libraries (`-L`) from previously built packages.

---

## 6. Development Workflow

### Branch Naming

CI is currently disabled (`.github_disabled/` contains workflows that are not active). No enforced branch naming convention is in place. Conventional practice: use `feature/<package>` for new package rules and `fix/<package>` for build fixes.

### Adding a New Package

1. Write a rule file in `rules/packages/<package>.yaml` (see README for schema).
2. Check `compat/catalog.yaml` for any missing functions the package needs.
3. Run `uv run mogrix analyze <srpm>` to see which rules would apply and which IRIX-incompatible patterns are present.
4. Run `uv run mogrix convert <srpm>` and then `mogrix build <converted-srpm> --cross`.
5. If compat functions are needed but missing, implement them in `compat/` and add to `catalog.yaml`.

### PR Process

No automated CI is running (the GitHub Actions workflows are in `.github_disabled/`). Code review is manual. Since this is a cross-compilation system for a 2006-discontinued OS, testing requires actual IRIX hardware or an emulator — document your test results in the PR description.

### Validation Commands

```bash
uv run mogrix validate-rules    # validate all rule YAML files
uv run mogrix audit-rules       # find duplicates and class candidates
uv run mogrix lint <rpms>       # rpmlint with IRIX config
```

---

## 7. Common Pitfalls

### Hardcoded mcm-engine Path

`pyproject.toml` references `mcm-engine` at `/home/edodd/projects/github/unxmaal/mcm-engine`. `uv sync` will fail on any machine where this path doesn't exist. Update the path before your first install.

### Fedora 40 Specifically

SRPMs must come from **Fedora 40**. The rules and compat catalog were built against that release's spec files and dependency names. SRPMs from other Fedora versions may produce unexpected failures.

### LLD Must Be Patched

The upstream LLD 18 has an IRIX-specific linker bug. Using an unpatched LLD produces silently broken binaries (they link but crash at runtime). Always build LLD via `scripts/build-lld-irix.sh`.

### The IRIX Sysroot Cannot Be Generated

`/opt/irix-sysroot/` must be copied from a real IRIX 6.5.x machine. There is no synthetic substitute. Without it, `setup-cross` will fail and no cross-compilation will work.

### Do Not Edit Spec Files Directly

The README is explicit: *"Direct spec file editing should be avoided."* All customization belongs in `rules/packages/<pkg>.yaml`. Edited spec files will be overwritten the next time `mogrix convert` runs.

### The rpmbuild Workspace Is Ephemeral

`~/rpmbuild/` is written and overwritten by every build. Do not store anything there that isn't also archived to `~/mogrix_outputs/RPMS/`. The `cleanup.sh` script wipes and rebuilds the staging environment from scratch — run it only when you intend to reset everything.

### Compat Check Before Implementing

Before writing a new compat function, run:

```bash
# Check catalog.yaml for existing implementations
grep -r "symbol_name" compat/catalog.yaml
```

Many POSIX functions missing from IRIX are already implemented in `compat/`. Duplicate implementations cause linker conflicts.

### Clang Must Have the MIPS Target

Distro-packaged Clang (e.g., `apt install clang`) is typically built without the MIPS backend. Running `mogrix build --cross` with such a Clang produces a "MIPS target not registered" error that looks like a mogrix bug but is a toolchain build issue. Build or download an upstream LLVM release with MIPS enabled.

