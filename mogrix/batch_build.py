"""Batch build orchestrator for mogrix.

Automates the fetch → convert → build pipeline for multiple packages.
Supports list-driven and roadmap-driven modes. Best-effort: always moves
on to the next package, never blocks on failures.
"""

import json
import shutil
import subprocess
import time
import re
from dataclasses import dataclass, field
from enum import Enum
from pathlib import Path

from rich.console import Console
from rich.table import Table

from mogrix.batch import BatchConverter
from mogrix.deps.resolver import DependencyResolver
from mogrix.rule_generator import RuleGenerator
from mogrix.rules.loader import RuleLoader


console = Console()


# ─── Data Structures ───────────────────────────────────────────────────────


class BuildStatus(Enum):
    """Status of a batch build attempt."""

    SUCCESS = "success"
    SKIPPED = "skipped"            # Already built
    NEEDS_REVIEW = "needs_review"  # Candidate rules generated, no build attempted
    FETCH_FAILED = "fetch_failed"
    CONVERT_FAILED = "convert_failed"
    BUILD_FAILED = "build_failed"
    TIMEOUT = "timeout"


class FailureCategory(Enum):
    """Category of a build failure (for triage)."""

    MISSING_BUILDREQUIRES = "missing_buildrequires"
    CONFIGURE_FAILURE = "configure_failure"
    COMPILE_ERROR = "compile_error"
    LINK_ERROR = "link_error"
    INSTALL_ERROR = "install_error"
    UNPACKAGED_FILES = "unpackaged_files"
    SPEC_ERROR = "spec_error"
    TIMEOUT = "timeout"
    UNKNOWN = "unknown"


@dataclass
class BuildTask:
    """A single package to be built."""

    package: str
    srpm_path: Path | None = None  # None = needs fetch
    has_rules: bool = False
    has_rpms: bool = False          # Already built
    build_order: int = 0


@dataclass
class FailureClassification:
    """Classified build failure for triage."""

    category: FailureCategory
    details: str
    missing_deps: list[str] = field(default_factory=list)


@dataclass
class BuildResult:
    """Result of building a single package."""

    package: str
    status: BuildStatus
    rpms: list[str] = field(default_factory=list)
    candidate_rules_path: str | None = None
    failure: FailureClassification | None = None
    duration_seconds: float = 0
    findings: list[str] = field(default_factory=list)  # Source analysis findings

    def to_dict(self) -> dict:
        """Convert to JSON-serializable dict."""
        d = {
            "name": self.package,
            "status": self.status.value,
            "duration_seconds": round(self.duration_seconds, 1),
        }
        if self.rpms:
            d["rpms"] = self.rpms
        if self.candidate_rules_path:
            d["candidate_rules"] = self.candidate_rules_path
        if self.findings:
            d["findings"] = self.findings
        if self.failure:
            d["category"] = self.failure.category.value
            d["details"] = self.failure.details
            if self.failure.missing_deps:
                d["missing_deps"] = self.failure.missing_deps
        return d


@dataclass
class BatchOptions:
    """Options for a batch build run."""

    dry_run: bool = False
    generate_rules: bool = True
    stop_on_error: bool = False
    skip_fetch: bool = False
    skip_built: bool = True
    build_timeout: int = 600  # seconds
    release: str = "40"
    base_url: str | None = None


@dataclass
class BatchReport:
    """Report from a batch build run."""

    mode: str  # "list" or "roadmap"
    input_source: str  # filename or target package
    results: list[BuildResult] = field(default_factory=list)
    start_time: str = ""
    end_time: str = ""

    @property
    def summary(self) -> dict[str, int]:
        counts: dict[str, int] = {}
        for r in self.results:
            key = r.status.value
            counts[key] = counts.get(key, 0) + 1
        return counts

    def to_dict(self) -> dict:
        return {
            "timestamp": self.start_time,
            "mode": self.mode,
            "input": self.input_source,
            "packages": [r.to_dict() for r in self.results],
            "summary": self.summary,
        }


# ─── Failure Classifier ───────────────────────────────────────────────────


