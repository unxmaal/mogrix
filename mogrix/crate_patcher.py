"""
Rust crate patcher for IRIX cross-compilation.

Applies mogrix-style transformation rules to Rust crates in the cargo
registry, making them compile for IRIX targets. Uses the same declarative
rule approach as mogrix's RPM spec conversion.

Rules live in rules/crates/:
  - generic_rust.yaml  — applied to ALL crates
  - <crate_name>.yaml  — crate-specific overrides

Validation (safepatch semantics):
  Crate-specific rules are STRICT by default — if a pattern isn't found,
  it's an error. This catches version bumps that invalidate patterns.
  Generic rules are LENIENT — patterns that don't match are silently skipped,
  since most crates won't have the pattern.

  Per-replacement override: set expected_count in the YAML entry.
    expected_count: 1   — exactly 1 match required (crate-specific default)
    expected_count: 0   — any count OK, including zero (generic default)
    expected_count: N   — exactly N matches required

Usage:
    uv run mogrix patch-crates [--registry-path PATH]
"""

import glob
import logging
import os
import re
import shutil

import yaml

from mogrix.text_transforms import (
    PatchError as _SharedPatchError,
    TransformStats as _SharedStats,
    apply_remove_lines as _shared_apply_remove_lines,
    apply_text_replacements as _shared_apply_text_replacements,
    count_matches as _count_matches,
    find_line_number as _find_line_number,
    replace_with_context_check as _replace_with_context_check,
)

log = logging.getLogger(__name__)

REGISTRY_BASE = os.path.expanduser("~/.cargo/registry/src")


class PatchError:
    """A validation failure from a text replacement.

    Thin wrapper around the shared PatchError, preserving the crate_patcher
    interface (uses 'crate' instead of 'context').
    """

    def __init__(self, crate: str, file: str, pattern: str,
                 expected: int, found: int, rule_file: str = ""):
        self.crate = crate
        self.file = file
        self.pattern = pattern[:120]
        self.expected = expected
        self.found = found
        self.rule_file = rule_file

    def __str__(self):
        return (
            f"  {self.crate}: pattern not {'found' if self.found == 0 else 'matched correctly'} "
            f"in {self.file} (expected {self.expected}, found {self.found})"
            f"{f'  [rule: {self.rule_file}]' if self.rule_file else ''}"
            f"\n    pattern: {self.pattern!r}"
        )


def find_registry_dir() -> str | None:
    """Find the cargo registry source directory."""
    dirs = glob.glob(f"{REGISTRY_BASE}/index.crates.io-*/")
    return dirs[0] if dirs else None


def load_crate_rules(rules_dir: str) -> tuple[dict, dict[str, dict]]:
    """Load generic and crate-specific rules from rules/crates/.

    Returns (generic_rules, {crate_pattern: crate_rules})
    """
    crates_dir = os.path.join(rules_dir, "crates")
    generic = {}
    specific = {}

    # Load generic rules
    generic_path = os.path.join(crates_dir, "generic_rust.yaml")
    if os.path.exists(generic_path):
        with open(generic_path) as f:
            generic = yaml.safe_load(f) or {}

    # Load crate-specific rules
    for path in sorted(glob.glob(os.path.join(crates_dir, "*.yaml"))):
        name = os.path.basename(path).replace(".yaml", "")
        if name == "generic_rust":
            continue
        with open(path) as f:
            rules = yaml.safe_load(f) or {}
        pattern = rules.get("name_pattern", f"{name}-*")
        specific[pattern] = rules
        specific[pattern]["_source_file"] = os.path.basename(path)

    return generic, specific


def match_crate(crate_dir_name: str, pattern: str) -> bool:
    """Check if a crate directory name matches a glob pattern."""
    import fnmatch
    return fnmatch.fnmatch(crate_dir_name, pattern)


