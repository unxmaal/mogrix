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
from mogrix.staging import ensure_staging_ready

console = Console()

# Default directories (relative to package)
RULES_DIR = Path(__file__).parent.parent / "rules"
HEADERS_DIR = Path(__file__).parent.parent / "headers"
COMPAT_DIR = Path(__file__).parent.parent / "compat"
PATCHES_DIR = Path(__file__).parent.parent / "patches"
CROSS_DIR = Path(__file__).parent.parent / "cross"


@click.group()
@click.version_option()
def main():
    """Mogrix - SRPM conversion engine for IRIX cross-compilation.

    \b
    Workflow:
      1. mogrix setup-cross          # One-time setup
      2. mogrix fetch <package>      # Download SRPM from Fedora
      3. mogrix convert <srpm>       # Apply IRIX rules
      4. mogrix build <srpm> --cross # Cross-compile for IRIX
      5. mogrix stage <rpms>         # Stage for dependent builds
    """
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

        # Copy patch files from mogrix patches directory if add_patch is specified
        patch_files = []
        if result.add_patches:
            patches_pkg_dir = PATCHES_DIR / "packages" / spec.name
            for patch_name in result.add_patches:
                patch_path = patches_pkg_dir / patch_name
                if patch_path.exists():
                    dest_file = out_path / patch_name
                    shutil.copy2(patch_path, dest_file)
                    patch_files.append(patch_name)
                else:
                    console.print(f"[yellow]Warning:[/yellow] Patch not found: {patch_path}")
            if patch_files:
                console.print(f"[bold]Patches added:[/bold] {len(patch_files)} files ({', '.join(patch_files)})")

        # Regenerate spec content with patches if any were added
        if patch_files:
            content = _generate_converted_spec(
                spec, result, headers_path, compat_path, patch_files
            )

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

        # Ensure staging environment is ready for build
        console.print()
        staging_status = ensure_staging_ready(verbose=True)
        if staging_status.created_resources:
            console.print(f"[bold]Staging resources created:[/bold] {len(staging_status.created_resources)}")
        if not staging_status.is_ready:
            console.print("[yellow]Warning: Staging environment may not be fully configured[/yellow]")
            for err in staging_status.errors:
                console.print(f"  [yellow]![/yellow] {err}")

    finally:
        # Clean up extracted temp directory
        if extracted_dir.exists():
            shutil.rmtree(extracted_dir)


