"""Fedora repository metadata cache and index for dependency resolution.

Downloads Fedora repo metadata (pre-built sqlite databases from repomd.xml),
merges them into a unified local sqlite index for fast BuildRequires/Provides
lookups.

Used by `mogrix roadmap` to resolve transitive build dependency graphs.
"""

import bz2
import gzip
import re
import shutil
import sqlite3
import tempfile
import time
import urllib.request
from pathlib import Path
from xml.etree import ElementTree as ET

from rich.console import Console

console = Console(stderr=True)

# Namespace used in repomd.xml
REPO_NS = "http://linux.duke.edu/metadata/repo"

# Default Fedora mirror base URLs
DEFAULT_BASES = {
    "releases": "https://archives.fedoraproject.org/pub/archive/fedora/linux/releases",
    "updates": "https://archives.fedoraproject.org/pub/archive/fedora/linux/updates",
}

# SRPM name extraction regex: name-version-release.src.rpm -> name
SRPM_NAME_RE = re.compile(r"^(.+)-[^-]+-[^-]+\.src\.rpm$")


def extract_srpm_name(sourcerpm: str) -> str:
    """Extract source package name from a sourcerpm string.

    Example: 'glib2-2.80.0-1.fc40.src.rpm' -> 'glib2'
    """
    m = SRPM_NAME_RE.match(sourcerpm)
    if m:
        return m.group(1)
    # Fallback: strip .src.rpm and take everything before last two hyphens
    base = sourcerpm.replace(".src.rpm", "")
    parts = base.rsplit("-", 2)
    return parts[0] if parts else sourcerpm


