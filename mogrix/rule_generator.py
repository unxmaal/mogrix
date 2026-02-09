"""Candidate rule file generator for mogrix batch-build.

Analyzes source tarballs and spec files to generate candidate YAML rule files
with >90% confidence. Generated files go to rules/candidates/ for human review
before promotion to rules/packages/.
"""

import subprocess
import tempfile
from dataclasses import dataclass, field
from datetime import datetime
from pathlib import Path

import yaml

from mogrix.analyzers.source import SourceAnalyzer, SourceAnalysisResult
from mogrix.parser.spec import SpecFile, SpecParser
from mogrix.parser.srpm import SRPMExtractor


# Known Linux-only BuildRequires that are safe to drop (>95% confidence)
KNOWN_LINUX_DEPS = {
    # systemd ecosystem
    "systemd", "systemd-devel", "systemd-rpm-macros", "systemd-units",
    "systemd-libs",
    # SELinux
    "libselinux-devel", "libsepol-devel", "checkpolicy", "policycoreutils",
    "selinux-policy", "selinux-policy-devel",
    # PAM
    "pam-devel", "pam",
    # Linux kernel-specific
    "audit-libs-devel", "libcap-devel", "libcap-ng-devel",
    "libseccomp-devel", "libfido2-devel", "libmnl-devel",
    "kernel-headers", "linux-firmware",
    # Fedora packaging infrastructure
    "perl-generators", "python3-rpm-generators",
    "python3-rpm-macros", "pyproject-rpm-macros",
    # Documentation tools (not needed for cross-build)
    "texinfo", "texinfo-tex", "help2man",
    # D-Bus (not available on IRIX)
    "dbus-devel", "dbus-glib-devel",
    # Kerberos (usually optional)
    "krb5-devel",
}

# Glob patterns for Linux-only deps
KNOWN_LINUX_PATTERNS = [
    "systemd*",
    "selinux*",
]


@dataclass
class CandidateRule:
    """A single rule with confidence annotation."""

    confidence: int  # 0-100
    reason: str
    yaml_key: str  # e.g., "inject_compat_functions", "drop_buildrequires"
    value: object  # The YAML value


@dataclass
class CandidateRules:
    """Generated candidate rules for a package."""

    package: str
    rules: list[CandidateRule] = field(default_factory=list)
    classes: list[str] = field(default_factory=list)
    todos: list[str] = field(default_factory=list)
    files_scanned: int = 0
    findings_count: int = 0

    @property
    def has_rules(self) -> bool:
        return bool(self.rules) or bool(self.classes)


