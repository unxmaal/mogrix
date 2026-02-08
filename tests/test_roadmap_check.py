"""Tests for mogrix roadmap-check — validation of roadmap predictions."""

import json
import sqlite3
from pathlib import Path

import pytest
import yaml

from mogrix.roadmap import Classification, RoadmapResolver
from mogrix.roadmap_check import (
    CheckResult,
    FalsePositive,
    RoadmapChecker,
)
from mogrix.rules.loader import RuleLoader


@pytest.fixture
def checker_setup(tmp_path):
    """Set up a RoadmapChecker with mock data."""
    # Rules dir
    generic = {"generic": {"drop_buildrequires": ["systemd"]}}
    (tmp_path / "generic.yaml").write_text(yaml.dump(generic))
    (tmp_path / "classes").mkdir()
    packages_dir = tmp_path / "packages"
    packages_dir.mkdir()

    # sysroot
    sysroot = {
        "libraries": ["libc.so"],
        "files": ["/bin/sh"],
        "capabilities": ["gcc"],
    }
    (tmp_path / "sysroot_provides.yaml").write_text(yaml.dump(sysroot))
    (tmp_path / "non_fedora_packages.yaml").write_text(
        yaml.dump({"packages": {}})
    )

    # No roadmap_config.yaml (checker bypasses it anyway)

    conn = sqlite3.connect(":memory:")
    conn.row_factory = sqlite3.Row
    conn.executescript("""
        CREATE TABLE binary_provides (
            provides_name TEXT, provides_flags TEXT,
            provides_version TEXT, binary_package TEXT,
            source_package TEXT, repo TEXT
        );
        CREATE TABLE source_buildrequires (
            source_package TEXT, requires_name TEXT,
            requires_flags TEXT, requires_version TEXT,
            repo TEXT
        );
        CREATE TABLE file_provides (
            file_path TEXT, binary_package TEXT,
            source_package TEXT, repo TEXT
        );
        CREATE TABLE meta (key TEXT PRIMARY KEY, value TEXT);
        CREATE INDEX idx_bp_name ON binary_provides(provides_name);
        CREATE INDEX idx_sbr_pkg ON source_buildrequires(source_package);
        CREATE INDEX idx_fp_path ON file_provides(file_path);
    """)

    # Insert test data:
    # "mypkg" is a built package that depends on fonts-rpm-macros and real-lib
    conn.executemany(
        "INSERT INTO binary_provides VALUES (?,?,?,?,?,?)",
        [
            ("mypkg", "", "", "mypkg", "mypkg", "releases"),
            ("fonts-rpm-macros", "", "", "fonts-rpm-macros", "fonts-rpm-macros", "releases"),
            ("real-lib", "", "", "real-lib", "real-lib", "releases"),
            ("dejagnu", "", "", "dejagnu", "dejagnu", "releases"),
        ],
    )
    conn.executemany(
        "INSERT INTO source_buildrequires VALUES (?,?,?,?,?)",
        [
            ("mypkg", "gcc", "", "", "releases"),
            ("mypkg", "fonts-rpm-macros", "", "", "releases"),
            ("mypkg", "real-lib", "", "", "releases"),
            ("mypkg", "dejagnu", "", "", "releases"),
        ],
    )
    conn.commit()

    # Create a fake RPM to make mypkg appear as "built"
    rpms_dir = tmp_path / "rpms"
    rpms_dir.mkdir()
    (rpms_dir / "mypkg-1.0-1.mips.rpm").touch()

    loader = RuleLoader(tmp_path)
    checker = RoadmapChecker(
        db=conn,
        rule_loader=loader,
        rules_dir=tmp_path,
        rpms_dir=rpms_dir,
    )
    return checker


