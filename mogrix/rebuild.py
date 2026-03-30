"""Rebuild-all orchestration — versioned workspaces + dependency-ordered rebuilds.

Each full rebuild gets its own workspace (~/mogrix_v10, ~/mogrix_v11, etc.).
--resume reuses the latest workspace. New rebuilds auto-increment.

Workspace layout:
    ~/mogrix_v10/
        gate-results/<pkg>/build-gate.json
        mogrix_outputs/SRPMS/
        mogrix_outputs/RPMS/
        rpmbuild/<pkg>/          (cleaned BEFORE each build, preserved after)
        rpmbuild/<pkg>/logs/     (build logs)

Gate 0: Ensures clean build environment (staging reset + cross-compilation setup).
Gate 2: Validates build outputs (shebangs, ELF ABI, paths) after each build.
"""

import json
import os
import re
import signal
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path

from rich.console import Console

from mogrix.gates import GateResult, pre_scan_rpms

# force_terminal=False disables Rich buffering when stdout is redirected to a file
console = Console(force_terminal=False)


def _log(msg: str) -> None:
    """Print and flush immediately so redirected output streams in real-time."""
    console.print(msg)
    sys.stdout.flush()


# Fixed directories
MOGRIX_INPUTS = Path.home() / "mogrix_inputs"
_PROJECT_ROOT = Path(__file__).parent.parent.resolve()
STAGING_DIR = _PROJECT_ROOT / "staging"
WORKSPACE_PREFIX = "mogrix_v"

# Use venv mogrix binary directly to avoid uv lock contention
# when rebuild-all is itself invoked via `uv run mogrix`.
MOGRIX_BIN = Path(__file__).parent.parent / ".venv" / "bin" / "mogrix"

# Ensure staging env vars are set for all subprocesses
import os as _os
_os.environ.setdefault("MOGRIX_STAGING", str(STAGING_DIR / "opt" / "mogrix"))
_os.environ.setdefault("MOGRIX_STAGING_ROOT", str(STAGING_DIR))
_os.environ.setdefault("LLVM_PROJECT", str(_PROJECT_ROOT / "tmp" / "llvm-build" / "llvm-project-22.1.2.src"))


def find_latest_workspace() -> Path | None:
    """Find the highest-numbered ~/mogrix_v* workspace."""
    home = Path.home()
    workspaces = []
    for d in home.iterdir():
        if d.is_dir() and d.name.startswith(WORKSPACE_PREFIX):
            try:
                version = int(d.name[len(WORKSPACE_PREFIX):])
                workspaces.append((version, d))
            except ValueError:
                pass
    if not workspaces:
        return None
    workspaces.sort()
    return workspaces[-1][1]


def _update_outputs_symlink(workspace: Path) -> None:
    """Ensure ~/mogrix_outputs symlink points to this workspace's outputs.

    Called on both create and resume so the bundler always reads the right RPMs.
    """
    outputs_link = Path.home() / "mogrix_outputs"
    ws_outputs = workspace / "mogrix_outputs"

    try:
        if outputs_link.is_symlink():
            current = outputs_link.resolve()
            if current == ws_outputs.resolve():
                return  # already correct
            outputs_link.unlink()
        elif outputs_link.is_dir():
            backup = outputs_link.with_name(
                f"mogrix_outputs_old.{time.strftime('%m%d%H%M')}"
            )
            outputs_link.rename(backup)
            _log(f"[dim]Backed up {outputs_link} → {backup.name}[/dim]")
        outputs_link.symlink_to(ws_outputs)
        _log(f"[dim]~/mogrix_outputs → {ws_outputs}[/dim]")
    except OSError as e:
        _log(f"[yellow]Warning: could not update ~/mogrix_outputs symlink: {e}[/yellow]")


def _validate_outputs_symlink(workspace: Path) -> None:
    """Pre-flight: verify ~/mogrix_outputs points to this workspace.

    Hard failure if:
    - ~/mogrix_outputs doesn't exist
    - ~/mogrix_outputs is not a symlink (it's a real directory)
    - ~/mogrix_outputs points to a different workspace

    On mismatch, offers to fix it. On missing, creates it.
    """
    outputs_link = Path.home() / "mogrix_outputs"
    ws_outputs = workspace / "mogrix_outputs"

    if not outputs_link.exists() and not outputs_link.is_symlink():
        # Missing entirely — create it
        _log(f"[dim]Creating ~/mogrix_outputs → {ws_outputs}[/dim]")
        outputs_link.symlink_to(ws_outputs)
        return

    if not outputs_link.is_symlink():
        _log(f"[red]Pre-flight FAILED: ~/mogrix_outputs is a real directory, not a symlink[/red]")
        _log(f"[red]Expected: symlink → {ws_outputs}[/red]")
        _log(f"[yellow]Move or remove ~/mogrix_outputs and retry.[/yellow]")
        raise SystemExit(1)

    current_target = outputs_link.resolve()
    expected_target = ws_outputs.resolve()
    if current_target != expected_target:
        _log(f"[red]Pre-flight FAILED: ~/mogrix_outputs symlink mismatch[/red]")
        _log(f"  [red]Points to:[/red] {current_target}")
        _log(f"  [red]Expected:[/red]  {expected_target}")
        _log(f"\n[yellow]Fix with: ln -sfn {ws_outputs} ~/mogrix_outputs[/yellow]")
        raise SystemExit(1)

    _log(f"[dim]~/mogrix_outputs → {ws_outputs} ✓[/dim]")


def _validate_staging_owner(staging_dir: Path, workspace: Path, resume: bool) -> None:
    """Pre-flight: verify staging-owner.json matches this workspace.

    On resume: hard error if staging belongs to a different workspace.
    On new build: write ownership (gate0 will also write it, but this catches it early).
    """
    owner_file = staging_dir / "staging-owner.json"

    if resume:
        if owner_file.exists():
            try:
                data = json.loads(owner_file.read_text())
                recorded = data.get("workspace", "")
                if recorded and Path(recorded).resolve() != workspace.resolve():
                    _log(f"[red]Pre-flight FAILED: staging owned by different workspace[/red]")
                    _log(f"  [red]Staging owner:[/red] {recorded}")
                    _log(f"  [red]This workspace:[/red] {workspace}")
                    _log(f"\n[yellow]Either use --workspace {recorded} or run a fresh (non-resume) build.[/yellow]")
                    raise SystemExit(1)
            except (json.JSONDecodeError, OSError):
                pass
        _log(f"[dim]Staging owner: {workspace} ✓[/dim]")
    else:
        # New build — claim staging ownership
        _write_staging_owner(staging_dir, workspace)
        _log(f"[dim]Staging owner set to {workspace}[/dim]")


def create_workspace() -> Path:
    """Create the next numbered workspace (auto-increment).

    DEPRECATED: Only used by legacy code. rebuild-all now requires --workspace.
    """
    latest = find_latest_workspace()
    if latest:
        current_version = int(latest.name[len(WORKSPACE_PREFIX):])
        next_version = current_version + 1
    else:
        next_version = 11  # Start at 11 (v10 is the migration from legacy)

    workspace = Path.home() / f"{WORKSPACE_PREFIX}{next_version}"
    workspace.mkdir(exist_ok=True)
    (workspace / "gate-results").mkdir(exist_ok=True)
    (workspace / "mogrix_outputs" / "SRPMS").mkdir(parents=True, exist_ok=True)
    (workspace / "mogrix_outputs" / "RPMS").mkdir(parents=True, exist_ok=True)
    (workspace / "rpmbuild").mkdir(exist_ok=True)

    _update_outputs_symlink(workspace)

    _log(f"[bold green]Created workspace: {workspace}[/bold green]")
    return workspace


@dataclass
class RebuildResult:
    """Result of rebuilding a single package."""
    package: str
    success: bool
    gate2_passed: bool = True
    error: str = ""
    rpms: list[str] = field(default_factory=list)
    duration_s: float = 0.0
    # Postcondition check results
    postcondition_errors: list[str] = field(default_factory=list)
    postcondition_warnings: list[str] = field(default_factory=list)
    # IRIX smoke test results
    smoke_test: str = ""  # "pass", "fail", "skipped", "unreachable", or ""


@dataclass
class RebuildPlan:
    """Full rebuild plan with dependency ordering."""
    build_order: list[str]
    cycles: list[list[str]]
    total: int = 0
    results: dict[str, RebuildResult] = field(default_factory=dict)
    gate_results_dir: Path = field(default_factory=lambda: Path("gate-results"))

    def __post_init__(self):
        self.total = len(self.build_order)