def _generate_converted_spec(
    spec,
    result,
    headers_path: Path,
    compat_path: Path,
    patch_files: list[str] | None = None,
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

    # Generate patch entries and prep commands
    patch_sources = None
    patch_prep = None
    if patch_files:
        # Generate Patch entries (start at 200 to avoid conflicts)
        patch_entries = []
        patch_cmds = []
        for i, patch_name in enumerate(patch_files):
            patch_num = 200 + i
            patch_entries.append(f"Patch{patch_num}: {patch_name}")
            patch_cmds.append(f"%patch -P{patch_num} -p1")
        patch_sources = "\n".join(patch_entries)
        patch_prep = "\n".join(patch_cmds)

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
        patch_sources=patch_sources,
        patch_prep=patch_prep,
        ac_cv_overrides=result.ac_cv_overrides if result.ac_cv_overrides else None,
        drop_requires=result.drop_requires if result.drop_requires else None,
        remove_lines=result.remove_lines if result.remove_lines else None,
        rpm_macros=result.rpm_macros if result.rpm_macros else None,
        export_vars=result.export_vars if result.export_vars else None,
        skip_find_lang=result.skip_find_lang,
        skip_check=result.skip_check,
        install_cleanup=result.install_cleanup if result.install_cleanup else None,
        spec_replacements=result.spec_replacements if result.spec_replacements else None,
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


# Cross-compilation default paths
SGUG_STAGING = Path("/opt/sgug-staging/usr/sgug")
IRIX_MACROS = Path("/opt/sgug-staging/rpmmacros.irix")
IRIX_SYSROOT = Path("/opt/irix-sysroot")
CROSS_BINDIR = Path("/opt/cross/bin")


@main.command()
@click.argument("srpm", type=click.Path(exists=True))
@click.option(
    "--rpmbuild-dir",
    type=click.Path(),
    default=None,
    help="rpmbuild directory (default: ~/rpmbuild)",
)
@click.option(
    "--cross",
    is_flag=True,
    help="Enable IRIX cross-compilation mode (uses /opt/sgug-staging/rpmmacros.irix)",
)
@click.option(
    "--macros",
    type=click.Path(exists=True),
    default=None,
    help="Path to custom RPM macros file (overrides --cross default)",
)
@click.option(
    "--dry-run",
    is_flag=True,
    help="Show what would be done without building",
)
def build(
    srpm: str,
    rpmbuild_dir: str | None,
    cross: bool,
    macros: str | None,
    dry_run: bool,
):
    """Build a converted SRPM.

    SRPM is a .src.rpm file to build (typically from mogrix convert).

    Workflow:
        1. mogrix fetch popt
        2. mogrix convert popt-*.src.rpm
        3. mogrix build <converted>.src.rpm --cross
        4. mogrix stage ~/rpmbuild/RPMS/mips/*.rpm

    Use --cross to enable IRIX cross-compilation, which:
      - Uses the cross-toolchain at /opt/cross/bin/
      - Loads rpmmacros.irix from /opt/sgug-staging/
      - Targets IRIX 6.5 N32 ABI
    """
    import subprocess

    input_path = Path(srpm)
    rpmbuild_path = Path(rpmbuild_dir) if rpmbuild_dir else Path.home() / "rpmbuild"

    # Determine if this is a spec or SRPM
    if input_path.suffix == ".spec":
        console.print("[yellow]Warning: Building from spec file directly.[/yellow]")
        console.print("[yellow]Recommended workflow: mogrix convert <srpm> then mogrix build <converted.src.rpm>[/yellow]\n")
        spec_path = input_path
        is_srpm = False
    elif input_path.name.endswith(".src.rpm"):
        is_srpm = True
        spec_path = None
    else:
        console.print("[red]Error: Input must be a .src.rpm file[/red]")
        console.print("\nWorkflow:")
        console.print("  1. mogrix fetch <package>")
        console.print("  2. mogrix convert <package>.src.rpm")
        console.print("  3. mogrix build <converted>.src.rpm --cross")
        raise SystemExit(1)

    # Resolve macros file
    macros_path = None
    if macros:
        macros_path = Path(macros)
    elif cross:
        macros_path = IRIX_MACROS

    # Ensure staging environment is ready (backstop - convert should have done this)
    if cross and not dry_run:
        staging_status = ensure_staging_ready(verbose=True)
        if not staging_status.is_ready:
            console.print("[red]Staging environment is not ready for cross-compilation[/red]")
            for err in staging_status.errors:
                console.print(f"  [red]![/red] {err}")
            console.print("\n[bold]Try running:[/bold] mogrix setup-cross")
            raise SystemExit(1)

    # Validate cross-compilation environment
    if cross:
        _validate_cross_env(dry_run)

    # Build rpmbuild command
    cmd = ["rpmbuild"]

    # Add macros - must chain with system macros for cross-compilation
    if macros_path:
        # Chain: system macros + system macros.d + our cross macros
        macro_chain = f"/usr/lib/rpm/macros:/usr/lib/rpm/macros.d/*:{macros_path}"
        cmd.extend(["--macros", macro_chain])

    # For cross-compilation:
    # 1. Skip RPM dependency checks (host tools may not be RPMs, target deps don't exist)
    # 2. Skip %check section (can't run IRIX binaries on Linux host)
    # 3. Force the target triple so configure gets --host=mips-sgi-irix6.5
    # 4. Set _arch for BUILDROOT path expansion
    # 5. Use --target to ensure RPMs are named with target architecture
    # Note: _target_os must be 'irix' (not 'irix6.5') for native rpm compatibility
    if cross:
        cmd.append("--nodeps")
        cmd.append("--nocheck")
        # --target ensures RPM filenames use target arch (mips) not host (x86_64)
        cmd.extend(["--target", "mips-sgi-irix"])
        cmd.extend(["--define", "_target mips-sgi-irix6.5"])
        cmd.extend(["--define", "_target_cpu mips"])
        cmd.extend(["--define", "_target_os irix"])
        cmd.extend(["--define", "_arch mips"])

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
        if cross:
            console.print("[bold]Mode:[/bold] IRIX cross-compilation")
            console.print(f"[bold]Sysroot:[/bold] {IRIX_SYSROOT}")
            console.print(f"[bold]Cross toolchain:[/bold] {CROSS_BINDIR}")
        if macros_path:
            console.print(f"[bold]Macros:[/bold] {macros_path}")
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
    if cross:
        console.print("[bold]Mode:[/bold] IRIX cross-compilation")
    console.print(f"[bold]Command:[/bold] {' '.join(cmd)}\n")

    try:
        result = subprocess.run(cmd, capture_output=True, text=True)

        if result.returncode == 0:
            console.print("\n[bold green]✓ Build succeeded[/bold green]")
            # Show where RPMs were created
            rpms_dir = rpmbuild_path / "RPMS"
            if rpms_dir.exists():
                rpms = list(rpms_dir.glob("**/*.rpm"))
                if rpms:
                    console.print(f"\n[bold]Built RPMs:[/bold]")
                    for rpm in rpms:
                        console.print(f"  {rpm}")
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


def _validate_cross_env(dry_run: bool = False):
    """Validate that the cross-compilation environment is set up.

    Checks for:
      - IRIX sysroot at /opt/irix-sysroot/
      - Deployed compiler wrapper at staging/bin/irix-cc
      - Deployed linker wrapper at staging/bin/irix-ld
      - rpmmacros.irix at /opt/sgug-staging/
      - Base cross-toolchain (clang, ld.lld-irix)

    Args:
        dry_run: If True, only warn about missing components
    """
    issues = []

    if not IRIX_SYSROOT.exists():
        issues.append(f"IRIX sysroot not found at {IRIX_SYSROOT}")

    # Check for deployed wrappers (from mogrix setup-cross)
    cc = SGUG_STAGING / "bin" / "irix-cc"
    if not cc.exists():
        issues.append(f"Compiler wrapper not found at {cc} (run: mogrix setup-cross)")

    ld = SGUG_STAGING / "bin" / "irix-ld"
    if not ld.exists():
        issues.append(f"Linker wrapper not found at {ld} (run: mogrix setup-cross)")

    if not IRIX_MACROS.exists():
        issues.append(f"rpmmacros.irix not found at {IRIX_MACROS} (run: mogrix setup-cross)")

    # Check for base cross-toolchain
    clang = CROSS_BINDIR / "clang"
    if not clang.exists():
        issues.append(f"clang not found at {clang}")

    lld = CROSS_BINDIR / "ld.lld-irix"
    if not lld.exists():
        issues.append(f"ld.lld-irix not found at {lld}")

    if issues:
        console.print("[bold yellow]Cross-compilation environment issues:[/bold yellow]")
        for issue in issues:
            console.print(f"  [yellow]![/yellow] {issue}")
        console.print()

        if not dry_run:
            console.print("[bold]Required setup:[/bold]")
            console.print("  1. IRIX sysroot at /opt/irix-sysroot/")
            console.print("  2. Cross-toolchain at /opt/cross/bin/")
            console.print("     - clang (MIPS-capable)")
            console.print("     - ld.lld-irix (vvuk's patched LLD)")
            console.print("  3. Run: mogrix setup-cross")
            console.print()
            console.print("See /src/plan.md for detailed setup instructions.")
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

    # Show what repo we're using with full URL
    archive_url = f"{repo.ARCHIVE_BASE}/{release}/Everything/source/tree/Packages/"
    if base_url:
        if base_url.lower() in repo.PRESETS:
            console.print(f"[bold]Fetching SRPMs from {base_url} ({repo.PRESETS[base_url.lower()]})[/bold]\n")
        else:
            console.print(f"[bold]Fetching SRPMs from:[/bold] {base_url}\n")
    else:
        console.print(f"[bold]Fetching SRPMs from Fedora {release} archives[/bold]")
        console.print(f"[dim]{archive_url}[/dim]\n")

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


@main.command("setup-cross")
@click.option(
    "--staging-dir",
    type=click.Path(),
    default="/opt/sgug-staging/usr/sgug",
    help="SGUG staging directory (default: /opt/sgug-staging/usr/sgug)",
)
@click.option(
    "--sysroot",
    type=click.Path(exists=True),
    default="/opt/irix-sysroot",
    help="IRIX sysroot directory (default: /opt/irix-sysroot)",
)
@click.option(
    "--cross-bindir",
    type=click.Path(exists=True),
    default="/opt/cross/bin",
    help="Cross-toolchain bin directory (default: /opt/cross/bin)",
)
@click.option(
    "--dry-run",
    is_flag=True,
    help="Show what would be done without making changes",
)
def setup_cross(
    staging_dir: str,
    sysroot: str,
    cross_bindir: str,
    dry_run: bool,
):
    """Set up the IRIX cross-compilation environment.

    Deploys compiler wrappers, dicl-clang-compat headers, and RPM macros
    to the staging directory for cross-compilation.

    Prerequisites:
      - IRIX sysroot at /opt/irix-sysroot/ (or --sysroot)
      - Cross-toolchain at /opt/cross/bin/ (or --cross-bindir)
        - clang (MIPS-capable)
        - ld.lld-irix (vvuk's patched LLD)
        - llvm-ar, llvm-ranlib, llvm-strip, llvm-nm
      - libsoft_float_stubs.a in staging lib32/
    """
    import shutil
    import stat

    staging_path = Path(staging_dir)
    sysroot_path = Path(sysroot)
    cross_bin_path = Path(cross_bindir)

    console.print("[bold]Setting up IRIX cross-compilation environment[/bold]\n")

    # Validate prerequisites
    issues = []

    if not sysroot_path.exists():
        issues.append(f"IRIX sysroot not found at {sysroot_path}")

    clang = cross_bin_path / "clang"
    if not clang.exists():
        issues.append(f"clang not found at {clang}")

    lld = cross_bin_path / "ld.lld-irix"
    if not lld.exists():
        issues.append(f"ld.lld-irix not found at {lld}")

    soft_float = staging_path / "lib32" / "libsoft_float_stubs.a"
    if not soft_float.exists() and staging_path.exists():
        issues.append(f"libsoft_float_stubs.a not found at {soft_float}")

    if issues:
        console.print("[bold red]Prerequisites not met:[/bold red]")
        for issue in issues:
            console.print(f"  [red]![/red] {issue}")
        console.print()
        if not dry_run:
            console.print("[bold]Please ensure prerequisites are installed.[/bold]")
            console.print("See /src/plan.md for setup instructions.")
            raise SystemExit(1)
        console.print("[yellow]Continuing dry-run despite issues...[/yellow]\n")

    # Define what to deploy
    deployments = [
        # (source, destination, description)
        (CROSS_DIR / "bin" / "irix-cc", staging_path / "bin" / "irix-cc", "C compiler wrapper"),
        (CROSS_DIR / "bin" / "irix-ld", staging_path / "bin" / "irix-ld", "Linker wrapper"),
        (CROSS_DIR / "rpmmacros.irix", staging_path.parent.parent / "rpmmacros.irix", "RPM macros"),
    ]

    # Add dicl-clang-compat headers (IRIX header fixes for clang)
    clang_compat_src = CROSS_DIR / "include" / "dicl-clang-compat"
    clang_compat_dst = staging_path / "include" / "dicl-clang-compat"

    if clang_compat_src.exists():
        for header in clang_compat_src.rglob("*.h"):
            rel_path = header.relative_to(clang_compat_src)
            dst = clang_compat_dst / rel_path
            deployments.append((header, dst, f"Header: {rel_path}"))

    console.print("[bold]Files to deploy:[/bold]")
    for src, dst, desc in deployments:
        status = "[green]exists[/green]" if dst.exists() else "[yellow]new[/yellow]"
        console.print(f"  {desc}")
        console.print(f"    [dim]{src}[/dim]")
        console.print(f"    → {dst} ({status})")

    if dry_run:
        console.print("\n[yellow]Dry run - no changes made[/yellow]")
        return

    console.print("\n[bold]Deploying files...[/bold]")

    for src, dst, desc in deployments:
        if not src.exists():
            console.print(f"  [red]![/red] Source not found: {src}")
            continue

        # Create parent directory
        dst.parent.mkdir(parents=True, exist_ok=True)

        # Copy file
        shutil.copy2(src, dst)

        # Make executables executable
        if dst.parent.name == "bin":
            dst.chmod(dst.stat().st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)

        console.print(f"  [green]✓[/green] {desc}")

    # Create C++ wrapper as symlink/copy of C wrapper
    cxx_wrapper = staging_path / "bin" / "irix-cxx"
    if not cxx_wrapper.exists():
        cc_wrapper = staging_path / "bin" / "irix-cc"
        if cc_wrapper.exists():
            shutil.copy2(cc_wrapper, cxx_wrapper)
            cxx_wrapper.chmod(cxx_wrapper.stat().st_mode | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH)
            console.print(f"  [green]✓[/green] C++ compiler wrapper (copy of C wrapper)")

    console.print("\n[bold green]Cross-compilation environment set up![/bold green]")
    console.print(f"\n[bold]Staging directory:[/bold] {staging_path}")
    console.print(f"[bold]RPM macros:[/bold] {staging_path.parent.parent / 'rpmmacros.irix'}")
    console.print("\n[bold]Workflow:[/bold]")
    console.print("  1. Fetch SRPM:    mogrix fetch <package>")
    console.print("  2. Convert SRPM:  mogrix convert <package>.src.rpm")
    console.print("  3. Build SRPM:    mogrix build <converted>.src.rpm --cross")
    console.print("  4. Stage RPMs:    mogrix stage ~/rpmbuild/RPMS/mips/*.rpm")


@main.command()
@click.argument("rpms", nargs=-1, type=click.Path(exists=True))
@click.option(
    "--staging-dir",
    type=click.Path(),
    default="/opt/sgug-staging",
    help="Staging root directory (default: /opt/sgug-staging)",
)
@click.option(
    "--clean",
    is_flag=True,
    help="Clean staged packages (preserves base cross-compilation setup)",
)
@click.option(
    "--list",
    "list_staged",
    is_flag=True,
    help="List staged packages",
)
@click.option(
    "--with-devel/--no-devel",
    default=True,
    help="Automatically include matching -devel packages (default: enabled)",
)
def stage(
    rpms: tuple[str, ...],
    staging_dir: str,
    clean: bool,
    list_staged: bool,
    with_devel: bool,
):
    """Stage cross-compiled RPMs for dependency resolution.

    Install RPMs to the staging area so subsequent builds can find
    their headers and libraries.

    Workflow:
        1. mogrix fetch popt
        2. mogrix convert popt-*.src.rpm
        3. mogrix build <converted>.src.rpm --cross
        4. mogrix stage ~/rpmbuild/RPMS/mips/popt*.rpm
        5. (Now dependent packages can find popt headers/libs)

    Examples:
        mogrix stage ~/rpmbuild/RPMS/mips/popt-*.mips.rpm
        mogrix stage ~/rpmbuild/RPMS/mips/*.rpm
        mogrix stage --list
        mogrix stage --clean
    """
    import subprocess

    staging_path = Path(staging_dir)

    # Files/directories to preserve during clean
    PRESERVE = {
        "usr/sgug/bin/irix-cc",
        "usr/sgug/bin/irix-cxx",
        "usr/sgug/bin/irix-ld",
        "usr/sgug/include/dicl-clang-compat",
        "usr/sgug/include/mogrix-compat",
        "usr/sgug/lib32/libsoft_float_stubs.a",
        "rpmmacros.irix",
    }

    # Pre-existing libraries that came with the staging area (not from our RPMs)
    PREEXISTING_LIBS = {
        "libbz2.a", "libformw.a", "libhistory.a", "liblzma.a", "liblzma.la",
        "libmenuw.a", "libncursesw.a", "libpanelw.a", "libreadline.a",
        "libtinfow.a", "libz.a",
    }

    PREEXISTING_HEADERS = {
        "bzlib.h", "gnumake.h", "lzma.h", "zconf.h", "zlib.h",
        "lzma", "ncursesw", "readline",
    }

    if list_staged:
        _list_staged_packages(staging_path, PREEXISTING_LIBS, PREEXISTING_HEADERS)
        return

    if clean:
        _clean_staged_packages(staging_path, PRESERVE, PREEXISTING_LIBS, PREEXISTING_HEADERS)
        return

    if not rpms:
        console.print("[yellow]No RPMs specified. Use --help for usage.[/yellow]")
        console.print("\nExamples:")
        console.print("  mogrix stage popt-1.19-6.mips.rpm  # Auto-includes popt-devel")
        console.print("  mogrix stage ~/rpmbuild/RPMS/mips/*.rpm")
        console.print("  mogrix stage --no-devel popt-1.19-6.mips.rpm  # Skip -devel")
        console.print("  mogrix stage --list")
        console.print("  mogrix stage --clean")
        return

    # Expand RPM list to include -devel packages if --with-devel
    import re
    expanded_rpms = list(rpms)
    if with_devel:
        for rpm_path in rpms:
            rpm_file = Path(rpm_path)
            # Parse RPM name: name-version-release.arch.rpm
            # e.g., libxml2-2.10.4-3.mips.rpm -> libxml2, 2.10.4-3.mips
            match = re.match(r'^(.+?)-(\d+[\d\.\-]+\w+)\.rpm$', rpm_file.name)
            if match and '-devel-' not in rpm_file.name:
                pkg_name = match.group(1)
                version_arch = match.group(2)
                devel_name = f"{pkg_name}-devel-{version_arch}.rpm"
                devel_path = rpm_file.parent / devel_name
                if devel_path.exists() and str(devel_path) not in expanded_rpms:
                    expanded_rpms.append(str(devel_path))
                    console.print(f"[dim]Auto-including:[/dim] {devel_name}")

    # Install RPMs to staging
    console.print(f"[bold]Staging RPMs to:[/bold] {staging_path}\n")

    for rpm_path in expanded_rpms:
        rpm_file = Path(rpm_path)
        console.print(f"[bold]Installing:[/bold] {rpm_file.name}")

        try:
            # Extract RPM to staging directory
            result = subprocess.run(
                f"cd {staging_path} && rpm2cpio {rpm_file.absolute()} | cpio -idm",
                shell=True,
                capture_output=True,
                text=True,
            )

            if result.returncode != 0:
                console.print(f"  [red]✗ Failed:[/red] {result.stderr}")
                continue

            console.print(f"  [green]✓ Installed[/green]")

        except Exception as e:
            console.print(f"  [red]✗ Error:[/red] {e}")

    # Fix multiarch headers (create mips64 variants from x86_64)
    _fix_multiarch_headers(staging_path)

    console.print("\n[bold green]Staging complete![/bold green]")
    console.print("\nStaged libraries are now available for cross-compilation.")


def _fix_multiarch_headers(staging_path: Path) -> None:
    """Create mips64 variants of multiarch headers.

    Some packages (lua, openssl) use multiarch header dispatch where the main
    header includes an architecture-specific header like:
        #include <luaconf-x86_64.h>  // on x86_64
        #include <luaconf-mips64.h>  // on mips64

    Since we're cross-compiling for mips64 but building on x86_64, the x86_64
    headers get installed. We need to create mips64 copies.
    """
    import shutil

    include_dir = staging_path / "usr" / "sgug" / "include"

    # Known multiarch headers: (x86_64 source, mips64 target)
    MULTIARCH_HEADERS = [
        # Lua
        ("luaconf-x86_64.h", "luaconf-mips64.h"),
        # OpenSSL
        ("openssl/configuration-x86_64.h", "openssl/configuration-mips64.h"),
        ("openssl/opensslconf-x86_64.h", "openssl/opensslconf-mips64.h"),
    ]

    fixed_any = False
    for src_name, dst_name in MULTIARCH_HEADERS:
        src_path = include_dir / src_name
        dst_path = include_dir / dst_name

        if src_path.exists() and not dst_path.exists():
            # Ensure parent directory exists
            dst_path.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(src_path, dst_path)
            if not fixed_any:
                console.print("\n[bold]Fixing multiarch headers:[/bold]")
                fixed_any = True
            console.print(f"  Created {dst_name} from {src_name}")


def _list_staged_packages(staging_path: Path, preexisting_libs: set, preexisting_headers: set):
    """List packages staged in the staging directory."""
    lib_dir = staging_path / "usr" / "sgug" / "lib32"
    include_dir = staging_path / "usr" / "sgug" / "include"

    console.print(f"[bold]Staged in:[/bold] {staging_path}\n")

    # List libraries (excluding pre-existing)
    console.print("[bold]Libraries:[/bold]")
    if lib_dir.exists():
        libs = sorted(lib_dir.glob("*.so*")) + sorted(lib_dir.glob("*.a"))
        staged_libs = [
            lib for lib in libs
            if lib.name not in preexisting_libs
            and not lib.name.startswith("libsoft_float")
        ]
        if staged_libs:
            for lib in staged_libs:
                console.print(f"  {lib.name}")
        else:
            console.print("  [dim](none)[/dim]")
    else:
        console.print("  [dim](lib32 directory not found)[/dim]")

    # List headers (excluding pre-existing and compat)
    console.print("\n[bold]Headers:[/bold]")
    if include_dir.exists():
        headers = sorted(include_dir.glob("*.h"))
        staged_headers = [
            h for h in headers
            if h.name not in preexisting_headers
        ]
        if staged_headers:
            for header in staged_headers:
                console.print(f"  {header.name}")
        else:
            console.print("  [dim](none)[/dim]")
    else:
        console.print("  [dim](include directory not found)[/dim]")


def _clean_staged_packages(
    staging_path: Path,
    preserve: set,
    preexisting_libs: set,
    preexisting_headers: set,
):
    """Clean staged packages while preserving base setup."""
    import shutil

    lib_dir = staging_path / "usr" / "sgug" / "lib32"
    include_dir = staging_path / "usr" / "sgug" / "include"
    bin_dir = staging_path / "usr" / "sgug" / "bin"
    pkgconfig_dir = lib_dir / "pkgconfig"

    console.print(f"[bold]Cleaning staged packages in:[/bold] {staging_path}\n")

    removed_count = 0

    # Clean libraries (keeping pre-existing and soft_float_stubs)
    if lib_dir.exists():
        for lib in lib_dir.glob("*.so*"):
            if lib.name not in preexisting_libs:
                console.print(f"  Removing: {lib.name}")
                lib.unlink()
                removed_count += 1

        # Remove .a files that aren't pre-existing
        for lib in lib_dir.glob("*.a"):
            if lib.name not in preexisting_libs and not lib.name.startswith("libsoft_float"):
                console.print(f"  Removing: {lib.name}")
                lib.unlink()
                removed_count += 1

        # Remove .la files that aren't pre-existing
        for la in lib_dir.glob("*.la"):
            if la.name not in preexisting_libs:
                console.print(f"  Removing: {la.name}")
                la.unlink()
                removed_count += 1

    # Clean pkgconfig files
    if pkgconfig_dir.exists():
        for pc in pkgconfig_dir.glob("*.pc"):
            # Keep pre-existing ones
            if pc.name not in {"liblzma.pc", "zlib.pc", "ncursesw.pc"}:
                console.print(f"  Removing: pkgconfig/{pc.name}")
                pc.unlink()
                removed_count += 1

    # Clean headers (keeping pre-existing and compat directories)
    if include_dir.exists():
        for header in include_dir.glob("*.h"):
            if header.name not in preexisting_headers:
                console.print(f"  Removing: {header.name}")
                header.unlink()
                removed_count += 1

    # Clean binaries (keeping wrappers)
    if bin_dir.exists():
        preserve_bins = {"irix-cc", "irix-cxx", "irix-ld"}
        for binary in bin_dir.iterdir():
            if binary.is_file() and binary.name not in preserve_bins:
                console.print(f"  Removing: bin/{binary.name}")
                binary.unlink()
                removed_count += 1

    # Clean misc directories that RPMs might have created
    for subdir in ["etc", "share"]:
        dir_path = staging_path / "usr" / "sgug" / subdir
        if dir_path.exists():
            console.print(f"  Removing: {subdir}/")
            shutil.rmtree(dir_path)
            removed_count += 1

    if removed_count > 0:
        console.print(f"\n[bold green]Cleaned {removed_count} items[/bold green]")
    else:
        console.print("[dim]Nothing to clean[/dim]")

    console.print("\n[bold]Preserved:[/bold]")
    console.print("  - Compiler wrappers (irix-cc, irix-cxx, irix-ld)")
    console.print("  - Compat headers (dicl-clang-compat, mogrix-compat)")
    console.print("  - libsoft_float_stubs.a")
    console.print("  - Pre-existing libraries (zlib, bz2, lzma, ncurses, readline)")


@main.command("sync-headers")
@click.option(
    "--staging-dir",
    type=click.Path(),
    default="/opt/sgug-staging/usr/sgug",
    help="Staging directory (default: /opt/sgug-staging/usr/sgug)",
)
def sync_headers(staging_dir: str):
    """Sync compat headers from repo to staging.

    This forces a resync of mogrix-compat and dicl-clang-compat headers
    from the mogrix repo to the staging environment.

    Use this after editing compat headers in the repo to update staging.

    Example:
        mogrix sync-headers
    """
    from .staging import StagingConfig, StagingManager, StagingStatus

    console.print("[bold]Syncing compat headers to staging...[/bold]")

    config = StagingConfig()
    config.staging_dir = Path(staging_dir)

    manager = StagingManager(config)
    status = StagingStatus()

    # Ensure directories exist
    config.include_dir.mkdir(parents=True, exist_ok=True)

    # Force sync headers
    manager._ensure_headers(status, verbose=True, force=True)

    if status.errors:
        for error in status.errors:
            console.print(f"[red]Error: {error}[/red]")
        raise SystemExit(1)

    console.print("[bold green]Headers synced successfully![/bold green]")


if __name__ == "__main__":
    main()
