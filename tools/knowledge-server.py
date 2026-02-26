#!/usr/bin/env python3
"""
Knowledge MCP Server — Structured knowledge lookup for mogrix.

Collapses multi-step lookup processes (grep INDEX.md, check compat catalog,
query knowledge DB) into single MCP tool calls. Purely local — no SSH.

Configuration via environment variables:
  KNOWLEDGE_LOG  - Log file path (default: /tmp/knowledge-mcp.log)
"""
import json
import os
import re
import signal
import sqlite3
import sys
import time
import traceback
from pathlib import Path
from typing import Any

# --- Paths ---

PROJECT_ROOT = Path(__file__).resolve().parent.parent
DB_PATH = PROJECT_ROOT / ".claude" / "knowledge.db"
CATALOG_PATH = PROJECT_ROOT / "compat" / "catalog.yaml"
INDEX_PATH = PROJECT_ROOT / "rules" / "INDEX.md"
KNOWLEDGE_LOG = os.environ.get("KNOWLEDGE_LOG", "/tmp/knowledge-mcp.log")

# --- Logging ---

_log_file = None


def log(msg: str) -> None:
    """Log to file only. NEVER write to stderr — Claude Code treats stderr as failure."""
    global _log_file
    try:
        if _log_file is None:
            _log_file = open(KNOWLEDGE_LOG, "a")
        ts = time.strftime("%Y-%m-%d %H:%M:%S")
        _log_file.write(f"[{ts}] {msg}\n")
        _log_file.flush()
    except Exception:
        pass


# --- JSON-RPC over stdio ---


def read_message() -> dict | None:
    line = sys.stdin.readline()
    if not line:
        return None
    return json.loads(line)


def write_message(msg: dict) -> None:
    try:
        json.dump(msg, sys.stdout, ensure_ascii=False)
        sys.stdout.write("\n")
        sys.stdout.flush()
    except (BrokenPipeError, IOError) as e:
        log(f"stdout broken: {e} — client gone, exiting")
        sys.exit(0)


# --- KnowledgeDB ---


