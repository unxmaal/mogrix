# Sub-Agent Prompt Templates

These templates are MANDATORY. Copy them verbatim into agent prompts.
Do not paraphrase, abbreviate, or omit the constraint blocks.

---

## Monitoring Agent (watching build progress)

```
Monitor the rebuild-all log at /tmp/rebuild-all.log.

YOU ARE READ-ONLY. You must follow these constraints — violation is immediate termination:
- NEVER run mogrix, rpmbuild, uv, make, cmake, or any build/compile command
- NEVER edit, create, or delete any file
- NEVER run sed -i, tee, >, or >> on any file
- NEVER run git commit, git add, git checkout, or any git write command
- NEVER modify rules/, compat/, patches/, cross/, mogrix/, or any source file
- ONLY use: tail, cat, head, grep, ls, find, wc, git log, git diff, git show

Check every 60 seconds using `tail -50 /tmp/rebuild-all.log`.
Watch for:
- FAILED packages (report name + error line)
- Progress milestones (every ~30 packages)
- Completion (look for "Rebuild Summary")

Continue for 15 minutes or until completion/failure.
Return: packages passed count, any failure names+errors, final progress X/N.
```

---

## Investigation Agent (reading build logs to diagnose errors)

```
Read the last 200 lines of <LOG_FILE> and find the build error.

YOU ARE READ-ONLY. You may read files and run non-destructive commands.
- NEVER run mogrix, rpmbuild, uv, make, cmake, or any build/compile command
- NEVER edit, create, or delete any file
- NEVER modify rules/, compat/, patches/, cross/, mogrix/, or any source file
- Return a concise summary to the parent. The PARENT applies fixes.

Focus on:
1. The actual compiler/linker error (search for "error:" near the end)
2. The specific symbol/file/line involved
3. A 1-line diagnosis
```

---

## Rules for the orchestrator (the agent launching sub-agents)

1. **Always copy the constraint block verbatim** from this file into sub-agent prompts.
   Do not paraphrase or abbreviate.

2. **Monitoring agents get the monitoring template.** They watch logs and report.
   They never build, fix, or modify anything.

3. **Investigation agents get the investigation template.** They read a specific
   log file and return a diagnosis. They never build, fix, or modify anything.

4. **Only the orchestrator modifies files, runs builds, and commits changes.**
   Sub-agents are eyes, not hands.

5. **If a sub-agent needs to run a build** (e.g., for a batch build wave),
   it gets a DIFFERENT template (see rules/methods/task-tracking.md Rule 0).
   Build agents use isolated rpmbuild directories and follow the full
   task-tracking protocol. They are NOT monitoring or investigation agents.
