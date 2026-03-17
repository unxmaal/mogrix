#!/usr/bin/env python3
"""Migrate spec_replacements to new primitives.

Analyzes existing spec_replacements in package YAML files and suggests
migrations to the higher-level primitives introduced in the spec handling
redesign (comment_matching, remove_matching, drop_patches, flip_globals,
section_replace, disable_features).

Usage:
    python tools/migrate-spec-replacements.py                # dry-run all
    python tools/migrate-spec-replacements.py --apply        # write changes
    python tools/migrate-spec-replacements.py --package vim  # single package
"""

import argparse
import re
import sys
from pathlib import Path

import yaml


def classify_replacement(repl: dict) -> tuple[str, dict | None]:
    """Classify a spec_replacement into a category and generate migration.

    Returns (category, migrated_entry) where migrated_entry is the
    new-primitive equivalent, or None if no migration is possible.
    """
    pattern = repl.get("pattern", "")
    replacement = repl.get("replacement", "")

    if not pattern:
        return ("empty", None)

    lines = pattern.strip().splitlines()

    # Category: comment out a line (replacement starts with #)
    if len(lines) == 1 and replacement.startswith("#"):
        # Check if it's just the same line with # prepended
        stripped_repl = replacement.lstrip("# ").strip()
        if stripped_repl == pattern.strip() or stripped_repl.startswith(pattern.strip()[:20]):
            return ("comment_line", {
                "regex": f"^{re.escape(pattern.strip())}$",
            })

    # Category: delete a line (replacement is empty or whitespace)
    if len(lines) == 1 and not replacement.strip():
        return ("remove_line", {
            "regex": f"^{re.escape(pattern.strip())}$",
        })

    # Category: flip %global
    global_match = re.match(r"^%global\s+(\S+)\s+([01])$", pattern.strip())
    if global_match:
        name = global_match.group(1)
        old_val = global_match.group(2)
        repl_match = re.match(rf"^%global\s+{re.escape(name)}\s+([01])$", replacement.strip())
        if repl_match and repl_match.group(1) != old_val:
            return ("flip_global", name)

    # Category: comment out %patch line
    patch_match = re.match(r"^%patch\s+-P\s*(\d+)", pattern.strip())
    if patch_match and replacement.startswith("#"):
        return ("drop_patch", int(patch_match.group(1)))

    # Category: comment out Patch<N>: header
    patch_header = re.match(r"^Patch(\d+)\s*:", pattern.strip())
    if patch_header and replacement.startswith("#"):
        return ("drop_patch_header", int(patch_header.group(1)))

    # Category: multiline section rewrite (>3 lines)
    if len(lines) > 3:
        return ("section_rewrite", None)

    # Category: single-line substitution (keep as spec_replacement)
    return ("specific", None)


def analyze_package(pkg_path: Path) -> dict:
    """Analyze a single package file for migration opportunities."""
    with open(pkg_path) as f:
        data = yaml.safe_load(f)

    if not data or "rules" not in data:
        return {"package": pkg_path.stem, "replacements": 0, "migrations": []}

    rules = data.get("rules") or {}
    replacements = rules.get("spec_replacements", [])
    if not replacements:
        return {"package": pkg_path.stem, "replacements": 0, "migrations": []}

    migrations = []
    drop_patch_nums = []
    flip_globals = []
    comment_lines = []
    remove_lines = []

    for i, repl in enumerate(replacements):
        category, entry = classify_replacement(repl)
        migrations.append({
            "index": i,
            "category": category,
            "original": repl,
            "migrated": entry,
        })

        if category == "drop_patch":
            drop_patch_nums.append(entry)
        elif category == "drop_patch_header":
            drop_patch_nums.append(entry)
        elif category == "flip_global":
            flip_globals.append(entry)
        elif category == "comment_line":
            comment_lines.append(entry)
        elif category == "remove_line":
            remove_lines.append(entry)

    return {
        "package": pkg_path.stem,
        "replacements": len(replacements),
        "migrations": migrations,
        "summary": {
            "drop_patches": sorted(set(drop_patch_nums)),
            "flip_globals": flip_globals,
            "comment_matching": comment_lines,
            "remove_matching": remove_lines,
            "migratable": sum(1 for m in migrations if m["migrated"] is not None),
            "remaining": sum(1 for m in migrations if m["migrated"] is None),
        },
    }


def print_report(results: list[dict]) -> None:
    """Print migration analysis report."""
    total_replacements = sum(r["replacements"] for r in results)
    total_migratable = sum(r["summary"]["migratable"] for r in results if "summary" in r)

    print(f"\n{'='*60}")
    print(f"Spec Replacement Migration Analysis")
    print(f"{'='*60}")
    print(f"Total replacements: {total_replacements}")
    print(f"Migratable: {total_migratable} ({100*total_migratable/max(total_replacements,1):.0f}%)")
    print(f"Remaining: {total_replacements - total_migratable}")
    print()

    # Category breakdown
    categories: dict[str, int] = {}
    for r in results:
        for m in r.get("migrations", []):
            cat = m["category"]
            categories[cat] = categories.get(cat, 0) + 1

    print("Category breakdown:")
    for cat, count in sorted(categories.items(), key=lambda x: -x[1]):
        print(f"  {cat:25s} {count:4d}")

    print()

    # Per-package details (only packages with migrations)
    for r in sorted(results, key=lambda x: -x.get("replacements", 0)):
        if r["replacements"] == 0:
            continue
        summary = r.get("summary", {})
        if summary.get("migratable", 0) == 0:
            continue

        print(f"\n{r['package']} ({r['replacements']} replacements, {summary['migratable']} migratable):")
        if summary["drop_patches"]:
            print(f"  drop_patches: {summary['drop_patches']}")
        if summary["flip_globals"]:
            print(f"  flip_globals: {summary['flip_globals']}")
        if summary["comment_matching"]:
            print(f"  comment_matching: {len(summary['comment_matching'])} entries")
        if summary["remove_matching"]:
            print(f"  remove_matching: {len(summary['remove_matching'])} entries")


def main():
    parser = argparse.ArgumentParser(description="Migrate spec_replacements to new primitives")
    parser.add_argument("--apply", action="store_true", help="Write migrated YAML files")
    parser.add_argument("--package", help="Analyze a single package")
    parser.add_argument("--rules-dir", default="rules/packages", help="Rules directory")
    args = parser.parse_args()

    rules_dir = Path(args.rules_dir)
    if not rules_dir.exists():
        print(f"Rules directory not found: {rules_dir}", file=sys.stderr)
        sys.exit(1)

    if args.package:
        pkg_path = rules_dir / f"{args.package}.yaml"
        if not pkg_path.exists():
            print(f"Package not found: {pkg_path}", file=sys.stderr)
            sys.exit(1)
        results = [analyze_package(pkg_path)]
    else:
        results = [analyze_package(p) for p in sorted(rules_dir.glob("*.yaml"))]

    print_report(results)

    if args.apply:
        print("\n--apply is not yet implemented. Review the report first.")


if __name__ == "__main__":
    main()
