"""Tests for spec file rewriter."""

from mogrix.emitter.spec import SpecWriter
from mogrix.parser.spec import SpecFile
from mogrix.rules.engine import TransformResult


def test_writer_removes_buildrequires():
    """Writer removes dropped BuildRequires from spec content."""
    original = """Name: test
Version: 1.0
BuildRequires: gcc
BuildRequires: systemd
BuildRequires: zlib
"""
    spec = SpecFile(
        name="test",
        version="1.0",
        buildrequires=["gcc", "zlib"],
        raw_content=original,
    )
    result = TransformResult(
        spec=spec,
        applied_rules=["drop_buildrequires: removed systemd"],
    )
    writer = SpecWriter()

    output = writer.write(result, drops=["systemd"])

    assert "BuildRequires: gcc" in output
    assert "BuildRequires: zlib" in output
    assert "BuildRequires: systemd" not in output


def test_writer_adds_buildrequires():
    """Writer adds new BuildRequires to spec content."""
    original = """Name: test
Version: 1.0
BuildRequires: gcc

%description
Test package.
"""
    spec = SpecFile(
        name="test",
        version="1.0",
        buildrequires=["gcc", "sgug-rpm-config"],
        raw_content=original,
    )
    result = TransformResult(
        spec=spec,
        applied_rules=["add_buildrequires: added sgug-rpm-config"],
    )
    writer = SpecWriter()

    output = writer.write(result, adds=["sgug-rpm-config"])

    assert "BuildRequires: sgug-rpm-config" in output


def test_writer_injects_configure_flags():
    """Writer injects --disable flags into %configure."""
    original = """Name: test
Version: 1.0

%build
%configure
make
"""
    spec = SpecFile(
        name="test",
        version="1.0",
        raw_content=original,
    )
    result = TransformResult(
        spec=spec,
        configure_disable=["selinux", "systemd"],
    )
    writer = SpecWriter()

    output = writer.write(result)

    assert "--disable-selinux" in output
    assert "--disable-systemd" in output


def test_writer_rewrites_paths():
    """Writer rewrites paths according to rules."""
    original = """Name: test
Version: 1.0

%install
install -d %{buildroot}/usr/lib64
"""
    spec = SpecFile(
        name="test",
        version="1.0",
        raw_content=original,
    )
    result = TransformResult(
        spec=spec,
        path_rewrites={"/usr/lib64": "/usr/sgug/lib32"},
    )
    writer = SpecWriter()

    output = writer.write(result)

    assert "/usr/sgug/lib32" in output
    assert "/usr/lib64" not in output


def test_writer_injects_cppflags():
    """Writer injects CPPFLAGS for header overlays after %build."""
    original = """Name: test
Version: 1.0

%build
%configure
make
"""
    spec = SpecFile(
        name="test",
        version="1.0",
        raw_content=original,
    )
    result = TransformResult(
        spec=spec,
        header_overlays=["generic"],
    )
    writer = SpecWriter()

    output = writer.write(result, cppflags="-I/usr/sgug/include/mogrix-compat/generic")

    assert "CPPFLAGS" in output
    assert "-I/usr/sgug/include/mogrix-compat/generic" in output
    # CPPFLAGS should come right after %build
    build_idx = output.find("%build")
    cppflags_idx = output.find("CPPFLAGS")
    assert build_idx < cppflags_idx


def test_writer_injects_compat_sources():
    """Writer injects compat source entries."""
    original = """Name: test
Version: 1.0
Source0: test-1.0.tar.gz

%description
Test package.

%prep
%setup -q

%build
make
"""
    spec = SpecFile(
        name="test",
        version="1.0",
        raw_content=original,
    )
    result = TransformResult(spec=spec)
    writer = SpecWriter()

    compat_sources = "Source100: strdup.c\nSource101: getline.c"
    compat_prep = "mkdir -p mogrix-compat\ncp %{SOURCE100} mogrix-compat/"
    compat_build = 'for f in mogrix-compat/*.c; do\n  %{__cc} %{optflags} -c "$f"'

    output = writer.write(
        result,
        compat_sources=compat_sources,
        compat_prep=compat_prep,
        compat_build=compat_build,
    )

    assert "Source100: strdup.c" in output
    assert "mogrix-compat" in output