class RepoMetaCache:
    """Downloads and caches Fedora repo metadata as a unified sqlite index.

    The index merges data from releases and updates repos (both source and
    binary) into three tables:
    - binary_provides: what each binary RPM provides (name -> source package)
    - source_buildrequires: what each source package needs to build
    - file_provides: which packages own which files (for /usr/bin/foo deps)
    """

    # Repo configurations: (repo_key, arch, data_type)
    REPOS = [
        ("releases", "source", "source"),
        ("updates", "source", "source"),
        ("releases", "x86_64", "binary"),
        ("updates", "x86_64", "binary"),
    ]

    def __init__(
        self,
        release: str = "40",
        base_url: str | None = None,
        cache_dir: Path | None = None,
    ):
        self.release = release
        self._base_url = base_url
        self.cache_dir = cache_dir or Path.home() / ".cache" / "mogrix" / "repometa" / f"fc{release}"

    def _get_base_urls(self) -> dict[str, str]:
        """Get base URLs for releases and updates repos."""
        if self._base_url:
            return {"releases": self._base_url, "updates": self._base_url}
        return dict(DEFAULT_BASES)

    def _get_repomd_url(self, repo_key: str, arch: str) -> str:
        """Build the repomd.xml URL for a given repo and arch."""
        bases = self._get_base_urls()
        base = bases[repo_key]
        if arch == "source":
            return f"{base}/{self.release}/Everything/source/tree/repodata/repomd.xml"
        else:
            return f"{base}/{self.release}/Everything/{arch}/os/repodata/repomd.xml"

    def _get_repodata_base_url(self, repo_key: str, arch: str) -> str:
        """Get the base URL for repodata files (directory containing repodata/)."""
        bases = self._get_base_urls()
        base = bases[repo_key]
        if arch == "source":
            return f"{base}/{self.release}/Everything/source/tree/"
        else:
            return f"{base}/{self.release}/Everything/{arch}/os/"

    def ensure_index(self, refresh: bool = False) -> sqlite3.Connection:
        """Ensure the unified sqlite index exists. Download and build if needed.

        Args:
            refresh: Force re-download and rebuild even if cache exists.

        Returns:
            sqlite3.Connection to the unified index.
        """
        self.cache_dir.mkdir(parents=True, exist_ok=True)
        index_path = self.cache_dir / "index.sqlite"

        if index_path.exists() and not refresh:
            conn = sqlite3.connect(str(index_path))
            conn.row_factory = sqlite3.Row
            return conn

        if refresh and index_path.exists():
            index_path.unlink()

        # Download all repo metadata
        downloaded = self._download_all_metadata()

        # Build unified index
        conn = self._build_index(index_path, downloaded)
        return conn

    def _download_all_metadata(self) -> dict[str, dict[str, Path]]:
        """Download metadata from all configured repos.

        Returns:
            Dict mapping (repo_key-arch) -> {"primary_db": Path, "filelists_db": Path}
        """
        downloaded = {}
        filelists_downloaded = {}

        for repo_key, arch, data_type in self.REPOS:
            key = f"{repo_key}-{arch}"
            console.print(f"[bold]Fetching metadata:[/bold] {key}")

            try:
                repomd_url = self._get_repomd_url(repo_key, arch)
                repodata_base = self._get_repodata_base_url(repo_key, arch)
                db_urls = self._parse_repomd(repomd_url)
            except Exception as e:
                console.print(f"  [yellow]Warning: Could not fetch {key}: {e}[/yellow]")
                continue

            result = {}

            # Always download primary_db
            if "primary_db" in db_urls:
                href = db_urls["primary_db"]
                url = repodata_base + href
                dest = self.cache_dir / f"{key}-primary.sqlite"
                if not dest.exists():
                    self._download_and_decompress(url, dest, href)
                result["primary_db"] = dest
            else:
                console.print(f"  [yellow]Warning: No primary_db in {key}[/yellow]")

            # Download filelists_db for binary repos (lazy — only binary has file provides)
            if data_type == "binary" and "filelists_db" in db_urls:
                href = db_urls["filelists_db"]
                url = repodata_base + href
                dest = self.cache_dir / f"{key}-filelists.sqlite"
                if not dest.exists():
                    self._download_and_decompress(url, dest, href)
                filelists_downloaded[key] = dest

            downloaded[key] = result

        # Store filelists paths for lazy loading
        self._filelists_paths = filelists_downloaded
        return downloaded

    def _parse_repomd(self, url: str) -> dict[str, str]:
        """Parse repomd.xml and return hrefs for database files.

        Returns:
            Dict mapping data type (e.g., "primary_db") to href string.
        """
        console.print(f"  Parsing repomd.xml...")
        with urllib.request.urlopen(url, timeout=30) as response:
            xml_data = response.read()

        root = ET.fromstring(xml_data)
        result = {}

        for data_elem in root.findall(f"{{{REPO_NS}}}data"):
            data_type = data_elem.get("type", "")
            if data_type in ("primary_db", "filelists_db"):
                location = data_elem.find(f"{{{REPO_NS}}}location")
                if location is not None:
                    href = location.get("href", "")
                    if href:
                        result[data_type] = href

        return result

    def _download_and_decompress(self, url: str, dest: Path, href: str):
        """Download a compressed sqlite database and decompress it.

        Handles both .gz and .bz2 compression (releases use .gz, updates use .bz2).
        """
        console.print(f"  Downloading {dest.name}...")
        with urllib.request.urlopen(url, timeout=300) as response:
            compressed_data = response.read()

        size_mb = len(compressed_data) / (1024 * 1024)
        console.print(f"  Downloaded {size_mb:.1f} MB, decompressing...")

        if href.endswith(".bz2"):
            data = bz2.decompress(compressed_data)
        elif href.endswith(".gz"):
            data = gzip.decompress(compressed_data)
        else:
            data = compressed_data

        dest.write_bytes(data)
        size_mb = len(data) / (1024 * 1024)
        console.print(f"  [green]✓[/green] {dest.name} ({size_mb:.1f} MB)")

    def _build_index(
        self, index_path: Path, downloaded: dict[str, dict[str, Path]]
    ) -> sqlite3.Connection:
        """Build the unified index from downloaded repo sqlite databases.

        Reads Fedora's pre-built sqlite databases and merges them into our
        unified schema.
        """
        console.print(f"\n[bold]Building unified index...[/bold]")

        conn = sqlite3.connect(str(index_path))
        conn.row_factory = sqlite3.Row
        conn.execute("PRAGMA journal_mode=WAL")
        conn.execute("PRAGMA synchronous=NORMAL")

        self._create_schema(conn)

        # Process each downloaded repo
        for key, paths in downloaded.items():
            repo_key = key.split("-")[0]  # "releases" or "updates"
            arch = key.split("-", 1)[1]  # "source" or "x86_64"
            is_source = arch == "source"

            if "primary_db" not in paths:
                continue

            db_path = paths["primary_db"]
            console.print(f"  Importing {key}...")

            if is_source:
                self._import_source_primary(db_path, repo_key, conn)
            else:
                self._import_binary_primary(db_path, repo_key, conn)

        # Import filelists from binary repos
        for key, flist_path in self._filelists_paths.items():
            repo_key = key.split("-")[0]
            console.print(f"  Importing filelists from {key}...")
            self._import_binary_filelists(flist_path, repo_key, conn)

        # Store metadata
        conn.execute(
            "INSERT OR REPLACE INTO meta (key, value) VALUES (?, ?)",
            ("build_time", str(int(time.time()))),
        )
        conn.execute(
            "INSERT OR REPLACE INTO meta (key, value) VALUES (?, ?)",
            ("release", self.release),
        )

        conn.commit()

        # Report stats
        bp_count = conn.execute("SELECT COUNT(*) FROM binary_provides").fetchone()[0]
        sbr_count = conn.execute("SELECT COUNT(*) FROM source_buildrequires").fetchone()[0]
        fp_count = conn.execute("SELECT COUNT(*) FROM file_provides").fetchone()[0]
        console.print(f"\n[green]✓ Index built:[/green]")
        console.print(f"  {bp_count:,} binary provides")
        console.print(f"  {sbr_count:,} source buildrequires")
        console.print(f"  {fp_count:,} file provides")

        return conn

    def _create_schema(self, conn: sqlite3.Connection):
        """Create the unified index schema."""
        conn.executescript("""
            CREATE TABLE IF NOT EXISTS binary_provides (
                provides_name TEXT NOT NULL,
                provides_flags TEXT,
                provides_version TEXT,
                binary_package TEXT NOT NULL,
                source_package TEXT NOT NULL,
                repo TEXT NOT NULL
            );

            CREATE TABLE IF NOT EXISTS source_buildrequires (
                source_package TEXT NOT NULL,
                requires_name TEXT NOT NULL,
                requires_flags TEXT,
                requires_version TEXT,
                repo TEXT NOT NULL
            );

            CREATE TABLE IF NOT EXISTS file_provides (
                file_path TEXT NOT NULL,
                binary_package TEXT NOT NULL,
                source_package TEXT NOT NULL,
                repo TEXT NOT NULL
            );

            CREATE TABLE IF NOT EXISTS meta (
                key TEXT PRIMARY KEY,
                value TEXT
            );

            CREATE INDEX IF NOT EXISTS idx_bp_name ON binary_provides(provides_name);
            CREATE INDEX IF NOT EXISTS idx_bp_srcpkg ON binary_provides(source_package);
            CREATE INDEX IF NOT EXISTS idx_sbr_pkg ON source_buildrequires(source_package);
            CREATE INDEX IF NOT EXISTS idx_sbr_req ON source_buildrequires(requires_name);
            CREATE INDEX IF NOT EXISTS idx_fp_path ON file_provides(file_path);
        """)

    def _import_source_primary(
        self, db_path: Path, repo_key: str, conn: sqlite3.Connection
    ):
        """Import BuildRequires from a source repo's primary.sqlite.

        Fedora's source primary.sqlite has tables:
        - packages: pkgKey, name, version, release, ...
        - requires: pkgKey, name, flags, version, ...
        """
        src_conn = sqlite3.connect(str(db_path))
        src_conn.row_factory = sqlite3.Row

        # Get all source package BuildRequires
        # In source repos, requires = BuildRequires
        rows = src_conn.execute("""
            SELECT p.name as source_package,
                   r.name as requires_name,
                   r.flags as requires_flags,
                   r.version as requires_version
            FROM packages p
            JOIN requires r ON p.pkgKey = r.pkgKey
        """).fetchall()

        conn.executemany(
            "INSERT INTO source_buildrequires (source_package, requires_name, requires_flags, requires_version, repo) VALUES (?, ?, ?, ?, ?)",
            [(r["source_package"], r["requires_name"], r["requires_flags"], r["requires_version"], repo_key) for r in rows],
        )

        count = len(rows)
        console.print(f"    {count:,} buildrequires entries")
        src_conn.close()

    def _import_binary_primary(
        self, db_path: Path, repo_key: str, conn: sqlite3.Connection
    ):
        """Import Provides from a binary repo's primary.sqlite.

        Fedora's binary primary.sqlite has tables:
        - packages: pkgKey, name, rpm_sourcerpm, ...
        - provides: pkgKey, name, flags, version, ...
        """
        src_conn = sqlite3.connect(str(db_path))
        src_conn.row_factory = sqlite3.Row

        # Get all binary package provides with their source package
        rows = src_conn.execute("""
            SELECT pr.name as provides_name,
                   pr.flags as provides_flags,
                   pr.version as provides_version,
                   p.name as binary_package,
                   p.rpm_sourcerpm as sourcerpm
            FROM packages p
            JOIN provides pr ON p.pkgKey = pr.pkgKey
        """).fetchall()

        conn.executemany(
            "INSERT INTO binary_provides (provides_name, provides_flags, provides_version, binary_package, source_package, repo) VALUES (?, ?, ?, ?, ?, ?)",
            [
                (
                    r["provides_name"],
                    r["provides_flags"],
                    r["provides_version"],
                    r["binary_package"],
                    extract_srpm_name(r["sourcerpm"]) if r["sourcerpm"] else r["binary_package"],
                    repo_key,
                )
                for r in rows
            ],
        )

        count = len(rows)
        console.print(f"    {count:,} provides entries")

        # Also import files from primary.sqlite's files table
        # (subset of most important files — executables, etc.)
        file_rows = src_conn.execute("""
            SELECT f.name as file_path,
                   p.name as binary_package,
                   p.rpm_sourcerpm as sourcerpm
            FROM packages p
            JOIN files f ON p.pkgKey = f.pkgKey
            WHERE f.type = 'file'
        """).fetchall()

        conn.executemany(
            "INSERT INTO file_provides (file_path, binary_package, source_package, repo) VALUES (?, ?, ?, ?)",
            [
                (
                    r["file_path"],
                    r["binary_package"],
                    extract_srpm_name(r["sourcerpm"]) if r["sourcerpm"] else r["binary_package"],
                    repo_key,
                )
                for r in file_rows
            ],
        )
        console.print(f"    {len(file_rows):,} primary file entries")

        src_conn.close()

    def _import_binary_filelists(
        self, db_path: Path, repo_key: str, conn: sqlite3.Connection
    ):
        """Import file-to-package mappings from a binary repo's filelists.sqlite.

        Fedora's filelists.sqlite has tables:
        - packages: pkgKey, pkgId
        - filelist: pkgKey, dirname, filenames, filetypes
        We need to join with the primary data to get source package names.
        """
        src_conn = sqlite3.connect(str(db_path))
        src_conn.row_factory = sqlite3.Row

        # filelists.sqlite only has pkgKey — we need to map to package names.
        # The primary.sqlite has the mapping. We'll use a subquery against
        # our already-imported binary_provides to get the source package.
        #
        # For efficiency, first build a pkgKey -> (binary_package, sourcerpm) map
        # from the filelists db's packages table + a name lookup.

        # The filelists.sqlite packages table has: pkgKey, pkgId
        # We need binary package names. The primary.sqlite has them but we
        # already imported. Let's read the primary for this repo to get the mapping.

        # Actually, filelists.sqlite's filelist table has:
        # pkgKey, dirname, filenames (space-separated), filetypes
        # And packages table has: pkgKey, pkgId (checksum)

        # We need to cross-reference with primary to get package names.
        # Since we're reading from the same repo's downloaded files,
        # let's get the primary db path and read package names from there.

        # Find the matching primary db
        key_prefix = f"{repo_key}-x86_64"
        primary_path = self.cache_dir / f"{key_prefix}-primary.sqlite"
        if not primary_path.exists():
            console.print(f"    [yellow]Warning: No primary for filelists mapping[/yellow]")
            src_conn.close()
            return

        primary_conn = sqlite3.connect(str(primary_path))
        primary_conn.row_factory = sqlite3.Row

        # Build pkgKey -> (binary_name, source_name) from primary
        pkg_map = {}
        for row in primary_conn.execute("SELECT pkgKey, name, rpm_sourcerpm FROM packages"):
            srcname = extract_srpm_name(row["rpm_sourcerpm"]) if row["rpm_sourcerpm"] else row["name"]
            pkg_map[row["pkgKey"]] = (row["name"], srcname)
        primary_conn.close()

        # Now read filelists and expand
        batch = []
        batch_size = 10000

        for row in src_conn.execute("SELECT pkgKey, dirname, filenames FROM filelist"):
            pkg_key = row["pkgKey"]
            if pkg_key not in pkg_map:
                continue

            binary_name, source_name = pkg_map[pkg_key]
            dirname = row["dirname"]
            filenames = row["filenames"]

            if not filenames:
                continue

            # filenames is space-separated (or could be individual)
            for fname in filenames.split("/"):
                if not fname:
                    continue
                filepath = dirname + "/" + fname if dirname != "/" else "/" + fname
                # Only store files that look like they could be deps
                # (executables, libraries, pkg-config files)
                if filepath.startswith(("/usr/bin/", "/usr/sbin/", "/bin/", "/sbin/",
                                        "/usr/lib", "/usr/share/pkgconfig/",
                                        "/usr/share/aclocal/")):
                    batch.append((filepath, binary_name, source_name, repo_key))

            if len(batch) >= batch_size:
                conn.executemany(
                    "INSERT INTO file_provides (file_path, binary_package, source_package, repo) VALUES (?, ?, ?, ?)",
                    batch,
                )
                batch = []

        if batch:
            conn.executemany(
                "INSERT INTO file_provides (file_path, binary_package, source_package, repo) VALUES (?, ?, ?, ?)",
                batch,
            )

        total = conn.execute("SELECT COUNT(*) FROM file_provides WHERE repo = ?", (repo_key,)).fetchone()[0]
        console.print(f"    {total:,} file provides entries")
        src_conn.close()
