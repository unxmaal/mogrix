# Test Coverage Map — Mogrix

## 1. Test Frameworks

| Tool | Version | Role |
|------|---------|------|
| **pytest** | ≥7.0 (lockfile pins ≥9.0.2) | Primary test runner |
| **pytest-cov** | ≥4.0 | Coverage reporting (optional dep) |

Configuration lives in `pyproject.toml` under `[tool.pytest.ini_options]`:

```toml
testpaths = ["tests"]
python_files = ["test_*.py"]
python_functions = ["test_*"]
```

No `conftest.py` was found; shared fixtures, if any, must live inside individual test files.

---

## 2. Tested Areas

The `tests/` directory contains 94 files totalling ~8,400 lines. Named test modules and their inferred scope:

### Spec-file pipeline (parse → transform → write → emit)

| File | Scope |
|------|-------|
| `test_spec_parser.py` | Parsing raw `.spec` file text into structured AST/model |
| `test_spec_primitives.py` | Low-level spec field types: tags, macros, version strings, flags |
| `test_sections.py` | Spec section handling: `%prep`, `%build`, `%install`, `%files`, `%changelog` |
| `test_subpackages.py` | Multi-package specs: `%package`, `%description`, `%files` per subpackage |
| `test_patches.py` | Patch declarations, `%patch` application macros, patch ordering |
| `test_conditionals.py` | `%if`/`%ifarch`/`%ifos` conditional blocks: comment-out and evaluation |
| `test_spec_writer.py` | Reconstructing `.spec` text from the internal model |
| `test_sections.py` | Section boundary detection and preservation |

### SRPM I/O

| File | Scope |
|------|-------|
| `test_srpm_handling.py` | Reading SRPMs: extracting spec, sources, patches from the RPM archive |
| `test_srpm_emitter.py` | Writing SRPMs: repackaging spec + sources into a valid `.src.rpm` |
| `test_emitter.py` | General emitter logic shared across SRPM/RPM output paths |
| `test_roundtrip_fidelity.py` | End-to-end round-trip: read SRPM → convert → re-read; asserts spec integrity |
| `test_rebuild.py` | Re-conversion of an already-converted SRPM; regression guard |

### Rule engine

| File | Scope |
|------|-------|
| `test_rule_loader.py` | Loading YAML rule files from `rules/packages/`, `rules/generic.yaml` |
| `test_rule_engine.py` | Applying rules to a spec model: drop deps, inject lines, set ac_cv overrides, etc. |

### Compat injection

| File | Scope |
|------|-------|
| `test_compat_injector.py` | Injecting compat function declarations and linker flags into spec `%build`/`BuildRequires` |
| `test_header_overlay.py` | Header overlay directories wired into compiler `-I` paths for compat headers |

### Dependency and roadmap

| File | Scope |
|------|-------|
| `test_deps.py` | Dependency detection: parsing `BuildRequires`, classifying as system/rules/unsupported |
| `test_roadmap.py` | Transitive dependency graph construction from rule files |
| `test_roadmap_check.py` | Roadmap consistency checks (cycle detection, missing rule references) |

### Validation and analysis

| File | Scope |
|------|-------|
| `test_spec_validator.py` | Spec structural validation: required tags, section ordering, mandatory fields |
| `test_validator.py` | General validation pass (likely wraps both spec and rule validation) |
| `test_source_analyzer.py` | Ripgrep-based source scanning for IRIX-incompatible patterns (`%zu`, `__thread`, epoll, etc.) |

### CLI and batch

| File | Scope |
|------|-------|
| `test_cli.py` | CLI command dispatch: `convert`, `analyze`, `validate-spec`, `list-rules`, etc. |
| `test_batch.py` | Batch SRPM conversion: directory walking, error isolation, output routing |

### RLD subsystem

| File | Scope |
|------|-------|
| `tests/rld/test_manifest.py` | RLD manifest format (a separate sub-module under `tests/rld/`) |

---

## 3. Untested Areas

Modules inferred from the README's CLI command table and the `mogrix/` package (50 files) with no corresponding test file:

