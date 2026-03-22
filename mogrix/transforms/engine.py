# pyright: basic
"""AST-based tree transform engine for mogrix.

Applies rule-driven modifications to a parsed spec AST. Each transform
operates on typed AST nodes instead of regex on raw text, avoiding the
structural limitations of the text-based engine.

Transforms return match counts so callers can detect zero-match rules.
"""

from __future__ import annotations

import logging
import re
from dataclasses import dataclass, field

from mogrix.parser.ast import (
    ASTNode,
    Blank,
    Comment,
    Conditional,
    ContinuationLine,
    Directive,
    DirectiveKind,
    MacroDef,
    PatchApp,
    Section,
    SectionKind,
    ShellLine,
    SourceSpan,
    SpecFile,
)

logger = logging.getLogger(__name__)


@dataclass
class TransformReport:
    """Summary of transforms applied to the AST."""

    applied: list[str] = field(default_factory=list)
    warnings: list[str] = field(default_factory=list)
    match_counts: dict[str, int] = field(default_factory=dict)

    def record(self, name: str, count: int) -> None:
        self.match_counts[name] = count
        if count > 0:
            self.applied.append(f"{name}: {count} matches")
        else:
            self.warnings.append(f"{name}: zero matches")


class ASTTransformEngine:
    """Apply rule-driven transforms to a SpecFile AST."""

    def __init__(self) -> None:
        self.report = TransformReport()

    def drop_buildrequires(self, spec: SpecFile, deps: list[str]) -> int:
        """Remove BuildRequires directives matching the given dep names."""
        drop_set = set(deps)
        count = self._drop_directives(spec.nodes, DirectiveKind.BUILD_REQUIRES, drop_set)
        self.report.record("drop_buildrequires", count)
        return count

    def drop_requires(self, spec: SpecFile, deps: list[str]) -> int:
        """Remove Requires/PreReq directives matching the given dep names."""
        drop_set = set(deps)
        count = 0
        for kind in (DirectiveKind.REQUIRES, DirectiveKind.PREREQ):
            count += self._drop_directives(spec.nodes, kind, drop_set)
        self.report.record("drop_requires", count)
        return count

    def drop_subpackages(self, spec: SpecFile, patterns: list[str]) -> int:
        """Remove entire subpackage sections matching patterns."""
        import fnmatch
        count = 0
        new_nodes: list[ASTNode] = []
        for node in spec.nodes:
            if isinstance(node, Section) and node.subpackage:
                if any(fnmatch.fnmatch(node.subpackage, p) for p in patterns):
                    count += 1
                    continue
            new_nodes.append(node)
        spec.nodes = new_nodes
        self.report.record("drop_subpackages", count)
        return count

    def flip_globals(self, spec: SpecFile, names: list[str]) -> int:
        """Flip %global values between 0 and 1."""
        name_set = set(names)
        count = 0

        def flip(nodes: list[ASTNode]) -> None:
            nonlocal count
            for node in nodes:
                if isinstance(node, MacroDef) and node.name in name_set:
                    if node.value in ("0", "1"):
                        new_val = "0" if node.value == "1" else "1"
                        # Reconstruct from parsed fields rather than str.replace,
                        # which fails on unusual formatting (extra spaces, comments)
                        orig = node.span.original_text
                        # Preserve any trailing comment or whitespace
                        line = orig.rstrip("\n")
                        reconstructed = f"%{node.keyword} {node.name} {new_val}"
                        # If the original had a trailing comment, preserve it
                        # Look for content after the value that isn't just whitespace
                        m = re.match(
                            rf"(%{node.keyword}\s+{re.escape(node.name)}\s+{re.escape(node.value)})(.*)",
                            line,
                        )
                        if m:
                            trailing = m.group(2)
                            reconstructed = f"%{node.keyword} {node.name} {new_val}{trailing}"
                        node.span.original_text = reconstructed + "\n" if orig.endswith("\n") else reconstructed
                        node.value = new_val
                        count += 1
                elif isinstance(node, (Section, Conditional)):
                    flip(self._get_children(node))

        flip(spec.nodes)
        self.report.record("flip_globals", count)
        return count

    def drop_patches(
        self, spec: SpecFile, config: str | dict[str, list[int]]
    ) -> int:
        """Comment out patch definitions and applications."""
        if config == "all":
            should_drop = lambda n: True  # noqa: E731
        elif isinstance(config, dict) and "except" in config:
            keep = set(config["except"])
            should_drop = lambda n: n not in keep  # noqa: E731
        elif isinstance(config, dict) and "only" in config:
            drop = set(config["only"])
            should_drop = lambda n: n in drop  # noqa: E731
        else:
            return 0

        count = 0

        def process(nodes: list[ASTNode]) -> None:
            nonlocal count
            for node in nodes:
                if isinstance(node, Directive) and node.kind == DirectiveKind.PATCH:
                    if node.number is not None and should_drop(node.number):
                        # Comment out the patch definition
                        node.span.original_text = "#" + node.span.original_text
                        count += 1
                elif isinstance(node, PatchApp):
                    if node.patch_number is not None and should_drop(node.patch_number):
                        node.span.original_text = "#" + node.span.original_text
                        count += 1
                elif isinstance(node, (Section, Conditional)):
                    process(self._get_children(node))

        process(spec.nodes)
        self.report.record("drop_patches", count)
        return count

    def configure_disable(self, spec: SpecFile, flags: list[str]) -> int:
        """Add --disable-{flag} to %configure invocations."""
        disable_str = " ".join(f"--disable-{f}" for f in flags)
        count = 0

        def process(nodes: list[ASTNode]) -> None:
            nonlocal count
            for node in nodes:
                if isinstance(node, ShellLine) and "%configure" in node.text:
                    node.span.original_text = node.span.original_text.replace(
                        "%configure",
                        f"%configure {disable_str}",
                        1,
                    )
                    node.text = node.text.replace(
                        "%configure",
                        f"%configure {disable_str}",
                        1,
                    )
                    count += 1
                elif isinstance(node, ContinuationLine) and "%configure" in node.logical_text:
                    # Replace in the first physical line
                    for i, line in enumerate(node.lines):
                        if "%configure" in line:
                            node.lines[i] = line.replace(
                                "%configure",
                                f"%configure {disable_str}",
                                1,
                            )
                            node.span.original_text = node.span.original_text.replace(
                                "%configure",
                                f"%configure {disable_str}",
                                1,
                            )
                            count += 1
                            break
                elif isinstance(node, (Section, Conditional)):
                    process(self._get_children(node))

        process(spec.nodes)
        self.report.record("configure_disable", count)
        return count

    def remove_conditionals(self, spec: SpecFile, names: list[str]) -> int:
        """Remove %if/%endif blocks whose condition matches a name."""
        count = 0

        def filter_conds(nodes: list[ASTNode]) -> list[ASTNode]:
            nonlocal count
            result: list[ASTNode] = []
            for node in nodes:
                if isinstance(node, Conditional):
                    if any(name in node.condition for name in names):
                        count += 1
                        continue  # Drop the entire block
                    # Recurse into branches
                    node.true_branch = filter_conds(node.true_branch)
                    node.false_branch = filter_conds(node.false_branch)
                elif isinstance(node, Section):
                    node.body = filter_conds(node.body)
                result.append(node)
            return result

        spec.nodes = filter_conds(spec.nodes)
        self.report.record("remove_conditionals", count)
        return count

    # --- Internal helpers ---

    def _drop_directives(
        self, nodes: list[ASTNode], kind: DirectiveKind, drop_set: set[str]
    ) -> int:
        """Remove or filter directives from a node list. Returns match count."""
        count = 0
        i = 0
        while i < len(nodes):
            node = nodes[i]
            if isinstance(node, Directive) and node.kind == kind:
                # Check if ANY value matches the drop set
                remaining = [
                    v for v in node.values
                    if not self._value_matches_any(v, drop_set)
                ]
                dropped = len(node.values) - len(remaining)
                if dropped > 0:
                    count += dropped
                    if not remaining:
                        # Remove entire line
                        nodes.pop(i)
                        continue
                    else:
                        # Rewrite the line with remaining values
                        new_raw = " ".join(remaining)
                        node.values = remaining
                        node.raw_value = new_raw
                        node.span.original_text = f"{node.tag}:{' ' * max(1, len(node.span.original_text.split(':')[0]) + 2 - len(node.tag) - 1)}{new_raw}\n"
            elif isinstance(node, Section):
                count += self._drop_directives(node.body, kind, drop_set)
            elif isinstance(node, Conditional):
                count += self._drop_directives(node.true_branch, kind, drop_set)
                count += self._drop_directives(node.false_branch, kind, drop_set)
            i += 1
        return count

    @staticmethod
    def _value_matches_any(value: str, drop_set: set[str]) -> bool:
        """Check if a directive value matches any entry in the drop set."""
        # Extract package name (strip version constraint)
        pkg = value.split()[0] if value else value
        # Strip %{_isa} and similar suffixes
        pkg_base = re.sub(r"%\{[^}]+\}", "", pkg)
        return pkg_base in drop_set or pkg in drop_set

    @staticmethod
    def _values_match_any(values: list[str], drop_set: set[str]) -> bool:
        """Check if ALL values in a directive match the drop set."""
        return all(
            ASTTransformEngine._value_matches_any(v, drop_set)
            for v in values
        )

    @staticmethod
    def _get_children(node: ASTNode) -> list[ASTNode]:
        """Get child nodes from a container node."""
        if isinstance(node, Section):
            return node.body
        elif isinstance(node, Conditional):
            return node.true_branch + node.false_branch
        elif isinstance(node, SpecFile):
            return node.nodes
        return []

    @staticmethod
    def _filter_nodes(
        nodes: list[ASTNode],
        predicate: object,
        counter: object = None,
    ) -> list[ASTNode]:
        """Filter nodes by predicate. Unused — left for future use."""
        return [n for n in nodes if predicate(n)]  # type: ignore[operator]

    @staticmethod
    def _increment(holder: dict[str, int], key: str) -> None:
        holder[key] = holder.get(key, 0) + 1
