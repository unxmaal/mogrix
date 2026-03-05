#!/usr/bin/env python3
"""One-shot migration from legacy knowledge-server.py schema to mcm-engine schema.

Usage:
    python tools/migrate-to-mcm.py [--dry-run]

Backs up .claude/knowledge.db to .claude/knowledge-old.db, then creates
a fresh DB with mcm-engine core schema + mogrix plugin schema, and copies
all rows with column mapping.
"""
from __future__ import annotations

import argparse
import shutil
import sqlite3
import sys
from pathlib import Path

# Ensure project root is on path so mcm_engine can be imported
PROJECT_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(PROJECT_ROOT))

from mcm_engine.db import KnowledgeDB
from mcm_engine.schema import migrate_core, migrate_plugin
from tools.mcm_mogrix import MogrixPlugin


OLD_DB = PROJECT_ROOT / ".claude" / "knowledge.db"
BACKUP_DB = PROJECT_ROOT / ".claude" / "knowledge-old.db"


def migrate(dry_run: bool = False) -> None:
    if not OLD_DB.exists():
        print(f"No database found at {OLD_DB}")
        sys.exit(1)

    # 1. Back up
    if not dry_run:
        shutil.copy2(OLD_DB, BACKUP_DB)
        print(f"Backed up to {BACKUP_DB}")
    else:
        print(f"[DRY RUN] Would back up to {BACKUP_DB}")

    # 2. Read all data from old DB
    old = sqlite3.connect(str(OLD_DB))
    old.row_factory = sqlite3.Row

    old_rules = old.execute("SELECT * FROM rules").fetchall()
    old_findings = old.execute("SELECT * FROM findings").fetchall()
    old_decisions = old.execute("SELECT * FROM decisions").fetchall()
    old_errors = old.execute("SELECT * FROM errors_seen").fetchall()
    old_negative = old.execute("SELECT * FROM negative_knowledge").fetchall()
    old_sessions = old.execute("SELECT * FROM sessions").fetchall()
    old_boundaries = old.execute("SELECT * FROM boundaries").fetchall()
    old_tasks = old.execute("SELECT * FROM tasks").fetchall()
    old.close()

    counts = {
        "rules": len(old_rules),
        "findings": len(old_findings),
        "decisions": len(old_decisions),
        "errors": len(old_errors),
        "negative": len(old_negative),
        "sessions": len(old_sessions),
        "boundaries": len(old_boundaries),
        "tasks": len(old_tasks),
    }
    print(f"Old DB counts: {counts}")

    if dry_run:
        print("[DRY RUN] Would migrate these rows. Exiting.")
        return

    # 3. Delete old DB and create fresh with mcm-engine schema
    OLD_DB.unlink()
    # Also remove WAL/SHM if present
    for suffix in ("-wal", "-shm"):
        p = OLD_DB.parent / (OLD_DB.name + suffix)
        if p.exists():
            p.unlink()

    db = KnowledgeDB(OLD_DB)
    migrate_core(db)

    # 4. Apply mogrix plugin schema
    plugin = MogrixPlugin()
    migrate_plugin(db, plugin.name, plugin.get_schema_sql(), plugin.version)

    # 5. Migrate rules: problem_class->title, mechanism->category, notes->description
    # The old `location` field was a free-text note about where a fix was applied,
    # NOT a file path to a rule document. Store it in description, not file_path.
    # Actual file_path values come from sync_rules scanning rules/ directories.
    for r in old_rules:
        location = r["location"] or ""
        notes = r["notes"] or ""
        description = notes
        if location:
            description = f"{notes} | Location: {location}" if notes else f"Location: {location}"
        db.execute_write(
            "INSERT INTO rules (title, keywords, category, file_path, description, hit_count, created_at, updated_at) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
            (
                r["problem_class"],
                r["keywords"],
                r["mechanism"] or "",
                "",  # file_path left empty — populated by sync_rules
                description,
                r["hit_count"],
                r["created_at"],
                r["updated_at"],
            ),
        )

    # 6. Migrate findings -> knowledge (kind='finding')
    for f in old_findings:
        db.execute_write(
            "INSERT INTO knowledge (topic, kind, summary, detail, tags, project, created_at, updated_at) "
            "VALUES (?, 'finding', ?, ?, ?, ?, ?, ?)",
            (
                f["topic"],
                f["finding"][:200],  # summary (truncated)
                f["finding"],        # detail (full text)
                "",                  # tags
                f["project"],
                f["created"],
                f["created"],
            ),
        )

    # 7. Migrate decisions -> knowledge (kind='decision')
    for d in old_decisions:
        db.execute_write(
            "INSERT INTO knowledge (topic, kind, summary, detail, tags, project, rationale, alternatives, created_at, updated_at) "
            "VALUES (?, 'decision', ?, ?, ?, ?, ?, ?, ?, ?)",
            (
                d["topic"],
                d["decision"][:200],
                d["decision"],
                "",
                d["project"],
                d["rationale"] or "",
                d["alternatives_rejected"] or "",
                d["created"],
                d["created"],
            ),
        )

    # 8. Migrate errors_seen -> errors
    for e in old_errors:
        db.execute_write(
            "INSERT INTO errors (pattern, context, root_cause, fix, project, created_at) "
            "VALUES (?, ?, ?, ?, ?, ?)",
            (
                e["pattern"],
                e["file_path"] or "",  # context
                e["root_cause"],
                e["fix"],
                e["project"],
                e["created"],
            ),
        )

    # 9. Migrate negative_knowledge
    for n in old_negative:
        db.execute_write(
            "INSERT INTO negative_knowledge (category, what_failed, why_failed, correct_approach, severity, project, created_at) "
            "VALUES (?, ?, ?, ?, ?, ?, ?)",
            (
                n["category"],
                n["description"],  # what_failed
                "",                # why_failed (not in old schema)
                "",                # correct_approach (not in old schema)
                n["severity"],
                n["package"] or "mogrix",
                n["created_at"],
            ),
        )

    # 10. Migrate sessions
    for s in old_sessions:
        db.execute_write(
            "INSERT INTO sessions (status, current_task, findings_summary, next_steps, blockers, created_at) "
            "VALUES (?, ?, ?, ?, ?, ?)",
            (
                s["summary"],           # status
                s["tasks_started"] or "",  # current_task
                "",                        # findings_summary
                s["key_findings"] or "",   # next_steps (was overloaded)
                "",                        # blockers
                s["created"],
            ),
        )

    # 11. Migrate boundaries (plugin table — identical schema)
    for b in old_boundaries:
        db.execute_write(
            "INSERT INTO boundaries (system, transition, register_or_state, value_before, value_after, "
            "who_restores, verified, notes, project, created) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
            (
                b["system"],
                b["transition"],
                b["register_or_state"],
                b["value_before"],
                b["value_after"],
                b["who_restores"],
                b["verified"],
                b["notes"],
                b["project"],
                b["created"],
            ),
        )

    # 12. Migrate tasks (plugin table — identical schema)
    for t in old_tasks:
        db.execute_write(
            "INSERT INTO tasks (subject, status, description, blockers, project, created, updated) "
            "VALUES (?, ?, ?, ?, ?, ?, ?)",
            (
                t["subject"],
                t["status"],
                t["description"],
                t["blockers"],
                t["project"],
                t["created"],
                t["updated"],
            ),
        )

    db.commit()

    # 13. Verify counts
    new_counts = {}
    for table, expected in [
        ("rules", counts["rules"]),
        ("knowledge", counts["findings"] + counts["decisions"]),
        ("errors", counts["errors"]),
        ("negative_knowledge", counts["negative"]),
        ("sessions", counts["sessions"]),
        ("boundaries", counts["boundaries"]),
        ("tasks", counts["tasks"]),
    ]:
        actual = db.execute(f"SELECT COUNT(*) as cnt FROM {table}").fetchone()["cnt"]
        new_counts[table] = actual
        status = "OK" if actual == expected else f"MISMATCH (expected {expected})"
        print(f"  {table}: {actual} {status}")

    print("\nMigration complete.")
    print(f"Old DB backed up at: {BACKUP_DB}")
    print(f"New DB at: {OLD_DB}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Migrate knowledge DB to mcm-engine schema")
    parser.add_argument("--dry-run", action="store_true", help="Show what would be migrated without changing anything")
    args = parser.parse_args()
    migrate(dry_run=args.dry_run)
