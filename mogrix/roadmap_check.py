"""Roadmap accuracy validation against build reality.

Compares roadmap predictions against actual build outcomes to identify
false positives (packages predicted as needed but never actually required).

Used by `mogrix roadmap-check`.
"""

import json
import sqlite3
from collections import defaultdict
from dataclasses import dataclass, field
from fnmatch import fnmatch
from pathlib import Path

import yaml

from mogrix.roadmap import Classification, RoadmapResolver, RoadmapResult
from mogrix.rules.loader import RuleLoader


@dataclass
class FalsePositive:
    """A package predicted as NEED_RULES that was not actually needed."""

    name: str
    suggested_category: str  # build_infra, doc_only, test_only, etc.
    confidence: str  # HIGH, MEDIUM, LOW
    reason: str  # Why this categorization
    found_in: list[str] = field(default_factory=list)  # Which target packages found it


@dataclass
class CheckResult:
    """Result of a roadmap-check for a single package."""

    target: str
    total_need_rules: int = 0
    false_positives: list[FalsePositive] = field(default_factory=list)


class RoadmapChecker:
    """Validates roadmap predictions against build reality.

    For each built package, runs its roadmap and identifies NEED_RULES
    packages as false positives (since the build succeeded without them).
    """

    # Heuristic patterns for auto-categorization of false positives.
    # Maps category -> list of (glob_pattern, reason) tuples.
    CATEGORY_HEURISTICS: dict[str, list[tuple[str, str]]] = {
        "build_infra": [
            ("*-rpm-macros", "RPM macro package"),
            ("*-rpm-generators", "RPM generator package"),
            ("*-filesystem", "Filesystem layout package"),
            ("multilib-rpm-config", "Multilib config"),
            ("rpm-mpi-hooks", "MPI hooks"),
            ("rpm-local-generator-support", "RPM generator support"),
            ("web-assets", "Web assets infrastructure"),
            ("nodejs-packaging", "Node.js packaging macros"),
            ("AMF", "AMD media framework headers"),
            ("nv-codec-headers", "NVIDIA codec headers"),
            ("opencl-headers", "OpenCL headers"),
            ("checksec", "Binary security checker"),
            ("annobin", "Annotation plugin"),
            ("gcc-gfortran", "Fortran compiler (not available)"),
            ("gcc-gdc", "D compiler (not available)"),
            ("gcc-gnat", "Ada compiler (not available)"),
            ("gcc-go", "Go compiler (not available)"),
            ("gcc-gm2", "Modula-2 compiler (not available)"),
            ("gcc-objc", "Obj-C compiler (not available)"),
            ("gcc-objc++", "Obj-C++ compiler (not available)"),
            ("gcc-plugin-devel", "GCC plugin development"),
        ],
        "doc_only": [
            ("docbook*", "DocBook toolchain"),
            ("texlive*", "TeX typesetting"),
            ("texinfo*", "GNU documentation system"),
            ("sgml-common", "SGML toolchain"),
            ("xml-common", "XML toolchain"),
            ("xmlto", "XML conversion tool"),
            ("asciidoc*", "Documentation format"),
            ("doxygen", "API documentation generator"),
            ("gtk-doc*", "GTK documentation"),
            ("help2man", "Man page generator from --help"),
            ("openjade", "DSSSL/SGML processor"),
            ("rubygem-asciidoctor*", "AsciiDoc processor"),
            ("python-sphinx*", "Sphinx documentation"),
        ],
        "test_only": [
            ("dejagnu", "DejaGNU test framework"),
            ("expect", "Expect test automation"),
            ("cmocka*", "C mock testing framework"),
            ("valgrind*", "Memory analysis tool"),
            ("check", "C unit test framework"),
            ("cunit", "C unit test framework"),
            ("libcheck*", "C check library"),
            ("gtest", "Google test framework"),
            ("gmock", "Google mock framework"),
            ("cpputest", "C++ unit test framework"),
        ],
        "asset_packages": [
            ("*-fonts", "Font package"),
            ("*-icon-theme", "Icon theme"),
            ("unicode-emoji", "Unicode emoji data"),
            ("unicode-ucd", "Unicode character database"),
            ("natural-earth-map-data", "Map data"),
            ("publicsuffix-list*", "Public suffix list"),
        ],
        "impossible_ecosystems": [
            ("rust-*", "Rust ecosystem"),
            ("golang-*", "Go ecosystem"),
            ("ghc-*", "Haskell ecosystem"),
            ("nodejs*", "Node.js ecosystem"),
            ("java-*", "Java ecosystem"),
            ("maven-*", "Maven build system"),
            ("ant*", "Apache Ant"),
            ("gradle*", "Gradle build system"),
            ("erlang*", "Erlang ecosystem"),
            ("ruby*", "Ruby ecosystem"),
            ("rubygem-*", "Ruby gem"),
            ("R-*", "R statistics ecosystem"),
            ("ocaml*", "OCaml ecosystem"),
            ("php", "PHP runtime"),
            ("php-*", "PHP ecosystem"),
            ("lua-*", "Lua ecosystem"),
            ("mono-*", "Mono/.NET ecosystem"),
            ("dotnet*", ".NET ecosystem"),
        ],
        "desktop_frameworks": [
            ("qt5*", "Qt5 framework"),
            ("qt6*", "Qt6 framework"),
            ("kf5*", "KDE Frameworks 5"),
            ("kf6*", "KDE Frameworks 6"),
            ("gnome-*", "GNOME desktop"),
            ("gtk3*", "GTK3 framework"),
            ("gtk4*", "GTK4 framework"),
            ("wayland*", "Wayland display protocol"),
            ("pipewire*", "Audio/video framework"),
            ("pulseaudio*", "Audio framework"),
            ("mutter*", "GNOME compositor"),
        ],
        "linux_kernel": [
            ("kernel-*", "Linux kernel"),
            ("systemd*", "systemd init system"),
            ("libseccomp*", "Linux syscall filter"),
            ("liburing*", "io_uring library"),
            ("libaio*", "Linux async I/O"),
            ("dbus*", "D-Bus IPC system"),
            ("polkit*", "Authorization framework"),
            ("selinux-*", "SELinux policy"),
            ("libselinux*", "SELinux library"),
            ("libsemanage*", "SELinux management"),
            ("libsepol*", "SELinux policy library"),
            ("audit*", "Linux audit framework"),
        ],
    }

    def __init__(
        self,
        db: sqlite3.Connection,
        rule_loader: RuleLoader,
        rules_dir: Path,
        rpms_dir: Path,
    ):
        self.db = db
        self.rule_loader = rule_loader
        self.rules_dir = rules_dir
        self.rpms_dir = rpms_dir

    def check_package(self, target: str) -> CheckResult:
        """Validate roadmap for a single built package.

        Runs the roadmap WITHOUT roadmap_config.yaml filtering to measure
        the raw predictions, then identifies NEED_RULES as false positives.
        """
        # Construct resolver without roadmap_config filtering.
        # We temporarily rename the config file so it's not loaded.
        resolver = RoadmapResolver(
            db=self.db,
            rule_loader=self.rule_loader,
            rules_dir=self.rules_dir,
            rpms_dir=self.rpms_dir,
        )
        # Clear roadmap drops to get unfiltered predictions
        resolver._roadmap_drops = {}

        result = resolver.resolve(target)
        return self._analyze_result(target, result)

    def check_all_built(self) -> list[CheckResult]:
        """Run roadmap-check for all packages with built RPMs.

        Uses a single RoadmapResolver instance for shared caches.
        """
        # Construct a single resolver (caches accumulate across calls)
        resolver = RoadmapResolver(
            db=self.db,
            rule_loader=self.rule_loader,
            rules_dir=self.rules_dir,
            rpms_dir=self.rpms_dir,
        )
        # Clear roadmap drops to get unfiltered predictions
        resolver._roadmap_drops = {}

        results = []
        for pkg_name in sorted(resolver._built_packages.keys()):
            roadmap = resolver.resolve(pkg_name)
            check = self._analyze_result(pkg_name, roadmap)
            results.append(check)
        return results

    def _analyze_result(self, target: str, roadmap: RoadmapResult) -> CheckResult:
        """Analyze a roadmap result and identify false positives."""
        check = CheckResult(target=target)

        need_rules = [
            pkg
            for pkg, info in roadmap.packages.items()
            if info.classification == Classification.NEED_RULES
        ]
        check.total_need_rules = len(need_rules)

        for pkg_name in need_rules:
            category, confidence, reason = self._categorize(pkg_name)
            fp = FalsePositive(
                name=pkg_name,
                suggested_category=category,
                confidence=confidence,
                reason=reason,
                found_in=[target],
            )
            check.false_positives.append(fp)

        return check

    def _categorize(self, pkg_name: str) -> tuple[str, str, str]:
        """Categorize a false positive into a roadmap_config category.

        Returns (category, confidence, reason).
        """
        for category, patterns in self.CATEGORY_HEURISTICS.items():
            for pattern, reason in patterns:
                if fnmatch(pkg_name, pattern) or pkg_name == pattern:
                    return category, "HIGH", reason
        return "uncategorized", "LOW", "No heuristic match"

    def aggregate_false_positives(
        self, results: list[CheckResult]
    ) -> dict[str, FalsePositive]:
        """Aggregate false positives across all checks.

        Returns dict of pkg_name -> FalsePositive with merged found_in lists.
        """
        aggregated: dict[str, FalsePositive] = {}
        for result in results:
            for fp in result.false_positives:
                if fp.name in aggregated:
                    aggregated[fp.name].found_in.extend(fp.found_in)
                else:
                    aggregated[fp.name] = FalsePositive(
                        name=fp.name,
                        suggested_category=fp.suggested_category,
                        confidence=fp.confidence,
                        reason=fp.reason,
                        found_in=list(fp.found_in),
                    )
        return aggregated

    def generate_suggestions(
        self,
        aggregated: dict[str, FalsePositive],
        min_freq: int = 3,
    ) -> dict[str, list[str]]:
        """Generate suggested additions to roadmap_config.yaml.

        Groups false positives by category and returns package names.
        Only includes packages appearing in at least min_freq roadmaps.
        """
        suggestions: dict[str, list[str]] = defaultdict(list)
        for fp in sorted(
            aggregated.values(), key=lambda f: (-len(f.found_in), f.name)
        ):
            if len(fp.found_in) >= min_freq:
                suggestions[fp.suggested_category].append(fp.name)
        return dict(suggestions)

    def format_check_result(self, check: CheckResult) -> str:
        """Format a single package check result as text."""
        lines = [f"Roadmap check: {check.target}"]
        lines.append(f"  NEED_RULES predicted: {check.total_need_rules}")
        lines.append(f"  False positives: {len(check.false_positives)}")

        if check.false_positives:
            # Group by category
            by_cat: dict[str, list[FalsePositive]] = defaultdict(list)
            for fp in check.false_positives:
                by_cat[fp.suggested_category].append(fp)

            lines.append("")
            for cat, fps in sorted(by_cat.items(), key=lambda x: -len(x[1])):
                names = ", ".join(f.name for f in fps[:5])
                extra = f" ...+{len(fps) - 5}" if len(fps) > 5 else ""
                lines.append(f"  {cat}: {len(fps)} ({names}{extra})")

        return "\n".join(lines)

    def format_aggregate_report(
        self,
        results: list[CheckResult],
        aggregated: dict[str, FalsePositive],
    ) -> str:
        """Format aggregate validation report as text."""
        lines = [
            f"Roadmap Validation Report",
            f"  Packages checked: {len(results)}",
            f"  Unique false positives: {len(aggregated)}",
            "",
        ]

        # Group by category
        by_cat: dict[str, list[FalsePositive]] = defaultdict(list)
        for fp in aggregated.values():
            by_cat[fp.suggested_category].append(fp)

        lines.append("By category:")
        for cat, fps in sorted(by_cat.items(), key=lambda x: -len(x[1])):
            lines.append(f"  {cat}: {len(fps)} unique packages")

        # Top 20 most frequent
        lines.append("")
        lines.append("Most frequent false positives:")
        sorted_fps = sorted(aggregated.values(), key=lambda f: -len(f.found_in))
        for fp in sorted_fps[:20]:
            lines.append(
                f"  {fp.name:<35s} in {len(fp.found_in):>2d}/{len(results)} "
                f"roadmaps  ({fp.suggested_category})"
            )

        return "\n".join(lines)

    def format_suggestions(
        self,
        suggestions: dict[str, list[str]],
        min_freq: int = 3,
    ) -> str:
        """Format suggestions as a YAML snippet for roadmap_config.yaml."""
        lines = [
            f"# Suggested additions to roadmap_config.yaml",
            f"# (packages appearing as false positives in {min_freq}+ builds)",
            "",
        ]
        output = {"roadmap_drops": {}}
        for category, packages in sorted(suggestions.items()):
            output["roadmap_drops"][category] = sorted(packages)
        lines.append(yaml.dump(output, default_flow_style=False, sort_keys=False))
        return "\n".join(lines)

    def format_json_report(
        self,
        results: list[CheckResult],
        aggregated: dict[str, FalsePositive],
    ) -> str:
        """Format full validation report as JSON."""
        by_cat: dict[str, list[str]] = defaultdict(list)
        for fp in aggregated.values():
            by_cat[fp.suggested_category].append(fp.name)

        data = {
            "packages_checked": len(results),
            "unique_false_positives": len(aggregated),
            "by_category": {
                cat: {"count": len(pkgs), "packages": sorted(pkgs)}
                for cat, pkgs in sorted(by_cat.items())
            },
            "per_package": [
                {
                    "target": r.target,
                    "need_rules": r.total_need_rules,
                    "false_positives": len(r.false_positives),
                }
                for r in results
            ],
            "most_frequent": [
                {
                    "name": fp.name,
                    "frequency": len(fp.found_in),
                    "category": fp.suggested_category,
                    "reason": fp.reason,
                }
                for fp in sorted(
                    aggregated.values(), key=lambda f: -len(f.found_in)
                )[:50]
            ],
        }
        return json.dumps(data, indent=2)
