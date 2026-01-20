"""Tests for compat source injector."""

from pathlib import Path
from mogrix.compat.injector import CompatInjector

COMPAT_DIR = Path(__file__).parent.parent / "compat"


def test_injector_loads_catalog():
    """Injector loads the function catalog."""
    injector = CompatInjector(COMPAT_DIR)
    assert injector.catalog is not None
    assert "functions" in injector.catalog


def test_injector_finds_function_file():
    """Injector finds source file for a function."""
    injector = CompatInjector(COMPAT_DIR)
    path = injector.get_source_file("strdup")
    assert path is not None
    assert path.exists()
    assert path.name == "strdup.c"


def test_injector_resolves_function_set():
    """Injector resolves function set to file list."""
    injector = CompatInjector(COMPAT_DIR)
    files = injector.resolve_set("basic_string")
    assert len(files) >= 2
    assert any("strdup" in str(f) for f in files)


def test_injector_resolves_function_list():
    """Injector resolves function names to unique files."""
    injector = CompatInjector(COMPAT_DIR)
    files = injector.resolve_functions(["strdup", "strndup", "getline"])
    # strdup and strndup are separate files, getline is one file
    assert len(files) == 3


def test_injector_deduplicates_files():
    """Injector deduplicates when multiple functions share a file."""
    injector = CompatInjector(COMPAT_DIR)
    # getline and getdelim are in the same file
    files = injector.resolve_functions(["getline", "getdelim"])
    assert len(files) == 1


def test_injector_generates_source_list():
    """Injector generates SOURCE macros for spec file."""
    injector = CompatInjector(COMPAT_DIR)
    sources = injector.get_source_entries(["strdup", "getline"], start_num=100)
    assert "Source100:" in sources
    assert "strdup.c" in sources
