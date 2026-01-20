"""Tests for conditional handling in spec files."""

import pytest
from mogrix.emitter.spec import SpecWriter
from mogrix.rules.engine import TransformResult
from mogrix.parser.spec import SpecFile


@pytest.fixture
def spec_with_conditionals():
    """Create a spec file with conditional blocks."""
    content = """Name: testpkg
Version: 1.0
Release: 1
Summary: Test package

%if 0%{?fedora}
BuildRequires: fedora-specific
%endif

%if 0%{?with_systemd}
BuildRequires: systemd-devel
Requires: systemd
%endif

%build
%configure

%if 0%{?with_tests}
%check
make check
%endif
"""
    return SpecFile(
        name="testpkg",
        version="1.0",
        release="1",
        summary="Test package",
        license="MIT",
        url="",
        buildrequires=[],
        requires=[],
        sources={},
        patches={},
        raw_content=content,
    )


def test_comment_out_conditional(spec_with_conditionals):
    """Test commenting out a conditional block."""
    result = TransformResult(spec=spec_with_conditionals)
    result.comment_conditionals = ["with_systemd"]

    writer = SpecWriter()
    content = writer.write(result)

    # The systemd conditional should be commented
    assert "#%if 0%{?with_systemd}" in content or "# %if 0%{?with_systemd}" in content
    # Fedora conditional should remain
    assert "%if 0%{?fedora}" in content


def test_remove_conditional_block(spec_with_conditionals):
    """Test removing a conditional block entirely."""
    result = TransformResult(spec=spec_with_conditionals)
    result.remove_conditionals = ["with_tests"]

    writer = SpecWriter()
    content = writer.write(result)

    # The tests conditional should be removed
    assert "with_tests" not in content
    assert "make check" not in content


def test_force_conditional_true(spec_with_conditionals):
    """Test forcing a conditional to always be true."""
    result = TransformResult(spec=spec_with_conditionals)
    result.force_conditionals = {"fedora": True}

    writer = SpecWriter()
    content = writer.write(result)

    # The conditional content should remain but %if/%endif removed
    assert "BuildRequires: fedora-specific" in content
    assert "%if 0%{?fedora}" not in content
