"""Tests for new spec transformation primitives (Phases 1-4)."""

import pytest

from mogrix.emitter.spec import SpecWriter, WriteResult, SpecReplacementError
from mogrix.parser.spec import SpecFile
from mogrix.rules.engine import TransformResult


@pytest.fixture
def spec_with_patches():
    """Spec with patch headers and applications."""
    content = """Name: testpkg
Version: 1.0
Source0: testpkg-1.0.tar.gz
Patch0: fix-build.patch
Patch1: fix-install.patch
Patch2: fix-test.patch
Patch100: fedora-systemd.patch
Patch101: fedora-selinux.patch

%description
A test package.

%prep
%autosetup
%patch -P0 -p1
%patch -P1 -p1
%patch -P2 -p1
%patch -P100 -p1
%patch -P101 -p1

%build
%configure
make

%install
make install DESTDIR=%{buildroot}
install -m 0644 foo.service %{_unitdir}/foo.service
install -m 0644 foo.sysusers %{_sysusersdir}/foo.conf

%files
%{_bindir}/testpkg
%{_unitdir}/foo.service
%{_sysusersdir}/foo.conf
"""
    return SpecFile(
        name="testpkg",
        version="1.0",
        release="1",
        summary="Test",
        license="MIT",
        url="",
        buildrequires=[],
        requires=[],
        sources={"Source0": "testpkg-1.0.tar.gz"},
        patches={},
        raw_content=content,
    )


@pytest.fixture
def spec_with_globals():
    """Spec with %global definitions."""
    content = """Name: testpkg
Version: 1.0
%global WITH_SELINUX 1
%global no_gnome_askpass 0
%global pie 1

%description
Test.

%build
make
"""
    return SpecFile(
        name="testpkg", version="1.0", release="1",
        summary="Test", license="MIT", url="",
        buildrequires=[], requires=[], sources={}, patches={},
        raw_content=content,
    )


@pytest.fixture
def spec_with_subpackages():
    """Spec with subpackages including %pre/%post."""
    content = """Name: testpkg
Version: 1.0

%description
Main package.

%package server
Summary: Server
Requires: testpkg

%description server
Server bits.

%package client
Summary: Client

%description client
Client bits.

%files
%{_bindir}/testpkg

%files server
%{_sbindir}/testpkg-server
%config /etc/testpkg/server.conf

%files client
%{_bindir}/testpkg-client

%pre server
echo "pre-install"

%post server
systemctl enable testpkg-server

%preun server
systemctl stop testpkg-server
"""
    return SpecFile(
        name="testpkg", version="1.0", release="1",
        summary="Test", license="MIT", url="",
        buildrequires=[], requires=[], sources={}, patches={},
        raw_content=content,
    )


# --- Phase 1: Match confirmation ---

class TestMatchConfirmation:
    def test_matched_replacement_not_in_unmatched(self):
        """Replacements that match are not in unmatched list."""
        spec = SpecFile(
            name="t", version="1", release="1", summary="", license="", url="",
            buildrequires=[], requires=[], sources={}, patches={},
            raw_content="hello world\n",
        )
        result = TransformResult(spec=spec)
        writer = SpecWriter()
        wr = writer.write(result, spec_replacements=[
            {"pattern": "hello", "replacement": "goodbye"},
        ])
        assert "goodbye world" in wr.content
        assert len(wr.unmatched) == 0
        assert len(wr.all_matches) == 1
        assert wr.all_matches[0].matched is True

    def test_unmatched_replacement_tracked(self):
        """Replacements that don't match appear in unmatched list."""
        spec = SpecFile(
            name="t", version="1", release="1", summary="", license="", url="",
            buildrequires=[], requires=[], sources={}, patches={},
            raw_content="hello world\n",
        )
        result = TransformResult(spec=spec)
        writer = SpecWriter()
        wr = writer.write(result, spec_replacements=[
            {"pattern": "nonexistent", "replacement": "something"},
        ])
        assert len(wr.unmatched) == 1
        assert wr.unmatched[0].pattern == "nonexistent"
        assert wr.unmatched[0].matched is False

    def test_optional_unmatched_not_in_required(self):
        """Optional replacements that don't match are not in unmatched_required."""
        spec = SpecFile(
            name="t", version="1", release="1", summary="", license="", url="",
            buildrequires=[], requires=[], sources={}, patches={},
            raw_content="hello world\n",
        )
        result = TransformResult(spec=spec)
        writer = SpecWriter()
        wr = writer.write(result, spec_replacements=[
            {"pattern": "nonexistent", "replacement": "x", "optional": True},
        ])
        assert len(wr.unmatched) == 1
        assert len(wr.unmatched_required) == 0

    def test_strict_mode_raises_on_unmatched(self):
        """Strict mode raises SpecReplacementError."""
        spec = SpecFile(
            name="t", version="1", release="1", summary="", license="", url="",
            buildrequires=[], requires=[], sources={}, patches={},
            raw_content="hello world\n",
        )
        result = TransformResult(spec=spec)
        writer = SpecWriter()
        with pytest.raises(SpecReplacementError):
            writer.write(result, spec_replacements=[
                {"pattern": "nonexistent", "replacement": "x"},
            ], strict=True)


