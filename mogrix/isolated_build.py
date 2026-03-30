"""Per-build staging isolation via OverlayFS.

Each `mogrix build --cross` gets a clean staging sysroot containing ONLY the
base toolchain + declared build dependencies. No leaking, no accumulation.

How it works:
  1. <project_root>/staging/ is the read-only base layer (toolchain, rpmmacros, base libs)
  2. Declared BuildRequires are extracted into a per-build upper dir
  3. unshare --mount creates a private mount namespace with overlayfs
  4. rpmbuild sees base + declared deps, nothing else
  5. After build, namespace exits and tmpdir is cleaned up
"""

import logging
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

from rich.console import Console

from mogrix.deps.resolver import DependencyResolver

console = Console()
log = logging.getLogger(__name__)

# System/host packages that don't need staging (they're build tools, not target libs)
SYSTEM_PACKAGES = {
    "gcc", "gcc-c++", "make", "autoconf", "automake", "libtool",
    "pkgconfig", "pkg-config", "cmake", "ninja-build", "meson",
    "bison", "flex", "gettext", "perl", "python3", "sed", "gawk",
    "findutils", "coreutils", "diffutils", "grep", "gzip", "tar",
    "texinfo", "help2man", "xmlto", "asciidoc", "docbook-utils",
    "systemd", "systemd-devel", "systemd-rpm-macros",
    "kernel-headers", "glibc-devel", "glibc-headers",
}


