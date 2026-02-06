"""Source-level static analysis for IRIX compatibility.

Scans extracted source trees for known IRIX-incompatible patterns.
Patterns are defined in rules/source_checks.yaml and compat/catalog.yaml,
not in this code. Adding a new check = adding a YAML entry.

Uses ripgrep (rg) for fast regex searching across source trees.
"""

import subprocess
from dataclasses import dataclass, field
from pathlib import Path

import yaml


@dataclass
class SourceFinding:
    """A single finding from source analysis."""

    check_id: str
    severity: str  # "error", "warning", "info"
    message: str
    fix: str
    file: str  # Relative path within source tree
    line: int
    match_text: str
    handled: bool = False  # True if existing rules address this
    handled_by: str = ""  # Description of what rule handles it


@dataclass
class SourceAnalysisResult:
    """Result of scanning a source tree."""

    findings: list[SourceFinding] = field(default_factory=list)
    checks_run: int = 0
    files_scanned: int = 0

    @property
    def errors(self) -> list[SourceFinding]:
        return [f for f in self.findings if f.severity == "error" and not f.handled]

    @property
    def warnings(self) -> list[SourceFinding]:
        return [f for f in self.findings if f.severity == "warning" and not f.handled]

    @property
    def info(self) -> list[SourceFinding]:
        return [f for f in self.findings if f.severity == "info" and not f.handled]

    @property
    def handled(self) -> list[SourceFinding]:
        return [f for f in self.findings if f.handled]

    @property
    def unhandled(self) -> list[SourceFinding]:
        return [f for f in self.findings if not f.handled]


@dataclass
class SourceCheck:
    """A check loaded from source_checks.yaml."""

    id: str
    severity: str
    pattern: str
    glob: str
    message: str
    fix: str
    reference: str = ""
    handled_by: list[str] = field(default_factory=list)


@dataclass
class CatalogPattern:
    """A pattern loaded from catalog.yaml source_patterns."""

    function_name: str
    pattern: str
    note: str
    glob: str = "*.c,*.h"


