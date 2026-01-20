"""Tests for SRPM emitter functionality."""

import tempfile
from pathlib import Path
from unittest.mock import patch, MagicMock

import pytest

from mogrix.emitter.srpm import SRPMEmitter


@pytest.fixture
def temp_rpmbuild_dir():
    """Create a temp rpmbuild directory."""
    with tempfile.TemporaryDirectory() as tmpdir:
        yield Path(tmpdir)


@pytest.fixture
def sample_spec_content():
    """Sample spec file content."""
    return """Name: test-package
Version: 1.0.0
Release: 1
Summary: Test package
License: MIT

%description
A test package.

%prep
%setup -q

%build
%configure
make

%install
make install DESTDIR=%{buildroot}

%files
%{_bindir}/*
"""


def test_srpm_emitter_setup_tree(temp_rpmbuild_dir):
    """Test that rpmbuild tree is created correctly."""
    emitter = SRPMEmitter(temp_rpmbuild_dir)
    emitter.setup_rpmbuild_tree()

    assert (temp_rpmbuild_dir / "SOURCES").exists()
    assert (temp_rpmbuild_dir / "SPECS").exists()
    assert (temp_rpmbuild_dir / "BUILD").exists()
    assert (temp_rpmbuild_dir / "RPMS").exists()
    assert (temp_rpmbuild_dir / "SRPMS").exists()


def test_srpm_emitter_writes_spec(temp_rpmbuild_dir, sample_spec_content):
    """Test that spec file is written correctly."""
    emitter = SRPMEmitter(temp_rpmbuild_dir)
    emitter.setup_rpmbuild_tree()

    spec_path = temp_rpmbuild_dir / "SPECS" / "test.spec"
    spec_path.write_text(sample_spec_content)

    assert spec_path.exists()
    assert spec_path.read_text() == sample_spec_content


def test_srpm_emitter_copies_sources(temp_rpmbuild_dir):
    """Test that source files are copied to SOURCES."""
    emitter = SRPMEmitter(temp_rpmbuild_dir)
    emitter.setup_rpmbuild_tree()

    # Create a temp source file
    with tempfile.NamedTemporaryFile(suffix=".tar.gz", delete=False) as f:
        f.write(b"fake tarball content")
        source_path = Path(f.name)

    try:
        import shutil
        sources_dir = temp_rpmbuild_dir / "SOURCES"
        shutil.copy2(source_path, sources_dir / source_path.name)

        assert (sources_dir / source_path.name).exists()
    finally:
        source_path.unlink()


@patch("subprocess.run")
def test_srpm_emitter_calls_rpmbuild(mock_run, temp_rpmbuild_dir, sample_spec_content):
    """Test that rpmbuild is called correctly."""
    mock_run.return_value = MagicMock(returncode=0, stderr="")

    # Create a fake SRPM to be "found"
    srpms_dir = temp_rpmbuild_dir / "SRPMS"
    srpms_dir.mkdir(parents=True, exist_ok=True)
    fake_srpm = srpms_dir / "test-package-1.0.0-1.src.rpm"
    fake_srpm.write_text("fake srpm")

    emitter = SRPMEmitter(temp_rpmbuild_dir)

    result = emitter.emit_srpm(
        spec_content=sample_spec_content,
        spec_name="test-package.spec",
    )

    # Verify rpmbuild was called
    mock_run.assert_called_once()
    call_args = mock_run.call_args[0][0]
    assert call_args[0] == "rpmbuild"
    assert "-bs" in call_args

    assert result == fake_srpm


@patch("subprocess.run")
def test_srpm_emitter_handles_rpmbuild_failure(mock_run, temp_rpmbuild_dir, sample_spec_content):
    """Test that rpmbuild failure is handled."""
    mock_run.return_value = MagicMock(returncode=1, stderr="Build failed")

    emitter = SRPMEmitter(temp_rpmbuild_dir)

    with pytest.raises(RuntimeError, match="rpmbuild failed"):
        emitter.emit_srpm(
            spec_content=sample_spec_content,
            spec_name="test-package.spec",
        )


def test_srpm_emitter_default_rpmbuild_dir():
    """Test that default rpmbuild dir is ~/rpmbuild."""
    emitter = SRPMEmitter()
    assert emitter.rpmbuild_dir == Path.home() / "rpmbuild"
