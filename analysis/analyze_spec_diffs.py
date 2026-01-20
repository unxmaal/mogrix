#!/usr/bin/env python3
"""
Analyze spec file changes between .spec.origfedora and .spec files.

This script diffs all Fedora original specs against SGUG-RSE modified specs
to identify common patterns of modification needed for IRIX builds.
"""

import os
import re
import sys
import difflib
from collections import defaultdict
from pathlib import Path

SGUG_RSE_PATH = "/home/edodd/projects/github/sgug-rse/packages"

def find_spec_pairs(base_path):
    """Find all packages with both .spec and .spec.origfedora files."""
    pairs = []
    for pkg_dir in os.listdir(base_path):
        pkg_path = os.path.join(base_path, pkg_dir)
        if not os.path.isdir(pkg_path):
            continue

        spec_files = [f for f in os.listdir(pkg_path) if f.endswith('.spec')]
        origfedora_files = [f for f in os.listdir(pkg_path) if f.endswith('.spec.origfedora')]

        for orig in origfedora_files:
            base_name = orig.replace('.spec.origfedora', '.spec')
            if base_name in spec_files:
                pairs.append({
                    'package': pkg_dir,
                    'original': os.path.join(pkg_path, orig),
                    'modified': os.path.join(pkg_path, base_name),
                })
    return pairs

def analyze_spec_diff(original_path, modified_path):
    """Analyze differences between original and modified spec files."""
    with open(original_path, 'r', errors='replace') as f:
        original_lines = f.readlines()
    with open(modified_path, 'r', errors='replace') as f:
        modified_lines = f.readlines()

    diff = list(difflib.unified_diff(original_lines, modified_lines, lineterm=''))

    result = {
        'added_lines': [],
        'removed_lines': [],
        'patterns': defaultdict(list),
    }

    for line in diff:
        if line.startswith('+') and not line.startswith('+++'):
            result['added_lines'].append(line[1:].strip())
        elif line.startswith('-') and not line.startswith('---'):
            result['removed_lines'].append(line[1:].strip())

    analyze_spec_patterns(result['added_lines'], result['removed_lines'], result['patterns'])
    return result

