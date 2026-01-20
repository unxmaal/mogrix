"""Tests for YAML rule loader."""

from pathlib import Path
from mogrix.rules.loader import RuleLoader

RULES_DIR = Path(__file__).parent.parent / "rules"


def test_load_generic_rules():
    """Loader can load generic.yaml."""
    loader = RuleLoader(RULES_DIR)
    rules = loader.load_generic()
    assert rules is not None
    assert "generic" in rules


def test_generic_drop_buildrequires():
    """Generic rules contain drop_buildrequires list."""
    loader = RuleLoader(RULES_DIR)
    rules = loader.load_generic()
    drops = rules["generic"]["drop_buildrequires"]
    assert "systemd" in drops
    assert "libselinux" in drops


def test_generic_add_buildrequires():
    """Generic rules contain add_buildrequires list."""
    loader = RuleLoader(RULES_DIR)
    rules = loader.load_generic()
    adds = rules["generic"]["add_buildrequires"]
    assert "sgug-rpm-config" in adds


def test_generic_configure_disable():
    """Generic rules contain configure_disable list."""
    loader = RuleLoader(RULES_DIR)
    rules = loader.load_generic()
    disables = rules["generic"]["configure_disable"]
    assert "selinux" in disables
    assert "systemd" in disables


def test_generic_rewrite_paths():
    """Generic rules contain path rewrites."""
    loader = RuleLoader(RULES_DIR)
    rules = loader.load_generic()
    paths = rules["generic"]["rewrite_paths"]
    assert paths["/usr/lib64"] == "/usr/sgug/lib32"


def test_generic_header_overlays():
    """Generic rules specify header overlays."""
    loader = RuleLoader(RULES_DIR)
    rules = loader.load_generic()
    overlays = rules["generic"]["header_overlays"]
    assert "generic" in overlays
