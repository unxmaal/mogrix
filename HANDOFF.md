# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-21 (session 98)
**Status**: Sed elimination nearly complete — down to 5 irreducible `sed -i` warnings (from 300+ originally). 29 more seds converted this session across 16 packages. All verified IDENTICAL via test-prep. WebKit IPC debug bundle still ready to deploy.

---

## Post-Compaction Checklist (READ THIS FIRST)

1. **Mogrix invocation**: `uv run mogrix <command>` — not `mogrix`, not `python -m mogrix`
2. **Grep rules/INDEX.md** before attempting ANY fix — the answer is probably already there
3. **Read GENERIC_SUMMARY.md** before starting any new package
4. **Paths are different things**:
   - `/opt/sgug-staging/` = cross-compilation sysroot (on Linux build host)
   - `/opt/chroot` = IRIX test chroot (on IRIX host, accessed via MCP tools)
5. **IRIX access**: Use MCP tools. NEVER raw SSH.
6. **Testing**: Use `mogrix-test` MCP tools. NOT ad-hoc irix_exec calls.
7. **Compat functions**: Grep `compat/catalog.yaml` before writing a new one.
8. **Always use full mogrix pipeline** for builds — manual compilation creates dirty state.
9. **Deploy irix-ld after editing**: `cp cross/bin/irix-ld /opt/sgug-staging/usr/sgug/bin/irix-ld`
10. **GSettings schemas**: Run `glib-compile-schemas` in chroot after installing GTK3 apps.
11. **NEVER install packages directly to /usr/sgug on the live IRIX system.** Always use `/opt/chroot`.

---

## Current State

### Sed Elimination Plan — Progress

**Completed:**
- `mogrix test-prep` command (snapshot + compare)
- safepatch extensions (`--delete-line`, `--insert-after`, `--insert-before`, `--insert-top`, `--regex`)
- `sed -i` validator warning in `mogrix/rules/validator.py`
- Baseline snapshots for all packages
- **Task #12: Literal sed→safepatch migration** — ~85+ calls converted in 25+ packages
- **Tasks #13/#14: Advanced seds** — 29 more seds converted across 16 packages (session 98)

**Conversion techniques used (session 98):**
- Literal replacements: safepatch `--old/--new`
- Regex replacements: `--regex` mode (e.g., `\bTICK\b`, `^#if defined \(HAVE_PSELECT\)$`)
- Multi-line matches: `$'...\n...'` for cross-line patterns
- Line-number seds: Converted to safepatch regex with `^`/`$` anchors
- Range deletes: Converted to `perl -0777 -pi -e` or `perl -ni -e`
- Multi-file bulk: `for f in $(find ...); do safepatch ... || :; done`
- Context-dependent: Include surrounding text in `--old` for uniqueness

**Verification:** All 16 packages re-verified via test-prep --compare (IDENTICAL).

**Remaining (5 irreducible warnings):**
- `icu.yaml`: `find|xargs sed` for %zu→%u across hundreds of source files (impractical with safepatch)
- `pwgen.yaml`: Regex backreference `&` (safepatch can't do this)
- `qt5-qtbase.yaml`: `find|xargs sed` for %zu across many 3rdparty files
- `tinc.yaml` (2): Insert before default case `*)` + range append in Makefile.am

These are either bulk operations across many files or use regex features safepatch doesn't support.

### WebKit IPC Debugging

**Bundle ready to deploy**: `~/mogrix_outputs/bundles/libwebkit2gtk-2.42.5-1-irix-bundle.0221260057.run` (103MB)

**What we know:** WebProcess gets SIGTERM (not crash), IPC primitives all work, GOT overflow fixed. WebProcess runs 25-47s before connection drops. Bundle has 12 IPC_LOG injection points writing to `/usr/people/edodd/ipc_<pid>.log`. See INDEX.md for details.

**Next step:** Deploy → run MiniBrowser → collect logs → identify which CLOSE path fired.

---

## Next Steps (prioritized)

1. **Deploy WebKit IPC debug bundle** — Copy to IRIX, run MiniBrowser, collect logs. Bundle ready at `~/mogrix_outputs/bundles/libwebkit2gtk-2.42.5-1-irix-bundle.0221260057.run`.
2. **Sed elimination polish** — 5 irreducible warnings remain (bulk find+xargs, regex backrefs). Could add `# safepatch:ok` exception comments or accept as-is.
3. **New package work** — See smallweb_plan.md for next packages to port.

---

## Recent Work (sessions 96-98)

**Session 98 (sed migration — advanced):**
- Converted 29 more seds across 16 packages: yasm(6), rxvt-unicode(4), st(3), bash(2), qt5-qtbase(2), tinc(2), icu(1), vim(1→2 calls), gdb(1), telescope(1), cairo(1), dash(1), feh(1), openssh(1), p11-kit(1), libxkbcommon(1)
- Techniques: regex mode, multi-line `$'...\n...'` matches, perl range deletes, for-loop bulk replacement
- Down to 5 irreducible sed -i warnings (from 34 at start of session, 300+ originally)
- All 16 packages verified IDENTICAL via test-prep

**Session 96-97 (sed migration — literals):**
- Built `mogrix test-prep` command, extended safepatch, added validator
- Converted ~85+ literal seds across 45+ packages

**Session 95 (WebKit IPC + sed planning):**
- Built WebKit IPC debug bundle with 12-point logging

---

## Key Commands

```sh
# Bundle + test
uv run mogrix bundle libwebkit2gtk

# Verify sed→safepatch migration
uv run mogrix test-prep <package> --compare

# Take new baseline snapshot
uv run mogrix test-prep <package>

# rld analysis harness
python3 tools/rld-harness.py analyze ~/mogrix_outputs/bundles/<bundle-dir>/

# Copy to IRIX host (two-step: chroot then host)
irix_copy_to local_path /opt/chroot/tmp/file
irix_host_exec "cp /opt/chroot/tmp/file /usr/people/edodd/file"
```