def analyze_spec_patterns(added, removed, patterns):
    """Identify common spec modification patterns."""

    added_text = '\n'.join(added)
    removed_text = '\n'.join(removed)

    # 1. BuildRequires changes
    for line in removed:
        if 'BuildRequires' in line:
            # Extract package name
            match = re.search(r'BuildRequires:\s*(.+)', line)
            if match:
                patterns['removed_buildrequires'].append(match.group(1).strip())

    for line in added:
        if 'BuildRequires' in line:
            match = re.search(r'BuildRequires:\s*(.+)', line)
            if match:
                patterns['added_buildrequires'].append(match.group(1).strip())

    # 2. Requires changes
    for line in removed:
        if re.match(r'^Requires:', line):
            match = re.search(r'Requires:\s*(.+)', line)
            if match:
                patterns['removed_requires'].append(match.group(1).strip())

    for line in added:
        if re.match(r'^Requires:', line):
            match = re.search(r'Requires:\s*(.+)', line)
            if match:
                patterns['added_requires'].append(match.group(1).strip())

    # 3. Commented out patches (arch-specific usually)
    for line in added:
        if re.match(r'^#\s*%?[Pp]atch', line) or re.match(r'^#\s*Patch\d+:', line):
            patterns['commented_patches'].append(line)

    # 4. SGUG macros usage
    sgug_macros = [
        'sgug_optimised_ldflags',
        'sgug_optimised_cflags',
        'sgug_no_optimised_cflags',
        'sgug_libdicl_ldflags',
        'sgug_no_optimised_ldflags',
        '_sgug_prefix',
    ]
    for macro in sgug_macros:
        if macro in added_text:
            patterns['sgug_macros_used'].append(macro)

    # 5. Configure flag changes
    for line in added:
        if '--disable-' in line:
            matches = re.findall(r'--disable-([a-zA-Z0-9_-]+)', line)
            patterns['configure_disable_flags'].extend(matches)
        if '--enable-' in line:
            matches = re.findall(r'--enable-([a-zA-Z0-9_-]+)', line)
            patterns['configure_enable_flags'].extend(matches)
        if '--without-' in line:
            matches = re.findall(r'--without-([a-zA-Z0-9_-]+)', line)
            patterns['configure_without_flags'].extend(matches)
        if '--with-' in line and '--without-' not in line:
            matches = re.findall(r'--with-([a-zA-Z0-9_-]+)', line)
            patterns['configure_with_flags'].extend(matches)

    # 6. ExcludeArch / ExclusiveArch changes
    if 'ExcludeArch' in added_text or 'ExclusiveArch' in added_text:
        patterns['arch_restrictions'].append(True)

    # 7. %ifarch / %ifnarch blocks commented or removed
    for line in removed:
        if '%ifarch' in line or '%ifnarch' in line:
            patterns['removed_arch_conditionals'].append(line)

    # 8. Subpackage removal (commented out %package)
    for line in added:
        if re.match(r'^#\s*%package', line):
            patterns['commented_subpackages'].append(line)

    # 9. systemd-related removals
    if 'systemd' in removed_text.lower():
        patterns['systemd_removed'].append(True)

    # 10. SELinux-related removals
    if 'selinux' in removed_text.lower():
        patterns['selinux_removed'].append(True)

    # 11. Documentation changes
    if '%doc' in added_text or '%license' in added_text:
        patterns['doc_changes'].append(True)

    # 12. Patch additions (new IRIX-specific patches)
    for line in added:
        if re.match(r'^Patch\d+:', line) and 'sgifixes' in line.lower():
            patterns['sgifixes_patches_added'].append(line)
        elif re.match(r'^Patch\d+:', line):
            patterns['other_patches_added'].append(line)

    # 13. LDFLAGS/CFLAGS modifications
    for line in added:
        if 'LDFLAGS' in line:
            patterns['ldflags_modified'].append(line)
        if 'CFLAGS' in line:
            patterns['cflags_modified'].append(line)
        if 'CXXFLAGS' in line:
            patterns['cxxflags_modified'].append(line)

    # 14. make_build / make_install changes
    if '%make_build' in added_text or '%make_install' in added_text:
        patterns['make_macros_used'].append(True)

    # 15. autoreconf / autoconf usage
    if 'autoreconf' in added_text or 'autoconf' in added_text:
        patterns['autoreconf_added'].append(True)

    # 16. %check disabled
    for line in added:
        if re.match(r'^#\s*%check', line):
            patterns['check_disabled'].append(True)

    # 17. lib64 -> lib32 path changes
    if 'lib32' in added_text and 'lib64' in removed_text:
        patterns['lib64_to_lib32'].append(True)

    # 18. /usr/sgug prefix usage
    if '/usr/sgug' in added_text:
        patterns['usr_sgug_prefix'].append(True)

def summarize_spec_results(all_results):
    """Summarize patterns across all spec diffs."""
    summary = {
        'total_specs': len(all_results),
        'packages': [],
        'pattern_counts': defaultdict(int),
        'removed_buildrequires_freq': defaultdict(int),
        'added_buildrequires_freq': defaultdict(int),
        'configure_disable_freq': defaultdict(int),
        'configure_without_freq': defaultdict(int),
        'sgug_macros_freq': defaultdict(int),
    }

    for pkg, result in all_results:
        summary['packages'].append(pkg)

        for pattern, occurrences in result['patterns'].items():
            if occurrences:
                summary['pattern_counts'][pattern] += 1

                if pattern == 'removed_buildrequires':
                    for br in occurrences:
                        # Normalize the package name
                        br_clean = re.split(r'[<>=\s]', br)[0].strip()
                        summary['removed_buildrequires_freq'][br_clean] += 1

                elif pattern == 'added_buildrequires':
                    for br in occurrences:
                        br_clean = re.split(r'[<>=\s]', br)[0].strip()
                        summary['added_buildrequires_freq'][br_clean] += 1

                elif pattern == 'configure_disable_flags':
                    for flag in occurrences:
                        summary['configure_disable_freq'][flag] += 1

                elif pattern == 'configure_without_flags':
                    for flag in occurrences:
                        summary['configure_without_freq'][flag] += 1

                elif pattern == 'sgug_macros_used':
                    for macro in occurrences:
                        summary['sgug_macros_freq'][macro] += 1

    return summary

