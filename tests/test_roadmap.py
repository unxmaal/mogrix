"""Tests for mogrix roadmap command — dependency graph resolution."""

import json
import sqlite3
import tempfile
from pathlib import Path
from unittest.mock import patch

import pytest
import yaml

from mogrix.repometa import extract_srpm_name
from mogrix.roadmap import (
    Classification,
    RoadmapResolver,
    RoadmapResult,
    format_json,
    format_text,
)
from mogrix.rules.loader import RuleLoader


# --- SRPM name extraction ---


class TestSRPMNameExtraction:
    def test_simple_name(self):
        assert extract_srpm_name("popt-1.19-6.fc40.src.rpm") == "popt"

    def test_hyphenated_name(self):
        assert extract_srpm_name("zlib-ng-2.1.6-2.fc40.src.rpm") == "zlib-ng"

    def test_lib_prefix(self):
        assert extract_srpm_name("libxml2-2.12.5-1.fc40.src.rpm") == "libxml2"

    def test_glib2(self):
        assert extract_srpm_name("glib2-2.80.0-1.fc40.src.rpm") == "glib2"

    def test_complex_version(self):
        assert extract_srpm_name("rpm-4.19.1.1-1.fc40.src.rpm") == "rpm"

    def test_epoch_in_release(self):
        # Some SRPMs have epoch-like release strings
        assert extract_srpm_name("perl-5.38.2-506.fc40.src.rpm") == "perl"

    def test_triple_hyphen_name(self):
        assert extract_srpm_name("xorg-x11-server-21.1.11-1.fc40.src.rpm") == "xorg-x11-server"


# --- Effective drops computation ---


class TestEffectiveDrops:
    @pytest.fixture
    def rules_dir(self, tmp_path):
        """Create a minimal rules directory for testing."""
        # generic.yaml
        generic = {
            "generic": {
                "drop_buildrequires": [
                    "systemd",
                    "systemd-devel",
                    "libselinux-devel",
                ]
            }
        }
        (tmp_path / "generic.yaml").write_text(yaml.dump(generic))

        # classes/nls-disabled.yaml
        classes_dir = tmp_path / "classes"
        classes_dir.mkdir()
        nls_class = {
            "rules": {
                "drop_buildrequires": ["gettext", "intltool"]
            }
        }
        (classes_dir / "nls-disabled.yaml").write_text(yaml.dump(nls_class))

        # packages directory
        packages_dir = tmp_path / "packages"
        packages_dir.mkdir()

        # packages/testpkg.yaml (uses nls-disabled class + has own drops)
        testpkg = {
            "package": "testpkg",
            "classes": ["nls-disabled"],
            "rules": {
                "drop_buildrequires": ["valgrind"]
            },
        }
        (packages_dir / "testpkg.yaml").write_text(yaml.dump(testpkg))

        # packages/simplepkg.yaml (no classes, no drops)
        simplepkg = {
            "package": "simplepkg",
            "rules": {},
        }
        (packages_dir / "simplepkg.yaml").write_text(yaml.dump(simplepkg))

        return tmp_path

    @pytest.fixture
    def mock_db(self):
        """Create an in-memory sqlite database with test data."""
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
        return conn

    def test_generic_drops_applied(self, rules_dir, mock_db):
        loader = RuleLoader(rules_dir)
        resolver = RoadmapResolver(
            db=mock_db,
            rule_loader=loader,
            rules_dir=rules_dir,
            rpms_dir=Path("/nonexistent"),
        )
        drops = resolver._compute_effective_drops("simplepkg")
        assert "systemd" in drops
        assert "systemd-devel" in drops
        assert "libselinux-devel" in drops

    def test_class_drops_applied(self, rules_dir, mock_db):
        loader = RuleLoader(rules_dir)
        resolver = RoadmapResolver(
            db=mock_db,
            rule_loader=loader,
            rules_dir=rules_dir,
            rpms_dir=Path("/nonexistent"),
        )
        drops = resolver._compute_effective_drops("testpkg")
        # Should have generic + class + package drops
        assert "systemd" in drops  # generic
        assert "gettext" in drops  # nls-disabled class
        assert "intltool" in drops  # nls-disabled class
        assert "valgrind" in drops  # package-specific

    def test_unknown_package_gets_generic_only(self, rules_dir, mock_db):
        loader = RuleLoader(rules_dir)
        resolver = RoadmapResolver(
            db=mock_db,
            rule_loader=loader,
            rules_dir=rules_dir,
            rpms_dir=Path("/nonexistent"),
        )
        drops = resolver._compute_effective_drops("unknown-package")
        assert "systemd" in drops
        assert "gettext" not in drops
        assert "valgrind" not in drops


# --- Provider resolution ---