# --- Phase 4: Regex support ---

class TestRegexReplacements:
    def test_regex_replacement(self):
        """pattern_regex uses re.sub."""
        spec = SpecFile(
            name="t", version="1", release="1", summary="", license="", url="",
            buildrequires=[], requires=[], sources={}, patches={},
            raw_content="Version: 1.0\nRelease: 3\n",
        )
        result = TransformResult(spec=spec)
        writer = SpecWriter()
        wr = writer.write(result, spec_replacements=[
            {"pattern_regex": r"Release:\s+\d+", "replacement": "Release: 99"},
        ])
        assert "Release: 99" in wr.content
        assert len(wr.unmatched) == 0

    def test_regex_backreference(self):
        """Regex backreferences work."""
        spec = SpecFile(
            name="t", version="1", release="1", summary="", license="", url="",
            buildrequires=[], requires=[], sources={}, patches={},
            raw_content="--with-default-path=/usr/bin\n",
        )
        result = TransformResult(spec=spec)
        writer = SpecWriter()
        wr = writer.write(result, spec_replacements=[
            {
                "pattern_regex": r"(--with-default-path=).*",
                "replacement": r"\1/opt/mogrix/bin",
            },
        ])
        assert "--with-default-path=/opt/mogrix/bin" in wr.content

    def test_regex_no_match_tracked(self):
        """Unmatched regex is tracked."""
        spec = SpecFile(
            name="t", version="1", release="1", summary="", license="", url="",
            buildrequires=[], requires=[], sources={}, patches={},
            raw_content="hello\n",
        )
        result = TransformResult(spec=spec)
        writer = SpecWriter()
        wr = writer.write(result, spec_replacements=[
            {"pattern_regex": r"^nonexistent$", "replacement": "x"},
        ])
        assert len(wr.unmatched) == 1
        assert wr.unmatched[0].is_regex is True


# --- Phase 2a: comment_matching / remove_matching ---

class TestCommentMatching:
    def test_comment_matching_global(self, spec_with_patches):
        result = TransformResult(spec=spec_with_patches)
        result.comment_matching = [
            {"regex": r"^%patch -P10[01]"},
        ]
        writer = SpecWriter()
        wr = writer.write(result)
        assert "# %patch -P100" in wr.content
        assert "# %patch -P101" in wr.content
        assert "%patch -P0 -p1" in wr.content  # not commented

    def test_comment_matching_with_section(self, spec_with_patches):
        result = TransformResult(spec=spec_with_patches)
        result.comment_matching = [
            {"regex": r"^install.*%\{_unitdir\}", "section": "%install"},
        ]
        writer = SpecWriter()
        wr = writer.write(result)
        # The install line in %install should be commented
        assert "# install" in wr.content
        # But the %files line should NOT be commented (different section)
        assert "%{_unitdir}/foo.service" in wr.content

    def test_comment_matching_custom_comment(self, spec_with_patches):
        result = TransformResult(spec=spec_with_patches)
        result.comment_matching = [
            {"regex": r"^%patch -P100", "comment": "disabled for IRIX"},
        ]
        writer = SpecWriter()
        wr = writer.write(result)
        assert "# disabled for IRIX %patch -P100" in wr.content


class TestRemoveMatching:
    def test_remove_matching_global(self, spec_with_patches):
        result = TransformResult(spec=spec_with_patches)
        result.remove_matching = [
            {"regex": r"^%patch -P10[01]"},
        ]
        writer = SpecWriter()
        wr = writer.write(result)
        assert "%patch -P100" not in wr.content
        assert "%patch -P101" not in wr.content
        assert "%patch -P0 -p1" in wr.content

    def test_remove_matching_section_scoped(self, spec_with_patches):
        result = TransformResult(spec=spec_with_patches)
        result.remove_matching = [
            {"regex": r"%\{_unitdir\}", "section": "%files"},
        ]
        writer = SpecWriter()
        wr = writer.write(result)
        # Removed from %files
        lines = wr.content.split("\n")
        files_section_lines = []
        in_files = False
        for line in lines:
            if line.strip().startswith("%files"):
                in_files = True
                continue
            if in_files and line.strip().startswith("%"):
                break
            if in_files:
                files_section_lines.append(line)
        assert not any("%{_unitdir}" in l for l in files_section_lines)