def print_spec_report(summary):
    """Print formatted report for spec diff analysis."""
    print("=" * 70)
    print("SGUG-RSE Spec File Modification Analysis Report")
    print("=" * 70)
    print()
    print(f"Total spec pairs analyzed: {summary['total_specs']}")
    print()

    print("-" * 70)
    print("MODIFICATION PATTERNS (sorted by frequency)")
    print("-" * 70)
    sorted_patterns = sorted(summary['pattern_counts'].items(),
                            key=lambda x: x[1], reverse=True)
    for pattern, count in sorted_patterns:
        pct = count * 100 // max(summary['total_specs'], 1)
        print(f"  {pattern:40s} : {count:4d} ({pct:2d}%)")
    print()

    print("-" * 70)
    print("MOST FREQUENTLY REMOVED BuildRequires")
    print("-" * 70)
    sorted_br = sorted(summary['removed_buildrequires_freq'].items(),
                      key=lambda x: x[1], reverse=True)[:30]
    for br, count in sorted_br:
        print(f"  {br:45s} : {count:4d}")
    print()

    print("-" * 70)
    print("MOST FREQUENTLY ADDED BuildRequires")
    print("-" * 70)
    sorted_br = sorted(summary['added_buildrequires_freq'].items(),
                      key=lambda x: x[1], reverse=True)[:30]
    for br, count in sorted_br:
        print(f"  {br:45s} : {count:4d}")
    print()

    print("-" * 70)
    print("MOST COMMON --disable-* FLAGS")
    print("-" * 70)
    sorted_flags = sorted(summary['configure_disable_freq'].items(),
                         key=lambda x: x[1], reverse=True)[:30]
    for flag, count in sorted_flags:
        print(f"  --disable-{flag:35s} : {count:4d}")
    print()

    print("-" * 70)
    print("MOST COMMON --without-* FLAGS")
    print("-" * 70)
    sorted_flags = sorted(summary['configure_without_freq'].items(),
                         key=lambda x: x[1], reverse=True)[:30]
    for flag, count in sorted_flags:
        print(f"  --without-{flag:35s} : {count:4d}")
    print()

    print("-" * 70)
    print("SGUG MACROS USAGE")
    print("-" * 70)
    sorted_macros = sorted(summary['sgug_macros_freq'].items(),
                          key=lambda x: x[1], reverse=True)
    for macro, count in sorted_macros:
        print(f"  %{{{macro}}}  : {count:4d}")
    print()

def write_json_report(summary, output_path):
    """Write summary as JSON."""
    import json

    json_summary = {
        'total_specs': summary['total_specs'],
        'pattern_counts': dict(summary['pattern_counts']),
        'removed_buildrequires_freq': dict(summary['removed_buildrequires_freq']),
        'added_buildrequires_freq': dict(summary['added_buildrequires_freq']),
        'configure_disable_freq': dict(summary['configure_disable_freq']),
        'configure_without_freq': dict(summary['configure_without_freq']),
        'sgug_macros_freq': dict(summary['sgug_macros_freq']),
    }

    with open(output_path, 'w') as f:
        json.dump(json_summary, f, indent=2)
    print(f"JSON report written to: {output_path}")

def main():
    print(f"Scanning {SGUG_RSE_PATH} for spec file pairs...")
    pairs = find_spec_pairs(SGUG_RSE_PATH)
    print(f"Found {len(pairs)} packages with both .spec and .spec.origfedora")
    print()

    all_results = []
    for pair in pairs:
        try:
            result = analyze_spec_diff(pair['original'], pair['modified'])
            all_results.append((pair['package'], result))
        except Exception as e:
            print(f"Error analyzing {pair['package']}: {e}", file=sys.stderr)

    summary = summarize_spec_results(all_results)
    print_spec_report(summary)

    json_path = os.path.join(os.path.dirname(__file__), 'spec_diff_summary.json')
    write_json_report(summary, json_path)

if __name__ == '__main__':
    main()
