"""Mogrix plugin for mcm-engine.

Domain-specific extensions for the mogrix cross-compilation knowledge system:
- boundaries table (ABI crossing maps for golang-irix)
- tasks table (cross-session task tracking)
- check_compat tool (searches compat/catalog.yaml)
- sync_rules override (handles .yaml package rules + .md methods)
- Backward-compatible tool aliases (knowledge_query, report_finding)
"""
from __future__ import annotations

import re
from pathlib import Path

import yaml

from mcm_engine.plugin import MCMPlugin, SearchScope
from mcm_engine.db import KnowledgeDB
from mcm_engine.tracker import SessionTracker


def _parse_yaml_rule(path: Path) -> dict:
    """Extract metadata from a mogrix YAML rule file.

    Returns dict with title, keywords, category, description.
    """
    try:
        content = path.read_text(encoding="utf-8")
    except (OSError, UnicodeDecodeError):
        return {}

    try:
        data = yaml.safe_load(content)
    except yaml.YAMLError:
        return {}

    if not isinstance(data, dict):
        return {}

    result: dict[str, str] = {}

    # Title from package name or filename
    pkg = data.get("package", "")
    if pkg:
        result["title"] = f"{pkg} package rules"
    else:
        result["title"] = f"{path.stem} package rules"

    # Description from first comment line
    for line in content.split("\n"):
        stripped = line.strip()
        if stripped.startswith("#"):
            desc = stripped.lstrip("# ").strip()
            if desc:
                result["description"] = desc
                break

    # Keywords: package name + rule type names
    kw_parts = [pkg or path.stem]
    rules = data.get("rules", {})
    if isinstance(rules, dict):
        kw_parts.extend(rules.keys())
    result["keywords"] = ", ".join(kw_parts)

    result["category"] = "packages"
    return result


def _parse_md_rule(path: Path) -> dict:
    """Extract metadata from a markdown rule file.

    Expected format:
        # Title
        **Keywords:** kw1, kw2
        **Category:** cat
        Body text...
    """
    try:
        content = path.read_text(encoding="utf-8")
    except (OSError, UnicodeDecodeError):
        return {}

    result: dict[str, str] = {}
    lines = content.split("\n")

    for line in lines:
        stripped = line.strip()
        if stripped.startswith("# ") and not stripped.startswith("## "):
            result["title"] = stripped[2:].strip()
            break

    for line in lines:
        stripped = line.strip()
        kw_match = re.match(r"\*\*Keywords?:\*\*\s*(.+)", stripped, re.IGNORECASE)
        if kw_match:
            result["keywords"] = kw_match.group(1).strip()
            continue
        cat_match = re.match(r"\*\*Category:\*\*\s*(.+)", stripped, re.IGNORECASE)
        if cat_match:
            result["category"] = cat_match.group(1).strip()
            continue

    in_body = False
    desc_lines: list[str] = []
    for line in lines:
        stripped = line.strip()
        if not in_body:
            if stripped.startswith("# ") and not stripped.startswith("## "):
                continue
            if re.match(r"\*\*(Keywords?|Category):\*\*", stripped, re.IGNORECASE):
                continue
            if stripped == "":
                continue
            in_body = True
        if in_body:
            if stripped == "" and desc_lines:
                break
            if stripped.startswith("## "):
                break
            desc_lines.append(stripped)

    if desc_lines:
        result["description"] = " ".join(desc_lines)

    return result


def _is_real_file_path(fp: str) -> bool:
    """Heuristic: does this look like a real file path vs. free text?

    Rejects globs, comma-separated lists, plain English phrases, N/A, etc.
    """
    if not fp:
        return False
    # Reject known non-path patterns
    if fp in ("N/A", "n/a", "none", "None"):
        return False
    if "*" in fp or "," in fp:
        return False
    # Must contain a dot or slash to look like a path
    if "." not in fp and "/" not in fp:
        return False
    # Reject if it contains spaces (likely free text like "Bundle wrappers")
    if " " in fp:
        return False
    return True