class SourceAnalyzer:
    """Scans extracted source for known IRIX-incompatible patterns.

    Loads patterns from two sources:
    1. rules/source_checks.yaml — issue-level patterns (need patches/overrides)
    2. compat/catalog.yaml — function-level patterns (need compat injection)
    """

    def __init__(
        self,
        source_checks_path: Path | None = None,
        catalog_path: Path | None = None,
    ):
        project_root = Path(__file__).parent.parent.parent
        if source_checks_path is None:
            source_checks_path = project_root / "rules" / "source_checks.yaml"
        if catalog_path is None:
            catalog_path = project_root / "compat" / "catalog.yaml"

        self.checks = self._load_checks(source_checks_path)
        self.catalog_patterns = self._load_catalog_patterns(catalog_path)

    def _load_checks(self, path: Path) -> list[SourceCheck]:
        """Load checks from source_checks.yaml."""
        if not path.exists():
            return []
        with open(path) as f:
            data = yaml.safe_load(f)
        if not data or "checks" not in data:
            return []
        checks = []
        for entry in data["checks"]:
            checks.append(
                SourceCheck(
                    id=entry["id"],
                    severity=entry.get("severity", "warning"),
                    pattern=entry["pattern"],
                    glob=entry.get("glob", "*.c,*.h"),
                    message=entry["message"],
                    fix=entry.get("fix", ""),
                    reference=entry.get("reference", ""),
                    handled_by=entry.get("handled_by", []),
                )
            )
        return checks

    def _load_catalog_patterns(self, path: Path) -> list[CatalogPattern]:
        """Load source_patterns from catalog.yaml."""
        if not path.exists():
            return []
        with open(path) as f:
            data = yaml.safe_load(f)
        if not data or "functions" not in data:
            return []
        patterns = []
        for func_name, func_data in data["functions"].items():
            for sp in func_data.get("source_patterns", []):
                patterns.append(
                    CatalogPattern(
                        function_name=func_name,
                        pattern=sp["pattern"],
                        note=sp.get("note", ""),
                    )
                )
        return patterns

    def scan_directory(
        self,
        source_dir: Path,
        handled_compat_functions: list[str] | None = None,
        handled_rules: dict | None = None,
    ) -> SourceAnalysisResult:
        """Scan an extracted source tree for known patterns.

        Args:
            source_dir: Path to extracted source directory.
            handled_compat_functions: Compat functions already in package rules
                (for marking findings as handled during convert).
            handled_rules: Full package rules dict for cross-referencing
                handled_by entries.

        Returns:
            SourceAnalysisResult with all findings.
        """
        result = SourceAnalysisResult()
        handled_compat = set(handled_compat_functions or [])
        handled_rules = handled_rules or {}

        # Run source_checks.yaml patterns
        for check in self.checks:
            result.checks_run += 1
            findings = self._run_rg(source_dir, check.pattern, check.glob)
            for file_path, line_num, match_text in findings:
                finding = SourceFinding(
                    check_id=check.id,
                    severity=check.severity,
                    message=check.message,
                    fix=check.fix,
                    file=file_path,
                    line=line_num,
                    match_text=match_text,
                )
                # Check if this finding is handled by existing rules
                if check.handled_by:
                    for handler in check.handled_by:
                        if self._is_handled(handler, handled_compat, handled_rules):
                            finding.handled = True
                            finding.handled_by = handler
                            break
                result.findings.append(finding)

        # Run catalog.yaml source_patterns
        for cp in self.catalog_patterns:
            result.checks_run += 1
            findings = self._run_rg(source_dir, cp.pattern, cp.glob)
            for file_path, line_num, match_text in findings:
                finding = SourceFinding(
                    check_id=f"compat:{cp.function_name}",
                    severity="info",
                    message=cp.note,
                    fix=f"Add inject_compat_functions: [{cp.function_name}]",
                    file=file_path,
                    line=line_num,
                    match_text=match_text,
                )
                # Mark as handled if the function is already injected
                if cp.function_name in handled_compat:
                    finding.handled = True
                    finding.handled_by = f"compat_functions:{cp.function_name}"
                result.findings.append(finding)

        # Deduplicate: keep one finding per (check_id, file) pair
        result.findings = self._deduplicate(result.findings)

        return result

    def scan_tarball(
        self,
        tarball: Path,
        workdir: Path,
        handled_compat_functions: list[str] | None = None,
        handled_rules: dict | None = None,
    ) -> SourceAnalysisResult:
        """Extract a tarball and scan its contents.

        Args:
            tarball: Path to .tar.gz, .tar.bz2, .tar.xz, etc.
            workdir: Directory to extract into (created if needed).
            handled_compat_functions: See scan_directory.
            handled_rules: See scan_directory.

        Returns:
            SourceAnalysisResult with findings.
        """
        workdir.mkdir(parents=True, exist_ok=True)

        # Detect compression and extract
        name = tarball.name.lower()
        if name.endswith((".tar.gz", ".tgz")):
            tar_flag = "z"
        elif name.endswith((".tar.bz2", ".tbz2")):
            tar_flag = "j"
        elif name.endswith((".tar.xz", ".txz")):
            tar_flag = "J"
        elif name.endswith(".tar"):
            tar_flag = ""
        else:
            # Try generic tar
            tar_flag = ""

        cmd = ["tar", f"-x{tar_flag}f", str(tarball), "-C", str(workdir)]
        proc = subprocess.run(cmd, capture_output=True, text=True)
        if proc.returncode != 0:
            # Return empty result if extraction fails
            return SourceAnalysisResult()

        # Find the top-level directory (tarballs usually have one)
        contents = list(workdir.iterdir())
        if len(contents) == 1 and contents[0].is_dir():
            source_dir = contents[0]
        else:
            source_dir = workdir

        return self.scan_directory(
            source_dir,
            handled_compat_functions=handled_compat_functions,
            handled_rules=handled_rules,
        )

    def _run_rg(
        self, search_dir: Path, pattern: str, glob_str: str
    ) -> list[tuple[str, int, str]]:
        """Run ripgrep and return matches as (file, line, text) tuples.

        Args:
            search_dir: Directory to search.
            pattern: Regex pattern.
            glob_str: Comma-separated glob patterns (e.g. "*.c,*.h").

        Returns:
            List of (relative_file_path, line_number, matched_text).
        """
        cmd = [
            "rg",
            "-n",            # line numbers
            "--no-heading",  # file:line:text format (one line per match)
        ]

        # Add glob filters
        for g in glob_str.split(","):
            g = g.strip()
            if g:
                cmd.extend(["--glob", g])

        cmd.extend(["-e", pattern, str(search_dir)])

        try:
            proc = subprocess.run(
                cmd,
                capture_output=True,
                text=True,
                timeout=30,
            )
        except (subprocess.TimeoutExpired, FileNotFoundError):
            return []

        # rg returns 1 for no matches, 2+ for errors
        if proc.returncode > 1:
            return []

        results = []
        for line in proc.stdout.splitlines():
            # Format: /path/to/file:123:matched text
            parts = line.split(":", 2)
            if len(parts) >= 3:
                file_path = parts[0]
                try:
                    line_num = int(parts[1])
                except ValueError:
                    continue
                match_text = parts[2].strip()

                # Make path relative to search_dir
                try:
                    rel_path = str(Path(file_path).relative_to(search_dir))
                except ValueError:
                    rel_path = file_path

                results.append((rel_path, line_num, match_text))

        return results

    def _is_handled(
        self,
        handler: str,
        handled_compat: set[str],
        handled_rules: dict,
    ) -> bool:
        """Check if a handler spec is satisfied by the package rules.

        Handler format: "field:value" (e.g. "compat_functions:explicit_bzero")
        """
        if ":" not in handler:
            return False
        field, value = handler.split(":", 1)
        if field == "compat_functions":
            return value in handled_compat
        # Extensible: add more field types as needed
        return False

    def _deduplicate(
        self, findings: list[SourceFinding]
    ) -> list[SourceFinding]:
        """Keep only one finding per (check_id, file) pair.

        Prefers the first occurrence (lowest line number).
        """
        seen: dict[tuple[str, str], SourceFinding] = {}
        for f in findings:
            key = (f.check_id, f.file)
            if key not in seen:
                seen[key] = f
        return list(seen.values())
