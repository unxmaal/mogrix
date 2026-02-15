# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-15 (session 49)
**Status**: XScreenSaver GL hacks now fully reproducible via mogrix pipeline. 131 MIPS N32 binaries built from hand-written spec + rules. Engine enhanced with package-level `rewrite_paths` override.

---

## Critical Warnings

**NEVER install packages directly to /usr/sgug on the live IRIX system.** Always use `/opt/chroot` for testing.

**NEVER use raw SSH to IRIX.** Always use MCP tools (`irix_exec`, `irix_copy_to`, `irix_read_file`, `irix_par`).

**NEVER CHEAT.** Don't copy from SGUG-RSE. Don't fake Provides. Every FC40 package must be properly cross-compiled.

---

## Current State

### xscreensaver-gl-hacks: DONE and reproducible

Fully mogrix-managed build. Key files:
- **Spec**: `specs/packages/xscreensaver-gl-hacks.spec` (hand-written, builds only hacks/glx/)
- **Rules**: `rules/packages/xscreensaver-gl-hacks.yaml` (no add_source/add_patch — spec handles those directly)
- **Patches**: `patches/packages/xscreensaver-gl-hacks/` (gl_stub.c, glu_stub.c, xscreensaver-irix.patch)
- **RPM**: `~/rpmbuild/RPMS/mips/xscreensaver-gl-hacks-6.08-1.mips.rpm` (131 binaries)
- **SRPM**: `~/mogrix_inputs/SRPMS/xscreensaver-gl-hacks-6.08-1.src.rpm` (upstream URL 410 Gone; tarball extracted from Fedora SRPM)

Lessons captured in rules/INDEX.md. Key gotchas for hand-written specs:
- Don't use `add_source`/`add_patch` in YAML if the spec already has Source/Patch lines (causes duplicates)
- Use `rewrite_paths` override to neutralize generic path rewriting (new engine feature added this session)
- xscreensaver's configure hardcodes XInput2/Xft as required; must `sed 's/^exit \$CONF_STATUS$/exit 0/'`

### Doxygen: INCOMPLETE

Build started (`rules/packages/doxygen.yaml`), got to 59% compile, then failed on `spdlog/filesystem.hpp` not recognizing IRIX. User interrupted to work on xscreensaver. Needs:
- Fix `deps/filesystem/filesystem.hpp` IRIX detection
- Fix `deps/spdlog` `::fileno()` scope issue

### cmake ecosystem: DONE

cmake, json-c, brotli, libwebp, ninja-build all built and staged.

### meson: installed on build host

`~/.local/bin/meson` (1.10.1 via uv tool). Not a cross-compiled package.

---

## Next Steps

1. **Resume doxygen build** — fix spdlog/filesystem.hpp for IRIX
2. **Build gdb** — complex (6444-line spec, 40+ patches)
3. **Build #203 remaining** — fossil, mercurial
4. **Build #201 GTK3 stack** — image libs all ready
5. **Build #204 IPC/heavy** — dbus, dbus-glib, icu, cyrus-sasl
6. **Low priority**: `drop_buildrequires_if_unavailable` engine feature, ksh build system

### Batch Progress

| Batch | Status |
|-------|--------|
| #191–#200 | COMPLETED (see plan.md for details) |
| #201 GTK3 stack | PENDING |
| #202 Build tools | cmake+ninja DONE; meson on host; doxygen INCOMPLETE; gdb PENDING |
| #203 Dev tools pt2 | re2c/yasm/quilt DONE; fossil/mercurial PENDING |
| #204 IPC/heavy | PENDING |
| #205 GUI apps | PENDING |

### Blocked Packages

| Package | Reason |
|---------|--------|
| ksh | ~3400-line custom build system |
| htop | needs IRIX /proc backend |
| openjpeg2 | no SRPM available |

---

## Recent Work

### Session 49: xscreensaver-gl-hacks reproducible build

- Made xscreensaver GL hacks fully reproducible via mogrix: hand-written spec, upstream tarball, GL/GLU stubs, IRIX patch
- Fixed 5 issues during pipeline integration:
  1. Double patch application (add_patch + Patch0 in spec)
  2. Source duplication (add_source + Source1/2 in spec)
  3. Path mangling by generic `rewrite_paths` rule on sysroot paths
  4. Missing CC= in configure (found gcc instead of irix-cc)
  5. xscreensaver configure aborting on missing XInput2/Xft
- **Engine enhancement**: Added `rewrite_paths` support to `_apply_package_rules()` in `mogrix/rules/engine.py` — packages can now override generic path rewrites (identity mappings neutralize them)
- Started doxygen build (interrupted — see above)

### Session 48: `__rld_obj_head` fix + GL hacks deployment

- Fixed `__rld_obj_head` crash: `--export-dynamic-symbol=__rld_obj_head` in irix-ld
- Bundled libz.so (zlib-ng 1.3.0) to fix "libpng error: bad parameters to zlib"
- User confirmed GL hacks working on IRIX

---

## Reference

| What | Where |
|------|-------|
| Generic rules summary | `rules/GENERIC_SUMMARY.md` (read this first) |
| Problem lookup | `rules/INDEX.md` (grep, don't read whole file) |
| Agent orchestration | `rules/methods/task-tracking.md` |
| Package rules | `rules/packages/<name>.yaml` |
| Project architecture | `plan.md` |
| Build methods | `rules/methods/*.md` |
| Compat functions | `compat/catalog.yaml` |
| Workspace paths & workflow | `CLAUDE.md` |
