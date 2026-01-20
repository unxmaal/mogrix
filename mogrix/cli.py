"""CLI for mogrix SRPM conversion engine."""

from pathlib import Path

import click
from rich.console import Console
from rich.table import Table

from mogrix.compat.injector import CompatInjector
from mogrix.emitter.spec import SpecWriter
from mogrix.headers.overlay import HeaderOverlayManager
from mogrix.parser.spec import SpecParser
from mogrix.rules.engine import RuleEngine
from mogrix.rules.loader import RuleLoader

console = Console()

# Default directories (relative to package)
RULES_DIR = Path(__file__).parent.parent / "rules"
HEADERS_DIR = Path(__file__).parent.parent / "headers"
COMPAT_DIR = Path(__file__).parent.parent / "compat"


@click.group()
@click.version_option()
def main():
    """Mogrix - SRPM conversion engine for IRIX."""
    pass


@main.command()
@click.argument("spec_or_srpm", type=click.Path(exists=True))
@click.option(
    "--rules-dir",
    type=click.Path(exists=True),
    default=None,
    help="Path to rules directory",
)
def analyze(spec_or_srpm: str, rules_dir: str | None):
    """Analyze a spec file or SRPM and show what rules would apply."""
    import shutil

    input_path = Path(spec_or_srpm)
    rules_path = Path(rules_dir) if rules_dir else RULES_DIR
    temp_dir = None

    # Handle SRPM vs spec file
    if input_path.name.endswith(".src.rpm"):
        from mogrix.parser.srpm import SRPMExtractor

        console.print(f"[bold]Extracting:[/bold] {input_path.name}")
        extractor = SRPMExtractor(input_path)
        temp_dir, spec_path = extractor.extract_spec()
        console.print(f"[bold]Found spec:[/bold] {spec_path.name}\n")
    else:
        spec_path = input_path

    try:
        # Parse spec
        parser = SpecParser()
        spec = parser.parse(spec_path)
    finally:
        # Clean up temp directory if we extracted an SRPM
        if temp_dir and temp_dir.exists():
            shutil.rmtree(temp_dir)

    # Load rules and apply
    loader = RuleLoader(rules_path)
    engine = RuleEngine(loader)
    result = engine.apply(spec)

    # Display results
    console.print(f"\n[bold]Package:[/bold] {spec.name} {spec.version}")
    console.print(f"[bold]Summary:[/bold] {spec.summary}\n")

    # BuildRequires table
    table = Table(title="BuildRequires")
    table.add_column("Original", style="cyan")
    table.add_column("After Rules", style="green")

    original_br = set(spec.buildrequires)
    final_br = set(result.spec.buildrequires)

    all_br = original_br | final_br
    for br in sorted(all_br):
        orig = br if br in original_br else ""
        final = br if br in final_br else "[red]REMOVED[/red]"
        if br not in original_br:
            orig = "[yellow]ADDED[/yellow]"
            final = br
        table.add_row(orig, final)

    console.print(table)

    # Applied rules
    if result.applied_rules:
        console.print("\n[bold]Applied Rules:[/bold]")
        for rule in result.applied_rules:
            console.print(f"  - {rule}")

    # Configure flags
    if result.configure_disable:
        console.print("\n[bold]Configure --disable flags:[/bold]")
        for flag in result.configure_disable:
            console.print(f"  --disable-{flag}")

    # Header overlays
    if result.header_overlays:
        console.print("\n[bold]Header Overlays:[/bold]")
        for overlay in result.header_overlays:
            console.print(f"  - {overlay}")

    # Compat functions
    if result.compat_functions:
        console.print("\n[bold]Compat Functions (injected):[/bold]")
        for func in result.compat_functions:
            console.print(f"  - {func}")

    # AC_CV overrides
    if result.ac_cv_overrides:
        console.print("\n[bold]Autoconf Cache Overrides:[/bold]")
        for var, val in result.ac_cv_overrides.items():
            console.print(f"  {var}={val}")


