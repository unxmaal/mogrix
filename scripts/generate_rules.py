#!/usr/bin/env python3
"""Generate mogrix rules from SGUG-RSE spec file changes.

Analyzes the differences between .spec.origfedora and .spec files
to extract IRIX-specific modifications.
"""

import os
import re
import sys
from pathlib import Path

SGUG_BASE = Path('/home/edodd/projects/github/sgug-rse/packages')
MOGRIX_RULES = Path('/home/edodd/projects/github/unxmaal/mogrix/rules/packages')
MOGRIX_PATCHES = Path('/home/edodd/projects/github/unxmaal/mogrix/patches/packages')

# BuildRequires that should be dropped for IRIX
LINUX_ONLY_DEPS = {
    'systemd', 'systemd-devel', 'libsystemd-devel',
    'libselinux-devel', 'selinux-policy', 'selinux-policy-devel',
    'audit-libs-devel', 'libcap-devel', 'libcap-ng-devel',
    'libattr-devel', 'libacl-devel',
    'kernel-headers', 'glibc-devel', 'glibc-static',
    'lksctp-tools-devel', 'systemtap-sdt-devel',
    'libbpf-devel', 'bpftool',
    'glibc-all-langpacks', 'glibc-langpack-en',
    'bluez-libs-devel',
    'valgrind', 'valgrind-devel',
}

def analyze_spec(pkg_name):
    """Analyze a package's spec files and extract IRIX changes."""
    pkg_dir = SGUG_BASE / pkg_name

    spec_file = pkg_dir / f'{pkg_name}.spec'
    orig_file = pkg_dir / f'{pkg_name}.spec.origfedora'

    if not spec_file.exists():
        return None

    result = {
        'package': pkg_name,
        'patches': [],
        'drop_buildrequires': [],
        'uses_libdicl': False,
        'configure_disable': [],
        'notes': [],
    }

    # Find patches in the mogrix patches directory
    patch_dir = MOGRIX_PATCHES / pkg_name
    if patch_dir.exists():
        for p in sorted(patch_dir.glob('*.patch')):
            result['patches'].append(p.name)

    # Read spec file
    spec_content = spec_file.read_text()

    # Check for libdicl usage
    if 'libdicl-devel' in spec_content:
        result['uses_libdicl'] = True
        result['notes'].append('Uses libdicl-devel - needs compat functions')

    # If we have the original, do a diff analysis
    if orig_file.exists():
        orig_content = orig_file.read_text()

        # Find commented-out BuildRequires
        for line in spec_content.split('\n'):
            # Look for commented BuildRequires that were in original
            match = re.match(r'#\s*BuildRequires:\s*(\S+)', line)
            if match:
                dep = match.group(1)
                # Check if it was uncommented in original
                if f'BuildRequires: {dep}' in orig_content or f'BuildRequires:\t{dep}' in orig_content:
                    # Strip version specs
                    base_dep = dep.split()[0] if ' ' in dep else dep
                    base_dep = re.sub(r'[<>=].*', '', base_dep)
                    result['drop_buildrequires'].append(base_dep)

    # Filter to just Linux-only deps
    result['drop_buildrequires'] = [d for d in result['drop_buildrequires']
                                     if any(x in d for x in LINUX_ONLY_DEPS) or d in LINUX_ONLY_DEPS]

    return result


def generate_yaml(analysis):
    """Generate YAML rule content from analysis."""
    lines = [f"# {analysis['package']} package rules for IRIX cross-compilation"]
    lines.append(f"package: {analysis['package']}")
    lines.append("")

    if analysis['patches']:
        lines.append("# IRIX-specific patches from SGUG-RSE")
        lines.append("add_patch:")
        for p in analysis['patches']:
            lines.append(f"  - {p}")
        lines.append("")

    if analysis['drop_buildrequires']:
        lines.append("# Linux-only dependencies to remove")
        lines.append("drop_buildrequires:")
        for dep in sorted(set(analysis['drop_buildrequires'])):
            lines.append(f"  - {dep}")
        lines.append("")

    if analysis['uses_libdicl']:
        lines.append("# Compat functions (replaces libdicl-devel)")
        lines.append("compat_functions:")
        lines.append("  - strdup")
        lines.append("  - strndup")
        lines.append("")

    if analysis['notes']:
        lines.append("# Notes:")
        for note in analysis['notes']:
            lines.append(f"#   {note}")

    return '\n'.join(lines)


def main():
    # Packages to process
    sgifixes_pkgs = """
    automake binutils cairo dejagnu elfutils emacs expect flex freetype gcr gd git glib2 gnupg2 gobject-introspection gpgme groff gtk2 harfbuzz libarchive libassuan libdb libffi libjpeg-turbo libsecret libsolv libtasn1 libtool libxml2 lua m4 openjade openssl p11-kit pango pixman python3 rpm sqlite symlinks tcl tdnf tk uuid zchunk
    """.split()

    for pkg in sorted(sgifixes_pkgs):
        analysis = analyze_spec(pkg)
        if not analysis:
            print(f"SKIP: {pkg} - no spec file found")
            continue

        yaml_content = generate_yaml(analysis)
        output_file = MOGRIX_RULES / f"{pkg}.yaml"

        # Check if rule already exists
        if output_file.exists():
            existing = output_file.read_text()
            # Check if patches are already referenced
            if analysis['patches'] and not any(p in existing for p in analysis['patches']):
                print(f"UPDATE: {pkg} - needs patch references added")
                # For now, just report - don't overwrite
            else:
                print(f"EXISTS: {pkg} - rule already exists")
        else:
            print(f"CREATE: {pkg}")
            output_file.write_text(yaml_content + '\n')


if __name__ == '__main__':
    main()