class TestRoadmapChecker:
    def test_check_built_package(self, checker_setup):
        """Built package's NEED_RULES deps are identified as false positives."""
        checker = checker_setup
        result = checker.check_package("mypkg")
        assert result.target == "mypkg"
        # fonts-rpm-macros, real-lib, dejagnu are NEED_RULES (not built)
        assert result.total_need_rules > 0
        fp_names = [fp.name for fp in result.false_positives]
        assert "fonts-rpm-macros" in fp_names
        assert "dejagnu" in fp_names

    def test_categorization_high_confidence(self, checker_setup):
        """Known patterns get HIGH confidence categorization."""
        checker = checker_setup
        cat, conf, reason = checker._categorize("fonts-rpm-macros")
        assert cat == "build_infra"
        assert conf == "HIGH"
        assert "RPM macro" in reason

    def test_categorization_doc_only(self, checker_setup):
        """Documentation packages are categorized as doc_only."""
        checker = checker_setup
        cat, conf, reason = checker._categorize("docbook-dtds")
        assert cat == "doc_only"
        assert conf == "HIGH"

    def test_categorization_unknown(self, checker_setup):
        """Unknown packages get LOW confidence uncategorized."""
        checker = checker_setup
        cat, conf, reason = checker._categorize("some-unknown-package")
        assert cat == "uncategorized"
        assert conf == "LOW"

    def test_categorization_font(self, checker_setup):
        """Font packages match *-fonts glob."""
        checker = checker_setup
        cat, conf, reason = checker._categorize("adobe-source-code-pro-fonts")
        assert cat == "asset_packages"
        assert conf == "HIGH"

    def test_categorization_test_only(self, checker_setup):
        """Test frameworks are categorized as test_only."""
        checker = checker_setup
        cat, conf, reason = checker._categorize("dejagnu")
        assert cat == "test_only"
        assert conf == "HIGH"


class TestAggregation:
    def test_aggregate_deduplication(self):
        """Same false positive from multiple targets is merged."""
        results = [
            CheckResult(
                target="pkg-a",
                total_need_rules=1,
                false_positives=[
                    FalsePositive(
                        name="fonts-rpm-macros",
                        suggested_category="build_infra",
                        confidence="HIGH",
                        reason="RPM macro package",
                        found_in=["pkg-a"],
                    )
                ],
            ),
            CheckResult(
                target="pkg-b",
                total_need_rules=1,
                false_positives=[
                    FalsePositive(
                        name="fonts-rpm-macros",
                        suggested_category="build_infra",
                        confidence="HIGH",
                        reason="RPM macro package",
                        found_in=["pkg-b"],
                    )
                ],
            ),
        ]
        checker = RoadmapChecker.__new__(RoadmapChecker)
        aggregated = checker.aggregate_false_positives(results)
        assert "fonts-rpm-macros" in aggregated
        assert set(aggregated["fonts-rpm-macros"].found_in) == {"pkg-a", "pkg-b"}

    def test_generate_suggestions_min_freq(self):
        """Suggestions respect min_freq threshold."""
        results = [
            CheckResult(
                target=f"pkg-{i}",
                total_need_rules=2,
                false_positives=[
                    FalsePositive(
                        name="fonts-rpm-macros",
                        suggested_category="build_infra",
                        confidence="HIGH",
                        reason="test",
                        found_in=[f"pkg-{i}"],
                    ),
                    FalsePositive(
                        name="rare-package",
                        suggested_category="uncategorized",
                        confidence="LOW",
                        reason="test",
                        found_in=[f"pkg-{i}"],
                    ),
                ],
            )
            for i in range(5)
        ]
        checker = RoadmapChecker.__new__(RoadmapChecker)
        aggregated = checker.aggregate_false_positives(results)
        suggestions = checker.generate_suggestions(aggregated, min_freq=3)
        # fonts-rpm-macros appears in 5 results (>= 3)
        assert "build_infra" in suggestions
        assert "fonts-rpm-macros" in suggestions["build_infra"]
        # rare-package appears in 5 results but IS included (freq 5 >= 3)
        # Let's test with min_freq=6 to exclude
        suggestions_strict = checker.generate_suggestions(aggregated, min_freq=6)
        assert not suggestions_strict  # nothing meets threshold


class TestFormatting:
    def test_format_check_result(self, checker_setup):
        """format_check_result produces readable output."""
        checker = checker_setup
        result = checker.check_package("mypkg")
        text = checker.format_check_result(result)
        assert "mypkg" in text
        assert "NEED_RULES" in text

    def test_format_json_report(self):
        """JSON output is valid and contains expected fields."""
        results = [
            CheckResult(
                target="test-pkg",
                total_need_rules=1,
                false_positives=[
                    FalsePositive(
                        name="fonts-rpm-macros",
                        suggested_category="build_infra",
                        confidence="HIGH",
                        reason="RPM macro package",
                        found_in=["test-pkg"],
                    )
                ],
            ),
        ]
        checker = RoadmapChecker.__new__(RoadmapChecker)
        aggregated = checker.aggregate_false_positives(results)
        json_str = checker.format_json_report(results, aggregated)
        data = json.loads(json_str)
        assert data["packages_checked"] == 1
        assert data["unique_false_positives"] == 1
        assert "build_infra" in data["by_category"]