@main.command()
@click.argument("spec_or_srpm", type=click.Path(exists=True))
@click.option(
    "--rules-dir",
    type=click.Path(exists=True),
    default=None,
    help="Path to rules directory",
)
@click.option(
    "--headers-dir",
    type=click.Path(exists=True),
    default=None,
    help="Path to headers directory",
)
@click.option(
    "--compat-dir",
    type=click.Path(exists=True),
    default=None,
    help="Path to compat sources directory",
)
@click.option(
    "--output-dir",
    "-o",
    type=click.Path(),
    default=None,
    help="Output directory (for SRPMs) or file (for specs)",
)
def convert(
    spec_or_srpm: str,
    rules_dir: str | None,
    headers_dir: str | None,
    compat_dir: str | None,
    output_dir: str | None,
):
    """Convert a spec file or SRPM using rules.

    For SRPM input: extracts, converts, and repackages as a new SRPM.
    For spec input: converts and outputs the spec content.
    """
    import shutil

    input_path = Path(spec_or_srpm)
    rules_path = Path(rules_dir) if rules_dir else RULES_DIR
    headers_path = Path(headers_dir) if headers_dir else HEADERS_DIR
    compat_path = Path(compat_dir) if compat_dir else COMPAT_DIR

    is_srpm = input_path.name.endswith(".src.rpm")

    if is_srpm:
        _convert_srpm_full(
            input_path, rules_path, headers_path, compat_path, output_dir
        )
    else:
        _convert_spec_only(
            input_path, rules_path, headers_path, compat_path, output_dir
        )


def _convert_spec_only(
    spec_path: Path,
    rules_path: Path,
    headers_path: Path,
    compat_path: Path,
    output: str | None,
):
    """Convert a spec file and output the content."""
    # Parse spec
    parser = SpecParser()
    spec = parser.parse(spec_path)

    # Load rules and apply
    loader = RuleLoader(rules_path)
    engine = RuleEngine(loader)
    result = engine.apply(spec)

    # Generate converted content
    content = _generate_converted_spec(
        spec, result, headers_path, compat_path
    )

    if output:
        Path(output).write_text(content)
        console.print(f"[bold]Wrote:[/bold] {output}")
    else:
        print(content)


def _convert_srpm_full(
    srpm_path: Path,
    rules_path: Path,
    headers_path: Path,
    compat_path: Path,
    output_dir: str | None,
):
    """Extract SRPM, convert spec, copy sources, and repackage."""
    import shutil
    from mogrix.parser.srpm import SRPMExtractor
    from mogrix.emitter.srpm import SRPMEmitter

    # Determine output directory
    if output_dir:
        out_path = Path(output_dir)
    else:
        # Default: create directory next to input SRPM
        out_path = srpm_path.parent / f"{srpm_path.stem}-converted"

    out_path.mkdir(parents=True, exist_ok=True)

    console.print(f"[bold]Extracting:[/bold] {srpm_path.name}")

    # Extract SRPM to temp directory
    extractor = SRPMExtractor(srpm_path)
    extracted_dir, spec_path = extractor.extract_spec()

    try:
        console.print(f"[bold]Found spec:[/bold] {spec_path.name}")

        # Parse and convert spec
        parser = SpecParser()
        spec = parser.parse(spec_path)

        loader = RuleLoader(rules_path)
        engine = RuleEngine(loader)
        result = engine.apply(spec)

        # Generate converted spec content
        content = _generate_converted_spec(
            spec, result, headers_path, compat_path
        )

        # Copy all files from extracted SRPM to output directory
        for src_file in extracted_dir.iterdir():
            if src_file.is_file():
                dest_file = out_path / src_file.name
                shutil.copy2(src_file, dest_file)

        # Copy compat source files if needed
        if result.compat_functions:
            injector = CompatInjector(compat_path)
            compat_files = injector.resolve_functions(result.compat_functions)
            for compat_file in compat_files:
                dest_file = out_path / compat_file.name
                shutil.copy2(compat_file, dest_file)
            console.print(f"[bold]Compat sources:[/bold] {len(compat_files)} files ({', '.join(f.name for f in compat_files)})")

        # Write converted spec (overwriting the original)
        converted_spec_path = out_path / spec_path.name
        converted_spec_path.write_text(content)

        console.print(f"[bold]Converted spec:[/bold] {converted_spec_path}")
        console.print(f"[bold]Sources copied:[/bold] {len(list(out_path.iterdir())) - 1} files")

        # Build new SRPM
        console.print("\n[bold]Building SRPM...[/bold]")

        # Gather source files (everything except the spec)
        sources = [
            f for f in out_path.iterdir()
            if f.is_file() and not f.name.endswith(".spec")
        ]

        emitter = SRPMEmitter()
        new_srpm = emitter.emit_srpm(
            spec_content=content,
            spec_name=spec_path.name,
            sources=sources,
            output_dir=out_path,
        )

        # Summary
        console.print("\n[bold green]Conversion complete![/bold green]")
        console.print(f"[bold]Output directory:[/bold] {out_path}")
        console.print(f"[bold]Converted SRPM:[/bold] {new_srpm}")
        console.print(f"[bold]Rules applied:[/bold] {len(result.applied_rules)}")

        if result.compat_functions:
            console.print(f"[bold]Compat functions:[/bold] {', '.join(result.compat_functions)}")

    finally:
        # Clean up extracted temp directory
        if extracted_dir.exists():
            shutil.rmtree(extracted_dir)


