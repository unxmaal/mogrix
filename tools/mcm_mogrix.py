"""Mogrix plugin for mcm-engine.

Domain-specific extensions for the mogrix cross-compilation knowledge system:
- boundaries table (ABI crossing maps for golang-irix)
- tasks table (cross-session task tracking)
- check_compat tool (searches compat/catalog.yaml)
- Backward-compatible tool aliases (knowledge_query, report_finding)
"""
from __future__ import annotations

import re
from pathlib import Path

import yaml

from mcm_engine.plugin import MCMPlugin, SearchScope
from mcm_engine.db import KnowledgeDB
from mcm_engine.tracker import SessionTracker


class CompatCatalog:
    """Searches compat/catalog.yaml for function implementations."""

    def __init__(self, catalog_path: Path):
        self._path = catalog_path
        self._data: dict | None = None

    def _load(self) -> dict:
        if self._data is None:
            try:
                with open(self._path) as f:
                    raw = yaml.safe_load(f)
                self._data = raw.get("functions", {}) if raw else {}
            except (OSError, yaml.YAMLError):
                self._data = {}
        return self._data

    def search(self, symbol: str) -> dict:
        functions = self._load()
        sym_lower = symbol.lower()

        # Direct name match
        for name, info in functions.items():
            if name.lower() == sym_lower:
                return self._format(name, info, "direct match")

        # Check provides lists
        for name, info in functions.items():
            provides = info.get("provides", [])
            if isinstance(provides, list):
                for p in provides:
                    if p.lower() == sym_lower:
                        return self._format(name, info, f"provides: {p}")

        # Check source_patterns
        for name, info in functions.items():
            patterns = info.get("source_patterns", [])
            if isinstance(patterns, list):
                for pat in patterns:
                    pat_str = pat if isinstance(pat, str) else pat.get("pattern", "")
                    if symbol in pat_str:
                        return self._format(name, info, f"source_pattern: {pat_str}")

        return {"found": False, "symbol": symbol}

    def _format(self, name: str, info: dict, matched_via: str) -> dict:
        return {
            "found": True,
            "name": name,
            "file": info.get("file", ""),
            "header": info.get("header", ""),
            "description": info.get("description", ""),
            "matched_via": matched_via,
            "notes": info.get("notes", ""),
        }


class MogrixPlugin(MCMPlugin):
    """Mogrix domain plugin for mcm-engine."""

    @property
    def name(self) -> str:
        return "mogrix"

    @property
    def version(self) -> int:
        return 1

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

        # Resolve catalog path relative to project root
        project_root = server.project_root
        catalog_path = project_root / "compat" / "catalog.yaml"
        catalog = CompatCatalog(catalog_path)

        # Get search_all_fn from server for report_error auto-search
        search_all_fn = getattr(server, "_search_all_fn", None)

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
            if result.get("found"):
                parts = [
                    f"FOUND: {result['name']}",
                    f"  File: compat/{result['file']}",
                    f"  Header: {result['header']}",
                    f"  Description: {result['description']}",
                    f"  Matched via: {result['matched_via']}",
                ]
                if result.get("notes"):
                    parts.append(f"  Notes: {result['notes']}")
                return "\n".join(parts)
            return f"NOT FOUND: {symbol}\nNo compat implementation exists. You may need to write one."

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
                db.execute_write(
                    "INSERT INTO negative_knowledge "
                    "(category, what_failed, why_failed, correct_approach, severity, project) "
                    "VALUES (?, ?, ?, ?, ?, ?)",
                    (topic, detail, "", "", severity, package or "mogrix"),
                )
                db.commit()
                return f"Stored negative knowledge: {topic} — {detail[:100]}"

            kind = "decision" if type == "decision" else "finding"
            db.execute_write(
                "INSERT INTO knowledge "
                "(topic, kind, summary, detail, tags, project, rationale, alternatives) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
                (topic, kind, detail[:200], detail, "", package or "mogrix", rationale, ""),
            )
            db.commit()
            return f"Stored {kind}: {topic} — {detail[:100]}"

    def get_nudge(self, tracker: SessionTracker) -> str | None:
        """Domain nudge: remind about MCP-first workflow."""
        if tracker.turn_count < 5:
            return None

        # Check if report_error or check_compat have been called this session
        tool_calls = set()
        for topic in tracker.topic_freq:
            # topic_freq tracks topics, not tool names — we rely on the
            # core nudge system for store reminders. This nudge fires once
            # as a general reminder.
            pass

        if tracker.turn_count == 5:
            return (
                "MOGRIX WORKFLOW CHECK: Have you used `report_error` for errors "
                "and `check_compat` for missing symbols? These are MANDATORY "
                "before manual fixes. Also: call `add_rule` IMMEDIATELY after "
                "confirming a fix works."
            )
        return None

    def on_session_start(self, db: KnowledgeDB) -> dict[str, str]:
        """Return active tasks and boundary count for session context."""
        result = {}

        # Active tasks
        try:
            tasks = db.execute(
                "SELECT subject, status, project FROM tasks WHERE status = 'active' ORDER BY project, id"
            ).fetchall()
            if tasks:
                lines = [f"  - [{t['project']}] {t['subject']}" for t in tasks]
                result["Active tasks"] = "\n" + "\n".join(lines)
            else:
                result["Active tasks"] = "None"
        except Exception:
            pass

        # Boundary count
        try:
            row = db.execute("SELECT COUNT(*) as cnt FROM boundaries").fetchone()
            result["ABI boundary maps"] = str(row["cnt"])
        except Exception:
            pass

        return result
