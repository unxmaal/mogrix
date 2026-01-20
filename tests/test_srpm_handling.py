"""Tests for SRPM handling in CLI commands."""

import tempfile
from pathlib import Path
from unittest.mock import patch, MagicMock

import pytest

from click.testing import CliRunner

from mogrix.cli import main


@pytest.fixture
def sample_spec_content():
    """Sample spec file content for testing."""
    return """Name: test-package
Version: 1.0.0
Release: 1
Summary: Test package
License: MIT
BuildRequires: gcc
BuildRequires: make

%description
A test package.

%build
%configure
make

%install
make install DESTDIR=%{buildroot}

%files
%{_bindir}/*
"""


@pytest.fixture
def mock_srpm_extractor(sample_spec_content):
    """Mock SRPMExtractor for testing without real SRPMs."""
    def _create_mock(temp_dir):
        # Create a spec file in the temp directory
        spec_path = temp_dir / "test-package.spec"
        spec_path.write_text(sample_spec_content)

        mock_extractor = MagicMock()
        mock_extractor.extract_spec.return_value = (temp_dir, spec_path)
        return mock_extractor

    return _create_mock


def test_analyze_rejects_binary_file_as_spec():
    """Test that analyze properly handles binary files."""
    runner = CliRunner()

    with tempfile.NamedTemporaryFile(suffix=".spec", delete=False) as f:
        # Write binary content that would fail UTF-8 decode
        f.write(b"\xed\x00\x00\x00binary content")
        f.flush()

        result = runner.invoke(main, ["analyze", f.name])

        # Should fail with a decode error or similar
        assert result.exit_code != 0


def test_convert_rejects_binary_file_as_spec():
    """Test that convert properly handles binary files."""
    runner = CliRunner()

    with tempfile.NamedTemporaryFile(suffix=".spec", delete=False) as f:
        # Write binary content that would fail UTF-8 decode
        f.write(b"\xed\x00\x00\x00binary content")
        f.flush()

        result = runner.invoke(main, ["convert", f.name])

        # Should fail with a decode error or similar
        assert result.exit_code != 0


def test_analyze_handles_srpm_extension(mock_srpm_extractor, sample_spec_content):
    """Test that analyze detects and handles .src.rpm files."""
    runner = CliRunner()

    with tempfile.TemporaryDirectory() as tmpdir:
        temp_dir = Path(tmpdir)

        # Create a fake SRPM file (just needs to exist with right extension)
        fake_srpm = temp_dir / "test-package-1.0.0-1.src.rpm"
        fake_srpm.write_bytes(b"fake rpm content")

        # Mock the SRPMExtractor at its source module
        with patch("mogrix.parser.srpm.SRPMExtractor") as mock_class:
            mock_instance = mock_srpm_extractor(temp_dir)
            mock_class.return_value = mock_instance

            result = runner.invoke(main, ["analyze", str(fake_srpm)])

            # Should have called SRPMExtractor
            mock_class.assert_called_once()
            mock_instance.extract_spec.assert_called_once()


def test_convert_handles_srpm_extension(mock_srpm_extractor, sample_spec_content):
    """Test that convert detects and handles .src.rpm files."""
    runner = CliRunner()

    with tempfile.TemporaryDirectory() as tmpdir:
        temp_dir = Path(tmpdir)

        # Create a fake SRPM file
        fake_srpm = temp_dir / "test-package-1.0.0-1.src.rpm"
        fake_srpm.write_bytes(b"fake rpm content")

        # Mock the SRPMExtractor at its source module
        with patch("mogrix.parser.srpm.SRPMExtractor") as mock_class:
            mock_instance = mock_srpm_extractor(temp_dir)
            mock_class.return_value = mock_instance

            result = runner.invoke(main, ["convert", str(fake_srpm)])

            # Should have called SRPMExtractor
            mock_class.assert_called_once()
            mock_instance.extract_spec.assert_called_once()

            # Should succeed
            assert result.exit_code == 0


def test_analyze_spec_file_directly(sample_spec_content):
    """Test that analyze works with a direct spec file."""
    runner = CliRunner()

    with tempfile.NamedTemporaryFile(suffix=".spec", mode="w", delete=False) as f:
        f.write(sample_spec_content)
        f.flush()

        result = runner.invoke(main, ["analyze", f.name])

        # Should succeed
        assert result.exit_code == 0
        assert "test-package" in result.output