def classify_build_failure(output: str, timed_out: bool = False) -> FailureClassification:
    """Parse rpmbuild output and classify the failure.

    Args:
        output: Combined stdout+stderr from rpmbuild
        timed_out: Whether the build was killed due to timeout

    Returns:
        FailureClassification with category and details
    """
    if timed_out:
        return FailureClassification(
            category=FailureCategory.TIMEOUT,
            details="Build exceeded timeout limit",
        )

    # Check for missing BuildRequires
    if "Failed build dependencies" in output or "is needed by" in output:
        missing = []
        for line in output.splitlines():
            m = re.match(r"^\s+(\S+?)(?:\s*[<>=]+\s*\S+)?\s+is needed by", line)
            if m:
                missing.append(m.group(1))
        return FailureClassification(
            category=FailureCategory.MISSING_BUILDREQUIRES,
            details=f"{len(missing)} missing dep(s)",
            missing_deps=missing,
        )

    # Check patterns in reverse build order (later phases = more specific)

    # Unpackaged files
    if "Installed (but unpackaged) file(s) found" in output:
        return FailureClassification(
            category=FailureCategory.UNPACKAGED_FILES,
            details="Installed files not listed in %files section",
        )

    # Link errors
    link_patterns = [
        r"ld\.lld: error:",
        r"undefined reference to",
        r"ld: error:",
        r"collect2: error: ld returned",
    ]
    for pattern in link_patterns:
        m = re.search(pattern, output)
        if m:
            # Extract the specific error line
            for line in output.splitlines():
                if re.search(pattern, line):
                    detail = line.strip()[:200]
                    break
            else:
                detail = "Linker error"
            return FailureClassification(
                category=FailureCategory.LINK_ERROR,
                details=detail,
            )

    # Compile errors
    compile_patterns = [
        r"error: implicit declaration of function",
        r"error: unknown type name",
        r"error: use of undeclared identifier",
        r"fatal error: .+ file not found",
    ]
    for pattern in compile_patterns:
        m = re.search(pattern, output)
        if m:
            for line in output.splitlines():
                if re.search(pattern, line):
                    detail = line.strip()[:200]
                    break
            else:
                detail = "Compilation error"
            return FailureClassification(
                category=FailureCategory.COMPILE_ERROR,
                details=detail,
            )

    # Configure errors
    if "configure: error:" in output:
        for line in output.splitlines():
            if "configure: error:" in line:
                detail = line.strip()[:200]
                break
        else:
            detail = "Configure error"
        return FailureClassification(
            category=FailureCategory.CONFIGURE_FAILURE,
            details=detail,
        )

    # Spec errors
    spec_patterns = [
        r"error: Bad exit status",
        r"error: line \d+:",
        r"Macro %\S+ not found",
    ]
    for pattern in spec_patterns:
        if re.search(pattern, output):
            for line in output.splitlines():
                if re.search(pattern, line):
                    detail = line.strip()[:200]
                    break
            else:
                detail = "Spec processing error"
            return FailureClassification(
                category=FailureCategory.SPEC_ERROR,
                details=detail,
            )

    # Unknown
    # Try to extract the last error-looking line
    last_error = ""
    for line in output.splitlines():
        if "error" in line.lower() or "Error" in line:
            last_error = line.strip()[:200]
    return FailureClassification(
        category=FailureCategory.UNKNOWN,
        details=last_error or "Unknown failure — check full build log",
    )


# ─── Batch Builder ─────────────────────────────────────────────────────────


