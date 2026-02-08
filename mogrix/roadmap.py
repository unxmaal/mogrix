"""Build dependency graph resolver and roadmap generator.

Given a target FC40 source package, resolves the full transitive build
dependency graph, classifies each dependency against mogrix state (rules,
built RPMs, sysroot), and produces a topologically sorted build plan.

Used by `mogrix roadmap <package>`.
"""

import json
import re
import sqlite3
from collections import defaultdict
from dataclasses import dataclass, field
from fnmatch import fnmatch
from enum import Enum
from pathlib import Path

import yaml
from rich.console import Console
from rich.tree import Tree

from mogrix.repometa import extract_srpm_name
from mogrix.rules.loader import RuleLoader


class Classification(Enum):
    """How a dependency is classified in the roadmap."""

    DROPPED = "dropped"
    SYSROOT = "sysroot"
    ALREADY_BUILT_VERIFIED = "built_verified"
    ALREADY_BUILT_UNVERIFIED = "built_unverified"
    HAS_RULES = "has_rules"
    NON_FEDORA = "non_fedora"
    NEED_RULES = "need_rules"
    UNRESOLVABLE = "unresolvable"


@dataclass
class PackageInfo:
    """Information about a package in the roadmap."""

    name: str
    classification: Classification
    complexity: str = ""  # LOW, MEDIUM, HIGH
    build_order: int = 0
    buildrequires: list[str] = field(default_factory=list)
    needed_by: list[str] = field(default_factory=list)
    drop_reason: str = ""  # e.g., "generic", "nls-disabled class", "package-level"
    non_fedora_source: str = ""  # e.g., "photon5"


@dataclass
class RoadmapResult:
    """Result of a roadmap resolution."""

    target: str
    packages: dict[str, PackageInfo] = field(default_factory=dict)
    build_order: list[str] = field(default_factory=list)
    cycles: list[list[str]] = field(default_factory=list)
    dropped_deps: dict[str, str] = field(default_factory=dict)  # dep_name -> reason
    unresolvable_deps: dict[str, str] = field(default_factory=dict)  # dep_name -> context
    sysroot_deps: set = field(default_factory=set)

    @property
    def summary(self) -> dict[str, int]:
        counts: dict[str, int] = defaultdict(int)
        for pkg in self.packages.values():
            counts[pkg.classification.value] += 1
        return dict(counts)


