"""Upstream source fetching and spec generation for non-Fedora packages.

Handles packages sourced from git repos or tarball URLs rather than
Fedora SRPMs. Generates spec files from templates and creates SRPMs
that enter the normal mogrix convert → build pipeline.
"""

import subprocess
import tempfile
import urllib.request
from pathlib import Path
from string import Template

from rich.console import Console

console = Console()

# Project root (parent of mogrix/ package dir)
MOGRIX_ROOT = Path(__file__).resolve().parent.parent
SPECS_DIR = MOGRIX_ROOT / "specs"
TEMPLATES_DIR = SPECS_DIR / "templates"
PACKAGES_DIR = SPECS_DIR / "packages"


class UpstreamSource:
    """Fetch upstream sources and generate spec files."""

    def __init__(self, rules_dir: Path | None = None):
        self.rules_dir = rules_dir or MOGRIX_ROOT / "rules"

    def load_upstream_config(self, package_name: str) -> dict:
        """Load upstream: block from a package's rules YAML.

        Returns the upstream dict or raises if not found.
        """
        import yaml

        rule_path = self.rules_dir / "packages" / f"{package_name}.yaml"
        if not rule_path.exists():
            raise RuntimeError(
                f"No rules file for '{package_name}' at {rule_path}"
            )

        with open(rule_path) as f:
            data = yaml.safe_load(f)

        if not data or "upstream" not in data:
            raise RuntimeError(
                f"Package '{package_name}' has no upstream: block in {rule_path}"
            )

        config = data["upstream"]
        config["name"] = package_name
        return config

    def fetch_source(self, config: dict, work_dir: Path) -> Path:
        """Fetch source from git or tarball URL. Returns path to tarball."""
        url = config["url"]
        name = config["name"]
        version = config["version"]
        source_type = config.get("type", self._infer_type(url))

        tarball_name = f"{name}-{version}.tar.gz"
        tarball_path = work_dir / tarball_name

        if source_type == "tarball":
            self._fetch_tarball(url, tarball_path)
        else:
            ref = config.get("ref", version)
            self._fetch_git(url, ref, name, version, tarball_path)

        return tarball_path

    def render_spec(self, config: dict) -> str:
        """Generate spec content from template or hand-written override."""
        name = config["name"]

        # Check for hand-written spec override
        override = PACKAGES_DIR / f"{name}.spec"
        if override.exists():
            console.print(f"  [dim]Using hand-written spec:[/dim] {override}")
            return override.read_text()

        # Use template
        build_system = config["build_system"]
        template_path = TEMPLATES_DIR / f"{build_system}.spec"
        if not template_path.exists():
            raise RuntimeError(
                f"No spec template for build_system '{build_system}' "
                f"at {template_path}"
            )

        template = Template(template_path.read_text())
        version = config["version"]
        source_filename = f"{name}-{version}.tar.gz"

        spec_content = template.safe_substitute(
            name=name,
            version=version,
            summary=config.get("summary", name),
            license=config.get("license", "Unknown"),
            url=config["url"],
            source_filename=source_filename,
            source_dir=config.get("source_dir", f"{name}-{version}"),
        )

        return spec_content

    def _infer_type(self, url: str) -> str:
        """Infer source type from URL."""
        tarball_exts = (
            ".tar.gz", ".tgz", ".tar.bz2", ".tar.xz", ".tar", ".zip",
        )
        if any(url.endswith(ext) for ext in tarball_exts):
            return "tarball"
        return "git"

    def _fetch_tarball(self, url: str, dest: Path) -> None:
        """Download a source tarball."""
        console.print(f"  [dim]Downloading:[/dim] {url}")
        urllib.request.urlretrieve(url, str(dest))
        console.print(f"  [green]Downloaded:[/green] {dest.name}")

    def _is_commit_sha(self, ref: str) -> bool:
        """Check if ref looks like a full commit SHA."""
        return len(ref) >= 40 and all(c in "0123456789abcdef" for c in ref)

    def _fetch_git(
        self, url: str, ref: str, name: str, version: str, dest: Path
    ) -> None:
        """Clone a git repo and create a tarball."""
        with tempfile.TemporaryDirectory(prefix="mogrix-git-") as tmpdir:
            clone_dir = Path(tmpdir) / name

            console.print(f"  [dim]Cloning:[/dim] {url} (ref: {ref})")

            if self._is_commit_sha(ref):
                # Full SHA: clone then checkout (--branch doesn't work with SHAs)
                result = subprocess.run(
                    ["git", "clone", url, str(clone_dir)],
                    capture_output=True,
                    text=True,
                )
                if result.returncode != 0:
                    raise RuntimeError(
                        f"git clone failed: {result.stderr.strip()}"
                    )
                result = subprocess.run(
                    ["git", "-C", str(clone_dir), "checkout", ref],
                    capture_output=True,
                    text=True,
                )
                if result.returncode != 0:
                    raise RuntimeError(
                        f"git checkout failed: {result.stderr.strip()}"
                    )
            else:
                # Branch/tag: shallow clone
                result = subprocess.run(
                    [
                        "git", "clone", "--depth", "1",
                        "--branch", ref, url, str(clone_dir),
                    ],
                    capture_output=True,
                    text=True,
                )
                if result.returncode != 0:
                    raise RuntimeError(
                        f"git clone failed: {result.stderr.strip()}"
                    )

            # Create tarball with proper prefix
            prefix = f"{name}-{version}/"
            console.print(f"  [dim]Creating tarball:[/dim] {dest.name}")
            result = subprocess.run(
                [
                    "git", "-C", str(clone_dir),
                    "archive", "--format=tar.gz",
                    f"--prefix={prefix}",
                    "-o", str(dest),
                    "HEAD",
                ],
                capture_output=True,
                text=True,
            )
            if result.returncode != 0:
                raise RuntimeError(
                    f"git archive failed: {result.stderr.strip()}"
                )

            console.print(f"  [green]Created:[/green] {dest.name}")