class KnowledgeDB:
    """SQLite wrapper for all knowledge DB operations."""

    def __init__(self, db_path: Path):
        self.db_path = db_path
        self.conn = sqlite3.connect(str(db_path))
        self.conn.row_factory = sqlite3.Row
        self.conn.execute("PRAGMA journal_mode=WAL")
        self.migrate()

    def migrate(self):
        """Add new tables. Existing tables untouched (CREATE IF NOT EXISTS)."""
        c = self.conn.cursor()

        c.execute("""
            CREATE TABLE IF NOT EXISTS rules (
                id INTEGER PRIMARY KEY,
                problem_class TEXT NOT NULL,
                keywords TEXT NOT NULL,
                mechanism TEXT,
                location TEXT,
                notes TEXT,
                hit_count INTEGER DEFAULT 0,
                created_at TEXT DEFAULT (datetime('now')),
                updated_at TEXT DEFAULT (datetime('now'))
            )
        """)

        c.execute("""
            CREATE TABLE IF NOT EXISTS keyword_aliases (
                id INTEGER PRIMARY KEY,
                alias TEXT NOT NULL,
                canonical_keyword TEXT NOT NULL,
                rule_id INTEGER REFERENCES rules(id),
                created_at TEXT DEFAULT (datetime('now'))
            )
        """)

        c.execute("""
            CREATE TABLE IF NOT EXISTS negative_knowledge (
                id INTEGER PRIMARY KEY,
                category TEXT NOT NULL,
                description TEXT NOT NULL,
                package TEXT,
                severity TEXT DEFAULT 'normal',
                created_at TEXT DEFAULT (datetime('now'))
            )
        """)

        c.execute("""
            CREATE TABLE IF NOT EXISTS session_meta (
                session_id TEXT PRIMARY KEY,
                start_time TEXT DEFAULT (datetime('now')),
                interaction_count INTEGER DEFAULT 0,
                last_rules_check TEXT,
                current_package TEXT
            )
        """)

        # FTS5 virtual table for full-text search on rules
        try:
            c.execute("""
                CREATE VIRTUAL TABLE IF NOT EXISTS rules_fts USING fts5(
                    problem_class, keywords, mechanism, notes,
                    content='rules', content_rowid='id'
                )
            """)
        except sqlite3.OperationalError as e:
            log(f"FTS5 creation failed (may already exist): {e}")

        # Triggers to keep FTS5 in sync
        c.execute("""
            CREATE TRIGGER IF NOT EXISTS rules_ai AFTER INSERT ON rules BEGIN
                INSERT INTO rules_fts(rowid, problem_class, keywords, mechanism, notes)
                VALUES (new.id, new.problem_class, new.keywords, new.mechanism, new.notes);
            END
        """)

        c.execute("""
            CREATE TRIGGER IF NOT EXISTS rules_au AFTER UPDATE ON rules BEGIN
                INSERT INTO rules_fts(rules_fts, rowid, problem_class, keywords, mechanism, notes)
                VALUES ('delete', old.id, old.problem_class, old.keywords, old.mechanism, old.notes);
                INSERT INTO rules_fts(rowid, problem_class, keywords, mechanism, notes)
                VALUES (new.id, new.problem_class, new.keywords, new.mechanism, new.notes);
            END
        """)

        c.execute("""
            CREATE TRIGGER IF NOT EXISTS rules_ad AFTER DELETE ON rules BEGIN
                INSERT INTO rules_fts(rules_fts, rowid, problem_class, keywords, mechanism, notes)
                VALUES ('delete', old.id, old.problem_class, old.keywords, old.mechanism, old.notes);
            END
        """)

        self.conn.commit()
        log("DB migration complete")

    def search_rules(self, query: str, limit: int = 10) -> list[dict]:
        """Search rules via FTS5, with LIKE fallback."""
        results = []

        # Try FTS5 first
        try:
            rows = self.conn.execute(
                """
                SELECT r.id, r.problem_class, r.keywords, r.mechanism,
                       r.location, r.notes, r.hit_count
                FROM rules_fts fts
                JOIN rules r ON fts.rowid = r.id
                WHERE rules_fts MATCH ?
                ORDER BY rank
                LIMIT ?
                """,
                (query, limit),
            ).fetchall()
            results = [dict(r) for r in rows]
        except sqlite3.OperationalError as e:
            log(f"FTS5 search failed, falling back to LIKE: {e}")

        # LIKE fallback if FTS5 returned nothing or failed
        if not results:
            pattern = f"%{query}%"
            rows = self.conn.execute(
                """
                SELECT id, problem_class, keywords, mechanism, location, notes, hit_count
                FROM rules
                WHERE problem_class LIKE ? OR keywords LIKE ?
                   OR mechanism LIKE ? OR notes LIKE ?
                ORDER BY hit_count DESC
                LIMIT ?
                """,
                (pattern, pattern, pattern, pattern, limit),
            ).fetchall()
            results = [dict(r) for r in rows]

        # Also check keyword_aliases
        if not results:
            alias_rows = self.conn.execute(
                """
                SELECT r.id, r.problem_class, r.keywords, r.mechanism,
                       r.location, r.notes, r.hit_count
                FROM keyword_aliases ka
                JOIN rules r ON ka.rule_id = r.id
                WHERE ka.alias LIKE ?
                LIMIT ?
                """,
                (f"%{query}%", limit),
            ).fetchall()
            results = [dict(r) for r in alias_rows]

        return results

    def search_errors(self, text: str) -> list[dict]:
        """Search errors_seen by pattern LIKE."""
        pattern = f"%{text}%"
        rows = self.conn.execute(
            """
            SELECT id, pattern, root_cause, fix, file_path
            FROM errors_seen
            WHERE pattern LIKE ?
            ORDER BY id DESC
            LIMIT 10
            """,
            (pattern,),
        ).fetchall()
        return [dict(r) for r in rows]

    def search_findings(self, text: str) -> list[dict]:
        """Search findings by topic and content LIKE."""
        pattern = f"%{text}%"
        rows = self.conn.execute(
            """
            SELECT id, topic, finding, source, confidence
            FROM findings
            WHERE topic LIKE ? OR finding LIKE ?
            ORDER BY id DESC
            LIMIT 10
            """,
            (pattern, pattern),
        ).fetchall()
        return [dict(r) for r in rows]

    def add_rule(
        self,
        problem_class: str,
        keywords: str,
        mechanism: str | None = None,
        location: str | None = None,
        notes: str | None = None,
    ) -> int:
        """Insert a new rule. Returns the new rule ID."""
        c = self.conn.execute(
            """
            INSERT INTO rules (problem_class, keywords, mechanism, location, notes)
            VALUES (?, ?, ?, ?, ?)
            """,
            (problem_class, keywords, mechanism, location, notes),
        )
        self.conn.commit()
        return c.lastrowid

    def add_finding(self, topic: str, finding: str, source: str | None = None,
                    project: str = "mogrix") -> int:
        """Insert into existing findings table."""
        c = self.conn.execute(
            """
            INSERT INTO findings (topic, finding, source, project)
            VALUES (?, ?, ?, ?)
            """,
            (topic, finding, source, project),
        )
        self.conn.commit()
        return c.lastrowid

    def add_decision(self, topic: str, decision: str, rationale: str | None = None,
                     alternatives: str | None = None, project: str = "mogrix") -> int:
        """Insert into existing decisions table."""
        c = self.conn.execute(
            """
            INSERT INTO decisions (topic, decision, rationale, alternatives_rejected, project)
            VALUES (?, ?, ?, ?, ?)
            """,
            (topic, decision, rationale, alternatives, project),
        )
        self.conn.commit()
        return c.lastrowid

    def add_negative(self, category: str, description: str,
                     package: str | None = None, severity: str = "normal") -> int:
        """Insert into negative_knowledge table."""
        c = self.conn.execute(
            """
            INSERT INTO negative_knowledge (category, description, package, severity)
            VALUES (?, ?, ?, ?)
            """,
            (category, description, package, severity),
        )
        self.conn.commit()
        return c.lastrowid

    def increment_hit_count(self, rule_id: int):
        """Bump hit_count and updated_at for a rule."""
        self.conn.execute(
            """
            UPDATE rules SET hit_count = hit_count + 1,
                             updated_at = datetime('now')
            WHERE id = ?
            """,
            (rule_id,),
        )
        self.conn.commit()

    def get_active_tasks(self) -> list[dict]:
        """Return all active tasks."""
        rows = self.conn.execute(
            """
            SELECT id, subject, status, description, project
            FROM tasks
            WHERE status = 'active'
            ORDER BY project, id
            """,
        ).fetchall()
        return [dict(r) for r in rows]

    def create_session(self, session_id: str) -> dict:
        """Create a session_meta record and return context summary."""
        self.conn.execute(
            """
            INSERT OR REPLACE INTO session_meta (session_id, start_time)
            VALUES (?, datetime('now'))
            """,
            (session_id,),
        )
        self.conn.commit()

        # Gather stats
        rule_count = self.conn.execute("SELECT COUNT(*) FROM rules").fetchone()[0]
        task_list = self.get_active_tasks()
        recent_findings = self.conn.execute(
            "SELECT COUNT(*) FROM findings WHERE created > datetime('now', '-7 days')"
        ).fetchone()[0]
        recent_errors = self.conn.execute(
            "SELECT COUNT(*) FROM errors_seen WHERE created > datetime('now', '-7 days')"
        ).fetchone()[0]

        return {
            "session_id": session_id,
            "rules_count": rule_count,
            "active_tasks": [dict(t) for t in task_list],
            "recent_findings_7d": recent_findings,
            "recent_errors_7d": recent_errors,
        }

    def get_session_stats(self, session_id: str) -> dict:
        """Get current session stats."""
        meta = self.conn.execute(
            "SELECT * FROM session_meta WHERE session_id = ?",
            (session_id,),
        ).fetchone()

        task_list = self.get_active_tasks()

        # Count findings/decisions since session start
        if meta:
            start = meta["start_time"]
            findings_count = self.conn.execute(
                "SELECT COUNT(*) FROM findings WHERE created >= ?",
                (start,),
            ).fetchone()[0]
            decisions_count = self.conn.execute(
                "SELECT COUNT(*) FROM decisions WHERE created >= ?",
                (start,),
            ).fetchone()[0]
        else:
            findings_count = 0
            decisions_count = 0

        return {
            "session_id": session_id,
            "start_time": meta["start_time"] if meta else None,
            "interaction_count": meta["interaction_count"] if meta else 0,
            "last_rules_check": meta["last_rules_check"] if meta else None,
            "findings_this_session": findings_count,
            "decisions_this_session": decisions_count,
            "active_tasks": [dict(t) for t in task_list],
        }

    def update_session_interaction(self, session_id: str):
        """Bump interaction count and last rules check time."""
        self.conn.execute(
            """
            UPDATE session_meta
            SET interaction_count = interaction_count + 1,
                last_rules_check = datetime('now')
            WHERE session_id = ?
            """,
            (session_id,),
        )
        self.conn.commit()

    def close(self):
        self.conn.close()


