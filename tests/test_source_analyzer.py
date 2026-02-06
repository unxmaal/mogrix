"""Tests for source-level static analysis."""

import subprocess
import textwrap
from pathlib import Path
from unittest.mock import patch

import pytest

from mogrix.analyzers.source import (
    CatalogPattern,
    SourceAnalyzer,
    SourceCheck,
    SourceFinding,
)


@pytest.fixture
def tmp_source(tmp_path):
    """Create a temporary source tree with test C files."""
    src = tmp_path / "src"
    src.mkdir()

    # File with %zu usage
    (src / "format.c").write_text(textwrap.dedent("""\
        #include <stdio.h>
        void print_size(size_t n) {
            printf("size is %zu bytes\\n", n);
        }
    """))

    # File with __thread usage
    (src / "tls.c").write_text(textwrap.dedent("""\
        static __thread int counter = 0;
        void increment(void) { counter++; }
    """))

    # File with strdup usage
    (src / "util.c").write_text(textwrap.dedent("""\
        #include <string.h>
        char *dup(const char *s) {
            return strdup(s);
        }
    """))

    # File with fopencookie usage
    (src / "stream.c").write_text(textwrap.dedent("""\
        #include <stdio.h>
        FILE *make_stream(void) {
            return fopencookie(NULL, "r", funcs);
        }
    """))

    # Clean file (no issues)
    (src / "clean.c").write_text(textwrap.dedent("""\
        #include <stdio.h>
        int main(void) {
            printf("hello world\\n");
            return 0;
        }
    """))

    # Header file with epoll
    (src / "poller.h").write_text(textwrap.dedent("""\
        #ifndef POLLER_H
        #define POLLER_H
        int setup_epoll(void);
        int fd = epoll_create(1);
        #endif
    """))

    return src


@pytest.fixture
def source_checks_yaml(tmp_path):
    """Create a test source_checks.yaml."""
    checks = tmp_path / "source_checks.yaml"
    checks.write_text(textwrap.dedent("""\
        checks:
          - id: printf-zu
            severity: error
            pattern: '"[^"]*%z[udx]'
            glob: "*.c,*.h"
            message: "IRIX libc doesn't support %zu"
            fix: "Use %u instead"

          - id: thread-local-storage
            severity: error
            pattern: '\\b__thread\\b'
            glob: "*.c,*.h"
            message: "IRIX rld has no __thread TLS support"
            fix: "Use pthread_key_t"

          - id: epoll-usage
            severity: info
            pattern: '\\bepoll_create\\b|\\bepoll_ctl\\b|\\bepoll_wait\\b'
            glob: "*.c,*.h"
            message: "IRIX has no epoll"
            fix: "Use configure --disable-epoll"

          - id: fopencookie-usage
            severity: error
            pattern: '\\bfopencookie\\s*\\('
            glob: "*.c,*.h"
            message: "fopencookie crashes on IRIX"
            fix: "Use funopen instead"
            handled_by:
              - compat_functions:funopen
    """))
    return checks


@pytest.fixture
def catalog_yaml(tmp_path):
    """Create a test catalog.yaml."""
    catalog = tmp_path / "catalog.yaml"
    catalog.write_text(textwrap.dedent("""\
        functions:
          strdup:
            file: string/strdup.c
            header: string.h
            description: Duplicate a string
            source_patterns:
              - pattern: '\\bstrdup\\s*\\('
                note: "dlmalloc mismatch — inject compat strdup"
          funopen:
            file: stdio/funopen.c
            header: stdio.h
            description: BSD cookie I/O
            source_patterns:
              - pattern: '\\bfopencookie\\s*\\('
                note: "fopencookie crashes — use funopen"
    """))
    return catalog


def _has_rg():
    """Check if ripgrep is available."""
    try:
        subprocess.run(["rg", "--version"], capture_output=True, check=True)
        return True
    except (FileNotFoundError, subprocess.CalledProcessError):
        return False


requires_rg = pytest.mark.skipif(not _has_rg(), reason="ripgrep not installed")


@requires_rg
def test_scan_finds_zu(tmp_source, source_checks_yaml, catalog_yaml):
    """Scanner should find %zu in printf calls."""
    analyzer = SourceAnalyzer(source_checks_yaml, catalog_yaml)
    result = analyzer.scan_directory(tmp_source)

    zu_findings = [f for f in result.findings if f.check_id == "printf-zu"]
    assert len(zu_findings) >= 1
    assert any("format.c" in f.file for f in zu_findings)


@requires_rg
def test_scan_finds_thread(tmp_source, source_checks_yaml, catalog_yaml):
    """Scanner should find __thread usage."""
    analyzer = SourceAnalyzer(source_checks_yaml, catalog_yaml)
    result = analyzer.scan_directory(tmp_source)

    tls_findings = [f for f in result.findings if f.check_id == "thread-local-storage"]
    assert len(tls_findings) >= 1
    assert any("tls.c" in f.file for f in tls_findings)


