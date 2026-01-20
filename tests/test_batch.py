"""Tests for batch conversion functionality."""

import tempfile
from pathlib import Path
from unittest.mock import patch, MagicMock

import pytest

from mogrix.batch import BatchConverter


@pytest.fixture
def sample_spec_content():
    """Sample spec file content for testing."""
    return """Name: test-package
Version: 1.0.0
Release: 1
Summary: Test package
License: MIT
BuildRequires: gcc

%description
A test package.

%prep
%autosetup

%build
%configure
make
"""


@pytest.fixture
def temp_srpms_dir(sample_spec_content):
    """Create a temp directory with fake SRPM files."""
    with tempfile.TemporaryDirectory() as tmpdir:
        srpms_path = Path(tmpdir)

        # Create fake SRPM files (just need the right extension)
        (srpms_path / "zlib-1.2.13-1.fc40.src.rpm").write_bytes(b"fake srpm")
        (srpms_path / "curl-8.0.0-1.fc40.src.rpm").write_bytes(b"fake srpm")

        yield srpms_path


@pytest.fixture
def temp_output_dir():
    """Create a temp output directory."""
    with tempfile.TemporaryDirectory() as tmpdir:
        yield Path(tmpdir)


def test_batch_discovers_srpms(temp_srpms_dir):
    """Test that batch converter finds all SRPM files."""
    converter = BatchConverter(temp_srpms_dir)
    srpms = converter.discover_srpms()

    assert len(srpms) == 2
    names = [s.name for s in srpms]
    assert "curl-8.0.0-1.fc40.src.rpm" in names
    assert "zlib-1.2.13-1.fc40.src.rpm" in names


def test_batch_converts_all(temp_srpms_dir, temp_output_dir, sample_spec_content):
    """Test batch conversion of multiple SRPMs."""
    converter = BatchConverter(temp_srpms_dir)

    # Create extracted directories for each SRPM
    with tempfile.TemporaryDirectory() as extract_base:
        extract_base_path = Path(extract_base)

        def mock_extractor_factory(srpm_path):
            """Create a mock extractor for each SRPM."""
            # Determine package name from SRPM filename
            pkg_name = srpm_path.name.rsplit("-", 2)[0]

            # Create extracted directory for this package
            extracted_dir = extract_base_path / pkg_name
            extracted_dir.mkdir(parents=True, exist_ok=True)

            # Create spec file
            spec_content = sample_spec_content.replace("test-package", pkg_name)
            spec_path = extracted_dir / f"{pkg_name}.spec"
            spec_path.write_text(spec_content)

            # Create fake source tarball
            (extracted_dir / f"{pkg_name}-1.0.0.tar.gz").write_bytes(b"fake tarball")

            mock = MagicMock()
            mock.extract_spec.return_value = (extracted_dir, spec_path)
            return mock

        with patch("mogrix.batch.SRPMExtractor", side_effect=mock_extractor_factory):
            with patch("mogrix.batch.SRPMEmitter") as mock_emitter_class:
                mock_emitter = MagicMock()
                mock_emitter.emit_srpm.side_effect = lambda **kwargs: kwargs["output_dir"] / "converted.src.rpm"
                mock_emitter_class.return_value = mock_emitter

                results = converter.convert_all(temp_output_dir)

                assert len(results) == 2
                # Check package directories were created
                assert (temp_output_dir / "curl").exists()
                assert (temp_output_dir / "zlib").exists()


def test_batch_reports_results(temp_srpms_dir, temp_output_dir, sample_spec_content):
    """Test that batch conversion returns result info."""
    converter = BatchConverter(temp_srpms_dir)

    with tempfile.TemporaryDirectory() as extract_base:
        extract_base_path = Path(extract_base)

        def mock_extractor_factory(srpm_path):
            pkg_name = srpm_path.name.rsplit("-", 2)[0]
            extracted_dir = extract_base_path / pkg_name
            extracted_dir.mkdir(parents=True, exist_ok=True)

            spec_content = sample_spec_content.replace("test-package", pkg_name)
            spec_path = extracted_dir / f"{pkg_name}.spec"
            spec_path.write_text(spec_content)
            (extracted_dir / f"{pkg_name}-1.0.0.tar.gz").write_bytes(b"fake tarball")

            mock = MagicMock()
            mock.extract_spec.return_value = (extracted_dir, spec_path)
            return mock

        with patch("mogrix.batch.SRPMExtractor", side_effect=mock_extractor_factory):
            with patch("mogrix.batch.SRPMEmitter") as mock_emitter_class:
                mock_emitter = MagicMock()
                mock_emitter.emit_srpm.side_effect = lambda **kwargs: kwargs["output_dir"] / "converted.src.rpm"
                mock_emitter_class.return_value = mock_emitter

                results = converter.convert_all(temp_output_dir)

                for result in results:
                    assert "package" in result
                    assert "status" in result
                    assert result["status"] in ["success", "error"]
                    if result["status"] == "success":
                        assert "output_srpm" in result
                        assert "applied_rules" in result


def test_batch_summary(temp_srpms_dir, temp_output_dir, sample_spec_content):
    """Test batch conversion summary generation."""
    converter = BatchConverter(temp_srpms_dir)

    with tempfile.TemporaryDirectory() as extract_base:
        extract_base_path = Path(extract_base)

        def mock_extractor_factory(srpm_path):
            pkg_name = srpm_path.name.rsplit("-", 2)[0]
            extracted_dir = extract_base_path / pkg_name
            extracted_dir.mkdir(parents=True, exist_ok=True)

            spec_content = sample_spec_content.replace("test-package", pkg_name)
            spec_path = extracted_dir / f"{pkg_name}.spec"
            spec_path.write_text(spec_content)
            (extracted_dir / f"{pkg_name}-1.0.0.tar.gz").write_bytes(b"fake tarball")

            mock = MagicMock()
            mock.extract_spec.return_value = (extracted_dir, spec_path)
            return mock

        with patch("mogrix.batch.SRPMExtractor", side_effect=mock_extractor_factory):
            with patch("mogrix.batch.SRPMEmitter") as mock_emitter_class:
                mock_emitter = MagicMock()
                mock_emitter.emit_srpm.side_effect = lambda **kwargs: kwargs["output_dir"] / "converted.src.rpm"
                mock_emitter_class.return_value = mock_emitter

                results = converter.convert_all(temp_output_dir)
                summary = converter.get_summary(results)

                assert summary["total"] == 2
                assert summary["success"] == 2
                assert summary["errors"] == 0
                assert "curl" in summary["success_packages"]
                assert "zlib" in summary["success_packages"]
