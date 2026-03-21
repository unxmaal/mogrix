#!/usr/bin/env python3
"""Remove redundant rules that duplicate generic.yaml entries.

Usage:
    python tools/promote-existing.py [--dry-run]
    uv run python tools/promote-existing.py [--dry-run]

Reports and removes entries in rules/packages/*.yaml that are already
covered by rules/generic.yaml.  After running, `mogrix promote-check`
should report 0 redundant entries.

Implementation note
-------------------
We intentionally do NOT round-trip through yaml.dump because that
would strip all inline comments and reformat the file.  Instead we
load the YAML once (to know WHAT to remove), then edit the raw text
with targeted line removal.

For each section key (drop_buildrequires, drop_requires, etc.) we
scan the raw file for list-item lines (lines matching ``  - <value>``)
that appear inside that section and remove only those lines.  Comments
and formatting of every other line are left untouched.
"""

import re
import sys
from pathlib import Path

import yaml


# ---------------------------------------------------------------------------
# helpers
# ---------------------------------------------------------------------------


def _load_generic_sets(rules_dir: Path) -> dict[str, set]:
    """Return sets of values that generic.yaml already covers."""
    generic_path = rules_dir / "generic.yaml"
    with open(generic_path) as f:
        generic_data = yaml.safe_load(f) or {}
    generic = generic_data.get("generic", {})
    return {
        "drop_buildrequires": set(generic.get("drop_buildrequires", [])),
        "drop_requires": set(generic.get("drop_requires", [])),
        "remove_lines": set(generic.get("remove_lines", [])),
        "configure_disable": set(generic.get("configure_disable", [])),
    }


def _find_redundant(yaml_path: Path, generic_sets: dict[str, set]) -> dict[str, list]:
    """Return {section_key: [redundant_value, ...]} for a package YAML."""
    with open(yaml_path) as f:
        data = yaml.safe_load(f) or {}

    rules = data.get("rules", data) or {}
    redundant: dict[str, list] = {}

    for key, generic_set in generic_sets.items():
        entries = rules.get(key)
        if not isinstance(entries, list):
            continue
        bad = [v for v in entries if v in generic_set]
        if bad:
            redundant[key] = bad

    return redundant


