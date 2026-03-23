# pyright: basic
"""Shared text transform primitives.

Extracted from crate_patcher.py so both `mogrix patch-crates` and
`mogrix transform` share the same validated engine.  All functions
operate on a source directory with file-relative paths.
"""

from __future__ import annotations

import glob
import logging
import os
import re
from dataclasses import dataclass, field

log = logging.getLogger(__name__)


class PatchError:
    """A validation failure from a text replacement."""

    def __init__(
        self,
        context: str,
        file: str,
        pattern: str,
        expected: int,
        found: int,
        rule_file: str = "",
    ):
        self.context = context
        self.file = file
        self.pattern = pattern[:120]
        self.expected = expected
        self.found = found
        self.rule_file = rule_file

    def __str__(self):
        return (
            f"  {self.context}: pattern not "
            f"{'found' if self.found == 0 else 'matched correctly'} "
            f"in {self.file} (expected {self.expected}, found {self.found})"
            f"{f'  [rule: {self.rule_file}]' if self.rule_file else ''}"
            f"\n    pattern: {self.pattern!r}"
        )


@dataclass
class TransformStats:
    """Accumulates counts across a transform run."""

    replacements: int = 0
    lines_removed: int = 0
    files_deleted: int = 0
    blocks_removed: int = 0
    ast_applied: int = 0
    ast_failed: int = 0
    errors: list[PatchError] = field(default_factory=list)
    files_modified: set[str] = field(default_factory=set)


def count_matches(content: str, pattern: str, is_regex: bool = False) -> int:
    """Count occurrences of a pattern in content."""
    if is_regex:
        return len(re.findall(pattern, content))
    count = 0
    start = 0
    while True:
        idx = content.find(pattern, start)
        if idx == -1:
            break
        count += 1
        start = idx + 1
    return count


def find_line_number(content: str, pattern: str) -> int | None:
    """Find the line number of the first occurrence of pattern."""
    idx = content.find(pattern)
    if idx == -1:
        return None
    return content[:idx].count("\n") + 1


def replace_with_context_check(
    content: str,
    match_str: str,
    replace_str: str,
    exclude_ctx: list[str],
    replace_all: bool,
) -> str:
    """Replace occurrences of match_str, skipping those near excluded context."""
    result = []
    pos = 0
    replaced = False

    while pos < len(content):
        idx = content.find(match_str, pos)
        if idx == -1:
            result.append(content[pos:])
            break

        ctx_window = content[max(0, idx - 200) : idx + len(match_str) + 200]
        excluded = any(exc in ctx_window for exc in exclude_ctx)

        if excluded or (replaced and not replace_all):
            result.append(content[pos : idx + len(match_str)])
        else:
            result.append(content[pos:idx])
            result.append(replace_str)
            replaced = True

        pos = idx + len(match_str)

    return "".join(result)