class IsolatedStaging:
    """Manages per-build staging isolation via overlayfs."""

    def __init__(self, base_staging: Path, rpms_dir: Path, rules_dir: Path):
        self.base_staging = base_staging       # <project_root>/staging
        self.rpms_dir = rpms_dir               # ~/mogrix_outputs/RPMS
        self.rules_dir = rules_dir
        self.resolver = DependencyResolver(rules_dir)
        self._overlay_available: bool | None = None

    def check_overlay_available(self) -> bool:
        """Check if overlayfs is available on this system."""
        if self._overlay_available is not None:
            return self._overlay_available

        try:
            result = subprocess.run(
                ["modprobe", "-n", "--first-time", "overlay"],
                capture_output=True, text=True,
            )
            # modprobe -n --first-time returns 1 if already loaded (good) or if unavailable
            # Check /proc/filesystems for actual availability
            proc_fs = Path("/proc/filesystems")
            if proc_fs.exists():
                content = proc_fs.read_text()
                self._overlay_available = "overlay" in content
            else:
                self._overlay_available = False
        except Exception:
            self._overlay_available = False

        return self._overlay_available

    def resolve_build_deps(self, spec_or_srpm: Path) -> list[str]:
        """Parse BuildRequires from a spec/SRPM, return base package names to stage.

        Filters out system/host packages and maps dep names to base package names
        (e.g., "ncurses-devel" -> "ncurses").
        """
        buildrequires = self._extract_buildrequires(spec_or_srpm)
        if not buildrequires:
            return []

        # Map to base package names, filter system deps
        base_packages = set()
        for dep in buildrequires:
            # Strip %{_isa} and similar rpm macros
            dep = re.sub(r"%\{[^}]+\}", "", dep).strip()
            if not dep:
                continue

            # Skip virtual provides like perl(Module), pkgconfig(foo), /usr/bin/foo
            if "(" in dep or dep.startswith("/"):
                continue

            # Skip system/host packages
            if dep in SYSTEM_PACKAGES:
                continue

            # Map to base package name
            pkg = self.resolver.get_package_for_dep(dep)
            if pkg:
                base_packages.add(pkg)
            else:
                # Try stripping common suffixes ourselves
                for suffix in ["-devel", "-libs", "-static", "-doc"]:
                    if dep.endswith(suffix):
                        base_packages.add(dep[: -len(suffix)])
                        break
                else:
                    base_packages.add(dep)

        return sorted(base_packages)

    def _extract_buildrequires(self, spec_or_srpm: Path) -> list[str]:
        """Extract BuildRequires from a spec file or SRPM."""
        if spec_or_srpm.suffix == ".spec":
            return self._parse_spec_buildrequires(spec_or_srpm)
        elif spec_or_srpm.name.endswith(".src.rpm"):
            return self._parse_srpm_buildrequires(spec_or_srpm)
        return []

    def _parse_spec_buildrequires(self, spec_path: Path) -> list[str]:
        """Parse BuildRequires from a spec file."""
        from mogrix.parser.spec import SpecParser
        parser = SpecParser()
        spec = parser.parse(spec_path)
        return spec.buildrequires

    def _parse_srpm_buildrequires(self, srpm_path: Path) -> list[str]:
        """Extract BuildRequires from an SRPM using rpm -qp."""
        try:
            result = subprocess.run(
                ["rpm", "-qp", "--requires", str(srpm_path)],
                capture_output=True, text=True,
            )
            if result.returncode != 0:
                log.warning("Failed to query SRPM requires: %s", result.stderr)
                return []

            deps = []
            for line in result.stdout.splitlines():
                line = line.strip()
                if not line or line.startswith("rpmlib("):
                    continue
                # Strip version constraints
                dep = re.split(r"\s*[<>=]", line)[0].strip()
                if dep:
                    deps.append(dep)
            return deps
        except Exception as e:
            log.warning("Failed to parse SRPM: %s", e)
            return []

    def find_rpms_for_dep(self, base_name: str) -> list[Path]:
        """Find all RPMs (main + devel + libs) for a base package in RPMS dir.

        Searches ~/mogrix_outputs/RPMS/ for RPMs matching the base package name.
        """
        if not self.rpms_dir.exists():
            return []

        found = []
        # Match: baseName-version-release.arch.rpm
        # But NOT: baseName-something-else-version... (different package)
        for rpm in self.rpms_dir.glob("*.rpm"):
            rpm_name = rpm.name
            # Skip source RPMs
            if rpm_name.endswith(".src.rpm"):
                continue

            # Parse: name-version-release.arch.rpm
            # We want RPMs where the name IS base_name, or base_name-devel, base_name-libs, etc.
            match = re.match(r"^(.+?)-(\d+[\d.]*(?:[-~]\w[\w.]*)*)\.\w+\.rpm$", rpm_name)
            if match:
                pkg_name = match.group(1)
                if pkg_name == base_name or pkg_name.startswith(f"{base_name}-"):
                    # Verify this is actually a sub-package, not an unrelated package
                    # e.g., "lib" matches "lib" and "lib-devel" but not "libxml2"
                    suffix = pkg_name[len(base_name):]
                    if not suffix or suffix.startswith("-"):
                        found.append(rpm)

        return found

    def stage_deps_to_dir(self, deps: list[str], target_dir: Path) -> list[str]:
        """Extract RPMs for each dep into target_dir.

        Returns list of package names that were successfully staged.
        """
        staged = []
        missing = []

        for dep in deps:
            rpms = self.find_rpms_for_dep(dep)
            if not rpms:
                missing.append(dep)
                continue

            for rpm in rpms:
                result = subprocess.run(
                    f"cd {target_dir} && rpm2cpio {rpm.absolute()} | cpio -idm 2>/dev/null",
                    shell=True, capture_output=True, text=True,
                )
                if result.returncode != 0:
                    log.warning("Failed to extract %s: %s", rpm.name, result.stderr)
                    continue

            staged.append(dep)

        if missing:
            console.print(f"[yellow]Warning: No RPMs found for deps:[/yellow] {', '.join(missing)}")
            console.print("[dim]Build these packages first, then rebuild.[/dim]")

        return staged

    def build_with_isolation(
        self, cmd: list[str], deps: list[str]
    ) -> subprocess.CompletedProcess:
        """Run rpmbuild inside a mount namespace with overlayfs staging.

        Args:
            cmd: The rpmbuild command to execute
            deps: Base package names to stage as build deps

        Returns:
            CompletedProcess from the rpmbuild execution
        """
        tmpdir = None
        try:
            tmpdir = Path(tempfile.mkdtemp(prefix="mogrix-overlay-"))
            upper = tmpdir / "upper"
            work = tmpdir / "work"
            upper.mkdir()
            work.mkdir()

            # Stage declared deps into upper dir
            if deps:
                staged = self.stage_deps_to_dir(deps, upper)
                if staged:
                    console.print(
                        f"[bold green]Staged {len(staged)} deps into overlay:[/bold green] "
                        f"{', '.join(staged)}"
                    )

            # Build the unshare + overlayfs command
            base = str(self.base_staging)
            overlay_cmd = (
                f"mount -t overlay overlay "
                f"-o lowerdir={base},upperdir={upper},workdir={work} "
                f"{base} && exec {' '.join(cmd)}"
            )

            full_cmd = ["unshare", "--mount", "sh", "-c", overlay_cmd]

            console.print("[bold]Running build in isolated mount namespace[/bold]")
            result = subprocess.run(full_cmd, capture_output=True, text=True)

            # Detect unshare/mount failures vs actual rpmbuild failures
            if result.returncode != 0 and result.stderr:
                first_line = result.stderr.strip().splitlines()[0].lower()
                if "unshare" in first_line or "mount" in first_line:
                    raise PermissionError(
                        f"Namespace setup failed: {result.stderr.strip().splitlines()[0]}"
                    )

            return result

        finally:
            if tmpdir and tmpdir.exists():
                shutil.rmtree(tmpdir, ignore_errors=True)

    def build_with_fallback(
        self, cmd: list[str], deps: list[str]
    ) -> subprocess.CompletedProcess:
        """Try isolated build; fall back to direct execution if overlay unavailable.

        This is the main entry point for builds.
        """
        if self.check_overlay_available():
            try:
                return self.build_with_isolation(cmd, deps)
            except PermissionError:
                console.print(
                    "[yellow]Warning: Insufficient permissions for mount namespace. "
                    "Running without isolation.[/yellow]"
                )
            except Exception as e:
                console.print(
                    f"[yellow]Warning: Overlay setup failed ({e}). "
                    f"Running without isolation.[/yellow]"
                )

        # Fallback: check deps in current staging, warn about missing ones
        self._check_deps_in_staging(deps)
        return subprocess.run(cmd, capture_output=True, text=True)

    def _check_deps_in_staging(self, deps: list[str]) -> None:
        """Check which deps are present in current staging (fallback mode)."""
        if not deps:
            return

        console.print(
            "[yellow]OverlayFS not available — checking deps in current staging[/yellow]"
        )

        lib_dir = self.base_staging / "opt" / "mogrix" / "lib32"
        include_dir = self.base_staging / "opt" / "mogrix" / "include"

        missing = []
        for dep in deps:
            # Skip deps with special characters that can't be filesystem-checked
            if not re.match(r"^[a-zA-Z0-9_\-]+$", dep):
                continue

            # Simple heuristic: check for lib*.so or include dir/header
            has_lib = any(lib_dir.glob(f"lib{dep}*")) if lib_dir.exists() else False
            has_header = (
                (include_dir / dep).exists()
                or any(include_dir.glob(f"{dep}*.h"))
            ) if include_dir.exists() else False

            if not has_lib and not has_header:
                missing.append(dep)

        if missing:
            console.print(
                f"[bold yellow]Missing from staging:[/bold yellow] {', '.join(missing)}"
            )
            console.print(
                "[dim]Build and stage these packages first, or use --no-isolate to skip checks.[/dim]"
            )
        else:
            console.print("[green]All declared deps found in staging[/green]")
