#!/usr/bin/env python3
"""
Analyze tdnf dependency tree from SGUG-RSE packages.
Outputs a build order for all packages needed to build tdnf.
"""

import os
import re
from collections import defaultdict
from pathlib import Path

SGUG_RSE_PATH = "/home/edodd/projects/github/sgug-rse/packages"

# Packages that are part of the base system or build env (don't need building)
BASE_SYSTEM = {
    'gcc', 'gcc-c++', 'g++', 'make', 'coreutils', 'sed', 'gawk', 'grep',
    'findutils', 'diffutils', 'patch', 'tar', 'gzip', 'bash',
    'perl-interpreter', 'perl', 'gettext',
}

# Packages to skip (Linux-specific, optional, or problematic)
SKIP_PACKAGES = {
    # Linux-specific
    'systemd', 'systemd-devel', 'libselinux', 'libselinux-devel',
    'selinux-policy', 'audit-libs-devel', 'kernel-headers',
    'glibc-all-langpacks', 'glibc-langpack-en', 'fakechroot',
    # Not needed for core build
    'doxygen', 'gtk-doc', 'valgrind', 'valgrind-devel',
    # Build tools assumed to exist
    'ninja-build', 'meson', 'cmake',
    # Test dependencies
    'python2-nose', 'python3-nose', 'python-sphinx', 'python2-sphinx',
    'python3-sphinx', 'python2-flask', 'python3-flask',
    'python2-requests', 'python3-requests', 'pyxattr', 'pygpgme',
    'python2-pyxattr', 'python3-pyxattr', 'python2-gpg', 'python3-gpg',
    'python2-mock', 'python3-mock', 'python2-pytest', 'python3-pytest',
    # Optional bindings we don't need
    'ruby', 'ruby-devel', 'perl-devel', 'perl-generators', 'swig',
    'python2-devel', 'python2',  # Focus on python3
    # X11/GUI stuff not needed for tdnf
    'libX11-devel', 'gtk3-devel', 'gtk+-devel',
}

def normalize_pkg_name(name):
    """Normalize package name for matching."""
    name = name.strip()

    # Skip empty or macro-only names
    if not name or name.startswith('%{') or name.startswith('%('):
        return None

    # Skip version numbers that got parsed as package names
    if re.match(r'^[\d.:><=\-]+$', name):
        return None

    # Skip paths
    if name.startswith('/'):
        return None

    # Skip perl() and other virtual provides that aren't real packages
    if name.startswith('perl(') or name.startswith('tex('):
        return None

    # Remove version constraints: >= 1.2.3, etc.
    name = re.split(r'\s*[<>=]+\s*', name)[0].strip()

    # Handle pkgconfig() names
    if name.startswith('pkgconfig('):
        inner = name[10:-1] if name.endswith(')') else name[10:]
        pkgconfig_map = {
            'rpm': 'rpm',
            'glib-2.0': 'glib2',
            'libxml-2.0': 'libxml2',
            'libcrypto': 'openssl',
            'openssl': 'openssl',
            'zck': 'zchunk',
            'libffi': 'libffi',
            'libpcre': 'pcre',
            'zlib': 'zlib',
            'libelf': 'elfutils',
            'liblzma': 'xz',
        }
        return pkgconfig_map.get(inner, inner.replace('-', ''))

    # Remove -devel suffix for dependency tracking (but note it)
    base_name = name
    if name.endswith('-devel'):
        base_name = name[:-6]

    # Clean up any remaining macro artifacts
    base_name = re.sub(r'%\{[^}]*\}', '', base_name).strip()

    if not base_name or len(base_name) < 2:
        return None

    return base_name

def parse_spec_requires(spec_path):
    """Extract BuildRequires from a spec file."""
    requires = set()
    try:
        with open(spec_path, 'r', errors='replace') as f:
            in_buildrequires = False
            for line in f:
                line = line.strip()

                # Skip comments
                if line.startswith('#'):
                    continue

                if line.startswith('BuildRequires:'):
                    deps_str = line[14:].strip()
                    # Handle continuation and comma-separated
                    for dep in re.split(r'[,\s]+', deps_str):
                        dep = dep.strip().rstrip(',')
                        pkg = normalize_pkg_name(dep)
                        if pkg and pkg not in BASE_SYSTEM and pkg not in SKIP_PACKAGES:
                            requires.add(pkg)
    except Exception as e:
        print(f"Error parsing {spec_path}: {e}")
    return requires

def find_package_spec(pkg_name):
    """Find the spec file for a package."""
    # Try direct match
    pkg_dir = os.path.join(SGUG_RSE_PATH, pkg_name)
    if os.path.isdir(pkg_dir):
        spec_file = os.path.join(pkg_dir, f"{pkg_name}.spec")
        if os.path.exists(spec_file):
            return spec_file

    # Try case-insensitive search
    for entry in os.listdir(SGUG_RSE_PATH):
        if entry.lower() == pkg_name.lower():
            spec_file = os.path.join(SGUG_RSE_PATH, entry, f"{entry}.spec")
            if os.path.exists(spec_file):
                return spec_file

    return None

