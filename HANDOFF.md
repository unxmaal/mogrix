# Project Handoff

**Last Updated**: 2026-02-24 (session 124)
**Status**: ir8 browser memory-optimized and deployed on IRIX. Sites load (HN, Kagi, example.com). Google.com crashes WebProcess gracefully instead of OOM-killing the browser.

---

## Post-Compaction Checklist (READ THIS FIRST)

1. **Mogrix invocation**: `uv run mogrix <command>`
2. **Grep rules/INDEX.md** before attempting ANY fix
3. **Read GENERIC_SUMMARY.md** before starting any new package
4. **Paths**: `/opt/sgug-staging/` = Linux sysroot, `/opt/chroot` = IRIX test chroot (MCP tools)
5. **IRIX access**: Use MCP tools. NEVER raw SSH. `irix_copy_to` supports `host_path=true` + `owner="edodd"`.
6. **Large builds**: Redirect output to log file. NEVER let rpmbuild output flood context.
7. **WebKit rebuild**: `rpmbuild --define "_topdir $HOME/rpmbuild" --nodeps --rebuild ~/mogrix_outputs/SRPMS/webkitgtk-2.42.5-1.src-converted/webkitgtk-2.42.5-1.src.rpm > ~/tmp/webkit-build.log 2>&1`
8. **Bundle deploy**: `uv run mogrix bundle ir8`, then `test_bundle` MCP tool.

---

## Current State

### ir8 Browser — Working on IRIX

JSC with baseline JIT, memory-optimized for IRIX constraints (256MB-1.5GB RAM).

**Latest bundle**: `ir8-1.0-1-irix-bundle.0224262147` at `/usr/people/edodd/apps/`

**Test results**: example.com, HN, Kagi all load. Google.com WebProcess crashes but browser survives (graceful degradation — memory pressure system working as designed).

### ir8 Build Workflow (Non-obvious)

The ir8 input SRPM tarball is stale — `create-srpm ir8` fails (no git tag "1.0"). When rebuilding ir8 after editing `patches/packages/ir8/` files, you must manually repack the tarball:

```bash
uv run mogrix convert ~/mogrix_inputs/SRPMS/ir8-1.0-1.src.rpm
# Then repack tarball with updated files:
mkdir -p ~/tmp/ir8-repack
tar xzf ~/mogrix_outputs/SRPMS/ir8-1.0-1.src-converted/ir8-1.0.tar.gz -C ~/tmp/ir8-repack
cp patches/packages/ir8/{main.c,Makefile,glib-n32-fixup.h} ~/tmp/ir8-repack/ir8-1.0/
tar czf ~/mogrix_outputs/SRPMS/ir8-1.0-1.src-converted/ir8-1.0.tar.gz -C ~/tmp/ir8-repack ir8-1.0/
rm -rf ~/tmp/ir8-repack
# Rebuild SRPM then build:
rpmbuild -bs --define "_topdir $HOME/rpmbuild" --define "_srcrpmdir $HOME/mogrix_outputs/SRPMS/ir8-1.0-1.src-converted" --define "_sourcedir $HOME/mogrix_outputs/SRPMS/ir8-1.0-1.src-converted" --define "_specdir $HOME/mogrix_outputs/SRPMS/ir8-1.0-1.src-converted" ~/mogrix_outputs/SRPMS/ir8-1.0-1.src-converted/ir8.spec
uv run mogrix build ~/mogrix_outputs/SRPMS/ir8-1.0-1.src-converted/ir8-1.0-1.src.rpm --cross
```

**rpmbuild note**: `mogrix build` fails with false "missing dependencies" for webkitgtk. Use `rpmbuild --nodeps --rebuild` directly.

---

## Next Steps

1. **GLib n32 fixup needs to be generic** — `glib-n32-fixup.h` is ir8-specific. Any GLib/GTK app on n32 hits the same issue. Add to `compat/include/` or as a generic rule. See `rules/methods/glib-n32-compat.md`.

2. **Fix ir8 upstream SRPM** — either tag the repo with "1.0" or change `upstream:` config so `create-srpm ir8` works. Currently requires manual tarball repack (see above).

3. **Google.com still crashes WebProcess** — options: (a) try `JSC_useJIT=false` to eliminate JIT memory overhead, (b) lower memory limit further, (c) accept graceful failure on heavy sites.

4. **Many bundle binaries fail `--version`** (64 of 111) — pre-existing issue. Utilities (bzip2, sqlite3, zstd, etc.) exit rc=1 with no output. Investigate rld or bundler issue.

5. **Performance benchmarks** — compare CLoop vs JIT `jsc` execution times.

---

## Recent Work (Sessions 122-124)

### Session 124
- ir8 memory optimization: memory pressure settings, DOCUMENT_VIEWER cache, disabled process swap, disabled page cache/offline cache/localStorage/media/webaudio, JSC GC tuning. Details: `rules/methods/webkit-memory.md`
- GLib 2.80 n32 build fix: `glib-n32-fixup.h`. Details: `rules/methods/glib-n32-compat.md`
- CC export fix for hand-written specs. Details: `rules/methods/makefile-builds.md`
- Created rules files, trimmed INDEX.md to use pointers instead of inline details

### Session 123
- JIT working on IRIX: endian patch, .cpload disable, push/pop alignment, three-step LLInt wrapper (5 iterations). All fixes in `rules/packages/webkitgtk.yaml` and `patches/packages/webkitgtk/`.
- jsc passes fib(30) test. Bundle deployed.

### Session 122
- Initial JIT implementation (7 build iterations), endian patch (10 functions)

---

## Key Files

- **ir8 sources**: `patches/packages/ir8/` (20 files including `glib-n32-fixup.h`)
- **ir8 rules**: `rules/packages/ir8.yaml`
- **Memory optimization docs**: `rules/methods/webkit-memory.md`
- **GLib n32 fix docs**: `rules/methods/glib-n32-compat.md`
- **WebKit rules**: `rules/packages/webkitgtk.yaml`
- **WebKit patches**: `patches/packages/webkitgtk/` (LLInt wrapper, endian patch, FP64 patch)