class BatchBuilder:
    """Orchestrates batch fetch → convert → build pipeline."""

    def __init__(
        self,
        rules_dir: Path,
        compat_dir: Path,
        headers_dir: Path,
        inputs_dir: Path,
        outputs_dir: Path,
    ):
        self.rules_dir = rules_dir
        self.compat_dir = compat_dir
        self.headers_dir = headers_dir
        self.inputs_dir = inputs_dir
        self.outputs_dir = outputs_dir

        self.rule_loader = RuleLoader(rules_dir)
        self.rule_generator = RuleGenerator(rules_dir, compat_dir)
        self.dep_resolver = DependencyResolver(rules_dir)

    def resolve_tasks_from_list(
        self, list_path: Path, options: BatchOptions
    ) -> list[BuildTask]:
        """Read a package list file and resolve to BuildTasks.

        File format: one package name per line, # comments, blank lines ignored.
        """
        packages = []
        with open(list_path) as f:
            for line in f:
                line = line.strip()
                if line and not line.startswith("#"):
                    packages.append(line)

        return self._resolve_packages(packages, options)

    def resolve_tasks_from_roadmap(
        self, target: str, options: BatchOptions
    ) -> list[BuildTask]:
        """Use RoadmapResolver to get dependency-ordered tasks.

        Requires repo metadata index. Falls back gracefully if unavailable.
        """
        from mogrix.repometa import RepoMetaCache
        from mogrix.roadmap import Classification, RoadmapResolver

        cache = RepoMetaCache(release=options.release, base_url=options.base_url)
        db = cache.ensure_index(refresh=False)

        resolver = RoadmapResolver(
            db=db,
            rule_loader=self.rule_loader,
            rules_dir=self.rules_dir,
            rpms_dir=self.outputs_dir / "RPMS",
            stop_at_rules=False,
            max_depth=0,
        )

        result = resolver.resolve(target)

        # Filter to packages that need building
        buildable_classifications = {
            Classification.NEED_RULES,
            Classification.HAS_RULES,
        }

        tasks = []
        for pkg_name in result.build_order:
            pkg_info = result.packages.get(pkg_name)
            if not pkg_info:
                continue
            if pkg_info.classification not in buildable_classifications:
                continue

            srpm_path = self._find_srpm(pkg_name)
            has_rules = self.rule_loader.load_package(pkg_name) is not None
            has_rpms = self._check_has_rpms(pkg_name)

            tasks.append(BuildTask(
                package=pkg_name,
                srpm_path=srpm_path,
                has_rules=has_rules,
                has_rpms=has_rpms,
                build_order=pkg_info.build_order,
            ))

        return tasks

    def _resolve_packages(
        self, packages: list[str], options: BatchOptions
    ) -> list[BuildTask]:
        """Convert package names to BuildTasks."""
        tasks = []
        for i, pkg in enumerate(packages):
            srpm_path = self._find_srpm(pkg)
            has_rules = self.rule_loader.load_package(pkg) is not None
            has_rpms = self._check_has_rpms(pkg)

            tasks.append(BuildTask(
                package=pkg,
                srpm_path=srpm_path,
                has_rules=has_rules,
                has_rpms=has_rpms,
                build_order=i,
            ))

        return tasks

    def _find_srpm(self, package: str) -> Path | None:
        """Find an existing SRPM for a package in the inputs directory."""
        srpms_dir = self.inputs_dir / "SRPMS"
        if not srpms_dir.exists():
            return None

        # Look for SRPMs matching the package name
        for srpm in srpms_dir.glob(f"{package}-*.src.rpm"):
            # Verify the package name matches (not just a prefix)
            basename = srpm.name.replace(".src.rpm", "")
            parts = basename.rsplit("-", 2)
            if len(parts) >= 3 and parts[0] == package:
                return srpm
        return None

    def _check_has_rpms(self, package: str) -> bool:
        """Check if RPMs exist for a package in the output directory."""
        rpms_dir = self.outputs_dir / "RPMS"
        if not rpms_dir.exists():
            return False
        return any(rpms_dir.glob(f"{package}-*.rpm"))

    def _find_converted_srpm(self, package: str) -> Path | None:
        """Find a converted SRPM in the outputs directory."""
        converted_dir = self.outputs_dir / "SRPMS"
        if not converted_dir.exists():
            return None

        # Check for converted SRPM directory
        for d in converted_dir.iterdir():
            if d.is_dir() and d.name.startswith(package):
                # Find the .src.rpm inside
                for srpm in d.glob("*.src.rpm"):
                    return srpm
        return None

    def run(
        self, tasks: list[BuildTask], options: BatchOptions, report: BatchReport
    ) -> BatchReport:
        """Execute the batch build pipeline.

        Processes each task sequentially. Always moves on to next
        package on failure (unless --stop-on-error).

        Args:
            tasks: Ordered list of packages to build
            options: Build options
            report: Report to populate with results

        Returns:
            Populated BatchReport
        """
        from datetime import datetime

        report.start_time = datetime.now().isoformat(timespec="seconds")

        total = len(tasks)
        for i, task in enumerate(tasks):
            progress = f"[{i + 1}/{total}]"

            # Skip if already built
            if options.skip_built and task.has_rpms:
                console.print(
                    f"  {progress} [dim]{task.package}[/dim] — skipped (already built)"
                )
                report.results.append(BuildResult(
                    package=task.package,
                    status=BuildStatus.SKIPPED,
                ))
                continue

            start = time.monotonic()

            # Step 1: Fetch SRPM if needed
            if task.srpm_path is None:
                if options.skip_fetch:
                    console.print(
                        f"  {progress} [yellow]{task.package}[/yellow] — "
                        "skipped (no SRPM, --skip-fetch)"
                    )
                    report.results.append(BuildResult(
                        package=task.package,
                        status=BuildStatus.FETCH_FAILED,
                        failure=FailureClassification(
                            category=FailureCategory.UNKNOWN,
                            details="No SRPM found and --skip-fetch set",
                        ),
                    ))
                    continue

                console.print(
                    f"  {progress} [cyan]{task.package}[/cyan] — fetching SRPM..."
                )
                fetched = self._fetch_srpm(task.package, options)
                if fetched is None:
                    elapsed = time.monotonic() - start
                    console.print(
                        f"  {progress} [red]{task.package}[/red] — fetch failed"
                    )
                    report.results.append(BuildResult(
                        package=task.package,
                        status=BuildStatus.FETCH_FAILED,
                        duration_seconds=elapsed,
                        failure=FailureClassification(
                            category=FailureCategory.UNKNOWN,
                            details="SRPM not found in Fedora archives",
                        ),
                    ))
                    if options.stop_on_error:
                        break
                    continue
                task.srpm_path = fetched

            # Step 2: Generate candidate rules if none exist
            if not task.has_rules and options.generate_rules:
                console.print(
                    f"  {progress} [cyan]{task.package}[/cyan] — "
                    "generating candidate rules..."
                )
                result = self._generate_candidate_rules(task.package, task.srpm_path)
                elapsed = time.monotonic() - start

                findings = []
                candidate_path = None
                if result is not None:
                    candidate_path = str(result)
                    findings = self._summarize_candidate(result)

                console.print(
                    f"  {progress} [yellow]{task.package}[/yellow] — "
                    f"candidate rules → {candidate_path or 'none'}"
                )
                report.results.append(BuildResult(
                    package=task.package,
                    status=BuildStatus.NEEDS_REVIEW,
                    candidate_rules_path=candidate_path,
                    findings=findings,
                    duration_seconds=elapsed,
                ))
                if options.stop_on_error:
                    break
                continue

            # Step 3: Convert SRPM
            if options.dry_run:
                console.print(
                    f"  {progress} [bold]{task.package}[/bold] — "
                    "would convert + build"
                )
                report.results.append(BuildResult(
                    package=task.package,
                    status=BuildStatus.SUCCESS,
                ))
                continue

            console.print(
                f"  {progress} [cyan]{task.package}[/cyan] — converting..."
            )
            converted_srpm = self._convert(task.package, task.srpm_path)
            if converted_srpm is None:
                elapsed = time.monotonic() - start
                err_detail = getattr(self, "_last_convert_error", "Unknown")
                console.print(
                    f"  {progress} [red]{task.package}[/red] — "
                    f"convert failed: {err_detail[:80]}"
                )
                report.results.append(BuildResult(
                    package=task.package,
                    status=BuildStatus.CONVERT_FAILED,
                    duration_seconds=elapsed,
                    failure=FailureClassification(
                        category=FailureCategory.SPEC_ERROR,
                        details=err_detail[:200],
                    ),
                ))
                if options.stop_on_error:
                    break
                continue

            # Step 4: Build
            console.print(
                f"  {progress} [cyan]{task.package}[/cyan] — building..."
            )
            build_result = self._build(converted_srpm, options)
            elapsed = time.monotonic() - start

            if build_result.status == BuildStatus.SUCCESS:
                console.print(
                    f"  {progress} [green]{task.package}[/green] — "
                    f"success ({len(build_result.rpms)} RPMs)"
                )
            else:
                cat = build_result.failure.category.value if build_result.failure else "unknown"
                console.print(
                    f"  {progress} [red]{task.package}[/red] — "
                    f"failed ({cat})"
                )

            build_result.duration_seconds = elapsed
            report.results.append(build_result)

            if build_result.status != BuildStatus.SUCCESS and options.stop_on_error:
                break

        report.end_time = datetime.now().isoformat(timespec="seconds")
        return report

    def _fetch_srpm(self, package: str, options: BatchOptions) -> Path | None:
        """Fetch an SRPM from Fedora archives."""
        from mogrix.deps.fedora import FedoraRepo

        srpms_dir = self.inputs_dir / "SRPMS"
        srpms_dir.mkdir(parents=True, exist_ok=True)

        try:
            repo = FedoraRepo(release=options.release, base_url=options.base_url)
            matches = repo.search_packages(package)

            # Find exact match
            exact = [m for m in matches if m.name == package]
            if exact:
                selected = exact[0]
            elif matches:
                selected = matches[0]  # Take first fuzzy match
            else:
                return None

            downloaded = repo.download_srpm(selected.url, srpms_dir)
            return downloaded

        except Exception:
            return None

    def _generate_candidate_rules(
        self, package: str, srpm_path: Path
    ) -> Path | None:
        """Generate candidate rules for a package without rules."""
        candidates_dir = self.rules_dir / "candidates"

        try:
            candidate = self.rule_generator.generate_from_srpm(package, srpm_path)
            if candidate.has_rules or candidate.todos:
                return self.rule_generator.write_candidate(candidate, candidates_dir)
            else:
                # Even empty candidates get written (minimal YAML for human to start from)
                return self.rule_generator.write_candidate(candidate, candidates_dir)
        except Exception:
            return None

    def _summarize_candidate(self, candidate_path: Path) -> list[str]:
        """Extract a summary of findings from a candidate file."""
        findings = []
        try:
            text = candidate_path.read_text()
            for line in text.splitlines():
                if line.startswith("  # confidence:"):
                    findings.append(line.strip().lstrip("# "))
        except Exception:
            pass
        return findings

    def _convert(self, package: str, srpm_path: Path) -> Path | None:
        """Convert an SRPM using BatchConverter.

        Returns path to converted SRPM on success, None on failure.
        Stores error details on self._last_convert_error for reporting.
        """
        self._last_convert_error = ""
        output_dir = self.outputs_dir / "SRPMS"
        output_dir.mkdir(parents=True, exist_ok=True)

        try:
            converter = BatchConverter(
                srpms_dir=srpm_path.parent,
                rules_dir=self.rules_dir,
                headers_dir=self.headers_dir,
                compat_dir=self.compat_dir,
            )
            result = converter.convert_one(srpm_path, output_dir)

            if result["status"] == "success" and result.get("output_srpm"):
                return Path(result["output_srpm"])
            self._last_convert_error = result.get("error", "Unknown conversion error")
            return None
        except Exception as e:
            self._last_convert_error = str(e)
            return None

    def _build(self, converted_srpm: Path, options: BatchOptions) -> BuildResult:
        """Run rpmbuild --cross on a converted SRPM."""
        rpmbuild_path = Path.home() / "rpmbuild"
        macros_path = Path("/opt/sgug-staging/rpmmacros.irix")
        out_rpms = self.outputs_dir / "RPMS"
        out_rpms.mkdir(parents=True, exist_ok=True)

        # Ensure rpmbuild directories exist
        for subdir in ["SOURCES", "SPECS", "BUILD", "RPMS", "SRPMS"]:
            (rpmbuild_path / subdir).mkdir(parents=True, exist_ok=True)

        # Snapshot existing RPM mtimes before build (to find only new ones)
        rpms_dir = rpmbuild_path / "RPMS"
        existing_mtimes = {}
        if rpms_dir.exists():
            for rpm in rpms_dir.glob("**/*.rpm"):
                existing_mtimes[rpm] = rpm.stat().st_mtime

        # Build command (mirrors cli.py build --cross)
        macro_chain = f"/usr/lib/rpm/macros:/usr/lib/rpm/macros.d/*:{macros_path}"
        cmd = [
            "rpmbuild",
            "--macros", macro_chain,
            "--nodeps", "--nocheck",
            "--target", "mips-sgi-irix",
            "--define", "_target_cpu mips",
            "--define", "_target_os irix",
            "--define", "_arch mips",
            "--define", f"_topdir {rpmbuild_path}",
            "--rebuild", str(converted_srpm),
        ]

        package = converted_srpm.name.rsplit("-", 2)[0]

        try:
            proc = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=options.build_timeout,
            )
        except subprocess.TimeoutExpired:
            return BuildResult(
                package=package,
                status=BuildStatus.TIMEOUT,
                failure=FailureClassification(
                    category=FailureCategory.TIMEOUT,
                    details=f"Build killed after {options.build_timeout}s",
                ),
            )

        combined_output = proc.stdout + proc.stderr

        if proc.returncode == 0:
            # Find only newly built or modified RPMs
            rpm_names = []
            new_rpms = []
            if rpms_dir.exists():
                for rpm in rpms_dir.glob("**/*.rpm"):
                    old_mtime = existing_mtimes.get(rpm)
                    if old_mtime is None or rpm.stat().st_mtime > old_mtime:
                        new_rpms.append(rpm)

            for rpm in sorted(new_rpms):
                dest = out_rpms / rpm.name
                shutil.copy2(rpm, dest)
                rpm_names.append(rpm.name)

            return BuildResult(
                package=package,
                status=BuildStatus.SUCCESS,
                rpms=rpm_names,
            )
        else:
            failure = classify_build_failure(combined_output)
            return BuildResult(
                package=package,
                status=BuildStatus.BUILD_FAILED,
                failure=failure,
            )