class RoadmapResolver:
    """Resolves build dependency graphs against mogrix state."""

    def __init__(
        self,
        db: sqlite3.Connection,
        rule_loader: RuleLoader,
        rules_dir: Path,
        rpms_dir: Path,
        stop_at_rules: bool = False,
        max_depth: int = 0,
    ):
        self.db = db
        self.rule_loader = rule_loader
        self.rules_dir = rules_dir
        self.rpms_dir = rpms_dir
        self.stop_at_rules = stop_at_rules
        self.max_depth = max_depth

        # Pre-load static data
        self._sysroot = self._load_sysroot_provides()
        self._non_fedora = self._load_non_fedora_packages()
        self._built_packages = self._scan_built_packages()
        self._rule_packages = self._scan_rule_packages()
        self._roadmap_drops = self._load_roadmap_config()

        # Cache for computed drops per package
        self._drops_cache: dict[str, set[str]] = {}

        # Cache for provider resolution
        self._provider_cache: dict[str, tuple[str, Classification]] = {}

    def _load_sysroot_provides(self) -> dict[str, set[str]]:
        """Load sysroot_provides.yaml."""
        path = self.rules_dir / "sysroot_provides.yaml"
        if not path.exists():
            return {"libraries": set(), "files": set(), "capabilities": set()}
        with open(path) as f:
            data = yaml.safe_load(f) or {}
        return {
            "libraries": set(data.get("libraries", [])),
            "files": set(data.get("files", [])),
            "capabilities": set(data.get("capabilities", [])),
        }

    def _load_non_fedora_packages(self) -> dict[str, str]:
        """Load non_fedora_packages.yaml. Returns pkg_name -> source."""
        path = self.rules_dir / "non_fedora_packages.yaml"
        if not path.exists():
            return {}
        with open(path) as f:
            data = yaml.safe_load(f) or {}
        return {
            name: info.get("source", "unknown")
            for name, info in data.get("packages", {}).items()
        }

    def _scan_built_packages(self) -> dict[str, bool]:
        """Scan ~/mogrix_outputs/RPMS/ for built packages.

        Uses the sqlite index to map binary RPM names back to source package
        names (e.g., libcurl -> curl, bzip2-libs -> bzip2).

        Returns dict of source_package_name -> has_smoke_test.
        """
        built: dict[str, bool] = {}
        if not self.rpms_dir.exists():
            return built

        # Extract binary package names from RPM filenames
        binary_names: set[str] = set()
        for rpm_file in self.rpms_dir.glob("*.rpm"):
            name = rpm_file.name
            # Strip .mips.rpm or .noarch.rpm suffix
            base = re.sub(r"\.(mips|noarch)\.rpm$", "", name)
            # Parse name-version-release
            parts = base.rsplit("-", 2)
            if len(parts) >= 1:
                binary_names.add(parts[0])

        # Map binary names to source package names via sqlite index
        source_names: set[str] = set()
        for bname in binary_names:
            row = self.db.execute(
                """SELECT DISTINCT source_package FROM binary_provides
                   WHERE binary_package = ?
                   LIMIT 1""",
                (bname,),
            ).fetchone()
            if row:
                source_names.add(row[0])
            else:
                # Fallback: check if it's a direct source package name
                row2 = self.db.execute(
                    """SELECT DISTINCT source_package FROM source_buildrequires
                       WHERE source_package = ?
                       LIMIT 1""",
                    (bname,),
                ).fetchone()
                if row2:
                    source_names.add(bname)
                else:
                    # Could be a rule-only package (sgugrse-release, etc.)
                    source_names.add(bname)

        # For each source name, check if there's a rule with smoke_test
        for name in source_names:
            rules = self.rule_loader.load_package(name)
            has_smoke = False
            if rules and rules.get("smoke_test"):
                has_smoke = True
            built[name] = has_smoke

        return built

    def _scan_rule_packages(self) -> set[str]:
        """Scan rules/packages/ for existing rule files."""
        packages_dir = self.rules_dir / "packages"
        if not packages_dir.exists():
            return set()
        return {p.stem for p in packages_dir.glob("*.yaml")}

    def _load_roadmap_config(self) -> dict[str, list[str]]:
        """Load roadmap_config.yaml for roadmap-only filtering.

        Returns dict of category_name -> list of glob patterns.
        Returns empty dict if file doesn't exist (backward compatible).
        """
        path = self.rules_dir / "roadmap_config.yaml"
        if not path.exists():
            return {}
        with open(path) as f:
            data = yaml.safe_load(f) or {}
        return data.get("roadmap_drops", {})

    def _check_roadmap_drop(self, source_pkg: str) -> str | None:
        """Check if a source package matches any roadmap_config.yaml pattern.

        Returns the category name if matched, None otherwise.
        """
        for category, patterns in self._roadmap_drops.items():
            for pattern in patterns:
                if fnmatch(source_pkg, pattern) or source_pkg == pattern:
                    return category
        return None

    def _compute_effective_drops(self, pkg: str) -> set[str]:
        """Compute the full set of dropped BuildRequires for a package.

        Merges drops from: generic.yaml -> classes -> package-specific rules.
        """
        if pkg in self._drops_cache:
            return self._drops_cache[pkg]

        drops: set[str] = set()

        # 1. Generic drops
        generic = self.rule_loader.load_generic()
        if generic:
            drops.update(generic.get("generic", {}).get("drop_buildrequires", []))

        # 2. Class drops (if package has a rule file with classes)
        pkg_rules = self.rule_loader.load_package(pkg)
        if pkg_rules:
            for cls_name in pkg_rules.get("classes", []):
                cls_rules = self.rule_loader.load_class(cls_name)
                if cls_rules:
                    cls_drops = cls_rules.get("rules", {}).get("drop_buildrequires", [])
                    drops.update(cls_drops)

            # 3. Package-specific drops
            pkg_drops = pkg_rules.get("rules", {}).get("drop_buildrequires", [])
            drops.update(pkg_drops)

        self._drops_cache[pkg] = drops
        return drops

    @staticmethod
    def _extract_simple_names(req_name: str) -> list[str]:
        """Extract simple package names from an RPM rich dependency expression.

        Rich deps look like:
          '(python3dist(foo) >= 1 with python3dist(foo) < 2)'
          '((A < 2 or A > 2) with A >= 1)'

        We extract all `name` tokens that look like package/provide names.
        """
        if not any(c in req_name for c in ("(", " with ", " or ", " if ", " unless ")):
            return [req_name]

        # Extract all tokens that look like package names or provides
        # Match: python3dist(foo), pkgconfig(bar), plain-name, /usr/bin/foo
        names: list[str] = []
        # python3dist(...), pkgconfig(...), etc.
        for m in re.finditer(r'(\w+dist\([^)]+\)|\w+config\([^)]+\))', req_name):
            names.append(m.group(1))
        # file paths
        for m in re.finditer(r'(/[\w/.-]+)', req_name):
            names.append(m.group(1))
        # plain package names (alphanumeric + hyphen, at start or after space)
        for m in re.finditer(r'(?:^|[\s(])([a-zA-Z][\w+-]*(?:-[\w+-]+)*)', req_name):
            name = m.group(1)
            # Skip version comparison operators and keywords
            if name not in ("or", "with", "if", "unless", "without", "and", "else"):
                names.append(name)

        # Deduplicate while preserving order
        seen: set[str] = set()
        result: list[str] = []
        for n in names:
            if n not in seen:
                seen.add(n)
                result.append(n)
        return result

    def _resolve_provider(
        self, req_name: str, drops: set[str], context_pkg: str
    ) -> tuple[str | None, Classification, str]:
        """Resolve a single BuildRequires to a source package + classification.

        Returns:
            (source_package_name, classification, detail_string)
            source_package_name is None for DROPPED/SYSROOT/UNRESOLVABLE.
        """
        # Check cache
        cache_key = f"{context_pkg}:{req_name}"
        if cache_key in self._provider_cache:
            cached = self._provider_cache[cache_key]
            return cached[0], cached[1], ""

        # 1. Drop rules (exact match or glob pattern)
        if req_name in drops:
            return None, Classification.DROPPED, "rule"
        for pattern in drops:
            if ("*" in pattern or "?" in pattern) and fnmatch(req_name, pattern):
                return None, Classification.DROPPED, f"rule ({pattern})"

        # Also check common Linux-only patterns
        linux_only_patterns = [
            "kernel-", "selinux-", "systemd-", "firewalld-",
            "dbus-daemon", "valgrind", "gdb-headless",
        ]
        for pattern in linux_only_patterns:
            if req_name.startswith(pattern) or req_name == pattern.rstrip("-"):
                return None, Classification.DROPPED, f"linux-only ({pattern})"

        # 2. Sysroot provides
        if req_name in self._sysroot["capabilities"]:
            return None, Classification.SYSROOT, "capability"
        if req_name in self._sysroot["files"]:
            return None, Classification.SYSROOT, "file"
        # Check library provides (e.g., "libc.so()(64bit)" matches "libc.so")
        for lib in self._sysroot["libraries"]:
            if req_name == lib or req_name.startswith(lib + "("):
                return None, Classification.SYSROOT, f"library ({lib})"

        # 4. Already built — check by provides name from sqlite
        src_pkg = self._find_source_package(req_name)
        if src_pkg:
            # Also check if the resolved source package matches a drop pattern
            if self._matches_drop_pattern(src_pkg, drops):
                return None, Classification.DROPPED, f"rule (source: {src_pkg})"
            # Check roadmap-only drops (against resolved source package name)
            roadmap_cat = self._check_roadmap_drop(src_pkg)
            if roadmap_cat:
                return None, Classification.DROPPED, f"roadmap ({roadmap_cat}: {src_pkg})"
            return self._classify_found_package(src_pkg, cache_key)

        # Try extracting names from rich dependency expressions
        simple_names = self._extract_simple_names(req_name)
        for simple_name in simple_names:
            if simple_name == req_name:
                continue  # Already tried
            src_pkg = self._find_source_package(simple_name)
            if src_pkg:
                if self._matches_drop_pattern(src_pkg, drops):
                    return None, Classification.DROPPED, f"rule (source: {src_pkg})"
                roadmap_cat = self._check_roadmap_drop(src_pkg)
                if roadmap_cat:
                    return None, Classification.DROPPED, f"roadmap ({roadmap_cat}: {src_pkg})"
                return self._classify_found_package(src_pkg, cache_key)

        # 8. Unresolvable
        return None, Classification.UNRESOLVABLE, req_name

    @staticmethod
    def _matches_drop_pattern(name: str, drops: set[str]) -> bool:
        """Check if a name matches any glob pattern in the drop set."""
        for pattern in drops:
            if ("*" in pattern or "?" in pattern) and fnmatch(name, pattern):
                return True
        return False

    def _classify_found_package(
        self, src_pkg: str, cache_key: str
    ) -> tuple[str, Classification, str]:
        """Classify a found source package."""
        if src_pkg in self._built_packages:
            has_smoke = self._built_packages[src_pkg]
            cls = (
                Classification.ALREADY_BUILT_VERIFIED
                if has_smoke
                else Classification.ALREADY_BUILT_UNVERIFIED
            )
            self._provider_cache[cache_key] = (src_pkg, cls)
            return src_pkg, cls, ""

        if src_pkg in self._rule_packages:
            self._provider_cache[cache_key] = (src_pkg, Classification.HAS_RULES)
            return src_pkg, Classification.HAS_RULES, ""

        if src_pkg in self._non_fedora:
            self._provider_cache[cache_key] = (src_pkg, Classification.NON_FEDORA)
            return src_pkg, Classification.NON_FEDORA, self._non_fedora[src_pkg]

        self._provider_cache[cache_key] = (src_pkg, Classification.NEED_RULES)
        return src_pkg, Classification.NEED_RULES, ""

    def _find_source_package(self, req_name: str) -> str | None:
        """Find the source package that provides a given capability.

        Checks binary_provides table, then file_provides for file-based deps.
        """
        # Check binary provides (prefer updates over releases)
        row = self.db.execute(
            """SELECT source_package FROM binary_provides
               WHERE provides_name = ?
               ORDER BY repo = 'updates' DESC
               LIMIT 1""",
            (req_name,),
        ).fetchone()
        if row:
            return row[0]

        # Check file provides for file-based deps (e.g., /usr/bin/perl)
        if req_name.startswith("/"):
            row = self.db.execute(
                """SELECT source_package FROM file_provides
                   WHERE file_path = ?
                   ORDER BY repo = 'updates' DESC
                   LIMIT 1""",
                (req_name,),
            ).fetchone()
            if row:
                return row[0]

        # Try stripping common suffixes and checking as a direct package name
        for suffix in ("-devel", "-libs", "-static", "-doc"):
            if req_name.endswith(suffix):
                base = req_name[: -len(suffix)]
                row = self.db.execute(
                    """SELECT source_package FROM binary_provides
                       WHERE provides_name = ?
                       ORDER BY repo = 'updates' DESC
                       LIMIT 1""",
                    (base,),
                ).fetchone()
                if row:
                    return row[0]

        return None

    def _get_buildrequires(self, pkg: str) -> list[str]:
        """Get BuildRequires for a source package from the index."""
        rows = self.db.execute(
            """SELECT DISTINCT requires_name FROM source_buildrequires
               WHERE source_package = ?
               ORDER BY requires_name""",
            (pkg,),
        ).fetchall()
        return [r[0] for r in rows]

    def resolve(self, target: str) -> RoadmapResult:
        """Resolve the full transitive build dependency graph for a target package.

        Always resolves the complete graph — even already-built packages have
        their deps traced. Classification only affects display, not recursion
        (except --stop-at-rules which stops at HAS_RULES nodes).
        """
        result = RoadmapResult(target=target)

        # BFS queue: (package_name, depth)
        queue: list[tuple[str, int]] = [(target, 0)]
        visited: set[str] = set()
        edges: list[tuple[str, str]] = []  # (dep, dependent)

        while queue:
            pkg, depth = queue.pop(0)
            if pkg in visited:
                continue
            visited.add(pkg)

            if self.max_depth > 0 and depth > self.max_depth:
                continue

            # Get BuildRequires from index
            buildrequires = self._get_buildrequires(pkg)
            drops = self._compute_effective_drops(pkg)

            pkg_info = PackageInfo(
                name=pkg,
                classification=self._classify_package(pkg),
                buildrequires=[],
            )

            for req in buildrequires:
                src_pkg, cls, detail = self._resolve_provider(req, drops, pkg)

                if cls == Classification.DROPPED:
                    result.dropped_deps[req] = detail
                    continue

                if cls == Classification.SYSROOT:
                    result.sysroot_deps.add(req)
                    continue

                if cls == Classification.UNRESOLVABLE:
                    result.unresolvable_deps[req] = detail
                    continue

                if src_pkg is None:
                    continue

                # Track the dependency
                pkg_info.buildrequires.append(src_pkg)
                if src_pkg != pkg:  # Skip self-deps
                    edges.append((src_pkg, pkg))

                    # Always recurse into the full graph
                    should_recurse = True
                    if self.stop_at_rules and cls == Classification.HAS_RULES:
                        should_recurse = False

                    if should_recurse and src_pkg not in visited:
                        queue.append((src_pkg, depth + 1))

            # Deduplicate buildrequires
            pkg_info.buildrequires = sorted(set(pkg_info.buildrequires))
            result.packages[pkg] = pkg_info

        # Topological sort
        result.build_order, result.cycles = self._topological_sort(edges, visited)

        # Assign build order numbers and compute needed_by
        for i, pkg in enumerate(result.build_order):
            if pkg in result.packages:
                result.packages[pkg].build_order = i + 1

        for dep, dependent in edges:
            if dep in result.packages:
                if dependent not in result.packages[dep].needed_by:
                    result.packages[dep].needed_by.append(dependent)

        # Compute complexity for NEED_RULES packages
        for pkg, info in result.packages.items():
            if info.classification == Classification.NEED_RULES:
                br = self._get_buildrequires(pkg)
                info.complexity = self._estimate_complexity(pkg, br)

        # Set non_fedora_source
        for pkg, info in result.packages.items():
            if info.classification == Classification.NON_FEDORA:
                info.non_fedora_source = self._non_fedora.get(pkg, "")

        return result

    def _classify_package(self, pkg: str) -> Classification:
        """Classify a package based on mogrix state (not as a dep provider)."""
        if pkg in self._built_packages:
            has_smoke = self._built_packages[pkg]
            return (
                Classification.ALREADY_BUILT_VERIFIED
                if has_smoke
                else Classification.ALREADY_BUILT_UNVERIFIED
            )
        if pkg in self._rule_packages:
            return Classification.HAS_RULES
        if pkg in self._non_fedora:
            return Classification.NON_FEDORA
        return Classification.NEED_RULES

    def _topological_sort(
        self, edges: list[tuple[str, str]], all_nodes: set[str]
    ) -> tuple[list[str], list[list[str]]]:
        """Topological sort with SCC condensation for proper cycle ordering.

        1. Find all SCCs using Tarjan's algorithm
        2. Condense SCCs into super-nodes, creating an acyclic condensed graph
        3. Topologically sort the condensed graph (Kahn's algorithm)
        4. Expand super-nodes back, ordering nodes within each SCC by
           "fewest unsatisfied deps" (greedy heuristic for best build order)

        Returns:
            (sorted_order, cycles) where cycles is a list of SCCs with >1 node.
        """
        adjacency: dict[str, list[str]] = defaultdict(list)
        reverse_adj: dict[str, list[str]] = defaultdict(list)
        for dep, dependent in edges:
            adjacency[dep].append(dependent)
            reverse_adj[dependent].append(dep)

        # Step 1: Find all SCCs (including singletons for completeness)
        all_sccs = self._find_all_sccs(all_nodes, adjacency)
        cycles = [scc for scc in all_sccs if len(scc) > 1]

        # Build node -> SCC id mapping
        node_to_scc: dict[str, int] = {}
        for i, scc in enumerate(all_sccs):
            for node in scc:
                node_to_scc[node] = i

        # Step 2: Build condensed DAG (edges between SCCs)
        condensed_adj: dict[int, set[int]] = defaultdict(set)
        condensed_in_degree: dict[int, int] = {i: 0 for i in range(len(all_sccs))}
        for dep, dependent in edges:
            scc_dep = node_to_scc.get(dep)
            scc_dependent = node_to_scc.get(dependent)
            if scc_dep is not None and scc_dependent is not None and scc_dep != scc_dependent:
                if scc_dependent not in condensed_adj[scc_dep]:
                    condensed_adj[scc_dep].add(scc_dependent)
                    condensed_in_degree[scc_dependent] += 1

        # Step 3: Topologically sort the condensed DAG
        queue = sorted([i for i, d in condensed_in_degree.items() if d == 0])
        scc_order: list[int] = []
        while queue:
            scc_id = queue.pop(0)
            scc_order.append(scc_id)
            for neighbor in sorted(condensed_adj.get(scc_id, set())):
                condensed_in_degree[neighbor] -= 1
                if condensed_in_degree[neighbor] == 0:
                    queue.append(neighbor)
                    queue.sort()

        # Add any remaining SCCs (shouldn't happen in condensed DAG, but safety)
        for i in range(len(all_sccs)):
            if i not in set(scc_order):
                scc_order.append(i)

        # Step 4: Expand back to node order
        order: list[str] = []
        for scc_id in scc_order:
            scc = all_sccs[scc_id]
            if len(scc) == 1:
                order.append(scc[0])
            else:
                # Order SCC nodes by greedy "fewest unsatisfied deps"
                ordered_scc = self._order_scc_greedy(scc, adjacency, reverse_adj)
                order.extend(ordered_scc)

        # Add any isolated nodes not in any SCC
        order_set = set(order)
        for node in sorted(all_nodes):
            if node not in order_set:
                order.append(node)

        return order, cycles

    def _order_scc_greedy(
        self,
        scc: list[str],
        adjacency: dict[str, list[str]],
        reverse_adj: dict[str, list[str]],
    ) -> list[str]:
        """Order nodes within an SCC using a greedy heuristic.

        Picks the node with the fewest unresolved deps (within the SCC) first.
        This gives a reasonable build order for bootstrapping cycles:
        the first package is the one to build with minimal deps, then its
        dependents become unblocked, etc.
        """
        scc_set = set(scc)
        remaining = set(scc)
        resolved: set[str] = set()
        result: list[str] = []

        while remaining:
            # Count how many SCC-internal deps each remaining node has unresolved
            best = None
            best_count = float("inf")
            for node in sorted(remaining):
                # Count deps within SCC that haven't been "resolved" yet
                unresolved = sum(
                    1
                    for dep in reverse_adj.get(node, [])
                    if dep in scc_set and dep in remaining
                )
                if unresolved < best_count:
                    best_count = unresolved
                    best = node

            if best is None:
                break
            result.append(best)
            remaining.discard(best)
            resolved.add(best)

        return result

    def _find_all_sccs(
        self, nodes: set[str], adjacency: dict[str, list[str]]
    ) -> list[list[str]]:
        """Find ALL strongly connected components (including singletons).

        Uses iterative Tarjan's algorithm to avoid recursion depth issues
        on large graphs (Fedora has 2800+ nodes).
        """
        index_counter = [0]
        stack: list[str] = []
        on_stack: set[str] = set()
        index: dict[str, int] = {}
        lowlink: dict[str, int] = {}
        sccs: list[list[str]] = []

        def strongconnect(v: str):
            call_stack = [(v, 0)]
            index[v] = lowlink[v] = index_counter[0]
            index_counter[0] += 1
            stack.append(v)
            on_stack.add(v)

            while call_stack:
                node, ni = call_stack[-1]
                neighbors = [n for n in adjacency.get(node, []) if n in nodes]

                if ni < len(neighbors):
                    call_stack[-1] = (node, ni + 1)
                    w = neighbors[ni]
                    if w not in index:
                        index[w] = lowlink[w] = index_counter[0]
                        index_counter[0] += 1
                        stack.append(w)
                        on_stack.add(w)
                        call_stack.append((w, 0))
                    elif w in on_stack:
                        lowlink[node] = min(lowlink[node], index[w])
                else:
                    if lowlink[node] == index[node]:
                        scc: list[str] = []
                        while True:
                            w = stack.pop()
                            on_stack.discard(w)
                            scc.append(w)
                            if w == node:
                                break
                        sccs.append(sorted(scc))

                    call_stack.pop()
                    if call_stack:
                        parent = call_stack[-1][0]
                        lowlink[parent] = min(lowlink[parent], lowlink[node])

        for v in sorted(nodes):
            if v not in index:
                strongconnect(v)

        return sccs

    def _estimate_complexity(self, pkg: str, buildrequires: list[str]) -> str:
        """Estimate build complexity for a package.

        Returns LOW, MEDIUM, or HIGH.
        """
        score = 0
        br_set = set(buildrequires)

        if "gcc-c++" in br_set:
            score += 2
        if any(br in br_set for br in ("cmake", "meson")):
            score += 1
        if "gobject-introspection-devel" in br_set or "gobject-introspection" in br_set:
            score += 3
        if len(buildrequires) > 40:
            score += 2
        elif len(buildrequires) > 20:
            score += 1
        if not any(br in br_set for br in ("autoconf", "automake", "libtool")):
            score += 1

        if score <= 1:
            return "LOW"
        elif score <= 3:
            return "MED"
        else:
            return "HIGH"


