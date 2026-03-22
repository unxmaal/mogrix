# pyright: basic
"""Round-trip fidelity test for the AST parser.

Parses every spec file extracted from SRPMs in ~/mogrix_inputs/SRPMS/,
emits them back through the AST emitter, and diffs against the original.
Any difference is a parser or emitter bug.

This test is skipped if the SRPM directory doesn't exist (CI environments).
Run on the build VM with: pytest tests/test_roundtrip_fidelity.py -v
"""

from __future__ import annotations

import glob
import os
import subprocess
import tempfile

import pytest

from mogrix.emitter.ast_emitter import ASTEmitter
from mogrix.parser.spec_ast import SpecASTParser

SRPMS_DIR = os.path.expanduser("~/mogrix_inputs/SRPMS")


def _extract_spec_from_srpm(srpm_path: str, tmpdir: str) -> str | None:
    """Extract the .spec file from an SRPM. Returns path or None."""
    try:
        # rpm2cpio + cpio to extract just the .spec
        result = subprocess.run(
            f"rpm2cpio {srpm_path} | cpio -idm --quiet '*.spec' 2>/dev/null",
            shell=True,
            cwd=tmpdir,
            capture_output=True,
            timeout=30,
        )
        if result.returncode != 0:
            return None
        specs = glob.glob(os.path.join(tmpdir, "*.spec"))
        return specs[0] if specs else None
    except (subprocess.TimeoutExpired, FileNotFoundError):
        return None


def _collect_srpms() -> list[str]:
    """Collect all SRPM paths."""
    if not os.path.isdir(SRPMS_DIR):
        return []
    return sorted(glob.glob(os.path.join(SRPMS_DIR, "*.src.rpm")))


srpms = _collect_srpms()


@pytest.mark.skipif(
    not srpms,
    reason=f"No SRPMs found in {SRPMS_DIR}",
)
@pytest.mark.parametrize(
    "srpm_path",
    srpms,
    ids=[os.path.basename(s).split("-")[0] for s in srpms],
)
def test_roundtrip_fidelity(srpm_path: str):
    """Parse a real spec file and verify round-trip emission is identical."""
    with tempfile.TemporaryDirectory() as tmpdir:
        spec_path = _extract_spec_from_srpm(srpm_path, tmpdir)
        if spec_path is None:
            pytest.skip(f"Could not extract spec from {os.path.basename(srpm_path)}")

        original = open(spec_path).read()

        # Parse into AST
        parser = SpecASTParser()
        ast = parser.parse(original)

        # Emit back from AST
        emitter = ASTEmitter()
        emitted = emitter.emit(ast)

        # Compare
        if emitted != original:
            # Find first difference for a useful error message
            orig_lines = original.split("\n")
            emit_lines = emitted.split("\n")
            for i, (o, e) in enumerate(zip(orig_lines, emit_lines), 1):
                if o != e:
                    pytest.fail(
                        f"Round-trip mismatch at line {i} in "
                        f"{os.path.basename(spec_path)}:\n"
                        f"  original: {o!r}\n"
                        f"  emitted:  {e!r}"
                    )
            # Different lengths
            if len(orig_lines) != len(emit_lines):
                pytest.fail(
                    f"Round-trip line count mismatch in "
                    f"{os.path.basename(spec_path)}: "
                    f"original={len(orig_lines)}, emitted={len(emit_lines)}"
                )
