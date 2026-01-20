"""Tests for rule application engine."""

from pathlib import Path

from mogrix.parser.spec import SpecFile
from mogrix.rules.engine import RuleEngine
from mogrix.rules.loader import RuleLoader

FIXTURES = Path(__file__).parent / "fixtures"
RULES_DIR = Path(__file__).parent.parent / "rules"


def test_engine_drops_buildrequires():
    """Engine removes BuildRequires listed in drop_buildrequires."""
    spec = SpecFile(
        name="test",
        version="1.0",
        buildrequires=["gcc", "systemd", "libselinux", "zlib"],
    )
    loader = RuleLoader(RULES_DIR)
    engine = RuleEngine(loader)

    result = engine.apply(spec)

    assert "systemd" not in result.spec.buildrequires
    assert "libselinux" not in result.spec.buildrequires
    assert "gcc" in result.spec.buildrequires
    assert "zlib" in result.spec.buildrequires


def test_engine_collects_rpm_macros():
    """Engine collects RPM macros from generic rules."""
    spec = SpecFile(
        name="test",
        version="1.0",
        buildrequires=["gcc"],
    )
    loader = RuleLoader(RULES_DIR)
    engine = RuleEngine(loader)

    result = engine.apply(spec)

    assert "_prefix" in result.rpm_macros
    assert result.rpm_macros["_prefix"] == "/usr/sgug"
    assert "_libdir" in result.rpm_macros
    assert "gcc" in result.spec.buildrequires


def test_engine_tracks_applied_rules():
    """Engine tracks which rules were applied."""
    spec = SpecFile(
        name="test",
        version="1.0",
        buildrequires=["systemd"],
    )
    loader = RuleLoader(RULES_DIR)
    engine = RuleEngine(loader)

    result = engine.apply(spec)

    assert len(result.applied_rules) > 0
    assert any("drop_buildrequires" in r for r in result.applied_rules)


def test_engine_collects_configure_disables():
    """Engine collects configure --disable flags."""
    spec = SpecFile(name="test", version="1.0")
    loader = RuleLoader(RULES_DIR)
    engine = RuleEngine(loader)

    result = engine.apply(spec)

    assert "selinux" in result.configure_disable
    assert "systemd" in result.configure_disable


def test_engine_collects_header_overlays():
    """Engine collects header overlay paths."""
    spec = SpecFile(name="test", version="1.0")
    loader = RuleLoader(RULES_DIR)
    engine = RuleEngine(loader)

    result = engine.apply(spec)

    assert "generic" in result.header_overlays


def test_engine_applies_package_rules():
    """Engine applies package-specific rules."""
    # Create a temporary package rule file
    import os
    import tempfile

    with tempfile.TemporaryDirectory() as tmpdir:
        # Copy generic rules
        import shutil

        shutil.copy(RULES_DIR / "generic.yaml", tmpdir)

        # Create packages directory and rule
        os.makedirs(os.path.join(tmpdir, "packages"))
        with open(os.path.join(tmpdir, "packages", "testpkg.yaml"), "w") as f:
            f.write("""
package: testpkg
rules:
  inject_compat_functions:
    - strdup
    - getline
  ac_cv_overrides:
    ac_cv_func_malloc_0_nonnull: "yes"
""")

        spec = SpecFile(name="testpkg", version="1.0")
        loader = RuleLoader(Path(tmpdir))
        engine = RuleEngine(loader)

        result = engine.apply(spec)

        assert "strdup" in result.compat_functions
        assert "getline" in result.compat_functions
        assert "ac_cv_func_malloc_0_nonnull" in result.ac_cv_overrides
