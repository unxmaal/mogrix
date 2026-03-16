"""Post-conversion verification for mogrix spec files.

Two layers of checks:
1. Per-rule postconditions: verify each declared rule had an observable effect.
2. Universal invariants: detect Linux-isms that should never survive conversion.
"""

import re
from dataclasses import dataclass, field
from pathlib import Path


@dataclass
class PostconditionIssue:
    severity: str  # "error" or "warning"
    rule_type: str  # e.g. "drop_requires", "invariant"
    message: str


@dataclass
class PostconditionReport:
    package: str
    issues: list[PostconditionIssue] = field(default_factory=list)

    @property
    def passed(self) -> bool:
        return not any(i.severity == "error" for i in self.issues)

    def error(self, rule_type: str, message: str) -> None:
        self.issues.append(PostconditionIssue("error", rule_type, message))

    def warning(self, rule_type: str, message: str) -> None:
        self.issues.append(PostconditionIssue("warning", rule_type, message))


# --- Universal invariants: Linux-isms that should never be in a converted spec ---

# Patterns that are errors (should have been removed/replaced)
LINUX_POISON = [
    (re.compile(r"(?<!/usr/sgug)/usr/lib64/"), "Linux lib64 path (should be /usr/sgug/lib32/)"),
    (re.compile(r"-lfstack-protector|__stack_chk"), "stack protector reference (IRIX has none)"),
    (re.compile(r"%\{_unitdir\}"), "systemd unit directory macro"),
    (re.compile(r"%\{_tmpfilesdir\}"), "systemd tmpfiles directory macro"),
    (re.compile(r"%\{_sysusersdir\}"), "systemd sysusers directory macro"),
    (re.compile(r"%\{_presetdir\}"), "systemd preset directory macro"),
]

# Patterns that are warnings (suspicious but might be legitimate)
LINUX_SMELL = [
    (re.compile(r"(?<![#%])\b-lrt\b"), "-lrt link flag (IRIX has no librt)"),
    (re.compile(r"<linux/"), "linux/ kernel header include"),
    (re.compile(r"%\{_sysctldir\}"), "sysctl directory macro (Linux-specific)"),
]


def _check_invariants(spec_content: str, report: PostconditionReport) -> None:
    """Check universal invariants on a converted spec."""
    for pattern, message in LINUX_POISON:
        for match in pattern.finditer(spec_content):
            # Skip if inside a comment
            line_start = spec_content.rfind("\n", 0, match.start()) + 1
            line = spec_content[line_start:spec_content.find("\n", match.start())]
            if line.lstrip().startswith("#"):
                continue
            report.error("invariant", f"{message}: {line.strip()[:80]}")

    for pattern, message in LINUX_SMELL:
        for match in pattern.finditer(spec_content):
            line_start = spec_content.rfind("\n", 0, match.start()) + 1
            line = spec_content[line_start:spec_content.find("\n", match.start())]
            if line.lstrip().startswith("#"):
                continue
            report.warning("invariant", f"{message}: {line.strip()[:80]}")

    # Check BuildRequires/Requires for Linux-only packages
    linux_deps = {"systemd", "systemd-devel", "libselinux-devel", "libsepol-devel",
                  "pam-devel", "libcap-devel", "libseccomp-devel", "audit-libs-devel",
                  "kernel-headers"}
    for line in spec_content.splitlines():
        stripped = line.strip()
        if stripped.startswith("#"):
            continue
        if stripped.startswith("BuildRequires:") or stripped.startswith("Requires:"):
            for dep in linux_deps:
                if dep in stripped:
                    report.error("invariant", f"Linux-only dependency: {dep} in {stripped[:60]}")


# --- Per-rule postcondition checks ---

def _check_drop_requires(rules: dict, spec_content: str, report: PostconditionReport) -> None:
    """Verify dropped requires are absent from converted spec."""
    for dep in rules.get("drop_requires", []):
        # Check both Requires: and PreReq: lines
        for line in spec_content.splitlines():
            stripped = line.strip()
            if stripped.startswith("#"):
                continue
            if (stripped.startswith("Requires:") or stripped.startswith("PreReq:")) and dep in stripped:
                report.error("drop_requires", f"'{dep}' still in: {stripped[:80]}")