# --- Phase 2b: drop_patches ---

class TestDropPatches:
    def test_drop_all_patches(self, spec_with_patches):
        result = TransformResult(spec=spec_with_patches)
        result.drop_patches = "all"
        writer = SpecWriter()
        wr = writer.write(result)
        # All patch headers and applications should be commented
        for line in wr.content.splitlines():
            stripped = line.strip()
            if stripped.startswith("Patch") and ":" in stripped:
                assert stripped.startswith("#"), f"Patch header not commented: {stripped}"

    def test_drop_patches_except(self, spec_with_patches):
        result = TransformResult(spec=spec_with_patches)
        result.drop_patches = {"except": [0, 1]}
        writer = SpecWriter()
        wr = writer.write(result)
        # Patches 0 and 1 should be kept
        assert "Patch0:" in wr.content
        assert "Patch1:" in wr.content
        # Others commented
        assert "# Patch2:" in wr.content
        assert "# Patch100:" in wr.content

    def test_drop_patches_only(self, spec_with_patches):
        result = TransformResult(spec=spec_with_patches)
        result.drop_patches = {"only": [100, 101]}
        writer = SpecWriter()
        wr = writer.write(result)
        # Only 100 and 101 should be commented
        assert "# Patch100:" in wr.content
        assert "# Patch101:" in wr.content
        # Others kept
        assert "Patch0: fix-build.patch" in wr.content


# --- Phase 2c: flip_globals ---

class TestFlipGlobals:
    def test_flip_1_to_0(self, spec_with_globals):
        result = TransformResult(spec=spec_with_globals)
        result.flip_globals = ["WITH_SELINUX"]
        writer = SpecWriter()
        wr = writer.write(result)
        assert "%global WITH_SELINUX 0" in wr.content

    def test_flip_0_to_1(self, spec_with_globals):
        result = TransformResult(spec=spec_with_globals)
        result.flip_globals = ["no_gnome_askpass"]
        writer = SpecWriter()
        wr = writer.write(result)
        assert "%global no_gnome_askpass 1" in wr.content

    def test_flip_multiple(self, spec_with_globals):
        result = TransformResult(spec=spec_with_globals)
        result.flip_globals = ["WITH_SELINUX", "pie"]
        writer = SpecWriter()
        wr = writer.write(result)
        assert "%global WITH_SELINUX 0" in wr.content
        assert "%global pie 0" in wr.content


# --- Phase 2d: Enhanced drop_subpackages ---

class TestEnhancedDropSubpackages:
    def test_drop_subpackage_pre_post(self, spec_with_subpackages):
        """drop_subpackages should comment out %pre and %post sections too."""
        result = TransformResult(spec=spec_with_subpackages)
        result.drop_subpackages = ["server"]
        writer = SpecWriter()
        wr = writer.write(result)
        content = wr.content

        # All server sections should be commented
        assert "#%package server" in content
        assert "#%description server" in content
        assert "#%files server" in content
        assert "#%pre server" in content
        assert "#%post server" in content
        assert "#%preun server" in content

        # Client sections should NOT be commented
        assert "%package client" in content
        assert "%files client" in content

    def test_drop_preserves_main_package(self, spec_with_subpackages):
        """Main package sections are untouched."""
        result = TransformResult(spec=spec_with_subpackages)
        result.drop_subpackages = ["server", "client"]
        writer = SpecWriter()
        wr = writer.write(result)
        content = wr.content

        # Main %files should be preserved
        assert "%{_bindir}/testpkg" in content
        # But subpackage files should be commented
        assert "#%{_sbindir}/testpkg-server" in content


# --- Phase 3a: section_replace ---

class TestSectionReplace:
    def test_replace_build_section(self):
        spec = SpecFile(
            name="t", version="1", release="1", summary="", license="", url="",
            buildrequires=[], requires=[], sources={}, patches={},
            raw_content="""Name: testpkg

%build
%configure
make

%install
make install
""",
        )
        result = TransformResult(spec=spec)
        result.section_replace = [
            {
                "section": "%build",
                "content": "\nexport CFLAGS=\"-O2\"\nmake all\n",
            },
        ]
        writer = SpecWriter()
        wr = writer.write(result)
        assert "export CFLAGS" in wr.content
        assert "make all" in wr.content
        assert "%configure" not in wr.content
        # %install should be unchanged
        assert "make install" in wr.content
