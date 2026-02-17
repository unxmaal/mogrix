# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-17 (session 67)
**Status**: 154 bundles shipping. 124 test results. **nedit 5.7 WORKING on IRIX** — renders Motif window correctly with menu bar and text area.

---

## Critical Warnings

**NEVER install packages directly to /usr/sgug on the live IRIX system.** Always use `/opt/chroot` for testing.

**NEVER use raw SSH to IRIX.** Always use MCP tools (`irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`) or the new `mogrix-test` MCP tools.

**NEVER CHEAT.** Don't copy from SGUG-RSE. Don't fake Provides. Every FC40 package must be properly cross-compiled.

**CHECK USEFULNESS FIRST.** Before porting a package, verify it will actually work on IRIX. See `rules/methods/before-you-start.md` section 0.

**SYNC STAGING -> CHROOT.** After rebuilding any library, compare sizes and redeploy to chroot. Mismatched versions cause silent SIGABRT. See INDEX.md "Staging/chroot library mismatch".

---

## Current State

### mogrix-test MCP Server: ACTIVE

Both MCP servers working (`irix` + `mogrix-test`). Test results in `test-results/`.

### Bundles: 154 shipping

All in `~/mogrix_outputs/bundles/`. Individual apps + suites.

**Session 65 additions:**
- **Built woff2 1.0.2** — cmake cross-compilation. 7.1 MB bundle. Key fix: drop_subpackages doesn't move files, must use spec_replacements to add binaries to main %files
- **Built GraphicsMagick 1.3.40** (via agent) — 9.9 MB bundle, 50 binaries, 25 packages
- **Built SDL2 2.30.1** (via agent) — library-only bundle (0.1 MB). Now available as dependency
- **Comprehensive IRIX testing sweep** — tested ALL remaining untested bundles:
  - Individual packages: banner(1/1), boxes(13/13), chrpath(1/1), diffstat(1/1), dtach(1/1), figlet(4/4), gperf(1/1), help2man(1/1), keychain(1/1), patchutils(31/31), screenfetch(1/1), sl(11/11), symlinks(1/1), time(1/1), giflib-utils(13/13), libjpeg-turbo-utils(5/5), libpng-tools(1/1), libtiff-tools(16/16), libsndfile-utils(11/11), uuid(1/1), slang-slsh(2/2), scrot(42/42), lemon(2/2), gzip(14/14), gnutls-utils(28/28), bomtool(4/5)
  - Suite bundles: mogrix-compression(44/44), mogrix-compression2(44/44), mogrix-coretools(173/174), mogrix-editors(22/22), mogrix-essentials(157/158), mogrix-extras(57/58), mogrix-fun(17/17), mogrix-fun2(22/24), mogrix-fun3(3/3), mogrix-funtools(2/4), mogrix-buildtools(45/47), mogrix-clitools(15/15), mogrix-devtools(49/54), mogrix-imagetools(52/55), mogrix-media(55/56), mogrix-net(59/60), mogrix-networking(62/65), mogrix-scripting(49/51), mogrix-security(68/70), mogrix-shells(41/42), mogrix-smallweb(34/34)
- Known failures (all pre-existing, not new):
  - dash: SIGSEGV (needs investigation)
  - rsync: SIGABRT (likely dlmalloc cross-heap)
  - cowsay/cowthink: Perl script shebang issues (signal 127)
  - cwebp/dwebp/gif2webp: crash on IRIX (signal 127)
  - automake-1.16/aclocal-1.16: versioned symlink issues
  - Various smoke test string mismatches (cosmetic)

**Session 64 additions:**
- Built woff2, tested 41+ individual packages on IRIX (bash 32/32, curl 6/6, man-db 41/41, wget2 47/47, gnupg2 46/46, etc.)

**Session 63 additions:**
- Built + tested feh, lua. Tested profanity. Bundled openssh, perl, tcl, mksh, gawk. New suites: scripting, networking.

**Previous sessions:**
- Git 2.44.0: 48.1 MB bundle, 22/25 tests pass
- 60+ packages bundled in sessions 60-61
- GTK3 working on IRIX

---

## Next Steps

1. **Build gtkterm** (GTK3 serial terminal) — first real GUI app bundle (no SRPM — needs upstream tarball)
2. **Investigate dash SIGSEGV** — dash crashes with signal 11 on `--version`
3. **Investigate rsync SIGABRT** — likely dlmalloc cross-heap issue
4. **Fix cwebp/dwebp/gif2webp crashes** — signal 127 on IRIX
5. **More packages** — look at packages_plan.md for candidates

### Dropped from old plan (not useful on IRIX)

