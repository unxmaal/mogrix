# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-24 (session 124)
**Status**: ir8 browser memory-optimized for IRIX. Google.com no longer kills the browser (WebProcess crash is gracefully handled). Other sites (HN, Kagi, example.com) load fine.

---

## Post-Compaction Checklist (READ THIS FIRST)

1. **Mogrix invocation**: `uv run mogrix <command>`
2. **Grep rules/INDEX.md** before attempting ANY fix
3. **Read GENERIC_SUMMARY.md** before starting any new package
4. **Paths**: `/opt/sgug-staging/` = Linux sysroot, `/opt/chroot` = IRIX test chroot (MCP tools)
5. **IRIX access**: Use MCP tools. NEVER raw SSH. `irix_copy_to` supports `host_path=true` + `owner="edodd"`.
6. **Large builds**: Redirect output to log file. NEVER let rpmbuild output flood context.
7. **WebKit rebuild**: `uv run mogrix convert ~/mogrix_inputs/SRPMS/webkitgtk-2.42.5-1.src.rpm && rpmbuild --define "_topdir $HOME/rpmbuild" --nodeps --rebuild ~/mogrix_outputs/SRPMS/webkitgtk-2.42.5-1.src-converted/webkitgtk-2.42.5-1.src.rpm > ~/tmp/webkit-build.log 2>&1`
8. **Bundle deploy**: `uv run mogrix bundle ir8`, then `test_bundle` MCP tool.

---

## Current State

### JIT — WORKING ON IRIX

JSC with baseline JIT + DFG compiles, links, and runs correctly on IRIX:
- `jsc -e "print(1+1)"` → `2`
- `jsc -e "function fib(n){...} print(fib(30))"` → `832040` (2.7M recursive calls)
- `ir8` binary loads (needs X display to proceed past GTK init)

**Latest bundle**: `ir8-1.0-1-irix-bundle.0224262147` at `/usr/people/edodd/apps/`

### Memory Optimization (Session 124)

ir8 now has comprehensive memory tuning for IRIX's constrained environment:

**C code changes** (`patches/packages/ir8/main.c`):
- WebKitMemoryPressureSettings: 512MB limit, conservative=0.33, strict=0.50, kill=0.90, poll=10s
- Cache model: DOCUMENT_VIEWER (no memory/disk cache, saves 30-50MB)
- Process swap on cross-site: DISABLED (keeps one WebProcess, saves 80-120MB per navigation)
- Disabled: page cache, offline cache, localStorage, HTML5 DB, media, webaudio

