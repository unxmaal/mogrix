"""Tests for spec file parser."""

from pathlib import Path
from mogrix.parser.spec import SpecParser

FIXTURES = Path(__file__).parent / "fixtures"


def test_parse_preamble_name():
    """Parser extracts Name field."""
    parser = SpecParser()
    spec = parser.parse(FIXTURES / "zlib.spec")
    assert spec.name == "zlib"


def test_parse_preamble_version():
    """Parser extracts Version field."""
    parser = SpecParser()
    spec = parser.parse(FIXTURES / "zlib.spec")
    assert spec.version == "1.2.11"


def test_parse_preamble_release():
    """Parser extracts Release field."""
    parser = SpecParser()
    spec = parser.parse(FIXTURES / "zlib.spec")
    assert spec.release == "20%{?dist}"


def test_parse_preamble_summary():
    """Parser extracts Summary field."""
    parser = SpecParser()
    spec = parser.parse(FIXTURES / "zlib.spec")
    assert spec.summary == "The compression and decompression library"


def test_parse_buildrequires():
    """Parser extracts BuildRequires."""
    parser = SpecParser()
    spec = parser.parse(FIXTURES / "zlib.spec")
    assert "automake" in spec.buildrequires
    assert "autoconf" in spec.buildrequires
    assert "libtool" in spec.buildrequires


def test_parse_sources():
    """Parser extracts Source entries."""
    parser = SpecParser()
    spec = parser.parse(FIXTURES / "zlib.spec")
    assert "Source" in spec.sources or "Source0" in spec.sources


def test_parse_patches():
    """Parser extracts Patch entries."""
    parser = SpecParser()
    spec = parser.parse(FIXTURES / "zlib.spec")
    assert "Patch0" in spec.patches