def compute_build_order(rules_dir: Path, rpms_dir: Path) -> RebuildPlan:
    """Compute global dependency-ordered build plan."""
    from mogrix.repometa import RepoMetaCache
    from mogrix.roadmap import RoadmapResolver
    from mogrix.rules.loader import RuleLoader

    cache = RepoMetaCache(release="40")
    db = cache.ensure_index(refresh=False)
    loader = RuleLoader(rules_dir)
    resolver = RoadmapResolver(
        db=db,
        rule_loader=loader,
        rules_dir=rules_dir,
        rpms_dir=rpms_dir,
        stop_at_rules=True,
    )
    build_order, cycles = resolver.resolve_all()
    db.close()
    return RebuildPlan(build_order=build_order, cycles=cycles)


def reset_staging(staging_dir: Path) -> None:
    """Reset staging to pristine base via mogrix stage --clean."""
    _log("[bold]Resetting staging to pristine base...[/bold]")
    result = subprocess.run(
        [str(MOGRIX_BIN), "stage", "--clean", "--staging-dir", str(staging_dir)],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        _log(f"[red]stage --clean failed:[/red] {result.stderr}")
        raise SystemExit(1)
    _log("[green]Staging reset complete[/green]")


def setup_cross() -> None:
    """Re-deploy cross-compilation tools."""
    _log("[bold]Re-deploying cross-compilation setup...[/bold]")
    result = subprocess.run(
        [str(MOGRIX_BIN), "setup-cross"],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        _log(f"[red]setup-cross failed:[/red] {result.stderr}")
        raise SystemExit(1)
    _log("[green]Cross-compilation setup deployed[/green]")


def _write_staging_owner(staging_dir: Path, workspace: Path) -> None:
    """Record which workspace owns staging, to prevent cross-workspace pollution."""
    owner_file = staging_dir / "staging-owner.json"
    tmp = staging_dir / ".staging-owner.json.tmp"
    tmp.write_text(json.dumps({
        "workspace": str(workspace),
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S"),
    }))
    os.rename(str(tmp), str(owner_file))


def _check_staging_owner(staging_dir: Path, workspace: Path) -> None:
    """Abort if a different workspace owns staging."""
    owner_file = staging_dir / "staging-owner.json"
    if not owner_file.exists():
        return  # no owner recorded yet
    try:
        data = json.loads(owner_file.read_text())
        recorded = data.get("workspace", "")
        if recorded and Path(recorded).resolve() != workspace.resolve():
            _log(
                f"[red]Staging is owned by {recorded}, not {workspace}.[/red]\n"
                f"[red]Run a full rebuild (without --resume) to reset, or "
                f"use --workspace {recorded} to resume that workspace.[/red]"
            )
            raise SystemExit(1)
    except (json.JSONDecodeError, OSError):
        pass  # corrupt file — ignore and proceed


def gate0_clean_slate(
    staging_dir: Path,
    workspace: Path,
    resume: bool = False,
) -> None:
    """Gate 0: Establish clean build environment.

    Resets staging and re-deploys cross-compilation tools.
    Workspace directories are never wiped — they're versioned.
    """
    _log("\n[bold]=== Gate 0: Clean Slate ===[/bold]\n")

    if resume:
        _log("[dim]--resume: keeping existing staging, re-deploying tools only[/dim]")
        _check_staging_owner(staging_dir, workspace)
        setup_cross()  # ensure toolchain is current
    else:
        reset_staging(staging_dir)
        setup_cross()
        _write_staging_owner(staging_dir, workspace)

    _log("\n[bold green]Gate 0 passed: clean slate established[/bold green]\n")


def _package_input_files(package: str, rules_dir: Path) -> list[tuple[str, Path]]:
    """All input files that affect a package's build output.

    Returns (key, path) pairs. If any of these change, the cached
    conversion and build results are stale.
    """
    project_root = Path(__file__).parent.parent
    inputs = [
        ("pkg_rule", rules_dir / "packages" / f"{package}.yaml"),
        ("generic", rules_dir / "generic.yaml"),
        ("compat_catalog", project_root / "compat" / "catalog.yaml"),
        # Global inputs: compiler/linker changes affect all packages
        ("irix_cc", project_root / "cross" / "bin" / "irix-cc"),
        ("irix_ld", project_root / "cross" / "bin" / "irix-ld"),
    ]
    # Package-specific patches directory — use the dir's own mtime
    # (updated when files are added/removed)
    patches_dir = project_root / "patches" / "packages" / package
    if patches_dir.is_dir():
        inputs.append(("patches_dir", patches_dir))
        # Also check individual patch files
        for patch in sorted(patches_dir.iterdir()):
            if patch.is_file():
                inputs.append((f"patch_{patch.name}", patch))
    return inputs


def _input_content_hash(path: Path) -> str:
    """SHA256 hex digest of file content, or empty string if missing.

    For directories, hashes the sorted list of filenames (detects adds/removes).
    """
    import hashlib
    if not path.exists():
        return ""
    if path.is_dir():
        # Hash the sorted directory listing — detects file adds/removes
        entries = sorted(f.name for f in path.iterdir() if f.is_file())
        return hashlib.sha256("\n".join(entries).encode()).hexdigest()
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _package_input_hashes(package: str, rules_dir: Path) -> dict[str, str]:
    """Content hashes for all inputs that affect a package's build."""
    return {
        key: _input_content_hash(path)
        for key, path in _package_input_files(package, rules_dir)
    }


def _inputs_changed(current_hashes: dict[str, str], stored: dict) -> bool:
    """Check if inputs changed. Handles both hash-based and legacy mtime stamps.

    If the stored stamp has 'input_hashes', compare content hashes.
    Otherwise fall back to mtime comparison for backward compatibility.
    """
    stored_hashes = stored.get("input_hashes")
    if stored_hashes:
        # Hash-based comparison: any hash mismatch = stale
        for key, current_hash in current_hashes.items():
            if current_hash != stored_hashes.get(key, ""):
                return True
        # Also check for removed inputs (key in stored but not in current)
        for key in stored_hashes:
            if key not in current_hashes and stored_hashes[key]:
                return True
        return False

    # Legacy fallback: no hashes stored, use mtime comparison
    # This handles gate-results/stamps written before the hash migration.
    # The stamp file's own mtime serves as the "build time" reference.
    return False  # trust legacy stamps — they'll get hash-upgraded on next build


def _conversion_is_stale(package: str, ws_srpms_dir: Path, rules_dir: Path) -> bool:
    """Check if a cached converted SRPM is stale because inputs changed.

    Uses content hashes (SHA256) to detect real changes. A cosmetic touch
    or git checkout that doesn't change file content won't trigger reconversion.
    Falls back to trusting legacy mtime stamps until they're upgraded.
    """
    stamp_file = ws_srpms_dir / f".{package}.convert-stamp"
    if not stamp_file.exists():
        return True  # no stamp = unknown provenance, reconvert to be safe

    try:
        stamp = json.loads(stamp_file.read_text())
    except (json.JSONDecodeError, OSError):
        return True  # corrupt stamp = stale

    current_hashes = _package_input_hashes(package, rules_dir)
    return _inputs_changed(current_hashes, stamp)


def _write_convert_stamp(package: str, ws_srpms_dir: Path, rules_dir: Path) -> None:
    """Write a conversion stamp with content hashes of all input files."""
    stamp = {
        "input_hashes": _package_input_hashes(package, rules_dir),
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S"),
    }
    stamp_file = ws_srpms_dir / f".{package}.convert-stamp"
    tmp = ws_srpms_dir / f".{package}.convert-stamp.tmp"
    tmp.write_text(json.dumps(stamp))
    os.rename(str(tmp), str(stamp_file))


def _extract_spec_from_srpm(srpm_path: Path) -> str | None:
    """Extract the .spec file content from an SRPM. Returns content or None."""
    import tempfile
    with tempfile.TemporaryDirectory() as tmpdir:
        # Extract just the spec file
        subprocess.run(
            f"cd {tmpdir} && rpm2cpio {srpm_path} | cpio -idm '*.spec' 2>/dev/null",
            shell=True, capture_output=True,
        )
        specs = list(Path(tmpdir).rglob("*.spec"))
        if specs:
            return specs[0].read_text(errors="replace")
    return None


def _verify_conversion_applied(
    package: str,
    original_srpm: Path,
    converted_srpm: Path,
    rules_dir: Path,
    rebuild_result: RebuildResult | None = None,
) -> bool:
    """Verify the conversion actually applied the declared rules.

    Invariant violations (Linux-isms) are hard errors — return False to fail the build.
    Rule postcondition mismatches are warnings — logged but don't fail.

    Returns True if OK to proceed, False if invariant violations found.
    """
    import yaml as _yaml
    from mogrix.postconditions import check_postconditions

    converted_spec = _extract_spec_from_srpm(converted_srpm)
    if not converted_spec:
        return True

    # Load the package rules
    rule_file = rules_dir / "packages" / f"{package}.yaml"
    rules = {}
    if rule_file.exists():
        try:
            with open(rule_file) as f:
                pkg_data = _yaml.safe_load(f) or {}
            rules = pkg_data.get("rules", {})
        except Exception:
            pass

    report = check_postconditions(package, rules, converted_spec)

    # Split: invariant violations are hard errors, rule postconditions are warnings
    invariant_errors = [
        f"{i.rule_type}: {i.message}" for i in report.issues
        if i.severity == "error" and i.rule_type == "invariant"
    ]
    rule_errors = [
        f"{i.rule_type}: {i.message}" for i in report.issues
        if i.severity == "error" and i.rule_type != "invariant"
    ]
    warnings = [f"{i.rule_type}: {i.message}" for i in report.issues if i.severity == "warning"]

    if invariant_errors:
        _log(f"  [red]INVARIANT VIOLATIONS for {package}:[/red]")
        for msg in invariant_errors:
            _log(f"    [red]{msg}[/red]")
    if rule_errors:
        _log(f"  [yellow]Rule postcondition mismatches for {package}:[/yellow]")
        for msg in rule_errors:
            _log(f"    [yellow]{msg}[/yellow]")
    if warnings:
        for msg in warnings[:3]:  # cap at 3 to avoid log spam
            _log(f"  [dim]postcondition: {msg}[/dim]")
        if len(warnings) > 3:
            _log(f"  [dim]...and {len(warnings) - 3} more warnings[/dim]")

    if rebuild_result:
        rebuild_result.postcondition_errors = invariant_errors + rule_errors
        rebuild_result.postcondition_warnings = warnings

    return len(invariant_errors) == 0


def convert_package(package: str, rules_dir: Path, outputs_dir: Path) -> Path | None:
    """Convert a package SRPM. Returns path to converted SRPM or None.

    mogrix convert writes SRPMs to ~/mogrix_outputs/SRPMS/ (hardcoded).
    We copy the result into the workspace's SRPMS dir.
    """
    ws_srpms_dir = outputs_dir / "SRPMS"
    ws_srpms_dir.mkdir(parents=True, exist_ok=True)

    # Check if already converted in this workspace
    existing = sorted(ws_srpms_dir.glob(f"{package}-[0-9]*.src.rpm"))
    if existing:
        if _conversion_is_stale(package, ws_srpms_dir, rules_dir):
            _log(f"  [yellow]Rules changed — reconverting {package}[/yellow]")
            quarantine = ws_srpms_dir / ".quarantine" / time.strftime("%m%d%H%M%S")
            quarantine.mkdir(parents=True, exist_ok=True)
            for stale in existing:
                shutil.move(str(stale), str(quarantine / stale.name))
        else:
            return existing[-1]

    # Find the source SRPM
    srpms_dir = MOGRIX_INPUTS / "SRPMS"
    candidates = sorted(srpms_dir.glob(f"{package}-[0-9]*.src.rpm"))
    if not candidates:
        _log(f"  [red]No SRPM found for {package} in {srpms_dir}[/red]")
        return None

    srpm = candidates[-1]  # latest version

    result = subprocess.run(
        [str(MOGRIX_BIN), "convert", str(srpm)],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        _log(f"  [red]Convert failed:[/red] {result.stderr[-500:]}")
        return None

    # mogrix convert writes to ~/mogrix_outputs/SRPMS/ — copy to workspace
    # (may be the same dir if ~/mogrix_outputs is a symlink to the workspace)
    legacy_srpms = Path.home() / "mogrix_outputs" / "SRPMS"
    converted = sorted(legacy_srpms.glob(f"{package}-[0-9]*.src.rpm"))
    if not converted:
        _log(f"  [red]No converted SRPM found after convert[/red]")
        return None

    src = converted[-1]
    dst = ws_srpms_dir / src.name
    if src.resolve() != dst.resolve():
        shutil.copy2(src, dst)
    _write_convert_stamp(package, ws_srpms_dir, rules_dir)
    return dst


def build_package(
    package: str,
    converted_srpm: Path,
    outputs_dir: Path,
    rpmbuild_dir: Path,
    short_circuit: str | None = None,
) -> tuple[bool, list[Path], str]:
    """Build a converted SRPM. Returns (success, rpm_paths, error_msg).

    Each package gets its own rpmbuild directory under workspace/rpmbuild/<pkg>.
    The directory is cleaned BEFORE building (not after) so build artifacts
    are preserved for post-mortem inspection on failure.

    If short_circuit is set ('build', 'install', or 'binary'), the existing
    BUILD dir is preserved and rpmbuild runs with --short-circuit to skip %prep.
    """
    # Per-package rpmbuild directory — fully isolated from other builds
    pkg_rpmbuild = rpmbuild_dir / package

    if short_circuit:
        # Preserve existing BUILD dir — that's the whole point
        pkg_rpmbuild.mkdir(parents=True, exist_ok=True)
    else:
        # Clean BEFORE build (preserves artifacts from previous builds for inspection)
        if pkg_rpmbuild.exists():
            subprocess.run(["rm", "-rf", str(pkg_rpmbuild)], check=False)
        pkg_rpmbuild.mkdir(parents=True, exist_ok=True)

    # Per-package log directory
    log_dir = pkg_rpmbuild / "logs"
    log_dir.mkdir(parents=True, exist_ok=True)
    log_file = log_dir / f"{package}-build.log"

    out_rpms_dir = outputs_dir / "RPMS"
    out_rpms_dir.mkdir(parents=True, exist_ok=True)

    build_cmd = [
        str(MOGRIX_BIN), "build", str(converted_srpm), "--cross",
        "--rpmbuild-dir", str(pkg_rpmbuild),
        "--output-dir", str(out_rpms_dir),
        "--skip-dep-check",
    ]
    if short_circuit:
        build_cmd.extend(["--short-circuit", short_circuit])

    result = subprocess.run(
        build_cmd,
        capture_output=True,
        text=True,
        errors='replace',
    )

    # Save full output to log (ensure dir exists — mogrix build may recreate rpmbuild tree)
    log_file.parent.mkdir(parents=True, exist_ok=True)
    log_file.write_text(result.stdout + "\n" + result.stderr)

    if result.returncode != 0:
        # Extract last 5 lines of error
        lines = (result.stdout + result.stderr).strip().split("\n")
        error = "\n".join(lines[-5:])
        return False, [], error

    # Deterministic RPM detection: scan rpmbuild's RPMS/ dir for what was
    # actually produced, then match to copies in out_rpms_dir.
    # This handles subpackages (e.g. libcurl-devel from curl) and overwrites.
    rpmbuild_rpm_names = set()
    for rpmdir in pkg_rpmbuild.glob("RPMS/*"):
        for rpm in rpmdir.glob("*.rpm"):
            rpmbuild_rpm_names.add(rpm.name)

    if rpmbuild_rpm_names:
        new_rpms = sorted(
            out_rpms_dir / name for name in rpmbuild_rpm_names
            if (out_rpms_dir / name).exists()
        )
    else:
        # Fallback: name-based glob (rpmbuild dir cleaned or missing)
        m = re.match(r"^(.+?)-[\d]", converted_srpm.name)
        pkg_prefix = m.group(1) if m else package
        new_rpms = sorted(set(
            list(out_rpms_dir.glob(f"{pkg_prefix}-[0-9]*.mips.rpm"))
            + list(out_rpms_dir.glob(f"{pkg_prefix}-[0-9]*.noarch.rpm"))
        ))

    return True, new_rpms, ""


def gate2_check(rpms: list[Path]) -> GateResult:
    """Gate 2: Validate build outputs."""
    combined = GateResult()
    for rpm in rpms:
        result = pre_scan_rpms(rpm.parent)
        for name, gate_result in result.items():
            if rpm.name == name:
                for issue in gate_result.issues:
                    if issue.severity == "error":
                        combined.error(issue.file, issue.message)
                    else:
                        combined.warning(issue.file, issue.message)
    return combined


def stage_package(rpms: list[Path], staging_dir: Path) -> bool:
    """Stage built RPMs."""
    rpm_args = [str(r) for r in rpms]
    result = subprocess.run(
        [str(MOGRIX_BIN), "stage", "--staging-dir", str(staging_dir)] + rpm_args,
        capture_output=True,
        text=True,
    )
    return result.returncode == 0


def save_gate_result(
    gate_results_dir: Path,
    package: str,
    rebuild_result: RebuildResult,
    gate2_result: GateResult | None = None,
    input_hashes: dict[str, str] | None = None,
) -> None:
    """Save gate results for a package."""
    pkg_dir = gate_results_dir / package
    pkg_dir.mkdir(parents=True, exist_ok=True)

    data = {
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S"),
        "package": package,
        "build_success": rebuild_result.success,
        "gate2_passed": rebuild_result.gate2_passed,
        "duration_s": rebuild_result.duration_s,
        "rpms": rebuild_result.rpms,
        "error": rebuild_result.error,
    }

    if input_hashes:
        data["input_hashes"] = input_hashes

    if rebuild_result.postcondition_errors or rebuild_result.postcondition_warnings:
        data["postconditions"] = {
            "errors": rebuild_result.postcondition_errors,
            "warnings": rebuild_result.postcondition_warnings,
        }

    if rebuild_result.smoke_test:
        data["smoke_test"] = rebuild_result.smoke_test

    if gate2_result:
        data["gate2_issues"] = [
            {"severity": i.severity, "file": i.file, "message": i.message}
            for i in gate2_result.issues
        ]

    # Atomic write: tmp + rename prevents truncated JSON on crash/Ctrl-C
    tmp = pkg_dir / ".build-gate.json.tmp"
    tmp.write_text(json.dumps(data, indent=2))
    os.rename(str(tmp), str(pkg_dir / "build-gate.json"))


def load_completed(
    gate_results_dir: Path,
    rpms_dir: Path,
    rules_dir: Path | None = None,
) -> set[str]:
    """Load packages that have already passed gates (for --resume).

    Only counts a package as completed if:
    1. Gate-results exist and show success
    2. At least one RPM is still present
    3. No input files (rules, patches, compat) changed since the build
    """
    completed = set()
    if not gate_results_dir.exists():
        return completed

    for pkg_dir in gate_results_dir.iterdir():
        if not pkg_dir.is_dir():
            continue
        gate_file = pkg_dir / "build-gate.json"
        if gate_file.exists():
            try:
                data = json.loads(gate_file.read_text())
                if not (data.get("build_success") and data.get("gate2_passed")):
                    continue
                # Verify at least one RPM still exists
                rpms_present = False
                for rpm_name in data.get("rpms", []):
                    if (rpms_dir / rpm_name).exists():
                        rpms_present = True
                        break
                if not rpms_present:
                    _log(f"[yellow]Stale gate-result (no RPMs): {pkg_dir.name} — will rebuild[/yellow]")
                    continue

                # Check if inputs changed since this build (content hashes)
                if rules_dir:
                    current_hashes = _package_input_hashes(pkg_dir.name, rules_dir)
                    if _inputs_changed(current_hashes, data):
                        _log(f"[yellow]Inputs changed: {pkg_dir.name} — will rebuild[/yellow]")
                        continue

                completed.add(pkg_dir.name)
            except (json.JSONDecodeError, KeyError):
                pass

    return completed


def unstage_packages(
    packages: list[str],
    rpms_dir: Path,
    gate_results_dir: Path,
    staging_dir: Path,
) -> int:
    """Remove files that invalidated packages previously installed into staging.

    For each package, reads the RPM file list and removes those paths from
    the staging sysroot. This prevents ghost .so files from persisting after
    a package's rules change.

    Returns count of packages un-staged.
    """
    unstaged = 0
    staging_root = staging_dir  # RPM paths are absolute, e.g. /opt/mogrix/lib32/libfoo.so

    for pkg in packages:
        # Get the RPM file list from gate-results
        rpm_names: list[str] = []
        gate_file = gate_results_dir / pkg / "build-gate.json"
        if gate_file.exists():
            try:
                data = json.loads(gate_file.read_text())
                rpm_names = data.get("rpms", [])
            except (json.JSONDecodeError, KeyError):
                pass

        if not rpm_names:
            continue

        # Query each RPM for its file list and remove from staging
        files_removed = 0
        for rpm_name in rpm_names:
            rpm_path = rpms_dir / rpm_name
            if not rpm_path.exists():
                continue
            result = subprocess.run(
                ["rpm", "-qpl", str(rpm_path)], capture_output=True, text=True
            )
            for line in result.stdout.splitlines():
                line = line.strip()
                if not line or line.endswith("/"):
                    continue  # skip directories
                # RPM paths are absolute (e.g. /opt/mogrix/lib32/libfoo.so.1)
                # Map to staging: <staging_root>/opt/mogrix/lib32/libfoo.so.1
                staged_file = staging_root / line.lstrip("/")
                if staged_file.exists() and staged_file.is_file():
                    staged_file.unlink()
                    files_removed += 1
                elif staged_file.is_symlink():
                    staged_file.unlink()
                    files_removed += 1

        if files_removed:
            unstaged += 1

    return unstaged


def _find_pkg_rpms(
    pkg: str,
    rpms_dir: Path,
    gate_results_dir: Path,
    rpmbuild_dir: Path,
) -> list[Path]:
    """Find RPM files for a package using gate-results, rpmbuild dir, or name glob."""
    pkg_rpms: list[Path] = []

    # Primary: gate-results JSON (accurate, includes subpackages)
    gate_file = gate_results_dir / pkg / "build-gate.json"
    if gate_file.exists():
        try:
            data = json.loads(gate_file.read_text())
            for rpm_name in data.get("rpms", []):
                rpm_path = rpms_dir / rpm_name
                if rpm_path.exists():
                    pkg_rpms.append(rpm_path)
        except (json.JSONDecodeError, KeyError):
            pass

    if pkg_rpms:
        return pkg_rpms

    # Fallback: rpmbuild dir (handles subpackages like libcurl-devel from curl)
    pkg_rpmbuild = rpmbuild_dir / pkg
    rpmbuild_rpm_names = set()
    for rpmdir in pkg_rpmbuild.glob("RPMS/*"):
        for rpm in rpmdir.glob("*.rpm"):
            rpmbuild_rpm_names.add(rpm.name)
    if rpmbuild_rpm_names:
        return sorted(
            rpms_dir / name for name in rpmbuild_rpm_names
            if (rpms_dir / name).exists()
        )

    # Last resort: name glob
    for ext in ("mips.rpm", "noarch.rpm"):
        pkg_rpms += sorted(rpms_dir.glob(f"{pkg}-[0-9]*.{ext}"))
        pkg_rpms += sorted(rpms_dir.glob(f"{pkg}-*-[0-9]*.{ext}"))
    return sorted(set(pkg_rpms))


def _validate_package_rpms(
    pkg: str,
    rpms_dir: Path,
    gate_results_dir: Path,
    rpmbuild_dir: Path,
) -> tuple[bool, str]:
    """Validate RPMs for a completed package using Gate 3 + rld checks.

    Returns (passed, reason). RPMs are NOT deleted on failure — they'll be
    overwritten when the package is rebuilt.
    """
    import tempfile

    from mogrix.gates import gate3_defect_scan, rld_check_rpms

    pkg_rpms = _find_pkg_rpms(pkg, rpms_dir, gate_results_dir, rpmbuild_dir)
    if not pkg_rpms:
        return True, ""

    # Gate 3: defect scan on this package's RPMs only
    mips_rpms = [r for r in pkg_rpms if r.name.endswith(".mips.rpm")]
    if mips_rpms:
        with tempfile.TemporaryDirectory() as tmpdir:
            tmppath = Path(tmpdir)
            for rpm in mips_rpms:
                (tmppath / rpm.name).symlink_to(rpm)
            report = gate3_defect_scan(tmppath)
            if report.has_critical:
                msgs = [
                    f"{i.elf}: {i.message}"
                    for i in report.issues
                    if i.severity == "CRITICAL"
                ]
                return False, f"defects: {'; '.join(msgs[:3])}"

    # rld check: verify NEEDED sonames resolve against current staging + IRIX sysroot
    rld = rld_check_rpms(pkg_rpms)
    if rld.unresolved:
        sonames = sorted(set(r.soname for r in rld.unresolved))
        return False, f"unresolved: {', '.join(sonames[:3])}"

    return True, ""


def _detect_hub_packages(build_order: list[str], rules_dir: Path, threshold: int = 3) -> set[str]:
    """Auto-detect hub packages: packages with >= threshold dependents.

    Uses build_after edges from rule files to count how many packages
    depend on each package.
    """
    import yaml as _yaml

    dependent_count: dict[str, int] = {}
    for pkg in build_order:
        rule_file = rules_dir / "packages" / f"{pkg}.yaml"
        if not rule_file.exists():
            continue
        try:
            with open(rule_file) as f:
                rules = _yaml.safe_load(f) or {}
            for dep in rules.get("build_after", []):
                dependent_count[dep] = dependent_count.get(dep, 0) + 1
        except Exception:
            pass

    return {pkg for pkg, count in dependent_count.items() if count >= threshold}


def _cascade_gate_check(
    package: str,
    rpms: list[Path],
    staging_dir: Path,
    hub_dependents: int,
) -> tuple[bool, str]:
    """Post-build verification for hub packages.

    Extracts RPMs and verifies at least one .so actually landed in staging.
    Returns (passed, reason).
    """
    import tempfile

    lib32 = staging_dir / "opt" / "mogrix" / "lib32"
    if not lib32.exists():
        # Fallback for old prefix layout
        lib32 = staging_dir / "usr" / "sgug" / "lib32"
    if not lib32.exists():
        return False, f"staging lib32 not found (checked opt/mogrix/lib32 and usr/sgug/lib32)"

    # Extract each RPM, find .so files it ships, verify they exist in staging
    expected_sos = set()
    for rpm_path in rpms:
        if not rpm_path.exists():
            continue
        result = subprocess.run(
            ["rpm", "-qpl", str(rpm_path)], capture_output=True, text=True
        )
        for line in result.stdout.splitlines():
            if "/lib32/" in line and ".so" in line:
                expected_sos.add(Path(line).name)

    if not expected_sos:
        # Package doesn't ship .so files — not a meaningful hub check, pass
        return True, ""

    # Verify at least one expected .so exists in staging
    found = [s for s in expected_sos if (lib32 / s).exists()]
    if not found:
        return False, (
            f"none of {len(expected_sos)} expected .so files found in staging "
            f"(expected: {', '.join(sorted(expected_sos)[:3])}...)"
        )

    return True, ""


def _validate_build_after(build_order: list[str], rules_dir: Path) -> None:
    """Verify every package in the build order has a build_after declaration.

    Packages without build_after get arbitrary positions in the topological
    sort, causing unreliable build ordering. This is a hard failure.
    """
    from mogrix.rules.loader import RuleLoader

    loader = RuleLoader(rules_dir)
    missing = []
    for pkg in build_order:
        pkg_rules = loader.load_package(pkg)
        if not pkg_rules:
            continue  # No rules file at all — handled elsewhere
        if pkg_rules.get("skip"):
            continue
        if "build_after" not in pkg_rules:
            missing.append(pkg)

    if missing:
        _log(f"\n[bold red]Pre-flight FAILED: {len(missing)} packages missing build_after:[/bold red]")
        for pkg in missing:
            _log(f"  [red]•[/red] {pkg}")
        _log("\nEvery package must declare build_after (use 'build_after: none' for root packages).")
        _log("Without it, build ordering is unreliable.\n")
        raise SystemExit(1)


def _fetch_missing_srpms(build_order: list[str], rules_dir: Path) -> None:
    """Scan build order for packages without source SRPMs and auto-fetch them.

    Tries two strategies:
    1. mogrix fetch (Fedora repos) for packages without upstream: definitions
    2. mogrix create-srpm (upstream tarballs/git) for packages with upstream:
    """
    import yaml as _yaml_fetch

    srpms_dir = MOGRIX_INPUTS / "SRPMS"
    missing_fedora = []
    missing_upstream = []
    for pkg in build_order:
        if sorted(srpms_dir.glob(f"{pkg}-[0-9]*.src.rpm")):
            continue
        # Check if package has upstream: definition
        rule_file = rules_dir / "packages" / f"{pkg}.yaml"
        has_upstream = False
        if rule_file.exists():
            try:
                with open(rule_file) as f:
                    rules = _yaml_fetch.safe_load(f) or {}
                has_upstream = bool(rules.get("upstream"))
            except Exception:
                pass
        if has_upstream:
            missing_upstream.append(pkg)
        else:
            missing_fedora.append(pkg)

    if not missing_fedora and not missing_upstream:
        return

    # Fetch from Fedora repos
    if missing_fedora:
        _log(f"[yellow]Missing SRPMs for {len(missing_fedora)} Fedora packages — fetching...[/yellow]")
        result = subprocess.run(
            [str(MOGRIX_BIN), "fetch"] + missing_fedora + ["-y"],
            capture_output=True,
            text=True,
        )
        still_missing = [
            pkg for pkg in missing_fedora
            if not sorted(srpms_dir.glob(f"{pkg}-[0-9]*.src.rpm"))
        ]
        if still_missing:
            _log(f"[red]No SRPM and no upstream: definition for: {', '.join(still_missing)}[/red]")
            _log(f"[red]Add upstream: to their rule files or fetch SRPMs manually[/red]")
        fetched = len(missing_fedora) - len(still_missing)
        if fetched:
            _log(f"[green]Fetched {fetched} SRPMs from Fedora[/green]")

    # Create SRPMs from upstream definitions
    if missing_upstream:
        _log(f"[yellow]Creating SRPMs for {len(missing_upstream)} upstream packages...[/yellow]")
        created = 0
        for pkg in missing_upstream:
            result = subprocess.run(
                [str(MOGRIX_BIN), "create-srpm", pkg],
                capture_output=True,
                text=True,
            )
            if sorted(srpms_dir.glob(f"{pkg}-[0-9]*.src.rpm")):
                created += 1
            else:
                _log(f"[yellow]  create-srpm failed for {pkg}: {result.stderr[-200:]}[/yellow]")
        if created:
            _log(f"[green]Created {created}/{len(missing_upstream)} upstream SRPMs[/green]")


def rebuild_all(
    rules_dir: Path,
    staging_dir: Path = STAGING_DIR,
    workspace: Path | None = None,
    resume: bool = False,
    dry_run: bool = False,
    skip_gates: bool = False,
    from_list: list[str] | None = None,
    fail_fast: bool = True,
) -> RebuildPlan:
    """Execute a full dependency-ordered rebuild.

    Args:
        rules_dir: Path to rules directory
        staging_dir: Staging sysroot path
        workspace: Explicit workspace path (required — no auto-increment)
        resume: Skip completed packages in the workspace
        dry_run: Just compute plan, don't build
        skip_gates: Run gates but treat failures as warnings
        from_list: Only build these packages (in dependency order)
        fail_fast: Stop at first failure (default True)
    """
    # Graceful shutdown on Ctrl-C: finish current package, then stop
    _shutdown_requested = False

    def _handle_sigint(signum, frame):
        nonlocal _shutdown_requested
        if _shutdown_requested:
            _log("[red]Second interrupt — aborting immediately[/red]")
            raise SystemExit(1)
        _shutdown_requested = True
        _log("\n[yellow]Interrupt received — will stop after current package[/yellow]")

    prev_handler = signal.signal(signal.SIGINT, _handle_sigint)

    # Resolve workspace — always explicit, never auto-increment
    if not workspace:
        _log("[red]--workspace is required. Example: mogrix rebuild-all --workspace ~/mogrix_v11[/red]")
        raise SystemExit(1)

    ws = workspace
    if resume:
        if not ws.exists():
            _log(f"[red]Workspace {ws} does not exist — cannot resume[/red]")
            raise SystemExit(1)
        _log(f"[bold]Resuming workspace: {ws}[/bold]")
    else:
        # New build — create workspace dirs
        ws.mkdir(exist_ok=True)
        (ws / "gate-results").mkdir(exist_ok=True)
        (ws / "mogrix_outputs" / "SRPMS").mkdir(parents=True, exist_ok=True)
        (ws / "mogrix_outputs" / "RPMS").mkdir(parents=True, exist_ok=True)
        (ws / "rpmbuild").mkdir(exist_ok=True)
        _log(f"[bold green]Created workspace: {ws}[/bold green]")

    # Validate ~/mogrix_outputs symlink
    _validate_outputs_symlink(ws)

    # Validate staging-owner.json matches this workspace
    _validate_staging_owner(staging_dir, ws, resume)

    # Workspace paths
    outputs_dir = ws / "mogrix_outputs"
    rpms_dir = outputs_dir / "RPMS"
    gate_results_dir = ws / "gate-results"
    rpmbuild_dir = ws / "rpmbuild"

    # Ensure directories exist
    for d in [outputs_dir / "SRPMS", rpms_dir, gate_results_dir, rpmbuild_dir]:
        d.mkdir(parents=True, exist_ok=True)

    # Check for compromised workspace sentinel
    sentinel = ws / "REBUILD_COMPROMISED"
    if resume and sentinel.exists():
        _log(f"[red]Workspace is marked compromised: {sentinel}[/red]")
        _log(f"[red]{sentinel.read_text().strip()}[/red]")
        raise SystemExit(1)

    _log(f"[bold]Workspace:[/bold] {ws}")
    _log("[bold]Computing global build order...[/bold]")
    plan = compute_build_order(rules_dir, rpms_dir)
    plan.gate_results_dir = gate_results_dir

    # Filter to requested packages if --from-list
    if from_list:
        requested = set(from_list)
        plan.build_order = [p for p in plan.build_order if p in requested]
        plan.total = len(plan.build_order)
        _log(f"[dim]Filtered to {plan.total} requested packages[/dim]")

    # Detect hub packages for cascade gating
    hub_packages = _detect_hub_packages(plan.build_order, rules_dir)
    if hub_packages:
        hub_in_order = [p for p in plan.build_order if p in hub_packages]
        _log(f"[dim]Cascade gates on {len(hub_in_order)} hub packages[/dim]")

    _log(f"[bold]Build order:[/bold] {plan.total} packages")
    if plan.cycles:
        _log(f"[yellow]Dependency cycles:[/yellow] {len(plan.cycles)}")

    # Pre-flight: verify every package has build_after declared
    _validate_build_after(plan.build_order, rules_dir)

    if dry_run:
        if resume:
            # Preview what resume would do
            _preview = load_completed(gate_results_dir, rpms_dir, rules_dir)
            _to_build = [p for p in plan.build_order if p not in _preview]
            _log(f"\n[bold]Dry run (resume preview):[/bold]")
            _log(f"  [green]Completed:[/green] {len(_preview)}")
            _log(f"  [yellow]To build:[/yellow] {len(_to_build)}")
            if _to_build:
                _log(f"\n[bold]Packages to rebuild:[/bold]")
                for j, pkg in enumerate(_to_build, 1):
                    _log(f"  {j:3d}. {pkg}")
        else:
            _log("\n[bold]Dry run — build order:[/bold]")
            for j, pkg in enumerate(plan.build_order, 1):
                _log(f"  {j:3d}. {pkg}")
        return plan

    # Pre-flight: fetch any missing SRPMs
    _fetch_missing_srpms(plan.build_order, rules_dir)

    # Gate 0: Reset staging + cross-compilation
    gate0_clean_slate(staging_dir, workspace=ws, resume=resume)

    # Load resume state — single pass with all filtering
    # load_completed checks: build_success, gate2_passed, RPMs on disk, input staleness
    completed = load_completed(gate_results_dir, rpms_dir, rules_dir) if resume else set()
    rpm_count_before = len(list(rpms_dir.glob("*.rpm"))) if resume and rpms_dir.exists() else 0

    if completed:
        _log(f"[dim]Resuming: {len(completed)} packages previously passed[/dim]")

        # Validate existing RPMs (Gate 3 defect scan + rld check per package).
        # Catches defective RPMs from prior builds. Failed packages are excluded
        # from completed (forces rebuild) but RPMs are NOT deleted.
        validation_failures = []
        for pkg in sorted(completed):
            valid, reason = _validate_package_rpms(
                pkg, rpms_dir, gate_results_dir, rpmbuild_dir,
            )
            if not valid:
                _log(f"[yellow]RPM validation failed: {pkg} — {reason} (will rebuild)[/yellow]")
                validation_failures.append(pkg)
        completed -= set(validation_failures)
        if validation_failures:
            _log(f"[dim]{len(validation_failures)} packages failed RPM validation[/dim]")

        # Identify invalidated packages: previously passed gates but now excluded
        # (stale inputs, missing RPMs, failed validation — any reason).
        # These need un-staging to remove ghost .so files from staging.
        all_previously_passed = set()
        if gate_results_dir.exists():
            for pkg_dir in gate_results_dir.iterdir():
                if not pkg_dir.is_dir():
                    continue
                gate_file = pkg_dir / "build-gate.json"
                if gate_file.exists():
                    try:
                        data = json.loads(gate_file.read_text())
                        if data.get("build_success") and data.get("gate2_passed"):
                            all_previously_passed.add(pkg_dir.name)
                    except (json.JSONDecodeError, KeyError):
                        pass
        invalidated = sorted(all_previously_passed - completed)

        if invalidated:
            _log(f"[yellow]{len(invalidated)} packages invalidated ({', '.join(invalidated[:5])}{'...' if len(invalidated) > 5 else ''})[/yellow]")
            # Un-stage invalidated packages: remove their files from staging so
            # ghost .so files don't persist after soname renames or file removals.
            unstaged = unstage_packages(
                invalidated, rpms_dir, gate_results_dir, staging_dir,
            )
            if unstaged:
                _log(f"[dim]Un-staged {unstaged} invalidated packages from staging[/dim]")

        # Re-stage trustworthy packages so their libs are available for rebuilds.
        # On --resume staging wasn't reset, so this is mostly a no-op — but it
        # repairs any files removed during un-staging of invalidated packages.
        restaged = 0
        for pkg in plan.build_order:
            if pkg not in completed:
                continue
            pkg_rpms = _find_pkg_rpms(pkg, rpms_dir, gate_results_dir, rpmbuild_dir)
            if pkg_rpms:
                staged = stage_package(pkg_rpms, staging_dir)
                if staged:
                    restaged += 1
        _log(f"[dim]Re-staged {restaged} trustworthy packages[/dim]")
        _log(f"[dim]Trustworthy: {len(completed)} packages[/dim]")

    # Build reverse dependency map for failure propagation (--no-fail-fast safety)
    import yaml as _yaml_deps
    _dep_graph: dict[str, set[str]] = {}  # pkg -> set of packages it depends on
    for _bo_pkg in plan.build_order:
        _rf = rules_dir / "packages" / f"{_bo_pkg}.yaml"
        if _rf.exists():
            try:
                with open(_rf) as _f_dep:
                    _dep_rules = _yaml_deps.safe_load(_f_dep) or {}
                for _dep in _dep_rules.get("build_after", []):
                    _dep_graph.setdefault(_bo_pkg, set()).add(_dep)
            except Exception:
                pass

    # Build each package in order
    failed = []
    failed_set: set[str] = set()
    runtime_blocked: set[str] = set()
    skipped_blocked = []
    loop_start = time.time()
    packages_built = 0
    for i, package in enumerate(plan.build_order, 1):
        if package in completed:
            _log(f"[dim]{i:3d}/{plan.total} {package} — already passed, skipping[/dim]")
            continue

        # Skip packages whose dependencies failed (--no-fail-fast safety)
        if package in runtime_blocked:
            blocking_deps = _dep_graph.get(package, set()) & (failed_set | runtime_blocked)
            _log(f"[dim]{i:3d}/{plan.total} {package} — skipped: dependency failed ({', '.join(sorted(blocking_deps)[:3])})[/dim]")
            skipped_blocked.append(package)
            continue

        # Check for blocked/skipped packages (blocked: true or skip: true in rule YAML)
        rule_file = rules_dir / "packages" / f"{package}.yaml"
        if rule_file.exists():
            import yaml as _yaml
            with open(rule_file) as _f:
                _pkg_rules = _yaml.safe_load(_f) or {}
            if _pkg_rules.get("blocked"):
                reason = _pkg_rules.get("blocked_reason", "no reason given")
                _log(f"[dim]{i:3d}/{plan.total} {package} — blocked: {reason}[/dim]")
                skipped_blocked.append(package)
                continue
            if _pkg_rules.get("skip"):
                _log(f"[dim]{i:3d}/{plan.total} {package} — skipped (skip: true in rules)[/dim]")
                skipped_blocked.append(package)
                continue

        _log(f"\n[bold]{'='*60}[/bold]")
        _log(f"[bold]{i:3d}/{plan.total} Building: {package}[/bold]")
        _log(f"[bold]{'='*60}[/bold]")

        start = time.time()
        result = RebuildResult(package=package, success=False)
        pkg_hashes = _package_input_hashes(package, rules_dir)

        # Step 1: Convert + postcondition check
        converted = convert_package(package, rules_dir, outputs_dir)
        if not converted:
            result.error = "Convert failed"
            plan.results[package] = result
            failed.append(package)
            failed_set.add(package)
            # Propagate failure to transitive dependents
            for _other in plan.build_order:
                if _dep_graph.get(_other, set()) & (failed_set | runtime_blocked):
                    runtime_blocked.add(_other)
            save_gate_result(gate_results_dir, package, result, input_hashes=pkg_hashes)
            _log(f"  [red]FAILED: {result.error}[/red]")
            if fail_fast:
                _log(f"[red]Stopping at first failure (--fail-fast). Fix and --resume.[/red]")
                break
            continue

        # Postcondition check on converted spec (stores results in manifest)
        # Invariant violations (Linux-isms) are hard failures.
        srpms_dir_input = MOGRIX_INPUTS / "SRPMS"
        original_srpms = sorted(srpms_dir_input.glob(f"{package}-[0-9]*.src.rpm"))
        if original_srpms:
            postconditions_ok = _verify_conversion_applied(
                package, original_srpms[-1], converted, rules_dir,
                rebuild_result=result,
            )
            if not postconditions_ok and not skip_gates:
                result.error = "Invariant violations in converted spec (Linux-isms remain)"
                result.duration_s = time.time() - start
                plan.results[package] = result
                failed.append(package)
                failed_set.add(package)
                for _other in plan.build_order:
                    if _dep_graph.get(_other, set()) & (failed_set | runtime_blocked):
                        runtime_blocked.add(_other)
                save_gate_result(gate_results_dir, package, result, input_hashes=pkg_hashes)
                _log(f"  [red]FAILED: fix rules to remove Linux-isms, then rebuild[/red]")
                if fail_fast:
                    _log(f"[red]Stopping at first failure (--fail-fast). Fix and --resume.[/red]")
                    break
                continue

        # Step 2: Build (each package gets its own isolated rpmbuild dir)
        success, rpms, error = build_package(package, converted, outputs_dir, rpmbuild_dir)
        if not success:
            result.error = f"Build failed: {error}"
            result.duration_s = time.time() - start
            plan.results[package] = result
            failed.append(package)
            save_gate_result(gate_results_dir, package, result, input_hashes=pkg_hashes)
            _log(f"  [red]FAILED: build error[/red]")
            if fail_fast:
                _log(f"[red]Stopping at first failure (--fail-fast). Fix and --resume.[/red]")
                break
            continue

        result.success = True
        result.rpms = [r.name for r in rpms]

        # Step 3: Gate 2 (build output validation)
        gate2 = None
        if rpms:
            gate2 = gate2_check(rpms)
            if not gate2.passed:
                result.gate2_passed = False
                if skip_gates:
                    _log(f"  [yellow]Gate 2 WARNINGS (--skip-gates):[/yellow]")
                    for issue in gate2.issues:
                        _log(f"    [{issue.severity}] {issue.file}: {issue.message}")
                else:
                    _log(f"  [red]Gate 2 FAILED:[/red]")
                    for issue in gate2.issues:
                        _log(f"    [{issue.severity}] {issue.file}: {issue.message}")
                    result.error = f"Gate 2 failed: {len(gate2.issues)} issues"
                    result.duration_s = time.time() - start
                    plan.results[package] = result
                    failed.append(package)
                    failed_set.add(package)
                    for _other in plan.build_order:
                        if _dep_graph.get(_other, set()) & (failed_set | runtime_blocked):
                            runtime_blocked.add(_other)
                    save_gate_result(gate_results_dir, package, result, gate2, input_hashes=pkg_hashes)
                    if fail_fast:
                        _log(f"[red]Stopping at first failure (--fail-fast). Fix and --resume.[/red]")
                        break
                    continue

        # Step 4: Stage
        if rpms:
            staged = stage_package(rpms, staging_dir)
            if not staged:
                _log(f"  [yellow]Warning: staging failed[/yellow]")

        # Step 4.5: Static rld check — verify all NEEDED sonames resolve
        if rpms:
            from mogrix.gates import rld_check_rpms
            rld = rld_check_rpms(rpms)
            if rld.unresolved:
                unresolved_sonames = sorted(set(r.soname for r in rld.unresolved))
                _log(f"  [yellow]rld: {len(unresolved_sonames)} unresolved sonames:[/yellow]")
                for soname in unresolved_sonames[:5]:
                    _log(f"    [yellow]{soname}[/yellow]")
                if len(unresolved_sonames) > 5:
                    _log(f"    [yellow]...and {len(unresolved_sonames) - 5} more[/yellow]")
                result.smoke_test = f"rld_fail:{','.join(unresolved_sonames)}"
            else:
                result.smoke_test = f"rld_pass:{rld.elfs_checked}_elfs"

        result.duration_s = time.time() - start
        plan.results[package] = result
        save_gate_result(gate_results_dir, package, result, gate2, input_hashes=pkg_hashes)
        packages_built += 1
        elapsed = time.time() - loop_start
        remaining_pkgs = sum(
            1 for p in plan.build_order[i:]
            if p not in completed and p not in runtime_blocked
        )
        if packages_built >= 3 and remaining_pkgs > 0:
            avg = elapsed / packages_built
            eta_min = (remaining_pkgs * avg) / 60
            _log(f"  [green]PASSED[/green] ({result.duration_s:.0f}s) [dim]~{remaining_pkgs} left, ~{eta_min:.0f}m remaining[/dim]")
        else:
            _log(f"  [green]PASSED[/green] ({result.duration_s:.0f}s)")

        # Graceful shutdown check
        if _shutdown_requested:
            _log("[yellow]Stopping after completed package (interrupt)[/yellow]")
            break

        # Step 5: Cascade gate for hub packages
        if package in hub_packages and rpms:
            remaining = [p for p in plan.build_order[i:] if p not in completed]
            cascade_ok, cascade_reason = _cascade_gate_check(
                package, rpms, staging_dir, len(remaining),
            )
            if not cascade_ok:
                _log(
                    f"[red]Cascade gate FAILED for hub {package}: {cascade_reason}. "
                    f"Fix before continuing — {len(remaining)} downstream packages depend on this.[/red]"
                )
                if fail_fast:
                    break

    # Restore original signal handler
    signal.signal(signal.SIGINT, prev_handler)

    # Safety invariant: RPM count must never decrease during a resume
    if resume and rpms_dir.exists():
        rpm_count_after = len(list(rpms_dir.glob("*.rpm")))
        if rpm_count_after < rpm_count_before:
            _log(f"[red]INVARIANT VIOLATION: RPM count decreased {rpm_count_before} → {rpm_count_after}[/red]")
            sentinel = ws / "REBUILD_COMPROMISED"
            sentinel.write_text(
                f"RPM count decreased from {rpm_count_before} to {rpm_count_after}\n"
                f"Timestamp: {time.strftime('%Y-%m-%dT%H:%M:%S')}\n"
                f"Delete this file to acknowledge and allow --resume.\n"
            )
            _log(f"[red]Wrote {sentinel} — delete to acknowledge before resuming[/red]")
        else:
            _log(f"\n[dim]RPMs: {rpm_count_before} → {rpm_count_after}[/dim]")

    # Summary
    _log(f"\n[bold]{'='*60}[/bold]")
    _log(f"[bold]Rebuild Summary[/bold]")
    _log(f"[bold]{'='*60}[/bold]")

    passed = [p for p, r in plan.results.items() if r.success and r.gate2_passed]
    _log(f"  [green]Passed:[/green] {len(passed)}")
    _log(f"  [red]Failed:[/red] {len(failed)}")
    if completed:
        _log(f"  [dim]Skipped (resume):[/dim] {len(completed)}")
    if skipped_blocked:
        _log(f"  [yellow]Blocked:[/yellow] {len(skipped_blocked)} ({', '.join(skipped_blocked[:5])}{'...' if len(skipped_blocked) > 5 else ''})")
    if runtime_blocked:
        _log(f"  [yellow]Dep-blocked:[/yellow] {len(runtime_blocked)} (dependency failed)")

    if failed:
        _log(f"\n[red]Failed packages:[/red]")
        for pkg in failed:
            r = plan.results.get(pkg)
            err = r.error if r else "unknown"
            _log(f"  {pkg}: {err}")

    # Gate 3: Defect scan on all built RPMs (always runs — catches stale RPMs from prior builds)
    if rpms_dir.exists() and any(rpms_dir.glob("*.mips.rpm")):
        from mogrix.gates import gate3_defect_scan
        _log(f"\n[bold]=== Gate 3: Defect Scan ===[/bold]")
        report = gate3_defect_scan(rpms_dir)
        if report.has_critical:
            _log(f"[red]CRITICAL defects found ({report.critical_count}):[/red]")
            for issue in report.issues:
                if issue.severity == "CRITICAL":
                    _log(f"  {issue.rpm}: {issue.elf}: {issue.message}")
            _log(f"[red]Fix these before bundling.[/red]")
        elif report.error_count:
            _log(f"[yellow]Errors: {report.error_count} (non-critical)[/yellow]")
        else:
            _log(f"[green]No defects in {report.scanned} ELF files across {report.rpms_scanned} RPMs[/green]")

    # Write workspace manifest — comprehensive record of this rebuild
    _write_workspace_manifest(ws, plan, completed, skipped_blocked, hub_packages)

    return plan


def _write_workspace_manifest(
    workspace: Path,
    plan: RebuildPlan,
    completed: set[str],
    blocked: list[str],
    hubs: set[str],
) -> None:
    """Write a workspace-level manifest with all findings per package."""
    manifest = {
        "workspace": str(workspace),
        "timestamp": time.strftime("%Y-%m-%dT%H:%M:%S"),
        "total_packages": plan.total,
        "hub_packages": sorted(hubs),
        "blocked_packages": blocked,
        "resumed_packages": sorted(completed),
        "packages": {},
    }

    # Compile per-package data from gate-results + plan results
    gate_results_dir = workspace / "gate-results"
    for pkg in plan.build_order:
        pkg_data: dict = {"status": "unknown"}

        if pkg in completed:
            pkg_data["status"] = "resumed"
        elif pkg in blocked:
            pkg_data["status"] = "blocked"

        # Load gate-result if it exists
        gate_file = gate_results_dir / pkg / "build-gate.json"
        if gate_file.exists():
            try:
                gate_data = json.loads(gate_file.read_text())
                pkg_data.update({
                    "status": "passed" if gate_data.get("build_success") and gate_data.get("gate2_passed") else "failed",
                    "rpms": gate_data.get("rpms", []),
                    "duration_s": gate_data.get("duration_s", 0),
                    "error": gate_data.get("error", ""),
                    "timestamp": gate_data.get("timestamp", ""),
                })
                if "postconditions" in gate_data:
                    pkg_data["postconditions"] = gate_data["postconditions"]
                if "smoke_test" in gate_data:
                    pkg_data["smoke_test"] = gate_data["smoke_test"]
                if "gate2_issues" in gate_data:
                    pkg_data["gate2_issues"] = gate_data["gate2_issues"]
            except (json.JSONDecodeError, KeyError):
                pass

        if pkg in hubs:
            pkg_data["is_hub"] = True

        manifest["packages"][pkg] = pkg_data

    # Summary counts
    statuses = [d.get("status") for d in manifest["packages"].values()]
    manifest["summary"] = {
        "passed": statuses.count("passed"),
        "failed": statuses.count("failed"),
        "resumed": statuses.count("resumed"),
        "blocked": statuses.count("blocked"),
        "postcondition_errors": sum(
            len(d.get("postconditions", {}).get("errors", []))
            for d in manifest["packages"].values()
        ),
        "rld_failures": sum(
            1 for d in manifest["packages"].values()
            if str(d.get("smoke_test", "")).startswith("rld_fail")
        ),
    }

    manifest_path = workspace / "build-manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2))
    _log(f"\n[dim]Manifest written: {manifest_path}[/dim]")


def rebuild_one(
    package: str,
    rules_dir: Path,
    workspace: Path,
    staging_dir: Path = STAGING_DIR,
    skip_gates: bool = False,
) -> RebuildResult:
    """Rebuild a single package within an existing workspace.

    Detects whether the package has a prior build attempt:
    - If BUILD dir + spec exist: uses --short-circuit to skip %prep (fast resume)
    - Otherwise: full convert + build from scratch

    Updates gate-results so rebuild-all --resume sees it correctly.
    Stages on success so downstream packages can build against it.
    """
    outputs_dir = workspace / "mogrix_outputs"
    rpms_dir = outputs_dir / "RPMS"
    gate_results_dir = workspace / "gate-results"
    rpmbuild_dir = workspace / "rpmbuild"

    for d in [outputs_dir / "SRPMS", rpms_dir, gate_results_dir, rpmbuild_dir]:
        d.mkdir(parents=True, exist_ok=True)

    start = time.time()
    result = RebuildResult(package=package, success=False)
    pkg_hashes = _package_input_hashes(package, rules_dir)

    # Detect prior build attempt
    pkg_rpmbuild = rpmbuild_dir / package
    specs = sorted((pkg_rpmbuild / "SPECS").glob("*.spec")) if (pkg_rpmbuild / "SPECS").exists() else []
    build_dir_exists = (pkg_rpmbuild / "BUILD").exists() and any((pkg_rpmbuild / "BUILD").iterdir())
    can_short_circuit = bool(specs and build_dir_exists)

    if can_short_circuit:
        _log(f"[bold cyan]Found prior build for {package} — using --short-circuit[/bold cyan]")
        _log(f"[dim]  Spec: {specs[0].name}[/dim]")
        obj_count = sum(1 for _ in (pkg_rpmbuild / "BUILD").rglob("*.o"))
        if obj_count:
            _log(f"[dim]  Cached objects: {obj_count} .o files[/dim]")
        short_circuit = "binary"  # -bb --short-circuit: build + install + package
    else:
        _log(f"[bold]Full build for {package} (no prior build to resume)[/bold]")
        short_circuit = None

    # Step 1: Convert (always needed — provides the SRPM path even for short-circuit)
    converted = convert_package(package, rules_dir, outputs_dir)
    if not converted:
        result.error = "Convert failed"
        result.duration_s = time.time() - start
        save_gate_result(gate_results_dir, package, result, input_hashes=pkg_hashes)
        _log(f"[red]FAILED: {result.error}[/red]")
        return result

    # Step 2: Build
    _log(f"\n{'='*60}")
    _log(f"[bold]Building: {package}" + (" (short-circuit)" if short_circuit else "") + "[/bold]")
    _log(f"{'='*60}")

    success, rpms, error = build_package(
        package, converted, outputs_dir, rpmbuild_dir,
        short_circuit=short_circuit,
    )
    if not success:
        result.error = f"Build failed: {error}"
        result.duration_s = time.time() - start
        save_gate_result(gate_results_dir, package, result, input_hashes=pkg_hashes)
        _log(f"[red]FAILED: build error[/red]")
        _log(f"[dim]{error}[/dim]")
        return result

    result.success = True
    result.rpms = [r.name for r in rpms]

    # Step 3: Gate 2
    gate2 = None
    if rpms:
        gate2 = gate2_check(rpms)
        if not gate2.passed:
            result.gate2_passed = False
            if skip_gates:
                _log(f"[yellow]Gate 2 WARNINGS (--skip-gates):[/yellow]")
                for issue in gate2.issues:
                    _log(f"  [{issue.severity}] {issue.file}: {issue.message}")
            else:
                _log(f"[red]Gate 2 FAILED:[/red]")
                for issue in gate2.issues:
                    _log(f"  [{issue.severity}] {issue.file}: {issue.message}")
                result.error = f"Gate 2 failed: {len(gate2.issues)} issues"
                result.duration_s = time.time() - start
                save_gate_result(gate_results_dir, package, result, gate2, input_hashes=pkg_hashes)
                return result

    # Step 4: Stage
    if rpms:
        staged = stage_package(rpms, staging_dir)
        if not staged:
            _log(f"[yellow]Warning: staging failed[/yellow]")

    result.duration_s = time.time() - start
    save_gate_result(gate_results_dir, package, result, gate2, input_hashes=pkg_hashes)
    _log(f"[green]PASSED[/green] ({result.duration_s:.0f}s)")

    if short_circuit:
        _log(f"[bold green]Short-circuit saved time — reused cached build artifacts[/bold green]")

    return result
