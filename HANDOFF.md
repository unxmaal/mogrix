# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-15 (session 40)
**Status**: 105+ source packages cross-compiled (290+ RPMs). All 35 bundle packages rebuilt with dlmalloc spin locks + high-fd fix. 20 bundles (.run installers) created. Essentials, bash, and extras verified on IRIX.

---

## Critical Warnings

**NEVER install packages directly to /usr/sgug on the live IRIX system.** Always use `/opt/chroot` for testing.

**NEVER use raw SSH to IRIX.** Always use MCP tools (`irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`).

**NEVER CHEAT.** Don't copy from SGUG-RSE. Don't fake Provides. Every FC40 package must be properly cross-compiled.

---

## Current State

**All bundles rebuilt (session 40)**: 35 packages rebuilt, 20 bundles created with hardened dlmalloc:

**Suites** (5):
| Suite | Packages | Size |
|-------|----------|------|
| mogrix-essentials | nano, grep, sed, gawk, less, coreutils, findutils, diffutils, tar, tree-pkg | 29.6 MB |
| mogrix-extras | bc, jq, man-db, tmux, vim-enhanced | 28.2 MB |
| mogrix-net | curl, rsync, gnupg2 | 21.1 MB |
| mogrix-smallweb | telescope, gmi100, lynx, snownews | 12.4 MB |
| mogrix-fun | cmatrix, figlet, sl | 0.7 MB |

**Individual bundles** (15): bash, bc, bitlbee, dmenu, groff, jq, man-db, rxvt-unicode, st, tcsh, tinc, tmux, vim-enhanced, weechat, wget2

**Deployed on IRIX** (`/usr/people/edodd/apps/`):
- mogrix-essentials (verified: bash, grep, sed, gawk, less, ls, find, diff, tar, nano, bc, tmux, vim, xxd, man, jq)
- bash (verified: $BASH_VERSION = 5.2.26, brace expansion, for loops)
- mogrix-extras (verified: vim 9.1, tmux 3.5a, bc, jq, man-db)

**Trampoline exclusions working**: bash bundle only installs `bash` and `bashbug` trampolines — `sh`, builtins (alias, cd, bg, fg, etc.) correctly excluded.

**Known issue — jq abort on exit**: jq produces correct output but calls abort() during exit cleanup. Functional but cosmetic. Not dlmalloc-related (output verified via par trace: dlmalloc high-fd working, fd 128 with FD_CLOEXEC).

**Build fixes this session**:
- **bash**: `void dprintf` → `int dprintf` in externs.h (POSIX conflict with compat header). `LIBS_FOR_BUILD=${LIBS}` → `LIBS_FOR_BUILD=` in support/Makefile.in (cross-compiled -lmogrix-compat leaking into native man2html build).
- **rsync**: Hardcoded SOURCE100-108 loop → SOURCE100-106 (only 7 compat sources exist, not 9).

**IRIX tar gotcha confirmed**: Native IRIX `tar` silently drops long paths. Must use `gtar` from chroot for extraction, then `gtar` pipe for copying to host filesystem.

---

## Next Steps

1. **Deploy remaining bundles to IRIX**: Copy and install mogrix-net, mogrix-smallweb, mogrix-fun, weechat, and other individual bundles
2. **Test X11 bundles from real terminal**: dmenu, rxvt-unicode, st
3. **Investigate jq abort-on-exit**: Low priority, functional output is correct
4. **Pending tasks:**
   - Build qtermwidget5 + qterminal
   - Add convert-time lint for duplicated rules
   - Implement mogrix elevate command
   - htop: BLOCKED (needs IRIX /proc platform backend)
5. **Low priority**: apropos segfault

---

## Recent Work

### Session 40: Full bundle rebuild + bash/rsync fixes

**All 35 packages rebuilt** with hardened dlmalloc (spin locks + high-fd). 33/35 succeeded initially; bash and rsync required fixes:
- **bash**: Two fixes — dprintf void→int conflict, LIBS_FOR_BUILD cross-contamination
- **rsync**: SOURCE100-108 loop referenced non-existent sources (only 100-106 defined)

**20 bundles created** (.run self-extracting installers). Trampoline exclusion system working — bash correctly excludes sh and builtins.

**Deployed and verified essentials, bash, extras on IRIX**: grep, sed, gawk, less, nano, coreutils, findutils, diffutils, tar, bash, vim, tmux, bc, jq, man-db all functional. Old bundles cleaned up.

**IRIX deployment lesson**: Native tar silently drops long paths. Always use gtar from chroot for extraction, then gtar pipe for host copy: `gtar cf - -C /tmp bundle-dir | gtar xf - -C /usr/people/edodd/apps/`

### Session 39: dlmalloc test suite + thread safety fix

**Focused dlmalloc test suite**: Wrote `tests/dlmalloc-test.c` (29 tests) to proactively find defects before rebuild. Found:
1. **Thread safety (CRITICAL)**: `USE_LOCKS=0` → 6/10 runs crash with concurrent pthreads. Fix: `USE_LOCKS=1 + USE_SPIN_LOCKS=1` (MIPS ll/sc atomics). 20/20 clean after fix.
2. **`-O2` optimization gotcha**: Compiler eliminated malloc+free pairs, making dlmalloc appear inactive. Fix: volatile sink pattern.
3. All other tests pass: page size (16384), /dev/zero fd (>=128, FD_CLOEXEC), calloc zeroing+overflow, memalign, 500MB allocs, fork safety, 50K stress cycles.

### Session 38: tcsh bundle + dlmalloc /dev/zero high-fd fix

**tcsh 6.24.10**: Built and bundled. Configure vendor match fix: sed `*-sgi-iri*` to `*-*-iri*` because config.sub canonicalizes `mips-irix6.5` to `mips-unknown-irix6.5`.

**dlmalloc /dev/zero high-fd fix**: dlmalloc opens `/dev/zero` for mmap and caches the fd (typically fd 3). Programs like tcsh that close low fds (0-14) invalidate this cached fd. Fix: `fcntl(fd, F_DUPFD, 128)` + FD_CLOEXEC.

### Session 37: SOLVED weechat TLS ABORT -- dlmalloc cross-heap corruption

**Fix -- dlmalloc moved to executables only**. `-Bsymbolic-functions` + per-library dlmalloc = cross-heap corruption. Now dlmalloc.o in exe only; .so inherits via rld.

---

## Reference

| What | Where |
|------|-------|
| Technical patterns & fixes | `rules/INDEX.md` |
| Package rules | `rules/packages/<name>.yaml` |
| Project architecture | `plan.md` |
| Build methods | `rules/methods/*.md` |
| Compat functions | `compat/catalog.yaml` |
| Workspace paths & workflow | `CLAUDE.md` |
| dlmalloc test suite | `tests/dlmalloc-test.c` |
