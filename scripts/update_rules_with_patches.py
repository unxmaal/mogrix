#!/usr/bin/env python3
"""Update existing mogrix rules to add patch references."""

import os
from pathlib import Path
import yaml

MOGRIX_RULES = Path('/home/edodd/projects/github/unxmaal/mogrix/rules/packages')
MOGRIX_PATCHES = Path('/home/edodd/projects/github/unxmaal/mogrix/patches/packages')

# Packages that have both existing rules AND patches
packages_to_update = [
    'cairo', 'elfutils', 'glib2', 'gnupg2', 'gobject-introspection',
    'gpgme', 'libarchive', 'libassuan', 'libsolv', 'libtasn1',
    'libxml2', 'lua', 'openssl', 'p11-kit', 'python3', 'rpm', 'tdnf', 'zchunk'
]


def get_patches_for_package(pkg):
    """Get list of patch files for a package."""
    patch_dir = MOGRIX_PATCHES / pkg
    if not patch_dir.exists():
        return []
    return sorted([p.name for p in patch_dir.glob('*.patch')])


def update_rule_file(pkg):
    """Update a rule file to add patch references."""
    rule_file = MOGRIX_RULES / f"{pkg}.yaml"
    patches = get_patches_for_package(pkg)

    if not patches:
        print(f"SKIP: {pkg} - no patches found")
        return

    if not rule_file.exists():
        print(f"SKIP: {pkg} - rule file not found")
        return

    content = rule_file.read_text()

    # Check if patches already referenced
    if 'add_patch:' in content:
        # Check if all patches are there
        missing = [p for p in patches if p not in content]
        if not missing:
            print(f"OK: {pkg} - patches already referenced")
            return
        print(f"PARTIAL: {pkg} - missing patches: {missing}")
        # For now, just report
        return

    # Add patch section after package: line
    lines = content.split('\n')
    new_lines = []
    added = False

    for i, line in enumerate(lines):
        new_lines.append(line)
        # Add after 'package:' line (and any empty lines following)
        if line.startswith('package:') and not added:
            # Find where to insert (after any comments/empty lines)
            j = i + 1
            while j < len(lines) and (lines[j].strip() == '' or lines[j].startswith('#')):
                new_lines.append(lines[j])
                j += 1

            # Add patch section
            new_lines.append('')
            new_lines.append('# IRIX-specific patches from SGUG-RSE')
            new_lines.append('add_patch:')
            for p in patches:
                new_lines.append(f'  - {p}')

            added = True
            # Skip the lines we already added
            lines = lines[:i+1] + lines[j:]

    if added:
        rule_file.write_text('\n'.join(new_lines))
        print(f"UPDATED: {pkg} - added {len(patches)} patches")
    else:
        print(f"ERROR: {pkg} - could not find insertion point")


def main():
    for pkg in sorted(packages_to_update):
        update_rule_file(pkg)


if __name__ == '__main__':
    main()