def _generate_converted_spec(
    spec,
    result,
    headers_path: Path,
    compat_path: Path,
) -> str:
    """Generate the converted spec content."""
    # Determine what was dropped/added
    original_br = set(spec.buildrequires)
    final_br = set(result.spec.buildrequires)
    drops = list(original_br - final_br)
    adds = list(final_br - original_br)

    # Generate CPPFLAGS for header overlays
    cppflags = None
    if result.header_overlays:
        overlay_mgr = HeaderOverlayManager(headers_path)
        cppflags = overlay_mgr.get_cppflags(result.header_overlays)

    # Generate compat source injection
    compat_sources = None
    compat_prep = None
    compat_build = None
    if result.compat_functions:
        injector = CompatInjector(compat_path)
        compat_sources = injector.get_source_entries(result.compat_functions)
        compat_prep = injector.get_prep_commands(result.compat_functions)
        compat_build = injector.get_build_commands(result.compat_functions)

    # Write modified spec
    writer = SpecWriter()
    return writer.write(
        result,
        drops=drops,
        adds=adds,
        cppflags=cppflags,
        compat_sources=compat_sources,
        compat_prep=compat_prep,
        compat_build=compat_build,
        ac_cv_overrides=result.ac_cv_overrides if result.ac_cv_overrides else None,
        drop_requires=result.drop_requires if result.drop_requires else None,
        remove_lines=result.remove_lines if result.remove_lines else None,
        rpm_macros=result.rpm_macros if result.rpm_macros else None,
    )


@main.command()
@click.argument("srpms_dir", type=click.Path(exists=True))
@click.argument("output_dir", type=click.Path())
@click.option(
    "--rules-dir",
    type=click.Path(exists=True),
    default=None,
    help="Path to rules directory",
)
@click.option(
    "--headers-dir",
    type=click.Path(exists=True),
    default=None,
    help="Path to headers directory",
)
@click.option(
    "--compat-dir",
    type=click.Path(exists=True),
    default=None,
    help="Path to compat sources directory",
)
def batch(
    srpms_dir: str,
    output_dir: str,
    rules_dir: str | None,
    headers_dir: str | None,
    compat_dir: str | None,
):
    """Convert multiple SRPMs in batch.

    SRPMS_DIR is a directory containing .src.rpm files.
    OUTPUT_DIR is where converted SRPMs will be written.
    """
    from mogrix.batch import BatchConverter

    srpms_path = Path(srpms_dir)
    output_path = Path(output_dir)
    rules_path = Path(rules_dir) if rules_dir else RULES_DIR
    headers_path = Path(headers_dir) if headers_dir else HEADERS_DIR
    compat_path = Path(compat_dir) if compat_dir else COMPAT_DIR

    converter = BatchConverter(
        srpms_path,
        rules_dir=rules_path,
        headers_dir=headers_path,
        compat_dir=compat_path,
    )

    console.print(f"[bold]Discovering SRPMs in:[/bold] {srpms_path}")
    srpms = converter.discover_srpms()
    console.print(f"Found {len(srpms)} SRPM files\n")

    console.print(f"[bold]Converting to:[/bold] {output_path}\n")
    results = converter.convert_all(output_path)

    # Display results
    summary = converter.get_summary(results)

    table = Table(title="Conversion Results")
    table.add_column("Package", style="cyan")
    table.add_column("Status", style="green")
    table.add_column("Rules Applied", style="dim")
    table.add_column("Output SRPM", style="dim")

    for r in results:
        status = "[green]OK[/green]" if r["status"] == "success" else "[red]ERROR[/red]"
        rules_count = str(len(r.get("applied_rules", [])))
        output_srpm = Path(r.get("output_srpm", "")).name if r.get("output_srpm") else "-"
        table.add_row(r["package"], status, rules_count, output_srpm)

    console.print(table)

    console.print("\n[bold]Summary:[/bold]")
    console.print(f"  Total: {summary['total']}")
    console.print(f"  Success: [green]{summary['success']}[/green]")
    if summary["errors"] > 0:
        console.print(f"  Errors: [red]{summary['errors']}[/red]")
        for err in summary["error_packages"]:
            console.print(f"    - {err['package']}: {err['error']}")