def apply_text_replacements(
    crate_dir: str,
    replacements: list[dict],
    stats: dict,
    strict: bool = False,
):
    """Apply text replacements to files in a crate directory.

    Each replacement dict has:
      - match: literal string to find
      - replace: replacement string
      - file: specific file (relative to crate dir)
      - file_glob: glob pattern for files (e.g., "**/*.rs")
      - exclude_context: list of strings — skip if any appear within 5 lines
      - all: if true, replace all occurrences (default: first only)
      - expected_count: how many matches to expect (default: 1 if strict, 0 if lenient)
        0 = any count OK (skip silently if not found)
        N = exactly N matches required (fail if wrong)

    When strict=True (crate-specific rules), missing patterns are errors.
    When strict=False (generic rules), missing patterns are silently skipped.
    Per-replacement expected_count overrides the strict default.
    """
    crate_name = os.path.basename(crate_dir.rstrip('/'))

    for rep in replacements:
        match_str = rep.get("match", "")
        replace_str = rep.get("replace", "")
        target_file = rep.get("file")
        file_glob = rep.get("file_glob", "**/*.rs")
        exclude_ctx = rep.get("exclude_context", [])
        replace_all = rep.get("all", False)
        regex_pattern = rep.get("match_regex")
        # Validation: strict rules default to expecting 1 match,
        # lenient rules default to 0 (any count OK)
        expected_count = rep.get("expected_count", 1 if strict else 0)

        if target_file:
            files = [os.path.join(crate_dir, target_file)]
        else:
            files = glob.glob(
                os.path.join(crate_dir, file_glob), recursive=True
            )

        pattern_for_search = regex_pattern or match_str
        total_matches = 0
        matched_files = []

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

            # Check exclude_context per-occurrence, not per-file.
            # Find all occurrences; keep only those without excluded context.
            if exclude_ctx and not regex_pattern:
                safe_count = 0
                start = 0
                while True:
                    idx = content.find(match_str, start)
                    if idx == -1:
                        break
                    ctx_window = content[max(0, idx - 200):idx + len(match_str) + 200]
                    excluded = any(exc in ctx_window for exc in exclude_ctx)
                    if not excluded:
                        safe_count += 1
                    start = idx + 1
                if safe_count == 0:
                    continue
                n = safe_count
            else:
                n = _count_matches(content, pattern_for_search, bool(regex_pattern))

            total_matches += n
            rel_path = os.path.relpath(fp, crate_dir)
            matched_files.append((fp, rel_path, n, content))

        # Validation
        if expected_count > 0 and total_matches == 0:
            # Pattern not found — error for strict rules
            err = PatchError(
                crate=crate_name,
                file=target_file or file_glob,
                pattern=pattern_for_search,
                expected=expected_count,
                found=0,
            )
            stats["errors"].append(err)
            log.warning("PATCH MISS: %s", err)
            continue

        if expected_count > 0 and total_matches != expected_count:
            # Wrong count
            err = PatchError(
                crate=crate_name,
                file=target_file or file_glob,
                pattern=pattern_for_search,
                expected=expected_count,
                found=total_matches,
            )
            stats["errors"].append(err)
            log.warning("PATCH COUNT: %s", err)
            # Still apply — wrong count is a warning, not a skip
            # (safepatch exits on wrong count, but for crates we want to
            # attempt the fix and report the discrepancy)

        # Apply replacements
        for fp, rel_path, n, content in matched_files:
            if regex_pattern:
                new_content = re.sub(
                    regex_pattern, replace_str, content,
                    count=0 if replace_all else 1
                )
            elif exclude_ctx:
                # Context-aware replacement: skip occurrences near excluded text
                new_content = _replace_with_context_check(
                    content, match_str, replace_str, exclude_ctx,
                    replace_all,
                )
            else:
                if replace_all:
                    new_content = content.replace(match_str, replace_str)
                else:
                    new_content = content.replace(match_str, replace_str, 1)

            if new_content != content:
                open(fp, 'w').write(new_content)
                stats["replacements"] += 1
                if strict:
                    line = _find_line_number(content, pattern_for_search)
                    log.info("  patched %s:%s", rel_path, line or "?")