# --- Output formatters ---


def format_text(
    result: RoadmapResult,
    list_drops: bool = False,
    show_satisfied: bool = False,
) -> str:
    """Format roadmap result as human-readable text."""
    lines: list[str] = []
    lines.append(f"Roadmap for: {result.target}")
    lines.append("")

    # Group packages by classification
    groups: dict[Classification, list[PackageInfo]] = defaultdict(list)
    for pkg in result.packages.values():
        groups[pkg.classification].append(pkg)

    # Sort each group
    for cls in groups:
        groups[cls].sort(key=lambda p: (p.build_order or 9999, p.name))

    # Already built (verified)
    verified = groups.get(Classification.ALREADY_BUILT_VERIFIED, [])
    if verified:
        names = [p.name for p in verified]
        lines.append(f"ALREADY BUILT (verified):    {len(verified)} packages")
        lines.append(f"  {', '.join(names)}")
        lines.append("")

    # Already built (unverified)
    unverified = groups.get(Classification.ALREADY_BUILT_UNVERIFIED, [])
    if unverified:
        names = [p.name for p in unverified]
        lines.append(f"ALREADY BUILT (unverified):  {len(unverified)} packages")
        lines.append(f"  {', '.join(names)}")
        lines.append("")

    # Have rules (need build)
    has_rules = groups.get(Classification.HAS_RULES, [])
    if has_rules:
        names = [p.name for p in has_rules]
        lines.append(f"HAVE RULES (need build):     {len(has_rules)} packages")
        lines.append(f"  {', '.join(names)}")
        lines.append("")

    # Non-Fedora
    non_fedora = groups.get(Classification.NON_FEDORA, [])
    if non_fedora:
        lines.append(f"NON-FEDORA:                  {len(non_fedora)} packages")
        for p in non_fedora:
            lines.append(f"  {p.name} ({p.non_fedora_source})")
        lines.append("")

    # Need new rules (build order)
    need_rules = groups.get(Classification.NEED_RULES, [])
    if need_rules:
        # Build set of packages in small cycles (<=10 packages) for marking
        small_cycle_pkgs: set[str] = set()
        for cycle in result.cycles:
            if len(cycle) <= 10:
                small_cycle_pkgs.update(cycle)

        lines.append(f"NEED NEW RULES:              {len(need_rules)} packages (in build order)")
        for i, p in enumerate(need_rules):
            cycle_marker = ""
            if p.name in small_cycle_pkgs:
                cycle_marker = " <- CIRCULAR"
            lines.append(
                f"  {i+1:3d}. {p.name:<20s} {p.complexity:<4s}  "
                f"uv run mogrix fetch {p.name} -y{cycle_marker}"
            )
        lines.append("")

    # Dropped deps — split rule drops from roadmap config drops
    if list_drops and result.dropped_deps:
        rule_drops = {k: v for k, v in result.dropped_deps.items()
                      if not v.startswith("roadmap")}
        roadmap_drops = {k: v for k, v in result.dropped_deps.items()
                         if v.startswith("roadmap")}
        if rule_drops:
            lines.append(f"DROPPED (rule hierarchy):    {len(rule_drops)} deps")
            for dep, reason in sorted(rule_drops.items()):
                lines.append(f"  {dep} ({reason})")
            lines.append("")
        if roadmap_drops:
            lines.append(f"DROPPED (roadmap config):    {len(roadmap_drops)} deps")
            for dep, reason in sorted(roadmap_drops.items()):
                lines.append(f"  {dep} ({reason})")
            lines.append("")

    # Sysroot deps
    if result.sysroot_deps:
        lines.append(f"SYSROOT:                     {len(result.sysroot_deps)} deps")
        lines.append(f"  {', '.join(sorted(result.sysroot_deps))}")
        lines.append("")

    # Unresolvable deps
    if result.unresolvable_deps:
        lines.append(f"UNRESOLVABLE:                {len(result.unresolvable_deps)} deps")
        for dep, ctx in sorted(result.unresolvable_deps.items()):
            lines.append(f"  {dep}")
        lines.append("")

    # Circular deps
    if result.cycles:
        small_cycles = [c for c in result.cycles if len(c) <= 10]
        large_cycles = [c for c in result.cycles if len(c) > 10]
        lines.append(f"CIRCULAR DEPENDENCIES:       {len(result.cycles)} cycles")
        for cycle in small_cycles:
            lines.append(f"  {' <-> '.join(cycle)}")
        for cycle in large_cycles:
            preview = ", ".join(cycle[:5])
            lines.append(f"  ({len(cycle)} packages: {preview}, ...)")
        lines.append("")

    # Summary
    summary = result.summary
    parts = []
    for cls_val, label in [
        ("need_rules", "new"),
        ("built_verified", "built+verified"),
        ("built_unverified", "built"),
        ("has_rules", "have rules"),
    ]:
        count = summary.get(cls_val, 0)
        if count:
            parts.append(f"{count} {label}")

    lines.append(f"SUMMARY: {'. '.join(parts)}.")

    return "\n".join(lines)