class CompatCatalog:
    """Parses compat/catalog.yaml on demand. No DB mirror.

    Ported from knowledge-server.py CompatCatalog.
    """

    def __init__(self, catalog_path: Path):
        self.catalog_path = catalog_path

    def search(self, symbol: str) -> dict | None:
        """Search catalog for a symbol. Returns match info or None."""
        try:
            with open(self.catalog_path) as f:
                data = yaml.safe_load(f)
        except FileNotFoundError:
            return {"error": f"Catalog not found: {self.catalog_path}"}
        except Exception as e:
            return {"error": f"Catalog parse error: {e}"}

        functions = data.get("functions", {})
        symbol_lower = symbol.lower()

        # Direct name match
        for name, info in functions.items():
            if name.lower() == symbol_lower:
                result = {
                    "found": True,
                    "name": name,
                    "file": info.get("file", ""),
                    "header": info.get("header", ""),
                    "description": info.get("description", ""),
                }
                if info.get("notes"):
                    result["notes"] = info["notes"]
                if info.get("provides"):
                    result["provides"] = info["provides"]
                if info.get("source_patterns"):
                    result["source_patterns"] = info["source_patterns"]
                return result

        # Check provides lists
        for name, info in functions.items():
            provides = info.get("provides", [])
            if isinstance(provides, list):
                for p in provides:
                    if isinstance(p, str) and p.lower() == symbol_lower:
                        return {
                            "found": True,
                            "name": name,
                            "matched_via": f"provides: {p}",
                            "file": info.get("file", ""),
                            "header": info.get("header", ""),
                            "description": info.get("description", ""),
                        }

        # Check source_patterns
        for name, info in functions.items():
            patterns = info.get("source_patterns", [])
            if isinstance(patterns, list):
                for sp in patterns:
                    pat = sp.get("pattern", "") if isinstance(sp, dict) else str(sp)
                    if symbol_lower in pat.lower():
                        return {
                            "found": True,
                            "name": name,
                            "matched_via": f"source_pattern: {pat}",
                            "file": info.get("file", ""),
                            "header": info.get("header", ""),
                            "description": info.get("description", ""),
                        }

        return {"found": False, "symbol": symbol}


def _sweep_check(project_root: Path, keywords: str, title: str, file_path: str) -> str | None:
    """Check if a fix might apply to other packages.

    Scans all package YAML files for patterns similar to the one being fixed.
    Returns a sweep advisory string if other packages might be affected, or None.
    """
    # Extract sweep-worthy patterns from keywords and title
    patterns_to_check: list[tuple[str, str]] = []  # (pattern_regex, description)
    combined = f"{keywords} {title}".lower()

    # soname_spec / library_names_spec issues
    if "soname" in combined or "library_names" in combined:
        patterns_to_check.append((r"soname_spec|library_names_spec", "custom soname_spec/library_names_spec"))

    # ac_cv override patterns
    if "ac_cv_c_undeclared_builtin" in combined:
        # Check which packages use autoreconf (may need this override)
        patterns_to_check.append((r"autoreconf|autoconf", "autoreconf (may need ac_cv_c_undeclared_builtin_options)"))

    # libtool fix patterns
    if "libtool" in combined and ("shared_ext" in combined or "version_type" in combined):
        patterns_to_check.append((r"fix-libtool|soname_spec|shared_ext", "custom libtool modifications"))

    # Double-dot or naming issues
    if "double.dot" in combined or "double-dot" in combined or "so.." in combined:
        patterns_to_check.append((r"soname_spec|library_names_spec|\$major|\$shared_ext", "soname/library naming"))

    if not patterns_to_check:
        return None

    pkg_dir = project_root / "rules" / "packages"
    if not pkg_dir.exists():
        return None

    # Get the package being fixed (skip it in results)
    fixed_pkg = ""
    if file_path:
        fixed_pkg = Path(file_path).stem

    affected: list[tuple[str, str]] = []  # (package, matched_pattern_desc)

    for yaml_file in sorted(pkg_dir.glob("*.yaml")):
        if yaml_file.stem == fixed_pkg:
            continue
        try:
            content = yaml_file.read_text(encoding="utf-8")
        except (OSError, UnicodeDecodeError):
            continue

        for pat_regex, pat_desc in patterns_to_check:
            if re.search(pat_regex, content, re.IGNORECASE):
                affected.append((yaml_file.stem, pat_desc))
                break  # One match per package is enough

    if not affected:
        return None

    lines = [
        f"\n⚠️  SWEEP NEEDED — {len(affected)} other package(s) may have the same defect class:",
    ]
    for pkg, desc in affected[:15]:
        lines.append(f"  - {pkg} ({desc})")
    if len(affected) > 15:
        lines.append(f"  ... and {len(affected) - 15} more")
    lines.append(f"\nDefect: {title}")
    lines.append("ACTION: Verify these packages don't have the same issue. "
                  "Create a task to track the sweep if not done immediately.")
    return "\n".join(lines)


