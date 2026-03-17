"""Tests for spec section splitting and reassembly."""

import pytest

from mogrix.parser.sections import (
    SpecSection,
    split_spec_sections,
    reassemble_spec,
    find_sections,
)


SIMPLE_SPEC = """Name: testpkg
Version: 1.0
Summary: Test

%description
A test package.

%prep
%autosetup

%build
%configure
make

%install
make install DESTDIR=%{buildroot}

%files
%{_bindir}/testpkg

%changelog
* Mon Jan 01 2024 Test User <test@test.com> - 1.0-1
- Initial package"""


SUBPACKAGE_SPEC = """Name: testpkg
Version: 1.0

%description
Main package.

%package server
Summary: Server subpackage
Requires: testpkg

%description server
The server component.

%files
%{_bindir}/testpkg

%files server
%{_sbindir}/testpkg-server

%pre server
echo "pre-install for server"

%post server
echo "post-install for server"
"""


def test_roundtrip():
    """split -> reassemble produces identical output."""
    sections = split_spec_sections(SIMPLE_SPEC)
    result = reassemble_spec(sections)
    assert result == SIMPLE_SPEC


def test_preamble_detected():
    """Preamble section is extracted before first section marker."""
    sections = split_spec_sections(SIMPLE_SPEC)
    assert sections[0].name == "preamble"
    assert "Name: testpkg" in sections[0].content


def test_sections_detected():
    """All major sections are found."""
    sections = split_spec_sections(SIMPLE_SPEC)
    names = [s.name for s in sections]
    assert "preamble" in names
    assert "%description" in names
    assert "%prep" in names
    assert "%build" in names
    assert "%install" in names
    assert "%files" in names
    assert "%changelog" in names


def test_subpackage_sections():
    """Subpackage suffix is correctly parsed."""
    sections = split_spec_sections(SUBPACKAGE_SPEC)

    # Find %files server
    server_files = find_sections(sections, name="%files", subpackage="server")
    assert len(server_files) == 1
    assert "%{_sbindir}/testpkg-server" in server_files[0].content

    # Find main %files (empty subpackage)
    main_files = find_sections(sections, name="%files", subpackage="")
    assert len(main_files) == 1
    assert "%{_bindir}/testpkg" in main_files[0].content


def test_pre_post_subpackage():
    """Pre/post scripts for subpackages are detected."""
    sections = split_spec_sections(SUBPACKAGE_SPEC)

    pre_server = find_sections(sections, name="%pre", subpackage="server")
    assert len(pre_server) == 1
    assert "pre-install" in pre_server[0].content

    post_server = find_sections(sections, name="%post", subpackage="server")
    assert len(post_server) == 1
    assert "post-install" in post_server[0].content


def test_roundtrip_subpackages():
    """Roundtrip with subpackages preserves content."""
    sections = split_spec_sections(SUBPACKAGE_SPEC)
    result = reassemble_spec(sections)
    assert result == SUBPACKAGE_SPEC


def test_no_sections():
    """Content with no section markers returns single preamble."""
    content = "Name: testpkg\nVersion: 1.0\n"
    sections = split_spec_sections(content)
    assert len(sections) == 1
    assert sections[0].name == "preamble"
    assert reassemble_spec(sections) == content


def test_find_sections_name_filter():
    """find_sections filters by name."""
    sections = split_spec_sections(SIMPLE_SPEC)
    builds = find_sections(sections, name="%build")
    assert len(builds) == 1
    assert "%configure" in builds[0].content


def test_section_content_excludes_marker():
    """Section content does not include the marker line itself."""
    sections = split_spec_sections(SIMPLE_SPEC)
    build = find_sections(sections, name="%build")[0]
    assert not build.content.startswith("%build")
    assert "%configure" in build.content


def test_dash_n_subpackage():
    """Handle -n prefix in subpackage names."""
    content = """Name: foo
%package -n libfoo
Summary: Library

%files -n libfoo
%{_libdir}/libfoo.so
"""
    sections = split_spec_sections(content)
    lib_files = find_sections(sections, name="%files", subpackage="libfoo")
    assert len(lib_files) == 1
