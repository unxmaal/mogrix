"""Tests for dependency resolution."""

import tempfile
from pathlib import Path
from unittest.mock import patch, MagicMock

import pytest

from mogrix.deps.resolver import DependencyResolver, MissingDep
from mogrix.deps.fedora import FedoraRepo, SRPMInfo


@pytest.fixture
def temp_rules_dir():
    """Create a temp rules directory with some package rules."""
    with tempfile.TemporaryDirectory() as tmpdir:
        rules_path = Path(tmpdir)
        packages_dir = rules_path / "packages"
        packages_dir.mkdir()

        # Create some package rules
        (packages_dir / "zlib.yaml").write_text("package: zlib\n")
        (packages_dir / "curl.yaml").write_text("package: curl\n")
        (packages_dir / "openssl.yaml").write_text("package: openssl\n")

        yield rules_path


def test_resolver_finds_available_rules(temp_rules_dir):
    """Test that resolver discovers available package rules."""
    resolver = DependencyResolver(temp_rules_dir)
    rules = resolver.available_rules

    assert "zlib" in rules
    assert "curl" in rules
    assert "openssl" in rules
    assert len(rules) == 3


def test_resolver_parses_rpmbuild_errors():
    """Test parsing of rpmbuild missing dependency output."""
    output = """Building target platforms: irix6.5-n32
Building for target irix6.5-n32
error: Failed build dependencies:
        gcc is needed by popt-1.19-6.irix6.5
        zlib-devel >= 1.2 is needed by popt-1.19-6.irix6.5
        openssl-devel is needed by popt-1.19-6.irix6.5
"""
    resolver = DependencyResolver(Path("/nonexistent"))
    missing = resolver.parse_rpmbuild_errors(output)

    assert len(missing) == 3
    assert missing[0].name == "gcc"
    assert missing[0].version is None
    assert missing[1].name == "zlib-devel"
    assert missing[1].version == "1.2"
    assert missing[2].name == "openssl-devel"


def test_resolver_checks_has_rule(temp_rules_dir):
    """Test that resolver checks for matching rules."""
    resolver = DependencyResolver(temp_rules_dir)

    # Direct match
    assert resolver._check_has_rule("zlib") is True
    assert resolver._check_has_rule("curl") is True

    # -devel suffix mapping
    assert resolver._check_has_rule("zlib-devel") is True
    assert resolver._check_has_rule("openssl-devel") is True

    # No match
    assert resolver._check_has_rule("unknown-package") is False


def test_resolver_gets_package_for_dep(temp_rules_dir):
    """Test mapping dependency names to package names."""
    resolver = DependencyResolver(temp_rules_dir)

    assert resolver.get_package_for_dep("zlib") == "zlib"
    assert resolver.get_package_for_dep("zlib-devel") == "zlib"
    assert resolver.get_package_for_dep("openssl-devel") == "openssl"
    assert resolver.get_package_for_dep("unknown") is None


def test_resolver_categorizes_deps(temp_rules_dir):
    """Test categorization of dependencies."""
    resolver = DependencyResolver(temp_rules_dir)

    deps = [
        MissingDep(name="gcc", has_rule=False),
        MissingDep(name="make", has_rule=False),
        MissingDep(name="zlib-devel", has_rule=True),
        MissingDep(name="unknown-lib", has_rule=False),
    ]

    categorized = resolver.categorize_deps(deps)

    # gcc and make are system packages
    assert len(categorized["system"]) == 2
    system_names = [d.name for d in categorized["system"]]
    assert "gcc" in system_names
    assert "make" in system_names

    # zlib-devel has a rule
    assert len(categorized["have_rules"]) == 1
    assert categorized["have_rules"][0].name == "zlib-devel"

    # unknown-lib needs a rule
    assert len(categorized["need_rules"]) == 1
    assert categorized["need_rules"][0].name == "unknown-lib"


# Tests for SRPMInfo

def test_srpm_info_from_simple_filename():
    """Test parsing a simple SRPM filename."""
    info = SRPMInfo.from_filename("popt-1.19-6.fc40.src.rpm", "http://example.com/")

    assert info.name == "popt"
    assert info.version == "1.19"
    assert info.release == "6.fc40"
    assert info.filename == "popt-1.19-6.fc40.src.rpm"
    assert info.url == "http://example.com/popt-1.19-6.fc40.src.rpm"


def test_srpm_info_from_hyphenated_name():
    """Test parsing SRPM with hyphenated package name."""
    info = SRPMInfo.from_filename("zlib-ng-2.1.6-2.fc40.src.rpm", "http://example.com/")

    assert info.name == "zlib-ng"
    assert info.version == "2.1.6"
    assert info.release == "2.fc40"


def test_srpm_info_from_complex_version():
    """Test parsing SRPM with complex version string."""
    info = SRPMInfo.from_filename("zlib-ada-1.4-0.37.20210811gitca39312.fc40.src.rpm", "http://example.com/")

    assert info.name == "zlib-ada"
    assert info.version == "1.4"
    assert info.release == "0.37.20210811gitca39312.fc40"


# Tests for FedoraRepo

def test_fedora_repo_default_base_url():
    """Test that FedoraRepo uses archive URL by default."""
    repo = FedoraRepo(release="40")
    url = repo._get_package_list_url("p")

    assert "archives.fedoraproject.org" in url
    assert "/40/" in url
    assert "/Packages/p/" in url


