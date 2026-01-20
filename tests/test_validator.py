"""Tests for rule validation."""

import tempfile
from pathlib import Path

import pytest

from mogrix.rules.validator import RuleValidator, ValidationResult


@pytest.fixture
def temp_rules_dir():
    """Create a temp directory with rule files."""
    with tempfile.TemporaryDirectory() as tmpdir:
        rules_path = Path(tmpdir)
        packages_path = rules_path / "packages"
        packages_path.mkdir()

        # Create generic.yaml
        (rules_path / "generic.yaml").write_text("""
generic:
  drop_buildrequires:
    - systemd
""")

        yield rules_path


@pytest.fixture
def temp_compat_dir():
    """Create a temp compat directory with catalog."""
    with tempfile.TemporaryDirectory() as tmpdir:
        compat_path = Path(tmpdir)

        (compat_path / "catalog.yaml").write_text("""
functions:
  strdup:
    file: string/strdup.c
  strndup:
    file: string/strndup.c
  getline:
    file: stdio/getline.c
  asprintf:
    file: stdio/asprintf.c
  vasprintf:
    file: stdio/asprintf.c
""")

        yield compat_path


def test_validator_empty_rules_dir():
    """Test validation with empty rules directory."""
    with tempfile.TemporaryDirectory() as tmpdir:
        validator = RuleValidator(Path(tmpdir))
        result = validator.validate_all()

        assert result.files_checked == 0
        assert len(result.warnings) > 0  # Missing generic.yaml


def test_validator_valid_package_rule(temp_rules_dir, temp_compat_dir):
    """Test validation of a valid package rule."""
    packages_path = temp_rules_dir / "packages"
    (packages_path / "test-pkg.yaml").write_text("""
package: test-pkg

rules:
  inject_compat_functions:
    - strdup
    - strndup
  drop_buildrequires:
    - systemd
""")

    validator = RuleValidator(temp_rules_dir, temp_compat_dir)
    result = validator.validate_all()

    assert result.is_valid
    assert result.packages_checked == 1


def test_validator_missing_package_key(temp_rules_dir, temp_compat_dir):
    """Test validation catches missing package key."""
    packages_path = temp_rules_dir / "packages"
    (packages_path / "bad-pkg.yaml").write_text("""
rules:
  drop_buildrequires:
    - systemd
""")

    validator = RuleValidator(temp_rules_dir, temp_compat_dir)
    result = validator.validate_all()

    assert not result.is_valid
    assert any("Missing required 'package' key" in e.message for e in result.errors)


def test_validator_invalid_compat_function(temp_rules_dir, temp_compat_dir):
    """Test validation catches invalid compat function names."""
    packages_path = temp_rules_dir / "packages"
    (packages_path / "test-pkg.yaml").write_text("""
package: test-pkg

rules:
  inject_compat_functions:
    - strdup
    - nonexistent_function
""")

    validator = RuleValidator(temp_rules_dir, temp_compat_dir)
    result = validator.validate_all()

    assert not result.is_valid
    assert any("Unknown compat function" in e.message for e in result.errors)


def test_validator_wrong_type_for_list(temp_rules_dir, temp_compat_dir):
    """Test validation catches wrong type for list field."""
    packages_path = temp_rules_dir / "packages"
    (packages_path / "test-pkg.yaml").write_text("""
package: test-pkg

rules:
  drop_buildrequires: not-a-list
""")

    validator = RuleValidator(temp_rules_dir, temp_compat_dir)
    result = validator.validate_all()

    assert not result.is_valid
    assert any("must be a list" in e.message for e in result.errors)


def test_validator_wrong_type_for_dict(temp_rules_dir, temp_compat_dir):
    """Test validation catches wrong type for dict field."""
    packages_path = temp_rules_dir / "packages"
    (packages_path / "test-pkg.yaml").write_text("""
package: test-pkg

rules:
  ac_cv_overrides:
    - not-a-dict
""")

    validator = RuleValidator(temp_rules_dir, temp_compat_dir)
    result = validator.validate_all()

    assert not result.is_valid
    assert any("must be a dictionary" in e.message for e in result.errors)


