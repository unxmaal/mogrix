"""Rebuild-all orchestration — clean slate + dependency-ordered rebuilds.

Gate 0: Ensures clean build environment before starting.
Gate 2: Validates build outputs (shebangs, ELF ABI, paths) after each build.
"""

import json
import shutil
import subprocess
import time
from dataclasses import dataclass, field
from pathlib import Path

import sys

from rich.console import Console

from mogrix.gates import GateResult, pre_scan_rpms

# force_terminal=False disables Rich buffering when stdout is redirected to a file
console = Console(force_terminal=False)


def _log(msg: str) -> None:
    """Print and flush immediately so redirected output streams in real-time."""
    console.print(msg)
    sys.stdout.flush()

# Directories
MOGRIX_OUTPUTS = Path.home() / "mogrix_outputs"
MOGRIX_INPUTS = Path.home() / "mogrix_inputs"
STAGING_DIR = Path("/opt/sgug-staging")
RPMBUILD_DIR = Path.home() / "rpmbuild"

# Use venv mogrix binary directly to avoid uv lock contention
# when rebuild-all is itself invoked via `uv run mogrix`.
MOGRIX_BIN = Path(__file__).parent.parent / ".venv" / "bin" / "mogrix"


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


def compute_build_order(rules_dir: Path) -> RebuildPlan:
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
        rpms_dir=MOGRIX_OUTPUTS / "RPMS",
        stop_at_rules=True,
    )
    build_order, cycles = resolver.resolve_all()
    db.close()
    return RebuildPlan(build_order=build_order, cycles=cycles)


def clean_outputs(outputs_dir: Path, timestamp: str) -> None:
    """Relocate old outputs to timestamped backup."""
    if not outputs_dir.exists():
        return

    old_dir = outputs_dir.parent / f"{outputs_dir.name}_old.{timestamp}"
    _log(f"[bold]Relocating outputs:[/bold] {outputs_dir} -> {old_dir}")
    shutil.move(str(outputs_dir), str(old_dir))

    # Create fresh directories
    for subdir in ["SRPMS", "RPMS", "bundles"]:
        (outputs_dir / subdir).mkdir(parents=True, exist_ok=True)
    _log("[green]Fresh output directories created[/green]")


def clean_rpmbuild(rpmbuild_dir: Path) -> None:
    """Remove rpmbuild workspace."""
    for subdir in ["BUILD", "BUILDROOT", "RPMS", "SRPMS"]:
        d = rpmbuild_dir / subdir
        if d.exists():
            # Use rm -rf instead of shutil.rmtree — more robust against
            # permission issues and race conditions from parallel builds
            subprocess.run(["rm", "-rf", str(d)], check=False)
            d.mkdir(parents=True, exist_ok=True)
    _log("[green]rpmbuild workspace cleaned[/green]")


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
    outputs_dir: Path,
    staging_dir: Path,
    rpmbuild_dir: Path,
    keep_old: bool = False,
    resume: bool = False,
) -> None:
    """Gate 0: Establish clean build environment.

    1. Relocate old outputs (unless --keep-old or --resume)
    2. Reset staging to pristine base
    3. Re-deploy cross-compilation tools
    4. Clean rpmbuild workspace
    """
    timestamp = time.strftime("%m%d%H%M")

    _log("\n[bold]=== Gate 0: Clean Slate ===[/bold]\n")

    if resume:
        # --resume: keep SRPMs and RPMs (needed for re-staging passed packages).
        # Only clean logs and per-package rpmbuild dirs.
        for subdir in ["logs"]:
            d = outputs_dir / subdir
            if d.exists():
                subprocess.run(["rm", "-rf", str(d)], check=False)
            d.mkdir(parents=True, exist_ok=True)
        for subdir in ["SRPMS", "RPMS", "bundles"]:
            (outputs_dir / subdir).mkdir(parents=True, exist_ok=True)
        _log("[dim]--resume: preserving SRPMs and RPMs for re-staging[/dim]")
    elif not keep_old:
        clean_outputs(outputs_dir, timestamp)
    else:
        # --keep-old preserves converted SRPMs (avoids slow re-convert) but
        # ALWAYS cleans built RPMs, bundles, and logs. Stale RPMs from prior
        # runs would mask dep-check failures and pollute bundles.
        for subdir in ["RPMS", "bundles", "logs"]:
            d = outputs_dir / subdir
            if d.exists():
                subprocess.run(["rm", "-rf", str(d)], check=False)
            d.mkdir(parents=True, exist_ok=True)
        (outputs_dir / "SRPMS").mkdir(parents=True, exist_ok=True)
        _log("[dim]--keep-old: preserving SRPMs, cleaning RPMs/bundles/logs[/dim]")

    reset_staging(staging_dir)
    setup_cross()
    clean_rpmbuild(rpmbuild_dir)

    # Clean per-package rpmbuild dirs from previous runs
    pkg_rpmbuild_parent = outputs_dir / "rpmbuild"
    if pkg_rpmbuild_parent.exists():
        subprocess.run(["rm", "-rf", str(pkg_rpmbuild_parent)], check=False)
        _log("[green]per-package rpmbuild dirs cleaned[/green]")

    _log("\n[bold green]Gate 0 passed: clean slate established[/bold green]\n")