def apply_remove_lines(
    crate_dir: str,
    patterns: list[dict],
    stats: dict,
):
    """Remove lines matching patterns from files in a crate directory.

    Each pattern dict has:
      - pattern: substring or regex to match
      - file_glob: glob pattern for files (default: **/*.rs)
      - is_regex: if true, use regex matching
    """
    for pat in patterns:
        pattern_str = pat.get("pattern", "")
        file_glob = pat.get("file_glob", "**/*.rs")
        is_regex = pat.get("is_regex", False)

        for fp in glob.glob(
            os.path.join(crate_dir, file_glob), recursive=True
        ):
            if not os.path.isfile(fp):
                continue
            try:
                content = open(fp).read()
            except (IOError, UnicodeDecodeError):
                continue

            if pattern_str not in content and not is_regex:
                continue

            lines = content.split('\n')
            if is_regex:
                compiled = re.compile(pattern_str)
                new_lines = [l for l in lines if not compiled.search(l)]
            else:
                new_lines = [l for l in lines if pattern_str not in l]

            if len(new_lines) != len(lines):
                open(fp, 'w').write('\n'.join(new_lines))
                stats["lines_removed"] += len(lines) - len(new_lines)


def apply_add_source(
    crate_dir: str,
    sources: list[dict],
    project_dir: str,
    stats: dict,
):
    """Copy source files into a crate directory.

    Each source dict has:
      - src: source path relative to project_dir
      - dst: destination path relative to crate dir
      - src_dir: if true, copy entire directory
    """
    for src_spec in sources:
        src = os.path.join(project_dir, src_spec["src"])
        dst = os.path.join(crate_dir, src_spec["dst"])

        if src_spec.get("src_dir"):
            os.makedirs(dst, exist_ok=True)
            if os.path.isdir(src):
                for item in os.listdir(src):
                    s = os.path.join(src, item)
                    d = os.path.join(dst, item)
                    if os.path.isfile(s):
                        shutil.copy2(s, d)
        else:
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            if os.path.isfile(src):
                shutil.copy2(src, dst)

        stats["sources_added"] += 1


def apply_create_file(
    crate_dir: str,
    files: list[dict],
    stats: dict,
):
    """Create files in the crate directory.

    Each file dict has:
      - path: relative path in the crate
      - content: file content
    """
    for file_spec in files:
        fp = os.path.join(crate_dir, file_spec["path"])
        os.makedirs(os.path.dirname(fp), exist_ok=True)
        if not os.path.exists(fp):
            open(fp, 'w').write(file_spec["content"])
            stats["files_created"] += 1


def apply_fix_bare_irix_lines(crate_dir: str, stats: dict):
    """Fix bare 'target_os = "irix"' lines by merging into preceding cfg.

    This handles broken upstream IRIX support where someone added
    target_os = "irix" lines without proper #[cfg(any(...))] wrapping.
    """
    for root, _, files in os.walk(crate_dir):
        for fname in files:
            if not fname.endswith('.rs'):
                continue
            fp = os.path.join(root, fname)
            try:
                content = open(fp).read()
            except (IOError, UnicodeDecodeError):
                continue
            if 'target_os = "irix"' not in content:
                continue

            lines = content.split('\n')
            new_lines = []
            changed = False

            for line in lines:
                stripped = line.strip()
                if stripped in ('target_os = "irix",', 'target_os = "irix"'):
                    if new_lines:
                        prev = new_lines[-1].rstrip()
                        if prev.endswith('))]'):
                            new_lines[-1] = prev[:-3] + ', target_os = "irix"))]'
                            changed = True
                            continue
                        elif prev.endswith(')]') and '#[cfg' in prev:
                            if 'not(' in prev:
                                new_lines[-1] = (
                                    prev.replace('not(', 'not(any(')[:-2]
                                    + ', target_os = "irix")))]'
                                )
                            else:
                                new_lines[-1] = prev[:-2] + ', target_os = "irix")]'
                            changed = True
                            continue
                new_lines.append(line)

            if changed:
                open(fp, 'w').write('\n'.join(new_lines))
                stats["bare_lines_fixed"] += 1


def apply_fix_malformed_not(crate_dir: str, stats: dict):
    """Fix not(target_os = "a", target_os = "b") → not(any(a, b))."""
    for root, _, files in os.walk(crate_dir):
        for fname in files:
            if not fname.endswith('.rs'):
                continue
            fp = os.path.join(root, fname)
            try:
                content = open(fp).read()
            except (IOError, UnicodeDecodeError):
                continue
            if 'not(target_os' not in content:
                continue
            new = re.sub(
                r'not\((target_os = "[^"]+",\s*target_os = "[^"]+")\)',
                r'not(any(\1))',
                content,
            )
            if new != content:
                open(fp, 'w').write(new)
                stats["malformed_not_fixed"] += 1


