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
import re
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
STAGING_DIR = Path("/opt/sgug-staging")
WORKSPACE_PREFIX = "mogrix_v"

# Use venv mogrix binary directly to avoid uv lock contention
# when rebuild-all is itself invoked via `uv run mogrix`.
MOGRIX_BIN = Path(__file__).parent.parent / ".venv" / "bin" / "mogrix"


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


def create_workspace() -> Path:
    """Create the next numbered workspace (auto-increment)."""
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


def gate0_clean_slate(
    staging_dir: Path,
    resume: bool = False,
) -> None:
    """Gate 0: Establish clean build environment.

    Resets staging and re-deploys cross-compilation tools.
    Workspace directories are never wiped — they're versioned.
    """
    _log("\n[bold]=== Gate 0: Clean Slate ===[/bold]\n")

    if resume:
        _log("[dim]--resume: reusing existing workspace[/dim]")

    reset_staging(staging_dir)
    setup_cross()

    _log("\n[bold green]Gate 0 passed: clean slate established[/bold green]\n")


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
    return dst


def build_package(
    package: str,
    converted_srpm: Path,
    outputs_dir: Path,
    rpmbuild_dir: Path,
) -> tuple[bool, list[Path], str]:
    """Build a converted SRPM. Returns (success, rpm_paths, error_msg).

    Each package gets its own rpmbuild directory under workspace/rpmbuild/<pkg>.
    The directory is cleaned BEFORE building (not after) so build artifacts
    are preserved for post-mortem inspection on failure.
    """
    # Per-package rpmbuild directory — fully isolated from other builds
    pkg_rpmbuild = rpmbuild_dir / package

    # Clean BEFORE build (preserves artifacts from previous builds for inspection)
    if pkg_rpmbuild.exists():
        subprocess.run(["rm", "-rf", str(pkg_rpmbuild)], check=False)
    pkg_rpmbuild.mkdir(parents=True, exist_ok=True)

    # Per-package log directory
    log_dir = pkg_rpmbuild / "logs"
    log_dir.mkdir(parents=True, exist_ok=True)
    log_file = log_dir / f"{package}-build.log"

    # Snapshot output RPMs before build to detect new ones
    out_rpms_dir = outputs_dir / "RPMS"
    out_rpms_dir.mkdir(parents=True, exist_ok=True)
    pre_build_rpms = set(out_rpms_dir.glob("*.mips.rpm")) | set(out_rpms_dir.glob("*.noarch.rpm"))

    result = subprocess.run(
        [str(MOGRIX_BIN), "build", str(converted_srpm), "--cross",
         "--rpmbuild-dir", str(pkg_rpmbuild),
         "--output-dir", str(out_rpms_dir)],
        capture_output=True,
        text=True,
        errors='replace',
    )

    # Save full output to log
    log_file.write_text(result.stdout + "\n" + result.stderr)

    if result.returncode != 0:
        # Extract last 5 lines of error
        lines = (result.stdout + result.stderr).strip().split("\n")
        error = "\n".join(lines[-5:])
        return False, [], error

    # The CLI copies built RPMs to outputs/RPMS/ — collect from there.
    post_build_rpms = set(out_rpms_dir.glob("*.mips.rpm")) | set(out_rpms_dir.glob("*.noarch.rpm"))
    new_rpms = sorted(post_build_rpms - pre_build_rpms)

    # Fallback to name-based glob if snapshot detection finds nothing
    if not new_rpms:
        m = re.match(r"^(.+?)-[\d]", converted_srpm.name)
        pkg_prefix = m.group(1) if m else package
        new_rpms = sorted(out_rpms_dir.glob(f"{pkg_prefix}*.mips.rpm"))
        new_rpms += sorted(out_rpms_dir.glob(f"{pkg_prefix}*.noarch.rpm"))

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

    if gate2_result:
        data["gate2_issues"] = [
            {"severity": i.severity, "file": i.file, "message": i.message}
            for i in gate2_result.issues
        ]

    (pkg_dir / "build-gate.json").write_text(json.dumps(data, indent=2))


