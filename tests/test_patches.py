"""Tests for patch catalog functionality."""

import pytest
from pathlib import Path
import tempfile

from mogrix.patches.catalog import PatchCatalog


@pytest.fixture
def temp_patches_dir():
    """Create a temporary patches directory with test patches."""
    with tempfile.TemporaryDirectory() as tmpdir:
        patches_path = Path(tmpdir)

        # Create generic patches
        generic_dir = patches_path / "generic"
        generic_dir.mkdir()
        (generic_dir / "fix-time-include.patch").write_text(
            "--- a/src/main.c\n+++ b/src/main.c\n@@ -1 +1,2 @@\n+#include <time.h>\n #include <sys/time.h>\n"
        )

        # Create package-specific patches
        pkg_dir = patches_path / "packages" / "openssl"
        pkg_dir.mkdir(parents=True)
        (pkg_dir / "irix-compat.patch").write_text(
            "--- a/crypto/rand.c\n+++ b/crypto/rand.c\n@@ -1 +1 @@\n-/* linux */\n+/* irix */\n"
        )
        (pkg_dir / "no-asm.patch").write_text(
            "--- a/Configure\n+++ b/Configure\n@@ -1 +1 @@\n-asm\n+no-asm\n"
        )

        # Create versioned package patches
        ver_dir = patches_path / "packages" / "gnutls" / "3.8"
        ver_dir.mkdir(parents=True)
        (ver_dir / "sgifixes01.patch").write_text("patch content")

        yield patches_path


def test_list_generic_patches(temp_patches_dir):
    """Test listing generic patches."""
    catalog = PatchCatalog(temp_patches_dir)
    patches = catalog.list_generic()

    assert len(patches) == 1
    assert "fix-time-include.patch" in [p.name for p in patches]


def test_list_package_patches(temp_patches_dir):
    """Test listing patches for a specific package."""
    catalog = PatchCatalog(temp_patches_dir)
    patches = catalog.list_package("openssl")

    assert len(patches) == 2
    names = [p.name for p in patches]
    assert "irix-compat.patch" in names
    assert "no-asm.patch" in names


def test_list_versioned_package_patches(temp_patches_dir):
    """Test listing patches for a specific package version."""
    catalog = PatchCatalog(temp_patches_dir)
    patches = catalog.list_package("gnutls", version="3.8")

    assert len(patches) == 1
    assert patches[0].name == "sgifixes01.patch"


def test_missing_package_returns_empty(temp_patches_dir):
    """Test that missing package returns empty list."""
    catalog = PatchCatalog(temp_patches_dir)
    patches = catalog.list_package("nonexistent")

    assert patches == []


def test_get_patch_content(temp_patches_dir):
    """Test reading patch content."""
    catalog = PatchCatalog(temp_patches_dir)
    content = catalog.get_content("packages/openssl/irix-compat.patch")

    assert "irix" in content


def test_get_spec_entries(temp_patches_dir):
    """Test generating spec file patch entries."""
    catalog = PatchCatalog(temp_patches_dir)
    patches = catalog.list_package("openssl")
    entries = catalog.get_spec_entries(patches, start_num=100)

    assert "Patch100:" in entries
    assert "Patch101:" in entries


def test_get_prep_commands(temp_patches_dir):
    """Test generating %prep patch commands."""
    catalog = PatchCatalog(temp_patches_dir)
    patches = catalog.list_package("openssl")
    commands = catalog.get_prep_commands(patches, start_num=100)

    assert "%patch100 -p1" in commands or "%patch -P100 -p1" in commands
