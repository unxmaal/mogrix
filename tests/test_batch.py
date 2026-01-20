"""Tests for batch conversion functionality."""

import tempfile
from pathlib import Path

import pytest

from mogrix.batch import BatchConverter


@pytest.fixture
def temp_specs_dir():
    """Create a temp directory with multiple spec files."""
    with tempfile.TemporaryDirectory() as tmpdir:
        specs_path = Path(tmpdir)

        # Create simple spec files
        (specs_path / "zlib.spec").write_text("""Name: zlib
Version: 1.2.13
Release: 1
Summary: Compression library
BuildRequires: gcc
%build
%configure
""")

        (specs_path / "curl.spec").write_text("""Name: curl
Version: 8.0.0
Release: 1
Summary: URL transfer tool
BuildRequires: zlib-devel
BuildRequires: openssl-devel
%build
%configure
""")

        yield specs_path


@pytest.fixture
def temp_output_dir():
    """Create a temp output directory."""
    with tempfile.TemporaryDirectory() as tmpdir:
        yield Path(tmpdir)


def test_batch_discovers_specs(temp_specs_dir):
    """Test that batch converter finds all spec files."""
    converter = BatchConverter(temp_specs_dir)
    specs = converter.discover_specs()

    assert len(specs) == 2
    names = [s.name for s in specs]
    assert "zlib.spec" in names
    assert "curl.spec" in names


def test_batch_converts_all(temp_specs_dir, temp_output_dir):
    """Test batch conversion of multiple specs."""
    converter = BatchConverter(temp_specs_dir)
    results = converter.convert_all(temp_output_dir)

    assert len(results) == 2
    assert (temp_output_dir / "zlib.spec").exists()
    assert (temp_output_dir / "curl.spec").exists()


def test_batch_reports_results(temp_specs_dir, temp_output_dir):
    """Test that batch conversion returns result info."""
    converter = BatchConverter(temp_specs_dir)
    results = converter.convert_all(temp_output_dir)

    for result in results:
        assert "package" in result
        assert "status" in result
        assert result["status"] in ["success", "error"]
