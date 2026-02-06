"""Tests for spec file validation."""

import pytest

from mogrix.validators.spec import SpecValidator


VALID_SPEC = """\
Name: testpkg
Version: 1.0
Release: 1%{?dist}
Summary: A test package
License: MIT

Source0: testpkg-1.0.tar.gz

%description
A test package.

%prep
%autosetup

%build
%configure
%make_build

%install
%make_install

%files
%{_bindir}/testpkg

%changelog
* Mon Jan 01 2024 Test User <test@example.com> - 1.0-1
- Initial package
"""


@pytest.fixture
def validator():
    return SpecValidator()


def test_valid_spec_passes(validator):
    """A well-formed spec should pass validation."""
    result = validator.validate(VALID_SPEC)
    assert result.is_valid
    assert len(result.errors) == 0


def test_missing_name_tag(validator):
    """Spec missing Name tag should produce an error."""
    spec = VALID_SPEC.replace("Name: testpkg\n", "")
    result = validator.validate(spec)
    assert any("Name" in e.message for e in result.errors)


def test_missing_version_tag(validator):
    """Spec missing Version tag should produce an error."""
    spec = VALID_SPEC.replace("Version: 1.0\n", "")
    result = validator.validate(spec)
    assert any("Version" in e.message for e in result.errors)


def test_missing_files_section(validator):
    """Spec missing %files should produce a warning."""
    spec = VALID_SPEC.replace("%files\n%{_bindir}/testpkg\n", "")
    result = validator.validate(spec)
    assert any("%files" in w.message for w in result.warnings)


def test_unbalanced_if_endif(validator):
    """Spec with %if but no %endif should produce an error."""
    spec = VALID_SPEC + "\n%if 0%{?rhel}\nSomething\n"
    result = validator.validate(spec)
    assert any("unclosed" in e.message for e in result.errors)


def test_extra_endif(validator):
    """Spec with extra %endif should produce an error."""
    spec = VALID_SPEC + "\n%endif\n"
    result = validator.validate(spec)
    assert any("unmatched %endif" in e.message for e in result.errors)


def test_balanced_conditionals(validator):
    """Spec with balanced %if/%endif should pass."""
    spec = VALID_SPEC.replace(
        "%description\n",
        "%if 0%{?fedora}\n%description\n%endif\n%description\n",
    )
    result = validator.validate(spec)
    assert not any(
        "unclosed" in e.message or "unmatched" in e.message
        for e in result.errors
    )


def test_commented_lines_ignored(validator):
    """Commented-out tags should not count as present."""
    spec = VALID_SPEC.replace("Name: testpkg", "#Name: testpkg")
    result = validator.validate(spec)
    assert any("Name" in e.message for e in result.errors)


def test_validation_result_properties(validator):
    """Test SpecValidationResult property accessors."""
    result = validator.validate(VALID_SPEC)
    # errors and warnings are lists
    assert isinstance(result.errors, list)
    assert isinstance(result.warnings, list)
    # is_valid is True when no errors
    assert result.is_valid is True
