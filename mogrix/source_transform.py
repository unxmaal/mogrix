# pyright: basic
"""General-purpose declarative source transformation.

Reads a YAML rule file and applies transforms to a source tree.
Rule files are self-contained: they name the project, declare the
source directory, and list all transforms.

Usage:
    mogrix transform <rules.yaml> [--source DIR] [--dry-run] [--check-only]
"""

from __future__ import annotations

import glob
import logging
import os
import re
import subprocess

import yaml

from mogrix.text_transforms import (
    PatchError,
    TransformStats,
    apply_remove_lines,
    apply_text_replacements,
    count_matches,
)

log = logging.getLogger(__name__)


def _resolve_rules_path(rules_arg: str) -> str:
    """Resolve a rules argument to an actual file path.

    Accepts:
      - An absolute or relative path to a .yaml file
      - A bare name that resolves to rules/transforms/<name>.yaml
    """
    # Direct path
    if os.path.isfile(rules_arg):
        return os.path.abspath(rules_arg)

    # Try with .yaml extension
    if os.path.isfile(rules_arg + ".yaml"):
        return os.path.abspath(rules_arg + ".yaml")

    # Try rules/transforms/<name>.yaml relative to project root
    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    candidate = os.path.join(project_root, "rules", "transforms", rules_arg)
    if os.path.isfile(candidate):
        return candidate
    if os.path.isfile(candidate + ".yaml"):
        return candidate + ".yaml"

    raise FileNotFoundError(
        f"Rule file not found: {rules_arg}\n"
        f"Searched: {rules_arg}, {rules_arg}.yaml, "
        f"rules/transforms/{rules_arg}.yaml"
    )


def delete_files(
    source_dir: str,
    files: list[str],
    stats: TransformStats,
    dry_run: bool = False,
) -> None:
    """Delete files from the source tree."""
    for rel_path in files:
        fp = os.path.join(source_dir, rel_path)
        if os.path.exists(fp):
            if dry_run:
                log.info("  [dry-run] would delete %s", rel_path)
            else:
                os.remove(fp)
                log.info("  deleted %s", rel_path)
            stats.files_deleted += 1
            stats.files_modified.add(rel_path)
        else:
            log.debug("  skip delete (not found): %s", rel_path)


def remove_blocks(
    source_dir: str,
    blocks: list[dict],
    stats: TransformStats,
    dry_run: bool = False,
) -> None:
    """Remove multi-line blocks from files by start/end markers.

    Each block dict has:
      - file: relative path in source tree
      - start: string marking the beginning of the block
      - end: string marking the end of the block
      - expected_count: how many blocks to expect (default 1)
      - include_end: whether to include the end marker line in removal (default True)
    """
    for block in blocks:
        target_file = block["file"]
        start_marker = block["start"]
        end_marker = block["end"]
        expected = block.get("expected_count", 1)
        include_end = block.get("include_end", True)

        fp = os.path.join(source_dir, target_file)
        if not os.path.isfile(fp):
            if expected > 0:
                stats.errors.append(
                    PatchError(
                        context="remove_blocks",
                        file=target_file,
                        pattern=start_marker,
                        expected=expected,
                        found=0,
                    )
                )
            continue

        try:
            content = open(fp).read()
        except (IOError, UnicodeDecodeError):
            continue

        lines = content.split("\n")
        new_lines: list[str] = []
        in_block = False
        removed_count = 0

        for line in lines:
            if not in_block:
                if start_marker in line:
                    in_block = True
                    removed_count += 1
                    continue  # skip start line
                new_lines.append(line)
            else:
                if end_marker in line:
                    in_block = False
                    if not include_end:
                        new_lines.append(line)
                    # block complete
                else:
                    pass  # skip lines inside block

        if expected > 0 and removed_count != expected:
            stats.errors.append(
                PatchError(
                    context="remove_blocks",
                    file=target_file,
                    pattern=start_marker,
                    expected=expected,
                    found=removed_count,
                )
            )

        if removed_count > 0:
            if dry_run:
                log.info(
                    "  [dry-run] would remove %d block(s) from %s",
                    removed_count,
                    target_file,
                )
            else:
                open(fp, "w").write("\n".join(new_lines))
            stats.blocks_removed += removed_count
            stats.files_modified.add(target_file)