def convert_package(package: str, rules_dir: Path) -> Path | None:
    """Convert a package SRPM. Returns path to converted SRPM or None."""
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

    # Find the converted SRPM
    converted_dir = MOGRIX_OUTPUTS / "SRPMS"
    converted = sorted(converted_dir.glob(f"{package}-[0-9]*.src.rpm"))
    if not converted:
        _log(f"  [red]No converted SRPM found after convert[/red]")
        return None

    return converted[-1]


def build_package(package: str, converted_srpm: Path) -> tuple[bool, list[Path], str]:
    """Build a converted SRPM. Returns (success, rpm_paths, error_msg).

    Each package gets its own rpmbuild directory under mogrix_outputs/rpmbuild/<pkg>
    to prevent cross-contamination between builds (stale RPMs, leftover sources, etc.).
    """
    logs_dir = MOGRIX_OUTPUTS / "logs"
    logs_dir.mkdir(parents=True, exist_ok=True)
    log_file = logs_dir / f"{package}-build.log"

    # Per-package rpmbuild directory — fully isolated from other builds
    pkg_rpmbuild = MOGRIX_OUTPUTS / "rpmbuild" / package
    pkg_rpmbuild.mkdir(parents=True, exist_ok=True)

    # Snapshot output RPMs before build to detect new ones
    out_rpms_dir = MOGRIX_OUTPUTS / "RPMS"
    pre_build_rpms = set(out_rpms_dir.glob("*.mips.rpm")) if out_rpms_dir.exists() else set()

    result = subprocess.run(
        [str(MOGRIX_BIN), "build", str(converted_srpm), "--cross",
         "--rpmbuild-dir", str(pkg_rpmbuild)],
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
        shutil.rmtree(pkg_rpmbuild, ignore_errors=True)
        return False, [], error

    # The CLI copies built RPMs to MOGRIX_OUTPUTS/RPMS/ — collect from there.
    # Snapshot comparison finds exactly the RPMs this build produced.
    post_build_rpms = set(out_rpms_dir.glob("*.mips.rpm")) if out_rpms_dir.exists() else set()
    new_rpms = sorted(post_build_rpms - pre_build_rpms)

    # Fallback to name-based glob if snapshot detection finds nothing
    if not new_rpms:
        import re as _re
        m = _re.match(r"^(.+?)-[\d]", converted_srpm.name)
        pkg_prefix = m.group(1) if m else package
        new_rpms = sorted(out_rpms_dir.glob(f"{pkg_prefix}*.mips.rpm"))

    # Clean up the per-package rpmbuild dir
    shutil.rmtree(pkg_rpmbuild, ignore_errors=True)

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


def load_completed(gate_results_dir: Path) -> set[str]:
    """Load packages that have already passed gates (for --resume)."""
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
                if data.get("build_success") and data.get("gate2_passed"):
                    completed.add(pkg_dir.name)
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
    bundles_dir = outputs_dir / "bundles"
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

        # Remove bundles
        if bundles_dir.exists():
            for bundle in bundles_dir.glob(f"{pkg_name}-*.tar.gz"):
                bundle.unlink(missing_ok=True)
            bundle_dir = bundles_dir / pkg_name
            if bundle_dir.exists():
                shutil.rmtree(bundle_dir, ignore_errors=True)

        cleaned += 1

    return trustworthy, cleaned


def rebuild_all(
    rules_dir: Path,
    staging_dir: Path = STAGING_DIR,
    outputs_dir: Path = MOGRIX_OUTPUTS,
    rpmbuild_dir: Path = RPMBUILD_DIR,
    keep_old: bool = False,
    resume: bool = False,
    dry_run: bool = False,
    skip_gates: bool = False,
    from_list: list[str] | None = None,
) -> RebuildPlan:
    """Execute a full dependency-ordered rebuild.

    Args:
        rules_dir: Path to rules directory
        staging_dir: Staging sysroot path
        outputs_dir: Output directory for RPMs/bundles
        rpmbuild_dir: rpmbuild workspace
        keep_old: Skip output relocation
        resume: Skip packages that already passed gates
        dry_run: Just compute plan, don't build
        skip_gates: Run gates but treat failures as warnings
        from_list: Only build these packages (in dependency order)
    """
    _log("[bold]Computing global build order...[/bold]")
    plan = compute_build_order(rules_dir)
    plan.gate_results_dir = Path("gate-results")

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

    # Gate 0: Clean slate
    gate0_clean_slate(outputs_dir, staging_dir, rpmbuild_dir, keep_old=keep_old, resume=resume)

    # Load resume state
    completed = load_completed(plan.gate_results_dir) if resume else set()
    if completed:
        _log(f"[dim]Resuming: {len(completed)} packages previously passed[/dim]")

        # Clean tainted artifacts: failed packages AND everything built after
        # the first failure. Packages after a failure were built in a potentially
        # contaminated environment (missing the failed package's libs).
        completed, cleaned = clean_tainted_artifacts(
            plan.gate_results_dir, outputs_dir, plan.build_order, completed
        )
        if cleaned:
            _log(f"[dim]Cleaned {cleaned} tainted packages (from first failure onward)[/dim]")
            _log(f"[dim]Trustworthy: {len(completed)} packages[/dim]")

        # Re-stage trustworthy packages so their libs are available for rebuilds.
        # Gate0 reset staging to pristine — we need to restore what passed cleanly.
        rpms_dir = outputs_dir / "RPMS"
        restaged = 0
        for pkg in plan.build_order:
            if pkg not in completed:
                continue
            # Find this package's RPMs in outputs
            pkg_rpms = sorted(rpms_dir.glob(f"{pkg}-[0-9]*.mips.rpm"))
            pkg_rpms += sorted(rpms_dir.glob(f"{pkg}-*-[0-9]*.mips.rpm"))
            # Deduplicate
            pkg_rpms = sorted(set(pkg_rpms))
            if pkg_rpms:
                staged = stage_package(pkg_rpms, staging_dir)
                if staged:
                    restaged += 1
        _log(f"[dim]Re-staged {restaged} trustworthy packages[/dim]")

    # Build each package in order
    failed = []
    for i, package in enumerate(plan.build_order, 1):
        if package in completed:
            _log(f"[dim]{i:3d}/{plan.total} {package} — already passed, skipping[/dim]")
            continue

        _log(f"\n[bold]{'='*60}[/bold]")
        _log(f"[bold]{i:3d}/{plan.total} Building: {package}[/bold]")
        _log(f"[bold]{'='*60}[/bold]")

        start = time.time()
        result = RebuildResult(package=package, success=False)

        # Step 1: Convert
        converted = convert_package(package, rules_dir)
        if not converted:
            result.error = "Convert failed"
            plan.results[package] = result
            failed.append(package)
            save_gate_result(plan.gate_results_dir, package, result)
            _log(f"  [red]FAILED: {result.error}[/red]")
            continue

        # Step 2: Build (each package gets its own isolated rpmbuild dir)
        success, rpms, error = build_package(package, converted)
        if not success:
            result.error = f"Build failed: {error}"
            result.duration_s = time.time() - start
            plan.results[package] = result
            failed.append(package)
            save_gate_result(plan.gate_results_dir, package, result)
            _log(f"  [red]FAILED: build error[/red]")
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
                    save_gate_result(plan.gate_results_dir, package, result, gate2)
                    continue

        # Step 4: Stage
        if rpms:
            staged = stage_package(rpms, staging_dir)
            if not staged:
                _log(f"  [yellow]Warning: staging failed[/yellow]")

        result.duration_s = time.time() - start
        plan.results[package] = result
        save_gate_result(plan.gate_results_dir, package, result, gate2)
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

    if failed:
        _log(f"\n[red]Failed packages:[/red]")
        for pkg in failed:
            r = plan.results.get(pkg)
            err = r.error if r else "unknown"
            _log(f"  {pkg}: {err}")

    return plan