def test_convert_spec_file_directly(sample_spec_content):
    """Test that convert works with a direct spec file."""
    runner = CliRunner()

    with tempfile.NamedTemporaryFile(suffix=".spec", mode="w", delete=False) as f:
        f.write(sample_spec_content)
        f.flush()

        result = runner.invoke(main, ["convert", f.name])

        # Should succeed and output converted spec
        assert result.exit_code == 0
        # Should contain RPM macros
        assert "%define _prefix" in result.output or "Name:" in result.output


def test_srpm_extractor_error_message():
    """Test that SRPM extraction errors include helpful messages."""
    from mogrix.parser.srpm import SRPMExtractor

    with tempfile.NamedTemporaryFile(suffix=".src.rpm", delete=False) as f:
        # Write invalid content
        f.write(b"not a valid rpm")
        f.flush()

        extractor = SRPMExtractor(Path(f.name))

        # Should raise with a helpful error message
        with pytest.raises(RuntimeError) as exc_info:
            extractor.extract()

        # Error should mention rpm2cpio or the actual problem
        assert "rpm2cpio" in str(exc_info.value).lower() or "rpm" in str(exc_info.value).lower()


def test_convert_srpm_copies_compat_sources():
    """Test that convert copies compat source files when package rules require them."""
    runner = CliRunner()

    with tempfile.TemporaryDirectory() as tmpdir:
        temp_dir = Path(tmpdir)
        extracted_dir = temp_dir / "extracted"
        extracted_dir.mkdir()
        output_dir = temp_dir / "output"

        # Create a spec file that will trigger compat function injection
        # Use package name "testpkg" and create a matching rule
        spec_content = """Name: testpkg
Version: 1.0.0
Release: 1
Summary: Test package
License: MIT

%description
A test package.

%build
%configure
make
"""
        spec_path = extracted_dir / "testpkg.spec"
        spec_path.write_text(spec_content)

        # Create a fake source tarball
        (extracted_dir / "testpkg-1.0.0.tar.gz").write_bytes(b"fake tarball")

        # Create a fake SRPM file
        fake_srpm = temp_dir / "testpkg-1.0.0-1.src.rpm"
        fake_srpm.write_bytes(b"fake rpm content")

        # Create a temporary rules directory with a package rule that injects compat functions
        rules_dir = temp_dir / "rules"
        rules_dir.mkdir()
        packages_dir = rules_dir / "packages"
        packages_dir.mkdir()

        # Copy generic.yaml from the real rules directory
        from mogrix.cli import RULES_DIR
        import shutil
        shutil.copy(RULES_DIR / "generic.yaml", rules_dir / "generic.yaml")

        # Create package rule that requests compat functions
        (packages_dir / "testpkg.yaml").write_text("""
package: testpkg
rules:
  inject_compat_functions:
    - strdup
    - strndup
""")

        # Mock SRPMExtractor to return our extracted directory
        with patch("mogrix.parser.srpm.SRPMExtractor") as mock_extractor_class:
            mock_extractor = MagicMock()
            mock_extractor.extract_spec.return_value = (extracted_dir, spec_path)
            mock_extractor_class.return_value = mock_extractor

            # Mock SRPMEmitter to avoid actually building
            with patch("mogrix.emitter.srpm.SRPMEmitter") as mock_emitter_class:
                mock_emitter = MagicMock()
                mock_emitter.emit_srpm.return_value = output_dir / "testpkg-1.0.0-1.src.rpm"
                mock_emitter_class.return_value = mock_emitter

                result = runner.invoke(main, [
                    "convert", str(fake_srpm),
                    "-o", str(output_dir),
                    "--rules-dir", str(rules_dir)
                ])

                # Should succeed
                assert result.exit_code == 0, f"Failed with: {result.output}"

                # Check that SRPMEmitter.emit_srpm was called with sources that include compat files
                mock_emitter.emit_srpm.assert_called_once()
                call_kwargs = mock_emitter.emit_srpm.call_args.kwargs
                sources = call_kwargs.get("sources", [])
                source_names = [s.name for s in sources]

                # Should have strdup.c and strndup.c in the sources
                assert "strdup.c" in source_names, f"strdup.c not in sources: {source_names}"
                assert "strndup.c" in source_names, f"strndup.c not in sources: {source_names}"

                # Also check files were copied to output directory
                assert (output_dir / "strdup.c").exists(), "strdup.c not copied to output dir"
                assert (output_dir / "strndup.c").exists(), "strndup.c not copied to output dir"