# --- CompatCatalog ---


class CompatCatalog:
    """Parses compat/catalog.yaml on demand. No DB mirror."""

    def __init__(self, catalog_path: Path):
        self.catalog_path = catalog_path

    def search(self, symbol: str) -> dict | None:
        """Search catalog for a symbol. Returns match info or None."""
        try:
            import yaml
        except ImportError:
            log("PyYAML not available — catalog search disabled")
            return {"error": "PyYAML not installed. Run: pip install pyyaml"}

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


# --- INDEX.md Migration ---


def migrate_index(db: KnowledgeDB, index_path: Path) -> int:
    """Parse INDEX.md markdown tables and import into rules table.

    Idempotent: only runs if rules table is empty.
    Returns number of rules imported.
    """
    existing = db.conn.execute("SELECT COUNT(*) FROM rules").fetchone()[0]
    if existing > 0:
        log(f"Rules table already has {existing} rows, skipping migration")
        return 0

    if not index_path.exists():
        log(f"INDEX.md not found at {index_path}")
        return 0

    text = index_path.read_text()
    count = 0

    # Section: Per-Package Problem Reference
    # | Problem class | Symptoms / triggers | Rule mechanism | Rule location | Notes |
    count += _parse_5col_table(db, text, "Per-Package Problem Reference")

    # Section: Platform Invariants
    # | Fact | Implication | Reference |
    count += _parse_3col_table(db, text, "Platform Invariants",
                               col_map={"fact": "problem_class", "implication": "keywords", "reference": "location"})

    # Section: Engine Bugs & Gotchas
    # | Bug | Workaround | Reference |
    count += _parse_3col_table(db, text, "Engine Bugs",
                               col_map={"bug": "problem_class", "workaround": "mechanism", "reference": "location"})

    # Section: Bundle & Wrapper
    # | Fact | Implication | Reference |
    count += _parse_3col_table(db, text, "Bundle & Wrapper",
                               col_map={"fact": "problem_class", "implication": "keywords", "reference": "location"})

    # Section: Debugging: Crash Handler
    # | Keyword | Description | Fix | Source |
    count += _parse_4col_table(db, text, "Debugging: Crash Handler",
                               col_map={"keyword": "keywords", "description": "problem_class",
                                        "fix": "mechanism", "source": "location"})

    # Section: Go IRIX Port — Signal & Semaphore Issues
    # | Problem | Symptoms | Root Cause | Fix | Status |
    count += _parse_go_table(db, text, "Go IRIX Port")

    # Section: Anti-Patterns
    # | Anti-Pattern | Do This Instead |
    count += _parse_2col_table(db, text, "Anti-Patterns",
                               col_map={"anti-pattern": "problem_class", "do this instead": "mechanism"})

    db.conn.commit()
    log(f"Migrated {count} rules from INDEX.md")
    return count