@main.command()
def list_rules():
    """List all available package rules."""
    rules_path = RULES_DIR / "packages"
    if not rules_path.exists():
        console.print("[red]No package rules found[/red]")
        return

    rules = sorted(rules_path.glob("*.yaml"))
    console.print(f"[bold]Available package rules ({len(rules)}):[/bold]\n")

    for rule_file in rules:
        console.print(f"  - {rule_file.stem}")


@main.command()
@click.argument("spec_or_srpm", type=click.Path(exists=True))
@click.option(
    "--rpmbuild-dir",
    type=click.Path(),
    default=None,
    help="rpmbuild directory (default: ~/rpmbuild)",
)
@click.option(
    "--target",
    type=str,
    default="irix6.5-n32",
    help="Build target (default: irix6.5-n32)",
)
@click.option(
    "--macros",
    type=click.Path(exists=True),
    default=None,
    help="Path to custom RPM macros file",
)
@click.option(
    "--dry-run",
    is_flag=True,
    help="Show what would be done without building",
)
def build(
    spec_or_srpm: str,
    rpmbuild_dir: str | None,
    target: str,
    macros: str | None,
    dry_run: bool,
):
    """Build a converted spec file or SRPM.

    SPEC_OR_SRPM is a .spec file or .src.rpm to build.
    """
    import subprocess

    input_path = Path(spec_or_srpm)
    rpmbuild_path = Path(rpmbuild_dir) if rpmbuild_dir else Path.home() / "rpmbuild"

    # Determine if this is a spec or SRPM
    if input_path.suffix == ".spec":
        spec_path = input_path
        is_srpm = False
    elif input_path.name.endswith(".src.rpm"):
        is_srpm = True
        spec_path = None
    else:
        console.print("[red]Error: Input must be a .spec file or .src.rpm[/red]")
        raise SystemExit(1)

    # Build rpmbuild command
    cmd = ["rpmbuild"]

    # Add target if specified
    if target:
        cmd.extend(["--target", target])

    # Add custom macros file
    if macros:
        cmd.extend(["--macros", macros])

    # Add source/spec directories
    cmd.extend(["--define", f"_topdir {rpmbuild_path}"])

    if is_srpm:
        cmd.extend(["--rebuild", str(input_path)])
    else:
        cmd.extend(["-ba", str(spec_path)])

    if dry_run:
        console.print("[bold]Dry run - would execute:[/bold]")
        console.print(f"  {' '.join(cmd)}")
        console.print(f"\n[bold]rpmbuild directory:[/bold] {rpmbuild_path}")
        console.print(f"[bold]Target:[/bold] {target}")
        if macros:
            console.print(f"[bold]Macros:[/bold] {macros}")
        return

    # Ensure rpmbuild directories exist
    for subdir in ["SOURCES", "SPECS", "BUILD", "RPMS", "SRPMS"]:
        (rpmbuild_path / subdir).mkdir(parents=True, exist_ok=True)

    # If building from spec, copy it to SPECS
    if not is_srpm:
        import shutil

        dest_spec = rpmbuild_path / "SPECS" / spec_path.name
        if spec_path != dest_spec:
            shutil.copy2(spec_path, dest_spec)
            cmd[-1] = str(dest_spec)

    console.print(f"[bold]Building:[/bold] {input_path.name}")
    console.print(f"[bold]Target:[/bold] {target}")
    console.print(f"[bold]Command:[/bold] {' '.join(cmd)}\n")

    try:
        result = subprocess.run(cmd, capture_output=True, text=True)

        if result.returncode == 0:
            console.print("\n[bold green]✓ Build succeeded[/bold green]")
        else:
            # Check for missing dependencies
            combined_output = result.stdout + result.stderr
            if "Failed build dependencies" in combined_output:
                fetchable = _handle_missing_deps(combined_output, RULES_DIR)
                if fetchable:
                    _fetch_and_convert_deps(fetchable, input_path.parent)
            else:
                # Some other error
                console.print(combined_output)
                console.print(f"\n[bold red]✗ Build failed (exit code {result.returncode})[/bold red]")
            raise SystemExit(result.returncode)

    except FileNotFoundError:
        console.print("[red]Error: rpmbuild not found. Install rpm-build package.[/red]")
        raise SystemExit(1)


