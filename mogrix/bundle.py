"""Self-contained app bundle generator for IRIX.

Creates app bundles that can be extracted on IRIX alongside SGUG-RSE
without conflicts. Each bundle includes all mogrix-built shared library
dependencies and wrapper scripts that set LD_LIBRARYN32_PATH.

Installation model (like Flatpak exports/bin):

    ~/apps/                   # extract bundles here
      bin/                              # ONE directory in PATH (trampolines)
        nano                            # trampoline: resolves own dir, execs
                                        #   ../nano-7.2-6-irix-bundle/nano
        nroff                           # trampoline -> ../groff-.../nroff
      nano-7.2-6-irix-bundle/
        nano, rnano, tput, ...          # wrapper scripts (set LD_LIBRARYN32_PATH)
        install                         # creates trampolines in ../bin/
        uninstall                       # removes them
        _bin/                           # actual binaries
        _lib32/                         # shared libraries (pruned to NEEDED only)
        share/                          # data files (terminfo trimmed to common terms)

    User setup (once):  PATH=~/apps/bin:$PATH; export PATH
    Per bundle:         cd ~/apps/nano-7.2-6-irix-bundle && ./install

Bundle optimization:
    - _lib32/ pruned to only sonames transitively NEEDED by bundle binaries
    - share/doc/, share/man/, share/info/, share/licenses/ stripped
    - terminfo trimmed to ~30 common terminals (iris-ansi, xterm, vt100, etc.)
    - Staging fallback libs use symlinks to avoid duplicating large files
    - Trampolines use relative paths (no baked-in absolute paths)
"""

import os
import re
import shutil
import subprocess
import tempfile
from datetime import datetime
from dataclasses import dataclass, field
from pathlib import Path

from rich.console import Console
from rich.table import Table

console = Console()

def _default_staging_lib_dir() -> Path:
    """Derive staging lib32 dir from project root."""
    project_root = Path(__file__).parent.parent.resolve()
    return project_root / "staging" / "opt" / "mogrix" / "lib32"

STAGING_LIB_DIR = _default_staging_lib_dir()

# Libraries that must NEVER be bundled — they exist on native IRIX and apps
# must use the native versions.  IRIX X11 libs use IRIX-specific transport/auth;
# the X server only works with its own libs.
#
# Matched by EXACT soname — IRIX native libs are .so.1 or unversioned .so.
# Our sgug-built versions (.so.6) are NOT excluded and WILL be bundled.
# This lets apps like rxvt-unicode bundle our libX11.so.6 while decker
# (which dlopens IRIX native X11) correctly skips them.
#
# Only libs that actually exist on IRIX 6.5 are listed here.  Libs that DON'T
# exist natively (libXft, libXrender, libxcb, libXau, libXcursor, libXrandr,
# libXcomposite, libXdamage, libXfixes, libXinerama) MUST be bundled.
IRIX_NATIVE_SONAMES = {
    # IRIX native X11 — exact sonames from /opt/irix-sysroot/usr/lib32/
    "libX11.so.1",
    "libXext.so",
    "libXi.so",
    "libXpm.so",
    "libXt.so",
    "libXmu.so",
    "libICE.so",
    "libSM.so",
}

# Global registry: dlopen'd plugin directories and their env var overrides.
# When the bundler detects _lib32/<subdir>/... containing .so files, it checks
# this map.  If the subdir (or a parent) matches, it sets the env var in the
# wrapper so the library finds its plugins inside the bundle instead of
# at the hardcoded /opt/mogrix/lib32/... path.
#
# Format: { "subdir_path_relative_to_lib32": ("ENV_VAR", "value_template") }
# value_template uses {lib32} as placeholder for "$dir/_lib32".
PLUGIN_DIR_ENV_MAP = {
    "imlib2/loaders": ("IMLIB2_LOADER_PATH", "{lib32}/imlib2/loaders"),
    "imlib2/filters": ("IMLIB2_FILTER_PATH", "{lib32}/imlib2/filters"),
    "gdk-pixbuf-2.0": ("GDK_PIXBUF_MODULEDIR", "{lib32}/gdk-pixbuf-2.0/2.10.0/loaders"),
    "pango": ("PANGO_LIBDIR", "{lib32}"),
    "qt5/plugins": ("QT_PLUGIN_PATH", "{lib32}/qt5/plugins"),
}

# Wrapper templates use only Bourne shell syntax (no $(...), no ${var:+...})
# because IRIX /bin/sh is the original Bourne shell, not POSIX sh.
#
# {terminfo_block} is replaced with TERMINFO export if bundle has terminfo data,
# or empty string if not.
#
# IMPORTANT: Use /bin/dirname and /bin/pwd (absolute paths) because the bundle
# may install a trampoline for "dirname" or "pwd" into ~/apps/bin/. If that
# directory is in $PATH, a relative "dirname" call would resolve to the
# trampoline, which calls the wrapper, which calls "dirname" — infinite loop.
WRAPPER_TEMPLATE = """\
#!/bin/sh
dir=`/bin/dirname "$0"`
case "$dir" in
    /*) ;;
    *)  dir="`/bin/pwd`/$dir" ;;
esac
LD_LIBRARYN32_PATH="$dir/_lib32{extra_lib_paths}:/usr/lib32"
export LD_LIBRARYN32_PATH
{rld_list_block}{terminfo_block}{extra_env_block}exec "$dir/_bin/{binary}" {extra_args}"$@"
"""

SBIN_WRAPPER_TEMPLATE = """\
#!/bin/sh
dir=`/bin/dirname "$0"`
case "$dir" in
    /*) ;;
    *)  dir="`/bin/pwd`/$dir" ;;
esac
LD_LIBRARYN32_PATH="$dir/_lib32{extra_lib_paths}:/usr/lib32"
export LD_LIBRARYN32_PATH
{rld_list_block}{terminfo_block}{extra_env_block}exec "$dir/_sbin/{binary}" "$@"
"""

# Wrapper for libexec binaries (WebKitGTK MiniBrowser, etc.).
# The binary stays in libexec/<subpath>; the wrapper is at the bundle root.
# Uses the binary's basename as the wrapper name.
LIBEXEC_WRAPPER_TEMPLATE = """\
#!/bin/sh
dir=`/bin/dirname "$0"`
case "$dir" in
    /*) ;;
    *)  dir="`/bin/pwd`/$dir" ;;
esac
LD_LIBRARYN32_PATH="$dir/_lib32{extra_lib_paths}:/usr/lib32"
export LD_LIBRARYN32_PATH
{rld_list_block}{terminfo_block}{extra_env_block}exec "$dir/{libexec_path}" "$@"
"""

# Block for _RLDN32_LIST preload of mogrix compat library.
# IRIX rld does NOT preempt shared library symbols from the executable
# (unlike Linux), so compat functions like bsearch must be in a .so that
# is preloaded via _RLDN32_LIST to override libc's buggy versions.
RLD_LIST_BLOCK = """\
_RLDN32_LIST=libmogrix_compat.so:DEFAULT
export _RLDN32_LIST
"""

TERMINFO_BLOCK = """\
TERMINFO="$dir/share/terminfo"
export TERMINFO
"""

# Binaries that must NEVER get trampolines in the shared bin/ directory.
# These would shadow critical IRIX system commands or are shell builtins
# that are useless as standalone binaries.
TRAMPOLINE_EXCLUDE_GLOBAL: set[str] = {
    # Would shadow /bin/sh — breaks IRIX scripts
    "sh",
    # Bash shell builtins — useless as standalone binaries
    "alias", "bg", "cd", "command", "fc", "fg", "getopts",
    "hash", "jobs", "read", "type", "ulimit", "umask",
    "unalias", "wait",
}

# Self-extracting installer: shell script header + appended tar.gz payload.
# Uses only commands available on base IRIX 6.5 (full paths to avoid PATH issues):
#   /bin/sh, /bin/tail, /usr/sbin/gzcat, /sbin/tar, /sbin/mkdir, /bin/pwd
# NOTE: IRIX native tar does NOT support -C flag. Must cd first, then extract.
# $0 is resolved to absolute path before cd so tail can still find the script.
SELF_EXTRACTING_TEMPLATE = """\
#!/bin/sh
# Mogrix self-extracting bundle: {display_name}
# Run:  sh {filename} [install_dir]
# Extract only:  sh {filename} --extract-only [dir]
# install_dir defaults to current directory
BUNDLE="{bundle_dir_name}"
SKIP={payload_line}
self="$0"
case "$self" in /*) ;; *) self="`/bin/pwd`/$self" ;; esac
dest="$1"
xonly=false
case "$1" in --extract-only) xonly=true; dest="$2" ;; esac
if [ -z "$dest" ]; then dest=`/bin/pwd`; fi
case "$dest" in /*) ;; *) dest="`/bin/pwd`/$dest" ;; esac
/sbin/mkdir -p "$dest" 2>/dev/null
if [ ! -d "$dest" ]; then echo "Error: cannot create $dest" >&2; exit 1; fi
echo "Installing $BUNDLE to $dest ..."
cd "$dest" && /bin/tail +$SKIP "$self" | /usr/sbin/gzcat | /sbin/tar xf -
if [ $? -ne 0 ]; then echo "Error: extraction failed" >&2; exit 1; fi
if [ "$xonly" = "true" ]; then echo "Extracted to $dest/$BUNDLE"; exit 0; fi
cd "$dest/$BUNDLE"
./install
echo ""
echo "Done. Add to PATH:"
echo "  PATH=$dest/bin:\\$PATH; export PATH"
exit 0
"""

# Install script: creates trampoline scripts in ../bin/ (Bourne shell compatible).
# Trampolines resolve their own location at runtime via dirname "$0" and use a
# relative path (../<bundle-name>/<cmd>) to reach the real wrapper. This works
# regardless of where the bundle is installed (chroot, different prefix, etc).
INSTALL_TEMPLATE = """\
#!/bin/sh
# Install {package} — create command trampolines in ../bin/
dir=`/bin/dirname "$0"`
case "$dir" in
    /*) ;;
    *)  dir="`/bin/pwd`/$dir" ;;
esac
dir=`cd "$dir" && /bin/pwd`
bundle=`/bin/basename "$dir"`
bindir=`/bin/dirname "$dir"`/bin
registry="$bindir/.bundle-owners"
mkdir -p "$bindir"
touch "$registry"
echo "Installing {package} commands into $bindir"
{trampoline_commands}
echo ""
echo "Done. Make sure $bindir is in your PATH:"
echo "  PATH=$bindir:\\$PATH; export PATH"
echo ""
echo "Add that line to ~/.profile to make it permanent."
"""

# Uninstall script: removes trampolines from ../bin/
UNINSTALL_TEMPLATE = """\
#!/bin/sh
# Uninstall {package} — remove command trampolines and bundle directory
dir=`/bin/dirname "$0"`
case "$dir" in
    /*) ;;
    *)  dir="`/bin/pwd`/$dir" ;;
esac
dir=`cd "$dir" && /bin/pwd`
bundle=`/bin/basename "$dir"`
bindir=`/bin/dirname "$dir"`/bin
registry="$bindir/.bundle-owners"
echo "Removing {package} commands from $bindir"
{unlink_commands}
# Clean registry entries for this bundle
if [ -f "$registry" ]; then
    tmp="$registry.tmp"
    grep -v "=$bundle\$" "$registry" > "$tmp" 2>/dev/null
    mv "$tmp" "$registry"
fi
echo ""
echo "To remove the bundle directory, run:"
echo "  rm -rf $dir"
"""