def _extract_section_table(text: str, section_heading: str) -> list[str]:
    """Extract table lines from a section of markdown."""
    lines = text.split("\n")
    in_section = False
    table_lines = []

    for line in lines:
        # Match section headers (## or ###)
        if re.match(r"^#{2,3}\s+", line):
            if section_heading.lower() in line.lower():
                in_section = True
                continue
            elif in_section:
                # Hit next section
                break

        if in_section and line.strip().startswith("|"):
            table_lines.append(line.strip())

    return table_lines


def _parse_table_rows(table_lines: list[str]) -> list[list[str]]:
    """Parse markdown table lines into rows of cells. Skip header + separator."""
    if len(table_lines) < 3:
        return []

    rows = []
    for line in table_lines[2:]:  # Skip header and separator
        cells = [c.strip() for c in line.split("|")]
        # Remove empty first/last from leading/trailing |
        if cells and cells[0] == "":
            cells = cells[1:]
        if cells and cells[-1] == "":
            cells = cells[:-1]
        if cells:
            rows.append(cells)
    return rows


def _clean_md(text: str) -> str:
    """Strip markdown formatting from cell text."""
    # Remove bold
    text = re.sub(r"\*\*(.+?)\*\*", r"\1", text)
    # Remove inline code
    text = re.sub(r"`(.+?)`", r"\1", text)
    # Remove links [text](url) -> text
    text = re.sub(r"\[(.+?)\]\(.+?\)", r"\1", text)
    return text.strip()


def _parse_5col_table(db: KnowledgeDB, text: str, section: str) -> int:
    """Parse 5-column table: Problem class | Symptoms | Mechanism | Location | Notes."""
    table_lines = _extract_section_table(text, section)
    rows = _parse_table_rows(table_lines)
    count = 0
    for row in rows:
        if len(row) >= 4:
            problem_class = _clean_md(row[0])
            keywords = _clean_md(row[1])
            mechanism = _clean_md(row[2]) if len(row) > 2 else None
            location = _clean_md(row[3]) if len(row) > 3 else None
            notes = _clean_md(row[4]) if len(row) > 4 else None
            if problem_class:
                db.conn.execute(
                    "INSERT INTO rules (problem_class, keywords, mechanism, location, notes) VALUES (?,?,?,?,?)",
                    (problem_class, keywords, mechanism, location, notes),
                )
                count += 1
    return count


def _parse_3col_table(db: KnowledgeDB, text: str, section: str,
                      col_map: dict) -> int:
    """Parse 3-column table with configurable column mapping."""
    table_lines = _extract_section_table(text, section)
    rows = _parse_table_rows(table_lines)
    count = 0
    for row in rows:
        if len(row) >= 2:
            vals = {
                "problem_class": "",
                "keywords": "",
                "mechanism": None,
                "location": None,
                "notes": None,
            }
            # Map columns by position using col_map keys in order
            col_keys = list(col_map.values())
            for i, cell in enumerate(row):
                if i < len(col_keys):
                    vals[col_keys[i]] = _clean_md(cell)

            if vals["problem_class"]:
                db.conn.execute(
                    "INSERT INTO rules (problem_class, keywords, mechanism, location, notes) VALUES (?,?,?,?,?)",
                    (vals["problem_class"], vals["keywords"], vals["mechanism"],
                     vals["location"], vals["notes"]),
                )
                count += 1
    return count


def _parse_4col_table(db: KnowledgeDB, text: str, section: str,
                      col_map: dict) -> int:
    """Parse 4-column table with configurable column mapping."""
    table_lines = _extract_section_table(text, section)
    rows = _parse_table_rows(table_lines)
    count = 0
    for row in rows:
        if len(row) >= 3:
            vals = {
                "problem_class": "",
                "keywords": "",
                "mechanism": None,
                "location": None,
                "notes": None,
            }
            col_keys = list(col_map.values())
            for i, cell in enumerate(row):
                if i < len(col_keys):
                    vals[col_keys[i]] = _clean_md(cell)

            if vals["problem_class"]:
                db.conn.execute(
                    "INSERT INTO rules (problem_class, keywords, mechanism, location, notes) VALUES (?,?,?,?,?)",
                    (vals["problem_class"], vals["keywords"], vals["mechanism"],
                     vals["location"], vals["notes"]),
                )
                count += 1
    return count


def _parse_2col_table(db: KnowledgeDB, text: str, section: str,
                      col_map: dict) -> int:
    """Parse 2-column table."""
    table_lines = _extract_section_table(text, section)
    rows = _parse_table_rows(table_lines)
    count = 0
    for row in rows:
        if len(row) >= 2:
            vals = {
                "problem_class": "",
                "keywords": "",
                "mechanism": None,
                "location": None,
                "notes": None,
            }
            col_keys = list(col_map.values())
            for i, cell in enumerate(row):
                if i < len(col_keys):
                    vals[col_keys[i]] = _clean_md(cell)

            if vals["problem_class"]:
                db.conn.execute(
                    "INSERT INTO rules (problem_class, keywords, mechanism, location, notes) VALUES (?,?,?,?,?)",
                    (vals["problem_class"], vals["keywords"], vals["mechanism"],
                     vals["location"], vals["notes"]),
                )
                count += 1
    return count