def test_validator_unknown_rule_key(temp_rules_dir, temp_compat_dir):
    """Test validation warns about unknown rule keys."""
    packages_path = temp_rules_dir / "packages"
    (packages_path / "test-pkg.yaml").write_text("""
package: test-pkg

rules:
  unknown_key: value
""")

    validator = RuleValidator(temp_rules_dir, temp_compat_dir)
    result = validator.validate_all()

    assert result.is_valid  # Unknown keys are warnings, not errors
    assert any("Unknown rule key" in w.message for w in result.warnings)


def test_validator_package_name_mismatch(temp_rules_dir, temp_compat_dir):
    """Test validation warns when package name doesn't match filename."""
    packages_path = temp_rules_dir / "packages"
    (packages_path / "test-pkg.yaml").write_text("""
package: different-name

rules:
  drop_buildrequires:
    - systemd
""")

    validator = RuleValidator(temp_rules_dir, temp_compat_dir)
    result = validator.validate_all()

    assert result.is_valid  # Mismatch is a warning, not error
    assert any("doesn't match filename" in w.message for w in result.warnings)


def test_validator_yaml_parse_error(temp_rules_dir, temp_compat_dir):
    """Test validation catches YAML parse errors."""
    packages_path = temp_rules_dir / "packages"
    (packages_path / "bad-yaml.yaml").write_text("""
package: bad-yaml
rules:
  - this: is
    invalid: yaml: syntax
""")

    validator = RuleValidator(temp_rules_dir, temp_compat_dir)
    result = validator.validate_all()

    assert not result.is_valid
    assert any("YAML parse error" in e.message for e in result.errors)


def test_validator_generic_yaml(temp_rules_dir):
    """Test validation of generic.yaml."""
    # Overwrite with invalid structure
    (temp_rules_dir / "generic.yaml").write_text("""
not_generic:
  key: value
""")

    validator = RuleValidator(temp_rules_dir)
    result = validator.validate_all()

    assert not result.is_valid
    assert any("must have a 'generic' key" in e.message for e in result.errors)


def test_validator_detects_drop_add_conflict(temp_rules_dir, temp_compat_dir):
    """Test that validator detects drop/add conflicts for same package."""
    packages_path = temp_rules_dir / "packages"
    (packages_path / "test-pkg.yaml").write_text("""
package: test-pkg

rules:
  drop_buildrequires:
    - systemd
    - openssl
  add_buildrequires:
    - systemd
""")

    validator = RuleValidator(temp_rules_dir, temp_compat_dir)
    result = validator.validate_all()

    # Conflicts are warnings, not errors
    assert result.is_valid
    assert any("Conflicting BuildRequires" in w.message for w in result.warnings)


def test_validator_detects_configure_conflict(temp_rules_dir, temp_compat_dir):
    """Test that validator detects configure disable/enable conflicts."""
    packages_path = temp_rules_dir / "packages"
    (packages_path / "test-pkg.yaml").write_text("""
package: test-pkg

rules:
  configure_disable:
    - systemd
    - selinux
  configure_enable:
    - systemd
""")

    validator = RuleValidator(temp_rules_dir, temp_compat_dir)
    result = validator.validate_all()

    assert result.is_valid
    assert any("Conflicting configure flags" in w.message for w in result.warnings)


def test_validator_detects_configure_flags_conflict(temp_rules_dir, temp_compat_dir):
    """Test that validator detects configure_flags add/remove conflicts."""
    packages_path = temp_rules_dir / "packages"
    (packages_path / "test-pkg.yaml").write_text("""
package: test-pkg

rules:
  configure_flags:
    add:
      - --with-ssl
      - --enable-threads
    remove:
      - --with-ssl
""")

    validator = RuleValidator(temp_rules_dir, temp_compat_dir)
    result = validator.validate_all()

    assert result.is_valid
    assert any("Conflicting configure_flags" in w.message for w in result.warnings)
