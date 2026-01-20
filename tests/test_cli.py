"""Tests for CLI."""

from pathlib import Path
from click.testing import CliRunner
from mogrix.cli import main, analyze

FIXTURES = Path(__file__).parent / "fixtures"


def test_cli_help():
    """CLI shows help."""
    runner = CliRunner()
    result = runner.invoke(main, ["--help"])
    assert result.exit_code == 0
    assert "SRPM conversion engine" in result.output


def test_analyze_command():
    """Analyze command parses spec and shows rules."""
    runner = CliRunner()
    result = runner.invoke(analyze, [str(FIXTURES / "zlib.spec")])
    assert result.exit_code == 0
    assert "zlib" in result.output


def test_analyze_shows_buildrequires():
    """Analyze shows BuildRequires."""
    runner = CliRunner()
    result = runner.invoke(analyze, [str(FIXTURES / "zlib.spec")])
    assert "BuildRequires" in result.output
    assert "automake" in result.output