@requires_rg
def test_scan_finds_epoll(tmp_source, source_checks_yaml, catalog_yaml):
    """Scanner should find epoll usage in .h files."""
    analyzer = SourceAnalyzer(source_checks_yaml, catalog_yaml)
    result = analyzer.scan_directory(tmp_source)

    epoll_findings = [f for f in result.findings if f.check_id == "epoll-usage"]
    assert len(epoll_findings) >= 1
    assert any("poller.h" in f.file for f in epoll_findings)


@requires_rg
def test_scan_finds_catalog_patterns(tmp_source, source_checks_yaml, catalog_yaml):
    """Scanner should find compat function usage from catalog.yaml."""
    analyzer = SourceAnalyzer(source_checks_yaml, catalog_yaml)
    result = analyzer.scan_directory(tmp_source)

    strdup_findings = [f for f in result.findings if f.check_id == "compat:strdup"]
    assert len(strdup_findings) >= 1
    assert any("util.c" in f.file for f in strdup_findings)


@requires_rg
def test_handled_compat_functions(tmp_source, source_checks_yaml, catalog_yaml):
    """Findings for already-injected compat functions should be marked handled."""
    analyzer = SourceAnalyzer(source_checks_yaml, catalog_yaml)
    result = analyzer.scan_directory(
        tmp_source,
        handled_compat_functions=["strdup", "funopen"],
    )

    strdup_findings = [f for f in result.findings if f.check_id == "compat:strdup"]
    assert all(f.handled for f in strdup_findings)

    # fopencookie check has handled_by: compat_functions:funopen
    fopen_findings = [f for f in result.findings if f.check_id == "fopencookie-usage"]
    assert all(f.handled for f in fopen_findings)


@requires_rg
def test_unhandled_compat_functions(tmp_source, source_checks_yaml, catalog_yaml):
    """Findings without matching compat functions should be unhandled."""
    analyzer = SourceAnalyzer(source_checks_yaml, catalog_yaml)
    result = analyzer.scan_directory(
        tmp_source,
        handled_compat_functions=[],  # nothing handled
    )

    strdup_findings = [f for f in result.findings if f.check_id == "compat:strdup"]
    assert all(not f.handled for f in strdup_findings)


@requires_rg
def test_result_properties(tmp_source, source_checks_yaml, catalog_yaml):
    """Test SourceAnalysisResult property accessors."""
    analyzer = SourceAnalyzer(source_checks_yaml, catalog_yaml)
    result = analyzer.scan_directory(tmp_source)

    assert isinstance(result.errors, list)
    assert isinstance(result.warnings, list)
    assert isinstance(result.info, list)
    assert isinstance(result.handled, list)
    assert isinstance(result.unhandled, list)

    # All unhandled by default (no handled_compat_functions passed)
    assert len(result.handled) == 0
    assert len(result.unhandled) == len(result.findings)


@requires_rg
def test_deduplication(tmp_source, source_checks_yaml, catalog_yaml):
    """Same check_id + file should be deduplicated to one finding."""
    # Add another strdup call to util.c
    (tmp_source / "util.c").write_text(textwrap.dedent("""\
        #include <string.h>
        char *dup(const char *s) { return strdup(s); }
        char *dup2(const char *s) { return strdup(s); }
    """))

    analyzer = SourceAnalyzer(source_checks_yaml, catalog_yaml)
    result = analyzer.scan_directory(tmp_source)

    # Should only have one finding for (compat:strdup, util.c)
    strdup_util = [
        f for f in result.findings
        if f.check_id == "compat:strdup" and "util.c" in f.file
    ]
    assert len(strdup_util) == 1


@requires_rg
def test_empty_source_tree(tmp_path, source_checks_yaml, catalog_yaml):
    """Empty directory should produce no findings."""
    empty = tmp_path / "empty"
    empty.mkdir()

    analyzer = SourceAnalyzer(source_checks_yaml, catalog_yaml)
    result = analyzer.scan_directory(empty)

    assert len(result.findings) == 0


def test_missing_checks_file(tmp_path):
    """Missing source_checks.yaml should not crash."""
    analyzer = SourceAnalyzer(
        source_checks_path=tmp_path / "nonexistent.yaml",
        catalog_path=tmp_path / "also_nonexistent.yaml",
    )
    assert len(analyzer.checks) == 0
    assert len(analyzer.catalog_patterns) == 0


@requires_rg
def test_scan_tarball(tmp_path, source_checks_yaml, catalog_yaml):
    """Scanner should be able to extract and scan a tarball."""
    # Create a small tarball
    src = tmp_path / "pkg-1.0"
    src.mkdir()
    (src / "main.c").write_text('printf("size: %zu\\n", n);')

    import subprocess
    subprocess.run(
        ["tar", "czf", str(tmp_path / "pkg-1.0.tar.gz"), "-C", str(tmp_path), "pkg-1.0"],
        check=True,
    )

    workdir = tmp_path / "scan"
    analyzer = SourceAnalyzer(source_checks_yaml, catalog_yaml)
    result = analyzer.scan_tarball(tmp_path / "pkg-1.0.tar.gz", workdir)

    zu_findings = [f for f in result.findings if f.check_id == "printf-zu"]
    assert len(zu_findings) >= 1
