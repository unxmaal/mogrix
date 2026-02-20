# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-20 (session 88+)
**Status**: R_MIPS_REL32 fix COMPLETE and integrated. jsc runs full JavaScript. MiniBrowser loads but has Pango class size mismatch blocking rendering.

---

## Critical Warnings

**NEVER install packages directly to /usr/sgug on the live IRIX system.** Always use `/opt/chroot` for testing.
**NEVER use raw SSH to IRIX.** Always use MCP tools.
**ALWAYS use full mogrix pipeline** (`create-srpm` → `convert` → `build --cross`).

---

## Current State

### R_MIPS_REL32 Fix: COMPLETE AND INTEGRATED

Tool: `cross/bin/fix-anon-relocs` (called automatically by irix-ld post-link)
- Two-symbol approach with STV_PROTECTED: repoints anonymous R_MIPS_REL32 to two defined symbols
- Fixes named R_MIPS_REL32 addend formula mismatch (pre-adds st_value)
- Sorts .rel.dyn, clears .MIPS.msym
- Safe for all .so files (exits early if no anonymous R_MIPS_REL32)

**jsc test results (ALL PASS):**
- Recursion (f(5)), Array.sort, closures, prototypes, Map, JSON, regex, fib(20), try/catch, Date
- No crashes under address displacement

### libmogrix_compat.so: Updated

Now contains: bsearch, socketpair, mincore, __muloti4
- socketpair: Complete AF_UNIX reimplementation (SOCK_SEQPACKET→SOCK_STREAM fallback)
- Source: `compat/sys/socketpair.c`
- Deployed to staging and IRIX bundle

### MiniBrowser: LOADS BUT NO RENDERING

**Blockers (in order of resolution):**
1. ~~rld unresolvable symbols (mincore, __muloti4)~~ → Fixed: added to libmogrix_compat.so
2. ~~SOCK_SEQPACKET abort~~ → Fixed: socketpair compat
3. ~~WebKit helper process ENOENT~~ → Fixed: symlinks at /usr/sgug/libexec/webkit2gtk-4.0/
4. **PangoCairoFcFont class size mismatch** → ACTIVE BLOCKER
   - Error: "specified class size for type 'PangoCairoFcFont' is smaller than the parent type's 'PangoFcFont' class size"
   - Causes cascading GObject failures → no text rendering
   - MiniBrowser runs for 60s+, creates 620 X11 windows, but content invisible
   - Root cause: Pango and PangoCairo built against different PangoFcFont struct sizes
   - Fix: Rebuild pango-1.0 and/or cairo to ensure ABI compatibility

### NEXT STEPS

1. Fix PangoCairoFcFont class size mismatch (rebuild Pango/Cairo)
2. Dismiss stale "Critical System Error" dialog on IRIX
3. Test MiniBrowser after font rendering fix
4. Update bundler to handle WEBKIT_EXEC_PATH (instead of manual symlinks)
5. Rebuild WebKit with SOCK_SEQPACKET source patch (currently runtime compat)

---

## Key Technical Facts

- **fix_all_defineds condition**: `(st_other != 0 && st_other != 4) || (binding == LOCAL)` — GLOBAL DEFAULT skipped
- **STV_PROTECTED (st_other=3)**: Triggers fix_all_defineds processing without breaking external resolution
- **Named R_MIPS_REL32 formula**: IRIX gives `addend + displacement`; glibc gives `st_value + addend + displacement`. Fix: pre-add st_value.
- **find_reloc entry[0] skip**: Binary search can return 0 directly (no backward walk), but backward walk always skips [0]. Two-symbol approach handles both paths.
- **IRIX SOCK_SEQPACKET = 6** (Linux = 5), AF_UNIX doesn't support it
- **IRIX dlfcn.h lacks RTLD_NEXT** — can't wrap libc functions via dlsym
- **rld UND strictness**: Only checks UND symbols when displacement ≠ 0 (fix_all_defineds runs)
- **Deploy irix-ld**: `cp cross/bin/irix-ld /opt/sgug-staging/usr/sgug/bin/irix-ld`

---

## Reference

| What | Where |
|------|-------|
| R_MIPS_REL32 fix tool | `cross/bin/fix-anon-relocs` |
| socketpair compat | `compat/sys/socketpair.c` |
| libmogrix_compat.so sources | bsearch.c, socketpair.c, mincore.c, muloti4.c |
| rld decompiled | `docs/rld/rld_full_decompile.c`, `docs/rld/rld_reloc_callers.c` |
| WebKit bundle on IRIX | `/usr/people/edodd/apps/libwebkit2gtk-2.42.5-1-irix-bundle.0219262022/` |
| MiniBrowser error output | `output.txt` in bundle dir |
| Stale symlinks (manual) | `/usr/sgug/libexec/webkit2gtk-4.0/WebKit{Web,Network}Process` |
| Generic rules summary | `rules/GENERIC_SUMMARY.md` |
| Problem lookup | `rules/INDEX.md` (grep, don't read whole file) |
| Compat functions | `compat/catalog.yaml` |