def patch_crate(
    crate_dir: str,
    generic_rules: dict,
    crate_rules: dict | None,
    project_dir: str,
    stats: dict,
):
    """Apply all rules to a single crate directory.

    Crate-specific rules run in STRICT mode (patterns must match).
    Generic rules run in LENIENT mode (missing patterns silently skipped).
    """
    crate_name = os.path.basename(crate_dir.rstrip('/'))

    # Apply CRATE-SPECIFIC rules FIRST (they match original text patterns)
    if crate_rules:
        if "remove_lines" in crate_rules:
            apply_remove_lines(crate_dir, crate_rules["remove_lines"], stats)
        if "add_source" in crate_rules:
            apply_add_source(
                crate_dir, crate_rules["add_source"], project_dir, stats
            )
        if "create_file" in crate_rules:
            apply_create_file(crate_dir, crate_rules["create_file"], stats)
        if "text_replacements" in crate_rules:
            apply_text_replacements(
                crate_dir, crate_rules["text_replacements"], stats,
                strict=True,
            )

    # Then apply GENERIC rules (catch-all patterns) — unless skip_generic
    skip_generic = crate_rules.get("skip_generic", False) if crate_rules else False
    if not skip_generic:
        if "text_replacements" in generic_rules:
            apply_text_replacements(
                crate_dir, generic_rules["text_replacements"], stats,
                strict=False,
            )
        if "remove_lines" in generic_rules:
            apply_remove_lines(crate_dir, generic_rules["remove_lines"], stats)

    # Always run structural fixers
    if generic_rules.get("fix_bare_irix_lines", True):
        apply_fix_bare_irix_lines(crate_dir, stats)
    if generic_rules.get("fix_malformed_not", True):
        apply_fix_malformed_not(crate_dir, stats)


def patch_all_crates(
    registry_dir: str | None = None,
    rules_dir: str | None = None,
    project_dir: str | None = None,
) -> dict:
    """Main entry point: patch all crates in the cargo registry.

    Returns stats dict with counts of changes made.
    """
    if registry_dir is None:
        registry_dir = find_registry_dir()
    if registry_dir is None:
        log.error("Cargo registry not found")
        return {"error": "registry not found"}

    if rules_dir is None:
        rules_dir = os.path.join(
            os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
            "rules",
        )

    if project_dir is None:
        # Default: mogrix project root (libc modules, compat sources, etc.
        # are stored directly in mogrix under patches/crates/, compat/rust/)
        project_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

    generic_rules, specific_rules = load_crate_rules(rules_dir)

    stats = {
        "crates_processed": 0,
        "crates_patched": 0,
        "replacements": 0,
        "lines_removed": 0,
        "sources_added": 0,
        "files_created": 0,
        "bare_lines_fixed": 0,
        "malformed_not_fixed": 0,
        "errors": [],
    }

    log.info("Registry: %s", registry_dir)
    log.info("Rules: %s/crates/", rules_dir)
    log.info("Project: %s", project_dir)
    log.info(
        "Loaded %d crate-specific rules, %d generic replacements",
        len(specific_rules),
        len(generic_rules.get("text_replacements", [])),
    )

    for crate_dir in sorted(glob.glob(f"{registry_dir}/*/")):
        crate_name = os.path.basename(crate_dir.rstrip('/'))
        stats["crates_processed"] += 1

        # Find matching crate-specific rules
        matched_rules = None
        for pattern, rules in specific_rules.items():
            if match_crate(crate_name, pattern):
                matched_rules = rules
                break

        before_replacements = stats["replacements"]
        before_errors = len(stats["errors"])
        patch_crate(crate_dir, generic_rules, matched_rules, project_dir, stats)

        changed = stats["replacements"] != before_replacements
        if changed:
            stats["crates_patched"] += 1
            rule_name = (
                matched_rules.get("_source_file", "generic")
                if matched_rules else "generic"
            )
            log.info("  [%s] (rule: %s)", crate_name, rule_name)

        # Log errors immediately for this crate
        new_errors = stats["errors"][before_errors:]
        for err in new_errors:
            err.rule_file = (
                matched_rules.get("_source_file", "")
                if matched_rules else ""
            )

    return stats