def test_fedora_repo_custom_base_url():
    """Test that FedoraRepo can use a custom base URL."""
    repo = FedoraRepo(release="40", base_url="http://custom.example.com")
    url = repo._get_package_list_url("p")

    assert url.startswith("http://custom.example.com")
    assert "/40/" in url


def test_fedora_repo_preset_photon():
    """Test that FedoraRepo supports Photon OS presets."""
    repo = FedoraRepo(base_url="photon5")

    assert repo._is_flat is True
    url = repo._get_package_list_url("t")
    assert "packages.vmware.com/photon" in url
    # Flat directory - no first-letter subdirectory
    assert "/Packages/t/" not in url


def test_fedora_repo_flat_directory():
    """Test that URLs ending with / are treated as flat directories."""
    repo = FedoraRepo(base_url="http://example.com/srpms/")

    assert repo._is_flat is True
    url = repo._get_package_list_url("p")
    # Should not add Fedora-style path structure
    assert url == "http://example.com/srpms/"


def test_fedora_repo_search_packages():
    """Test package search with mocked HTTP response."""
    html_content = """
    <html>
    <a href="popt-1.19-6.fc40.src.rpm">popt-1.19-6.fc40.src.rpm</a>
    <a href="popt-devel-1.19-6.fc40.src.rpm">popt-devel-1.19-6.fc40.src.rpm</a>
    <a href="poppler-24.02.0-1.fc40.src.rpm">poppler-24.02.0-1.fc40.src.rpm</a>
    </html>
    """

    repo = FedoraRepo(release="40")

    with patch("urllib.request.urlopen") as mock_urlopen:
        mock_response = MagicMock()
        mock_response.read.return_value = html_content.encode("utf-8")
        mock_response.__enter__ = MagicMock(return_value=mock_response)
        mock_response.__exit__ = MagicMock(return_value=False)
        mock_urlopen.return_value = mock_response

        matches = repo.search_packages("popt")

        # Should find popt and popt-devel, not poppler
        assert len(matches) == 2
        names = [m.name for m in matches]
        assert "popt" in names
        assert "popt-devel" in names


def test_fedora_repo_get_srpm_url_exact_match():
    """Test getting SRPM URL with exact match."""
    html_content = """
    <a href="popt-1.19-6.fc40.src.rpm">popt-1.19-6.fc40.src.rpm</a>
    <a href="poppler-24.02.0-1.fc40.src.rpm">poppler-24.02.0-1.fc40.src.rpm</a>
    """

    repo = FedoraRepo(release="40")

    with patch("urllib.request.urlopen") as mock_urlopen:
        mock_response = MagicMock()
        mock_response.read.return_value = html_content.encode("utf-8")
        mock_response.__enter__ = MagicMock(return_value=mock_response)
        mock_response.__exit__ = MagicMock(return_value=False)
        mock_urlopen.return_value = mock_response

        url = repo.get_srpm_url("popt")

        assert url is not None
        assert "popt-1.19-6.fc40.src.rpm" in url


def test_fedora_repo_fuzzy_search():
    """Test fuzzy search when no exact matches exist."""
    html_content = """
    <a href="libpopt-1.0-1.fc40.src.rpm">libpopt-1.0-1.fc40.src.rpm</a>
    <a href="pypopt-2.0-1.fc40.src.rpm">pypopt-2.0-1.fc40.src.rpm</a>
    <a href="unrelated-3.0-1.fc40.src.rpm">unrelated-3.0-1.fc40.src.rpm</a>
    """

    repo = FedoraRepo(release="40")

    with patch("urllib.request.urlopen") as mock_urlopen:
        mock_response = MagicMock()
        mock_response.read.return_value = html_content.encode("utf-8")
        mock_response.__enter__ = MagicMock(return_value=mock_response)
        mock_response.__exit__ = MagicMock(return_value=False)
        mock_urlopen.return_value = mock_response

        # Search for "popt" - no exact match, should return fuzzy matches
        matches = repo.search_packages("popt")

        # Should find libpopt and pypopt (both contain "popt")
        assert len(matches) == 2
        names = [m.name for m in matches]
        assert "libpopt" in names
        assert "pypopt" in names
        assert "unrelated" not in names


def test_fedora_repo_exact_matches_preferred_over_fuzzy():
    """Test that exact matches are preferred over fuzzy matches."""
    html_content = """
    <a href="popt-1.19-6.fc40.src.rpm">popt-1.19-6.fc40.src.rpm</a>
    <a href="libpopt-1.0-1.fc40.src.rpm">libpopt-1.0-1.fc40.src.rpm</a>
    <a href="pypopt-2.0-1.fc40.src.rpm">pypopt-2.0-1.fc40.src.rpm</a>
    """

    repo = FedoraRepo(release="40")

    with patch("urllib.request.urlopen") as mock_urlopen:
        mock_response = MagicMock()
        mock_response.read.return_value = html_content.encode("utf-8")
        mock_response.__enter__ = MagicMock(return_value=mock_response)
        mock_response.__exit__ = MagicMock(return_value=False)
        mock_urlopen.return_value = mock_response

        # Search for "popt" - has exact match, should only return that
        matches = repo.search_packages("popt")

        # Should only find the exact match "popt", not the fuzzy ones
        assert len(matches) == 1
        assert matches[0].name == "popt"