def _check_drop_buildrequires(rules: dict, spec_content: str, report: PostconditionReport) -> None:
    """Verify dropped build requires are absent."""
    for dep in rules.get("drop_buildrequires", []):
        for line in spec_content.splitlines():
            stripped = line.strip()
            if stripped.startswith("#"):
                continue
            if stripped.startswith("BuildRequires:") and dep in stripped:
                report.error("drop_buildrequires", f"'{dep}' still in: {stripped[:80]}")


def _check_spec_replacements(rules: dict, spec_content: str, report: PostconditionReport) -> None:
    """Verify spec_replacements had an effect.

    Checks whether the full original pattern text is still present in the spec.
    If the full pattern remains, the replacement didn't fire.
    """
    for repl in rules.get("spec_replacements", []):
        pattern = repl.get("pattern", "")
        replacement = repl.get("replacement", "")
        if not pattern:
            continue

        # Use first non-empty line for display
        display = next((l.strip() for l in pattern.split("\n") if l.strip()), pattern[:60])

        # Normalize trailing whitespace on each line for comparison,
        # since the spec may have different trailing spaces than the YAML pattern
        def _normalize(s: str) -> str:
            return "\n".join(line.rstrip() for line in s.split("\n"))

        pattern_norm = _normalize(pattern)
        spec_norm = _normalize(spec_content)

        if pattern_norm in spec_norm:
            # The full original pattern is still present — replacement didn't fire
            if replacement and _normalize(replacement) in spec_norm:
                # Both old and new exist — the replacement was inserted but the
                # original wasn't removed (possible with non-unique patterns)
                report.warning(
                    "spec_replacements",
                    f"pattern still present (alongside replacement): {display[:60]}",
                )
            else:
                report.error(
                    "spec_replacements",
                    f"pattern not replaced: {display[:60]}",
                )


def _check_configure_flags(rules: dict, spec_content: str, report: PostconditionReport) -> None:
    """Verify configure flags were added/removed."""
    flags = rules.get("configure_flags", {})

    for flag in flags.get("add", []):
        # Flag should appear somewhere in %configure, %build, or a configure invocation
        if flag not in spec_content:
            report.error("configure_flags.add", f"flag not found in spec: {flag}")

    for flag in flags.get("remove", []):
        # Flag should NOT appear (outside comments)
        for line in spec_content.splitlines():
            stripped = line.strip()
            if stripped.startswith("#"):
                continue
            if flag in stripped and ("configure" in stripped.lower() or "CFLAGS" in stripped or "LDFLAGS" in stripped):
                report.error("configure_flags.remove", f"flag still present: {flag} in {stripped[:60]}")


def _check_bcond_flip(rules: dict, spec_content: str, report: PostconditionReport) -> None:
    """Verify bcond defaults were flipped."""
    for bcond in rules.get("bcond_flip", []):
        # After flipping, we can't know the expected direction without the original.
        # But we CAN verify the bcond exists at all.
        if f"%bcond_with {bcond}" not in spec_content and f"%bcond_without {bcond}" not in spec_content:
            # bcond was completely removed or never existed
            report.warning("bcond_flip", f"bcond '{bcond}' not found in converted spec")


def _check_drop_subpackages(rules: dict, spec_content: str, report: PostconditionReport) -> None:
    """Verify dropped subpackages are gone."""
    for sub in rules.get("drop_subpackages", []):
        # Normalize: handle -n prefix
        sub_name = sub.lstrip("-n ").strip()
        # Check for %package <name> sections
        pattern = re.compile(rf"^%package\s+(-n\s+)?{re.escape(sub_name)}\s*$", re.MULTILINE)
        if pattern.search(spec_content):
            report.error("drop_subpackages", f"subpackage '{sub_name}' still has %package section")


def check_postconditions(
    package: str,
    rules: dict,
    spec_content: str,
) -> PostconditionReport:
    """Run all postcondition checks on a converted spec.

    Args:
        package: Package name
        rules: The 'rules' dict from the package YAML (the value under the 'rules:' key)
        spec_content: The converted spec file content
    """
    report = PostconditionReport(package=package)

    # Universal invariants (always checked)
    _check_invariants(spec_content, report)

    # Per-rule postconditions (only if rules exist)
    if rules:
        _check_drop_requires(rules, spec_content, report)
        _check_drop_buildrequires(rules, spec_content, report)
        _check_spec_replacements(rules, spec_content, report)
        _check_configure_flags(rules, spec_content, report)
        _check_bcond_flip(rules, spec_content, report)
        _check_drop_subpackages(rules, spec_content, report)

    return report
