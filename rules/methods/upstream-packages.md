# Upstream Packages (Non-Fedora Sources)

Mogrix isn't limited to Fedora SRPMs. Any package with a git repo or tarball can enter the pipeline via `mogrix create-srpm`.

## When to Use

- Package not in Fedora (e.g., telescope, gmi100, gmid)
- Fedora version is too old/new and you want a specific upstream version
- Package is only available as a git repo or tarball download

## How It Works

1. Add an `upstream:` block to the package's rules YAML
2. Run `mogrix create-srpm <package>` to generate an SRPM
3. Continue with normal `mogrix convert` → `mogrix build --cross` pipeline

The `upstream:` block tells mogrix where to get the source and how to build it. Mogrix generates a spec file from a template (or uses a hand-written one), fetches the source, and packages everything into an SRPM.

## Package YAML Schema

Add `upstream:` as a top-level key alongside `rules:`:

```yaml
package: telescope
upstream:
  url: https://github.com/telescope-browser/telescope
  version: "0.9"
  ref: "0.9"               # git tag/branch (default: version)
  type: git                 # "git" or "tarball" (inferred from url)
  build_system: autoconf    # "autoconf", "cmake", "meson", "makefile"
  license: ISC
  summary: "Gemini, Gopher, and Finger TUI browser"
  source_dir: telescope-0.9  # directory inside tarball (default: name-version)
rules:
  configure_flags:
    add:
      - --without-libbsd
```

### Required Fields

| Field | Description |
|-------|-------------|
| `url` | Git repo URL or tarball download URL |
| `version` | Package version string |
| `build_system` | One of: `autoconf`, `cmake`, `meson`, `makefile` |

### Optional Fields

| Field | Default | Description |
|-------|---------|-------------|
| `ref` | Same as version | Git tag, branch, or commit hash |
| `type` | Inferred from URL | `git` (clone + archive) or `tarball` (download) |
| `license` | "Unknown" | SPDX license identifier |
| `summary` | Package name | One-line description |
| `source_dir` | `name-version` | Directory name inside the tarball |

## Spec Templates

Mogrix generates specs from templates in `specs/templates/`:

| Build System | Template | Build Commands |
|-------------|----------|----------------|
| `autoconf` | `autoconf.spec` | `%configure` + `%make_build` + `%make_install` |
| `cmake` | `cmake.spec` | `%cmake` + `%cmake_build` + `%cmake_install` |
| `meson` | `meson.spec` | `%meson` + `%meson_build` + `%meson_install` |
| `makefile` | `makefile.spec` | `%make_build` + `%make_install PREFIX=%{_prefix} DESTDIR=%{buildroot}` |

Templates produce minimal specs. Mogrix rules handle the rest (compat injection, flag tweaking, etc.) during `mogrix convert`.

### Hand-Written Spec Override

For complex packages, put a hand-written spec at `specs/packages/<name>.spec`. If this file exists, `create-srpm` uses it instead of the template.

## Full Workflow Example

```bash
# 1. Create the package rules file with upstream: block
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

# 3. Normal pipeline continues
uv run mogrix convert ~/mogrix_inputs/SRPMS/gmi100-1.0-1.src.rpm
uv run mogrix build ~/mogrix_outputs/SRPMS/.../gmi100-1.0-1.src.rpm --cross
uv run mogrix stage ~/rpmbuild/RPMS/mips/gmi100*.rpm

# 4. Bundle for distribution
uv run mogrix bundle gmi100
```

## Suite Bundles

Combine multiple packages into a single bundle:

```bash
# Named suite
mogrix bundle telescope snownews lynx --name mogrix-smallweb

# Auto-named suite (uses first package name + "-suite")
mogrix bundle telescope snownews lynx
```

Suite bundles share libraries between all included packages, deduplicate sonames, and generate a single install/uninstall script covering all binaries.
