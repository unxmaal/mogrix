"""Tests for rule application engine."""

from pathlib import Path
from mogrix.parser.spec import SpecParser, SpecFile
from mogrix.rules.loader import RuleLoader
from mogrix.rules.engine import RuleEngine

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


def test_engine_adds_buildrequires():
    """Engine adds BuildRequires listed in add_buildrequires."""
    spec = SpecFile(
        name="test",
        version="1.0",
        buildrequires=["gcc"],
    )
    loader = RuleLoader(RULES_DIR)
    engine = RuleEngine(loader)

    result = engine.apply(spec)

    assert "sgug-rpm-config" in result.spec.buildrequires
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
