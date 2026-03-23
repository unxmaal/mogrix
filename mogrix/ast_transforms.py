# pyright: basic
"""Tree-sitter AST transform engine for mogrix.

Applies structural source transforms using tree-sitter queries.
Rules target code structures (functions, classes, call sites, properties)
instead of raw text, surviving reformatting and minor refactors.

Requires: uv sync --extra ast
"""

from __future__ import annotations

import logging
import os
from dataclasses import dataclass, field

from mogrix.text_transforms import PatchError, TransformStats

log = logging.getLogger(__name__)

try:
    import tree_sitter
    TREE_SITTER_AVAILABLE = True
except ImportError:
    TREE_SITTER_AVAILABLE = False

# Map file extensions to (package_name, function_name) for tree-sitter grammars.
# Most packages expose language(), but tree-sitter-typescript has language_tsx()/language_typescript().
GRAMMAR_MAP: dict[str, tuple[str, str]] = {
    ".ts": ("tree_sitter_typescript", "language_tsx"),
    ".tsx": ("tree_sitter_typescript", "language_tsx"),
    ".js": ("tree_sitter_javascript", "language"),
    ".jsx": ("tree_sitter_javascript", "language"),
    ".py": ("tree_sitter_python", "language"),
    ".c": ("tree_sitter_c", "language"),
    ".h": ("tree_sitter_c", "language"),
    ".cpp": ("tree_sitter_cpp", "language"),
    ".cxx": ("tree_sitter_cpp", "language"),
    ".cc": ("tree_sitter_cpp", "language"),
    ".hpp": ("tree_sitter_cpp", "language"),
    ".go": ("tree_sitter_go", "language"),
    ".rs": ("tree_sitter_rust", "language"),
}

# Map language override strings to (package_name, function_name)
LANGUAGE_MAP: dict[str, tuple[str, str]] = {
    "typescript": ("tree_sitter_typescript", "language_tsx"),
    "tsx": ("tree_sitter_typescript", "language_tsx"),
    "javascript": ("tree_sitter_javascript", "language"),
    "python": ("tree_sitter_python", "language"),
    "c": ("tree_sitter_c", "language"),
    "cpp": ("tree_sitter_cpp", "language"),
    "go": ("tree_sitter_go", "language"),
    "rust": ("tree_sitter_rust", "language"),
}


@dataclass
class ASTTransformResult:
    """Result of applying AST transforms to a single file."""
    file: str
    applied: int = 0
    failed: int = 0
    errors: list[str] = field(default_factory=list)


class TemplateLoader:
    """Load and resolve tree-sitter query templates from YAML files.

    Templates live in rules/transforms/queries/{language}.yaml.
    Each template has a name, params, query pattern, capture, and action.
    Templates support inheritance (C++ inherits from C) and parameter
    substitution (${function_name} replaced with actual value).
    """

    def __init__(self):
        self._templates: dict[str, dict] = {}  # lang → {name → template}
        self._inherits: dict[str, str] = {}     # lang → parent_lang
        self._loaded = False

    def _ensure_loaded(self) -> None:
        if self._loaded:
            return
        self._loaded = True

        import yaml
        queries_dir = os.path.join(
            os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
            "rules", "transforms", "queries",
        )
        if not os.path.isdir(queries_dir):
            return

        for fname in sorted(os.listdir(queries_dir)):
            if not fname.endswith(".yaml"):
                continue
            fpath = os.path.join(queries_dir, fname)
            try:
                with open(fpath) as f:
                    data = yaml.safe_load(f) or {}
            except Exception:
                continue

            lang = data.get("language", fname.replace(".yaml", ""))
            templates = data.get("templates", {})
            self._templates[lang] = templates

            if "inherits" in data:
                self._inherits[lang] = data["inherits"]

    def resolve(
        self, template_name: str, language: str, params: dict[str, str],
    ) -> dict | None:
        """Resolve a template by name for a language.

        Returns a dict with 'query', 'capture', 'action' keys,
        with ${param} substitutions applied. Returns None if not found.
        """
        self._ensure_loaded()

        template = self._find_template(template_name, language)
        if template is None:
            return None

        # Deep copy and substitute params
        query = template.get("query", "")
        for key, value in params.items():
            query = query.replace(f"${{{key}}}", value)

        return {
            "query": query,
            "capture": template.get("capture", "body"),
            "action": template.get("action", "replace"),
        }

    def _find_template(self, name: str, language: str) -> dict | None:
        """Find a template, checking the language then its parent chain."""
        lang_templates = self._templates.get(language, {})
        if name in lang_templates:
            return lang_templates[name]

        # Check inheritance chain
        parent = self._inherits.get(language)
        if parent:
            return self._find_template(name, parent)

        return None


