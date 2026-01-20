#!/usr/bin/env python3
"""
Analyze .sgifixes.patch files from SGUG-RSE to extract common patterns.

This script parses all .sgifixes.patch files and categorizes the types
of fixes needed to make Linux code compile on IRIX.
"""

import os
import re
import sys
from collections import defaultdict
from pathlib import Path

SGUG_RSE_PATH = "/home/edodd/projects/github/sgug-rse/packages"

def find_sgifixes_patches(base_path):
    """Find all .sgifixes.patch files."""
    patches = []
    for root, dirs, files in os.walk(base_path):
        for f in files:
            if '.sgifixes' in f and f.endswith('.patch'):
                patches.append(os.path.join(root, f))
    return sorted(patches)

def parse_patch_file(patch_path):
    """Parse a patch file and extract modified files and patterns."""
    with open(patch_path, 'r', errors='replace') as f:
        content = f.read()

    result = {
        'path': patch_path,
        'package': os.path.basename(os.path.dirname(patch_path)),
        'files_modified': [],
        'patterns': defaultdict(list),
        'raw_hunks': []
    }

    # Extract file names from diff headers
    file_pattern = re.compile(r'^diff.*?([^\s]+)\s+([^\s]+)$', re.MULTILINE)
    for match in file_pattern.finditer(content):
        result['files_modified'].append(match.group(2))

    # Also check +++ lines
    plus_pattern = re.compile(r'^\+\+\+ ([^\t\n]+)', re.MULTILINE)
    for match in plus_pattern.finditer(content):
        fname = match.group(1).strip()
        if fname not in result['files_modified'] and fname != '/dev/null':
            result['files_modified'].append(fname)

    # Extract hunks (actual changes)
    hunk_pattern = re.compile(r'^@@.*?@@.*?(?=^@@|^diff|\Z)', re.MULTILINE | re.DOTALL)
    result['raw_hunks'] = hunk_pattern.findall(content)

    # Analyze patterns in the patch content
    analyze_patterns(content, result['patterns'])

    return result

def analyze_patterns(content, patterns):
    """Analyze patch content for common fix patterns."""

    # 1. __sgi guards
    if re.search(r'#if.*defined\s*\(\s*__sgi\s*\)', content):
        matches = re.findall(r'#if.*defined\s*\(\s*__sgi\s*\).*?#endif', content, re.DOTALL)
        patterns['sgi_ifdef_guards'].extend(matches[:3])  # Keep first 3 examples

    # 2. Include order fixes (time.h before sys/time.h, etc.)
    if re.search(r'\+#include <time\.h>', content) and re.search(r'sys/time\.h', content):
        patterns['include_time_h_before_sys_time'].append(True)

    # 3. Missing includes added
    include_adds = re.findall(r'^\+\s*#include\s*[<"]([^>"]+)[>"]', content, re.MULTILINE)
    for inc in include_adds:
        patterns['added_includes'].append(inc)

    # 4. config.h additions
    if re.search(r'\+#include\s*[<"]config\.h[>"]', content):
        patterns['added_config_h'].append(True)

    # 5. Packed struct fixes
    if re.search(r'#pragma\s+pack', content):
        patterns['pragma_pack_fixes'].append(True)

    # 6. __attribute__ replacements
    if re.search(r'__attribute__\s*\(\s*\(\s*packed', content):
        patterns['attribute_packed_fixes'].append(True)

    # 7. Function replacements/stubs
    func_patterns = [
        (r'vfork', 'vfork_fixes'),
        (r'getprogname', 'getprogname_fixes'),
        (r'eaccess', 'eaccess_fixes'),
        (r'strerror_r', 'strerror_r_fixes'),
        (r'posix_memalign', 'posix_memalign_fixes'),
        (r'clock_gettime', 'clock_gettime_fixes'),
        (r'getline', 'getline_fixes'),
        (r'getdelim', 'getdelim_fixes'),
        (r'strdup', 'strdup_fixes'),
        (r'strndup', 'strndup_fixes'),
        (r'asprintf', 'asprintf_fixes'),
        (r'vasprintf', 'vasprintf_fixes'),
        (r'mkostemp', 'mkostemp_fixes'),
        (r'pipe2', 'pipe2_fixes'),
        (r'dup3', 'dup3_fixes'),
        (r'accept4', 'accept4_fixes'),
        (r'epoll', 'epoll_fixes'),
        (r'inotify', 'inotify_fixes'),
        (r'signalfd', 'signalfd_fixes'),
        (r'eventfd', 'eventfd_fixes'),
        (r'timerfd', 'timerfd_fixes'),
    ]

    for pattern, key in func_patterns:
        if re.search(pattern, content, re.IGNORECASE):
            patterns[key].append(True)

    # 8. FPU/floating point fixes
    if re.search(r'fpu|fpset|fpc_csr|isnan|isinf', content, re.IGNORECASE):
        patterns['fpu_fixes'].append(True)

    # 9. Signal handling fixes
    if re.search(r'sigset_t|sigjmp|sigaction|SIGCHLD', content):
        patterns['signal_fixes'].append(True)

    # 10. Thread/pthread fixes
    if re.search(r'pthread|__thread|thread_local', content):
        patterns['pthread_fixes'].append(True)

    # 11. Endianness fixes
    if re.search(r'BYTE_ORDER|BIG_ENDIAN|LITTLE_ENDIAN|__BYTE_ORDER', content):
        patterns['endian_fixes'].append(True)

    # 12. Integer type fixes
    if re.search(r'int64_t|uint64_t|intptr_t|uintptr_t|ssize_t', content):
        patterns['int_type_fixes'].append(True)

    # 13. Linker/visibility fixes
    if re.search(r'visibility|__attribute__.*default|__attribute__.*hidden', content):
        patterns['visibility_fixes'].append(True)

    # 14. AIX compatibility (IRIX often similar)
    if re.search(r'_AIX|AIX', content):
        patterns['aix_compat_pattern'].append(True)