- dbus, dbus-glib, icu, cyrus-sasl (dbus is pointless on IRIX)
- fossil (niche), mercurial (needs Python)
- hexchat, pidgin (replaced with gtkterm + nedit as first GUI targets)
- fastfetch (too many Linux-specific deps: dbus, NetworkManager, pciutils)
- neofetch (bash script, needs /usr/sgug/bin/bash which isn't standard)
- mupdf (too many missing deps: gumbo-parser, jbig2dec, leptonica, tesseract, openjpeg2, freeglut, mesa)

### Blocked/Skipped Packages

| Package | Reason |
|---------|--------|
| gdb 14.2 | Binary crashes on IRIX (SIGABRT) — no native debug support |
| ksh | ~3400-line custom build system |
| htop | Needs Linux /proc backend |
| openjpeg2 | No SRPM available |
| m4 rebuild | gnulib nanosleep conflicts (old build works) |
| mupdf | Too many missing dependencies |

---

## Recent Work

### Session 67: nedit FIXED (R_MIPS_REL32 partial failure workaround)

**Root cause identified**: IRIX rld silently fails to apply some R_MIPS_REL32 relocations in large
cross-compiled executables. In nedit's binary, relocations 4-22 (out of 24) in the .rel.dyn table
were not applied, leaving superclass pointers and `_XtInherit` method pointers as NULL in widget
ClassRec structures. Small test binaries with the same symbol references work fine — the issue is
binary-size-dependent (likely related to GOT size or relocation table position).

**Affected class records**: `xmlFolderClassRec` (Microline/XmL/Folder.c) and `xrwsBubbleButtonClassRec`
(Xlt/BubbleButton.c). `textClassRec` and `xltSlideContextClassRec` were NOT affected.

**Fix**: `patches/packages/nedit/fix_class_recs.c` — an `__attribute__((constructor))` function that
runs before main() and patches all broken fields. Declares class records as parent types (XmManagerClassRec,
XmPushButtonClassRec) to avoid needing private headers. Checks each field `== NULL` then assigns the
GOT-resolved symbol value (GOT references are resolved correctly by rld).

**Result**: NEdit 5.7 renders a proper Motif window on IRIX with menu bar (File, Edit, Search,
Preferences, Shell, Macro, Windows) and grey text editing area. Confirmed via screenshot.

**Files changed**:
- `patches/packages/nedit/fix_class_recs.c` — constructor workaround (new file)
- `rules/packages/nedit.yaml` — add_source, prep_commands, spec_replacements (debug code removed)
- `rules/INDEX.md` — added "R_MIPS_REL32 partial failure" entry

**Lesson**: This pattern may apply to any large cross-compiled Motif/Xt executable with widget
class records referencing external symbols. The constructor workaround is reusable.

### Session 66: nedit debugging (XMSTRINGDEFINES fix + SIGBUS investigation)

**Key fix applied this session**: Added `-DXMSTRINGDEFINES -DXTSTRINGDEFINES` to `cross/bin/irix-cc`.
This forces Motif/Xt resource name macros to expand to plain string literals instead of pointer
expressions that generate R_MIPS_REL32 relocations. **This fix applies to ALL packages using Motif/Xt.**

### Session 65: woff2 + GraphicsMagick + SDL2 + comprehensive testing

- **Built woff2 1.0.2** — cmake cross-compilation, 2 build iterations
- **Built GraphicsMagick 1.3.40** via background agent — 50 binaries, 9.9 MB installer
- **Built SDL2 2.30.1** via background agent — library-only, available as dependency
- **Comprehensive IRIX testing** — tested every remaining untested bundle (individual + suites)
- **124 test result files**, 154 bundle installers
- **mupdf rejected** — too many missing dependencies

### Session 64: woff2 build + massive testing sweep

- Built woff2, tested 41+ individual packages, 105 total packages tested

### Session 63: feh + lua + suites

- Built + tested feh (6 iterations). Built + tested lua. New suites: scripting, networking.

### Session 62: Massive bundling + profanity + screen fix

- Fixed screen RPM packaging bug. Bundled 25+ packages. Built profanity. Created buildtools/compression/fun3 suites.

### Session 61: Git built! Massive bundling wave

- Git 2.44.0 cross-compiled after 32 build iterations. Bundled 30+ packages. Created 4 new suites.

### Session 60: Test harness MCP server + task list revision

- Built `tools/mogrix-test-server.py` — 780-line Python MCP server with 6 tools

---

## Reference

| What | Where |
|------|-------|
| Generic rules summary | `rules/GENERIC_SUMMARY.md` (read this first) |
| Problem lookup | `rules/INDEX.md` (grep, don't read whole file) |
| Test harness | `tools/mogrix-test-server.py` + `.mcp.json` |
| Package plan & tiers | `packages_plan.md` |
| Agent orchestration | `rules/methods/task-tracking.md` |
| Package rules | `rules/packages/<name>.yaml` |
| Build methods | `rules/methods/*.md` |
| Compat functions | `compat/catalog.yaml` |
| Workspace paths & workflow | `CLAUDE.md` |
| SGUG RSE reference | `/home/edodd/projects/github/sgug-rse` |

---

## Post-Compaction Checklist (READ THIS FIRST)

You just woke up from a context compaction. You WILL get basic things wrong unless you re-anchor now.

1. **Mogrix invocation**: `uv run mogrix <command>` — not `mogrix`, not `python -m mogrix`
2. **Grep INDEX.md** before attempting ANY fix: `Grep pattern="keyword" path="rules/INDEX.md"` — the answer is probably already there
3. **Read GENERIC_SUMMARY.md** before starting any new package
4. **Paths are different things**:
   - `/opt/sgug-staging/` = cross-compilation sysroot (on Linux build host)
   - `/opt/chroot` = IRIX test chroot (on IRIX host, accessed via MCP tools)
   - These are NOT the same. Don't mix them up.
5. **Chroot is root-owned**. Everything in `/opt/chroot` is owned by root. Don't be surprised by permission errors.
6. **IRIX access**: Use MCP tools (`irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`). NEVER raw SSH.
7. **Testing**: Use `mogrix-test` MCP tools (`test_bundle`, `test_binary`, `check_deps`, `par_trace`, `screenshot`). NOT ad-hoc irix_exec calls.
8. **Compat functions**: Grep `compat/catalog.yaml` before writing a new one — it probably already exists.