class TestProviderResolution:
    @pytest.fixture
    def setup(self, tmp_path):
        """Set up rules dir, mock db, and resolver."""
        # minimal rules
        generic = {"generic": {"drop_buildrequires": ["systemd"]}}
        (tmp_path / "generic.yaml").write_text(yaml.dump(generic))
        (tmp_path / "classes").mkdir()
        (tmp_path / "packages").mkdir()

        # sysroot provides
        sysroot = {
            "libraries": ["libc.so", "libm.so"],
            "files": ["/bin/sh", "/usr/bin/env"],
            "capabilities": ["gcc", "glibc-devel"],
        }
        (tmp_path / "sysroot_provides.yaml").write_text(yaml.dump(sysroot))

        # non-fedora packages
        non_fedora = {"packages": {"tdnf": {"source": "photon5"}}}
        (tmp_path / "non_fedora_packages.yaml").write_text(yaml.dump(non_fedora))

        # mock db
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

        # Insert test data
        conn.executemany(
            "INSERT INTO binary_provides VALUES (?,?,?,?,?,?)",
            [
                ("zlib-devel", "", "", "zlib-devel", "zlib-ng", "releases"),
                ("libxml2-devel", "", "", "libxml2-devel", "libxml2", "releases"),
                ("expat-devel", "", "", "expat-devel", "expat", "releases"),
                ("gmp-devel", "", "", "gmp-devel", "gmp", "releases"),
            ],
        )
        conn.executemany(
            "INSERT INTO file_provides VALUES (?,?,?,?)",
            [
                ("/usr/bin/perl", "perl-interpreter", "perl", "releases"),
            ],
        )
        conn.executemany(
            "INSERT INTO source_buildrequires VALUES (?,?,?,?,?)",
            [
                ("expat", "gcc", "", "", "releases"),
                ("expat", "zlib-devel", "", "", "releases"),
            ],
        )
        conn.commit()

        loader = RuleLoader(tmp_path)
        resolver = RoadmapResolver(
            db=conn,
            rule_loader=loader,
            rules_dir=tmp_path,
            rpms_dir=Path("/nonexistent"),
        )
        return resolver

    def test_dropped_dep(self, setup):
        resolver = setup
        drops = {"systemd"}
        pkg, cls, _ = resolver._resolve_provider("systemd", drops, "test")
        assert cls == Classification.DROPPED

    def test_sysroot_capability(self, setup):
        resolver = setup
        pkg, cls, _ = resolver._resolve_provider("gcc", set(), "test")
        assert cls == Classification.SYSROOT

    def test_sysroot_file(self, setup):
        resolver = setup
        pkg, cls, _ = resolver._resolve_provider("/bin/sh", set(), "test")
        assert cls == Classification.SYSROOT

    def test_sysroot_library(self, setup):
        resolver = setup
        pkg, cls, _ = resolver._resolve_provider("libc.so", set(), "test")
        assert cls == Classification.SYSROOT

    def test_binary_provides(self, setup):
        resolver = setup
        pkg, cls, _ = resolver._resolve_provider("zlib-devel", set(), "test")
        assert pkg == "zlib-ng"
        assert cls == Classification.NEED_RULES

    def test_file_provides(self, setup):
        resolver = setup
        pkg, cls, _ = resolver._resolve_provider("/usr/bin/perl", set(), "test")
        assert pkg == "perl"

    def test_unresolvable(self, setup):
        resolver = setup
        pkg, cls, _ = resolver._resolve_provider("nonexistent-package", set(), "test")
        assert cls == Classification.UNRESOLVABLE

    def test_linux_only_pattern(self, setup):
        resolver = setup
        pkg, cls, _ = resolver._resolve_provider("kernel-headers", set(), "test")
        assert cls == Classification.DROPPED


# --- Topological sort ---