def _parse_go_table(db: KnowledgeDB, text: str, section: str) -> int:
    """Parse Go IRIX Port table: Problem | Symptoms | Root Cause | Fix | Status."""
    table_lines = _extract_section_table(text, section)
    rows = _parse_table_rows(table_lines)
    count = 0
    for row in rows:
        if len(row) >= 4:
            problem = _clean_md(row[0])
            symptoms = _clean_md(row[1])
            root_cause = _clean_md(row[2]) if len(row) > 2 else ""
            fix = _clean_md(row[3]) if len(row) > 3 else ""
            status = _clean_md(row[4]) if len(row) > 4 else ""

            keywords = symptoms
            if root_cause:
                keywords += f", {root_cause}"

            notes = status if status else None

            if problem:
                db.conn.execute(
                    "INSERT INTO rules (problem_class, keywords, mechanism, location, notes) VALUES (?,?,?,?,?)",
                    (problem, keywords, fix, "golang-irix", notes),
                )
                count += 1
    return count


# --- KnowledgeServer ---


class KnowledgeServer:
    """MCP server for knowledge DB operations."""

    def __init__(self):
        self.name = "knowledge-mcp"
        self.version = "1.0.0"
        self.protocol_version = "2025-11-25"
        self.fallback_version = "2024-11-05"
        self.initialized = False

        self.db = KnowledgeDB(DB_PATH)
        self.catalog = CompatCatalog(CATALOG_PATH)
        self.session_id = None

        self.tools = [
            {
                "name": "knowledge_query",
                "description": (
                    "Search mogrix rules database by keywords. Searches FTS5 index, "
                    "keyword aliases, errors_seen, and findings. Use this instead of "
                    "grepping INDEX.md. Returns matching rules ranked by relevance. "
                    "Example: knowledge_query {query: \"dlmalloc\"}"
                ),
                "inputSchema": {
                    "type": "object",
                    "properties": {
                        "query": {
                            "type": "string",
                            "description": "Search keywords (e.g., 'dlmalloc', 'rld crash', 'setenv')",
                        },
                        "limit": {
                            "type": "integer",
                            "description": "Max results (default: 10)",
                        },
                    },
                    "required": ["query"],
                },
            },
            {
                "name": "report_error",
                "description": (
                    "Report a build/link/runtime error AND automatically search for "
                    "matching rules and known fixes. THE KILLER FEATURE: one tool call "
                    "replaces noticing an error + remembering to grep INDEX.md + grepping + "
                    "reading results. Always returns both the logged confirmation and any "
                    "matching rules/fixes. Example: report_error {error_text: \"undefined reference to setenv\"}"
                ),
                "inputSchema": {
                    "type": "object",
                    "properties": {
                        "error_text": {
                            "type": "string",
                            "description": "The error message or text",
                        },
                        "package": {
                            "type": "string",
                            "description": "Package name if applicable",
                        },
                        "context": {
                            "type": "string",
                            "description": "Additional context (build phase, file, etc.)",
                        },
                    },
                    "required": ["error_text"],
                },
            },
            {
                "name": "report_finding",
                "description": (
                    "Store a discovery, decision, or negative knowledge in the DB. "
                    "Routes to existing findings/decisions tables or new negative_knowledge table. "
                    "Example: report_finding {type: \"finding\", detail: \"IRIX rld ignores DT_RUNPATH\", topic: \"rld\"}"
                ),
                "inputSchema": {
                    "type": "object",
                    "properties": {
                        "type": {
                            "type": "string",
                            "enum": ["finding", "decision", "negative"],
                            "description": "Type of knowledge to store",
                        },
                        "detail": {
                            "type": "string",
                            "description": "The finding/decision/negative knowledge text",
                        },
                        "topic": {
                            "type": "string",
                            "description": "Topic or area (e.g., 'rld', 'signal_handling')",
                        },
                        "package": {
                            "type": "string",
                            "description": "Package name if applicable",
                        },
                        "rationale": {
                            "type": "string",
                            "description": "For decisions: why this choice was made",
                        },
                        "severity": {
                            "type": "string",
                            "description": "For negative knowledge: 'normal', 'critical'",
                        },
                    },
                    "required": ["type", "detail"],
                },
            },
            {
                "name": "check_compat",
                "description": (
                    "Search the compat function catalog (compat/catalog.yaml) for a symbol. "
                    "Use this instead of grepping catalog.yaml. Returns implementation file, "
                    "header, description. Example: check_compat {symbol: \"getline\"}"
                ),
                "inputSchema": {
                    "type": "object",
                    "properties": {
                        "symbol": {
                            "type": "string",
                            "description": "Function/symbol name to search for",
                        },
                    },
                    "required": ["symbol"],
                },
            },
            {
                "name": "session_start",
                "description": (
                    "Initialize a knowledge session. Returns active tasks, "
                    "recent findings count, total rules count. Call at session start."
                ),
                "inputSchema": {
                    "type": "object",
                    "properties": {},
                },
            },
            {
                "name": "add_rule",
                "description": (
                    "Add a new rule to the knowledge DB. Use after fixing a problem "
                    "to ensure it's findable next time. "
                    "Example: add_rule {problem_class: \"IRIX lacks foo\", keywords: \"foo, undefined\"}"
                ),
                "inputSchema": {
                    "type": "object",
                    "properties": {
                        "problem_class": {
                            "type": "string",
                            "description": "Short problem description",
                        },
                        "keywords": {
                            "type": "string",
                            "description": "Comma-separated search keywords",
                        },
                        "mechanism": {
                            "type": "string",
                            "description": "Fix mechanism (e.g., 'inject_compat_functions')",
                        },
                        "location": {
                            "type": "string",
                            "description": "Rule file location (e.g., 'rules/packages/foo.yaml')",
                        },
                        "notes": {
                            "type": "string",
                            "description": "Additional notes",
                        },
                    },
                    "required": ["problem_class", "keywords"],
                },
            },
            {
                "name": "session_summary",
                "description": (
                    "Get current session state snapshot: findings/decisions this session, "
                    "active tasks, last rules check time."
                ),
                "inputSchema": {
                    "type": "object",
                    "properties": {},
                },
            },
        ]

    # --- Tool handlers ---

    def _handle_knowledge_query(self, args: dict) -> str:
        query = args.get("query", "")
        limit = args.get("limit", 10)

        if not query:
            return "Error: query is required"

        results = []

        # Search rules
        rules = self.db.search_rules(query, limit)
        if rules:
            for r in rules:
                self.db.increment_hit_count(r["id"])
            results.append(f"## Rules ({len(rules)} matches)\n")
            for r in rules:
                results.append(
                    f"- **{r['problem_class']}** (hits: {r['hit_count']})\n"
                    f"  Keywords: {r['keywords']}\n"
                    f"  Mechanism: {r.get('mechanism', 'N/A')}\n"
                    f"  Location: {r.get('location', 'N/A')}\n"
                    f"  Notes: {r.get('notes', '')}\n"
                )

        # Search errors_seen
        errors = self.db.search_errors(query)
        if errors:
            results.append(f"\n## Known Errors ({len(errors)} matches)\n")
            for e in errors:
                results.append(
                    f"- **Pattern**: {e['pattern']}\n"
                    f"  Root cause: {e['root_cause']}\n"
                    f"  Fix: {e['fix']}\n"
                    f"  File: {e.get('file_path', 'N/A')}\n"
                )

        # Search findings
        findings = self.db.search_findings(query)
        if findings:
            results.append(f"\n## Related Findings ({len(findings)} matches)\n")
            for f in findings:
                results.append(
                    f"- **{f['topic']}** [{f['confidence']}]: {f['finding']}\n"
                )

        if self.session_id:
            self.db.update_session_interaction(self.session_id)

        if not results:
            return (
                f"No rules match '{query}'.\n\n"
                "If you just fixed this problem, use `add_rule` to store the fix "
                "so it's findable next time."
            )

        return "\n".join(results)

    def _handle_report_error(self, args: dict) -> str:
        error_text = args.get("error_text", "")
        package = args.get("package", "")
        context = args.get("context", "")

        if not error_text:
            return "Error: error_text is required"

        # Log as a finding
        topic = f"error:{package}" if package else "error"
        detail = error_text
        if context:
            detail += f" (context: {context})"
        finding_id = self.db.add_finding(topic, detail, source="report_error")

        output = [f"Logged as finding #{finding_id}.\n"]

        # Extract keywords for searching
        # Take significant words from error text
        keywords = _extract_keywords(error_text)

        # Search rules for each keyword
        all_rules = {}
        all_errors = {}
        for kw in keywords:
            for r in self.db.search_rules(kw, limit=5):
                all_rules[r["id"]] = r
            for e in self.db.search_errors(kw):
                all_errors[e["id"]] = e

        if all_rules:
            output.append(f"## Matching Rules ({len(all_rules)} found)\n")
            for r in all_rules.values():
                self.db.increment_hit_count(r["id"])
                output.append(
                    f"- **{r['problem_class']}**\n"
                    f"  Keywords: {r['keywords']}\n"
                    f"  Mechanism: {r.get('mechanism', 'N/A')}\n"
                    f"  Location: {r.get('location', 'N/A')}\n"
                    f"  Notes: {r.get('notes', '')}\n"
                )

        if all_errors:
            output.append(f"\n## Known Error Patterns ({len(all_errors)} found)\n")
            for e in all_errors.values():
                output.append(
                    f"- **{e['pattern']}**\n"
                    f"  Root cause: {e['root_cause']}\n"
                    f"  Fix: {e['fix']}\n"
                )

        # Also check compat catalog if error looks like a missing symbol
        symbol = _extract_symbol(error_text)
        if symbol:
            compat = self.catalog.search(symbol)
            if compat and compat.get("found"):
                output.append(
                    f"\n## Compat Catalog Match\n"
                    f"- **{compat['name']}**: {compat.get('description', '')}\n"
                    f"  File: compat/{compat['file']}\n"
                    f"  Header: {compat.get('header', 'N/A')}\n"
                )

        if not all_rules and not all_errors:
            output.append(
                "\nNo matching rules or known errors found. "
                "After fixing, use `add_rule` to store the solution."
            )

        if self.session_id:
            self.db.update_session_interaction(self.session_id)

        return "\n".join(output)

    def _handle_report_finding(self, args: dict) -> str:
        finding_type = args.get("type", "finding")
        detail = args.get("detail", "")
        topic = args.get("topic", "general")
        package = args.get("package")
        rationale = args.get("rationale")
        severity = args.get("severity", "normal")

        if not detail:
            return "Error: detail is required"

        if finding_type == "finding":
            fid = self.db.add_finding(topic, detail, source="report_finding")
            return f"Finding #{fid} stored: [{topic}] {detail[:100]}..."

        elif finding_type == "decision":
            did = self.db.add_decision(topic, detail, rationale=rationale)
            return f"Decision #{did} stored: [{topic}] {detail[:100]}..."

        elif finding_type == "negative":
            category = topic if topic != "general" else "failed_attempt"
            nid = self.db.add_negative(category, detail, package=package, severity=severity)
            return f"Negative knowledge #{nid} stored: [{category}] {detail[:100]}..."

        return f"Error: unknown type '{finding_type}'"

    def _handle_check_compat(self, args: dict) -> str:
        symbol = args.get("symbol", "")
        if not symbol:
            return "Error: symbol is required"

        result = self.catalog.search(symbol)
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

    def _handle_session_start(self, args: dict) -> str:
        self.session_id = time.strftime("%Y%m%d_%H%M%S")
        summary = self.db.create_session(self.session_id)

        lines = [f"## Session {self.session_id} started\n"]
        lines.append(f"- **Rules in DB**: {summary['rules_count']}")
        lines.append(f"- **Recent findings (7d)**: {summary['recent_findings_7d']}")
        lines.append(f"- **Recent errors (7d)**: {summary['recent_errors_7d']}")

        tasks = summary.get("active_tasks", [])
        if tasks:
            lines.append(f"\n### Active Tasks ({len(tasks)})\n")
            for t in tasks:
                lines.append(f"- [{t.get('project', 'mogrix')}] {t['subject']}")
                if t.get("description"):
                    desc = t["description"][:120]
                    lines.append(f"  {desc}")
        else:
            lines.append("\nNo active tasks.")

        return "\n".join(lines)

    def _handle_add_rule(self, args: dict) -> str:
        problem_class = args.get("problem_class", "")
        keywords = args.get("keywords", "")

        if not problem_class or not keywords:
            return "Error: problem_class and keywords are required"

        rule_id = self.db.add_rule(
            problem_class=problem_class,
            keywords=keywords,
            mechanism=args.get("mechanism"),
            location=args.get("location"),
            notes=args.get("notes"),
        )

        return (
            f"Rule #{rule_id} added:\n"
            f"- Problem: {problem_class}\n"
            f"- Keywords: {keywords}\n"
            f"- Mechanism: {args.get('mechanism', 'N/A')}\n"
            f"- Location: {args.get('location', 'N/A')}"
        )

    def _handle_session_summary(self, args: dict) -> str:
        if not self.session_id:
            return "No active session. Call session_start first."

        stats = self.db.get_session_stats(self.session_id)

        lines = [f"## Session {self.session_id}\n"]
        lines.append(f"- **Started**: {stats.get('start_time', 'N/A')}")
        lines.append(f"- **Interactions**: {stats['interaction_count']}")
        lines.append(f"- **Last rules check**: {stats.get('last_rules_check', 'N/A')}")
        lines.append(f"- **Findings this session**: {stats['findings_this_session']}")
        lines.append(f"- **Decisions this session**: {stats['decisions_this_session']}")

        tasks = stats.get("active_tasks", [])
        if tasks:
            lines.append(f"\n### Active Tasks ({len(tasks)})")
            for t in tasks:
                lines.append(f"- [{t.get('project', 'mogrix')}] {t['subject']}")

        return "\n".join(lines)

    # --- MCP protocol handlers ---

    def handle_initialize(self, request_id: Any, params: dict) -> dict:
        client_version = params.get("protocolVersion", self.fallback_version)
        client_info = params.get("clientInfo", {})
        log(
            f"Initialize from {client_info.get('name', '?')} "
            f"v{client_info.get('version', '?')} "
            f"(protocol {client_version})"
        )

        if client_version in [self.protocol_version, self.fallback_version]:
            version = client_version
        else:
            version = self.protocol_version

        return {
            "jsonrpc": "2.0",
            "id": request_id,
            "result": {
                "protocolVersion": version,
                "capabilities": {
                    "tools": {"listChanged": False},
                },
                "serverInfo": {
                    "name": self.name,
                    "version": self.version,
                },
                "instructions": (
                    "Mogrix knowledge DB. Use knowledge_query for rule/error lookup "
                    "(replaces grepping INDEX.md). Use report_error to log AND auto-search "
                    "for fixes. Use check_compat to search compat catalog. Use report_finding "
                    "to store discoveries. Use add_rule after fixing new problems."
                ),
            },
        }

    def handle_tools_list(self, request_id: Any) -> dict:
        return {
            "jsonrpc": "2.0",
            "id": request_id,
            "result": {"tools": self.tools},
        }

    def handle_tools_call(self, request_id: Any, params: dict) -> dict:
        tool_name = params.get("name")
        args = params.get("arguments", {})

        handlers = {
            "knowledge_query": self._handle_knowledge_query,
            "report_error": self._handle_report_error,
            "report_finding": self._handle_report_finding,
            "check_compat": self._handle_check_compat,
            "session_start": self._handle_session_start,
            "add_rule": self._handle_add_rule,
            "session_summary": self._handle_session_summary,
        }

        handler = handlers.get(tool_name)
        if not handler:
            return self._error(-32602, f"Unknown tool: {tool_name}", request_id)

        try:
            text = handler(args)
            is_error = text.startswith("Error:")
            return self._tool_result(request_id, text, is_error=is_error)
        except Exception as e:
            log(f"Tool {tool_name} error: {e}")
            log(traceback.format_exc())
            return self._tool_result(
                request_id, f"Error: {e}", is_error=True
            )

    # --- Response helpers ---

    def _tool_result(self, request_id: Any, text: str, is_error: bool = False) -> dict:
        return {
            "jsonrpc": "2.0",
            "id": request_id,
            "result": {
                "content": [{"type": "text", "text": text}],
                "isError": is_error,
            },
        }

    def _error(self, code: int, message: str, request_id: Any) -> dict:
        return {
            "jsonrpc": "2.0",
            "id": request_id,
            "error": {"code": code, "message": message},
        }

    # --- Main loop ---

    def handle_request(self, msg: dict) -> dict | None:
        method = msg.get("method")
        request_id = msg.get("id")
        params = msg.get("params", {})

        if method == "initialize":
            return self.handle_initialize(request_id, params)
        elif method == "notifications/initialized":
            self.initialized = True
            log("Initialized — ready")
            return None
        elif method == "notifications/cancelled":
            log(f"Client cancelled request {params.get('requestId')}")
            return None
        elif method == "tools/list":
            return self.handle_tools_list(request_id)
        elif method == "tools/call":
            return self.handle_tools_call(request_id, params)
        elif method == "ping":
            return {"jsonrpc": "2.0", "id": request_id, "result": {}}
        elif request_id is not None:
            return self._error(-32601, f"Method not found: {method}", request_id)
        return None

    def run(self) -> None:
        log(f"Knowledge MCP Server v{self.version}")
        log(f"DB: {DB_PATH}")

        try:
            while True:
                try:
                    msg = read_message()
                    if msg is None:
                        log("stdin closed, exiting")
                        break

                    request_id = msg.get("id")

                    try:
                        response = self.handle_request(msg)
                    except Exception as e:
                        log(f"Error handling request: {e}")
                        log(traceback.format_exc())
                        if request_id is not None:
                            response = self._error(
                                -32603, f"Internal error: {e}", request_id,
                            )
                        else:
                            response = None

                    if response is not None:
                        write_message(response)

                except json.JSONDecodeError as e:
                    log(f"JSON parse error: {e}")
                    write_message({
                        "jsonrpc": "2.0",
                        "id": None,
                        "error": {"code": -32700, "message": "Parse error"},
                    })
                except KeyboardInterrupt:
                    log("Interrupted")
                    break
        finally:
            log("Shutting down — closing DB")
            self.db.close()
            if _log_file:
                _log_file.close()

        log("Shutdown complete")


