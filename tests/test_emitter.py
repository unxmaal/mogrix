"""Tests for spec emitter functionality."""

import pytest
from mogrix.emitter.spec import SpecWriter
from mogrix.rules.engine import TransformResult
from mogrix.parser.spec import SpecFile


@pytest.fixture
def basic_spec():
    """Create a basic spec file."""
    content = """Name: testpkg
Version: 1.0
Release: 1
Summary: Test package

BuildRequires: gcc
BuildRequires: make

%build
%configure --with-ssl
make %{?_smp_mflags}

%install
make install DESTDIR=%{buildroot}
"""
    return SpecFile(
        name="testpkg",
        version="1.0",
        release="1",
        summary="Test package",
        license="MIT",
        url="",
        buildrequires=["gcc", "make"],
        requires=[],
        sources={},
        patches={},
        raw_content=content,
    )


def test_configure_disable(basic_spec):
    """Test adding --disable flags to %configure."""
    result = TransformResult(spec=basic_spec)
    result.configure_disable = ["selinux", "systemd"]

    writer = SpecWriter()
    content = writer.write(result)

    assert "--disable-selinux" in content
    assert "--disable-systemd" in content


def test_configure_flags_add(basic_spec):
    """Test adding configure flags."""
    result = TransformResult(spec=basic_spec)
    result.configure_flags_add = ["--without-dbus", "--disable-nls"]

    writer = SpecWriter()
    content = writer.write(result)

    assert "--without-dbus" in content
    assert "--disable-nls" in content


def test_configure_flags_remove(basic_spec):
    """Test removing configure flags."""
    result = TransformResult(spec=basic_spec)
    result.configure_flags_remove = ["--with-ssl"]

    writer = SpecWriter()
    content = writer.write(result)

    assert "--with-ssl" not in content


def test_configure_flags_combined(basic_spec):
    """Test adding and removing configure flags together."""
    result = TransformResult(spec=basic_spec)
    result.configure_flags_add = ["--without-dbus"]
    result.configure_flags_remove = ["--with-ssl"]
    result.configure_disable = ["nls"]

    writer = SpecWriter()
    content = writer.write(result)

    assert "--without-dbus" in content
    assert "--with-ssl" not in content
    assert "--disable-nls" in content


def test_drop_buildrequires(basic_spec):
    """Test dropping BuildRequires."""
    result = TransformResult(spec=basic_spec)

    writer = SpecWriter()
    content = writer.write(result, drops=["make"])

    assert "BuildRequires: make" not in content
    assert "BuildRequires: gcc" in content


def test_add_buildrequires(basic_spec):
    """Test adding BuildRequires."""
    result = TransformResult(spec=basic_spec)

    writer = SpecWriter()
    content = writer.write(result, adds=["zlib-devel"])

    assert "BuildRequires: zlib-devel" in content


def test_ac_cv_overrides(basic_spec):
    """Test ac_cv override injection."""
    result = TransformResult(spec=basic_spec)

    writer = SpecWriter()
    content = writer.write(
        result,
        ac_cv_overrides={"ac_cv_func_malloc_0_nonnull": "yes"}
    )

    assert "export ac_cv_func_malloc_0_nonnull=yes" in content
    assert "Autoconf cache overrides" in content
