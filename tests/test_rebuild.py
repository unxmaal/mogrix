"""Tests for rebuild workspace management and resume logic."""

import json
from pathlib import Path

import pytest

from mogrix.rebuild import (
    WORKSPACE_PREFIX,
    create_workspace,
    find_latest_workspace,
    load_completed,
)


@pytest.fixture
def home_dir(tmp_path, monkeypatch):
    """Redirect Path.home() to a temp directory."""
    monkeypatch.setattr(Path, "home", lambda: tmp_path)
    return tmp_path


def _make_workspace(home: Path, version: int) -> Path:
    """Create a workspace directory with standard subdirs."""
    ws = home / f"{WORKSPACE_PREFIX}{version}"
    ws.mkdir()
    (ws / "gate-results").mkdir()
    (ws / "mogrix_outputs" / "SRPMS").mkdir(parents=True)
    (ws / "mogrix_outputs" / "RPMS").mkdir(parents=True)
    (ws / "rpmbuild").mkdir()
    return ws


def _make_gate_result(ws: Path, package: str, rpms: list[str], success: bool = True) -> None:
    """Write a gate-result JSON for a package."""
    pkg_dir = ws / "gate-results" / package
    pkg_dir.mkdir(parents=True, exist_ok=True)
    (pkg_dir / "build-gate.json").write_text(json.dumps({
        "timestamp": "2026-03-11T12:00:00",
        "package": package,
        "build_success": success,
        "gate2_passed": success,
        "duration_s": 10.0,
        "rpms": rpms,
        "error": "" if success else "build failed",
    }))


def _make_rpm(ws: Path, rpm_name: str) -> None:
    """Create a dummy RPM file in the workspace outputs."""
    rpm_path = ws / "mogrix_outputs" / "RPMS" / rpm_name
    rpm_path.write_text("dummy")


class TestFindLatestWorkspace:
    def test_no_workspaces(self, home_dir):
        assert find_latest_workspace() is None

    def test_single_workspace(self, home_dir):
        ws = _make_workspace(home_dir, 10)
        assert find_latest_workspace() == ws

    def test_multiple_workspaces_returns_highest(self, home_dir):
        _make_workspace(home_dir, 10)
        _make_workspace(home_dir, 11)
        ws12 = _make_workspace(home_dir, 12)
        assert find_latest_workspace() == ws12

    def test_ignores_non_numeric_suffixes(self, home_dir):
        ws10 = _make_workspace(home_dir, 10)
        # Create a bogus directory with non-numeric suffix
        (home_dir / f"{WORKSPACE_PREFIX}foo").mkdir()
        assert find_latest_workspace() == ws10


class TestCreateWorkspace:
    def test_first_workspace_starts_at_11(self, home_dir):
        ws = create_workspace()
        assert ws.name == f"{WORKSPACE_PREFIX}11"
        assert ws.exists()
        assert (ws / "gate-results").exists()
        assert (ws / "mogrix_outputs" / "RPMS").exists()
        assert (ws / "rpmbuild").exists()

    def test_increments_from_existing(self, home_dir):
        _make_workspace(home_dir, 10)
        ws = create_workspace()
        assert ws.name == f"{WORKSPACE_PREFIX}11"

    def test_increments_from_highest(self, home_dir):
        _make_workspace(home_dir, 10)
        _make_workspace(home_dir, 15)
        ws = create_workspace()
        assert ws.name == f"{WORKSPACE_PREFIX}16"


class TestLoadCompleted:
    def test_empty_gate_results(self, home_dir):
        ws = _make_workspace(home_dir, 10)
        result = load_completed(ws / "gate-results", ws / "mogrix_outputs" / "RPMS")
        assert result == set()

    def test_completed_with_rpms(self, home_dir):
        ws = _make_workspace(home_dir, 10)
        _make_gate_result(ws, "bash", ["bash-5.2-1.mips.rpm"])
        _make_rpm(ws, "bash-5.2-1.mips.rpm")

        result = load_completed(ws / "gate-results", ws / "mogrix_outputs" / "RPMS")
        assert result == {"bash"}

    def test_stale_gate_result_without_rpm_excluded(self, home_dir):
        ws = _make_workspace(home_dir, 10)
        # Gate says passed but RPM doesn't exist
        _make_gate_result(ws, "bash", ["bash-5.2-1.mips.rpm"])

        result = load_completed(ws / "gate-results", ws / "mogrix_outputs" / "RPMS")
        assert result == set()

    def test_failed_gate_result_excluded(self, home_dir):
        ws = _make_workspace(home_dir, 10)
        _make_gate_result(ws, "bash", ["bash-5.2-1.mips.rpm"], success=False)
        _make_rpm(ws, "bash-5.2-1.mips.rpm")

        result = load_completed(ws / "gate-results", ws / "mogrix_outputs" / "RPMS")
        assert result == set()

    def test_mixed_completed_and_stale(self, home_dir):
        ws = _make_workspace(home_dir, 10)
        # bash: gate + RPM = completed
        _make_gate_result(ws, "bash", ["bash-5.2-1.mips.rpm"])
        _make_rpm(ws, "bash-5.2-1.mips.rpm")
        # zlib: gate but no RPM = stale
        _make_gate_result(ws, "zlib", ["zlib-1.3-1.mips.rpm"])
        # ncurses: gate + RPM = completed
        _make_gate_result(ws, "ncurses", ["ncurses-6.4-1.mips.rpm"])
        _make_rpm(ws, "ncurses-6.4-1.mips.rpm")

        result = load_completed(ws / "gate-results", ws / "mogrix_outputs" / "RPMS")
        assert result == {"bash", "ncurses"}

    def test_nonexistent_gate_dir(self, home_dir):
        result = load_completed(
            home_dir / "nonexistent" / "gate-results",
            home_dir / "nonexistent" / "RPMS",
        )
        assert result == set()