def format_json(result: RoadmapResult) -> str:
    """Format roadmap result as JSON."""
    data = {
        "target": result.target,
        "packages": {},
        "build_order": result.build_order,
        "circular": result.cycles,
        "dropped": result.dropped_deps,
        "sysroot": sorted(result.sysroot_deps),
        "unresolvable": result.unresolvable_deps,
        "summary": result.summary,
    }

    for name, pkg in result.packages.items():
        data["packages"][name] = {
            "status": pkg.classification.value,
            "complexity": pkg.complexity,
            "build_order": pkg.build_order,
            "buildrequires": pkg.buildrequires,
            "needed_by": pkg.needed_by,
        }

    return json.dumps(data, indent=2)


def format_tree(result: RoadmapResult, console: Console):
    """Render dependency tree using Rich Tree widget."""
    cls_icons = {
        Classification.ALREADY_BUILT_VERIFIED: "[green]OK[/green]",
        Classification.ALREADY_BUILT_UNVERIFIED: "[yellow]BUILT[/yellow]",
        Classification.HAS_RULES: "[blue]RULES[/blue]",
        Classification.NON_FEDORA: "[cyan]EXT[/cyan]",
        Classification.NEED_RULES: "[red]NEW[/red]",
        Classification.UNRESOLVABLE: "[red bold]???[/red bold]",
    }

    tree = Tree(f"[bold]{result.target}[/bold]")
    visited: set[str] = set()

    def _add_children(parent_tree: Tree, pkg_name: str, depth: int = 0):
        if depth > 10:
            parent_tree.add("[dim]...(depth limit)[/dim]")
            return

        info = result.packages.get(pkg_name)
        if not info:
            return

        for dep in sorted(info.buildrequires):
            dep_info = result.packages.get(dep)
            if not dep_info:
                continue

            icon = cls_icons.get(dep_info.classification, "")
            label = f"{dep} {icon}"

            if dep in visited:
                parent_tree.add(f"{label} [dim](see above)[/dim]")
            else:
                visited.add(dep)
                child = parent_tree.add(label)
                _add_children(child, dep, depth + 1)

    visited.add(result.target)
    _add_children(tree, result.target)
    console.print(tree)


