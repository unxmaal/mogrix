# pyright: basic
"""AST serializer for RPM spec files.

Walks the AST and emits text. For unmodified nodes, emits span.original_text
verbatim (round-trip fidelity). For modified nodes, reconstructs text from
the node's structured fields.

Key invariant: serialize(parse(text)) == text when no transforms are applied.
"""

from __future__ import annotations

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
    ShellLine,
    SourceSpan,
    SpecFile,
)


class ASTEmitter:
    """Serialize a SpecFile AST back to text."""

    def emit(self, spec: SpecFile) -> str:
        """Serialize the entire AST to a string."""
        return "".join(self._emit_node(n) for n in spec.nodes)

    def _emit_node(self, node: ASTNode) -> str:
        """Emit a single node. Uses original_text if the node wasn't modified."""
        if isinstance(node, SpecFile):
            return "".join(self._emit_node(n) for n in node.nodes)
        elif isinstance(node, Section):
            return self._emit_section(node)
        elif isinstance(node, Conditional):
            return self._emit_conditional(node)
        elif isinstance(node, (Comment, Blank, ShellLine, ContinuationLine,
                               PatchApp, MacroDef)):
            return node.span.original_text
        elif isinstance(node, Directive):
            return self._emit_directive(node)
        else:
            # Fallback for unknown node types
            return node.span.original_text

    def _emit_directive(self, node: Directive) -> str:
        """Emit a directive node.

        If the node's values were modified (different from raw_value parsing),
        reconstruct the line. Otherwise, emit original text.
        """
        # For now, always emit original text. The transform engine will
        # set a 'modified' flag when it changes values.
        return node.span.original_text

    def _emit_section(self, node: Section) -> str:
        """Emit a section: header + body nodes."""
        parts = [node.header]
        for child in node.body:
            parts.append(self._emit_node(child))
        return "".join(parts)

    def _emit_conditional(self, node: Conditional) -> str:
        """Emit a conditional: %if + true branch + optional %else + false branch + %endif."""
        parts = [node.if_line]
        for child in node.true_branch:
            parts.append(self._emit_node(child))
        if node.else_line is not None:
            parts.append(node.else_line)
            for child in node.false_branch:
                parts.append(self._emit_node(child))
        parts.append(node.endif_line)
        return "".join(parts)