# Singleton template loader
_template_loader = TemplateLoader()


class ASTTransformEngine:
    """Apply tree-sitter structural queries to source files."""

    def __init__(self, source_dir: str):
        if not TREE_SITTER_AVAILABLE:
            raise RuntimeError(
                "tree-sitter not installed. Run: uv sync --extra ast"
            )
        self.source_dir = source_dir
        self._language_cache: dict[str, object] = {}

    def apply_transforms(
        self,
        transforms: list[dict],
        stats: TransformStats,
        dry_run: bool = False,
        debug_queries: bool = False,
    ) -> list[ASTTransformResult]:
        """Apply a list of AST transform rules.

        Each transform dict has:
          - file: relative path to source file
          - query: tree-sitter query string
          - capture: which @capture to target (default: first capture)
          - action: replace | remove | replace_body | comment | insert_before | remove_statement
          - replacement: text for replace/replace_body actions
          - content: text for insert_before action
          - comment_style: "line" or "block" for comment action
          - expected_count: how many matches to expect (0 = lenient)
          - language: override language detection (optional)
        """
        results: list[ASTTransformResult] = []

        for transform in transforms:
            target_file = transform.get("file", "")
            if not target_file:
                continue

            fp = os.path.join(self.source_dir, target_file)
            if not os.path.isfile(fp):
                result = ASTTransformResult(file=target_file, failed=1)
                result.errors.append(f"File not found: {target_file}")
                results.append(result)
                stats.ast_failed += 1
                continue

            result = self._apply_single(
                fp, target_file, transform, stats, dry_run, debug_queries
            )
            results.append(result)

        return results

    def _apply_single(
        self,
        filepath: str,
        rel_path: str,
        transform: dict,
        stats: TransformStats,
        dry_run: bool,
        debug_queries: bool,
    ) -> ASTTransformResult:
        """Apply a single AST transform to one file."""
        result = ASTTransformResult(file=rel_path)

        try:
            source = open(filepath, "rb").read()
        except (IOError, OSError) as e:
            result.failed = 1
            result.errors.append(f"Read error: {e}")
            stats.ast_failed += 1
            return result

        # Detect language
        lang_override = transform.get("language")
        ext = os.path.splitext(filepath)[1]
        language = self._get_language(ext, lang_override)
        if language is None:
            result.failed = 1
            result.errors.append(
                f"No tree-sitter grammar for {ext}"
                + (f" (language={lang_override})" if lang_override else "")
            )
            stats.ast_failed += 1
            return result

        # Parse
        parser = tree_sitter.Parser(language)
        tree = parser.parse(source)

        if debug_queries:
            self._print_ast(tree.root_node, rel_path)

        # Resolve query — either inline or from template
        template_name = transform.get("template")
        query_text = transform.get("query", "")

        if template_name and not query_text:
            # Resolve from template library
            # Detect language name for template lookup
            lang_name = lang_override
            if not lang_name:
                for lname, (pkg, func) in LANGUAGE_MAP.items():
                    ext_info = GRAMMAR_MAP.get(ext)
                    if ext_info and ext_info == (pkg, func):
                        lang_name = lname
                        break
            if not lang_name:
                lang_name = ext.lstrip(".")

            params = transform.get("params", {})
            resolved = _template_loader.resolve(template_name, lang_name, params)
            if resolved is None:
                result.failed = 1
                result.errors.append(
                    f"Template '{template_name}' not found for language '{lang_name}'"
                )
                stats.ast_failed += 1
                return result

            query_text = resolved["query"]
            # Template provides defaults; transform can override
            if "capture" not in transform:
                transform = {**transform, "capture": resolved["capture"]}
            if "action" not in transform:
                transform = {**transform, "action": resolved["action"]}
            # For replace actions, use replacement_body param if no explicit replacement
            if "replacement" not in transform and "replacement_body" in params:
                transform = {**transform, "replacement": params["replacement_body"]}

        if not query_text:
            result.failed = 1
            result.errors.append("No query or template specified")
            stats.ast_failed += 1
            return result

        try:
            query = tree_sitter.Query(language, query_text)
        except Exception as e:
            result.failed = 1
            result.errors.append(f"Query parse error: {e}")
            stats.ast_failed += 1
            return result

        cursor = tree_sitter.QueryCursor(query)
        captures = cursor.captures(tree.root_node)
        # captures is dict[str, list[Node]]

        target_capture = transform.get("capture", None)

        if target_capture and target_capture in captures:
            nodes = captures[target_capture]
        elif captures:
            first_key = next(iter(captures))
            nodes = captures[first_key]
            if target_capture:
                log.warning(
                    "Capture @%s not found, using @%s",
                    target_capture, first_key,
                )
        else:
            nodes = []

        # Validate match count
        expected = transform.get("expected_count", -1)
        match_count = len(nodes)

        if expected > 0 and match_count == 0:
            err = PatchError(
                context="ast_transform",
                file=rel_path,
                pattern=query_text[:120],
                expected=expected,
                found=0,
            )
            stats.errors.append(err)
            stats.ast_failed += 1
            result.failed = 1
            result.errors.append(f"No matches (expected {expected})")
            log.warning("AST MISS: %s in %s", query_text[:60], rel_path)
            return result

        if expected > 0 and match_count != expected:
            err = PatchError(
                context="ast_transform",
                file=rel_path,
                pattern=query_text[:120],
                expected=expected,
                found=match_count,
            )
            stats.errors.append(err)
            log.warning(
                "AST COUNT: expected %d, found %d in %s",
                expected, match_count, rel_path,
            )

        if match_count == 0:
            return result

        if debug_queries:
            log.info(
                "  Query matched %d node(s) in %s",
                match_count, rel_path,
            )
            for node in nodes:
                log.info(
                    "    @%s: %s [%d:%d - %d:%d] (%d bytes)",
                    target_capture or "match",
                    node.type,
                    node.start_point[0], node.start_point[1],
                    node.end_point[0], node.end_point[1],
                    node.end_byte - node.start_byte,
                )

        # Apply action
        action = transform.get("action", "replace")
        new_source = self._apply_action(source, nodes, action, transform)

        if new_source != source:
            if dry_run:
                log.info(
                    "  [dry-run] would apply AST %s to %s (%d match(es))",
                    action, rel_path, match_count,
                )
            else:
                open(filepath, "wb").write(new_source)
                log.info(
                    "  AST %s: %s (%d match(es))",
                    action, rel_path, match_count,
                )
            result.applied = match_count
            stats.ast_applied += match_count
            stats.files_modified.add(rel_path)
        else:
            log.debug("  AST %s: no change in %s", action, rel_path)

        return result

    def _apply_action(
        self,
        source: bytes,
        nodes: list,
        action: str,
        transform: dict,
    ) -> bytes:
        """Apply a transform action to source using captured nodes.

        Replacements are applied in reverse byte order to avoid
        offset invalidation.
        """
        # Sort nodes by start_byte descending (reverse order)
        sorted_nodes = sorted(nodes, key=lambda n: n.start_byte, reverse=True)

        result = bytearray(source)

        for node in sorted_nodes:
            start = node.start_byte
            end = node.end_byte

            if action == "replace":
                replacement = transform.get("replacement", "").encode("utf-8")
                result[start:end] = replacement

            elif action == "remove":
                # Remove the node and any trailing whitespace/newline
                remove_end = end
                while remove_end < len(result) and result[remove_end:remove_end+1] in (b" ", b"\t"):
                    remove_end += 1
                if remove_end < len(result) and result[remove_end:remove_end+1] == b"\n":
                    remove_end += 1
                result[start:remove_end] = b""

            elif action == "replace_body":
                replacement = transform.get("replacement", "").encode("utf-8")
                result[start:end] = replacement

            elif action == "comment":
                style = transform.get("comment_style", "line")
                text = source[start:end].decode("utf-8", errors="replace")
                if style == "block":
                    commented = f"/* {text} */".encode("utf-8")
                else:
                    lines = text.split("\n")
                    commented = "\n".join(f"// {line}" for line in lines).encode("utf-8")
                result[start:end] = commented

            elif action == "insert_before":
                content = transform.get("content", "").encode("utf-8")
                result[start:start] = content

            elif action == "remove_statement":
                # Walk up to find the enclosing statement and remove it
                # including any trailing comma/semicolon
                stmt_node = node.parent
                while stmt_node and stmt_node.type not in (
                    "expression_statement", "pair", "property",
                    "field_declaration", "import_statement",
                    "export_statement", "lexical_declaration",
                    "variable_declaration",
                ):
                    if stmt_node.parent is None:
                        break
                    stmt_node = stmt_node.parent

                if stmt_node:
                    s = stmt_node.start_byte
                    e = stmt_node.end_byte
                    # Remove trailing comma if present
                    while e < len(result) and result[e:e+1] in (b" ", b"\t"):
                        e += 1
                    if e < len(result) and result[e:e+1] == b",":
                        e += 1
                    # Remove trailing newline
                    while e < len(result) and result[e:e+1] in (b" ", b"\t"):
                        e += 1
                    if e < len(result) and result[e:e+1] == b"\n":
                        e += 1
                    result[s:e] = b""
                else:
                    # Fallback: just remove the node
                    result[start:end] = b""

            else:
                log.warning("Unknown action: %s", action)

        return bytes(result)

    def _get_language(self, ext: str, override: str | None = None):
        """Get tree-sitter Language for a file extension or language name."""
        if override:
            info = LANGUAGE_MAP.get(override)
        else:
            info = GRAMMAR_MAP.get(ext)

        if info is None:
            return None

        pkg_name, func_name = info
        cache_key = f"{pkg_name}:{func_name}"

        if cache_key in self._language_cache:
            return self._language_cache[cache_key]

        try:
            mod = __import__(pkg_name)
            lang_func = getattr(mod, func_name)
            lang_ptr = lang_func()
            # tree-sitter 0.24+ grammar packages return PyCapsule;
            # wrap in tree_sitter.Language for the Parser API
            language = tree_sitter.Language(lang_ptr)
            self._language_cache[cache_key] = language
            return language
        except (ImportError, AttributeError) as e:
            log.warning("Grammar %s.%s not available: %s", pkg_name, func_name, e)
            return None

    def _print_ast(self, node, filepath: str, indent: int = 0, max_depth: int = 6):
        """Print AST structure for debugging."""
        if indent == 0:
            log.info("\nAST for %s:", filepath)

        if indent > max_depth:
            return

        prefix = "  " * indent
        text_preview = ""
        if node.child_count == 0:
            raw = node.text.decode("utf-8", errors="replace")[:40]
            text_preview = f' "{raw}"' if raw.strip() else ""

        log.info(
            "%s%s [%d:%d - %d:%d]%s",
            prefix,
            node.type,
            node.start_point[0], node.start_point[1],
            node.end_point[0], node.end_point[1],
            text_preview,
        )

        for child in node.children:
            self._print_ast(child, filepath, indent + 1, max_depth)
