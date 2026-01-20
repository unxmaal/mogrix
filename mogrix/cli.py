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
@click.argument("spec_file", type=click.Path(exists=True))
@click.option(
    "--rules-dir",
    type=click.Path(exists=True),
    default=None,
    help="Path to rules directory",
)
def analyze(spec_file: str, rules_dir: str | None):
    """Analyze a spec file and show what rules would apply."""
    spec_path = Path(spec_file)
    rules_path = Path(rules_dir) if rules_dir else RULES_DIR

    # Parse spec
    parser = SpecParser()
    spec = parser.parse(spec_path)

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
@click.argument("spec_file", type=click.Path(exists=True))
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
    "--output",
    "-o",
    type=click.Path(),
    default=None,
    help="Output file (default: stdout)",
)
def convert(
    spec_file: str,
    rules_dir: str | None,
    headers_dir: str | None,
    compat_dir: str | None,
    output: str | None,
):
    """Convert a spec file using rules."""
    spec_path = Path(spec_file)
    rules_path = Path(rules_dir) if rules_dir else RULES_DIR
    headers_path = Path(headers_dir) if headers_dir else HEADERS_DIR
    compat_path = Path(compat_dir) if compat_dir else COMPAT_DIR

    # Parse spec
    parser = SpecParser()
    spec = parser.parse(spec_path)

    # Load rules and apply
    loader = RuleLoader(rules_path)
    engine = RuleEngine(loader)
    result = engine.apply(spec)

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
    content = writer.write(
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
    )

    if output:
        Path(output).write_text(content)
        console.print(f"Wrote: {output}")
    else:
        print(content)


@main.command()
@click.argument("specs_dir", type=click.Path(exists=True))
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
    specs_dir: str,
    output_dir: str,
    rules_dir: str | None,
    headers_dir: str | None,
    compat_dir: str | None,
):
    """Convert multiple spec files in batch.

    SPECS_DIR is a directory containing .spec files.
    OUTPUT_DIR is where converted specs will be written.
    """
    from mogrix.batch import BatchConverter

    specs_path = Path(specs_dir)
    output_path = Path(output_dir)
    rules_path = Path(rules_dir) if rules_dir else RULES_DIR
    headers_path = Path(headers_dir) if headers_dir else HEADERS_DIR
    compat_path = Path(compat_dir) if compat_dir else COMPAT_DIR

    converter = BatchConverter(
        specs_path,
        rules_dir=rules_path,
        headers_dir=headers_path,
        compat_dir=compat_path,
    )

    console.print(f"[bold]Discovering specs in:[/bold] {specs_path}")
    specs = converter.discover_specs()
    console.print(f"Found {len(specs)} spec files\n")

    console.print(f"[bold]Converting to:[/bold] {output_path}\n")
    results = converter.convert_all(output_path)

    # Display results
    summary = converter.get_summary(results)

    table = Table(title="Conversion Results")
    table.add_column("Package", style="cyan")
    table.add_column("Status", style="green")
    table.add_column("Rules Applied", style="dim")

    for r in results:
        status = "[green]OK[/green]" if r["status"] == "success" else "[red]ERROR[/red]"
        rules_count = str(len(r.get("applied_rules", [])))
        table.add_row(r["package"], status, rules_count)

    console.print(table)

    console.print(f"\n[bold]Summary:[/bold]")
    console.print(f"  Total: {summary['total']}")
    console.print(f"  Success: [green]{summary['success']}[/green]")
    if summary["errors"] > 0:
        console.print(f"  Errors: [red]{summary['errors']}[/red]")
        for err in summary["error_packages"]:
            console.print(f"    - {err['package']}: {err['error']}")


@main.command()
@click.argument("srpm_file", type=click.Path(exists=True))
@click.option(
    "--output-dir",
    "-o",
    type=click.Path(),
    default=None,
    help="Output directory for converted spec and sources",
)
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
def convert_srpm(
    srpm_file: str,
    output_dir: str | None,
    rules_dir: str | None,
    headers_dir: str | None,
    compat_dir: str | None,
):
    """Convert an SRPM file directly.

    Extracts the SRPM, converts the spec, and outputs to OUTPUT_DIR.
    """
    from mogrix.parser.srpm import SRPMExtractor

    srpm_path = Path(srpm_file)
    rules_path = Path(rules_dir) if rules_dir else RULES_DIR
    headers_path = Path(headers_dir) if headers_dir else HEADERS_DIR
    compat_path = Path(compat_dir) if compat_dir else COMPAT_DIR

    # Determine output directory
    if output_dir:
        out_path = Path(output_dir)
    else:
        out_path = Path.cwd() / f"{srpm_path.stem}-converted"

    out_path.mkdir(parents=True, exist_ok=True)

    console.print(f"[bold]Extracting:[/bold] {srpm_path}")

    # Extract SRPM
    extractor = SRPMExtractor(srpm_path)
    extracted_dir, spec_path = extractor.extract_spec()

    console.print(f"[bold]Found spec:[/bold] {spec_path.name}")

    # Parse and convert
    parser = SpecParser()
    spec = parser.parse(spec_path)

    loader = RuleLoader(rules_path)
    engine = RuleEngine(loader)
    result = engine.apply(spec)

    # Calculate changes
    original_br = set(spec.buildrequires)
    final_br = set(result.spec.buildrequires)
    drops = list(original_br - final_br)
    adds = list(final_br - original_br)

    # Generate overlays
    cppflags = None
    if result.header_overlays:
        overlay_mgr = HeaderOverlayManager(headers_path)
        cppflags = overlay_mgr.get_cppflags(result.header_overlays)

    # Generate compat injection
    compat_sources = None
    compat_prep = None
    compat_build = None
    if result.compat_functions:
        injector = CompatInjector(compat_path)
        compat_sources = injector.get_source_entries(result.compat_functions)
        compat_prep = injector.get_prep_commands(result.compat_functions)
        compat_build = injector.get_build_commands(result.compat_functions)

    # Write converted spec
    writer = SpecWriter()
    content = writer.write(
        result,
        drops=drops,
        adds=adds,
        cppflags=cppflags,
        compat_sources=compat_sources,
        compat_prep=compat_prep,
        compat_build=compat_build,
        ac_cv_overrides=result.ac_cv_overrides or None,
        drop_requires=result.drop_requires or None,
        remove_lines=result.remove_lines or None,
    )

    # Save converted spec
    out_spec = out_path / spec_path.name
    out_spec.write_text(content)

    # Copy source files
    import shutil

    for src_file in extracted_dir.iterdir():
        if src_file.name != spec_path.name:
            shutil.copy2(src_file, out_path / src_file.name)

    # Clean up temp dir
    shutil.rmtree(extracted_dir)

    console.print(f"\n[bold green]Converted successfully![/bold green]")
    console.print(f"[bold]Output:[/bold] {out_path}")
    console.print(f"[bold]Spec:[/bold] {out_spec}")
    console.print(f"[bold]Rules applied:[/bold] {len(result.applied_rules)}")

    if result.compat_functions:
        console.print(f"[bold]Compat functions:[/bold] {', '.join(result.compat_functions)}")


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


if __name__ == "__main__":
    main()