def _fetch_and_convert_deps(packages: list[str], output_dir: Path):
    """Fetch and convert dependency packages.

    Args:
        packages: List of package names to fetch
        output_dir: Directory to save fetched/converted packages
    """
    from mogrix.deps.fedora import FedoraRepo

    deps_dir = output_dir / "deps"
    deps_dir.mkdir(parents=True, exist_ok=True)

    repo = FedoraRepo()
    fetched = []

    console.print(f"\n[bold]Fetching dependencies to:[/bold] {deps_dir}\n")

    for pkg in packages:
        console.print(f"[bold]Fetching:[/bold] {pkg}")
        try:
            matches = repo.search_packages(pkg)
            exact = [m for m in matches if m.name == pkg]

            if exact:
                selected = exact[0]
            elif matches:
                # Show options if no exact match
                console.print(f"  No exact match. Found {len(matches)} similar packages:")
                selected = _prompt_select_srpm(matches)
                if selected is None:
                    continue
            else:
                console.print(f"  [red]✗ Not found in Fedora archives[/red]")
                continue

            console.print(f"  Downloading {selected.filename}...")
            downloaded = repo.download_srpm(selected.url, deps_dir)
            console.print(f"  [green]✓ Downloaded:[/green] {downloaded.name}")
            fetched.append(downloaded)

        except Exception as e:
            console.print(f"  [red]✗ Failed:[/red] {e}")

    if fetched:
        console.print(f"\n[bold]Downloaded {len(fetched)} SRPMs to {deps_dir}[/bold]")
        console.print("\n[bold]Next steps:[/bold]")
        console.print("  1. Convert each dependency:")
        for srpm in fetched:
            console.print(f"     mogrix convert {srpm} -o {deps_dir}")
        console.print("  2. Build dependencies in order (leaf deps first)")
        console.print("  3. Retry building your original package")


def _handle_missing_deps(output: str, rules_dir: Path) -> list[str]:
    """Handle missing dependency errors from rpmbuild.

    Returns:
        List of package names that can be fetched and converted
    """
    from mogrix.deps.resolver import DependencyResolver

    resolver = DependencyResolver(rules_dir)
    missing = resolver.parse_rpmbuild_errors(output)

    if not missing:
        console.print(output)
        return []

    categorized = resolver.categorize_deps(missing)

    console.print("\n[bold red]✗ Build failed: Missing dependencies[/bold red]\n")

    # System packages (should be installed on build host)
    if categorized["system"]:
        console.print("[bold yellow]System packages needed on build host:[/bold yellow]")
        for dep in categorized["system"]:
            ver = f" >= {dep.version}" if dep.version else ""
            console.print(f"  • {dep.name}{ver}")
        console.print()

    # Packages we have rules for
    fetchable_pkgs = []
    if categorized["have_rules"]:
        console.print("[bold green]Packages with mogrix rules (can be fetched & converted):[/bold green]")
        for dep in categorized["have_rules"]:
            pkg = resolver.get_package_for_dep(dep.name)
            ver = f" >= {dep.version}" if dep.version else ""
            console.print(f"  • {dep.name}{ver}  →  [cyan]{pkg}[/cyan]")
            if pkg:
                fetchable_pkgs.append(pkg)
        console.print()

    # Packages we don't have rules for
    if categorized["need_rules"]:
        console.print("[bold red]Packages needing rules (not yet supported):[/bold red]")
        for dep in categorized["need_rules"]:
            ver = f" >= {dep.version}" if dep.version else ""
            console.print(f"  • {dep.name}{ver}")
        console.print()

    # Prompt to fetch dependencies if we have any that can be converted
    if fetchable_pkgs:
        console.print(f"[bold]Fetchable dependencies:[/bold] {', '.join(fetchable_pkgs)}")
        if click.confirm("\nFetch and convert these dependencies?", default=True):
            return fetchable_pkgs

    # Show manual next steps if not fetching
    console.print("\n[bold]Manual steps:[/bold]")
    if categorized["system"]:
        console.print("  1. Install system packages on your build host")
    if fetchable_pkgs:
        console.print(f"  2. Fetch dependencies: mogrix fetch {' '.join(fetchable_pkgs)}")
        console.print("  3. Convert each: mogrix convert <package>.src.rpm -o deps/")
        console.print("  4. Build deps, then retry original package")
    if categorized["need_rules"]:
        console.print("  5. Create rules for unsupported packages in rules/packages/")

    return []