def load_completed(gate_results_dir: Path, rpms_dir: Path) -> set[str]:
    """Load packages that have already passed gates (for --resume).

    Only counts a package as completed if its gate-results exist AND
    at least one of its RPMs is present in the outputs directory.
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
                if rpms_present:
                    completed.add(pkg_dir.name)
                else:
                    _log(f"[yellow]Stale gate-result (no RPMs): {pkg_dir.name} — will rebuild[/yellow]")
            except (json.JSONDecodeError, KeyError):
                pass

    return completed


def clean_tainted_artifacts(
    gate_results_dir: Path,
    outputs_dir: Path,
    build_order: list[str],
    completed: set[str],
) -> tuple[set[str], int]:
    """Remove artifacts for failed packages AND everything built after them.

    If package B at position 50 failed, packages 51+ were built in a
    potentially contaminated environment (missing B's libs in staging).
    Even if they "passed," their builds are suspect. We must clean
    everything from the first failure onward and rebuild it.

    Returns (trustworthy_set, cleaned_count) where trustworthy_set is the
    subset of completed that is safe to keep (built before any failure).
    """
    if not gate_results_dir.exists():
        return completed, 0

    # Walk build order to find the first failed package
    first_failure_idx = None
    for i, pkg in enumerate(build_order):
        if pkg not in completed:
            # This package either failed or was never attempted.
            # Check if it has a gate result (failed) vs missing SRPM (never ran).
            gate_file = gate_results_dir / pkg / "build-gate.json"
            if gate_file.exists():
                first_failure_idx = i
                break

    if first_failure_idx is None:
        # No failures found in build order — nothing to clean
        return completed, 0

    # Everything from first_failure_idx onward is tainted
    tainted = set(build_order[first_failure_idx:])
    trustworthy = completed - tainted

    # Clean artifacts for all tainted packages
    rpms_dir = outputs_dir / "RPMS"
    cleaned = 0

    for pkg_name in tainted:
        # Remove gate results
        pkg_gate_dir = gate_results_dir / pkg_name
        if pkg_gate_dir.exists():
            shutil.rmtree(pkg_gate_dir, ignore_errors=True)

        # Remove RPMs
        if rpms_dir.exists():
            for rpm in rpms_dir.glob(f"{pkg_name}-[0-9]*.mips.rpm"):
                rpm.unlink(missing_ok=True)
            for rpm in rpms_dir.glob(f"{pkg_name}-*-[0-9]*.mips.rpm"):
                rpm.unlink(missing_ok=True)

        cleaned += 1

    return trustworthy, cleaned


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
        workspace: Explicit workspace path (auto-detected if None)
        resume: Reuse latest workspace and skip completed packages
        dry_run: Just compute plan, don't build
        skip_gates: Run gates but treat failures as warnings
        from_list: Only build these packages (in dependency order)
        fail_fast: Stop at first failure (default True)
    """
    # Resolve workspace
    if workspace:
        ws = workspace
    elif resume:
        ws = find_latest_workspace()
        if not ws:
            _log("[red]No existing workspace found for --resume[/red]")
            raise SystemExit(1)
        _log(f"[bold]Resuming workspace: {ws}[/bold]")
    else:
        ws = create_workspace()

    # Workspace paths
    outputs_dir = ws / "mogrix_outputs"
    rpms_dir = outputs_dir / "RPMS"
    gate_results_dir = ws / "gate-results"
    rpmbuild_dir = ws / "rpmbuild"

    # Ensure directories exist
    for d in [outputs_dir / "SRPMS", rpms_dir, gate_results_dir, rpmbuild_dir]:
        d.mkdir(parents=True, exist_ok=True)

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

    _log(f"[bold]Build order:[/bold] {plan.total} packages")
    if plan.cycles:
        _log(f"[yellow]Dependency cycles:[/yellow] {len(plan.cycles)}")

    if dry_run:
        _log("\n[bold]Dry run — build order:[/bold]")
        for i, pkg in enumerate(plan.build_order, 1):
            _log(f"  {i:3d}. {pkg}")
        return plan

    # Gate 0: Reset staging + cross-compilation
    gate0_clean_slate(staging_dir, resume=resume)

    # Load resume state
    completed = load_completed(gate_results_dir, rpms_dir) if resume else set()
    if completed:
        _log(f"[dim]Resuming: {len(completed)} packages previously passed[/dim]")

        # Clean tainted artifacts: failed packages AND everything built after
        # the first failure. Packages after a failure were built in a potentially
        # contaminated environment (missing the failed package's libs).
        completed, cleaned = clean_tainted_artifacts(
            gate_results_dir, outputs_dir, plan.build_order, completed
        )
        if cleaned:
            _log(f"[dim]Cleaned {cleaned} tainted packages (from first failure onward)[/dim]")
            _log(f"[dim]Trustworthy: {len(completed)} packages[/dim]")

        # Re-stage trustworthy packages so their libs are available for rebuilds.
        # Gate0 reset staging to pristine — we need to restore what passed cleanly.
        restaged = 0
        for pkg in plan.build_order:
            if pkg not in completed:
                continue
            gate_file = gate_results_dir / pkg / "build-gate.json"
            pkg_rpms = []
            if gate_file.exists():
                try:
                    data = json.loads(gate_file.read_text())
                    for rpm_name in data.get("rpms", []):
                        rpm_path = rpms_dir / rpm_name
                        if rpm_path.exists():
                            pkg_rpms.append(rpm_path)
                except (json.JSONDecodeError, KeyError):
                    pass
            # Fallback to name-based glob if gate results unavailable
            if not pkg_rpms:
                for ext in ("mips.rpm", "noarch.rpm"):
                    pkg_rpms += sorted(rpms_dir.glob(f"{pkg}-[0-9]*.{ext}"))
                    pkg_rpms += sorted(rpms_dir.glob(f"{pkg}-*-[0-9]*.{ext}"))
                pkg_rpms = sorted(set(pkg_rpms))
            if pkg_rpms:
                staged = stage_package(pkg_rpms, staging_dir)
                if staged:
                    restaged += 1
        _log(f"[dim]Re-staged {restaged} trustworthy packages[/dim]")

    # Build each package in order
    failed = []
    skipped_blocked = []
    for i, package in enumerate(plan.build_order, 1):
        if package in completed:
            _log(f"[dim]{i:3d}/{plan.total} {package} — already passed, skipping[/dim]")
            continue

        # Check for blocked packages (blocked: true in rule YAML)
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

        _log(f"\n[bold]{'='*60}[/bold]")
        _log(f"[bold]{i:3d}/{plan.total} Building: {package}[/bold]")
        _log(f"[bold]{'='*60}[/bold]")

        start = time.time()
        result = RebuildResult(package=package, success=False)

        # Step 1: Convert
        converted = convert_package(package, rules_dir, outputs_dir)
        if not converted:
            result.error = "Convert failed"
            plan.results[package] = result
            failed.append(package)
            save_gate_result(gate_results_dir, package, result)
            _log(f"  [red]FAILED: {result.error}[/red]")
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
            save_gate_result(gate_results_dir, package, result)
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
                    save_gate_result(gate_results_dir, package, result, gate2)
                    if fail_fast:
                        _log(f"[red]Stopping at first failure (--fail-fast). Fix and --resume.[/red]")
                        break
                    continue

        # Step 4: Stage
        if rpms:
            staged = stage_package(rpms, staging_dir)
            if not staged:
                _log(f"  [yellow]Warning: staging failed[/yellow]")

        result.duration_s = time.time() - start
        plan.results[package] = result
        save_gate_result(gate_results_dir, package, result, gate2)
        _log(f"  [green]PASSED[/green] ({result.duration_s:.0f}s)")

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
        _log(f"  [yellow]Blocked:[/yellow] {len(skipped_blocked)} ({', '.join(skipped_blocked)})")

    if failed:
        _log(f"\n[red]Failed packages:[/red]")
        for pkg in failed:
            r = plan.results.get(pkg)
            err = r.error if r else "unknown"
            _log(f"  {pkg}: {err}")

    return plan