class TestTopologicalSort:
    @pytest.fixture
    def resolver(self, tmp_path):
        generic = {"generic": {"drop_buildrequires": []}}
        (tmp_path / "generic.yaml").write_text(yaml.dump(generic))
        (tmp_path / "classes").mkdir()
        (tmp_path / "packages").mkdir()
        (tmp_path / "sysroot_provides.yaml").write_text(
            yaml.dump({"libraries": [], "files": [], "capabilities": []})
        )
        (tmp_path / "non_fedora_packages.yaml").write_text(
            yaml.dump({"packages": {}})
        )

        conn = sqlite3.connect(":memory:")
        conn.row_factory = sqlite3.Row
        conn.executescript("""
            CREATE TABLE binary_provides (provides_name TEXT, provides_flags TEXT,
                provides_version TEXT, binary_package TEXT, source_package TEXT, repo TEXT);
            CREATE TABLE source_buildrequires (source_package TEXT, requires_name TEXT,
                requires_flags TEXT, requires_version TEXT, repo TEXT);
            CREATE TABLE file_provides (file_path TEXT, binary_package TEXT,
                source_package TEXT, repo TEXT);
            CREATE TABLE meta (key TEXT PRIMARY KEY, value TEXT);
            CREATE INDEX idx_bp_name ON binary_provides(provides_name);
            CREATE INDEX idx_sbr_pkg ON source_buildrequires(source_package);
            CREATE INDEX idx_fp_path ON file_provides(file_path);
        """)

        loader = RuleLoader(tmp_path)
        return RoadmapResolver(
            db=conn,
            rule_loader=loader,
            rules_dir=tmp_path,
            rpms_dir=Path("/nonexistent"),
        )

    def test_simple_dag(self, resolver):
        edges = [("a", "b"), ("a", "c"), ("b", "c")]
        order, cycles = resolver._topological_sort(edges, {"a", "b", "c"})
        assert order.index("a") < order.index("b")
        assert order.index("a") < order.index("c")
        assert order.index("b") < order.index("c")
        assert cycles == []

    def test_no_edges(self, resolver):
        order, cycles = resolver._topological_sort([], {"a", "b", "c"})
        assert set(order) == {"a", "b", "c"}
        assert cycles == []

    def test_cycle_detected(self, resolver):
        edges = [("a", "b"), ("b", "a")]
        order, cycles = resolver._topological_sort(edges, {"a", "b"})
        # Cycle means both nodes remain
        assert len(cycles) == 1
        assert set(cycles[0]) == {"a", "b"}


# --- Complexity scoring ---


class TestComplexityScoring:
    @pytest.fixture
    def resolver(self, tmp_path):
        generic = {"generic": {"drop_buildrequires": []}}
        (tmp_path / "generic.yaml").write_text(yaml.dump(generic))
        (tmp_path / "classes").mkdir()
        (tmp_path / "packages").mkdir()
        (tmp_path / "sysroot_provides.yaml").write_text(
            yaml.dump({"libraries": [], "files": [], "capabilities": []})
        )
        (tmp_path / "non_fedora_packages.yaml").write_text(
            yaml.dump({"packages": {}})
        )

        conn = sqlite3.connect(":memory:")
        conn.row_factory = sqlite3.Row
        conn.executescript("""
            CREATE TABLE binary_provides (provides_name TEXT, provides_flags TEXT,
                provides_version TEXT, binary_package TEXT, source_package TEXT, repo TEXT);
            CREATE TABLE source_buildrequires (source_package TEXT, requires_name TEXT,
                requires_flags TEXT, requires_version TEXT, repo TEXT);
            CREATE TABLE file_provides (file_path TEXT, binary_package TEXT,
                source_package TEXT, repo TEXT);
            CREATE TABLE meta (key TEXT PRIMARY KEY, value TEXT);
        """)

        loader = RuleLoader(tmp_path)
        return RoadmapResolver(
            db=conn,
            rule_loader=loader,
            rules_dir=tmp_path,
            rpms_dir=Path("/nonexistent"),
        )

    def test_low_complexity(self, resolver):
        br = ["gcc", "autoconf", "automake"]
        assert resolver._estimate_complexity("test", br) == "LOW"

    def test_medium_complexity(self, resolver):
        br = ["gcc-c++", "autoconf", "automake"]
        # gcc-c++ = +2, has autoconf = no penalty -> 2 = MED
        assert resolver._estimate_complexity("test", br) == "MED"

    def test_high_complexity(self, resolver):
        br = ["gcc-c++", "gobject-introspection-devel"] + [f"dep{i}" for i in range(25)]
        assert resolver._estimate_complexity("test", br) == "HIGH"


# --- Output formatting ---


class TestOutputFormatting:
    def test_format_json_roundtrip(self):
        result = RoadmapResult(target="test-pkg")
        result.packages["dep1"] = PackageInfo(
            name="dep1",
            classification=Classification.NEED_RULES,
            complexity="LOW",
            build_order=1,
        )
        result.build_order = ["dep1", "test-pkg"]

        json_str = format_json(result)
        data = json.loads(json_str)
        assert data["target"] == "test-pkg"
        assert "dep1" in data["packages"]
        assert data["packages"]["dep1"]["status"] == "need_rules"

    def test_format_text_includes_summary(self):
        result = RoadmapResult(target="test-pkg")
        result.packages["dep1"] = PackageInfo(
            name="dep1",
            classification=Classification.NEED_RULES,
            complexity="LOW",
            build_order=1,
        )
        result.build_order = ["dep1"]

        text = format_text(result)
        assert "NEED NEW RULES:" in text
        assert "dep1" in text
        assert "SUMMARY:" in text


# Import for the test above
from mogrix.roadmap import PackageInfo
