"""Fedora repository access for fetching SRPMs."""

import re
import urllib.request
from pathlib import Path
from dataclasses import dataclass


@dataclass
class SRPMInfo:
    """Information about an available SRPM."""
    name: str
    version: str
    release: str
    filename: str
    url: str

    @classmethod
    def from_filename(cls, filename: str, base_url: str) -> "SRPMInfo":
        """Parse SRPM info from filename.

        Expected format: name-version-release.src.rpm
        e.g., popt-1.19-6.fc40.src.rpm
        """
        # Remove .src.rpm suffix
        base = filename.replace(".src.rpm", "")

        # Split on last two hyphens to get name-version-release
        # This handles package names with hyphens like "zlib-ng"
        parts = base.rsplit("-", 2)
        if len(parts) >= 3:
            name = parts[0]
            version = parts[1]
            release = parts[2]
        elif len(parts) == 2:
            name = parts[0]
            version = parts[1]
            release = ""
        else:
            name = base
            version = ""
            release = ""

        return cls(
            name=name,
            version=version,
            release=release,
            filename=filename,
            url=f"{base_url}{filename}",
        )


class FedoraRepo:
    """Access Fedora and other SRPM repositories."""

    # Fedora archive base URL
    ARCHIVE_BASE = "https://archives.fedoraproject.org/pub/archive/fedora/linux/releases"

    # Current Fedora base URL (for newer releases)
    CURRENT_BASE = "https://download.fedoraproject.org/pub/fedora/linux/releases"

    # Known repo presets
    PRESETS = {
        "photon5": "https://packages.vmware.com/photon/5.0/photon_srpms_5.0_x86_64/",
        "photon4": "https://packages.vmware.com/photon/4.0/photon_srpms_4.0_x86_64/",
        "photon3": "https://packages.vmware.com/photon/3.0/photon_srpms_3.0_x86_64/",
    }

    def __init__(self, release: str = "40", base_url: str | None = None):
        """Initialize with Fedora release version or custom URL.

        Args:
            release: Fedora release version (default: "40")
            base_url: Custom base URL or preset name (e.g., "photon5").
                      If URL ends with /, treated as flat directory.
                      Otherwise, uses Fedora-style structure.
        """
        self.release = release
        self._base_url = base_url
        self._is_flat = False

        # Check for preset names
        if base_url and base_url.lower() in self.PRESETS:
            self._base_url = self.PRESETS[base_url.lower()]
            self._is_flat = True
        elif base_url:
            # Treat URLs ending with / as flat directories
            self._is_flat = base_url.endswith("/")

    def _get_base_url(self) -> str:
        """Get the base URL for SRPM downloads."""
        if self._base_url:
            return self._base_url.rstrip("/")
        # Use archive for FC40 and earlier
        return self.ARCHIVE_BASE

    def _get_package_list_url(self, first_letter: str) -> str:
        """Get URL for the package listing directory."""
        base = self._get_base_url()

        if self._is_flat:
            # Flat directory structure (e.g., Photon OS)
            return base + "/"
        else:
            # Fedora-style structure with first-letter subdirectories
            return f"{base}/{self.release}/Everything/source/tree/Packages/{first_letter}/"

    def search_packages(self, search_term: str, fuzzy: bool = True) -> list[SRPMInfo]:
        """Search for SRPMs matching a search term.

        Args:
            search_term: Package name or partial name to search for
            fuzzy: If True, also return substring matches when no exact matches

        Returns:
            List of matching SRPMInfo objects
        """
        first_letter = search_term[0].lower()
        url = self._get_package_list_url(first_letter)

        try:
            with urllib.request.urlopen(url, timeout=15) as response:
                html = response.read().decode("utf-8")
        except Exception as e:
            raise RuntimeError(f"Failed to fetch package list: {e}")

        # Find all SRPM files
        pattern = r'href="([^"]+\.src\.rpm)"'
        all_srpms = re.findall(pattern, html)

        # Filter to those matching the search term
        exact_matches = []
        fuzzy_matches = []
        search_lower = search_term.lower()

        for filename in all_srpms:
            filename_lower = filename.lower()

            # Exact prefix match (package name starts with search term)
            if filename_lower.startswith(search_lower + "-") or \
               filename_lower.startswith(search_lower + "_"):
                info = SRPMInfo.from_filename(filename, url)
                exact_matches.append(info)
            # Fuzzy match (search term appears anywhere in filename)
            elif fuzzy and search_lower in filename_lower:
                info = SRPMInfo.from_filename(filename, url)
                fuzzy_matches.append(info)

        # Prefer exact matches, fall back to fuzzy
        matches = exact_matches if exact_matches else fuzzy_matches

        # Sort by name, then version
        matches.sort(key=lambda x: (x.name, x.version))

        return matches

    def get_srpm_url(self, package_name: str) -> str | None:
        """Get the URL for a package's SRPM (exact match).

        Args:
            package_name: Package name (e.g., "popt", "zlib")

        Returns:
            URL to the SRPM or None if not found
        """
        matches = self.search_packages(package_name)

        # Filter to exact package name matches
        exact = [m for m in matches if m.name == package_name]

        if exact:
            # Return the first exact match
            return exact[0].url
        elif matches:
            # Return first partial match if no exact
            return matches[0].url

        return None

    def download_srpm(self, url: str, output_dir: Path) -> Path:
        """Download an SRPM to a directory.

        Args:
            url: URL to the SRPM
            output_dir: Directory to save the file

        Returns:
            Path to the downloaded file
        """
        filename = url.split("/")[-1]
        output_path = output_dir / filename

        with urllib.request.urlopen(url, timeout=120) as response:
            total_size = response.headers.get("Content-Length")
            if total_size:
                total_size = int(total_size)

            # Download with progress indication
            data = response.read()
            output_path.write_bytes(data)

        return output_path

    def find_srpms_for_deps(self, dep_names: list[str]) -> dict[str, str | None]:
        """Find SRPM URLs for a list of dependencies.

        Args:
            dep_names: List of dependency names

        Returns:
            Dict mapping dep name to SRPM URL (or None if not found)
        """
        results = {}

        for dep in dep_names:
            # Strip common suffixes to get base package name
            pkg_name = dep
            for suffix in ["-devel", "-libs", "-static", "-doc"]:
                if dep.endswith(suffix):
                    pkg_name = dep[:-len(suffix)]
                    break

            results[dep] = self.get_srpm_url(pkg_name)

        return results