def format_diff(result: RoadmapResult, previous_path: str) -> str:
    """Compare current roadmap against a previous JSON output."""
    with open(previous_path) as f:
        prev = json.load(f)

    prev_pkgs = prev.get("packages", {})
    curr_pkgs = result.packages

    # Find changes
    newly_built = []
    new_rules = []
    newly_added = []
    removed = []

    for name, info in curr_pkgs.items():
        prev_info = prev_pkgs.get(name)
        if prev_info is None:
            newly_added.append(name)
        elif prev_info["status"] in ("need_rules",) and info.classification in (
            Classification.ALREADY_BUILT_VERIFIED,
            Classification.ALREADY_BUILT_UNVERIFIED,
        ):
            newly_built.append(name)
        elif prev_info["status"] == "need_rules" and info.classification == Classification.HAS_RULES:
            new_rules.append(name)

    for name in prev_pkgs:
        if name not in curr_pkgs:
            removed.append(name)

    prev_need = sum(1 for p in prev_pkgs.values() if p["status"] == "need_rules")
    curr_need = sum(
        1
        for p in curr_pkgs.values()
        if p.classification == Classification.NEED_RULES
    )

    lines = [f"Since previous roadmap:"]
    if newly_built:
        lines.append(f"  NEWLY BUILT:    +{len(newly_built)} ({', '.join(newly_built)})")
    if new_rules:
        lines.append(f"  NEW RULES:      +{len(new_rules)} ({', '.join(new_rules)})")
    if newly_added:
        lines.append(f"  NEW DEPS:       +{len(newly_added)} ({', '.join(newly_added)})")
    if removed:
        lines.append(f"  REMOVED:        -{len(removed)} ({', '.join(removed)})")
    lines.append(f"  REMAINING:      {prev_need} -> {curr_need}")

    return "\n".join(lines)