def summarize_results(all_results):
    """Summarize patterns across all patches."""
    summary = {
        'total_patches': len(all_results),
        'packages': set(),
        'all_modified_files': [],
        'pattern_counts': defaultdict(int),
        'added_includes_freq': defaultdict(int),
        'examples': defaultdict(list),
    }

    for result in all_results:
        summary['packages'].add(result['package'])
        summary['all_modified_files'].extend(result['files_modified'])

        for pattern, occurrences in result['patterns'].items():
            if occurrences:
                summary['pattern_counts'][pattern] += 1
                if pattern == 'added_includes':
                    for inc in occurrences:
                        summary['added_includes_freq'][inc] += 1
                elif len(summary['examples'][pattern]) < 3:
                    summary['examples'][pattern].append({
                        'package': result['package'],
                        'example': occurrences[0] if occurrences else None
                    })

    return summary

def print_report(summary):
    """Print a formatted report."""
    print("=" * 70)
    print("SGUG-RSE .sgifixes.patch Analysis Report")
    print("=" * 70)
    print()
    print(f"Total patches analyzed: {summary['total_patches']}")
    print(f"Packages with sgifixes: {len(summary['packages'])}")
    print()

    print("-" * 70)
    print("PATTERN FREQUENCY (sorted by occurrence)")
    print("-" * 70)
    sorted_patterns = sorted(summary['pattern_counts'].items(),
                            key=lambda x: x[1], reverse=True)
    for pattern, count in sorted_patterns:
        pct = count * 100 // summary['total_patches']
        print(f"  {pattern:40s} : {count:4d} ({pct:2d}%)")
    print()

    print("-" * 70)
    print("MOST FREQUENTLY ADDED INCLUDES")
    print("-" * 70)
    sorted_includes = sorted(summary['added_includes_freq'].items(),
                            key=lambda x: x[1], reverse=True)[:20]
    for inc, count in sorted_includes:
        print(f"  #include <{inc:30s}> : {count:4d}")
    print()

    print("-" * 70)
    print("MOST FREQUENTLY MODIFIED FILES (by path suffix)")
    print("-" * 70)
    file_suffix_freq = defaultdict(int)
    for f in summary['all_modified_files']:
        # Get last 2 path components
        parts = f.split('/')
        suffix = '/'.join(parts[-2:]) if len(parts) >= 2 else f
        file_suffix_freq[suffix] += 1

    sorted_files = sorted(file_suffix_freq.items(),
                         key=lambda x: x[1], reverse=True)[:30]
    for fname, count in sorted_files:
        print(f"  {fname:50s} : {count:4d}")
    print()

def write_json_report(summary, output_path):
    """Write summary as JSON for further processing."""
    import json

    # Convert sets to lists for JSON serialization
    json_summary = {
        'total_patches': summary['total_patches'],
        'packages': sorted(list(summary['packages'])),
        'pattern_counts': dict(summary['pattern_counts']),
        'added_includes_freq': dict(summary['added_includes_freq']),
    }

    with open(output_path, 'w') as f:
        json.dump(json_summary, f, indent=2)
    print(f"JSON report written to: {output_path}")

def main():
    print(f"Scanning {SGUG_RSE_PATH} for .sgifixes.patch files...")
    patches = find_sgifixes_patches(SGUG_RSE_PATH)
    print(f"Found {len(patches)} patch files")
    print()

    all_results = []
    for patch_path in patches:
        try:
            result = parse_patch_file(patch_path)
            all_results.append(result)
        except Exception as e:
            print(f"Error parsing {patch_path}: {e}", file=sys.stderr)

    summary = summarize_results(all_results)
    print_report(summary)

    # Write JSON for further analysis
    json_path = os.path.join(os.path.dirname(__file__), 'sgifixes_summary.json')
    write_json_report(summary, json_path)

if __name__ == '__main__':
    main()