# --- Keyword extraction helpers ---


def _extract_keywords(error_text: str) -> list[str]:
    """Extract significant search keywords from error text."""
    # Common noise words to skip
    noise = {
        "error", "warning", "undefined", "reference", "to", "in", "the", "a",
        "an", "for", "of", "from", "with", "not", "no", "is", "was", "at",
        "by", "on", "or", "and", "that", "this", "it", "be", "as", "are",
        "but", "if", "line", "file", "symbol", "function", "type",
    }

    # Extract words, symbols, and identifiers
    words = re.findall(r"[a-zA-Z_][a-zA-Z0-9_]*", error_text)
    keywords = []
    seen = set()
    for w in words:
        wl = w.lower()
        if wl not in noise and wl not in seen and len(wl) > 2:
            keywords.append(wl)
            seen.add(wl)
            if len(keywords) >= 8:
                break

    return keywords


def _extract_symbol(error_text: str) -> str | None:
    """Try to extract a symbol name from an error message."""
    # "undefined reference to `foo'"
    m = re.search(r"undefined reference to [`'](\w+)'", error_text)
    if m:
        return m.group(1)

    # "use of undeclared identifier 'foo'"
    m = re.search(r"undeclared identifier [`'](\w+)'", error_text)
    if m:
        return m.group(1)

    # "undefined symbol: foo"
    m = re.search(r"undefined symbol:\s*(\w+)", error_text)
    if m:
        return m.group(1)

    # "'foo' undeclared"
    m = re.search(r"'(\w+)'\s+undeclared", error_text)
    if m:
        return m.group(1)

    return None


# --- CLI entry point ---


def _handle_sigterm(signum, frame):
    """Clean exit on SIGTERM."""
    raise SystemExit(0)


def main():
    if len(sys.argv) > 1 and sys.argv[1] == "--migrate-index":
        db = KnowledgeDB(DB_PATH)
        count = migrate_index(db, INDEX_PATH)
        print(f"Migrated {count} rules from INDEX.md")
        db.close()
        return

    signal.signal(signal.SIGTERM, _handle_sigterm)
    server = KnowledgeServer()
    server.run()


if __name__ == "__main__":
    main()