def check_postconditions(
    source_dir: str,
    checks: dict,
) -> tuple[bool, list[str]]:
    """Run postcondition checks against the source tree.

    Returns (all_passed, list_of_messages).
    """
    messages: list[str] = []
    passed = True

    # absent_strings: must NOT appear anywhere
    for s in checks.get("absent_strings", []):
        hits = _grep_source(source_dir, s)
        if hits:
            passed = False
            messages.append(f"FAIL absent: {s!r} found in {len(hits)} file(s):")
            for h in hits[:5]:
                messages.append(f"  {h}")
        else:
            messages.append(f"OK   absent: {s!r}")

    # present_strings: must STILL appear somewhere
    for s in checks.get("present_strings", []):
        hits = _grep_source(source_dir, s)
        if hits:
            messages.append(f"OK   present: {s!r}")
        else:
            passed = False
            messages.append(f"FAIL present: {s!r} not found anywhere")

    # build_command: run and check exit code
    build_cmd = checks.get("build_command")
    if build_cmd:
        messages.append(f"Running build: {build_cmd}")
        result = subprocess.run(
            build_cmd,
            shell=True,
            cwd=source_dir,
            capture_output=True,
            text=True,
        )
        if result.returncode == 0:
            messages.append("OK   build succeeded")
        else:
            passed = False
            messages.append(f"FAIL build failed (rc={result.returncode})")
            if result.stderr:
                for line in result.stderr.strip().split("\n")[-10:]:
                    messages.append(f"  {line}")

    return passed, messages


def _grep_source(source_dir: str, pattern: str) -> list[str]:
    """Find files containing a literal string. Returns list of relative paths."""
    hits: list[str] = []
    for root, _, files in os.walk(source_dir):
        # Skip hidden dirs and node_modules
        basename = os.path.basename(root)
        if basename.startswith(".") or basename == "node_modules":
            continue
        for fname in files:
            fp = os.path.join(root, fname)
            try:
                content = open(fp).read()
            except (IOError, UnicodeDecodeError):
                continue
            if pattern in content:
                hits.append(os.path.relpath(fp, source_dir))
    return hits


def transform_project(
    rules_arg: str,
    source_override: str | None = None,
    dry_run: bool = False,
    check_only: bool = False,
) -> tuple[TransformStats, bool]:
    """Main orchestrator: load rules and apply all transforms.

    Returns (stats, postconditions_passed).
    """
    rules_path = _resolve_rules_path(rules_arg)
    with open(rules_path) as f:
        rules = yaml.safe_load(f) or {}

    project = rules.get("project", "unknown")
    source_dir = source_override or rules.get("source_dir", ".")
    source_dir = os.path.expanduser(source_dir)

    if not os.path.isdir(source_dir):
        raise FileNotFoundError(f"Source directory not found: {source_dir}")

    log.info("Transform: project=%s source=%s rules=%s", project, source_dir, rules_path)

    stats = TransformStats()

    if check_only:
        # Only run postconditions
        postconditions = rules.get("postconditions", {})
        if postconditions:
            ok, msgs = check_postconditions(source_dir, postconditions)
            return stats, ok
        return stats, True

    # Phase 1: Delete files
    if "delete_files" in rules:
        log.info("Phase: delete_files (%d files)", len(rules["delete_files"]))
        delete_files(source_dir, rules["delete_files"], stats, dry_run)

    # Phase 2: Remove blocks (before text replacements — blocks may contain
    # text that would otherwise match replacement patterns)
    if "remove_blocks" in rules:
        log.info("Phase: remove_blocks (%d blocks)", len(rules["remove_blocks"]))
        remove_blocks(source_dir, rules["remove_blocks"], stats, dry_run)

    # Phase 3: Text replacements
    if "text_replacements" in rules:
        log.info(
            "Phase: text_replacements (%d rules)",
            len(rules["text_replacements"]),
        )
        apply_text_replacements(
            source_dir,
            rules["text_replacements"],
            stats,
            strict=True,
            context_name=project,
            dry_run=dry_run,
        )

    # Phase 4: Remove lines
    if "remove_lines" in rules:
        log.info("Phase: remove_lines (%d patterns)", len(rules["remove_lines"]))
        apply_remove_lines(source_dir, rules["remove_lines"], stats, dry_run)

    # Phase 5: Postconditions
    postconditions = rules.get("postconditions", {})
    postconditions_ok = True
    if postconditions and not dry_run:
        log.info("Phase: postconditions")
        postconditions_ok, msgs = check_postconditions(source_dir, postconditions)
        for msg in msgs:
            log.info("  %s", msg)

    return stats, postconditions_ok