| Functional Area | CLI Command | Notes |
|----------------|------------|-------|
| App bundle creation | `mogrix bundle` | ELF dependency scanning, terminfo pruning, install-script generation — no `test_bundle.py` |
| RPM staging | `mogrix stage` | Extracts RPM contents to `/opt/sgug-staging` — no `test_stage.py` |
| SRPM fetching | `mogrix fetch` | Fedora repository HTTP download, interactive selection — no `test_fetch.py` |
| Upstream SRPM creation | `mogrix create-srpm` | Git/tarball → spec generation for autoconf/cmake/meson/makefile projects — no `test_create_srpm.py` |
| Source transforms | `mogrix transform` | Declarative YAML text replacements, block removal, postcondition checks — no `test_transform.py` |
| Rust crate patching | `mogrix patch-crates` | Cargo registry patching for IRIX cross-compilation — no `test_patch_crates.py` |
| Cross-compilation setup | `mogrix setup-cross` | Deploys compiler wrappers, sysroot configuration — no `test_setup_cross.py` |
| Batch build pipeline | `mogrix batch-build` | Topological build ordering, candidate YAML generation, failure isolation — no `test_batch_build.py` (distinct from `test_batch.py` which covers batch *conversion*) |
| RPM linting | `mogrix lint` | rpmlint integration with IRIX-specific config — no `test_lint.py` |
| Rule management commands | `mogrix audit-rules`, `mogrix score-rules`, `mogrix list-rules` | No tests found |
| Header sync | `mogrix sync-headers` | No `test_sync_headers.py` |
| Bootstrap tarball | (internal) | Self-contained tdnf deployment archive — no tests found |
| Cross-compilation integration | `mogrix build --cross` | Full rpmbuild invocation with IRIX toolchain wrappers — no integration test |

The `scripts/` directory (13 files, ~2,900 lines) — shell scripts for LLD build, runtime object build, cleanup — has no automated tests.

The `compat/` directory (80 files, ~7,600 lines) — C source implementations of the 35+ compat functions — has no unit tests. Correctness is validated only indirectly when a cross-compiled package runs on IRIX.

---

## 4. Test Execution

### Prerequisites

```bash
# Install dependencies (includes pytest)
uv sync --extra dev
```

### Run all tests

```bash
uv run pytest
# or, inside the venv:
pytest tests/
```

### Run with coverage

```bash
uv run pytest --cov=mogrix --cov-report=term-missing
```

### Run a specific file

```bash
uv run pytest tests/test_rule_engine.py -v
```

### Run the RLD sub-suite

```bash
uv run pytest tests/rld/ -v
```

### Special setup requirements

- Tests that exercise SRPM I/O (test_srpm_handling, test_srpm_emitter, test_roundtrip_fidelity, test_rebuild) require `rpmbuild` installed on the host. They will likely skip or fail on machines without RPM tooling.
- Tests that invoke cross-compilation toolchain paths (`/opt/cross/bin/`, `/opt/irix-sysroot/`) require the sysroot and clang/LLD setup described in `docs/setup-guide.md`. No test stubs or mocks for these paths were identified.
- `test_source_analyzer.py` invokes `ripgrep` (`rg`) as a subprocess; `rg` must be on `PATH`.

---

## 5. Known Test Issues

No tests marked `@pytest.mark.skip`, `@pytest.mark.xfail`, or `# type: ignore`-style workarounds were identified in the provided file listing. The test file names are straightforward with no `_broken`, `_wip`, or `_disabled` suffixes.

One structural gap worth flagging: the spec-file pipeline has six test files covering parsing, primitives, sections, subpackages, patches, and writing — but no dedicated test for the `%changelog` section or for multi-line macro expansion. These are common sources of round-trip fidelity bugs.

---

## 6. CI Pipeline

| Mode | Status |
|------|--------|
| Automated (GitHub Actions) | **Disabled** — `.github_disabled/` directory present (1 file, ~100 lines); renamed to prevent execution |
| Manual | `uv run pytest` locally |

There is no active CI. The `.github_disabled/` configuration was written but is not running. All test execution is currently manual. There is no lint step, no build step, and no cross-compilation smoke test in the pipeline.