**Launcher env vars** (`mogrix/bundle.py`):
- JSC_forceRAMSize=536870912 (tell GC it's a 512MB machine)
- JSC_criticalGCMemoryThreshold=0.50 (aggressive GC trigger)
- JSC_small/medium/largeHeapGrowthFactor=1.1 (slow heap growth, WPE pattern)

**Build fixes** (discovered during this session):
- `glib-n32-fixup.h`: GLib 2.80 added `G_STATIC_ASSERT(sizeof *(location)==sizeof(gpointer))` to ALL `g_once_init_enter/leave` macro variants, breaking n32. Fix: `-include glib-n32-fixup.h` which undefs the macros so function versions are called directly.
- `spec_replacements`: `export CC="%{__cc}"` before `%make_build` — hand-written specs don't get CC from `%configure`.

**Test results**:
- example.com: PASS (rc=0)
- news.ycombinator.com: PASS (rc=0)
- kagi.com: PASS (renders, interactive)
- google.com: WebProcess crashes but browser survives (graceful degradation vs previous full OOM kill)

### What's in mogrix rules

All JIT fixes are stored in `rules/packages/webkitgtk.yaml` and `patches/packages/webkitgtk/`:

- **Endian patch**: `jit-bigendian-jsvalue32.patch` — 10 functions in AssemblyHelpers.h with `#if __BYTE_ORDER__` guards for storePair32/loadPair32 arg swaps
- **FP64 patch**: `jit-mips-vmov-fp64.patch` — stack-based FP register transfer for FR=1
- **Three-step LLInt wrapper**: `compile-llint-twostep.sh` — bypasses clang MIPS 56GB memory bug:
  1. `clang -emit-llvm -c` → bitcode (126MB)
  2. `llc -mtriple=mips64 -target-abi n32 -mattr=+xgot -relocation-model=pic` → assembly
  3. Post-process: strip `.size`/`.file`/`$tmp`, convert `%got()`/`%got_disp()` → `%got_hi`/`%got_lo` pairs
  4. `mips-sgi-irix6.5-as -mabi=n32 -KPIC` → object
- **Safepatches**: PlatformEnable.h (JIT+DFG on), InlineASM.h (sys/cachectl.h), LLIntOfflineAsmConfig.h (.cpload disable)
- **Perl fixes**: `mul` → `mult`+`mflo` in mips.rb, push/pop 4→8 byte alignment
- **cmake injection**: RULE_LAUNCH_COMPILE on LowLevelInterpreterLib

**rpmbuild note**: `mogrix build` fails with false "missing dependencies" for webkitgtk. Use `rpmbuild --nodeps --rebuild` directly (see checklist above).

---

## Next Steps

1. **Google.com still crashes WebProcess** — the memory optimizations turned a full-OOM browser kill into a graceful WebProcess crash. Google's JS is simply too heavy for 512MB. Options: (a) try `JSC_useJIT=false` to eliminate JIT memory overhead, (b) lower memory limit further, (c) accept graceful failure on heavy sites.

2. **Many bundle binaries fail `--version`** (64 of 111) — this is a pre-existing issue unrelated to memory optimization. Many bundled utilities (bzip2, sqlite3, zstd, etc.) exit with rc=1 and no output. Investigate if this is an rld or bundler issue.

3. **GLib n32 fixup needs to be generic** — `glib-n32-fixup.h` is currently ir8-specific. Any GLib/GTK app on n32 will hit the same issue. Consider adding it to compat/include or as a generic rule.

4. **Upstream SRPM tarball is stale** — `create-srpm ir8` fails (no tag "1.0"). The tarball in the input SRPM doesn't match current patches/packages/ir8/ files. Had to manually repack. Fix: either tag the repo or update the upstream config.

5. **Consider performance benchmarks** — compare CLoop vs JIT `jsc` execution times on a JS benchmark suite.

---

## Recent Work (Sessions 121-124)

### Session 124 (current)
- Implemented ir8 memory optimization plan (WPE/PlayStation/Haiku patterns)
- Added WebKitMemoryPressureSettings, DOCUMENT_VIEWER cache, disabled process swap
- Disabled page cache, offline cache, localStorage, HTML5 DB, media, webaudio
- Added JSC GC tuning: forceRAMSize, criticalGCMemoryThreshold, heap growth factors
- Fixed GLib 2.80 n32 build: created glib-n32-fixup.h to undef broken g_once_init macros
- Fixed CC export for hand-written specs via spec_replacements rule
- Deployed and tested on IRIX: example.com, HN, Kagi all work. Google crashes WebProcess gracefully.

### Session 123
- Implemented comprehensive JIT plan: endian patch (3 missing functions), .cpload disable, push/pop alignment
- Fixed three-step LLInt wrapper through 5 iterations:
  - ccache detection (cmake passes ccache as $1)
  - .size/.file/$tmp stripping (LLVM MIPS assembler bugs)
  - `-target-abi n32` for proper N32 code gen (not mips64 64-bit addresses)
  - `%got()`/`%got_disp()` → `%got_hi`/`%got_lo` xgot conversion (GOT >64KB overflow)
  - `-KPIC` flag for PIC/CPIC ELF flags
- Full rpmbuild completed: JSC, WebCore, WebKit, all binaries
- Deployed bundle, confirmed jsc works (fib(30)=832040)
- Updated rules/INDEX.md with 4 new LLInt entries

### Session 122
- Implemented initial JIT plan, built iteratively (7 attempts)
- Created endian patch (7 functions initially), identified 3 more needed

### Session 121
- Discovered clang MIPS 56GB memory bug, created two-step compile workaround
- Found LLInt `mul` instruction issue, fixed to `mult`+`mflo`

---

## Key Files

- **Three-step wrapper**: `patches/packages/webkitgtk/compile-llint-twostep.sh`
- **Endian patch**: `patches/packages/webkitgtk/jit-bigendian-jsvalue32.patch`
- **FP64 patch**: `patches/packages/webkitgtk/jit-mips-vmov-fp64.patch`
- **WebKit rules**: `rules/packages/webkitgtk.yaml`
- **WebKit spec**: `specs/packages/webkitgtk.spec`
- **ir8 sources**: `patches/packages/ir8/` (20 files, includes glib-n32-fixup.h)
- **ir8 rules**: `rules/packages/ir8.yaml` (add_source, spec_replacements, smoke_test)
- **Latest ir8 bundle**: `ir8-1.0-1-irix-bundle.0224262147` at `/usr/people/edodd/apps/`