def build_dependency_graph(root_packages, max_depth=10):
    """Build a dependency graph starting from root packages."""
    graph = defaultdict(set)  # package -> set of dependencies
    visited = set()
    queue = [(pkg, 0) for pkg in root_packages]  # (package, depth)
    packages_found = set()

    while queue:
        pkg, depth = queue.pop(0)
        if pkg in visited or depth > max_depth:
            continue
        visited.add(pkg)

        spec_path = find_package_spec(pkg)
        if spec_path:
            packages_found.add(pkg)
            deps = parse_spec_requires(spec_path)
            graph[pkg] = deps
            for dep in deps:
                if dep not in visited and dep not in SKIP_PACKAGES:
                    queue.append((dep, depth + 1))
        else:
            graph[pkg] = set()

    return graph, packages_found

def topological_sort(graph):
    """Topological sort - dependencies first."""
    in_degree = {pkg: 0 for pkg in graph}

    for pkg in graph:
        for dep in graph[pkg]:
            if dep in in_degree:
                in_degree[pkg] += 1

    # Start with packages that have no unresolved deps
    queue = sorted([pkg for pkg in graph if in_degree[pkg] == 0])
    result = []

    while queue:
        pkg = queue.pop(0)
        result.append(pkg)

        for other_pkg in graph:
            if pkg in graph[other_pkg]:
                in_degree[other_pkg] -= 1
                if in_degree[other_pkg] == 0 and other_pkg not in result:
                    queue.append(other_pkg)
                    queue.sort()

    # Add any remaining (circular deps)
    for pkg in graph:
        if pkg not in result:
            result.append(pkg)

    return result

def check_sgifixes(pkg_name):
    """Check if a package has sgifixes patches."""
    pkg_dir = os.path.join(SGUG_RSE_PATH, pkg_name)
    if os.path.isdir(pkg_dir):
        for f in os.listdir(pkg_dir):
            if 'sgifixes' in f.lower() and f.endswith('.patch'):
                return True
    return False

def check_libdicl_dep(pkg_name):
    """Check if a package depends on libdicl."""
    spec_path = find_package_spec(pkg_name)
    if spec_path:
        with open(spec_path, 'r', errors='replace') as f:
            content = f.read()
            return 'libdicl' in content
    return False

def main():
    print("=" * 70)
    print("TDNF DEPENDENCY TREE ANALYSIS")
    print("=" * 70)
    print()

    # Start with tdnf and its direct dependencies
    root = ['tdnf']

    graph, found_packages = build_dependency_graph(root)
    build_order = topological_sort(graph)

    # Filter to only packages found in SGUG-RSE
    build_order = [pkg for pkg in build_order if pkg in found_packages]

    print(f"Packages found in SGUG-RSE: {len(found_packages)}")
    print()

    print("-" * 70)
    print("BUILD ORDER (dependencies first)")
    print("-" * 70)
    print()

    libdicl_pkgs = []
    sgifixes_pkgs = []

    for i, pkg in enumerate(build_order, 1):
        has_sgifixes = check_sgifixes(pkg)
        needs_libdicl = check_libdicl_dep(pkg)
        deps = graph.get(pkg, set())
        real_deps = [d for d in deps if d in found_packages]

        flags = []
        if has_sgifixes:
            flags.append("SGIFIXES")
            sgifixes_pkgs.append(pkg)
        if needs_libdicl:
            flags.append("LIBDICL")
            libdicl_pkgs.append(pkg)

        flag_str = f"  [{', '.join(flags)}]" if flags else ""
        dep_str = f"← {', '.join(sorted(real_deps)[:4])}" if real_deps else ""
        if len(real_deps) > 4:
            dep_str += f" +{len(real_deps)-4} more"

        print(f"{i:3d}. {pkg:25s} {dep_str}{flag_str}")

    print()
    print("=" * 70)
    print("SUMMARY")
    print("=" * 70)
    print()
    print(f"Total packages to build: {len(build_order)}")
    print(f"Packages with SGIFIXES:  {len(sgifixes_pkgs)}")
    print(f"Packages needing LIBDICL: {len(libdicl_pkgs)}")
    print()

    if libdicl_pkgs:
        print("Packages needing libdicl (mogrix must provide compat sources):")
        for pkg in libdicl_pkgs:
            print(f"  - {pkg}")
        print()

    if sgifixes_pkgs:
        print("Packages with sgifixes patches:")
        for pkg in sgifixes_pkgs:
            print(f"  - {pkg}")
        print()

    # Output JSON
    import json
    output = {
        'target': 'tdnf',
        'build_order': build_order,
        'total_packages': len(build_order),
        'dependency_graph': {k: list(v) for k, v in graph.items() if k in found_packages},
        'libdicl_packages': libdicl_pkgs,
        'sgifixes_packages': sgifixes_pkgs,
    }

    json_path = os.path.join(os.path.dirname(__file__), 'tdnf_deps.json')
    with open(json_path, 'w') as f:
        json.dump(output, f, indent=2)
    print(f"JSON output: {json_path}")

if __name__ == '__main__':
    main()