class RuleGenerator:
    """Generates candidate YAML rule files from source analysis."""

    def __init__(
        self,
        rules_dir: Path,
        compat_dir: Path | None = None,
    ):
        project_root = Path(__file__).parent.parent
        self.rules_dir = rules_dir
        self.compat_dir = compat_dir or project_root / "compat"
        self.analyzer = SourceAnalyzer(
            source_checks_path=rules_dir / "source_checks.yaml",
            catalog_path=self.compat_dir / "catalog.yaml",
        )

    def generate_from_srpm(
        self,
        package: str,
        srpm_path: Path,
    ) -> CandidateRules:
        """Analyze an SRPM and generate candidate rules.

        Extracts the SRPM, scans source for compat needs, and inspects
        the spec for droppable BuildRequires and NLS patterns.

        Args:
            package: Package name
            srpm_path: Path to the .src.rpm file

        Returns:
            CandidateRules with detected rules and TODOs
        """
        candidate = CandidateRules(package=package)

        with tempfile.TemporaryDirectory(prefix="mogrix-rulegen-") as tmpdir:
            tmpdir_path = Path(tmpdir)

            # Extract SRPM
            try:
                extractor = SRPMExtractor(srpm_path)
                extracted_dir, spec_path = extractor.extract_spec()
            except Exception:
                candidate.todos.append("SRPM extraction failed — manual review needed")
                return candidate

            try:
                # Parse spec for BuildRequires analysis
                parser = SpecParser()
                spec = parser.parse(spec_path)

                # Detect droppable BuildRequires
                self._detect_droppable_buildrequires(spec, candidate)

                # Find source tarballs and scan them
                self._scan_sources(extracted_dir, tmpdir_path, candidate)

                # Detect NLS class candidate
                self._detect_nls_class(extracted_dir, tmpdir_path, candidate)

                # Detect MIPS assembly files that might need -fno-integrated-as
                self._detect_mips_assembly(extracted_dir, tmpdir_path, candidate)

            finally:
                # Clean up extracted SRPM
                import shutil
                if extracted_dir.exists():
                    shutil.rmtree(extracted_dir)

        return candidate

    def _detect_droppable_buildrequires(
        self, spec: SpecFile, candidate: CandidateRules
    ):
        """Detect BuildRequires that are known Linux-only."""
        droppable = []
        for br in spec.buildrequires:
            # Strip version specifier for matching
            dep_name = br.split()[0] if br else br
            if dep_name in KNOWN_LINUX_DEPS:
                droppable.append(dep_name)
            else:
                # Check glob patterns
                from fnmatch import fnmatch
                for pattern in KNOWN_LINUX_PATTERNS:
                    if fnmatch(dep_name, pattern):
                        droppable.append(dep_name)
                        break

        if droppable:
            candidate.rules.append(CandidateRule(
                confidence=95,
                reason="Known Linux-only BuildRequires",
                yaml_key="drop_buildrequires",
                value=sorted(set(droppable)),
            ))

    def _scan_sources(
        self,
        extracted_dir: Path,
        workdir: Path,
        candidate: CandidateRules,
    ):
        """Scan source tarballs for compat function needs."""
        # Find source tarballs in extracted SRPM
        tarball_exts = {".tar.gz", ".tgz", ".tar.bz2", ".tbz2", ".tar.xz", ".txz"}
        tarballs = [
            f for f in extracted_dir.iterdir()
            if f.is_file() and any(f.name.endswith(ext) for ext in tarball_exts)
        ]

        if not tarballs:
            # Try scanning the extracted directory directly (no tarball)
            return

        # Scan the first/main tarball
        tarball = tarballs[0]
        scan_dir = workdir / "scan"
        result = self.analyzer.scan_tarball(tarball, scan_dir)

        candidate.files_scanned = result.files_scanned
        candidate.findings_count = len(result.findings)

        # Collect unhandled compat function needs
        compat_functions = set()
        for finding in result.unhandled:
            if finding.check_id.startswith("compat:"):
                func_name = finding.check_id.split(":", 1)[1]
                compat_functions.add(func_name)

        if compat_functions:
            candidate.rules.append(CandidateRule(
                confidence=95,
                reason="SourceAnalyzer detected function usage in C source",
                yaml_key="inject_compat_functions",
                value=sorted(compat_functions),
            ))

        # Add TODOs for non-compat findings
        for finding in result.unhandled:
            if not finding.check_id.startswith("compat:") and finding.severity == "error":
                candidate.todos.append(
                    f"{finding.check_id}: {finding.message} ({finding.file}:{finding.line})"
                )

    def _detect_nls_class(
        self,
        extracted_dir: Path,
        workdir: Path,
        candidate: CandidateRules,
    ):
        """Detect if the package uses GNU gettext (NLS class candidate)."""
        # Find source tarballs
        tarball_exts = {".tar.gz", ".tgz", ".tar.bz2", ".tbz2", ".tar.xz", ".txz"}
        tarballs = [
            f for f in extracted_dir.iterdir()
            if f.is_file() and any(f.name.endswith(ext) for ext in tarball_exts)
        ]

        if not tarballs:
            return

        # Check if configure.ac/configure.in contains AM_GNU_GETTEXT
        scan_dir = workdir / "scan"
        if not scan_dir.exists():
            # Extract if not already done
            tarball = tarballs[0]
            self.analyzer.scan_tarball(tarball, scan_dir)

        # Look for AM_GNU_GETTEXT in configure.ac or configure.in
        try:
            result = subprocess.run(
                ["rg", "-l", "AM_GNU_GETTEXT", "--glob", "configure.*",
                 "--glob", "*.ac", "--glob", "*.in", str(scan_dir)],
                capture_output=True, text=True, timeout=10,
            )
            if result.returncode == 0 and result.stdout.strip():
                candidate.classes.append("nls-disabled")
                candidate.rules.append(CandidateRule(
                    confidence=92,
                    reason="configure.ac contains AM_GNU_GETTEXT",
                    yaml_key="_class_nls",
                    value="nls-disabled",
                ))
        except (subprocess.TimeoutExpired, FileNotFoundError):
            pass

    def _detect_mips_assembly(
        self,
        extracted_dir: Path,
        workdir: Path,
        candidate: CandidateRules,
    ):
        """Detect MIPS assembly files that may need special handling."""
        scan_dir = workdir / "scan"
        if not scan_dir.exists():
            return

        try:
            result = subprocess.run(
                ["rg", "-l", r"\.(cpsetup|gpword|gpdword|set\s+mips)",
                 "--glob", "*.S", "--glob", "*.s", str(scan_dir)],
                capture_output=True, text=True, timeout=10,
            )
            if result.returncode == 0 and result.stdout.strip():
                asm_files = result.stdout.strip().splitlines()
                candidate.todos.append(
                    f"MIPS assembly detected in {len(asm_files)} file(s) — "
                    "may need -fno-integrated-as (LLVM integrated assembler bug)"
                )
        except (subprocess.TimeoutExpired, FileNotFoundError):
            pass

    def write_candidate(
        self,
        candidate: CandidateRules,
        output_dir: Path,
    ) -> Path:
        """Write candidate rules to a YAML file.

        Args:
            candidate: Generated candidate rules
            output_dir: Directory to write to (e.g., rules/candidates/)

        Returns:
            Path to the written candidate file
        """
        output_dir.mkdir(parents=True, exist_ok=True)
        output_path = output_dir / f"{candidate.package}.yaml"

        lines = []
        lines.append(f"# CANDIDATE RULES — generated by mogrix batch-build")
        lines.append(f"# Review and move to rules/packages/{candidate.package}.yaml to activate")
        lines.append(f"#")
        lines.append(f"# Source analysis: {candidate.files_scanned} files scanned, "
                     f"{candidate.findings_count} findings")
        lines.append(f"# Generated: {datetime.now().isoformat(timespec='seconds')}")
        lines.append(f"")
        lines.append(f"package: {candidate.package}")

        # Classes
        if candidate.classes:
            lines.append(f"classes: {candidate.classes}")
        lines.append(f"")

        # Rules section
        lines.append(f"rules:")

        # Group rules by key
        rules_by_key: dict[str, list[CandidateRule]] = {}
        for rule in candidate.rules:
            if rule.yaml_key.startswith("_"):
                continue  # Skip internal markers like _class_nls
            rules_by_key.setdefault(rule.yaml_key, []).append(rule)

        for key, rules in rules_by_key.items():
            for rule in rules:
                lines.append(f"  # confidence: {rule.confidence}% — {rule.reason}")
            # Emit the YAML value
            if isinstance(rules[0].value, list):
                lines.append(f"  {key}:")
                for item in rules[0].value:
                    lines.append(f"    - {item}")
            elif isinstance(rules[0].value, dict):
                lines.append(f"  {key}:")
                for k, v in rules[0].value.items():
                    lines.append(f"    {k}: {v!r}")
            else:
                lines.append(f"  {key}: {rules[0].value}")
            lines.append(f"")

        # If no rules were generated, add a minimal placeholder
        if not rules_by_key:
            lines.append(f"  # No high-confidence rules auto-detected")
            lines.append(f"  inject_compat_functions: []")
            lines.append(f"")

        # TODOs
        if candidate.todos:
            lines.append(f"# TODO (manual review needed):")
            for todo in candidate.todos:
                lines.append(f"# - {todo}")
            lines.append(f"")

        output_path.write_text("\n".join(lines) + "\n")
        return output_path
