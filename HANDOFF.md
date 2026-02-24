# Mogrix Cross-Compilation Handoff

**Last Updated**: 2026-02-23 (session 119)
**Status**: ir8 browser BUILT AND RUNNING on IRIX. Homepage renders. Ready for real-world testing.

---

## Post-Compaction Checklist (READ THIS FIRST)

1. **Mogrix invocation**: `uv run mogrix <command>`
2. **Grep rules/INDEX.md** before attempting ANY fix
3. **Read GENERIC_SUMMARY.md** before starting any new package
4. **Paths**: `/opt/sgug-staging/` = Linux sysroot, `/opt/chroot` = IRIX test chroot (MCP tools)
5. **IRIX access**: Use MCP tools. NEVER raw SSH. `irix_copy_to` supports `host_path=true` + `owner="edodd"`.
6. **Large builds**: Use haiku sub-agents. NEVER background Bash.
7. **WebKit rebuild**: `uv run mogrix convert ~/mogrix_inputs/SRPMS/webkitgtk-2.42.5-1.src.rpm && uv run mogrix build ~/mogrix_outputs/SRPMS/webkitgtk-2.42.5-1.src-converted/webkitgtk-2.42.5-1.src.rpm --cross && uv run mogrix stage ~/mogrix_outputs/RPMS/libwebkit2gtk*.rpm && uv run mogrix bundle libwebkit2gtk` (fast with ccache)
8. **Bundle deploy**: `irix_copy_to` with `host_path=true, owner="edodd"` to `/usr/people/edodd/`. Extract with `sh <bundle>.run /usr/people/edodd/apps`, installs to `/usr/people/edodd/apps/bin/`.

---

## Current State

### ir8 Browser — RUNNING (session 119)

Bundle `ir8-1.0-1-irix-bundle.0223262343` deployed and running on IRIX. Homepage (`ir8-about:home`) renders with search bar and branding. `ir8 --version` outputs `ir8 1.0 (WebKitGTK 2.42.5)`.

**ir8 build pipeline** (from source):
```bash
# 1. Create tarball from patches/packages/ir8/
rm -rf /tmp/ir8-1.0 && mkdir -p /tmp/ir8-1.0
cp patches/packages/ir8/* /tmp/ir8-1.0/
tar czf ~/rpmbuild/SOURCES/ir8-1.0.tar.gz -C /tmp ir8-1.0

# 2. Build SRPM, convert, cross-compile
rpmbuild -bs specs/packages/ir8.spec
uv run mogrix convert ~/rpmbuild/SRPMS/ir8-1.0-1.src.rpm
uv run mogrix build ~/mogrix_outputs/SRPMS/ir8-1.0-1.src-converted/ir8-1.0-1.src.rpm --cross

# 3. Stage and bundle (auto-includes WebKit deps)
uv run mogrix stage ~/mogrix_outputs/RPMS/ir8-1.0-1.mips.rpm
uv run mogrix bundle ir8 --include libwebkit2gtk --include libjavascriptcoregtk
```

**Key build fixes discovered**:
- Hand-written specs must `export CC="%{__cc}"` before `%make_build` (rpmmacros.irix `%make_build` doesn't export CC unlike `%configure`)
- GLib 2.80 n32 ABI: Must add `-DGLIB_VERSION_MIN_REQUIRED=G_ENCODE_VERSION(2,78) -DGLIB_VERSION_MAX_ALLOWED=G_ENCODE_VERSION(2,78)` to CFLAGS. Affects ALL GTK/GLib apps on n32.
- Both fixes documented in INDEX.md Platform Invariants table.

### ir8 Source Files

All in `patches/packages/ir8/` (19 files listed in `rules/packages/ir8.yaml`):
- `main.c` — entry point, homepage/about URI handler, IRIX defaults
- `ir8-window.c/h` — main browser window, toolbar, tabs
- `ir8-tab.c/h` — individual browser tabs
- `ir8-searchbox.c/h` — search entry widget
- `ir8-downloads.c/h` — download bar
- `ir8-settings.c/h` — settings dialog
- `ir8-cellrenderer.c/h` — custom cell renderer
- `ir8-bookmarks.c/h` — flat-file bookmark system (~/.config/ir8/bookmarks.txt)
- `ir8-marshal.c/h` — pre-generated GLib marshalling (from ir8-marshal.list)
- `Makefile` — build with pkg-config webkit2gtk-4.0

### WebKit HTTPS — WORKING (session 118)

Bundle `0223262218` renders HTTPS pages on IRIX. Verified with `https://example.com/`.

### WebKit HTTP — WORKING (session 116)

All 5 silent blockers bypassed.

---

## Next Steps

1. **Test ir8 with real websites** — HTTP and HTTPS pages, verify navigation, tabs, bookmarks
2. **Fix any runtime issues** — check ir8 stability on IRIX with extended use
3. **User's security stripping request** — strip unnecessary WebKit security features for IRIX (cookie sandbox, etc.)
4. **Consider surf browser** — simpler WebKit browser as alternative

---

## Recent Work (Sessions 117-119)

### Session 119
- Built ir8 browser from scratch: transformed MiniBrowser sources → ir8 branding
- Created 19 source files in patches/packages/ir8/
- Created hand-written spec (specs/packages/ir8.spec), rules (rules/packages/ir8.yaml), Makefile
- Fixed multiple build issues: glib-genmarshal (pre-generated), pkg-config paths, GLib 2.80 n32 ABI, CC export
- Successfully built, bundled (104.7 MB), deployed, and verified running on IRIX
- Added GLib n32 and CC export findings to rules/INDEX.md

### Session 118
- HTTPS confirmed working on IRIX
- Fixed bundle pruning bug for GIO module deps

### Session 117
- Added MOGRIX_DIAG env var gate, TLS DIAG tags
- WEBKIT_TLS_CAFILE_PEM bypass and export

---

## Key Files

- **ir8 sources**: `patches/packages/ir8/` (19 files)
- **ir8 rules**: `rules/packages/ir8.yaml`
- **ir8 spec**: `specs/packages/ir8.spec`
- **WebKit rules**: `rules/packages/webkitgtk.yaml` (all bypasses + DIAG tags)
- **DIAG header**: `patches/packages/webkitgtk/webkit_subprocess_diag.h`
- **Bundle wrapper**: `mogrix/bundle.py`
- **Latest ir8 bundle**: `ir8-1.0-1-irix-bundle.0223262343` at `/usr/people/edodd/apps/`
- **HTTPS plan**: `.claude/plans/misty-beaming-patterson.md` (COMPLETE)