class MogrixPlugin(MCMPlugin):
    """Mogrix domain plugin for mcm-engine."""

    @property
    def name(self) -> str:
        return "mogrix"

    @property
    def version(self) -> int:
        return 2

    def get_schema_sql(self) -> str:
        return """
        CREATE TABLE IF NOT EXISTS boundaries (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            system TEXT NOT NULL,
            transition TEXT NOT NULL,
            register_or_state TEXT NOT NULL,
            value_before TEXT,
            value_after TEXT,
            who_restores TEXT,
            verified INTEGER DEFAULT 0,
            notes TEXT,
            project TEXT DEFAULT 'golang-irix',
            created TEXT DEFAULT (datetime('now'))
        );

        CREATE INDEX IF NOT EXISTS idx_boundaries_system ON boundaries(system);

        CREATE VIRTUAL TABLE IF NOT EXISTS boundaries_fts USING fts5(
            system, transition, register_or_state, notes,
            content='boundaries',
            content_rowid='id'
        );

        CREATE TRIGGER IF NOT EXISTS boundaries_ai AFTER INSERT ON boundaries BEGIN
            INSERT INTO boundaries_fts(rowid, system, transition, register_or_state, notes)
            VALUES (new.id, new.system, new.transition, new.register_or_state, new.notes);
        END;

        CREATE TRIGGER IF NOT EXISTS boundaries_au AFTER UPDATE ON boundaries BEGIN
            DELETE FROM boundaries_fts WHERE rowid = old.id;
            INSERT INTO boundaries_fts(rowid, system, transition, register_or_state, notes)
            VALUES (new.id, new.system, new.transition, new.register_or_state, new.notes);
        END;

        CREATE TRIGGER IF NOT EXISTS boundaries_ad AFTER DELETE ON boundaries BEGIN
            DELETE FROM boundaries_fts WHERE rowid = old.id;
        END;

        CREATE TABLE IF NOT EXISTS tasks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            subject TEXT NOT NULL,
            status TEXT NOT NULL DEFAULT 'active'
                CHECK(status IN ('active', 'blocked', 'completed', 'dropped')),
            description TEXT,
            blockers TEXT,
            project TEXT DEFAULT 'mogrix',
            created TEXT DEFAULT (datetime('now')),
            updated TEXT DEFAULT (datetime('now'))
        );

        CREATE INDEX IF NOT EXISTS idx_tasks_status ON tasks(status);
        CREATE INDEX IF NOT EXISTS idx_tasks_project ON tasks(project);
        """

    def get_search_scopes(self) -> list[SearchScope]:
        return [
            SearchScope(
                name="boundaries",
                label="BOUNDARY",
                fts_table="boundaries_fts",
                base_table="boundaries",
                fts_columns=["system", "transition", "register_or_state", "notes"],
                display_columns=["system", "transition", "register_or_state",
                                 "value_before", "value_after", "who_restores"],
                like_columns=["system", "transition", "register_or_state", "notes"],
                format_fn=self._format_boundary,
            ),
        ]

    @staticmethod
    def _format_boundary(row) -> str:
        parts = [f"[BOUNDARY] {row['system']}: {row['transition']}"]
        parts.append(f"  {row['register_or_state']}: {row['value_before'] or '?'} -> {row['value_after'] or '?'}")
        if row['who_restores']:
            parts.append(f"  Restored by: {row['who_restores']}")
        return "\n".join(parts)

    def register_tools(self, server) -> None:
        """Register mogrix-specific tools and backward-compat aliases."""
        mcp = server.mcp
        db = server.db
        tracker = server.tracker

        # Track which MCP tools the agent has called this session.
        # Used by get_nudge() to enforce the search-first mandate.
        self._mcp_tools_called: set[str] = set()

        # Register the mcp_first nudge type so the escalation system
        # can block the agent if they ignore it.
        tracker.RESOLVES["mcp_first"] = frozenset({
            "search", "report_error", "check_compat", "knowledge_query",
        })

        # Resolve paths relative to project root
        project_root = server.project_root
        catalog_path = project_root / "compat" / "catalog.yaml"
        catalog = CompatCatalog(catalog_path)

        # Get search_all_fn from server for backward-compat alias
        search_all_fn = getattr(server, "_search_all_fn", None)

        # Get rules_paths from config for sync_rules override
        from mcm_engine.config import load_config
        config = load_config(project_root=project_root)
        rules_paths = config.resolve_rules_paths(project_root)

        @mcp.tool()
        def check_compat(symbol: str) -> str:
            """Search compat/catalog.yaml for a symbol implementation.

            Use this BEFORE writing compat functions — many IRIX-missing POSIX
            functions already have implementations in compat/.

            Args:
                symbol: Function/symbol name to search for
            """
            tracker.record_call("check_compat", topic=symbol)
            result = catalog.search(symbol)

            if result is None:
                return f"'{symbol}' not found in compat catalog."

            if result.get("error"):
                return f"Error: {result['error']}"

            if not result.get("found"):
                return (
                    f"'{symbol}' not in compat catalog.\n\n"
                    "If you need this function, create it in compat/ and add to "
                    "compat/catalog.yaml. See rules/methods/compat-functions.md."
                )

            lines = [f"## Compat: {result['name']}\n"]
            lines.append(f"- **File**: compat/{result['file']}")
            lines.append(f"- **Header**: {result.get('header', 'N/A')}")
            lines.append(f"- **Description**: {result.get('description', '')}")
            if result.get("matched_via"):
                lines.append(f"- **Matched via**: {result['matched_via']}")
            if result.get("notes"):
                lines.append(f"- **Notes**: {result['notes']}")
            if result.get("provides"):
                lines.append(f"- **Also provides**: {', '.join(result['provides'])}")
            if result.get("source_patterns"):
                lines.append(f"- **Source patterns**: {result['source_patterns']}")

            return "\n".join(lines)

        # --- Backward-compat aliases ---

        @mcp.tool()
        def knowledge_query(query: str, limit: int = 10) -> str:
            """Search mogrix rules database by keywords.

            Backward-compatible alias for the `search` tool. Searches all knowledge
            scopes: rules, knowledge, errors, negative knowledge, boundaries.

            Args:
                query: Search keywords (e.g., 'dlmalloc', 'rld crash', 'setenv')
                limit: Max results (default: 10)
            """
            tracker.record_call("knowledge_query", topic=query)
            if search_all_fn:
                results = search_all_fn(query, limit=limit)
                if results:
                    return results
                return f"No results for: {query}"
            return "Search function not available. Use the `search` tool directly."

        @mcp.tool()
        def report_finding(
            type: str,
            detail: str,
            topic: str = "general",
            package: str = "",
            rationale: str = "",
            severity: str = "normal",
        ) -> str:
            """Store a discovery, decision, or negative knowledge.

            Backward-compatible wrapper. Routes to add_knowledge or add_negative
            based on type.

            Args:
                type: 'finding', 'decision', or 'negative'
                detail: The finding/decision/negative knowledge text
                topic: Topic or area (e.g., 'rld', 'signal_handling')
                package: Package name if applicable
                rationale: For decisions — why this choice was made
                severity: For negative knowledge — 'normal' or 'critical'
            """
            tracker.record_call("report_finding", topic=topic)
            tracker.record_store()

            if type == "negative":
                category = topic if topic != "general" else "failed_attempt"
                db.execute_write(
                    "INSERT INTO negative_knowledge "
                    "(category, what_failed, why_failed, correct_approach, severity, project) "
                    "VALUES (?, ?, ?, ?, ?, ?)",
                    (category, detail, "", "", severity, package or "mogrix"),
                )
                db.commit()
                return f"Negative knowledge stored: [{category}] {detail[:100]}..."

            kind = "decision" if type == "decision" else "finding"
            db.execute_write(
                "INSERT INTO knowledge "
                "(topic, kind, summary, detail, tags, project, rationale, alternatives) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
                (topic, kind, detail[:200], detail, "", package or "mogrix", rationale, ""),
            )
            db.commit()
            entry_id = db.execute("SELECT last_insert_rowid()").fetchone()[0]
            return f"{kind.title()} #{entry_id} stored: [{topic}] {detail[:100]}..."

        # --- Override sync_rules to handle .yaml package rules ---
        # FastMCP keeps first registration, so we must remove the core tool first
        if hasattr(mcp, "_tool_manager") and "sync_rules" in mcp._tool_manager._tools:
            del mcp._tool_manager._tools["sync_rules"]

        @mcp.tool()
        def sync_rules() -> str:
            """Re-index all rule files (.md and .yaml) across configured rules directories.

            Mogrix override: also indexes YAML package rule files and only orphan-deletes
            rules whose file_path looks like a real file path (not free text or globs
            from legacy migration).
            """
            tracker.record_call("sync_rules")

            rule_files: list[tuple[Path, str]] = []  # (path, type)
            missing_paths: list[str] = []

            for rp in rules_paths:
                if rp.exists():
                    rule_files.extend((f, "md") for f in sorted(rp.rglob("*.md")))
                    rule_files.extend((f, "yaml") for f in sorted(rp.rglob("*.yaml")))
                else:
                    missing_paths.append(str(rp))

            if not rule_files and missing_paths:
                return f"No rules directories found: {', '.join(missing_paths)}"

            indexed = 0
            updated = 0
            removed = 0

            for rule_file, ftype in rule_files:
                try:
                    rel_path = str(rule_file.relative_to(project_root))
                except ValueError:
                    rel_path = str(rule_file)

                if ftype == "yaml":
                    parsed = _parse_yaml_rule(rule_file)
                else:
                    parsed = _parse_md_rule(rule_file)

                if not parsed.get("title"):
                    continue

                title = parsed["title"]
                keywords = parsed.get("keywords", "")
                category = parsed.get("category", "")
                description = parsed.get("description", "")

                existing = db.execute(
                    "SELECT id FROM rules WHERE file_path = ?", (rel_path,)
                ).fetchone()

                if existing:
                    db.execute_write(
                        "UPDATE rules SET title = ?, keywords = ?, description = ?, "
                        "category = ?, updated_at = datetime('now') WHERE id = ?",
                        (title, keywords, description, category, existing["id"]),
                    )
                    updated += 1
                else:
                    db.execute_write(
                        "INSERT INTO rules (title, keywords, file_path, description, category) "
                        "VALUES (?, ?, ?, ?, ?)",
                        (title, keywords, rel_path, description, category),
                    )
                    indexed += 1

            # Orphan removal: only delete rules whose file_path was found during
            # this scan (i.e., files we actually manage). Legacy migrated rules
            # with bare filenames, free text, or partial paths are left alone —
            # they contain valuable knowledge even if the path doesn't resolve.
            scanned_paths = {str(f.relative_to(project_root)) if f.is_relative_to(project_root) else str(f)
                            for f, _ in rule_files}
            all_rules = db.execute(
                "SELECT id, file_path FROM rules WHERE file_path IS NOT NULL AND file_path != ''"
            ).fetchall()
            for rule in all_rules:
                fp = rule["file_path"]
                if fp not in scanned_paths:
                    continue  # Not a managed file — leave it alone
                full = project_root / fp
                if not full.exists():
                    db.execute_write("DELETE FROM rules WHERE id = ?", (rule["id"],))
                    removed += 1

            db.commit()
            return f"Sync complete: {indexed} new, {updated} updated, {removed} orphans removed."

        # --- Override add_rule to add defect sweep check ---
        if hasattr(mcp, "_tool_manager") and "add_rule" in mcp._tool_manager._tools:
            del mcp._tool_manager._tools["add_rule"]

        @mcp.tool()
        def add_rule(
            title: str,
            keywords: str,
            content: str = "",
            category: str = "",
            file_path: str = "",
        ) -> str:
            """Create or index a rule file, with automatic defect sweep check.

            Mogrix override: after storing the rule, scans all package YAML files
            for the same defect pattern. If other packages may be affected, returns
            a SWEEP NEEDED advisory.

            Args:
                title: Rule title
                keywords: Comma-separated search keywords
                content: Rule body text (used when creating a new file)
                category: Rule category (used for directory organization)
                file_path: Relative path to existing rule file (indexes it if provided)
            """
            tracker.record_call("add_rule", topic=title)
            tracker.record_store()

            # Check for duplicate by title
            existing = db.execute(
                "SELECT id, file_path FROM rules WHERE title = ?", (title,)
            ).fetchone()

            if existing:
                db.execute_write(
                    "UPDATE rules SET keywords = ?, description = ?, category = ?, "
                    "file_path = ?, updated_at = datetime('now') WHERE id = ?",
                    (keywords, content[:500] if content else "", category,
                     file_path or existing["file_path"], existing["id"]),
                )
                db.commit()
                result_msg = f"Updated existing rule: {title} (id={existing['id']})"
            else:
                if file_path:
                    full = project_root / file_path
                    if full.exists() and not content:
                        parsed = _parse_yaml_rule(full) if file_path.endswith(".yaml") else _parse_md_rule(full)
                        if parsed.get("description"):
                            content = parsed["description"]

                db.execute_write(
                    "INSERT INTO rules (title, keywords, file_path, description, category) "
                    "VALUES (?, ?, ?, ?, ?)",
                    (title, keywords, file_path, content[:500] if content else "", category),
                )
                db.commit()
                entry_id = db.execute("SELECT last_insert_rowid()").fetchone()[0]
                result_msg = f"Rule #{entry_id} added: {title}"
                if file_path:
                    result_msg += f"\n  File: {file_path}"

            # Defect sweep check
            sweep = _sweep_check(project_root, keywords, title, file_path)
            if sweep:
                result_msg += sweep

                # Auto-create sweep task in DB
                sweep_subject = f"Defect sweep: {title[:80]}"
                existing_task = db.execute(
                    "SELECT id FROM tasks WHERE subject = ? AND status = 'active'",
                    (sweep_subject,)
                ).fetchone()
                if not existing_task:
                    db.execute_write(
                        "INSERT INTO tasks (subject, status, description, project) "
                        "VALUES (?, 'active', ?, 'mogrix')",
                        (sweep_subject, sweep),
                    )
                    db.commit()
                    result_msg += "\n📋 Sweep task created in knowledge DB."

            # Append nudge if applicable
            nudge = tracker.get_nudge()
            if nudge:
                result_msg += f"\n\n---\n{nudge}"

            return result_msg

        # --- Override session_handoff to auto-populate findings_summary ---
        # The core mcm-engine session_handoff hardcodes findings_summary="".
        # This override computes it from the DB and warns if nothing was stored.
        import json as _json

        @mcp.tool()
        def session_handoff(
            status: str,
            current_task: str = "",
            next_steps: str = "",
            blockers: str = "",
        ) -> str:
            """Snapshot session state for the next session to pick up.

            Resets ALL behavioral counters. Automatically creates a final snapshot
            with the handoff data. Call this before ending a session or when nudged
            to checkpoint.

            Args:
                status: One-line session status summary
                current_task: What's currently in progress
                next_steps: What the next session should do
                blockers: Any blockers or decisions needed
            """
            tracker.record_call("session_handoff")

            # Compute findings_summary from DB before resetting
            turn_count = tracker.turn_count
            duration = tracker.elapsed_seconds()

            # Count knowledge/errors/rules stored this session (last 24h as proxy)
            counts = {}
            for table in ("knowledge", "errors", "rules"):
                try:
                    row = db.execute(
                        f"SELECT COUNT(*) as cnt FROM {table} "
                        "WHERE created >= datetime('now', '-1 day')"
                    ).fetchone()
                    if row and row["cnt"] > 0:
                        counts[table] = row["cnt"]
                except Exception:
                    pass

            if counts:
                findings_summary = ", ".join(f"{v} {k}" for k, v in counts.items())
            else:
                findings_summary = ""

            # Pre-handoff warning if nothing stored in a non-trivial session
            warning = ""
            if not counts and turn_count > 10:
                warning = (
                    "\nWARNING: No findings stored this session "
                    f"({turn_count} tool calls). Consider calling "
                    "add_knowledge or add_rule before handoff."
                )

            tracker.reset_all()

            context = _json.dumps({
                "turn_count": turn_count,
                "session_duration_s": duration,
                "knowledge_stored": sum(counts.values()) if counts else 0,
            })

            db.execute_write(
                "INSERT INTO sessions "
                "(status, current_task, findings_summary, next_steps, blockers, context_snapshot) "
                "VALUES (?, ?, ?, ?, ?, ?)",
                (status, current_task, findings_summary, next_steps, blockers, context),
            )
            db.commit()

            # Auto-create a final snapshot
            session_row = db.execute(
                "SELECT id FROM sessions ORDER BY id DESC LIMIT 1"
            ).fetchone()
            session_id = session_row["id"] if session_row else None

            if session_id is not None:
                seq_row = db.execute(
                    "SELECT COALESCE(MAX(sequence_num), 0) + 1 as next_seq "
                    "FROM snapshots WHERE session_id = ?",
                    (session_id,),
                ).fetchone()
            else:
                seq_row = db.execute(
                    "SELECT COALESCE(MAX(sequence_num), 0) + 1 as next_seq "
                    "FROM snapshots WHERE session_id IS NULL",
                ).fetchone()
            next_seq = seq_row["next_seq"]

            try:
                db.execute_write(
                    "INSERT INTO snapshots "
                    "(session_id, sequence_num, goal, progress, next_steps, blockers) "
                    "VALUES (?, ?, ?, ?, ?, ?)",
                    (session_id, next_seq, current_task, status, next_steps, blockers),
                )
                db.commit()
            except Exception:
                pass

            result = "Session handoff recorded. Counters reset."
            if findings_summary:
                result += f"\nFindings stored: {findings_summary}"
            result += warning
            return result

    def get_nudge(self, tracker: SessionTracker) -> str | None:
        """Domain nudge: enforce MCP-first workflow.

        Tiered enforcement:
        - Turn 3-5: WARNING if no search/report_error/check_compat called
        - Turn 6+: Adds "mcp_first" to pending_nudges so escalation system
          blocks after 2 ignores (nudge_escalation_threshold=2)
        - Every 15 turns: periodic reminder to store findings
        """
        if tracker.turn_count < 3:
            return None

        # The mcp_first nudge lifecycle:
        # 1. Not yet fired: "mcp_first" absent from pending AND ignored
        # 2. Fired, awaiting resolution: "mcp_first" in pending_nudges
        # 3. Resolved: record_call() removed it from pending+ignored
        #
        # We use a separate flag to track whether we've ever fired it.
        # Once resolved (agent called search/report_error/check_compat),
        # we stop firing.
        mcp_first_ever_fired = hasattr(self, "_mcp_first_fired") and self._mcp_first_fired
        mcp_first_resolved = (
            mcp_first_ever_fired
            and "mcp_first" not in tracker.pending_nudges
            and "mcp_first" not in tracker.ignored_counts
        )
        mcp_first_pending = "mcp_first" in tracker.pending_nudges

        # Already resolved — agent used MCP tools, stop nagging
        if mcp_first_resolved:
            pass  # fall through to periodic check
        # Turn 3+: first fire
        elif not mcp_first_ever_fired:
            self._mcp_first_fired = True
            tracker.pending_nudges.add("mcp_first")
            return (
                "WARNING: No MCP search tools called yet (search, report_error, "
                "check_compat). You MUST call one of these BEFORE making fixes. "
                "This is non-negotiable — fixes without MCP search violate "
                "project rules and will be blocked."
            )
        # Still pending — fire again (escalation system tracks ignores and blocks)
        elif mcp_first_pending:
            return (
                "MANDATORY: You have not called report_error, search, or "
                "check_compat this session. Call one NOW. The escalation "
                "system will block all other tools after further ignores."
            )

        # Every 15 turns: periodic reminder
        if tracker.turn_count > 0 and tracker.turn_count % 15 == 0:
            return (
                "PERIODIC CHECK: Have you stored findings with add_rule or "
                "add_knowledge? Have you checked the knowledge DB for existing "
                "solutions before implementing new ones?"
            )

        return None

    def on_session_start(self, db: KnowledgeDB) -> dict[str, str]:
        """Return active tasks, boundary count, and last session findings."""
        result = {}

        # Reset tool tracking for this new session
        self._mcp_tools_called = set()
        self._mcp_first_fired = False

        # Active tasks with age
        try:
            tasks = db.execute(
                "SELECT subject, status, project, created FROM tasks "
                "WHERE status = 'active' ORDER BY project, id"
            ).fetchall()
            if tasks:
                lines = []
                for t in tasks:
                    age = ""
                    if t["created"]:
                        try:
                            from datetime import datetime
                            created = datetime.fromisoformat(t["created"].replace("Z", "+00:00"))
                            days = (datetime.now(created.tzinfo) - created).days
                            if days > 0:
                                age = f" ({days}d ago)"
                        except Exception:
                            pass
                    lines.append(f"  - [{t['project']}] {t['subject']}{age}")
                result["Active tasks"] = "\n" + "\n".join(lines)
            else:
                result["Active tasks"] = "None"
        except Exception:
            pass

        # Last session findings summary
        try:
            last_session = db.execute(
                "SELECT status, current_task, findings_summary, created_at "
                "FROM sessions ORDER BY id DESC LIMIT 1"
            ).fetchone()
            if last_session and last_session["created_at"]:
                ts = last_session["created_at"]
                # Count knowledge/rules/errors stored since last session
                counts = {}
                for table in ("knowledge", "errors", "rules"):
                    try:
                        row = db.execute(
                            f"SELECT COUNT(*) as cnt FROM {table} WHERE created >= ?",
                            (ts,),
                        ).fetchone()
                        if row and row["cnt"] > 0:
                            counts[table] = row["cnt"]
                    except Exception:
                        pass
                if counts:
                    parts = [f"{v} {k}" for k, v in counts.items()]
                    result["Last session stored"] = ", ".join(parts)
        except Exception:
            pass

        # Boundary count
        try:
            row = db.execute("SELECT COUNT(*) as cnt FROM boundaries").fetchone()
            result["ABI boundary maps"] = str(row["cnt"])
        except Exception:
            pass

        return result