def _remove_list_items_from_text(text: str, section_key: str, values_to_remove: set) -> tuple[str, list]:
    """Remove specific list items from a YAML section in raw text.

    We look for the section heading (e.g. ``  drop_buildrequires:``) then
    scan the following list items and remove lines whose value matches an
    entry in *values_to_remove*.

    YAML allows two common styles for a mapping value that is a sequence:

      Style A — items indented deeper than the key:
        drop_buildrequires:
          - foo
          - bar

      Style B — items at the same indent as the key (compact notation):
        drop_buildrequires:
        - foo
        - bar

    We handle both by accepting list-item lines whose indent depth is
    >= the section heading's indent depth, while stopping the scan when
    we encounter a non-list, non-comment, non-blank line whose indent is
    <= the section heading's indent (i.e. a sibling or parent key).

    Returns (new_text, [actually_removed_values]).
    """
    lines = text.splitlines(keepends=True)
    removed: list = []

    # Regex that matches a YAML list-item line.
    # Captures the indent prefix and the value (plain, single- or double-quoted).
    item_re = re.compile(
        r'^(?P<indent>\s*)-\s+'
        r'(?P<value>'
        r'"(?P<dq>[^"]*)"'      # double-quoted
        r"|'(?P<sq>[^']*)'"     # single-quoted
        r'|(?P<plain>[^\s#][^\n]*?)'  # plain scalar
        r')\s*(?:#.*)?$'        # optional trailing comment
    )

    # Find the section heading at any indent level.
    heading_re = re.compile(r'^(?P<ind>\s*)' + re.escape(section_key) + r'\s*:\s*$')

    out_lines: list[str] = []
    i = 0
    while i < len(lines):
        line = lines[i]
        m = heading_re.match(line)
        if not m:
            out_lines.append(line)
            i += 1
            continue

        # Found the section heading.  Record position; we will decide after
        # scanning whether to keep or drop it.
        heading_line = line
        heading_out_pos = len(out_lines)
        out_lines.append(line)  # tentatively keep; may remove below
        section_indent_len = len(m.group('ind'))
        items_kept = 0      # non-removed list items in this section
        items_removed = 0   # removed list items in this section
        i += 1

        # Scan items that belong to this section.
        while i < len(lines):
            raw = lines[i]
            stripped = raw.rstrip('\n').rstrip('\r')

            # Blank line: keep and continue scanning.
            if stripped.strip() == '':
                out_lines.append(raw)
                i += 1
                continue

            current_indent_len = len(raw) - len(raw.lstrip())

            # Comment line: keep if it is at a deeper indent than the section
            # key (it belongs to the list), stop otherwise.
            if stripped.lstrip().startswith('#'):
                if current_indent_len > section_indent_len:
                    out_lines.append(raw)
                    i += 1
                    continue
                else:
                    break  # comment at peer/parent level — end of section

            # List item at indent >= section heading indent → part of this list.
            im = item_re.match(raw)
            if im and current_indent_len >= section_indent_len:
                val = (
                    im.group('dq')
                    if im.group('dq') is not None
                    else im.group('sq')
                    if im.group('sq') is not None
                    else (im.group('plain') or '').strip()
                )
                if val in values_to_remove:
                    removed.append(val)
                    items_removed += 1
                    i += 1
                    continue
                else:
                    out_lines.append(raw)
                    items_kept += 1
                    i += 1
                    continue

            # Non-list, non-blank, non-comment line.
            # If indent <= section heading indent → sibling/parent key, stop.
            if current_indent_len <= section_indent_len:
                break

            # Deeper non-list content (unlikely in these sections, keep it).
            out_lines.append(raw)
            i += 1

        # If we removed items but kept none, remove the heading line too
        # (leaving it behind would produce a null-valued key in the YAML).
        if items_removed > 0 and items_kept == 0:
            # Remove any trailing blank lines that were buffered between
            # the heading and the end of the section, then remove the heading.
            while out_lines and out_lines[-1].strip() == '' and len(out_lines) > heading_out_pos:
                out_lines.pop()
            # Remove the heading itself (it's at heading_out_pos).
            if len(out_lines) > heading_out_pos and out_lines[heading_out_pos] == heading_line:
                out_lines.pop(heading_out_pos)

    return ''.join(out_lines), removed


def _process_file(yaml_path: Path, generic_sets: dict[str, set], dry_run: bool) -> list[tuple[str, str]]:
    """Process one package YAML.  Returns list of (section_key, value) removed."""
    redundant = _find_redundant(yaml_path, generic_sets)
    if not redundant:
        return []

    text = yaml_path.read_text(encoding='utf-8')
    all_removed: list[tuple[str, str]] = []

    for key, values in redundant.items():
        text, removed = _remove_list_items_from_text(text, key, set(values))
        for v in removed:
            all_removed.append((key, v))

    if all_removed and not dry_run:
        yaml_path.write_text(text, encoding='utf-8')

    return all_removed


# ---------------------------------------------------------------------------
# main
# ---------------------------------------------------------------------------


def main() -> int:
    dry_run = '--dry-run' in sys.argv

    repo_root = Path(__file__).resolve().parent.parent
    rules_dir = repo_root / 'rules'

    generic_sets = _load_generic_sets(rules_dir)
    pkg_dir = rules_dir / 'packages'

    total_removed = 0
    modified_files = 0

    for yaml_path in sorted(pkg_dir.glob('*.yaml')):
        removed = _process_file(yaml_path, generic_sets, dry_run)
        if not removed:
            continue

        modified_files += 1
        total_removed += len(removed)
        pkg_name = yaml_path.stem
        print(f'\n{pkg_name} ({yaml_path.name}):')
        for key, value in removed:
            prefix = '  [DRY-RUN] would remove' if dry_run else '  REMOVED'
            print(f'{prefix}: {key}: {value}')

    action = 'Would remove' if dry_run else 'Removed'
    print(f'\n{action} {total_removed} redundant entries from {modified_files} file(s).')
    if dry_run and total_removed:
        print('Re-run without --dry-run to apply changes.')
    return 0


if __name__ == '__main__':
    sys.exit(main())
