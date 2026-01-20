"""CLI for mogrix SRPM conversion engine."""

from pathlib import Path

import click
from rich.console import Console
from rich.table import Table

from mogrix.parser.spec import SpecParser
from mogrix.rules.loader import RuleLoader
from mogrix.rules.engine import RuleEngine
from mogrix.emitter.spec import SpecWriter
from mogrix.headers.overlay import HeaderOverlayManager

console = Console()

# Default directories (relative to package)
RULES_DIR = Path(__file__).parent.parent / "rules"
HEADERS_DIR = Path(__file__).parent.parent / "headers"


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
    output: str | None,
):
    """Convert a spec file using rules."""
    spec_path = Path(spec_file)
    rules_path = Path(rules_dir) if rules_dir else RULES_DIR
    headers_path = Path(headers_dir) if headers_dir else HEADERS_DIR

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

    # Write modified spec
    writer = SpecWriter()
    content = writer.write(result, drops=drops, adds=adds, cppflags=cppflags)

    if output:
        Path(output).write_text(content)
        console.print(f"Wrote: {output}")
    else:
        print(content)


if __name__ == "__main__":
    main()