# ─── Display Helpers ───────────────────────────────────────────────────────


def print_report(report: BatchReport):
    """Print a Rich table summary of the batch build."""
    table = Table(title="Batch Build Results")
    table.add_column("Package", style="bold")
    table.add_column("Status")
    table.add_column("Category")
    table.add_column("Notes")
    table.add_column("Time", justify="right")

    for r in report.results:
        # Status styling
        if r.status == BuildStatus.SUCCESS:
            status = "[green]SUCCESS[/green]"
        elif r.status == BuildStatus.SKIPPED:
            status = "[dim]SKIPPED[/dim]"
        elif r.status == BuildStatus.NEEDS_REVIEW:
            status = "[yellow]NEEDS_REVIEW[/yellow]"
        else:
            status = f"[red]{r.status.value.upper()}[/red]"

        # Category
        category = ""
        if r.failure:
            category = r.failure.category.value

        # Notes
        notes = ""
        if r.rpms:
            notes = f"{len(r.rpms)} RPMs"
        elif r.candidate_rules_path:
            notes = Path(r.candidate_rules_path).name
        elif r.failure:
            notes = r.failure.details[:60]

        # Duration
        duration = f"{r.duration_seconds:.0f}s" if r.duration_seconds > 0 else ""

        table.add_row(r.package, status, category, notes, duration)

    console.print(table)

    # Summary line
    s = report.summary
    parts = []
    total = sum(s.values())
    parts.append(f"{total} attempted")
    if s.get("success"):
        parts.append(f"[green]{s['success']} success[/green]")
    if s.get("skipped"):
        parts.append(f"[dim]{s['skipped']} skipped[/dim]")
    if s.get("needs_review"):
        parts.append(f"[yellow]{s['needs_review']} needs_review[/yellow]")
    failed = sum(v for k, v in s.items() if k not in ("success", "skipped", "needs_review"))
    if failed:
        parts.append(f"[red]{failed} failed[/red]")

    console.print(f"\n[bold]Summary:[/bold] {' | '.join(parts)}")

    # List generated candidates
    candidates = [
        r.candidate_rules_path
        for r in report.results
        if r.candidate_rules_path
    ]
    if candidates:
        console.print(f"\n[bold]Candidates written:[/bold]")
        for c in candidates:
            console.print(f"  {c}")


def write_json_report(report: BatchReport, output_path: Path):
    """Write the batch report as JSON."""
    output_path.parent.mkdir(parents=True, exist_ok=True)
    with open(output_path, "w") as f:
        json.dump(report.to_dict(), f, indent=2)
    console.print(f"\n[bold]Report written:[/bold] {output_path}")
