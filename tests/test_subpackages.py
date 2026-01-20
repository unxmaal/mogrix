"""Tests for subpackage management."""

import pytest
from mogrix.emitter.spec import SpecWriter
from mogrix.rules.engine import TransformResult
from mogrix.parser.spec import SpecFile


@pytest.fixture
def spec_with_subpackages():
    """Create a spec file with subpackages."""
    content = """Name: testpkg
Version: 1.0
Release: 1
Summary: Test package

%package devel
Summary: Development files
Requires: %{name}%{?_isa} = %{version}-%{release}

%description devel
Development files for testpkg.

%package debuginfo
Summary: Debug information
AutoReqProv: no

%description debuginfo
Debug information for testpkg.

%package langpack-en
Summary: English language pack

%description langpack-en
English language pack.

%build
%configure
make

%install
make install DESTDIR=%{buildroot}

%files
%{_libdir}/lib*.so.*

%files devel
%{_includedir}/*
%{_libdir}/lib*.so

%files debuginfo
%{_prefix}/lib/debug/*

%files langpack-en
%{_datadir}/locale/en/*
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


def test_drop_subpackage(spec_with_subpackages):
    """Test dropping a subpackage."""
    result = TransformResult(spec=spec_with_subpackages)
    result.drop_subpackages = ["debuginfo"]

    writer = SpecWriter()
    content = writer.write(result)

    # debuginfo subpackage should be commented out
    assert "#%package debuginfo" in content or "# %package debuginfo" in content
    assert "%package devel" in content  # devel should remain


def test_drop_subpackage_wildcard(spec_with_subpackages):
    """Test dropping subpackages with wildcard pattern."""
    result = TransformResult(spec=spec_with_subpackages)
    result.drop_subpackages = ["langpack-*"]

    writer = SpecWriter()
    content = writer.write(result)

    # langpack subpackages should be commented
    assert "#%package langpack-en" in content or "# %package langpack-en" in content
    assert "%package devel" in content  # devel should remain


def test_drop_multiple_subpackages(spec_with_subpackages):
    """Test dropping multiple subpackages."""
    result = TransformResult(spec=spec_with_subpackages)
    result.drop_subpackages = ["debuginfo", "langpack-*"]

    writer = SpecWriter()
    content = writer.write(result)

    # Both should be commented
    assert "#%package debuginfo" in content or "# %package debuginfo" in content
    assert "#%package langpack-en" in content or "# %package langpack-en" in content
    # devel should remain
    assert "%package devel" in content


def test_keep_subpackage(spec_with_subpackages):
    """Test that unlisted subpackages are kept."""
    result = TransformResult(spec=spec_with_subpackages)
    result.drop_subpackages = ["nonexistent"]

    writer = SpecWriter()
    content = writer.write(result)

    # All subpackages should remain
    assert "%package devel" in content
    assert "%package debuginfo" in content
    assert "%package langpack-en" in content