@dataclass
class BundleManifest:
    """Tracks what's in a bundle and where it came from."""

    target_package: str
    target_version: str
    included_rpms: list[Path] = field(default_factory=list)
    irix_sonames: set[str] = field(default_factory=set)
    staging_sonames: set[str] = field(default_factory=set)
    unresolved_sonames: set[str] = field(default_factory=set)
    binaries: list[str] = field(default_factory=list)
    target_rpms: set[Path] = field(default_factory=set)
    bundle_name: str = ""
    bundle_dir: Path | None = None
    tarball_path: Path | None = None
    run_path: Path | None = None
    suite_name: str | None = None
    suite_packages: list[str] = field(default_factory=list)


class BundleBuilder:
    """Builds self-contained app bundles for IRIX from mogrix RPMs."""

    def __init__(
        self,
        rpms_dir: Path,
        irix_sysroot: Path = Path("/opt/irix-sysroot"),
    ):
        self.rpms_dir = rpms_dir
        self.irix_sysroot = irix_sysroot

        # Maps built during initialization
        self._soname_to_rpm: dict[str, Path] = {}
        self._name_to_rpms: dict[str, list[Path]] = {}
        self._source_to_rpms: dict[str, list[Path]] = {}
        self._irix_sonames: set[str] = set()

        self._build_maps()

    def _build_maps(self) -> None:
        """Scan all RPMs and IRIX sysroot to build lookup maps."""
        rpm_files = sorted(self.rpms_dir.glob("*.rpm"))
        if not rpm_files:
            console.print(f"[red]No RPMs found in {self.rpms_dir}[/red]")
            return

        console.print(f"[dim]Scanning {len(rpm_files)} RPMs...[/dim]")

        for rpm_path in rpm_files:
            # Get package name
            name = self._rpm_query(rpm_path, "%{NAME}")
            if not name:
                continue
            self._name_to_rpms.setdefault(name, []).append(rpm_path)

            # Get SOURCERPM for sibling grouping
            source_rpm = self._rpm_query(rpm_path, "%{SOURCERPM}")
            if source_rpm:
                self._source_to_rpms.setdefault(source_rpm, []).append(rpm_path)

            # Get file list for soname mapping
            filelist = self._rpm_filelist(rpm_path)
            for filepath in filelist:
                if "/lib32/" in filepath and ".so" in filepath:
                    soname = os.path.basename(filepath)
                    # First RPM wins (prefer non-devel)
                    if soname not in self._soname_to_rpm:
                        self._soname_to_rpm[soname] = rpm_path

        # Scan IRIX sysroot for native sonames
        for sysroot_dir in [
            self.irix_sysroot / "usr" / "lib32",
            self.irix_sysroot / "lib32",
        ]:
            if sysroot_dir.is_dir():
                for entry in sysroot_dir.iterdir():
                    if ".so" in entry.name:
                        self._irix_sonames.add(entry.name)

        console.print(
            f"[dim]  {len(self._soname_to_rpm)} sonames, "
            f"{len(self._name_to_rpms)} packages, "
            f"{len(self._irix_sonames)} IRIX native libs[/dim]"
        )

    def _resolve_package_name(self, name: str) -> str | None:
        """Resolve a user-provided package name to an actual RPM name.

        Handles cases where the rule name differs from the RPM name:
          - 'vim' → 'vim-enhanced' (subpackage from same SRPM)
          - 'tree-pkg' → 'tree' (different RPM name)

        Resolution order:
          1. Exact match in _name_to_rpms
          2. Check rule YAML for 'rpm_name' field
          3. Find RPMs from same source (SOURCERPM starts with package name)
          4. Fuzzy match: any RPM name containing the package name
        """
        # 1. Exact match
        if name in self._name_to_rpms:
            return name

        # 2. Check rule YAML for rpm_name alias
        from pathlib import Path as _Path
        _rules_dir = _Path(__file__).parent.parent / "rules" / "packages"
        _rule_file = _rules_dir / f"{name}.yaml"
        if _rule_file.exists():
            import yaml as _yaml
            with open(_rule_file) as _f:
                _rule = _yaml.safe_load(_f) or {}
            rpm_name = _rule.get("rpm_name")
            if rpm_name and rpm_name in self._name_to_rpms:
                console.print(f"[dim]  {name} → {rpm_name} (from rule rpm_name)[/dim]")
                return rpm_name

        # 3. Find RPMs from same SRPM (e.g., vim → vim-enhanced via SOURCERPM)
        for source_rpm, rpms in self._source_to_rpms.items():
            # SOURCERPM format: "vim-9.1.158-1.src.rpm"
            # Extract the base package name from SOURCERPM
            src_base = source_rpm.rsplit("-", 2)[0] if source_rpm.count("-") >= 2 else source_rpm
            if src_base == name:
                # Found RPMs from this source — pick the best one
                # Prefer: exact name > name-enhanced > first non-devel non-lib
                rpm_names = [self._rpm_query(r, "%{NAME}") for r in rpms]
                for candidate in rpm_names:
                    if candidate == name:
                        return candidate
                # Try enhanced/minimal variants
                for candidate in rpm_names:
                    if candidate.startswith(name + "-") and not candidate.endswith(("-devel", "-static", "-doc", "-libs")):
                        console.print(f"[dim]  {name} → {candidate} (from SRPM {source_rpm})[/dim]")
                        return candidate
                # Take first non-devel
                for candidate in rpm_names:
                    if not candidate.endswith(("-devel", "-static", "-doc")):
                        console.print(f"[dim]  {name} → {candidate} (from SRPM {source_rpm})[/dim]")
                        return candidate

        # 4. Fuzzy: any RPM whose name starts with the package name
        for rpm_name in self._name_to_rpms:
            if rpm_name.startswith(name + "-") and not rpm_name.endswith(("-devel", "-static", "-doc", "-libs")):
                console.print(f"[dim]  {name} → {rpm_name} (fuzzy prefix match)[/dim]")
                return rpm_name

        return None

    def _rpm_query(self, rpm_path: Path, fmt: str) -> str:
        """Query RPM metadata field."""
        result = subprocess.run(
            ["rpm", "-qp", "--queryformat", fmt, str(rpm_path)],
            capture_output=True,
            text=True,
        )
        return result.stdout.strip() if result.returncode == 0 else ""

    def _rpm_filelist(self, rpm_path: Path) -> list[str]:
        """Get file list from RPM."""
        result = subprocess.run(
            ["rpm", "-qpl", str(rpm_path)],
            capture_output=True,
            text=True,
        )
        if result.returncode != 0:
            return []
        return [line.strip() for line in result.stdout.splitlines() if line.strip()]

    def _is_elf(self, filepath: Path) -> bool:
        """Check if file is an ELF binary."""
        try:
            with open(filepath, "rb") as f:
                magic = f.read(4)
            return magic == b"\x7fELF"
        except (OSError, PermissionError):
            return False

    def _readelf_needed(self, elf_path: Path) -> list[str]:
        """Get NEEDED sonames from an ELF binary using readelf."""
        result = subprocess.run(
            ["readelf", "-d", str(elf_path)],
            capture_output=True,
            text=True,
        )
        if result.returncode != 0:
            return []

        needed = []
        for line in result.stdout.splitlines():
            # Match: 0x00000001 (NEEDED)  Shared library: [libfoo.so]
            match = re.search(r"\(NEEDED\)\s+Shared library:\s+\[(.+?)\]", line)
            if match:
                needed.append(match.group(1))
        return needed

    def _get_sibling_rpms(self, rpm_path: Path) -> list[Path]:
        """Find RPMs built from the same source, excluding -devel/-debuginfo."""
        source_rpm = self._rpm_query(rpm_path, "%{SOURCERPM}")
        if not source_rpm or source_rpm not in self._source_to_rpms:
            return [rpm_path]

        siblings = []
        for sibling in self._source_to_rpms[source_rpm]:
            name = self._rpm_query(sibling, "%{NAME}")
            if not name:
                continue
            # Skip devel and debug subpackages
            if any(
                suffix in name
                for suffix in ["-devel", "-debuginfo", "-debugsource", "-static"]
            ):
                continue
            siblings.append(sibling)
        return siblings

    def _extract_rpm(self, rpm_path: Path, dest_dir: Path) -> bool:
        """Extract an RPM to a destination directory."""
        result = subprocess.run(
            f"cd {dest_dir} && rpm2cpio {rpm_path.absolute()} | cpio -idm 2>/dev/null",
            shell=True,
            capture_output=True,
            text=True,
        )
        return result.returncode == 0

    def _scan_elf_needed(self, extract_dir: Path) -> set[str]:
        """Find all NEEDED sonames from ELF files in an extracted RPM."""
        needed = set()
        for root, _dirs, files in os.walk(extract_dir):
            for filename in files:
                filepath = Path(root) / filename
                if filepath.is_symlink():
                    continue
                if self._is_elf(filepath):
                    for soname in self._readelf_needed(filepath):
                        needed.add(soname)
        return needed

    def _create_soname_symlinks(self, bundle_dir: Path) -> None:
        """Create missing soname symlinks in _lib32/.

        Some ELF libraries have unversioned SONAMEs (e.g. libz.so) but the
        runtime RPM only ships versioned files (libz.so.1, libz.so.1.3.0).
        The unversioned symlink lives in the -devel RPM which is excluded
        from bundles.  Detect these gaps and create symlinks so rld can
        resolve the NEEDED entries at runtime.
        """
        lib_dir = bundle_dir / "_lib32"
        if not lib_dir.is_dir():
            return

        # Collect all NEEDED sonames from all ELFs in the bundle
        needed_sonames: set[str] = set()
        for subdir in ("_bin", "_sbin", "_lib32"):
            d = bundle_dir / subdir
            if not d.is_dir():
                continue
            for f in d.iterdir():
                target = f.resolve() if f.is_symlink() else f
                if target.exists() and self._is_elf(target):
                    needed_sonames.update(self._readelf_needed(target))

        # For each needed soname missing from _lib32/, try to find a
        # versioned file that provides it (via ELF SONAME header or prefix).
        created = []
        for soname in sorted(needed_sonames):
            soname_path = lib_dir / soname
            if soname_path.exists():
                continue
            # Look for a file whose ELF SONAME matches
            prefix = soname.split(".so")[0]
            best_fallback: str | None = None
            for candidate in lib_dir.iterdir():
                # Prefix must match exactly up to ".so" or "." boundary
                # to avoid e.g. "libm" matching "libmogrix_compat.so"
                cname = candidate.name
                if not cname.startswith(prefix):
                    continue
                rest = cname[len(prefix):]
                if rest and not rest.startswith("."):
                    continue
                target = candidate.resolve() if candidate.is_symlink() else candidate
                if not target.exists() or not self._is_elf(target):
                    continue
                # Check if its SONAME matches what we need
                result = subprocess.run(
                    ["readelf", "-d", str(target)],
                    capture_output=True, text=True,
                )
                has_soname = False
                for line in result.stdout.splitlines():
                    if "(SONAME)" in line:
                        has_soname = True
                        if f"[{soname}]" in line:
                            # Exact SONAME match
                            soname_path.symlink_to(candidate.name)
                            created.append(f"{soname} -> {candidate.name}")
                            break
                if soname_path.exists():
                    break
                # Track shortest-named candidate with no SONAME as fallback
                # (e.g. zlib-ng has no SONAME; libz.so.1 is the best target)
                if not has_soname and (
                    best_fallback is None
                    or len(candidate.name) < len(best_fallback)
                ):
                    best_fallback = candidate.name
            # Fallback: library has no SONAME (e.g. zlib-ng), link to
            # shortest versioned file with matching prefix
            if not soname_path.exists() and best_fallback:
                soname_path.symlink_to(best_fallback)
                created.append(f"{soname} -> {best_fallback} (no-SONAME fallback)")

        if created:
            console.print(
                f"  [dim]Created {len(created)} soname symlinks:[/dim]"
            )
            for s in created:
                console.print(f"    [dim]{s}[/dim]")

    def _prune_unused_libs(self, bundle_dir: Path) -> None:
        """Remove .so files from _lib32/ not NEEDED by any binary in the bundle.

        RPM extraction pulls in all libs from a package (e.g., ncurses-libs has
        14 .so files) but the bundle may only need a few. Scan all ELFs in _bin/
        and _sbin/ to find actual NEEDED sonames, then remove everything else.
        """
        lib_dir = bundle_dir / "_lib32"
        if not lib_dir.is_dir():
            return

        # Collect all NEEDED sonames from binaries.
        # Always keep _RLDN32_LIST libraries (preloaded, not NEEDED deps).
        needed = {"libmogrix_compat.so", "irix_rld_stubs.so"}
        for bin_subdir in ("_bin", "_sbin"):
            d = bundle_dir / bin_subdir
            if not d.is_dir():
                continue
            for f in d.iterdir():
                if self._is_elf(f) or (f.is_symlink() and self._is_elf(f.resolve())):
                    needed.update(self._readelf_needed(f))

        # Also scan libexec/ recursively — packages like WebKitGTK put
        # their main binaries there (MiniBrowser, WebKitWebProcess, etc.)
        libexec_dir = bundle_dir / "libexec"
        if libexec_dir.is_dir():
            for f in libexec_dir.rglob("*"):
                if f.is_file() and (self._is_elf(f) or (f.is_symlink() and self._is_elf(f.resolve()))):
                    needed.update(self._readelf_needed(f))

        # Also check libs themselves — they may depend on other libs
        # (transitive closure).  Walk symlink chains step-by-step so
        # intermediate links (e.g. libz.so → libz.so.1 → libz.so.1.3.0.zlib-ng)
        # are all added to the kept set.
        checked = set()
        queue = list(needed)
        while queue:
            soname = queue.pop()
            if soname in checked:
                continue
            checked.add(soname)
            lib_file = lib_dir / soname
            # Walk symlink chain one hop at a time, adding each name
            while lib_file.is_symlink():
                link_target = os.readlink(str(lib_file))
                checked.add(link_target)
                lib_file = lib_dir / link_target
            # Now lib_file is the real file — scan its NEEDEDs
            if lib_file.exists() and self._is_elf(lib_file):
                checked.add(lib_file.name)
                for dep in self._readelf_needed(lib_file):
                    if dep not in checked:
                        queue.append(dep)

        needed = checked

        # Protect dlopen'd modules from pruning (GIO modules for TLS/proxy)
        gio_modules_dir = lib_dir / "gio" / "modules"
        if gio_modules_dir.is_dir():
            for f in gio_modules_dir.iterdir():
                if f.is_file() and ".so" in f.name:
                    # Add GIO module deps to the needed set
                    needed.add(f.name)
                    if self._is_elf(f):
                        for dep in self._readelf_needed(f):
                            if dep not in needed:
                                needed.add(dep)
                                # Resolve symlinks so real file isn't pruned
                                dep_path = lib_dir / dep
                                if dep_path.is_symlink():
                                    needed.add(os.readlink(str(dep_path)))
                                # Also protect deps-of-deps (e.g., libgnutls → libnettle)
                                real_dep = dep_path.resolve() if dep_path.exists() else dep_path
                                if real_dep.exists() and self._is_elf(real_dep):
                                    for dep2 in self._readelf_needed(real_dep):
                                        if dep2 not in needed:
                                            needed.add(dep2)
                                            dep2_path = lib_dir / dep2
                                            if dep2_path.is_symlink():
                                                needed.add(os.readlink(str(dep2_path)))

        # Remove .so files not in the needed set, and always remove
        # native IRIX system libs that must not be bundled (X11 etc.)
        removed = []
        for f in sorted(lib_dir.iterdir()):
            if ".so" not in f.name:
                continue
            # Always remove never-bundle libs (even if in needed set)
            if f.name in IRIX_NATIVE_SONAMES:
                f.unlink()
                removed.append(f.name)
                continue
            if f.name in needed:
                continue
            f.unlink()
            removed.append(f.name)

        if removed:
            console.print(
                f"  [dim]Pruned {len(removed)} unused libs from _lib32/[/dim]"
            )

    def _strip_rpaths(self, bundle_dir: Path) -> None:
        """Strip DT_RPATH/DT_RUNPATH from all shared libraries in _lib32/.

        Build-time RPATHs (e.g. /home/user/rpmbuild/BUILD/...) leak into
        shared libraries and cause IRIX rld to search wrong directories for
        transitive dependencies, bypassing LD_LIBRARYN32_PATH.  Since
        bundle wrapper scripts set LD_LIBRARYN32_PATH to _lib32/, RPATHs
        are unnecessary and harmful.

        Method: parse the ELF .dynamic section and change DT_RPATH (15) /
        DT_RUNPATH (29) tags to DT_DEBUG (21), which rld ignores.
        Cannot use DT_NULL (0) because that terminates .dynamic scanning.
        Works for both big-endian (MIPS) and little-endian ELFs.
        """
        import struct

        DT_NULL = 0
        DT_DEBUG = 21
        DT_RPATH = 15
        DT_RUNPATH = 29

        lib_dir = bundle_dir / "_lib32"
        if not lib_dir.is_dir():
            return

        stripped_count = 0
        for f in sorted(lib_dir.iterdir()):
            if not f.is_file() or f.is_symlink():
                continue
            if ".so" not in f.name:
                continue

            try:
                data = bytearray(f.read_bytes())
            except OSError:
                continue

            # Quick ELF magic check
            if data[:4] != b"\x7fELF":
                continue

            ei_class = data[4]  # 1=32-bit, 2=64-bit
            ei_data = data[5]  # 1=LE, 2=BE
            if ei_class == 1:
                endian = ">" if ei_data == 2 else "<"
                e_phoff = struct.unpack_from(endian + "I", data, 28)[0]
                e_phentsize = struct.unpack_from(endian + "H", data, 42)[0]
                e_phnum = struct.unpack_from(endian + "H", data, 44)[0]
                ptr_fmt = endian + "iI"  # d_tag (signed), d_val (unsigned)
                entry_size = 8
            elif ei_class == 2:
                endian = ">" if ei_data == 2 else "<"
                e_phoff = struct.unpack_from(endian + "Q", data, 32)[0]
                e_phentsize = struct.unpack_from(endian + "H", data, 54)[0]
                e_phnum = struct.unpack_from(endian + "H", data, 56)[0]
                ptr_fmt = endian + "qQ"
                entry_size = 16
            else:
                continue

            # Find PT_DYNAMIC program header (type 2)
            dyn_offset = 0
            dyn_size = 0
            PT_DYNAMIC = 2
            for i in range(e_phnum):
                ph_start = e_phoff + i * e_phentsize
                p_type = struct.unpack_from(endian + "I", data, ph_start)[0]
                if p_type == PT_DYNAMIC:
                    if ei_class == 1:
                        dyn_offset = struct.unpack_from(endian + "I", data, ph_start + 4)[0]
                        dyn_size = struct.unpack_from(endian + "I", data, ph_start + 16)[0]
                    else:
                        dyn_offset = struct.unpack_from(endian + "Q", data, ph_start + 8)[0]
                        dyn_size = struct.unpack_from(endian + "Q", data, ph_start + 32)[0]
                    break

            if dyn_offset == 0:
                continue

            # Scan .dynamic entries, zero out DT_RPATH and DT_RUNPATH
            modified = False
            pos = dyn_offset
            while pos + entry_size <= dyn_offset + dyn_size:
                d_tag = struct.unpack_from(ptr_fmt, data, pos)[0]
                if d_tag == DT_NULL:
                    break
                if d_tag in (DT_RPATH, DT_RUNPATH):
                    struct.pack_into(ptr_fmt, data, pos, DT_DEBUG, 0)
                    modified = True
                pos += entry_size

            if modified:
                # Ensure file is writable (RPM-extracted files may be read-only)
                import stat
                f.chmod(f.stat().st_mode | stat.S_IWUSR)
                f.write_bytes(data)
                stripped_count += 1

        if stripped_count:
            console.print(
                f"  [dim]Stripped RPATH from {stripped_count} libs in _lib32/[/dim]"
            )

    def _rebase_libraries(self, bundle_dir: Path) -> None:
        """Rebase libstdc++ to a unique base address for C++ RTTI pre-resolution.

        IRIX rld crashes when 2+ libraries have displacement=0 (all loaded at
        their preferred address). So we rebase ONLY libstdc++ — the library
        containing C++ typeinfo symbols (__cxxabiv1::__si_class_type_info etc.)
        that fix-anon-relocs --pre-resolve-only needs correct addresses for.

        Other libraries stay at 0x0f800000 and rld displaces them normally.
        This is safe because rld handles displacement correctly for all cases
        EXCEPT the R_MIPS_REL32 UND symbol pre-resolution in executables,
        which only needs libstdc++'s address to be known at bundle time.

        """
        lib_dir = bundle_dir / "_lib32"
        if not lib_dir.is_dir():
            return

        mrqs = Path(__file__).parent.parent / "cross" / "bin" / "mrqs"
        if not mrqs.exists():
            return

        # Find libstdc++ (the only library we need to rebase)
        target_lib = None
        for f in lib_dir.iterdir():
            if f.is_file() and not f.is_symlink() and f.name.startswith("libstdc++"):
                target_lib = f
                break

        if target_lib is None:
            return

        # Skip rebasing the SGUG GCC-built libstdc++ — mrqs corrupts it.
        # The SGUG library has a different section layout than clang-built
        # versions, and mrqs patches wrong offsets, causing SIGSEGV during
        # static init.  With few bundled libs, rld handles displacement fine.
        SGUG_LIBSTDCXX_SIZE = 15886858  # libstdc++.so.6.0.27 from SGUG GCC 9
        if target_lib.stat().st_size == SGUG_LIBSTDCXX_SIZE:
            console.print("  [dim]mrqs: skipping SGUG libstdc++ rebase (known incompatible)[/dim]")
            return

        # Rebase just libstdc++ in an isolated temp dir
        import tempfile
        with tempfile.TemporaryDirectory() as tmp:
            tmp_path = Path(tmp)
            import shutil as _shutil
            _shutil.copy2(str(target_lib), str(tmp_path / target_lib.name))

            result = subprocess.run(
                ["python3", str(mrqs), str(tmp_path)],
                capture_output=True,
                text=True,
            )

            if result.returncode != 0:
                console.print(f"[yellow]mrqs failed for libstdc++: {result.stderr[:200]}[/yellow]")
                return

            # Copy rebased libstdc++ back
            _shutil.copy2(str(tmp_path / target_lib.name), str(target_lib))

        for line in (result.stdout.strip().split('\n') if result.stdout.strip() else []):
            if 'rebased' in line or 'mrqs:' in line:
                console.print(f"  [dim]{line.strip()}[/dim]")
                break

    def _pre_resolve_executables(self, bundle_dir: Path) -> None:
        """Pre-resolve UND symbol relocations in executables using rebased libraries.

        After mrqs rebases _lib32/, the library symbol addresses are final.
        This runs fix-anon-relocs --pre-resolve-only on each executable to
        resolve R_MIPS_REL32 entries for UND symbols (e.g. C++ typeinfo).
        """
        lib_dir = bundle_dir / "_lib32"
        if not lib_dir.is_dir():
            return

        fix_anon = Path(__file__).parent.parent / "cross" / "bin" / "fix-anon-relocs"
        if not fix_anon.exists():
            return

        exe_dirs = []
        for subdir in ("_bin", "_sbin"):
            d = bundle_dir / subdir
            if d.is_dir():
                exe_dirs.append(d)
        # Also check libexec/ for executables (WebKitWebProcess etc.)
        libexec_dir = bundle_dir / "libexec"
        if libexec_dir.is_dir():
            exe_dirs.append(libexec_dir)

        resolved_total = 0
        exe_count = 0

        for d in exe_dirs:
            for f in (d.rglob("*") if d.name == "libexec" else d.iterdir()):
                if not f.is_file() or f.is_symlink():
                    continue
                if not self._is_elf(f):
                    continue

                result = subprocess.run(
                    [
                        "python3", str(fix_anon),
                        "--pre-resolve-only",
                        "--lib-path", str(lib_dir),
                        str(f), str(f),
                    ],
                    capture_output=True,
                    text=True,
                )
                if result.returncode == 0:
                    # Check if any relocations were resolved
                    for line in result.stdout.split('\n'):
                        if "Pre-resolved" in line and "0 R_MIPS_REL32" not in line:
                            resolved_total += 1
                            break
                    exe_count += 1

        if resolved_total > 0:
            console.print(
                f"  [dim]Pre-resolved UND relocations in {resolved_total}/{exe_count} "
                f"executables[/dim]"
            )

    # Common terminal types to keep in trimmed terminfo.
    # Covers: SGI IRIX terminals, xterm variants, VT series, screen/tmux,
    # Linux console, and common remote terminals.
    TERMINFO_KEEP = {
        "iris-ansi", "iris-ansi-ap", "iris-ansi-net",
        "xterm", "xterm-color", "xterm-256color", "xterm-16color",
        "vt100", "vt100-am", "vt102", "vt220", "vt320", "vt52",
        "screen", "screen-256color", "screen.xterm-256color",
        "tmux", "tmux-256color",
        "linux", "dumb", "ansi",
        "rxvt", "rxvt-unicode", "rxvt-unicode-256color",
        "putty", "putty-256color",
        "dtterm", "sun",
    }

    def _include_ca_bundle(self, bundle_dir: Path) -> None:
        """Include system CA certificates for TLS verification.

        Copies the build host's CA bundle into the bundle so gnutls/openssl
        can verify server certificates on IRIX (which has no CA store).
        """
        ca_sources = [
            Path("/etc/ssl/certs/ca-certificates.crt"),  # Debian/Ubuntu
            Path("/etc/pki/tls/certs/ca-bundle.crt"),  # RHEL/Fedora
        ]
        for src in ca_sources:
            if src.is_file():
                dest = bundle_dir / "etc" / "pki" / "tls" / "certs" / "ca-bundle.crt"
                dest.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(src, dest)
                break

    def _include_gio_modules(self, bundle_dir: Path) -> None:
        """Include GIO extension modules for TLS/proxy support.

        GIO modules (libgiognutls.so, libgioenvironmentproxy.so) are dlopen'd
        at runtime by GLib — they're never in any binary's NEEDED chain.
        If the bundle uses GLib (libgio-2.0.so.0), copy modules from staging
        so GLib can find them via GIO_MODULE_DIR.

        Also copies transitive dependencies (libgnutls, libtasn1, libnettle,
        libhogweed, libgmp) and generates a giomodule.cache file.
        """
        lib_dir = bundle_dir / "_lib32"
        if not lib_dir.is_dir():
            return

        # Only include GIO modules if the bundle uses GLib
        has_gio = (lib_dir / "libgio-2.0.so.0").exists() or any(
            f.name.startswith("libgio-2.0.so") for f in lib_dir.iterdir()
            if f.is_file() or f.is_symlink()
        )
        if not has_gio:
            return

        staging_gio = STAGING_LIB_DIR / "gio" / "modules"
        if not staging_gio.is_dir():
            return

        gio_dest = lib_dir / "gio" / "modules"
        gio_dest.mkdir(parents=True, exist_ok=True)

        # Copy GIO module .so files from staging
        modules_found = []
        for f in staging_gio.iterdir():
            if f.is_file() and f.name.endswith(".so"):
                dest = gio_dest / f.name
                if not dest.exists():
                    shutil.copy2(str(f), str(dest))
                modules_found.append(f.name)

        if not modules_found:
            return

        # Copy transitive deps of GIO modules into _lib32/ if missing.
        # libgiognutls.so -> libgnutls -> libtasn1, libnettle, libhogweed, libgmp
        for mod_file in gio_dest.iterdir():
            if mod_file.is_file() and self._is_elf(mod_file):
                for dep in self._readelf_needed(mod_file):
                    dep_dest = lib_dir / dep
                    if not dep_dest.exists():
                        dep_src = STAGING_LIB_DIR / dep
                        if dep_src.exists():
                            real_file = dep_src.resolve()
                            shutil.copy2(str(real_file), str(lib_dir / real_file.name))
                            if real_file.name != dep:
                                (lib_dir / dep).symlink_to(real_file.name)
                            # Also copy deps of deps (one level deeper)
                            if self._is_elf(real_file):
                                for dep2 in self._readelf_needed(real_file):
                                    if not (lib_dir / dep2).exists():
                                        dep2_src = STAGING_LIB_DIR / dep2
                                        if dep2_src.exists():
                                            real2 = dep2_src.resolve()
                                            shutil.copy2(str(real2), str(lib_dir / real2.name))
                                            if real2.name != dep2:
                                                (lib_dir / dep2).symlink_to(real2.name)

        # Generate giomodule.cache (gio-querymodules can't run cross-compiled).
        # Format: "filename: extension-point[,extension-point]"
        cache_lines = []
        module_registry = {
            "libgiognutls.so": "gio-tls-backend",
            "libgioenvironmentproxy.so": "gio-proxy-resolver",
            "libgiognomeproxy.so": "gio-proxy-resolver",
            "libgiofam.so": "gio-local-file-monitor,gio-nfs-file-monitor",
            "libgiolibproxy.so": "gio-proxy-resolver",
            "libgvfsdbus.so": "gio-vfs",
        }
        for mod_name in sorted(modules_found):
            ext_point = module_registry.get(mod_name)
            if ext_point:
                cache_lines.append(f"{mod_name}: {ext_point}")
        if cache_lines:
            cache_file = gio_dest / "giomodule.cache"
            cache_file.write_text("\n".join(cache_lines) + "\n")

        console.print(
            f"  [dim]Included GIO modules: {', '.join(modules_found)}[/dim]"
        )

    def _include_fonts(self, bundle_dir: Path) -> bool:
        """Include TTF fonts for X11 bundles that use Xft/fontconfig.

        Copies TTF files from the mogrix fonts/ directory into the bundle,
        adds a relative <dir> to fonts.conf so fontconfig discovers them,
        and adds a conf.d snippet mapping 'monospace' to the bundled font.

        Returns True if fonts were included, False otherwise.
        """
        # Find mogrix fonts/ directory (sibling of mogrix/ package dir)
        mogrix_root = Path(__file__).resolve().parent.parent
        fonts_src = mogrix_root / "fonts"
        if not fonts_src.is_dir():
            return False

        ttf_files = list(fonts_src.glob("*.ttf"))
        if not ttf_files:
            return False

        # Only include fonts if bundle has fontconfig (etc/fonts/fonts.conf)
        fonts_conf = bundle_dir / "etc" / "fonts" / "fonts.conf"
        if not fonts_conf.is_file():
            return False

        # Copy TTF files to share/fonts/
        fonts_dest = bundle_dir / "share" / "fonts"
        fonts_dest.mkdir(parents=True, exist_ok=True)
        for ttf in ttf_files:
            shutil.copy2(str(ttf), str(fonts_dest / ttf.name))

        # Add relative <dir> to fonts.conf so fontconfig finds bundle fonts.
        # prefix="relative" makes the path relative to the fonts.conf file
        # location (etc/fonts/), so ../../share/fonts -> bundle/share/fonts.
        conf_text = fonts_conf.read_text()
        if "../../share/fonts" not in conf_text:
            conf_text = conf_text.replace(
                "<!-- Font directory list -->",
                '<!-- Font directory list -->\n\n\t'
                '<dir prefix="relative">../../share/fonts</dir>',
            )
            fonts_conf.write_text(conf_text)

        # Add cachedir relative to bundle (so fc-cache doesn't need /opt/mogrix)
        if "../../var/cache/fontconfig" not in conf_text:
            conf_text = fonts_conf.read_text()
            conf_text = conf_text.replace(
                "<!-- Font cache directory list -->",
                '<!-- Font cache directory list -->\n\n\t'
                '<cachedir prefix="relative">../../var/cache/fontconfig</cachedir>',
            )
            fonts_conf.write_text(conf_text)

        # Create conf.d/50-monospace.conf mapping 'monospace' to bundled font.
        # Detect font family name from filename pattern.
        conf_d = bundle_dir / "etc" / "fonts" / "conf.d"
        conf_d.mkdir(parents=True, exist_ok=True)
        monospace_conf = conf_d / "50-monospace.conf"
        monospace_conf.write_text(
            '<?xml version="1.0"?>\n'
            '<!DOCTYPE fontconfig SYSTEM "urn:fontconfig:fonts.dtd">\n'
            '<fontconfig>\n'
            '  <alias>\n'
            '    <family>monospace</family>\n'
            '    <prefer>\n'
            '      <family>Iosevka Nerd Font</family>\n'
            '    </prefer>\n'
            '  </alias>\n'
            '</fontconfig>\n'
        )

        console.print(
            f"  [dim]Included {len(ttf_files)} font(s) in share/fonts/[/dim]"
        )
        return True

    def _trim_terminfo(self, bundle_dir: Path) -> None:
        """Trim terminfo database to common terminals only.

        The full ncurses terminfo set is ~12MB with ~2500 entries. Most IRIX
        users need only a handful (iris-ansi, xterm, vt100, screen).
        """
        terminfo_dir = bundle_dir / "share" / "terminfo"
        if not terminfo_dir.is_dir():
            return

        removed = 0
        for subdir in sorted(terminfo_dir.iterdir()):
            if not subdir.is_dir():
                continue
            for entry in sorted(subdir.iterdir()):
                if entry.name not in self.TERMINFO_KEEP:
                    entry.unlink()
                    removed += 1
            # Remove empty subdirectories
            if not any(subdir.iterdir()):
                subdir.rmdir()

        if removed:
            console.print(
                f"  [dim]Trimmed {removed} terminfo entries (kept {len(self.TERMINFO_KEEP)})[/dim]"
            )

    def resolve_deps(
        self,
        target_package: str,
        extra_packages: list[str] | None = None,
    ) -> BundleManifest:
        """Resolve all transitive runtime dependencies for a package bundle."""
        # Find target RPMs — resolve name mismatches (vim → vim-enhanced, etc.)
        resolved_name = self._resolve_package_name(target_package)
        if resolved_name is None:
            # Package truly not found — check rules for build hints
            from pathlib import Path as _Path
            _rules_dir = _Path(__file__).parent.parent / "rules" / "packages"
            _rule_file = _rules_dir / f"{target_package}.yaml"
            if _rule_file.exists():
                import yaml as _yaml
                with open(_rule_file) as _f:
                    _rule = _yaml.safe_load(_f) or {}
                if _rule.get("skip") or _rule.get("blocked"):
                    reason = _rule.get("blocked_reason", "blocked/skipped in rules")
                    console.print(
                        f"[red]Package '{target_package}' is blocked: {reason}[/red]"
                    )
                elif "upstream" in _rule:
                    _url = _rule["upstream"].get("url", "")
                    console.print(
                        f"[red]Package '{target_package}' not found in {self.rpms_dir}[/red]\n"
                        f"[yellow]But it has upstream sources in {_rule_file.name}:[/yellow]\n"
                        f"  url: {_url}\n"
                        f"[yellow]Build it first:[/yellow]\n"
                        f"  uv run mogrix create-srpm {target_package}\n"
                        f"  uv run mogrix convert ~/mogrix_inputs/SRPMS/{target_package}-*.src.rpm\n"
                        f"  uv run mogrix build ~/mogrix_outputs/converted/{target_package}-*-converted/{target_package}-*.src.rpm --cross\n"
                        f"  uv run mogrix stage ~/rpmbuild/RPMS/mips/{target_package}*.rpm\n"
                        f"  cp ~/rpmbuild/RPMS/mips/{target_package}*.rpm ~/mogrix_outputs/RPMS/"
                    )
                else:
                    console.print(
                        f"[red]Package '{target_package}' not found in {self.rpms_dir}[/red]\n"
                        f"[yellow]Rule file exists ({_rule_file.name}) but no RPM built.[/yellow]\n"
                        f"[yellow]Try: uv run mogrix fetch {target_package} -y[/yellow]"
                    )
            else:
                console.print(
                    f"[red]Package '{target_package}' not found in {self.rpms_dir}[/red]"
                )
            raise SystemExit(1)

        if resolved_name != target_package:
            target_package = resolved_name

        target_rpms = self._name_to_rpms[target_package]
        target_version = self._rpm_query(target_rpms[0], "%{VERSION}-%{RELEASE}")

        # Collect initial RPMs: target + siblings + extras
        queue: list[Path] = []
        for rpm in target_rpms:
            queue.extend(self._get_sibling_rpms(rpm))

        for extra in extra_packages or []:
            resolved_extra = self._resolve_package_name(extra)
            if resolved_extra:
                for rpm in self._name_to_rpms[resolved_extra]:
                    queue.extend(self._get_sibling_rpms(rpm))
            else:
                console.print(
                    f"[yellow]Warning: extra package '{extra}' not found[/yellow]"
                )

        # Deduplicate initial queue
        queue = list(dict.fromkeys(queue))

        # Track which RPMs are target/explicitly-included (not deps)
        target_rpm_set = set(queue)

        manifest = BundleManifest(
            target_package=target_package,
            target_version=target_version,
        )

        visited_rpms: set[Path] = set()
        all_needed_sonames: set[str] = set()

        with tempfile.TemporaryDirectory() as tmpdir:
            tmppath = Path(tmpdir)

            while queue:
                rpm_path = queue.pop(0)
                if rpm_path in visited_rpms:
                    continue
                visited_rpms.add(rpm_path)

                rpm_name = self._rpm_query(rpm_path, "%{NAME}")
                console.print(f"  [dim]Scanning:[/dim] {rpm_name} ({rpm_path.name})")

                # Extract and scan
                extract_dir = tmppath / rpm_path.stem
                extract_dir.mkdir(parents=True, exist_ok=True)
                if not self._extract_rpm(rpm_path, extract_dir):
                    console.print(
                        f"  [yellow]Warning: failed to extract {rpm_path.name}[/yellow]"
                    )
                    continue

                needed = self._scan_elf_needed(extract_dir)
                all_needed_sonames.update(needed)

                # Resolve each needed soname
                for soname in needed:
                    if soname in manifest.irix_sonames:
                        continue
                    # Never bundle native IRIX system libs (X11, etc.).
                    # These must come from /usr/lib32 — our cross-compiled
                    # versions can't talk to the IRIX X server.
                    if soname in IRIX_NATIVE_SONAMES:
                        manifest.irix_sonames.add(soname)
                        continue
                    # Mogrix-built RPMs take priority over IRIX sysroot.
                    # IRIX may have ancient ABI-incompatible versions of
                    # libraries we've rebuilt (e.g. libz.so / zlib-ng).
                    # Try exact soname first, then versioned variants
                    # (libz.so → libz.so.1) since -devel symlinks aren't
                    # in runtime RPMs but the versioned lib is.
                    dep_rpm = self._soname_to_rpm.get(soname)
                    if not dep_rpm and ".so" in soname and not soname[-1].isdigit():
                        # Unversioned soname (libz.so) — look for libz.so.1, .2, etc.
                        for candidate_soname, candidate_rpm in self._soname_to_rpm.items():
                            if candidate_soname.startswith(soname + "."):
                                dep_rpm = candidate_rpm
                                break
                    if dep_rpm:
                        if dep_rpm not in visited_rpms:
                            # Add this RPM and its siblings
                            for sibling in self._get_sibling_rpms(dep_rpm):
                                if sibling not in visited_rpms:
                                    queue.append(sibling)
                        continue
                    if soname in self._irix_sonames:
                        manifest.irix_sonames.add(soname)
                        continue
                    # Skip absolute paths (not real sonames)
                    if soname.startswith("/"):
                        continue
                    # Staging fallback
                    if STAGING_LIB_DIR.is_dir():
                        staging_matches = list(STAGING_LIB_DIR.glob(soname))
                        if staging_matches:
                            manifest.staging_sonames.add(soname)
                            continue
                    manifest.unresolved_sonames.add(soname)

        manifest.included_rpms = sorted(visited_rpms)
        manifest.target_rpms = target_rpm_set & visited_rpms
        return manifest

    def create_bundle(
        self,
        target_package: str,
        output_dir: Path,
        extra_packages: list[str] | None = None,
        output_format: str = "run",
        suite_name: str | None = None,
        trampoline_exclude: set[str] | None = None,
    ) -> BundleManifest:
        """Create a self-contained app bundle.

        output_format: "run" (self-extracting, default), "tarball" (.tar.gz),
                       or "directory" (no packaging).

        If suite_name is provided, creates a suite bundle combining multiple
        packages under a single name (e.g., "mogrix-smallweb").
        """
        if suite_name:
            all_pkgs = [target_package] + list(extra_packages or [])
            console.print(
                f"\n[bold]Creating suite: {suite_name}[/bold]"
            )
            console.print(
                f"[dim]Packages: {', '.join(all_pkgs)}[/dim]\n"
            )
        else:
            console.print(f"\n[bold]Resolving dependencies for: {target_package}[/bold]\n")

        manifest = self.resolve_deps(target_package, extra_packages)

        if suite_name:
            manifest.suite_name = suite_name
            manifest.suite_packages = [target_package] + list(extra_packages or [])

        if manifest.unresolved_sonames:
            console.print("\n[red]Unresolved dependencies:[/red]")
            for soname in sorted(manifest.unresolved_sonames):
                console.print(f"  [red]  {soname}[/red]")

        if manifest.staging_sonames:
            console.print(
                "\n[yellow]Warning: These sonames will be copied from staging "
                "(not mogrix-built):[/yellow]"
            )
            for soname in sorted(manifest.staging_sonames):
                console.print(f"  [yellow]  {soname}[/yellow]")

        # Create bundle directory with alphabetic revision suffix
        if suite_name:
            base_name = f"{suite_name}-1"
        else:
            base_name = (
                f"{manifest.target_package}-{manifest.target_version}"
            )
        output_dir.mkdir(parents=True, exist_ok=True)

        # Clean up old bundles for this package (letter-suffix or date-serial)
        bundle_prefix = f"{base_name}-irix-bundle"
        for existing in sorted(output_dir.iterdir()):
            name = existing.name
            if existing.is_dir() and name.startswith(base_name) and (
                name.endswith("-irix-bundle")
                or "-irix-bundle." in name
            ):
                shutil.rmtree(existing)
            elif existing.is_file() and name.startswith(base_name) and (
                name.endswith("-irix-bundle.tar.gz")
                or "-irix-bundle." in name and name.endswith(".tar.gz")
            ):
                existing.unlink()

        # Date-serial suffix: MMDDYYHHmm
        date_serial = datetime.now().strftime("%m%d%y%H%M")
        bundle_name = f"{bundle_prefix}.{date_serial}"
        manifest.bundle_name = bundle_name
        bundle_dir = output_dir / bundle_name
        bundle_dir.mkdir(parents=True)

        console.print(f"\n[bold]Creating bundle: {bundle_name}[/bold]\n")

        # Extract all RPMs into a temp dir, then reorganize into bundle layout
        with tempfile.TemporaryDirectory() as tmpdir:
            tmppath = Path(tmpdir) / "extract"
            tmppath.mkdir()

            for rpm_path in manifest.included_rpms:
                rpm_name = self._rpm_query(rpm_path, "%{NAME}")
                console.print(f"  [dim]Extracting:[/dim] {rpm_name}")
                self._extract_rpm(rpm_path, tmppath)

            # Move prefix dir contents to bundle root with internal prefixes
            # Support both old (/usr/sgug) and new (/opt/mogrix) prefix
            sgug_dir = tmppath / "opt" / "mogrix"
            if not sgug_dir.is_dir():
                sgug_dir = tmppath / "usr" / "sgug"
            if sgug_dir.is_dir():
                for item in sgug_dir.iterdir():
                    # Rename bin/ -> _bin/, sbin/ -> _sbin/, lib32/ -> _lib32/
                    if item.name in ("bin", "sbin", "lib32"):
                        dest_name = f"_{item.name}"
                    else:
                        dest_name = item.name
                    dest = bundle_dir / dest_name
                    if dest.exists():
                        if item.is_dir() and dest.is_dir():
                            _merge_dirs(item, dest)
                    else:
                        shutil.move(str(item), str(dest))

        # Copy staging fallback sonames (symlink to avoid duplicating large files)
        if manifest.staging_sonames:
            lib_dir = bundle_dir / "_lib32"
            lib_dir.mkdir(exist_ok=True)
            for soname in manifest.staging_sonames:
                src = STAGING_LIB_DIR / soname
                if src.exists():
                    real_file = src.resolve()
                    if real_file != src and real_file.exists():
                        # Copy the real file, symlink the soname to it
                        shutil.copy2(str(real_file), str(lib_dir / real_file.name))
                        dest_link = lib_dir / soname
                        if not dest_link.exists():
                            dest_link.symlink_to(real_file.name)
                    else:
                        shutil.copy2(str(src), str(lib_dir / soname))

        # Include mogrix compat shared library for _RLDN32_LIST preload.
        # IRIX rld doesn't preempt shared library symbols from the executable,
        # so compat overrides (e.g. bsearch) must be in a preloaded .so.
        compat_so_src = STAGING_LIB_DIR / "libmogrix_compat.so"
        if compat_so_src.exists():
            lib_dir = bundle_dir / "_lib32"
            lib_dir.mkdir(exist_ok=True)
            shutil.copy2(str(compat_so_src), str(lib_dir / "libmogrix_compat.so"))

        # Include rld stubs library for _RLDN32_LIST preload.
        # Provides stub implementations for symbols below IRIX rld's GOTSYM
        # threshold (e.g. g_power_profile_monitor_* in libgio). Built without
        # CRT objects (no DT_INIT) to avoid SIGILL on _RLDN32_LIST preload.
        rld_stubs_src = STAGING_LIB_DIR / "irix_rld_stubs.so"
        if rld_stubs_src.exists():
            lib_dir = bundle_dir / "_lib32"
            lib_dir.mkdir(exist_ok=True)
            shutil.copy2(str(rld_stubs_src), str(lib_dir / "irix_rld_stubs.so"))

        # NOTE: Do NOT force-include libgcc_s.so.1 here. It has non-weak
        # pthread symbol refs that cause either "unresolvable symbol" (if
        # libpthread isn't loaded) or "_RLD_PTHREADS_START invoked twice"
        # (if libpthread is preloaded via _RLDN32_LIST). Packages that need
        # libgcc_s quad-float builtins (__getf2, __multf3) should add -lgcc_s
        # to their link flags so it appears as a proper DT_NEEDED entry.

        # Create missing soname symlinks.  -devel RPMs (excluded from bundles)
        # often contain unversioned .so symlinks (e.g. libz.so → libz.so.1)
        # that are needed at runtime when the ELF SONAME is unversioned.
        self._create_soname_symlinks(bundle_dir)

        # Include GIO extension modules (TLS, proxy) for GLib-using bundles.
        # Must run before pruning so module deps are in the needed set.
        self._include_gio_modules(bundle_dir)

        # Prune _lib32/ to only sonames actually NEEDED by bundle binaries.
        # RPMs include all libs from the package (e.g., ncurses-libs has 14 .so
        # files) but the bundle may only need a few.
        self._prune_unused_libs(bundle_dir)

        # Strip build-time RPATHs that break IRIX rld library search.
        self._strip_rpaths(bundle_dir)

        # Rebase shared libraries to unique non-overlapping base addresses.
        # Set MOGRIX_NO_MRQS=1 to skip rebasing (debugging), or set
        # skip_mrqs_rebase: true in the package rules YAML.
        # IRIX rld loads libraries at their preferred address if possible.
        # Without rebasing, all mogrix-built libs share 0x0f800000 and rld
        # displaces all but one — breaking pre-resolved R_MIPS_REL32 relocs.
        # mrqs assigns unique addresses and patches each library in-place.
        skip_rebase = bool(os.environ.get("MOGRIX_NO_MRQS"))
        if not skip_rebase:
            # Check package rules for skip_mrqs_rebase flag
            from pathlib import Path as _P
            _rf = _P(__file__).parent.parent / "rules" / "packages" / f"{manifest.target_package}.yaml"
            if _rf.exists():
                import yaml as _y
                with open(_rf) as _f:
                    _r = _y.safe_load(_f) or {}
                skip_rebase = _r.get("skip_mrqs_rebase", False)
        if not skip_rebase:
            self._rebase_libraries(bundle_dir)
        else:
            console.print("[yellow]  mrqs rebase SKIPPED (skip_mrqs_rebase or MOGRIX_NO_MRQS)[/yellow]")

            # Pre-resolve UND symbol R_MIPS_REL32 relocations in executables.
            # After mrqs rebases libraries, their symbol addresses are final.
            # fix-anon-relocs --pre-resolve-only reads the rebased .dynsym
            # tables and writes correct runtime addresses into the executable,
            # zeroing the relocation entries so rld doesn't try to process them.
            # This fixes C++ dynamic_cast crashes (typeinfo vtable pointers).
            self._pre_resolve_executables(bundle_dir)

        # Strip runtime-unnecessary data directories
        for strip_dir in ("doc", "man", "info", "licenses"):
            d = bundle_dir / "share" / strip_dir
            if d.is_dir():
                shutil.rmtree(d)

        # Trim terminfo to common terminals (full set is ~12MB)
        self._trim_terminfo(bundle_dir)

        # Compile GSettings schemas if present (GTK/GLib abort without these)
        gsettings_schema_dir = bundle_dir / "share" / "glib-2.0" / "schemas"
        if gsettings_schema_dir.is_dir() and any(
            f.name.endswith(".gschema.xml") for f in gsettings_schema_dir.iterdir()
        ):
            result = subprocess.run(
                ["glib-compile-schemas", str(gsettings_schema_dir)],
                capture_output=True,
                text=True,
            )
            if result.returncode == 0:
                console.print(f"[dim]Compiled GSettings schemas in {gsettings_schema_dir}[/dim]")
            else:
                console.print(f"[yellow]Failed to compile GSettings schemas: {result.stderr}[/yellow]")

        # Include system CA certificates for TLS-using bundles
        self._include_ca_bundle(bundle_dir)

        # Include bundled fonts for X11 apps using Xft/fontconfig
        has_fonts = self._include_fonts(bundle_dir)

        # Detect if bundle has terminfo data (for TERMINFO env var in wrappers)
        has_terminfo = (bundle_dir / "share" / "terminfo").is_dir()
        terminfo_block = TERMINFO_BLOCK if has_terminfo else ""

        # Detect _lib32/ subdirectories that contain .so files (private lib dirs).
        # e.g., man-db installs libs in lib32/man-db/ which rld can't find
        # unless we add the directory to LD_LIBRARYN32_PATH.
        extra_lib_paths = ""
        lib_dir = bundle_dir / "_lib32"
        if lib_dir.is_dir():
            for subdir in sorted(lib_dir.iterdir()):
                if subdir.is_dir() and any(
                    f.name.endswith(".so") or ".so." in f.name
                    for f in subdir.iterdir()
                    if f.is_file() or f.is_symlink()
                ):
                    extra_lib_paths += f":$dir/_lib32/{subdir.name}"

        # Detect app-specific env vars needed for plugin loading etc.
        extra_env_lines = []
        extra_args_map = {}  # binary name -> extra CLI args string

        # Global plugin directory detection: scan _lib32/ for known plugin
        # subdirs and set env vars so libraries find their dlopen'd modules
        # inside the bundle instead of at hardcoded /opt/mogrix/lib32/ paths.
        lib32_dir = bundle_dir / "_lib32"
        if lib32_dir.is_dir():
            for subdir_rel, (env_var, val_template) in PLUGIN_DIR_ENV_MAP.items():
                plugin_path = lib32_dir / subdir_rel
                if plugin_path.is_dir() and any(
                    f.name.endswith(".so") for f in plugin_path.iterdir() if f.is_file()
                ):
                    val = val_template.replace("{lib32}", "$dir/_lib32")
                    extra_env_lines.append(f'{env_var}="{val}"')
                    extra_env_lines.append(f"export {env_var}")

        # weechat: WEECHAT_EXTRA_LIBDIR for dlopen-loaded plugins
        weechat_plugins = bundle_dir / "_lib32" / "weechat" / "plugins"
        if weechat_plugins.is_dir():
            extra_env_lines.append(
                'WEECHAT_EXTRA_LIBDIR="$dir/_lib32/weechat"'
            )
            extra_env_lines.append("export WEECHAT_EXTRA_LIBDIR")
        # Fontconfig — point to bundle's fonts.conf so fontconfig doesn't look
        # at /opt/mogrix/etc/fonts/ (which doesn't exist on IRIX).
        # Set this whenever fonts.conf exists in the bundle, not just when
        # TTF fonts were included — fontconfig needs the config even without
        # bundled fonts (it can use IRIX system fonts via the config).
        bundle_fonts_conf = bundle_dir / "etc" / "fonts" / "fonts.conf"
        if has_fonts or bundle_fonts_conf.is_file():
            extra_env_lines.append(
                'FONTCONFIG_FILE="$dir/etc/fonts/fonts.conf"'
            )
            extra_env_lines.append("export FONTCONFIG_FILE")
        # figlet: FIGLET_FONTDIR so bundled fonts are found
        figlet_fonts = bundle_dir / "share" / "figlet"
        if figlet_fonts.is_dir():
            extra_env_lines.append(
                'FIGLET_FONTDIR="$dir/share/figlet"'
            )
            extra_env_lines.append("export FIGLET_FONTDIR")
        # Per-package bundle_env from rules YAML — generic mechanism for any
        # package to declare env vars needed in bundle wrappers.
        # Format in rules/packages/<pkg>.yaml:
        #   bundle_env:
        #     MC_DATADIR: "$dir/share/mc/"
        #     MC_SYSCONFDIR: "$dir/etc/"
        from pathlib import Path as _BPath
        _rules_dir = _BPath(__file__).parent.parent / "rules" / "packages"
        _rule_file = _rules_dir / f"{manifest.target_package}.yaml"
        if _rule_file.exists():
            import yaml as _yaml
            with open(_rule_file) as _rf:
                _rule = _yaml.safe_load(_rf) or {}
            for env_var, env_val in _rule.get("bundle_env", {}).items():
                extra_env_lines.append(f'{env_var}="{env_val}"')
                extra_env_lines.append(f"export {env_var}")
            # bundle_post_commands: shell commands run in the bundle dir
            # after assembly, before packaging. For creating symlinks, etc.
            for cmd in _rule.get("bundle_post_commands", []):
                console.print(f"  [dim]post-command:[/dim] {cmd}")
                subprocess.run(
                    cmd, shell=True, cwd=str(bundle_dir),
                    capture_output=True, text=True,
                )
        # CA certificate bundle — set env var + weechat gnutls_ca_user
        ca_bundle = bundle_dir / "etc" / "pki" / "tls" / "certs" / "ca-bundle.crt"
        if ca_bundle.is_file():
            extra_env_lines.append(
                'SSL_CERT_FILE="$dir/etc/pki/tls/certs/ca-bundle.crt"'
            )
            extra_env_lines.append("export SSL_CERT_FILE")
            # GnuTLS CA file for WebKit — WEBKIT_TLS_CAFILE_PEM overrides
            # the compiled-in path so soup session loads the bundle's CA certs
            extra_env_lines.append(
                'WEBKIT_TLS_CAFILE_PEM="$dir/etc/pki/tls/certs/ca-bundle.crt"'
            )
            extra_env_lines.append("export WEBKIT_TLS_CAFILE_PEM")
        # dillo: dpid needs dpidrc pointing to bundle's DPI directory, and
        # dillo launches dpid via execl(~/.dillo/dpid) so we need a wrapper
        # there too. Also write dillorc with IRIX bitmap fonts.
        dillo_dpi_dir = bundle_dir / "_lib32" / "dillo" / "dpi"
        if dillo_dpi_dir.is_dir():
            extra_env_lines.append(
                '# Set up dillo DPI daemon config for bundle\n'
                '_dillo_home="$HOME/.dillo"\n'
                '/bin/mkdir -p "$_dillo_home" 2>/dev/null\n'
                '# dpidrc: tell dpid where DPI plugins live\n'
                'echo "dpi_dir=$dir/_lib32/dillo/dpi" > "$_dillo_home/dpidrc"\n'
                'echo "proto.file=file/file.dpi" >> "$_dillo_home/dpidrc"\n'
                'echo "proto.ftp=ftp/ftp.filter.dpi" >> "$_dillo_home/dpidrc"\n'
                'echo "proto.https=https/https.filter.dpi" >> "$_dillo_home/dpidrc"\n'
                'echo "proto.data=datauri/datauri.filter.dpi" >> "$_dillo_home/dpidrc"\n'
                '# dpid wrapper: dillo exec()s ~/.dillo/dpid directly\n'
                'cat > "$_dillo_home/dpid" << DPID_EOF\n'
                '#!/bin/sh\n'
                'LD_LIBRARYN32_PATH="$dir/_lib32:/usr/lib32"\n'
                'export LD_LIBRARYN32_PATH\n'
                '_RLDN32_LIST="$dir/_lib32/libmogrix_compat.so:DEFAULT"\n'
                'export _RLDN32_LIST\n'
                'exec "$dir/_bin/dpid" "\\$@"\n'
                'DPID_EOF\n'
                '/bin/chmod 755 "$_dillo_home/dpid"\n'
                '# dillorc: IRIX bitmap fonts (no Xft/DejaVu)\n'
                'if [ ! -f "$_dillo_home/dillorc" ]; then\n'
                '  cat > "$_dillo_home/dillorc" << DILLORC_EOF\n'
                'font_serif="times"\n'
                'font_sans_serif="helvetica"\n'
                'font_cursive="helvetica"\n'
                'font_fantasy="helvetica"\n'
                'font_monospace="courier"\n'
                'font_factor=1.0\n'
                'DILLORC_EOF\n'
                'fi'
            )
        # WebKitGTK: WEBKIT_EXEC_PATH tells WebKit where to find subprocess
        # executables (WebKitWebProcess, WebKitNetworkProcess). Without this,
        # WebKit uses the compiled-in PKGLIBEXECDIR (/opt/mogrix/libexec/webkit2gtk-4.0)
        # which doesn't exist on the live IRIX host.
        webkit_libexec = bundle_dir / "libexec" / "webkit2gtk-4.0"
        if webkit_libexec.is_dir():
            extra_env_lines.append(
                'WEBKIT_EXEC_PATH="$dir/libexec/webkit2gtk-4.0"'
            )
            extra_env_lines.append("export WEBKIT_EXEC_PATH")
            # XDG directories — IRIX has no login manager to create these.
            # WebKit/GLib need ~/.cache and ~/.local/share for data manager,
            # fontconfig cache, etc. Without them, fresh users get IPC errors
            # because WebProcess/NetworkProcess fail to initialize.
            extra_env_lines.append(
                '/bin/mkdir -p "$HOME/.cache" "$HOME/.local/share" 2>/dev/null'
            )
            # XDG_RUNTIME_DIR — GLib expects this for runtime state.
            # IRIX has no systemd; create a per-user temp dir.
            extra_env_lines.append(
                'if [ -z "$XDG_RUNTIME_DIR" ]; then\n'
                '    XDG_RUNTIME_DIR="/tmp/.xdg-runtime-`id -un`"\n'
                '    /bin/mkdir -p "$XDG_RUNTIME_DIR" 2>/dev/null\n'
                '    chmod 700 "$XDG_RUNTIME_DIR" 2>/dev/null\n'
                '    export XDG_RUNTIME_DIR\n'
                'fi'
            )
        # GIO modules — point to bundle's module directory for TLS/proxy support.
        # GIO modules are dlopen'd at runtime (not NEEDED deps), so GLib needs
        # GIO_MODULE_DIR to find them. Without this, no TLS backend = no HTTPS.
        gio_modules_dir = bundle_dir / "_lib32" / "gio" / "modules"
        if gio_modules_dir.is_dir() and any(
            f.name.endswith(".so")
            for f in gio_modules_dir.iterdir()
            if f.is_file() or f.is_symlink()
        ):
            extra_env_lines.append(
                'GIO_MODULE_DIR="$dir/_lib32/gio/modules"'
            )
            extra_env_lines.append("export GIO_MODULE_DIR")
        # GSettings schemas — GTK/GLib need compiled schemas for settings.
        # Without GSETTINGS_SCHEMA_DIR, GSettings looks in system paths that
        # don't exist on IRIX. Point to bundle's schemas directory.
        gsettings_dir = bundle_dir / "share" / "glib-2.0" / "schemas"
        if gsettings_dir.is_dir() and any(
            f.name.endswith(".gschema.xml") for f in gsettings_dir.iterdir()
        ):
            extra_env_lines.append(
                'GSETTINGS_SCHEMA_DIR="$dir/share/glib-2.0/schemas"'
            )
            extra_env_lines.append("export GSETTINGS_SCHEMA_DIR")
        # JSC JIT memory tuning — IRIX N32 has 2GB virtual address limit and
        # machines typically have 256MB-1.5GB physical RAM. Disable DFG optimizing
        # JIT (keep baseline JIT for decent performance with much less memory) and
        # cap JIT code buffer. Only set if not already set so users can override.
        if webkit_libexec.is_dir():
            extra_env_lines.append(
                '# JSC memory tuning for IRIX (override with env vars before launch)\n'
                ': ${JSC_useDFGJIT=false}\n'
                'export JSC_useDFGJIT\n'
                ': ${JSC_jitMemoryReservationSize=16777216}\n'
                'export JSC_jitMemoryReservationSize\n'
                '# JSC GC tuning — tell GC this is a 512MB machine (WPE embedded pattern)\n'
                ': ${JSC_forceRAMSize=536870912}\n'
                'export JSC_forceRAMSize\n'
                ': ${JSC_criticalGCMemoryThreshold=0.50}\n'
                'export JSC_criticalGCMemoryThreshold\n'
                ': ${JSC_smallHeapGrowthFactor=1.1}\n'
                'export JSC_smallHeapGrowthFactor\n'
                ': ${JSC_mediumHeapGrowthFactor=1.1}\n'
                'export JSC_mediumHeapGrowthFactor\n'
                ': ${JSC_largeHeapGrowthFactor=1.1}\n'
                'export JSC_largeHeapGrowthFactor'
            )
        # libevent: IRIX /dev/poll doesn't work with STREAMS-based PTY masters.
        # Disable devpoll backend so libevent falls back to poll() syscall.
        has_libevent = any(
            f.name.startswith("libevent")
            for f in (bundle_dir / "_lib32").iterdir()
            if f.is_file() or f.is_symlink()
        ) if (bundle_dir / "_lib32").is_dir() else False
        if has_libevent:
            extra_env_lines.append('EVENT_NODEVPOLL=1')
            extra_env_lines.append("export EVENT_NODEVPOLL")
        extra_env_block = (
            "\n".join(extra_env_lines) + "\n" if extra_env_lines else ""
        )

        # Find all binaries (real files + symlinks in _bin/ and _sbin/)
        bin_dir = bundle_dir / "_bin"
        sbin_dir = bundle_dir / "_sbin"
        binaries = []
        if bin_dir.is_dir():
            for f in sorted(bin_dir.iterdir()):
                if not f.name.startswith("."):
                    binaries.append(f.name)
        sbin_binaries = []
        if sbin_dir.is_dir():
            for f in sorted(sbin_dir.iterdir()):
                if not f.name.startswith("."):
                    sbin_binaries.append(f.name)

        manifest.binaries = binaries + [f"sbin/{b}" for b in sbin_binaries]

        # Generate wrapper scripts at bundle root, named after the commands
        # Build _RLDN32_LIST from all preload libraries in the bundle.
        # Use absolute paths ($dir/_lib32/...) so child processes (e.g. user's
        # shell spawned by tmux) can find them even outside the bundle's
        # LD_LIBRARYN32_PATH.
        # NOTE: Do NOT preload libgcc_s.so.1 — it has non-weak pthread
        # refs that cause rld errors. Do NOT preload libpthread.so — it
        # causes "_RLD_PTHREADS_START invoked twice". Only preload compat
        # libs that have no problematic dependencies.
        rld_list_libs = []
        for preload_name in ("libmogrix_compat.so", "irix_rld_stubs.so"):
            if (bundle_dir / "_lib32" / preload_name).exists():
                rld_list_libs.append(f"$dir/_lib32/{preload_name}")
        if rld_list_libs:
            rld_list_value = ":".join(rld_list_libs) + ":DEFAULT"
            rld_list_block = f'_RLDN32_LIST="{rld_list_value}"\nexport _RLDN32_LIST\n'
        else:
            rld_list_block = ""

        for binary in binaries:
            wrapper_path = bundle_dir / binary
            wrapper_path.write_text(
                WRAPPER_TEMPLATE.format(
                    binary=binary,
                    terminfo_block=terminfo_block,
                    extra_env_block=extra_env_block,
                    extra_lib_paths=extra_lib_paths,
                    extra_args=extra_args_map.get(binary, ""),
                    rld_list_block=rld_list_block,
                )
            )
            wrapper_path.chmod(0o755)

        for binary in sbin_binaries:
            wrapper_path = bundle_dir / binary
            wrapper_path.write_text(
                SBIN_WRAPPER_TEMPLATE.format(
                    binary=binary,
                    terminfo_block=terminfo_block,
                    extra_env_block=extra_env_block,
                    extra_lib_paths=extra_lib_paths,
                    extra_args=extra_args_map.get(binary, ""),
                    rld_list_block=rld_list_block,
                )
            )
            wrapper_path.chmod(0o755)

        # Generate wrappers for libexec/ binaries (e.g. MiniBrowser).
        # These are placed at the bundle root using the binary's basename.
        libexec_dir = bundle_dir / "libexec"
        if libexec_dir.is_dir():
            for f in libexec_dir.rglob("*"):
                if f.is_file() and self._is_elf(f):
                    rel_path = f.relative_to(bundle_dir)
                    wrapper_name = f.name
                    # Avoid colliding with existing wrappers
                    if (bundle_dir / wrapper_name).exists():
                        continue
                    wrapper_path = bundle_dir / wrapper_name
                    wrapper_path.write_text(
                        LIBEXEC_WRAPPER_TEMPLATE.format(
                            libexec_path=str(rel_path),
                            terminfo_block=terminfo_block,
                            extra_env_block=extra_env_block,
                            extra_lib_paths=extra_lib_paths,
                            rld_list_block=rld_list_block,
                        )
                    )
                    wrapper_path.chmod(0o755)
                    binaries.append(wrapper_name)

        # Determine which binaries belong to the target/explicitly-included
        # packages (not transitive dependencies). Only these get trampolines
        # in the shared bin/ directory. Dependency binaries still have wrapper
        # scripts in the bundle dir for direct invocation.
        target_bins: set[str] = set()
        for rpm_path in manifest.target_rpms:
            for fpath in self._rpm_filelist(rpm_path):
                fname = Path(fpath).name
                if fpath.startswith("/opt/mogrix/bin/") and fname in binaries:
                    target_bins.add(fname)
                elif fpath.startswith("/opt/mogrix/sbin/") and fname in sbin_binaries:
                    target_bins.add(fname)

        # Apply global + package-level trampoline exclusions
        excluded = TRAMPOLINE_EXCLUDE_GLOBAL.copy()
        if trampoline_exclude:
            excluded |= trampoline_exclude

        trampoline_cmds = [b for b in binaries if b in target_bins and b not in excluded]
        trampoline_cmds += [b for b in sbin_binaries if b in target_bins and b not in excluded]

        skipped = sorted(excluded & target_bins)
        if skipped:
            console.print(f"[dim]  Trampoline exclusions: {', '.join(skipped)}[/dim]")

        # Generate install/uninstall scripts (trampolines only for target binaries)
        install_label = manifest.suite_name or manifest.target_package
        self._generate_install_scripts(
            install_label, bundle_name, trampoline_cmds, bundle_dir
        )

        # Generate README
        self._generate_readme(manifest, bundle_dir)

        # Copy package-specific docs from docs/ directory
        docs_dir = Path(__file__).parent.parent / "docs"
        if docs_dir.is_dir():
            target_pkg = manifest.target_package or ""
            for doc in docs_dir.iterdir():
                if doc.is_file() and target_pkg and doc.stem.startswith(target_pkg):
                    shutil.copy2(str(doc), str(bundle_dir / doc.name))

        manifest.bundle_dir = bundle_dir

        # Fix permissions: RPM extraction preserves restrictive modes (700 dirs,
        # 600 files). Make everything world-readable for IRIX users.
        bundle_dir.chmod(bundle_dir.stat().st_mode | 0o555)
        for root, dirs, files in os.walk(bundle_dir):
            for d in dirs:
                p = Path(root) / d
                if p.is_symlink():
                    continue
                p.chmod(p.stat().st_mode | 0o555)
            for f in files:
                p = Path(root) / f
                if p.is_symlink():
                    continue
                mode = p.stat().st_mode
                # Add read for all; add execute for all if owner has execute
                p.chmod(mode | 0o444 | (0o111 if mode & 0o100 else 0))

        # Package the bundle
        if output_format in ("run", "tarball"):
            tarball_name = f"{bundle_name}.tar.gz"
            tarball_path = output_dir / tarball_name
            console.print(f"\n[dim]Creating tarball: {tarball_name}[/dim]")
            subprocess.run(
                [
                    "tar",
                    "--format=ustar",
                    "--owner=0",
                    "--group=0",
                    "-czf",
                    str(tarball_path),
                    "-C",
                    str(output_dir),
                    bundle_name,
                ],
                check=True,
            )

            if output_format == "run":
                display = (
                    manifest.suite_name
                    or f"{manifest.target_package} {manifest.target_version}"
                )
                run_name = f"{bundle_name}.run"
                run_path = output_dir / run_name
                console.print(f"[dim]Creating installer: {run_name}[/dim]")
                self._create_self_extracting(
                    run_path, tarball_path, bundle_name, display
                )
                tarball_path.unlink()  # Remove intermediate tarball
                manifest.run_path = run_path
            else:
                manifest.tarball_path = tarball_path

            # Clean up the working directory — output_dir is for produced
            # bundles (.run / .tar.gz), not temp assembly directories.
            shutil.rmtree(bundle_dir)
            manifest.bundle_dir = None

        # Print summary
        self._print_summary(manifest)

        return manifest

    def _generate_install_scripts(
        self,
        package: str,
        bundle_name: str,
        commands: list[str],
        bundle_dir: Path,
    ) -> None:
        """Generate install and uninstall scripts.

        Install creates 2-line trampoline scripts in ../bin/ that exec the
        real wrappers. This avoids the symlink dirname problem (dirname of a
        symlink resolves to the symlink's directory, not the target's).
        """
        # Each trampoline resolves its own location at runtime and uses a
        # relative path (../<bundle>/<cmd>) to reach the wrapper. This avoids
        # baking in absolute paths that break across chroot boundaries.
        trampoline_lines = []
        unlink_lines = []
        for cmd in commands:
            # Check registry for existing owner — warn on conflict
            trampoline_lines.append(
                f'prev_owner=`grep "^{cmd}=" "$registry" 2>/dev/null | sed "s/^{cmd}=//"`'
            )
            trampoline_lines.append(
                f'if [ -n "$prev_owner" ] && [ "$prev_owner" != "$bundle" ]; then'
            )
            trampoline_lines.append(
                f'  echo "  {cmd} (was: $prev_owner)"'
            )
            trampoline_lines.append("else")
            trampoline_lines.append(f'  echo "  {cmd}"')
            trampoline_lines.append("fi")
            trampoline_lines.append(
                f'echo \'#!/bin/sh\' > "$bindir/{cmd}"'
            )
            trampoline_lines.append(
                f'echo \'dir=`/bin/dirname "$0"`\' >> "$bindir/{cmd}"'
            )
            trampoline_lines.append(
                f'echo \'case "$dir" in /*) ;; *) dir="`/bin/pwd`/$dir" ;; esac\' >> "$bindir/{cmd}"'
            )
            trampoline_lines.append(
                f'echo "exec \\"\\$dir/../$bundle/{cmd}\\" \\"\\$@\\"" >> "$bindir/{cmd}"'
            )
            trampoline_lines.append(
                f'chmod 755 "$bindir/{cmd}"'
            )
            # Update registry: remove old entry, add new
            trampoline_lines.append(
                f'grep -v "^{cmd}=" "$registry" > "$registry.tmp" 2>/dev/null; mv "$registry.tmp" "$registry"'
            )
            trampoline_lines.append(
                f'echo "{cmd}=$bundle" >> "$registry"'
            )
            # Uninstall: only remove if this bundle still owns the trampoline
            unlink_lines.append(
                f'owner=`grep "^{cmd}=" "$registry" 2>/dev/null | sed "s/^{cmd}=//"`'
            )
            unlink_lines.append(
                f'if [ "$owner" = "$bundle" ] || [ -z "$owner" ]; then'
            )
            unlink_lines.append(
                f'  rm -f "$bindir/{cmd}" && echo "  {cmd}"'
            )
            unlink_lines.append("else")
            unlink_lines.append(
                f'  echo "  {cmd} (kept — owned by $owner)"'
            )
            unlink_lines.append("fi")

        install_path = bundle_dir / "install"
        install_path.write_text(
            INSTALL_TEMPLATE.format(
                package=package,
                trampoline_commands="\n".join(trampoline_lines),
            )
        )
        install_path.chmod(0o755)

        uninstall_path = bundle_dir / "uninstall"
        uninstall_path.write_text(
            UNINSTALL_TEMPLATE.format(
                package=package,
                unlink_commands="\n".join(unlink_lines),
            )
        )
        uninstall_path.chmod(0o755)

    def _generate_readme(self, manifest: BundleManifest, bundle_dir: Path) -> None:
        """Generate README with bundle contents and instructions."""
        bundle_name = bundle_dir.name
        is_suite = bool(manifest.suite_name)

        if is_suite:
            title = f"{manifest.suite_name} — IRIX App Suite"
            description = (
                f"Suite bundle containing {len(manifest.suite_packages)} "
                f"packages for IRIX 6.5 (MIPS n32)."
            )
        else:
            title = f"{manifest.target_package} {manifest.target_version} — IRIX App Bundle"
            description = "Self-contained app bundle for IRIX 6.5 (MIPS n32)."

        primary = (
            manifest.target_package
            if manifest.target_package
            in [os.path.basename(b) for b in manifest.binaries]
            else (manifest.binaries[0] if manifest.binaries else "BINARY")
        )
        lines = [
            f"# {title}",
            "",
            description,
            "Generated by mogrix (https://github.com/unxmaal/mogrix).",
            "",
        ]

        if is_suite:
            lines.extend([
                "## Included Applications",
                "",
            ])
            for pkg in manifest.suite_packages:
                lines.append(f"  - {pkg}")
            lines.append("")

        lines.extend([
            "## Quick Install (from .run file)",
            "",
            f"    sh {bundle_name}.run ~/apps",
            "",
            "## Manual Install (from .tar.gz)",
            "",
            f"    cd ~/apps",
            f"    gunzip {bundle_name}.tar.gz",
            f"    tar xf {bundle_name}.tar",
            f"    cd {bundle_name}",
            "    ./install",
            "",
            "Then add ~/apps/bin to your PATH (once, in ~/.profile):",
            "",
            "    PATH=~/apps/bin:$PATH; export PATH",
            "",
            f"Now just run: {primary}",
            "",
            "## Uninstall",
            "",
            f"    cd ~/apps/{bundle_name}",
            "    ./uninstall",
            f"    rm -rf ~/apps/{bundle_name}",
            "",
            "## Available Commands",
            "",
        ])
        for binary in manifest.binaries:
            basename = os.path.basename(binary)
            lines.append(f"    {basename}")
        lines.extend(
            [
                "",
                "## Included Packages",
                "",
            ]
        )
        for rpm_path in manifest.included_rpms:
            name = self._rpm_query(rpm_path, "%{NAME}-%{VERSION}-%{RELEASE}")
            lines.append(f"  - {name}")

        if manifest.irix_sonames:
            lines.extend(
                [
                    "",
                    "## IRIX System Libraries (not bundled)",
                    "",
                ]
            )
            for soname in sorted(manifest.irix_sonames):
                lines.append(f"  - {soname}")

        if manifest.staging_sonames:
            lines.extend(
                [
                    "",
                    "## SGUG-RSE Libraries (bundled from staging, not mogrix-built)",
                    "",
                ]
            )
            for soname in sorted(manifest.staging_sonames):
                lines.append(f"  - {soname}")

        lines.append("")
        (bundle_dir / "README").write_text("\n".join(lines))

    def _create_self_extracting(
        self,
        run_path: Path,
        tarball_path: Path,
        bundle_name: str,
        display_name: str,
    ) -> None:
        """Create a self-extracting .run file from a tar.gz payload.

        Concatenates a Bourne shell header with the tar.gz binary data.
        The header uses tail +N to skip itself, piping through gzcat | tar.
        """
        # Format template with a placeholder for SKIP line number
        script = SELF_EXTRACTING_TEMPLATE.format(
            display_name=display_name,
            filename=run_path.name,
            bundle_dir_name=bundle_name,
            payload_line="__PLACEHOLDER__",
        )
        # Count lines and replace placeholder with actual line count + 1
        line_count = script.count("\n")
        # tail +N starts output at line N, so payload starts at line_count + 1
        script = script.replace("__PLACEHOLDER__", str(line_count + 1))

        # Write script header (text) then append binary payload
        with open(run_path, "wb") as f:
            f.write(script.encode("ascii"))
            with open(tarball_path, "rb") as payload:
                while True:
                    chunk = payload.read(65536)
                    if not chunk:
                        break
                    f.write(chunk)

        # Make executable
        run_path.chmod(0o755)

    def _print_summary(self, manifest: BundleManifest) -> None:
        """Print a Rich summary table."""
        console.print()

        if manifest.suite_name:
            table = Table(title=f"Suite: {manifest.suite_name}")
        else:
            table = Table(title=f"Bundle: {manifest.target_package}")
        table.add_column("Category", style="bold")
        table.add_column("Details")

        if manifest.suite_name:
            table.add_row("Suite packages", ", ".join(manifest.suite_packages))
            table.add_row("Bundle", manifest.bundle_name)
        else:
            table.add_row("Version", f"{manifest.target_version} (bundle: {manifest.bundle_name})" if manifest.bundle_name else manifest.target_version)
        table.add_row(
            "Included RPMs",
            str(len(manifest.included_rpms)),
        )

        rpm_names = []
        for rpm_path in manifest.included_rpms:
            rpm_names.append(self._rpm_query(rpm_path, "%{NAME}"))
        table.add_row("Packages", ", ".join(rpm_names))

        table.add_row("Binaries", ", ".join(manifest.binaries) or "(none)")
        table.add_row(
            "IRIX native libs",
            f"{len(manifest.irix_sonames)} skipped",
        )
        if manifest.staging_sonames:
            table.add_row(
                "Staging fallback",
                ", ".join(sorted(manifest.staging_sonames)),
            )
        if manifest.unresolved_sonames:
            table.add_row(
                "UNRESOLVED",
                ", ".join(sorted(manifest.unresolved_sonames)),
            )

        if manifest.bundle_dir:
            # Get bundle size
            total_size = sum(
                f.stat().st_size
                for f in manifest.bundle_dir.rglob("*")
                if f.is_file()
            )
            table.add_row("Bundle size", f"{total_size / 1024 / 1024:.1f} MB")

        if manifest.run_path and manifest.run_path.exists():
            run_size = manifest.run_path.stat().st_size
            table.add_row(
                "Installer",
                f"{manifest.run_path.name} ({run_size / 1024 / 1024:.1f} MB)",
            )

        if manifest.tarball_path and manifest.tarball_path.exists():
            tarball_size = manifest.tarball_path.stat().st_size
            table.add_row(
                "Tarball",
                f"{manifest.tarball_path.name} ({tarball_size / 1024 / 1024:.1f} MB)",
            )

        console.print(table)

        if manifest.unresolved_sonames:
            console.print(
                "\n[red]Bundle may not work — unresolved dependencies![/red]"
            )
        elif manifest.staging_sonames:
            # libgcc_s and libstdc++ always come from the cross-compiler toolchain;
            # they're bundled in _lib32/ so the bundle IS self-contained.
            toolchain_only = manifest.staging_sonames <= {
                "libgcc_s.so.1", "libstdc++.so.6",
            }
            if toolchain_only:
                console.print(
                    "\n[bold green]Bundle is self-contained.[/bold green] "
                    "[dim](toolchain libs from staging)[/dim]"
                )
            else:
                console.print(
                    "\n[yellow]Bundle includes non-mogrix libs from staging: "
                    + ", ".join(sorted(manifest.staging_sonames - {"libgcc_s.so.1", "libstdc++.so.6"}))
                    + "[/yellow]"
                )
        else:
            console.print("\n[bold green]Bundle is fully self-contained.[/bold green]")


def _merge_dirs(src: Path, dest: Path) -> None:
    """Recursively merge src directory into dest."""
    for item in src.iterdir():
        dest_item = dest / item.name
        if item.is_dir():
            if dest_item.is_dir():
                _merge_dirs(item, dest_item)
            elif not dest_item.exists():
                shutil.move(str(item), str(dest_item))
        else:
            if not dest_item.exists():
                shutil.move(str(item), str(dest_item))