@main.command()
@click.option(
    "--rules-dir",
    type=click.Path(exists=True),
    default=None,
    help="Path to rules directory",
)
@click.option(
    "--compat-dir",
    type=click.Path(exists=True),
    default=None,
    help="Path to compat directory",
)
def validate_rules(rules_dir: str | None, compat_dir: str | None):
    """Validate all rule files for errors and warnings."""
    from mogrix.rules.validator import RuleValidator

    rules_path = Path(rules_dir) if rules_dir else RULES_DIR
    compat_path = Path(compat_dir) if compat_dir else COMPAT_DIR

    console.print(f"[bold]Validating rules in:[/bold] {rules_path}\n")

    validator = RuleValidator(rules_path, compat_path)
    result = validator.validate_all()

    # Display results
    if result.errors:
        console.print("[bold red]Errors:[/bold red]")
        for issue in result.errors:
            console.print(f"  [red]✗[/red] {issue.file}: {issue.message}")
        console.print()

    if result.warnings:
        console.print("[bold yellow]Warnings:[/bold yellow]")
        for issue in result.warnings:
            console.print(f"  [yellow]![/yellow] {issue.file}: {issue.message}")
        console.print()

    # Summary
    console.print("[bold]Summary:[/bold]")
    console.print(f"  Files checked: {result.files_checked}")
    console.print(f"  Package rules: {result.packages_checked}")
    console.print(f"  Errors: [red]{len(result.errors)}[/red]")
    console.print(f"  Warnings: [yellow]{len(result.warnings)}[/yellow]")

    if result.is_valid:
        console.print("\n[bold green]✓ All rules are valid[/bold green]")
    else:
        console.print("\n[bold red]✗ Validation failed[/bold red]")
        raise SystemExit(1)