def apply_text_replacements(
    source_dir: str,
    replacements: list[dict],
    stats: TransformStats,
    strict: bool = False,
    context_name: str = "",
    dry_run: bool = False,
):
    """Apply text replacements to files in a source directory.

    Each replacement dict has:
      - match: literal string to find
      - replace: replacement string
      - file: specific file (relative to source_dir)
      - file_glob: glob pattern for files (e.g., "**/*.rs")
      - exclude_context: list of strings — skip if any appear within ~200 chars
      - all: if true, replace all occurrences (default: first only)
      - match_regex: regex pattern (alternative to match)
      - expected_count: how many matches to expect (default: 1 if strict, 0 if lenient)
    """
    ctx = context_name or os.path.basename(source_dir.rstrip("/"))

    for rep in replacements:
        match_str = rep.get("match", "")
        replace_str = rep.get("replace", "")
        target_file = rep.get("file")
        file_glob = rep.get("file_glob", "**/*")
        exclude_ctx = rep.get("exclude_context", [])
        replace_all = rep.get("all", False)
        regex_pattern = rep.get("match_regex")
        expected_count = rep.get("expected_count", 1 if strict else 0)

        if target_file:
            files = [os.path.join(source_dir, target_file)]
        else:
            files = glob.glob(
                os.path.join(source_dir, file_glob), recursive=True
            )

        pattern_for_search = regex_pattern or match_str
        total_matches = 0
        matched_files: list[tuple[str, str, int, str]] = []

        for fp in files:
            if not os.path.isfile(fp):
                continue
            try:
                content = open(fp).read()
            except (IOError, UnicodeDecodeError):
                continue

            if regex_pattern:
                if not re.search(regex_pattern, content):
                    continue
            else:
                if match_str not in content:
                    continue

            if exclude_ctx and not regex_pattern:
                safe_count = 0
                start = 0
                while True:
                    idx = content.find(match_str, start)
                    if idx == -1:
                        break
                    ctx_window = content[
                        max(0, idx - 200) : idx + len(match_str) + 200
                    ]
                    excluded = any(exc in ctx_window for exc in exclude_ctx)
                    if not excluded:
                        safe_count += 1
                    start = idx + 1
                if safe_count == 0:
                    continue
                n = safe_count
            else:
                n = count_matches(content, pattern_for_search, bool(regex_pattern))

            total_matches += n
            rel_path = os.path.relpath(fp, source_dir)
            matched_files.append((fp, rel_path, n, content))

        # Validation
        if expected_count > 0 and total_matches == 0:
            err = PatchError(
                context=ctx,
                file=target_file or file_glob,
                pattern=pattern_for_search,
                expected=expected_count,
                found=0,
            )
            stats.errors.append(err)
            log.warning("PATCH MISS: %s", err)
            continue

        if expected_count > 0 and total_matches != expected_count:
            err = PatchError(
                context=ctx,
                file=target_file or file_glob,
                pattern=pattern_for_search,
                expected=expected_count,
                found=total_matches,
            )
            stats.errors.append(err)
            log.warning("PATCH COUNT: %s", err)

        # Apply replacements
        for fp, rel_path, n, content in matched_files:
            if regex_pattern:
                new_content = re.sub(
                    regex_pattern,
                    replace_str,
                    content,
                    count=0 if replace_all else 1,
                )
            elif exclude_ctx:
                new_content = replace_with_context_check(
                    content, match_str, replace_str, exclude_ctx, replace_all
                )
            else:
                if replace_all:
                    new_content = content.replace(match_str, replace_str)
                else:
                    new_content = content.replace(match_str, replace_str, 1)

            if new_content != content:
                if dry_run:
                    line = find_line_number(content, pattern_for_search)
                    log.info("  [dry-run] would patch %s:%s", rel_path, line or "?")
                else:
                    open(fp, "w").write(new_content)
                    if strict:
                        line = find_line_number(content, pattern_for_search)
                        log.info("  patched %s:%s", rel_path, line or "?")
                stats.replacements += 1
                stats.files_modified.add(rel_path)


def apply_remove_lines(
    source_dir: str,
    patterns: list[dict],
    stats: TransformStats,
    dry_run: bool = False,
):
    """Remove lines matching patterns from files in a source directory.

    Each pattern dict has:
      - pattern: substring or regex to match
      - file_glob: glob pattern for files (default: **/*.*)
      - file: specific file (relative to source_dir)
      - is_regex: if true, use regex matching
    """
    for pat in patterns:
        pattern_str = pat.get("pattern", "")
        file_glob = pat.get("file_glob", "**/*")
        target_file = pat.get("file")
        is_regex = pat.get("is_regex", False)

        if target_file:
            file_list = [os.path.join(source_dir, target_file)]
        else:
            file_list = glob.glob(
                os.path.join(source_dir, file_glob), recursive=True
            )

        for fp in file_list:
            if not os.path.isfile(fp):
                continue
            try:
                content = open(fp).read()
            except (IOError, UnicodeDecodeError):
                continue

            if pattern_str not in content and not is_regex:
                continue

            lines = content.split("\n")
            if is_regex:
                compiled = re.compile(pattern_str)
                new_lines = [ln for ln in lines if not compiled.search(ln)]
            else:
                new_lines = [ln for ln in lines if pattern_str not in ln]

            removed = len(lines) - len(new_lines)
            if removed > 0:
                rel_path = os.path.relpath(fp, source_dir)
                if dry_run:
                    log.info(
                        "  [dry-run] would remove %d lines from %s",
                        removed,
                        rel_path,
                    )
                else:
                    open(fp, "w").write("\n".join(new_lines))
                stats.lines_removed += removed
                stats.files_modified.add(rel_path)
