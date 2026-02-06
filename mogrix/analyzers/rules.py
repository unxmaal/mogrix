"""Rule auditor for detecting duplicated rules across package yamls.

Scans all package rule files and reports patterns that appear in multiple
packages, flagging candidates for elevation to class or generic level.
"""

import re
from collections import defaultdict
from dataclasses import dataclass, field
from pathlib import Path

import yaml


@dataclass
class DuplicateEntry:
    """A single duplicated rule value and the packages that use it."""

    rule_key: str
    value: str
    packages: list[str]
    suggested_level: str  # "WATCH", "CLASS", "GENERIC"

    @property
    def count(self) -> int:
        return len(self.packages)


@dataclass
class AuditReport:
    """Result of auditing all package rules for duplication."""

    duplicates: list[DuplicateEntry] = field(default_factory=list)
    packages_scanned: int = 0
    generic_rules: dict = field(default_factory=dict)

    @property
    def class_candidates(self) -> list[DuplicateEntry]:
        return [d for d in self.duplicates if d.suggested_level == "CLASS"]

    @property
    def generic_candidates(self) -> list[DuplicateEntry]:
        return [d for d in self.duplicates if d.suggested_level == "GENERIC"]

    @property
    def watch_list(self) -> list[DuplicateEntry]:
        return [d for d in self.duplicates if d.suggested_level == "WATCH"]


class RuleAuditor:
    """Scans all package yamls and detects duplicated rules."""

    # Threshold: 2 = WATCH, 3+ = CLASS candidate
    WATCH_THRESHOLD = 2
    CLASS_THRESHOLD = 3

    def __init__(self, rules_dir: Path):
        self.rules_dir = Path(rules_dir)

    def audit(self) -> AuditReport:
        """Load all package yamls, count rule values, flag duplicates."""
        report = AuditReport()

        # Load generic rules to exclude already-elevated values
        generic_path = self.rules_dir / "generic.yaml"
        if generic_path.exists():
            with open(generic_path) as f:
                generic_data = yaml.safe_load(f)
            report.generic_rules = generic_data.get("generic", {})

        generic_drop_br = set(report.generic_rules.get("drop_buildrequires", []))
        generic_ac_cv = report.generic_rules.get("ac_cv_overrides", {})
        generic_disable = set(report.generic_rules.get("configure_disable", []))

        # Trackers: rule_key -> value -> [package_names]
        trackers: dict[str, dict[str, list[str]]] = defaultdict(lambda: defaultdict(list))

        # Scan all package yamls
        packages_dir = self.rules_dir / "packages"
        if not packages_dir.exists():
            return report

        for yaml_file in sorted(packages_dir.glob("*.yaml")):
            pkg_name = yaml_file.stem
            report.packages_scanned += 1

            try:
                with open(yaml_file) as f:
                    data = yaml.safe_load(f)
            except (yaml.YAMLError, OSError):
                continue

            if not isinstance(data, dict):
                continue

            rules = data.get("rules", {})
            if not isinstance(rules, dict):
                continue

            # Track ac_cv_overrides (each key:value pair independently)
            for key, val in rules.get("ac_cv_overrides", {}).items():
                pair = f"{key}={val}"
                # Skip if already in generic
                if generic_ac_cv.get(key) == val:
                    continue
                trackers["ac_cv_overrides"][pair].append(pkg_name)

            # Track inject_compat_functions (each function independently)
            for func in rules.get("inject_compat_functions", []):
                trackers["inject_compat_functions"][func].append(pkg_name)

            # Track configure_flags.add (each flag independently)
            cfg = rules.get("configure_flags", {})
            if isinstance(cfg, dict):
                for flag in cfg.get("add", []):
                    trackers["configure_flags.add"][flag].append(pkg_name)
                for flag in cfg.get("remove", []):
                    trackers["configure_flags.remove"][flag].append(pkg_name)

            # Track configure_disable (each feature independently)
            for feature in rules.get("configure_disable", []):
                if feature not in generic_disable:
                    trackers["configure_disable"][feature].append(pkg_name)

            # Track drop_buildrequires (excluding generic's)
            for dep in rules.get("drop_buildrequires", []):
                if dep not in generic_drop_br:
                    trackers["drop_buildrequires"][dep].append(pkg_name)

            # Track boolean flags
            if rules.get("skip_find_lang"):
                trackers["skip_find_lang"]["true"].append(pkg_name)

            # Track common prep_command patterns
            for cmd in rules.get("prep_commands", []):
                if re.search(r"%zu|%zd|%zx", cmd):
                    trackers["prep_commands (pattern)"]["%zu fix"].append(pkg_name)
                if re.search(r"gnulib-tests|SUBDIRS", cmd):
                    trackers["prep_commands (pattern)"]["SUBDIRS removal"].append(pkg_name)

            # Track spec_replacement patterns
            for repl in rules.get("spec_replacements", []):
                pattern = repl.get("pattern", "")
                if "%find_lang" in pattern:
                    trackers["spec_replacements (pattern)"]["%find_lang removal"].append(pkg_name)
                if "AUTOPOINT=true" in repl.get("replacement", ""):
                    trackers["spec_replacements (pattern)"]["AUTOPOINT=true"].append(pkg_name)

            # Track classes used (informational)
            for cls in data.get("classes", []):
                trackers["classes"][cls].append(pkg_name)

        # Build duplicate entries from trackers
        for rule_key, value_map in sorted(trackers.items()):
            for value, packages in sorted(value_map.items(), key=lambda x: -len(x[1])):
                if len(packages) < self.WATCH_THRESHOLD:
                    continue

                if len(packages) >= self.CLASS_THRESHOLD:
                    level = "CLASS"
                else:
                    level = "WATCH"

                report.duplicates.append(
                    DuplicateEntry(
                        rule_key=rule_key,
                        value=value,
                        packages=packages,
                        suggested_level=level,
                    )
                )

        # Sort: CLASS first, then by count descending
        report.duplicates.sort(key=lambda d: (d.suggested_level != "CLASS", -d.count))

        return report
