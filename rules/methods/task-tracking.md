# Task Tracking Method

> **For AI agents**: Use ultralist for granular task tracking.
> Run `ultralist list` at session start to see pending work.

---

## Tool Hierarchy

| Tool | Purpose | When to Update |
|------|---------|----------------|
| **ultralist** | Granular actionable tasks | During work |
| **HANDOFF.md** | Current state, session context | End of session |
| **plan.md** | High-level goals, architecture | When goals change |

---

## Session Workflow

### At Session Start

```bash
ultralist list
```

This shows pending tasks. Pick one and work on it.

### During Work

```bash
# Mark task complete
ultralist c <id>

# Add new task discovered during work
ultralist add "description +project @context"

# Add note to existing task
ultralist addnote <id> "details about progress or blockers"
```

### At Session End

1. Complete finished tasks: `ultralist c <id>`
2. Add discovered tasks: `ultralist add "..."`
3. Update HANDOFF.md with session summary

---

## Ultralist Syntax

### Adding Tasks

```bash
# Basic task
ultralist add "Fix libtool for popt"

# With project tag
ultralist add "Fix libtool for popt +migration"

# With context
ultralist add "Test rpm install @irix"

# With priority (default false)
ultralist add "Critical fix for build" --priority

# With due date
ultralist add "Release milestone" --due tomorrow
```

### Listing Tasks

```bash
# All incomplete tasks
ultralist list

# Only prioritized
ultralist list --priority

# By project
ultralist list +migration

# By context
ultralist list @irix
```

### Managing Tasks

```bash
# Complete
ultralist c <id>

# Delete
ultralist d <id>

# Edit
ultralist e <id> "new description"

# Prioritize
ultralist p <id>

# Archive (hide completed)
ultralist ar <id>
```

---

## Project Tags

Use consistent tags for mogrix work:

| Tag | Meaning |
|-----|---------|
| `+migration` | sed→patch migration work |
| `+testing` | IRIX testing tasks |
| `+packages` | Building new packages |
| `+docs` | Documentation updates |
| `+compat` | Compat function work |

---

## Context Tags

| Tag | Meaning |
|-----|---------|
| `@irix` | Requires IRIX access |
| `@linux` | Linux-only task |
| `@review` | Needs human review |

---

## Example Session

```bash
# Start of session
$ ultralist list
1  [ ] Test tdnf install on IRIX +testing @irix
2  [ ] Migrate libsolv inline seds +migration
3  [ ] Migrate rpm inline seds +migration

# Work on task 1, it works!
$ ultralist c 1
$ ultralist add "Build ncurses package +packages"

# End of session
$ ultralist list
2  [ ] Migrate libsolv inline seds +migration
3  [ ] Migrate rpm inline seds +migration
4  [ ] Build ncurses package +packages
```

---

## Files

Ultralist stores tasks in `.todos.json` in the project root. This file can be committed to git if desired.
