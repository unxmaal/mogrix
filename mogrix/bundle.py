"""Self-contained app bundle generator for IRIX.

Creates app bundles that can be extracted on IRIX alongside SGUG-RSE
without conflicts. Each bundle includes all mogrix-built shared library
dependencies and wrapper scripts that set LD_LIBRARYN32_PATH.

Installation model (like Flatpak exports/bin):

    /opt/mogrix-apps/                   # extract bundles here
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

    User setup (once):  PATH=/opt/mogrix-apps/bin:$PATH; export PATH
    Per bundle:         cd /opt/mogrix-apps/nano-7.2-6-irix-bundle && ./install

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
from dataclasses import dataclass, field
from pathlib import Path

from rich.console import Console
from rich.table import Table

console = Console()

STAGING_LIB_DIR = Path("/opt/sgug-staging/usr/sgug/lib32")

# Wrapper templates use only Bourne shell syntax (no $(...), no ${var:+...})
# because IRIX /bin/sh is the original Bourne shell, not POSIX sh.
#
# {terminfo_block} is replaced with TERMINFO export if bundle has terminfo data,
# or empty string if not.
WRAPPER_TEMPLATE = """\
#!/bin/sh
dir=`dirname "$0"`
case "$dir" in
    /*) ;;
    *)  dir="`pwd`/$dir" ;;
esac
if [ -n "$LD_LIBRARYN32_PATH" ]; then
    LD_LIBRARYN32_PATH="$dir/_lib32:$LD_LIBRARYN32_PATH"
else
    LD_LIBRARYN32_PATH="$dir/_lib32"
fi
export LD_LIBRARYN32_PATH
{terminfo_block}exec "$dir/_bin/{binary}" "$@"
"""

SBIN_WRAPPER_TEMPLATE = """\
#!/bin/sh
dir=`dirname "$0"`
case "$dir" in
    /*) ;;
    *)  dir="`pwd`/$dir" ;;
esac
if [ -n "$LD_LIBRARYN32_PATH" ]; then
    LD_LIBRARYN32_PATH="$dir/_lib32:$LD_LIBRARYN32_PATH"
else
    LD_LIBRARYN32_PATH="$dir/_lib32"
fi
export LD_LIBRARYN32_PATH
{terminfo_block}exec "$dir/_sbin/{binary}" "$@"
"""

TERMINFO_BLOCK = """\
TERMINFO="$dir/share/terminfo"
export TERMINFO
"""

# Install script: creates trampoline scripts in ../bin/ (Bourne shell compatible).
# Trampolines resolve their own location at runtime via dirname "$0" and use a
# relative path (../<bundle-name>/<cmd>) to reach the real wrapper. This works
# regardless of where the bundle is installed (chroot, different prefix, etc).
INSTALL_TEMPLATE = """\
#!/bin/sh
# Install {package} — create command trampolines in ../bin/
dir=`dirname "$0"`
case "$dir" in
    /*) ;;
    *)  dir="`pwd`/$dir" ;;
esac
dir=`cd "$dir" && pwd`
bundle=`basename "$dir"`
bindir=`dirname "$dir"`/bin
mkdir -p "$bindir"
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
# Uninstall {package} — remove command trampolines from ../bin/
dir=`dirname "$0"`
case "$dir" in
    /*) ;;
    *)  dir="`pwd`/$dir" ;;
esac
dir=`cd "$dir" && pwd`
bindir=`dirname "$dir"`/bin
echo "Removing {package} commands from $bindir"
{unlink_commands}
echo "Done."
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
    bundle_dir: Path | None = None
    tarball_path: Path | None = None


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

    def _prune_unused_libs(self, bundle_dir: Path) -> None:
        """Remove .so files from _lib32/ not NEEDED by any binary in the bundle.

        RPM extraction pulls in all libs from a package (e.g., ncurses-libs has
        14 .so files) but the bundle may only need a few. Scan all ELFs in _bin/
        and _sbin/ to find actual NEEDED sonames, then remove everything else.
        """
        lib_dir = bundle_dir / "_lib32"
        if not lib_dir.is_dir():
            return

        # Collect all NEEDED sonames from binaries
        needed = set()
        for bin_subdir in ("_bin", "_sbin"):
            d = bundle_dir / bin_subdir
            if not d.is_dir():
                continue
            for f in d.iterdir():
                if self._is_elf(f) or (f.is_symlink() and self._is_elf(f.resolve())):
                    needed.update(self._readelf_needed(f))

        # Also check libs themselves — they may depend on other libs
        # (transitive closure)
        checked = set()
        queue = list(needed)
        while queue:
            soname = queue.pop()
            if soname in checked:
                continue
            checked.add(soname)
            lib_file = lib_dir / soname
            if lib_file.exists() and not lib_file.is_symlink() and self._is_elf(lib_file):
                for dep in self._readelf_needed(lib_file):
                    if dep not in checked:
                        queue.append(dep)
            elif lib_file.is_symlink():
                target = lib_file.resolve()
                if target.exists() and self._is_elf(target):
                    for dep in self._readelf_needed(target):
                        if dep not in checked:
                            queue.append(dep)

        needed = checked

        # Remove .so files not in the needed set (keep symlinks whose targets
        # are needed, and real files whose names are needed)
        removed = []
        for f in sorted(lib_dir.iterdir()):
            if ".so" not in f.name:
                continue
            # Keep if this filename is needed
            if f.name in needed:
                continue
            # Keep if this is a symlink target of a needed soname
            is_target = False
            for n in needed:
                link = lib_dir / n
                if link.is_symlink() and link.resolve().name == f.name:
                    is_target = True
                    break
            if is_target:
                continue
            f.unlink()
            removed.append(f.name)

        if removed:
            console.print(
                f"  [dim]Pruned {len(removed)} unused libs from _lib32/[/dim]"
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
        # Find target RPMs
        if target_package not in self._name_to_rpms:
            console.print(
                f"[red]Package '{target_package}' not found in {self.rpms_dir}[/red]"
            )
            raise SystemExit(1)

        target_rpms = self._name_to_rpms[target_package]
        target_version = self._rpm_query(target_rpms[0], "%{VERSION}-%{RELEASE}")

        # Collect initial RPMs: target + siblings + extras
        queue: list[Path] = []
        for rpm in target_rpms:
            queue.extend(self._get_sibling_rpms(rpm))

        for extra in extra_packages or []:
            if extra in self._name_to_rpms:
                for rpm in self._name_to_rpms[extra]:
                    queue.extend(self._get_sibling_rpms(rpm))
            else:
                console.print(
                    f"[yellow]Warning: extra package '{extra}' not found[/yellow]"
                )

        # Deduplicate initial queue
        queue = list(dict.fromkeys(queue))

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
                    if soname in self._irix_sonames:
                        manifest.irix_sonames.add(soname)
                        continue
                    if soname in self._soname_to_rpm:
                        dep_rpm = self._soname_to_rpm[soname]
                        if dep_rpm not in visited_rpms:
                            # Add this RPM and its siblings
                            for sibling in self._get_sibling_rpms(dep_rpm):
                                if sibling not in visited_rpms:
                                    queue.append(sibling)
                        continue
                    # Staging fallback
                    if STAGING_LIB_DIR.is_dir():
                        staging_matches = list(STAGING_LIB_DIR.glob(soname))
                        if staging_matches:
                            manifest.staging_sonames.add(soname)
                            continue
                    manifest.unresolved_sonames.add(soname)

        manifest.included_rpms = sorted(visited_rpms)
        return manifest

    def create_bundle(
        self,
        target_package: str,
        output_dir: Path,
        extra_packages: list[str] | None = None,
        create_tarball: bool = True,
    ) -> BundleManifest:
        """Create a self-contained app bundle."""
        console.print(f"\n[bold]Resolving dependencies for: {target_package}[/bold]\n")

        manifest = self.resolve_deps(target_package, extra_packages)

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

        # Create bundle directory
        bundle_name = (
            f"{manifest.target_package}-{manifest.target_version}-irix-bundle"
        )
        output_dir.mkdir(parents=True, exist_ok=True)
        bundle_dir = output_dir / bundle_name

        # Clean up previous bundle if it exists
        if bundle_dir.exists():
            shutil.rmtree(bundle_dir)
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

            # Move usr/sgug/* to bundle root with internal prefixes
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

        # Prune _lib32/ to only sonames actually NEEDED by bundle binaries.
        # RPMs include all libs from the package (e.g., ncurses-libs has 14 .so
        # files) but the bundle may only need a few.
        self._prune_unused_libs(bundle_dir)

        # Strip runtime-unnecessary data directories
        for strip_dir in ("doc", "man", "info", "licenses"):
            d = bundle_dir / "share" / strip_dir
            if d.is_dir():
                shutil.rmtree(d)

        # Trim terminfo to common terminals (full set is ~12MB)
        self._trim_terminfo(bundle_dir)

        # Detect if bundle has terminfo data (for TERMINFO env var in wrappers)
        has_terminfo = (bundle_dir / "share" / "terminfo").is_dir()
        terminfo_block = TERMINFO_BLOCK if has_terminfo else ""

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
        for binary in binaries:
            wrapper_path = bundle_dir / binary
            wrapper_path.write_text(
                WRAPPER_TEMPLATE.format(
                    binary=binary, terminfo_block=terminfo_block
                )
            )
            wrapper_path.chmod(0o755)

        for binary in sbin_binaries:
            wrapper_path = bundle_dir / binary
            wrapper_path.write_text(
                SBIN_WRAPPER_TEMPLATE.format(
                    binary=binary, terminfo_block=terminfo_block
                )
            )
            wrapper_path.chmod(0o755)

        # Generate install/uninstall scripts
        all_cmds = binaries + sbin_binaries
        self._generate_install_scripts(
            manifest.target_package, bundle_name, all_cmds, bundle_dir
        )

        # Generate README
        self._generate_readme(manifest, bundle_dir)

        manifest.bundle_dir = bundle_dir

        # Create tarball
        if create_tarball:
            tarball_name = f"{bundle_name}.tar.gz"
            tarball_path = output_dir / tarball_name
            console.print(f"\n[dim]Creating tarball: {tarball_name}[/dim]")
            subprocess.run(
                [
                    "tar",
                    "czf",
                    str(tarball_path),
                    "-C",
                    str(output_dir),
                    bundle_name,
                ],
                check=True,
            )
            manifest.tarball_path = tarball_path

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
            trampoline_lines.append(
                f'echo \'#!/bin/sh\' > "$bindir/{cmd}"'
            )
            trampoline_lines.append(
                f'echo \'dir=`dirname "$0"`\' >> "$bindir/{cmd}"'
            )
            trampoline_lines.append(
                f'echo \'case "$dir" in /*) ;; *) dir="`pwd`/$dir" ;; esac\' >> "$bindir/{cmd}"'
            )
            trampoline_lines.append(
                f'echo "exec \\"\\$dir/../$bundle/{cmd}\\" \\"\\$@\\"" >> "$bindir/{cmd}"'
            )
            trampoline_lines.append(
                f'chmod 755 "$bindir/{cmd}" && echo "  {cmd}"'
            )
            unlink_lines.append(
                f'rm -f "$bindir/{cmd}" && echo "  {cmd}"'
            )

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
        bundle_name = (
            f"{manifest.target_package}-{manifest.target_version}-irix-bundle"
        )
        primary = (
            manifest.target_package
            if manifest.target_package
            in [os.path.basename(b) for b in manifest.binaries]
            else (manifest.binaries[0] if manifest.binaries else "BINARY")
        )
        lines = [
            f"# {manifest.target_package} {manifest.target_version} — IRIX App Bundle",
            "",
            "Self-contained app bundle for IRIX 6.5 (MIPS n32).",
            "Generated by mogrix (https://github.com/unxmaal/mogrix).",
            "",
            "## Install",
            "",
            f"    tar xzf {bundle_name}.tar.gz -C /opt/mogrix-apps/",
            f"    cd /opt/mogrix-apps/{bundle_name}",
            "    ./install",
            "",
            "Then add /opt/mogrix-apps/bin to your PATH (once, in ~/.profile):",
            "",
            "    PATH=/opt/mogrix-apps/bin:$PATH; export PATH",
            "",
            f"Now just run: {primary}",
            "",
            "## Uninstall",
            "",
            f"    cd /opt/mogrix-apps/{bundle_name}",
            "    ./uninstall",
            f"    rm -rf /opt/mogrix-apps/{bundle_name}",
            "",
            "## Available Commands",
            "",
        ]
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

    def _print_summary(self, manifest: BundleManifest) -> None:
        """Print a Rich summary table."""
        console.print()

        table = Table(title=f"Bundle: {manifest.target_package}")
        table.add_column("Category", style="bold")
        table.add_column("Details")

        table.add_row("Version", manifest.target_version)
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
            console.print(
                "\n[yellow]Bundle includes non-mogrix libs from staging. "
                "User may need SGUG-RSE installed for these.[/yellow]"
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