@main.command()
@click.argument("packages", nargs=-1, required=True)
@click.option(
    "--output-dir",
    "-o",
    type=click.Path(),
    default=".",
    help="Directory to save downloaded SRPMs (default: current directory)",
)
@click.option(
    "--release",
    "-r",
    type=str,
    default="40",
    help="Fedora release version (default: 40)",
)
@click.option(
    "--base-url",
    type=str,
    default=None,
    help="Custom base URL or preset (photon5, photon4, photon3). URLs ending with / are treated as flat directories.",
)
@click.option(
    "--yes",
    "-y",
    is_flag=True,
    help="Auto-confirm single matches without prompting",
)
def fetch(
    packages: tuple[str, ...],
    output_dir: str,
    release: str,
    base_url: str | None,
    yes: bool,
):
    """Fetch SRPMs from Fedora repositories.

    PACKAGES are the package names to search for (e.g., popt zlib curl).

    Searches the Fedora archives for matching SRPMs. If multiple matches
    are found, prompts for selection. Use -y to auto-confirm single matches.
    """
    from mogrix.deps.fedora import FedoraRepo

    output_path = Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)

    repo = FedoraRepo(release=release, base_url=base_url)

    # Show what repo we're using
    if base_url:
        if base_url.lower() in repo.PRESETS:
            console.print(f"[bold]Fetching SRPMs from {base_url} ({repo.PRESETS[base_url.lower()]})[/bold]\n")
        else:
            console.print(f"[bold]Fetching SRPMs from:[/bold] {base_url}\n")
    else:
        console.print(f"[bold]Fetching SRPMs from Fedora {release} archives[/bold]\n")

    success = []
    failed = []
    skipped = []

    for pkg in packages:
        console.print(f"[bold]Searching for:[/bold] {pkg}")

        try:
            matches = repo.search_packages(pkg)
        except RuntimeError as e:
            console.print(f"  [red]✗ Search failed:[/red] {e}")
            failed.append((pkg, str(e)))
            console.print()
            continue

        if not matches:
            console.print(f"  [red]✗ No SRPMs found matching '{pkg}' in Fedora {release}[/red]")
            console.print(f"  [dim]Note: Package may not exist in Fedora or may be named differently[/dim]")
            failed.append((pkg, "No matches found"))
            console.print()
            continue

        # Check for exact match (package name == search term)
        exact_matches = [m for m in matches if m.name == pkg]
        is_fuzzy = len(exact_matches) == 0

        if len(exact_matches) == 1:
            # Single exact match
            selected = exact_matches[0]
            console.print(f"  Found: [cyan]{selected.filename}[/cyan]")
            if not yes:
                if not click.confirm("  Download this SRPM?", default=True):
                    console.print("  [yellow]Skipped[/yellow]")
                    skipped.append(pkg)
                    console.print()
                    continue
        elif len(exact_matches) > 1:
            # Multiple exact matches (different versions?)
            console.print(f"  Found {len(exact_matches)} versions:")
            selected = _prompt_select_srpm(exact_matches)
            if selected is None:
                skipped.append(pkg)
                console.print()
                continue
        elif len(matches) == 1:
            # Single fuzzy match
            selected = matches[0]
            console.print(f"  No exact match. Similar package found: [cyan]{selected.filename}[/cyan]")
            if not click.confirm("  Download this SRPM?", default=True):
                console.print("  [yellow]Skipped[/yellow]")
                skipped.append(pkg)
                console.print()
                continue
        else:
            # Multiple fuzzy matches
            console.print(f"  No exact match. Found {len(matches)} similar packages:")
            selected = _prompt_select_srpm(matches)
            if selected is None:
                skipped.append(pkg)
                console.print()
                continue

        # Download the selected SRPM
        try:
            console.print(f"  Downloading {selected.filename}...")
            downloaded = repo.download_srpm(selected.url, output_path)
            console.print(f"  [green]✓ Downloaded:[/green] {downloaded}")
            success.append((pkg, downloaded))
        except Exception as e:
            console.print(f"  [red]✗ Download failed:[/red] {e}")
            failed.append((pkg, str(e)))

        console.print()

    # Summary
    console.print("[bold]Summary:[/bold]")
    console.print(f"  Downloaded: [green]{len(success)}[/green]")
    if skipped:
        console.print(f"  Skipped: [yellow]{len(skipped)}[/yellow]")
    if failed:
        console.print(f"  Failed: [red]{len(failed)}[/red]")
        for pkg, error in failed:
            console.print(f"    • {pkg}: {error}")

    if success:
        console.print("\n[bold]Next step:[/bold]")
        for pkg, path in success:
            console.print(f"  mogrix convert {path}")

    if failed:
        raise SystemExit(1)


def _prompt_select_srpm(matches: list) -> object | None:
    """Prompt user to select from multiple SRPM matches.

    Args:
        matches: List of SRPMInfo objects

    Returns:
        Selected SRPMInfo or None if cancelled
    """
    for i, m in enumerate(matches, 1):
        console.print(f"    [{i}] {m.filename}")
    console.print(f"    [0] Skip")

    while True:
        try:
            choice = click.prompt("  Select", type=int, default=1)
            if choice == 0:
                console.print("  [yellow]Skipped[/yellow]")
                return None
            if 1 <= choice <= len(matches):
                return matches[choice - 1]
            console.print(f"  [red]Invalid choice. Enter 0-{len(matches)}[/red]")
        except click.Abort:
            return None


if __name__ == "__main__":
    main()
