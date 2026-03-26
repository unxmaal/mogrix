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
RPMLINT_CONFIG = Path(__file__).parent.parent / "rpmlint.toml"

# Default user directories for inputs/outputs
MOGRIX_INPUTS = Path.home() / "mogrix_inputs"
MOGRIX_OUTPUTS = Path.home() / "mogrix_outputs"
# Conversion workspace (working dirs + intermediate SRPMs)
# Separate from SRPMS repo to keep ~/mogrix_outputs/SRPMS/ clean for dnf repo mirror
MOGRIX_CONVERTED = MOGRIX_OUTPUTS / "converted"


STAGING_DIR = Path("/opt/sgug-staging")

# Cross-compilation tools that must stay in sync between repo and staging
# Format: (repo path relative to CROSS_DIR, staging path relative to STAGING_DIR)
_CROSS_TOOLS = [
    ("bin/irix-cc", "usr/sgug/bin/irix-cc"),
    ("bin/irix-ld", "usr/sgug/bin/irix-ld"),
    ("bin/fix-anon-relocs", "usr/sgug/bin/fix-anon-relocs"),
    ("bin/strip-verneed", "usr/sgug/bin/strip-verneed"),
]


def _check_tool_checksums(auto_redeploy: bool = True) -> bool:
    """Verify cross-compilation tools in staging match the repo versions.

    Returns True if all tools match (or were successfully redeployed).
    """
    import hashlib
    import shutil

    staging = STAGING_DIR
    mismatched = []

    for repo_rel, staging_rel in _CROSS_TOOLS:
        repo_path = CROSS_DIR / repo_rel
        staging_path = staging / staging_rel
        if not repo_path.exists():
            continue
        if not staging_path.exists():
            mismatched.append((repo_path, staging_path, "missing"))
            continue

        repo_hash = hashlib.sha256(repo_path.read_bytes()).hexdigest()[:12]
        staging_hash = hashlib.sha256(staging_path.read_bytes()).hexdigest()[:12]
        if repo_hash != staging_hash:
            mismatched.append((repo_path, staging_path, f"repo={repo_hash} staging={staging_hash}"))

    if not mismatched:
        return True

    for repo_path, staging_path, reason in mismatched:
        console.print(f"[yellow]Tool drift: {repo_path.name} ({reason})[/yellow]")
        if auto_redeploy:
            import stat
            shutil.copy2(repo_path, staging_path)
            staging_path.chmod(staging_path.stat().st_mode | stat.S_IEXEC)
            console.print(f"  [green]Auto-redeployed {repo_path.name}[/green]")

    return auto_redeploy  # True if we fixed them, False if just warned


@click.group()
@click.version_option()
def main():
    """Mogrix - SRPM conversion engine for IRIX cross-compilation.

    \b
    Workflow (Fedora packages):
      1. mogrix setup-cross                    # One-time setup
      2. mogrix fetch <package> -y             # → ~/mogrix_inputs/SRPMS/
      3. mogrix convert <srpm>                 # → ~/mogrix_outputs/converted/ + SRPMS/
      4. mogrix build <converted.src.rpm> --cross  # → ~/mogrix_outputs/RPMS/
      5. mogrix stage <rpms>                   # → /opt/sgug-staging/

    \b
    Workflow (upstream git/tarball packages):
      1. mogrix create-srpm <package>          # → ~/mogrix_inputs/SRPMS/
      2. mogrix convert <srpm>                 # → ~/mogrix_outputs/converted/ + SRPMS/
      3. mogrix build <converted.src.rpm> --cross  # → ~/mogrix_outputs/RPMS/
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
@click.option(
    "--no-source-scan",
    is_flag=True,
    help="Skip source code scanning for IRIX compatibility issues",
)
def analyze(spec_or_srpm: str, rules_dir: str | None, no_source_scan: bool):
    """Analyze a spec file or SRPM and show what rules would apply."""
    import shutil
    import tempfile

    input_path = Path(spec_or_srpm)
    rules_path = Path(rules_dir) if rules_dir else RULES_DIR
    temp_dir = None
    extracted_dir = None

    # Handle SRPM vs spec file
    if input_path.name.endswith(".src.rpm"):
        from mogrix.parser.srpm import SRPMExtractor

        console.print(f"[bold]Extracting:[/bold] {input_path.name}")
        extractor = SRPMExtractor(input_path)
        extracted_dir, spec_path = extractor.extract_spec()
        temp_dir = extracted_dir
        console.print(f"[bold]Found spec:[/bold] {spec_path.name}\n")
    else:
        spec_path = input_path

    try:
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

        # Platform-derived transforms
        from mogrix.platform import load_platform

        platform = load_platform(rules_path)
        platform_info = []
        for subsys in sorted(platform.lacking_subsystems()):
            if any(subsys in flag for flag in result.configure_disable):
                platform_info.append(f"  --disable-{subsys} (platform lacks subsystem)")
        if platform_info:
            console.print("\n[bold]Platform-Derived Transforms:[/bold]")
            for info in platform_info:
                console.print(info)

        # Source code scanning (for SRPMs with tarballs)
        if not no_source_scan and extracted_dir:
            _run_source_analysis(extracted_dir, show_handled=True)

    finally:
        # Clean up temp directory if we extracted an SRPM
        if temp_dir and temp_dir.exists():
            shutil.rmtree(temp_dir)


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
@click.option(
    "--no-validate",
    is_flag=True,
    help="Skip spec validation after conversion",
)
@click.option(
    "--strict",
    is_flag=True,
    help="Treat validation warnings as errors",
)
@click.option(
    "--no-source-scan",
    is_flag=True,
    help="Skip source code scanning for IRIX compatibility issues",
)
def convert(
    spec_or_srpm: str,
    rules_dir: str | None,
    headers_dir: str | None,
    compat_dir: str | None,
    output_dir: str | None,
    no_validate: bool,
    strict: bool,
    no_source_scan: bool,
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
            input_path, rules_path, headers_path, compat_path, output_dir,
            validate=not no_validate, strict=strict,
            source_scan=not no_source_scan,
        )
    else:
        _convert_spec_only(
            input_path, rules_path, headers_path, compat_path, output_dir,
            validate=not no_validate, strict=strict,
        )


def _convert_spec_only(
    spec_path: Path,
    rules_path: Path,
    headers_path: Path,
    compat_path: Path,
    output: str | None,
    validate: bool = True,
    strict: bool = False,
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
    write_result = _generate_converted_spec(
        spec, result, headers_path, compat_path
    )
    content = write_result.content

    # Report unmatched spec_replacements
    if write_result.unmatched_required:
        console.print(f"[yellow]Warning: {len(write_result.unmatched_required)} spec_replacement(s) did not match[/yellow]")
        for m in write_result.unmatched_required:
            console.print(f"  [yellow]![/yellow] {m.pattern}")

    # Validate converted spec
    if validate:
        _validate_spec_content(content, spec_path.name, strict)

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
    validate: bool = True,
    strict: bool = False,
    source_scan: bool = True,
):
    """Extract SRPM, convert spec, copy sources, and repackage."""
    import shutil
    from mogrix.parser.srpm import SRPMExtractor
    from mogrix.emitter.srpm import SRPMEmitter

    # Determine output directory
    if output_dir:
        out_path = Path(output_dir)
    else:
        # Default: ~/mogrix_outputs/converted/<name>-converted
        out_path = MOGRIX_CONVERTED / f"{srpm_path.stem}-converted"

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
        write_result = _generate_converted_spec(
            spec, result, headers_path, compat_path
        )
        content = write_result.content

        # Copy all files from extracted SRPM to output directory
        for src_file in extracted_dir.iterdir():
            if src_file.is_file():
                dest_file = out_path / src_file.name
                shutil.copy2(src_file, dest_file)

        # Copy compat source files if needed
        if result.compat_functions:
            injector = CompatInjector(compat_path)
            compat_files = injector.resolve_functions(result.compat_functions)
            extra_files = injector.get_extra_files(result.compat_functions)
            all_compat = list(compat_files) + extra_files
            for compat_file in all_compat:
                dest_file = out_path / compat_file.name
                shutil.copy2(compat_file, dest_file)
            console.print(f"[bold]Compat sources:[/bold] {len(all_compat)} files ({', '.join(f.name for f in all_compat)})")

        # Copy patch files from mogrix patches directory if add_patch is specified
        patch_files = []
        if result.add_patches:
            patches_pkg_dir = PATCHES_DIR / "packages" / spec.name
            for patch_name in result.add_patches:
                patch_path = patches_pkg_dir / patch_name
                if not patch_path.exists():
                    patch_path = PATCHES_DIR / "shared" / patch_name
                if patch_path.exists():
                    dest_file = out_path / patch_name
                    shutil.copy2(patch_path, dest_file)
                    patch_files.append(patch_name)
                else:
                    console.print(f"[yellow]Warning:[/yellow] Patch not found: {patch_name} (checked packages/{spec.name}/ and shared/)")
            if patch_files:
                console.print(f"[bold]Patches added:[/bold] {len(patch_files)} files ({', '.join(patch_files)})")

        # Copy extra source files from mogrix patches directory if add_source is specified
        source_files = []
        if result.add_sources:
            patches_pkg_dir = PATCHES_DIR / "packages" / spec.name
            for source_name in result.add_sources:
                source_path = patches_pkg_dir / source_name
                if not source_path.exists():
                    # Fallback to shared patches directory
                    source_path = PATCHES_DIR / "shared" / source_name
                if source_path.exists():
                    dest_file = out_path / source_name
                    shutil.copy2(source_path, dest_file)
                    source_files.append(source_name)
                else:
                    console.print(f"[yellow]Warning:[/yellow] Source not found: {source_name} (checked packages/{spec.name}/ and shared/)")
            if source_files:
                console.print(f"[bold]Sources added:[/bold] {len(source_files)} files ({', '.join(source_files)})")

        # Regenerate spec content with patches/sources if any were added
        if patch_files or source_files:
            write_result = _generate_converted_spec(
                spec, result, headers_path, compat_path, patch_files, source_files
            )
            content = write_result.content

        # Validate converted spec
        if validate:
            _validate_spec_content(content, spec_path.name, strict)

        # Source scan: check for unaddressed IRIX issues
        if source_scan:
            _run_source_analysis(
                extracted_dir,
                handled_compat_functions=result.compat_functions,
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

        # Copy SRPM to repo directory (~/mogrix_outputs/SRPMS/)
        srpms_repo = MOGRIX_OUTPUTS / "SRPMS"
        srpms_repo.mkdir(parents=True, exist_ok=True)
        repo_srpm = srpms_repo / new_srpm.name
        shutil.copy2(new_srpm, repo_srpm)

        # Summary
        console.print("\n[bold green]Conversion complete![/bold green]")
        console.print(f"[bold]Workspace:[/bold] {out_path}")
        console.print(f"[bold]SRPM (repo):[/bold] {repo_srpm}")
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
    source_files: list[str] | None = None,
) -> "WriteResult":
    """Generate the converted spec content.

    Returns a WriteResult with .content (str) and match tracking info.
    """
    from mogrix.emitter.spec import WriteResult  # noqa: F811
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
        # Generate Patch entries (start at 500 to avoid conflicts with existing patches)
        patch_entries = []
        patch_cmds = []
        for i, patch_name in enumerate(patch_files):
            patch_num = 500 + i
            patch_entries.append(f"Patch{patch_num}: {patch_name}")
            patch_cmds.append(f"%patch -P{patch_num} -p1")
        patch_sources = "\n".join(patch_entries)
        patch_prep = "\n".join(patch_cmds)

    # Generate extra source entries (no prep commands - sources are just copied)
    extra_sources = None
    if source_files:
        source_entries = []
        for i, source_name in enumerate(source_files):
            source_num = 200 + i
            source_entries.append(f"Source{source_num}: {source_name}")
        extra_sources = "\n".join(source_entries)

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
        extra_sources=extra_sources,
        ac_cv_overrides=result.ac_cv_overrides if result.ac_cv_overrides else None,
        drop_requires=result.drop_requires if result.drop_requires else None,
        add_requires=result.add_requires if result.add_requires else None,
        remove_lines=result.remove_lines if result.remove_lines else None,
        rpm_macros=result.rpm_macros if result.rpm_macros else None,
        export_vars=result.export_vars if result.export_vars else None,
        extra_cflags=result.extra_cflags if result.extra_cflags else None,
        skip_find_lang=result.skip_find_lang,
        skip_check=result.skip_check,
        install_cleanup=result.install_cleanup if result.install_cleanup else None,
        spec_replacements=result.spec_replacements if result.spec_replacements else None,
    )


def _validate_spec_content(content: str, filename: str, strict: bool) -> None:
    """Validate converted spec content and display results.

    Args:
        content: Spec file content string.
        filename: Name for display purposes.
        strict: If True, treat warnings as errors and abort.
    """
    from mogrix.validators.spec import SpecValidator

    validator = SpecValidator()
    result = validator.validate(content, filename)

    if result.errors:
        console.print("\n[bold red]Spec validation errors:[/bold red]")
        for issue in result.errors:
            line_info = f" (line {issue.line})" if issue.line else ""
            console.print(f"  [red]✗[/red] {issue.message}{line_info}")

    if result.warnings:
        console.print("\n[bold yellow]Spec validation warnings:[/bold yellow]")
        for issue in result.warnings:
            line_info = f" (line {issue.line})" if issue.line else ""
            console.print(f"  [yellow]![/yellow] {issue.message}{line_info}")

    if result.is_valid and not result.warnings:
        console.print("\n[bold green]✓ Spec validation passed[/bold green]")
    elif result.is_valid and not strict:
        console.print(
            f"\n[bold yellow]Spec validation: {len(result.warnings)} warning(s)[/bold yellow]"
        )
    elif not result.is_valid or (strict and result.warnings):
        severity = "errors" if result.errors else "warnings (strict mode)"
        console.print(f"\n[bold red]✗ Spec validation failed: {severity}[/bold red]")
        raise SystemExit(1)


def _run_source_analysis(
    extracted_dir: Path,
    handled_compat_functions: list[str] | None = None,
    handled_rules: dict | None = None,
    show_handled: bool = False,
) -> None:
    """Scan source tarballs in an extracted SRPM directory for IRIX issues.

    Args:
        extracted_dir: Directory containing extracted SRPM files.
        handled_compat_functions: Compat functions already in rules (convert mode).
        handled_rules: Full package rules dict for cross-referencing.
        show_handled: If True, show handled findings (analyze mode).
    """
    import shutil
    import tempfile

    from mogrix.analyzers.source import SourceAnalyzer

    # Check for ripgrep
    if shutil.which("rg") is None:
        console.print("[dim]Skipping source scan: ripgrep (rg) not found[/dim]")
        return

    # Find source tarballs
    tarball_exts = (".tar.gz", ".tgz", ".tar.bz2", ".tbz2", ".tar.xz", ".txz", ".tar")
    tarballs = [
        f for f in extracted_dir.iterdir()
        if f.is_file() and any(f.name.lower().endswith(ext) for ext in tarball_exts)
    ]

    if not tarballs:
        return

    analyzer = SourceAnalyzer()
    all_findings = []

    for tarball in tarballs:
        scan_dir = Path(tempfile.mkdtemp(prefix="mogrix-scan-"))
        try:
            result = analyzer.scan_tarball(
                tarball,
                scan_dir,
                handled_compat_functions=handled_compat_functions,
                handled_rules=handled_rules,
            )
            all_findings.extend(result.findings)
        finally:
            shutil.rmtree(scan_dir, ignore_errors=True)

    if not all_findings:
        console.print("\n[bold green]Source scan:[/bold green] No IRIX compatibility issues found")
        return

    unhandled = [f for f in all_findings if not f.handled]
    handled = [f for f in all_findings if f.handled]

    # Display unhandled findings grouped by severity
    if unhandled:
        console.print(f"\n[bold]Source Scan: {len(unhandled)} finding(s)[/bold]")

        for severity, style in [("error", "red"), ("warning", "yellow"), ("info", "cyan")]:
            sev_findings = [f for f in unhandled if f.severity == severity]
            if not sev_findings:
                continue

            console.print(f"\n  [{style}]{severity.upper()}S ({len(sev_findings)}):[/{style}]")

            # Group by check_id for cleaner output
            by_check: dict[str, list] = {}
            for f in sev_findings:
                by_check.setdefault(f.check_id, []).append(f)

            for check_id, findings in by_check.items():
                first = findings[0]
                console.print(f"    [{style}]{first.message}[/{style}]")
                if first.fix:
                    console.print(f"    [dim]Fix: {first.fix}[/dim]")
                for f in findings[:5]:  # Show up to 5 locations
                    console.print(f"      {f.file}:{f.line}")
                if len(findings) > 5:
                    console.print(f"      [dim]...and {len(findings) - 5} more[/dim]")

    # Display handled findings (analyze mode shows these for reference)
    if show_handled and handled:
        console.print(f"\n  [green]HANDLED ({len(handled)}):[/green]")
        by_check: dict[str, list] = {}
        for f in handled:
            by_check.setdefault(f.check_id, []).append(f)
        for check_id, findings in by_check.items():
            first = findings[0]
            console.print(f"    [dim]{first.message} — {first.handled_by}[/dim]")

    if not unhandled:
        console.print(
            f"\n[bold green]Source scan:[/bold green] "
            f"{len(handled)} finding(s), all handled by existing rules"
        )


@main.command("validate-spec")
@click.argument("spec_file", type=click.Path(exists=True))
@click.option(
    "--strict",
    is_flag=True,
    help="Treat warnings as errors",
)
def validate_spec(spec_file: str, strict: bool):
    """Validate a spec file for structural issues.

    Uses the specfile library to check that the spec can be parsed and
    has the required tags and sections.
    """
    from mogrix.validators.spec import SpecValidator

    spec_path = Path(spec_file)
    content = spec_path.read_text()

    console.print(f"[bold]Validating:[/bold] {spec_path.name}\n")

    validator = SpecValidator()
    result = validator.validate(content, spec_path.name)

    if result.errors:
        console.print("[bold red]Errors:[/bold red]")
        for issue in result.errors:
            line_info = f" (line {issue.line})" if issue.line else ""
            console.print(f"  [red]✗[/red] {issue.message}{line_info}")
        console.print()

    if result.warnings:
        console.print("[bold yellow]Warnings:[/bold yellow]")
        for issue in result.warnings:
            line_info = f" (line {issue.line})" if issue.line else ""
            console.print(f"  [yellow]![/yellow] {issue.message}{line_info}")
        console.print()

    console.print("[bold]Summary:[/bold]")
    console.print(f"  Errors: [red]{len(result.errors)}[/red]")
    console.print(f"  Warnings: [yellow]{len(result.warnings)}[/yellow]")

    if result.is_valid and (not strict or not result.warnings):
        console.print("\n[bold green]✓ Spec validation passed[/bold green]")
    else:
        console.print("\n[bold red]✗ Spec validation failed[/bold red]")
        raise SystemExit(1)


@main.command()
@click.argument("targets", nargs=-1, required=True, type=click.Path(exists=True))
@click.option(
    "--config",
    "-c",
    type=click.Path(exists=True),
    default=None,
    help="Custom rpmlint configuration file",
)
def lint(targets: tuple[str, ...], config: str | None):
    """Lint RPMs or spec files using rpmlint.

    TARGETS are RPM files, spec files, or directories containing RPMs.

    Examples:
        mogrix lint ~/mogrix_outputs/RPMS/bash-5.2.26-3.mips.rpm
        mogrix lint ~/mogrix_outputs/RPMS/
        mogrix lint my-package.spec
    """
    import shutil
    import subprocess

    rpmlint_bin = shutil.which("rpmlint")
    if rpmlint_bin is None:
        console.print("[red]Error: rpmlint not found.[/red]")
        console.print("Install it with: uv pip install rpmlint")
        raise SystemExit(1)

    # Expand directories to individual files
    files = []
    for target in targets:
        target_path = Path(target)
        if target_path.is_dir():
            files.extend(sorted(target_path.glob("*.rpm")))
            files.extend(sorted(target_path.glob("*.spec")))
        else:
            files.append(target_path)

    if not files:
        console.print("[yellow]No RPM or spec files found[/yellow]")
        return

    console.print(f"[bold]Linting {len(files)} file(s) with rpmlint...[/bold]\n")

    cmd = [rpmlint_bin]
    if config:
        cmd.extend(["-c", config])
    elif RPMLINT_CONFIG.exists():
        cmd.extend(["-c", str(RPMLINT_CONFIG)])
        console.print(f"[dim]Using config: {RPMLINT_CONFIG}[/dim]\n")
    cmd.extend(str(f) for f in files)

    result = subprocess.run(cmd, capture_output=True, text=True)

    # Display rpmlint output with Rich formatting
    for line in (result.stdout + result.stderr).splitlines():
        if ": E: " in line:
            console.print(f"  [red]{line}[/red]")
        elif ": W: " in line:
            console.print(f"  [yellow]{line}[/yellow]")
        else:
            console.print(f"  {line}")

    if result.returncode != 0:
        console.print(f"\n[bold yellow]rpmlint exited with code {result.returncode}[/bold yellow]")
    else:
        console.print("\n[bold green]✓ rpmlint passed[/bold green]")


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
@click.option(
    "--output-dir",
    "-o",
    type=click.Path(),
    default=None,
    help="Directory to copy built RPMs (default: ~/mogrix_outputs/RPMS/)",
)
@click.option(
    "--no-isolate",
    is_flag=True,
    help="Skip overlayfs staging isolation (use current staging as-is)",
)
@click.option(
    "--skip-dep-check",
    is_flag=True,
    help="Skip post-build ELF dependency validation",
)
@click.option(
    "--short-circuit",
    type=click.Choice(["build", "install", "binary"]),
    default=None,
    help="Resume a partial build: 'build' = -bc, 'install' = -bi, 'binary' = -bb (all with --short-circuit, skips %prep)",
)
def build(
    srpm: str,
    rpmbuild_dir: str | None,
    cross: bool,
    macros: str | None,
    dry_run: bool,
    output_dir: str | None,
    no_isolate: bool,
    skip_dep_check: bool,
    short_circuit: str | None,
):
    """Build a converted SRPM.

    SRPM is a .src.rpm file to build (typically from mogrix convert).

    Workflow:
        1. mogrix fetch popt -y
        2. mogrix convert ~/mogrix_inputs/SRPMS/popt-*.src.rpm
        3. mogrix build <converted>.src.rpm --cross
        4. mogrix stage ~/mogrix_outputs/RPMS/popt*.rpm

    Use --cross to enable IRIX cross-compilation, which:
      - Uses the cross-toolchain at /opt/cross/bin/
      - Loads rpmmacros.irix from /opt/sgug-staging/
      - Targets IRIX 6.5 N32 ABI
    """
    import subprocess

    # Verify cross tools are in sync before building
    if cross and not dry_run:
        _check_tool_checksums(auto_redeploy=True)

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

    # Per-build staging isolation
    isolated = None
    build_deps = []
    if cross and not dry_run and not no_isolate:
        from mogrix.isolated_build import IsolatedStaging

        isolated = IsolatedStaging(
            base_staging=Path("/opt/sgug-staging"),
            rpms_dir=MOGRIX_OUTPUTS / "RPMS",
            rules_dir=RULES_DIR,
        )
        build_deps = isolated.resolve_build_deps(input_path)
        if build_deps:
            console.print(f"[bold]Build deps:[/bold] {', '.join(build_deps)}")

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
        # --target sets the RPM package OS/arch tags. Must use 'irix' (not 'irix6.5')
        # so IRIX rpm accepts the packages (it expects OS=irix).
        # Configure's --host needs 'irix6.5' for libtool shared lib support —
        # this is hardcoded in rpmmacros.irix's %_configure_args.
        cmd.extend(["--target", "mips-sgi-irix"])
        cmd.extend(["--define", "_target_cpu mips"])
        cmd.extend(["--define", "_target_os irix"])
        cmd.extend(["--define", "_arch mips"])

    # Add source/spec directories
    cmd.extend(["--define", f"_topdir {rpmbuild_path}"])

    if short_circuit:
        # Resume a partial build using existing BUILD dir and spec.
        # Requires the spec to already be in rpmbuild_path/SPECS/.
        # rpmbuild --short-circuit only works with -bc (build) and -bi (install).
        # To get full RPMs we chain: -bc --short-circuit then -bi --short-circuit,
        # then a final -bb to create the packages.
        specs_dir = rpmbuild_path / "SPECS"
        spec_files = list(specs_dir.glob("*.spec")) if specs_dir.exists() else []
        if not spec_files:
            console.print(f"[red]Error: No spec file found in {specs_dir}[/red]")
            console.print("[dim]--short-circuit requires a prior build attempt with spec in SPECS/[/dim]")
            raise SystemExit(1)
        if len(spec_files) > 1:
            console.print(f"[yellow]Warning: Multiple specs found, using {spec_files[0].name}[/yellow]")
        sc_spec = spec_files[0]

        if short_circuit == "build":
            cmd.extend(["-bc", "--short-circuit", str(sc_spec)])
            console.print(f"[bold cyan]Short-circuit:[/bold cyan] -bc --short-circuit (compile only)")
        elif short_circuit == "install":
            cmd.extend(["-bi", "--short-circuit", str(sc_spec)])
            console.print(f"[bold cyan]Short-circuit:[/bold cyan] -bi --short-circuit (install only)")
        elif short_circuit == "binary":
            # Full pipeline skipping %prep: compile → install → package.
            # rpmbuild --short-circuit only supports -bc and -bi, so we chain
            # three invocations. The base cmd already has all the defines/flags.
            _sc_base = list(cmd)  # copy before extending
            _sc_stages = [
                ("-bc", "--short-circuit", str(sc_spec)),
                ("-bi", "--short-circuit", str(sc_spec)),
                ("-bb", "--short-circuit", str(sc_spec)),
            ]
            console.print(f"[bold cyan]Short-circuit:[/bold cyan] -bc → -bi → -bb (full pipeline, skip %%prep)")
            # Stash for the runner below
            cmd = ("__chained__", _sc_base, _sc_stages)
        is_srpm = False  # don't fall through to --rebuild
    elif is_srpm:
        cmd.extend(["--rebuild", str(input_path)])
    else:
        cmd.extend(["-ba", str(spec_path)])

    _is_chained = isinstance(cmd, tuple) and cmd[0] == "__chained__"

    if dry_run:
        console.print("[bold]Dry run - would execute:[/bold]")
        if _is_chained:
            _, sc_base, sc_stages = cmd
            for stage_args in sc_stages:
                console.print(f"  {' '.join(sc_base + list(stage_args))}")
        else:
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
    if not is_srpm and not _is_chained:
        import shutil

        dest_spec = rpmbuild_path / "SPECS" / spec_path.name
        if spec_path != dest_spec:
            shutil.copy2(spec_path, dest_spec)
            cmd[-1] = str(dest_spec)

    console.print(f"[bold]Building:[/bold] {input_path.name}")
    if cross:
        console.print("[bold]Mode:[/bold] IRIX cross-compilation")
    if not _is_chained:
        console.print(f"[bold]Command:[/bold] {' '.join(cmd)}\n")

    # Build lock: prevent concurrent builds of the same package.
    # Acquired BEFORE quarantine so two processes can't both quarantine RPMs.
    import re as _re_lock
    import os as _os_lock
    _lock_pkg = None
    _lock_file = None
    if is_srpm:
        _m_lock = _re_lock.match(r"^(.+?)-[\d]", input_path.name)
        if _m_lock:
            _lock_pkg = _m_lock.group(1)
            _lock_file = rpmbuild_path / f".lock.{_lock_pkg}"
            if _lock_file.exists():
                try:
                    _lock_pid = int(_lock_file.read_text().strip())
                    # Check if the PID is still running
                    _os_lock.kill(_lock_pid, 0)
                    console.print(f"[red]Build lock exists for {_lock_pkg} (PID {_lock_pid} is running)[/red]")
                    console.print(f"[red]Another build of {_lock_pkg} is already in progress.[/red]")
                    console.print(f"[dim]Lock file: {_lock_file}[/dim]")
                    console.print("[dim]If this is stale, remove it manually.[/dim]")
                    raise SystemExit(1)
                except (ValueError, ProcessLookupError, PermissionError):
                    # PID is gone or invalid — stale lock, remove it
                    _lock_file.unlink(missing_ok=True)
            _lock_file.write_text(str(_os_lock.getpid()))

    # Quarantine old RPMs before building — restored on failure, cleaned on success.
    # Never delete RPMs: build failure → old RPMs survive.
    # Uses version-anchored glob to avoid quarantining RPMs from other source packages
    # (e.g., building "git" must not quarantine "git-lfs" which is a separate package).
    _quarantine_dir = None
    _quarantine_out_dir = None
    if is_srpm and not dry_run:
        import re as _re
        import time as _time
        # Extract name and version-release from SRPM: git-2.43.0-1.fc40.src.rpm → ("git", "2.43.0-1")
        _m = _re.match(r"^(.+?)-(\d[^-]*-\d[^.]*)\.", input_path.name)
        if _m:
            _pkg_prefix = _m.group(1)
            _pkg_vr = _m.group(2)  # version-release, e.g. "2.43.0-1"
            _quarantine_out_dir = Path(output_dir) if output_dir else MOGRIX_OUTPUTS / "RPMS"
            if _quarantine_out_dir.exists():
                # Anchor on version-release: "git*-2.43.0-1.*.rpm" matches git + git-core
                # but NOT git-lfs-3.5.1-1.mips.rpm (different version).
                _old = set(_quarantine_out_dir.glob(f"{_pkg_prefix}-{_pkg_vr}.*.rpm"))
                _old |= set(_quarantine_out_dir.glob(f"{_pkg_prefix}-*-{_pkg_vr}.*.rpm"))
                if _old:
                    _quarantine_dir = _quarantine_out_dir / ".quarantine" / _time.strftime("%m%d%H%M%S")
                    _quarantine_dir.mkdir(parents=True, exist_ok=True)
                    import shutil as _shutil_q
                    for old_rpm in sorted(_old):
                        console.print(f"[dim]Quarantining old RPM:[/dim] {old_rpm.name}")
                        _shutil_q.move(str(old_rpm), str(_quarantine_dir / old_rpm.name))

    # Snapshot RPMs before build so we can identify which ones are new
    rpms_dir = rpmbuild_path / "RPMS"
    pre_build_rpms = set(rpms_dir.glob("**/*.rpm")) if rpms_dir.exists() else set()

    try:
        if isinstance(cmd, tuple) and cmd[0] == "__chained__":
            # Short-circuit binary mode: chain -bc, -bi, -bb sequentially
            _, sc_base, sc_stages = cmd
            result = None
            stage_names = ["%build", "%install", "%package"]
            for stage_args, stage_name in zip(sc_stages, stage_names):
                stage_cmd = sc_base + list(stage_args)
                console.print(f"[bold]  → {stage_name}:[/bold] {' '.join(stage_args[:2])}")
                if isolated and build_deps:
                    result = isolated.build_with_fallback(stage_cmd, build_deps)
                else:
                    result = subprocess.run(stage_cmd, capture_output=True, text=True, errors='replace')
                if result.returncode != 0:
                    break
        elif isolated and build_deps:
            result = isolated.build_with_fallback(cmd, build_deps)
        else:
            result = subprocess.run(cmd, capture_output=True, text=True, errors='replace')

        if result.returncode == 0:
            import shutil

            console.print("\n[bold green]✓ Build succeeded[/bold green]")
            # Collect only newly built RPMs (not stale ones from prior builds)
            post_build_rpms = set(rpms_dir.glob("**/*.rpm")) if rpms_dir.exists() else set()
            rpms = sorted(post_build_rpms - pre_build_rpms)

            if rpms:
                console.print(f"\n[bold]Built RPMs:[/bold]")
                for rpm in rpms:
                    console.print(f"  {rpm}")

            # Post-build ELF dependency validation
            if cross and rpms and not skip_dep_check:
                from mogrix.validate_deps import validate_rpm_deps

                console.print("\n[bold]Validating ELF dependencies...[/bold]")
                out_rpms_check = Path(output_dir) if output_dir else MOGRIX_OUTPUTS / "RPMS"
                unresolved = validate_rpm_deps(rpms, out_rpms_check, IRIX_SYSROOT)
                if unresolved:
                    console.print("\n[bold red]✗ Unresolved NEEDED sonames:[/bold red]")
                    for rpm_name, missing in sorted(unresolved.items()):
                        console.print(f"  [bold]{rpm_name}:[/bold]")
                        for soname in missing:
                            console.print(f"    [red]• {soname}[/red]")
                    console.print(
                        "\n[dim]Use --skip-dep-check to bypass this check.[/dim]"
                    )
                    raise SystemExit(1)
                else:
                    console.print("[green]✓ All ELF dependencies resolve[/green]")

            # Copy RPMs to output directory
            out_rpms = Path(output_dir) if output_dir else MOGRIX_OUTPUTS / "RPMS"
            out_rpms.mkdir(parents=True, exist_ok=True)
            if rpms:
                console.print(f"\n[bold]Copying to:[/bold] {out_rpms}")
                for rpm in rpms:
                    dest = out_rpms / rpm.name
                    shutil.copy2(rpm, dest)
                    console.print(f"  → {dest.name}")
        else:
            # Check for missing dependencies
            combined_output = result.stdout + result.stderr
            if "Failed build dependencies" in combined_output:
                fetchable = _handle_missing_deps(combined_output, RULES_DIR)
                if fetchable:
                    _fetch_and_convert_deps(fetchable, input_path.parent)
            else:
                # Some other error
                console.print(combined_output, markup=False)
                console.print(f"\n[bold red]✗ Build failed (exit code {result.returncode})[/bold red]")
            raise SystemExit(result.returncode)

    except FileNotFoundError:
        console.print("[red]Error: rpmbuild not found. Install rpm-build package.[/red]")
        raise SystemExit(1)
    finally:
        # Restore quarantined RPMs that weren't replaced by the build.
        # On success: same-name RPMs already overwritten, restore is a no-op.
        # On failure: all quarantined RPMs are restored — no RPMs lost.
        if _quarantine_dir and _quarantine_dir.exists() and _quarantine_out_dir:
            import shutil as _shutil_r
            for _qrpm in sorted(_quarantine_dir.iterdir()):
                _dest = _quarantine_out_dir / _qrpm.name
                if not _dest.exists():
                    _shutil_r.move(str(_qrpm), str(_dest))
                    console.print(f"[dim]Restored RPM:[/dim] {_qrpm.name}")
            _shutil_r.rmtree(_quarantine_dir, ignore_errors=True)
        # Always clean up the build lock
        if _lock_file and _lock_file.exists():
            _lock_file.unlink(missing_ok=True)


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


@main.command("test-prep")
@click.argument("package")
@click.option(
    "--compare",
    is_flag=True,
    help="Compare current prep output against saved snapshot",
)
@click.option(
    "--snapshot-dir",
    type=click.Path(),
    default=None,
    help="Directory for snapshots (default: ~/mogrix_outputs/prep-snapshots/)",
)
@click.option(
    "--rpmbuild-dir",
    type=click.Path(),
    default=None,
    help="rpmbuild directory (default: ~/rpmbuild)",
)
def test_prep(package: str, compare: bool, snapshot_dir: str | None, rpmbuild_dir: str | None):
    """Snapshot or compare prep output for a package.

    Creates a snapshot of all files after running rpmbuild -bp (prep phase
    only). Use --compare to diff current prep output against a saved snapshot.

    This is the safety net for migrating sed to safepatch — run before and
    after changing rules to verify identical prep output.

    \b
    Workflow:
      1. mogrix test-prep webkitgtk          # Save baseline snapshot
      2. (modify rules: sed → safepatch)
      3. mogrix convert <srpm>               # Re-convert with new rules
      4. mogrix test-prep webkitgtk --compare # Verify identical output

    \b
    Examples:
      mogrix test-prep popt
      mogrix test-prep popt --compare
      mogrix test-prep webkitgtk --snapshot-dir /tmp/snapshots
    """
    import hashlib
    import json
    import subprocess
    from datetime import datetime

    snap_dir = Path(snapshot_dir) if snapshot_dir else MOGRIX_OUTPUTS / "prep-snapshots"
    snap_dir.mkdir(parents=True, exist_ok=True)
    snap_file = snap_dir / f"{package}.json"

    rpmbuild_path = Path(rpmbuild_dir) if rpmbuild_dir else Path.home() / "rpmbuild"

    # Find the converted SRPM in the conversion workspace
    converted_dir = MOGRIX_CONVERTED
    # Match package-VERSION (dash after name) or package-converted (for upstream)
    all_converted = sorted(converted_dir.glob(f"{package}*-converted/{package}*.src.rpm"))
    # Filter to exact package name: next char after package name must be - or .
    candidates = [
        c for c in all_converted
        if c.parent.name == f"{package}-converted"  # upstream: exactly pkg-converted
        or c.parent.name[len(package)] in ("-", ".")  # fedora: pkg-VERSION-converted
    ]
    if not candidates:
        # Fallback: try any match in the converted directory
        candidates = sorted(converted_dir.glob(f"{package}-*-converted/*.src.rpm"))
    if not candidates:
        console.print(f"[red]No converted SRPM found for '{package}' in {converted_dir}[/red]")
        console.print("\n[bold]Run first:[/bold]")
        console.print(f"  mogrix fetch {package} -y")
        console.print(f"  mogrix convert ~/mogrix_inputs/SRPMS/{package}*.src.rpm")
        raise SystemExit(1)

    if len(candidates) > 1:
        console.print(f"[yellow]Multiple SRPMs found for '{package}':[/yellow]")
        for c in candidates:
            console.print(f"  {c}")
        console.print(f"\n[bold]Using:[/bold] {candidates[-1]}")

    srpm_path = candidates[-1]

    # Clean BUILD directory to get a fresh prep
    import shutil
    build_dir = rpmbuild_path / "BUILD"
    if build_dir.exists():
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True, exist_ok=True)

    # Ensure rpmbuild directories exist
    for subdir in ["SOURCES", "SPECS", "BUILD", "RPMS", "SRPMS"]:
        (rpmbuild_path / subdir).mkdir(parents=True, exist_ok=True)

    # Install the SRPM to extract spec and sources into rpmbuild tree
    install_cmd = [
        "rpm", "-i",
        "--define", f"_topdir {rpmbuild_path}",
        str(srpm_path),
    ]
    try:
        result = subprocess.run(install_cmd, capture_output=True, text=True)
        if result.returncode != 0:
            console.print(f"[red]Failed to install SRPM:[/red]")
            console.print(result.stderr.strip(), markup=False)
            raise SystemExit(1)
    except FileNotFoundError:
        console.print("[red]Error: rpm not found.[/red]")
        raise SystemExit(1)

    # Find the spec file
    spec_files = list((rpmbuild_path / "SPECS").glob("*.spec"))
    if not spec_files:
        console.print("[red]No spec file found after installing SRPM[/red]")
        raise SystemExit(1)
    spec_path = spec_files[0]

    # Build rpmbuild -bp command (prep only)
    macros_path = IRIX_MACROS
    macro_chain = f"/usr/lib/rpm/macros:/usr/lib/rpm/macros.d/*:{macros_path}"

    cmd = [
        "rpmbuild",
        "--macros", macro_chain,
        "--nodeps",
        "--target", "mips-sgi-irix",
        "--define", "_target_cpu mips",
        "--define", "_target_os irix",
        "--define", "_arch mips",
        "--define", f"_topdir {rpmbuild_path}",
        "-bp", str(spec_path),
    ]

    console.print(f"[bold]Package:[/bold] {package}")
    console.print(f"[bold]SRPM:[/bold] {srpm_path.name}")
    console.print(f"[bold]Running:[/bold] rpmbuild -bp (prep only)")

    try:
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            console.print(f"\n[bold red]Prep failed (exit {result.returncode})[/bold red]")
            combined = result.stdout + result.stderr
            # Show last 30 lines of output
            lines = combined.strip().splitlines()
            for line in lines[-30:]:
                console.print(f"  {line}", markup=False)
            raise SystemExit(1)
    except FileNotFoundError:
        console.print("[red]Error: rpmbuild not found. Install rpm-build package.[/red]")
        raise SystemExit(1)

    console.print("[green]Prep succeeded[/green]")

    # Walk BUILD directory and hash all files
    file_hashes = {}
    build_contents = sorted(build_dir.rglob("*"))

    for fpath in build_contents:
        if fpath.is_file():
            rel = str(fpath.relative_to(build_dir))
            h = hashlib.sha256()
            try:
                with open(fpath, "rb") as f:
                    while True:
                        chunk = f.read(65536)
                        if not chunk:
                            break
                        h.update(chunk)
                file_hashes[rel] = h.hexdigest()
            except (OSError, PermissionError):
                file_hashes[rel] = "UNREADABLE"

    snapshot = {
        "package": package,
        "srpm": srpm_path.name,
        "timestamp": datetime.now().isoformat(),
        "file_count": len(file_hashes),
        "files": file_hashes,
    }

    if compare:
        # Compare mode
        if not snap_file.exists():
            console.print(f"[red]No snapshot found at {snap_file}[/red]")
            console.print(f"[bold]Run without --compare first:[/bold] mogrix test-prep {package}")
            raise SystemExit(1)

        with open(snap_file) as f:
            baseline = json.load(f)

        baseline_files = baseline["files"]
        current_files = file_hashes

        added = sorted(set(current_files) - set(baseline_files))
        removed = sorted(set(baseline_files) - set(current_files))
        changed = sorted(
            f for f in set(current_files) & set(baseline_files)
            if current_files[f] != baseline_files[f]
        )

        if not added and not removed and not changed:
            console.print(f"\n[bold green]IDENTICAL[/bold green] — prep output matches snapshot")
            console.print(f"  Baseline: {baseline['timestamp']} ({baseline['file_count']} files)")
            console.print(f"  Current:  {len(current_files)} files")
        else:
            console.print(f"\n[bold red]DIFFERENCES FOUND[/bold red]")
            console.print(f"  Baseline: {baseline['timestamp']} ({baseline['file_count']} files)")
            console.print(f"  Current:  {len(current_files)} files")

            if added:
                console.print(f"\n  [green]Added ({len(added)}):[/green]")
                for f in added[:20]:
                    console.print(f"    + {f}")
                if len(added) > 20:
                    console.print(f"    ... and {len(added) - 20} more")

            if removed:
                console.print(f"\n  [red]Removed ({len(removed)}):[/red]")
                for f in removed[:20]:
                    console.print(f"    - {f}")
                if len(removed) > 20:
                    console.print(f"    ... and {len(removed) - 20} more")

            if changed:
                console.print(f"\n  [yellow]Changed ({len(changed)}):[/yellow]")
                for f in changed[:20]:
                    console.print(f"    ~ {f}")
                if len(changed) > 20:
                    console.print(f"    ... and {len(changed) - 20} more")

            raise SystemExit(1)
    else:
        # Save snapshot
        with open(snap_file, "w") as f:
            json.dump(snapshot, f, indent=2)

        console.print(f"\n[bold]Snapshot saved:[/bold] {snap_file}")
        console.print(f"  Files: {len(file_hashes)}")
        console.print(f"  SRPM:  {srpm_path.name}")


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


@main.command("check-elf")
@click.argument("paths", nargs=-1, required=True, type=click.Path(exists=True))
@click.option(
    "--generate-fix",
    "-g",
    is_flag=True,
    help="Generate a fix_class_recs.c scaffold to stdout",
)
@click.option(
    "--json-output",
    is_flag=True,
    help="Machine-readable JSON output",
)
def check_elf(paths: tuple[str, ...], generate_fix: bool, json_output: bool):
    """Check MIPS ELF binaries for IRIX relocation issues.

    Detects R_MIPS_REL32 relocations in widget ClassRec structures that
    IRIX rld may silently fail to resolve, leaving function pointers NULL.

    PATHS can be RPM files, directories, or individual ELF binaries.
    """
    from mogrix.analyzers.elf import check_path, format_results, generate_fix_scaffold

    all_results = []
    for p in paths:
        path = Path(p)
        console.print(f"[bold]Checking:[/bold] {path.name}")
        results = check_path(path)
        all_results.extend(results)

    if not all_results:
        console.print("No MIPS ELF binaries found.")
        return

    if generate_fix:
        print(generate_fix_scaffold(all_results))
        return

    if json_output:
        import json

        data = []
        for r in all_results:
            entry = {
                "binary": str(r.binary_path),
                "type": r.elf_type,
                "rel32_total": r.rel32_total,
                "rel32_undef": r.rel32_undef,
                "class_recs": [
                    {
                        "name": f.name,
                        "address": f"0x{f.address:08x}",
                        "size": f.size,
                        "at_risk_relocs": [
                            {
                                "offset": f"+0x{rel.offset - f.address:03x}",
                                "symbol": rel.sym_name,
                            }
                            for rel in f.at_risk_relocs
                        ],
                    }
                    for f in r.class_rec_findings
                ],
                "warnings": r.warnings,
            }
            data.append(entry)
        print(json.dumps(data, indent=2))
        return

    console.print(format_results(all_results))

    # Summary
    total_findings = sum(len(r.class_rec_findings) for r in all_results)
    total_relocs = sum(r.total_at_risk_relocs for r in all_results)
    total_warnings = sum(len(r.warnings) for r in all_results)

    if total_findings:
        console.print(
            f"[bold yellow]⚠ {total_findings} ClassRec(s) with "
            f"{total_relocs} at-risk relocations[/bold yellow]"
        )
        console.print(
            "  Action: Create fix_class_recs.c with "
            "__attribute__((constructor)) patches."
        )
        console.print(
            "  Run with --generate-fix for a scaffold."
        )
        raise SystemExit(1)
    elif total_warnings:
        console.print(f"[yellow]{total_warnings} warning(s)[/yellow]")
    else:
        console.print("[bold green]✓ No IRIX relocation issues found[/bold green]")


@main.command("audit-rules")
@click.option(
    "--rules-dir",
    type=click.Path(exists=True),
    default=None,
    help="Path to rules directory",
)
@click.option(
    "--verbose",
    "-v",
    is_flag=True,
    help="Show WATCH-level entries (2 packages) in addition to CLASS candidates",
)
def audit_rules(rules_dir: str | None, verbose: bool):
    """Audit package rules for duplication and elevation candidates.

    Scans all package yamls and reports rules that appear in multiple
    packages, suggesting candidates for elevation to class or generic level.

    \b
    Thresholds:
      CLASS: 3+ packages share the same rule value
      WATCH: 2 packages share the same rule value (shown with -v)
    """
    from mogrix.analyzers.rules import RuleAuditor

    rules_path = Path(rules_dir) if rules_dir else RULES_DIR

    console.print(f"[bold]Auditing rules in:[/bold] {rules_path}\n")

    auditor = RuleAuditor(rules_path)
    report = auditor.audit()

    console.print(f"[bold]Packages scanned:[/bold] {report.packages_scanned}\n")

    # Show CLASS candidates
    class_candidates = report.class_candidates
    if class_candidates:
        table = Table(title="CLASS Candidates (3+ packages)")
        table.add_column("Rule Key", style="cyan")
        table.add_column("Value", style="white")
        table.add_column("#", style="bold yellow", justify="right")
        table.add_column("Packages", style="dim")

        for entry in class_candidates:
            pkgs = ", ".join(entry.packages[:6])
            if len(entry.packages) > 6:
                pkgs += f", ...+{len(entry.packages) - 6}"
            table.add_row(
                entry.rule_key,
                entry.value,
                str(entry.count),
                pkgs,
            )

        console.print(table)
    else:
        console.print("[green]No CLASS candidates found[/green]")

    # Show WATCH list (verbose only)
    if verbose:
        watch = report.watch_list
        if watch:
            console.print()
            table = Table(title="WATCH List (2 packages)")
            table.add_column("Rule Key", style="cyan")
            table.add_column("Value", style="white")
            table.add_column("#", style="dim", justify="right")
            table.add_column("Packages", style="dim")

            for entry in watch:
                table.add_row(
                    entry.rule_key,
                    entry.value,
                    str(entry.count),
                    ", ".join(entry.packages),
                )

            console.print(table)

    # Summary
    console.print(f"\n[bold]Summary:[/bold]")
    console.print(f"  CLASS candidates: [yellow]{len(class_candidates)}[/yellow]")
    console.print(f"  WATCH entries: [dim]{len(report.watch_list)}[/dim]")

    # Show existing classes
    classes_dir = rules_path / "classes"
    if classes_dir.exists():
        class_files = sorted(classes_dir.glob("*.yaml"))
        if class_files:
            console.print(f"\n[bold]Existing classes:[/bold]")
            for cf in class_files:
                console.print(f"  - {cf.stem}")

    # Show which rules are already in generic
    if report.generic_rules.get("ac_cv_overrides"):
        console.print(f"\n[bold]Already in generic.yaml ac_cv_overrides:[/bold]")
        for key, val in report.generic_rules["ac_cv_overrides"].items():
            console.print(f"  {key}={val}")


@main.command("rebuild-all")
@click.option(
    "--rules-dir",
    type=click.Path(exists=True),
    default=None,
    help="Path to rules directory",
)
@click.option("--resume", is_flag=True, help="Resume latest workspace (skip completed packages)")
@click.option("--workspace", type=click.Path(), default=None, help="Explicit workspace path (overrides auto-detect)")
@click.option("--dry-run", is_flag=True, help="Compute plan without building")
@click.option("--skip-gates", is_flag=True, help="Treat gate failures as warnings")
@click.option("--from-list", type=click.Path(exists=True), help="File with package names to build")
@click.option("--no-fail-fast", is_flag=True, help="Continue building after failures (default: stop at first failure)")
def rebuild_all_cmd(
    rules_dir: str | None,
    resume: bool,
    workspace: str | None,
    dry_run: bool,
    skip_gates: bool,
    from_list: str | None,
    no_fail_fast: bool,
):
    """Full dependency-ordered rebuild with quality gates.

    Each full rebuild gets a versioned workspace (~/mogrix_v11, ~/mogrix_v12, etc.).
    Use --resume to continue building in the latest workspace.

    Gate 0: Reset staging + cross-compilation setup.
    Gate 2: Build validation — ELF ABI, shebangs, hardcoded paths.

    Each package is: convert -> build -> gate 2 check -> stage.
    Packages are built in dependency order so downstream builds link
    against freshly-built upstream libraries.

    Examples:
      mogrix rebuild-all --dry-run            # Preview build order
      mogrix rebuild-all                      # New workspace (auto-increment)
      mogrix rebuild-all --resume             # Continue latest workspace
      mogrix rebuild-all --from-list pkgs.txt # Build specific packages
    """
    from mogrix.rebuild import rebuild_all

    # Verify cross tools are in sync before rebuilding
    if not dry_run:
        _check_tool_checksums(auto_redeploy=True)

    rules_path = Path(rules_dir) if rules_dir else RULES_DIR

    pkg_list = None
    if from_list:
        pkg_list = [
            line.strip()
            for line in Path(from_list).read_text().splitlines()
            if line.strip() and not line.startswith("#")
        ]

    ws = Path(workspace) if workspace else None

    rebuild_all(
        rules_dir=rules_path,
        workspace=ws,
        resume=resume,
        dry_run=dry_run,
        skip_gates=skip_gates,
        from_list=pkg_list,
        fail_fast=not no_fail_fast,
    )


@main.command("rebuild")
@click.argument("package")
@click.option(
    "--workspace",
    type=click.Path(exists=True),
    required=True,
    help="Workspace path (e.g. ~/mogrix_v10)",
)
@click.option(
    "--rules-dir",
    type=click.Path(exists=True),
    default=None,
    help="Path to rules directory",
)
@click.option("--skip-gates", is_flag=True, help="Treat gate failures as warnings")
def rebuild_cmd(
    package: str,
    workspace: str,
    rules_dir: str | None,
    skip_gates: bool,
):
    """Rebuild a single package within a workspace.

    Automatically detects prior build attempts and uses --short-circuit
    to skip %prep when possible (reusing cached object files).

    Updates gate-results so rebuild-all --resume sees it correctly.
    Stages built RPMs into the sysroot on success.

    Examples:
      mogrix rebuild webkitgtk --workspace ~/mogrix_v10
      mogrix rebuild curl --workspace ~/mogrix_v10 --skip-gates
    """
    from mogrix.rebuild import rebuild_one

    rules_path = Path(rules_dir) if rules_dir else RULES_DIR
    ws = Path(workspace)

    result = rebuild_one(
        package=package,
        rules_dir=rules_path,
        workspace=ws,
        skip_gates=skip_gates,
    )

    if not result.success or not result.gate2_passed:
        raise SystemExit(1)


@main.command("rebuild-order")
@click.option(
    "--rules-dir",
    type=click.Path(exists=True),
    default=None,
    help="Path to rules directory",
)
@click.option("--json", "as_json", is_flag=True, help="Output as JSON")
def rebuild_order(rules_dir: str | None, as_json: bool):
    """Compute global dependency-ordered build plan for all packages with rules.

    Uses the roadmap resolver to topologically sort all packages so that
    dependencies are built before dependents.
    """
    import json as json_mod

    from mogrix.repometa import RepoMetaCache
    from mogrix.roadmap import RoadmapResolver

    rules_path = Path(rules_dir) if rules_dir else RULES_DIR

    cache = RepoMetaCache(release="40")
    try:
        db = cache.ensure_index(refresh=False)
    except Exception as e:
        console.print(f"[red]Failed to load repo index: {e}[/red]")
        console.print("Run: mogrix roadmap <any-package> --refresh")
        raise SystemExit(1)

    loader = RuleLoader(rules_path)
    resolver = RoadmapResolver(
        db=db,
        rule_loader=loader,
        rules_dir=rules_path,
        rpms_dir=MOGRIX_OUTPUTS / "RPMS",
        stop_at_rules=True,
    )

    build_order, cycles = resolver.resolve_all()
    db.close()

    if as_json:
        console.print(json_mod.dumps({
            "build_order": build_order,
            "cycles": cycles,
            "total": len(build_order),
        }, indent=2))
        return

    console.print(f"[bold]Global Build Order ({len(build_order)} packages)[/bold]\n")
    for i, pkg in enumerate(build_order, 1):
        console.print(f"  {i:3d}. {pkg}")

    if cycles:
        console.print(f"\n[yellow]Dependency cycles ({len(cycles)}):[/yellow]")
        for cycle in cycles:
            console.print(f"  {' -> '.join(cycle)} -> {cycle[0]}")

    console.print(f"\n[bold]Total:[/bold] {len(build_order)} packages")


@main.command("audit-smoke-coverage")
@click.option(
    "--rules-dir",
    type=click.Path(exists=True),
    default=None,
    help="Path to rules directory",
)
def audit_smoke_coverage(rules_dir: str | None):
    """Audit smoke test coverage across all packages.

    Reports which packages have smoke tests, which are library-only,
    and which are missing smoke tests entirely.
    """
    import yaml

    rules_path = Path(rules_dir) if rules_dir else RULES_DIR
    packages_dir = rules_path / "packages"

    if not packages_dir.exists():
        console.print(f"[red]No packages directory at {packages_dir}[/red]")
        raise SystemExit(1)

    has_smoke = []
    library_only = []
    missing = []

    for yaml_file in sorted(packages_dir.glob("*.yaml")):
        with open(yaml_file) as f:
            try:
                data = yaml.safe_load(f)
            except yaml.YAMLError:
                continue

        if not data:
            continue

        pkg_name = data.get("package", yaml_file.stem)
        smoke = data.get("smoke_test")

        if smoke == "library-only":
            library_only.append(pkg_name)
        elif smoke:
            has_smoke.append(pkg_name)
        else:
            missing.append(pkg_name)

    total = len(has_smoke) + len(library_only) + len(missing)
    coverage = (len(has_smoke) + len(library_only)) / total * 100 if total else 0

    console.print(f"[bold]Smoke Test Coverage Audit[/bold]\n")
    console.print(f"  Packages scanned: {total}")
    console.print(f"  With smoke tests: [green]{len(has_smoke)}[/green]")
    console.print(f"  Library-only:     [cyan]{len(library_only)}[/cyan]")
    console.print(f"  [red]Missing:[/red]          [red]{len(missing)}[/red]")
    console.print(f"  Coverage:         {coverage:.0f}%\n")

    if missing:
        console.print(f"[bold red]Packages missing smoke tests ({len(missing)}):[/bold red]")
        for pkg in missing:
            console.print(f"  - {pkg}")

    if library_only:
        console.print(f"\n[bold cyan]Library-only packages ({len(library_only)}):[/bold cyan]")
        for pkg in library_only:
            console.print(f"  - {pkg}")


@main.command("pre-scan")
@click.option(
    "--rpm-dir",
    type=click.Path(exists=True),
    default=None,
    help="Directory containing built RPMs (default: ~/mogrix_outputs/RPMS/)",
)
def pre_scan(rpm_dir: str | None):
    """Pre-scan existing RPMs for Gate 2 issues.

    Checks shebangs, hardcoded paths, and ELF ABI in built RPMs
    BEFORE starting a clean rebuild. Fix rules first, then rebuild.
    """
    from mogrix.gates import pre_scan_rpms

    scan_dir = Path(rpm_dir) if rpm_dir else MOGRIX_OUTPUTS / "RPMS"

    if not scan_dir.exists():
        console.print(f"[red]RPM directory not found: {scan_dir}[/red]")
        raise SystemExit(1)

    rpms = sorted(f for f in scan_dir.glob("*.rpm") if not f.name.endswith(".src.rpm"))
    console.print(f"[bold]Pre-scanning {len(rpms)} RPMs in:[/bold] {scan_dir}\n")

    results = pre_scan_rpms(scan_dir)

    total_errors = 0
    total_warnings = 0
    failed_rpms = []

    for rpm_name, result in sorted(results.items()):
        errors = [i for i in result.issues if i.severity == "error"]
        warnings = [i for i in result.issues if i.severity == "warning"]
        total_errors += len(errors)
        total_warnings += len(warnings)

        if errors or warnings:
            failed_rpms.append(rpm_name)
            console.print(f"[bold]{rpm_name}[/bold]")
            for issue in result.issues:
                color = "red" if issue.severity == "error" else "yellow"
                console.print(f"  [{color}]{issue.severity}[/{color}] {issue.file}: {issue.message}")

    console.print(f"\n[bold]Summary:[/bold]")
    console.print(f"  RPMs scanned: {len(results)}")
    console.print(f"  RPMs with issues: [{'red' if failed_rpms else 'green'}]{len(failed_rpms)}[/{'red' if failed_rpms else 'green'}]")
    console.print(f"  Errors: [{'red' if total_errors else 'green'}]{total_errors}[/{'red' if total_errors else 'green'}]")
    console.print(f"  Warnings: [{'yellow' if total_warnings else 'green'}]{total_warnings}[/{'yellow' if total_warnings else 'green'}]")

    if total_errors:
        console.print(f"\n[red]Fix these issues in package rules before rebuilding.[/red]")
        raise SystemExit(1)


@main.command("check-conversion")
@click.argument("packages", nargs=-1)
@click.option(
    "--rules-dir",
    type=click.Path(exists=True),
    default=None,
    help="Path to rules directory",
)
@click.option(
    "--srpms-dir",
    type=click.Path(exists=True),
    default=None,
    help="Directory containing converted SRPMs (default: ~/mogrix_outputs/SRPMS/)",
)
@click.option("--all", "check_all", is_flag=True, help="Check all converted SRPMs")
def check_conversion(
    packages: tuple[str, ...],
    rules_dir: str | None,
    srpms_dir: str | None,
    check_all: bool,
):
    """Verify converted specs pass postcondition checks.

    Checks that declared rules had an observable effect and that no
    Linux-isms survived conversion (systemd, /usr/lib64, stack protector, etc.).

    Examples:
      mogrix check-conversion nano curl         # Check specific packages
      mogrix check-conversion --all             # Check all converted SRPMs
    """
    import subprocess
    import tempfile

    import yaml

    from mogrix.postconditions import check_postconditions

    rules_path = Path(rules_dir) if rules_dir else RULES_DIR
    scan_dir = Path(srpms_dir) if srpms_dir else MOGRIX_OUTPUTS / "SRPMS"

    if not scan_dir.exists():
        console.print(f"[red]SRPMs directory not found: {scan_dir}[/red]")
        raise SystemExit(1)

    # Determine which SRPMs to check
    if check_all:
        srpms = sorted(scan_dir.glob("*.src.rpm"))
    elif packages:
        srpms = []
        for pkg in packages:
            found = sorted(scan_dir.glob(f"{pkg}-[0-9]*.src.rpm"))
            if found:
                srpms.append(found[-1])
            else:
                console.print(f"[yellow]No converted SRPM found for {pkg}[/yellow]")
    else:
        console.print("[red]Specify packages or use --all[/red]")
        raise SystemExit(1)

    console.print(f"[bold]Checking {len(srpms)} converted SRPMs[/bold]\n")

    total_errors = 0
    total_warnings = 0
    failed_packages = []

    for srpm_path in srpms:
        # Extract package name from SRPM filename
        import re as _re
        m = _re.match(r"^(.+?)-[\d]", srpm_path.name)
        pkg_name = m.group(1) if m else srpm_path.stem

        # Extract spec from SRPM
        with tempfile.TemporaryDirectory() as tmpdir:
            subprocess.run(
                f"cd {tmpdir} && rpm2cpio {srpm_path} | cpio -idm '*.spec' 2>/dev/null",
                shell=True, capture_output=True,
            )
            specs = list(Path(tmpdir).rglob("*.spec"))
            if not specs:
                console.print(f"[yellow]{pkg_name}: no spec found in SRPM[/yellow]")
                continue
            spec_content = specs[0].read_text(errors="replace")

        # Load rules
        rule_file = rules_path / "packages" / f"{pkg_name}.yaml"
        rules = {}
        if rule_file.exists():
            try:
                with open(rule_file) as f:
                    pkg_data = yaml.safe_load(f) or {}
                rules = pkg_data.get("rules", {})
            except Exception:
                pass

        report = check_postconditions(pkg_name, rules, spec_content)

        errors = [i for i in report.issues if i.severity == "error"]
        warnings = [i for i in report.issues if i.severity == "warning"]
        total_errors += len(errors)
        total_warnings += len(warnings)

        if errors or warnings:
            failed_packages.append(pkg_name)
            console.print(f"[bold]{pkg_name}[/bold]")
            for issue in report.issues:
                color = "red" if issue.severity == "error" else "yellow"
                console.print(f"  [{color}]{issue.rule_type}: {issue.message}[/{color}]")

    console.print(f"\n[bold]Summary:[/bold]")
    console.print(f"  SRPMs checked: {len(srpms)}")
    console.print(f"  Packages with issues: [{'red' if failed_packages else 'green'}]{len(failed_packages)}[/{'red' if failed_packages else 'green'}]")
    console.print(f"  Errors: [{'red' if total_errors else 'green'}]{total_errors}[/{'red' if total_errors else 'green'}]")
    console.print(f"  Warnings: [{'yellow' if total_warnings else 'green'}]{total_warnings}[/{'yellow' if total_warnings else 'green'}]")

    if total_errors:
        raise SystemExit(1)


@main.command("promote-check")
@click.option("--rules-dir", type=click.Path(exists=True), default=None, help="Path to rules directory")
def promote_check(rules_dir: str | None):
    """Scan package rules for redundant or promotable entries.

    Reports three categories:

    \b
    REDUNDANT: Rules already in generic.yaml (remove from package YAML)
    PROMOTION CANDIDATE: Rules in 3+ packages but not in generic.yaml
    UNIQUE: Package-specific rules (fine as-is, not shown)

    Examples:
      mogrix promote-check
      mogrix promote-check --rules-dir /path/to/rules
    """
    import yaml

    rules_path = Path(rules_dir) if rules_dir else RULES_DIR

    # Load generic rules
    generic_path = rules_path / "generic.yaml"
    with open(generic_path) as f:
        generic_data = yaml.safe_load(f) or {}
    generic_rules = generic_data.get("generic", {})

    # Extract sets from generic
    generic_drop_br = set(generic_rules.get("drop_buildrequires", []))
    generic_drop_req = set(generic_rules.get("drop_requires", []))
    generic_remove_lines = set(generic_rules.get("remove_lines", []))
    generic_configure_disable = set(generic_rules.get("configure_disable", []))

    # Scan all package YAMLs
    pkg_dir = rules_path / "packages"
    pkg_files = sorted(pkg_dir.glob("*.yaml"))

    # Track occurrences: rule_type -> {value: [packages]}
    occurrences: dict[str, dict[str, list[str]]] = {
        "drop_buildrequires": {},
        "drop_requires": {},
        "remove_lines": {},
        "configure_disable": {},
        "configure_flags.remove": {},
    }
    redundant_entries = []  # (pkg, rule_type, value)

    for pkg_file in pkg_files:
        pkg_name = pkg_file.stem
        with open(pkg_file) as f:
            pkg_data = yaml.safe_load(f) or {}
        rules = pkg_data.get("rules", pkg_data) or {}

        for dep in rules.get("drop_buildrequires", []):
            if dep in generic_drop_br:
                redundant_entries.append((pkg_name, "drop_buildrequires", dep))
            else:
                occurrences["drop_buildrequires"].setdefault(dep, []).append(pkg_name)

        for dep in rules.get("drop_requires", []):
            if dep in generic_drop_req:
                redundant_entries.append((pkg_name, "drop_requires", dep))
            else:
                occurrences["drop_requires"].setdefault(dep, []).append(pkg_name)

        for pattern in rules.get("remove_lines", []):
            if pattern in generic_remove_lines:
                redundant_entries.append((pkg_name, "remove_lines", pattern))
            else:
                occurrences["remove_lines"].setdefault(pattern, []).append(pkg_name)

        for flag in rules.get("configure_disable", []):
            if flag in generic_configure_disable:
                redundant_entries.append((pkg_name, "configure_disable", flag))
            else:
                occurrences["configure_disable"].setdefault(flag, []).append(pkg_name)

        cfg = rules.get("configure_flags", {})
        for flag in (cfg or {}).get("remove", []):
            occurrences["configure_flags.remove"].setdefault(flag, []).append(pkg_name)

    # Report
    console.print("[bold]Promote Check Report[/bold]\n")

    if redundant_entries:
        console.print(f"[red bold]REDUNDANT ({len(redundant_entries)} entries)[/red bold]")
        console.print("[dim]Already in generic.yaml — should be removed from package YAML[/dim]")
        for pkg, rule_type, value in sorted(redundant_entries):
            console.print(f"  [red]{pkg}[/red]: {rule_type}: {value[:60]}")
    else:
        console.print("[green]No redundant entries found[/green]")

    console.print()

    promotion_count = 0
    for rule_type, value_pkgs in occurrences.items():
        for value, pkgs in sorted(value_pkgs.items()):
            if len(pkgs) >= 3:
                if promotion_count == 0:
                    console.print("[yellow bold]PROMOTION CANDIDATES (3+ packages)[/yellow bold]")
                    console.print("[dim]Consider moving to generic.yaml[/dim]")
                promotion_count += 1
                console.print(
                    f"  [yellow]{rule_type}[/yellow]: {value[:60]} "
                    f"({len(pkgs)} pkgs: {', '.join(pkgs[:5])}{'...' if len(pkgs) > 5 else ''})"
                )

    if promotion_count == 0:
        console.print("[green]No promotion candidates found[/green]")

    console.print()
    console.print(
        f"[bold]Summary:[/bold] {len(pkg_files)} packages scanned, "
        f"{len(redundant_entries)} redundant, {promotion_count} promotion candidates"
    )


@main.command("scan-defects")
@click.option(
    "--rpm-dir",
    type=click.Path(exists=True),
    default=None,
    help="Directory containing built RPMs (default: ~/mogrix_outputs/RPMS/)",
)
def scan_defects_cmd(rpm_dir: str | None):
    """Scan RPMs for ELF-level defects (Gate 3).

    Checks for old image-base, stack protector symbols, GNU version sections.
    These are CRITICAL defects that will crash on IRIX.

    Examples:
      mogrix scan-defects
      mogrix scan-defects --rpm-dir ~/mogrix_v13/mogrix_outputs/RPMS/
    """
    from mogrix.gates import gate3_defect_scan

    scan_dir = Path(rpm_dir) if rpm_dir else MOGRIX_OUTPUTS / "RPMS"

    if not scan_dir.exists():
        console.print(f"[red]RPM directory not found: {scan_dir}[/red]")
        raise SystemExit(1)

    console.print(f"[bold]Scanning RPMs for ELF defects:[/bold] {scan_dir}\n")
    report = gate3_defect_scan(scan_dir)

    crits = [i for i in report.issues if i.severity == "CRITICAL"]
    errors = [i for i in report.issues if i.severity == "ERROR"]

    if crits:
        console.print(f"[bold red]CRITICAL ({len(crits)})[/bold red]")
        for issue in sorted(crits, key=lambda i: i.rpm):
            console.print(f"  {issue.rpm}: {issue.elf}: {issue.message}")

    if errors:
        console.print(f"\n[bold red]ERRORS ({len(errors)})[/bold red]")
        for issue in sorted(errors, key=lambda i: i.rpm):
            console.print(f"  {issue.rpm}: {issue.elf}: {issue.message}")

    if not report.issues:
        console.print("[bold green]No defects found![/bold green]")

    console.print(f"\n[bold]Scanned:[/bold] {report.scanned} ELF files across {report.rpms_scanned} RPMs")
    console.print(f"  Critical: [{'red' if crits else 'green'}]{len(crits)}[/{'red' if crits else 'green'}]")
    console.print(f"  Errors: [{'red' if errors else 'green'}]{len(errors)}[/{'red' if errors else 'green'}]")

    if report.has_critical:
        raise SystemExit(1)


@main.command()
@click.argument("packages", nargs=-1, required=True)
@click.option(
    "--output-dir",
    "-o",
    type=click.Path(),
    default=None,
    help="Directory to save downloaded SRPMs (default: ~/mogrix_inputs/SRPMS/)",
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

    output_path = Path(output_dir) if output_dir else MOGRIX_INPUTS / "SRPMS"
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
        (CROSS_DIR / "bin" / "strip-verneed", staging_path / "bin" / "strip-verneed", "Strip GNU version sections"),
        (CROSS_DIR / "bin" / "fix-anon-relocs", staging_path / "bin" / "fix-anon-relocs", "Fix anonymous R_MIPS_REL32"),
        (CROSS_DIR / "rpmmacros.irix", staging_path.parent.parent / "rpmmacros.irix", "RPM macros"),
        (CROSS_DIR / "pkgconfig" / "pthread-stubs.pc", staging_path / "lib32" / "pkgconfig" / "pthread-stubs.pc", "pthread-stubs (IRIX has pthreads in libc)"),
        # Runtime libraries (cross-compiled from GCC 9.5.0 source)
        (CROSS_DIR / "lib32" / "libgcc_s.so.1", staging_path / "lib32" / "libgcc_s.so.1", "libgcc_s runtime (from GCC 9.5.0)"),
        (CROSS_DIR / "lib32" / "libstdc++.so.6", staging_path / "lib32" / "libstdc++.so.6", "libstdc++ runtime (from GCC 9.5.0)"),
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

    # Create dev symlinks for runtime libraries
    runtime_symlinks = [
        ("libgcc_s.so.1", "libgcc_s.so"),
        ("libstdc++.so.6", "libstdc++.so"),
    ]
    for target, link_name in runtime_symlinks:
        link_path = staging_path / "lib32" / link_name
        if link_path.exists() or link_path.is_symlink():
            link_path.unlink()
        link_path.symlink_to(target)
        console.print(f"  [green]✓[/green] Symlink: {link_name} → {target}")

    # Symlink IRIX sysroot directories into staging
    # The staging sysroot is a merge of IRIX system files + cross-compiled sgug files.
    # IRIX system headers and libs come from /opt/irix-sysroot and must always be
    # present for linking. Symlinking (not copying) ensures they survive stage --clean
    # and stay in sync with the sysroot tarball.
    irix_sysroot = Path(sysroot)
    sysroot_links = [
        (irix_sysroot / "usr" / "include", staging_path.parent.parent / "usr" / "include"),
        (irix_sysroot / "usr" / "lib32", staging_path.parent.parent / "usr" / "lib32"),
        (irix_sysroot / "lib32", staging_path.parent.parent / "lib32"),
    ]
    for target, link_path in sysroot_links:
        if not target.exists():
            console.print(f"  [yellow]![/yellow] Sysroot path not found: {target}")
            continue
        if link_path.is_symlink():
            if link_path.resolve() == target.resolve():
                console.print(f"  [dim]✓ Already linked: {link_path.name} → {target}[/dim]")
                continue
            link_path.unlink()
        elif link_path.is_dir():
            # Real directory exists (e.g. from old manual copy) — replace with symlink
            if not any(link_path.iterdir()):
                link_path.rmdir()
            else:
                console.print(f"  [yellow]![/yellow] {link_path} is a non-empty directory, skipping symlink")
                continue
        link_path.parent.mkdir(parents=True, exist_ok=True)
        link_path.symlink_to(target)
        console.print(f"  [green]✓[/green] Sysroot symlink: {link_path} → {target}")

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
    console.print("  4. Stage RPMs:    mogrix stage ~/mogrix_outputs/RPMS/*.rpm")


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
@click.option(
    "--force",
    is_flag=True,
    help="Force staging even if RPM contains protected files",
)
def stage(
    rpms: tuple[str, ...],
    staging_dir: str,
    clean: bool,
    list_staged: bool,
    with_devel: bool,
    force: bool,
):
    """Stage cross-compiled RPMs for dependency resolution.

    Install RPMs to the staging area so subsequent builds can find
    their headers and libraries.

    Workflow:
        1. mogrix fetch popt -y
        2. mogrix convert ~/mogrix_inputs/SRPMS/popt-*.src.rpm
        3. mogrix build <converted>.src.rpm --cross
        4. mogrix stage ~/mogrix_outputs/RPMS/popt*.rpm
        5. (Now dependent packages can find popt headers/libs)

    Examples:
        mogrix stage ~/mogrix_outputs/RPMS/popt-*.mips.rpm
        mogrix stage ~/mogrix_outputs/RPMS/*.rpm
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
        "usr/sgug/bin/fix-anon-relocs",
        "usr/sgug/bin/strip-verneed",
        "usr/sgug/include/dicl-clang-compat",
        "usr/sgug/include/mogrix-compat",
        "usr/sgug/lib32/libsoft_float_stubs.a",
        "usr/sgug/lib32/libmogrix_compat.so",
        "usr/sgug/lib32/dlmalloc.o",
        "usr/sgug/lib32/safe_mem.o",
        "usr/sgug/lib32/dso_handle.o",
        "usr/sgug/lib32/eh_frame_reg.o",
        "usr/sgug/lib32/pkgconfig/pthread-stubs.pc",
        "rpmmacros.irix",
    }

    # Protected file patterns — critical runtime/toolchain files that must NOT be
    # overwritten by `mogrix stage`. Uses fnmatch glob patterns against paths
    # relative to the staging root.
    PROTECTED_PATTERNS = {
        "usr/sgug/lib32/libstdc++*",
        "usr/sgug/lib32/libgcc_s*",
        "usr/sgug/lib32/libmogrix_compat.so",
        "usr/sgug/lib32/dlmalloc.o",
        "usr/sgug/lib32/crt*.o",
        "usr/sgug/lib32/dso_handle.o",
        "usr/sgug/lib32/eh_frame_reg.o",
        "usr/sgug/lib32/safe_mem.o",
        "usr/sgug/bin/irix-cc",
        "usr/sgug/bin/irix-cxx",
        "usr/sgug/bin/irix-ld",
        "usr/sgug/bin/fix-anon-relocs",
        "usr/sgug/bin/strip-verneed",
    }

    # Legacy set for backward compat with _list_staged_packages
    PREEXISTING_LIBS = {
        "libstdc++.so", "libstdc++.so.6",
        "libgcc_s.so", "libgcc_s.so.1",
    }

    PREEXISTING_HEADERS: set[str] = set()

    if list_staged:
        _list_staged_packages(staging_path, PREEXISTING_LIBS, PREEXISTING_HEADERS)
        return

    if clean:
        _clean_staged_packages(staging_path, PRESERVE, PREEXISTING_LIBS, PREEXISTING_HEADERS, PROTECTED_PATTERNS)
        return

    if not rpms:
        console.print("[yellow]No RPMs specified. Use --help for usage.[/yellow]")
        console.print("\nExamples:")
        console.print("  mogrix stage popt-1.19-6.mips.rpm  # Auto-includes popt-devel")
        console.print("  mogrix stage ~/mogrix_outputs/RPMS/*.rpm")
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
            # Pre-stage validation: check for protected file conflicts
            file_list = _get_rpm_file_list(rpm_file)
            conflicts = _check_protected_files(rpm_file, file_list, PROTECTED_PATTERNS)
            if conflicts and not force:
                console.print(f"  [red]✗ BLOCKED — RPM overwrites protected files:[/red]")
                for cf in conflicts:
                    console.print(f"    [red]{cf}[/red]")
                console.print(f"  [yellow]Skipping {rpm_file.name} (use --force to override)[/yellow]")
                continue
            elif conflicts and force:
                console.print(f"  [yellow]⚠ WARNING — overwriting protected files (--force):[/yellow]")
                for cf in conflicts:
                    console.print(f"    [yellow]{cf}[/yellow]")

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
            _record_staged_package(staging_path, rpm_file.name, file_list)

        except Exception as e:
            console.print(f"  [red]✗ Error:[/red] {e}")

    # Fix multiarch headers (create mips64 variants from x86_64)
    _fix_multiarch_headers(staging_path)

    console.print("\n[bold green]Staging complete![/bold green]")
    console.print("\nStaged libraries are now available for cross-compilation.")


MANIFEST_FILE = ".mogrix-staged.json"


def _get_rpm_file_list(rpm_path: Path) -> list[str]:
    """Get list of files in an RPM without extracting (rpm2cpio | cpio -t)."""
    import subprocess
    result = subprocess.run(
        f"rpm2cpio {rpm_path.absolute()} | cpio -t 2>/dev/null",
        shell=True,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        return []
    # cpio -t outputs paths with leading ./ — strip it
    return [f.lstrip("./") for f in result.stdout.strip().split("\n") if f.strip()]


def _check_protected_files(
    rpm_path: Path, file_list: list[str], protected_patterns: set[str]
) -> list[str]:
    """Check if any files in the RPM match protected patterns. Returns conflicts."""
    from fnmatch import fnmatch

    conflicts = []
    for filepath in file_list:
        for pattern in protected_patterns:
            if fnmatch(filepath, pattern):
                conflicts.append(filepath)
                break
    return conflicts


def _record_staged_package(staging_path: Path, rpm_filename: str, file_list: list[str] | None = None) -> None:
    """Record a staged RPM in the manifest file."""
    import json
    from datetime import datetime

    manifest_path = staging_path / MANIFEST_FILE
    manifest = {}
    if manifest_path.exists():
        try:
            manifest = json.loads(manifest_path.read_text())
        except (json.JSONDecodeError, OSError):
            manifest = {}

    if "packages" not in manifest:
        manifest["packages"] = {}

    # Parse package name from RPM filename (e.g., "ncurses-devel-6.4-3.mips.rpm")
    import re
    match = re.match(r"^(.+?)-(\d+[\d.]*(?:[-~]\w[\w.]*)*)\.\w+\.rpm$", rpm_filename)
    pkg_name = match.group(1) if match else rpm_filename

    entry = {
        "rpm": rpm_filename,
        "staged_at": datetime.now().isoformat(),
    }
    if file_list is not None:
        entry["files"] = file_list

    manifest["packages"][pkg_name] = entry

    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n")


def _clear_manifest(staging_path: Path) -> None:
    """Remove the staging manifest file."""
    manifest_path = staging_path / MANIFEST_FILE
    if manifest_path.exists():
        manifest_path.unlink()
        console.print("  [dim]Cleared staging manifest[/dim]")


def _show_manifest(staging_path: Path) -> None:
    """Show contents of the staging manifest."""
    import json

    manifest_path = staging_path / MANIFEST_FILE
    if not manifest_path.exists():
        console.print("[dim]No staging manifest found (packages staged before tracking was added)[/dim]")
        return

    try:
        manifest = json.loads(manifest_path.read_text())
    except (json.JSONDecodeError, OSError):
        console.print("[yellow]Manifest file is corrupt[/yellow]")
        return

    packages = manifest.get("packages", {})
    if not packages:
        console.print("[dim]No packages recorded in manifest[/dim]")
        return

    console.print(f"\n[bold]Manifest ({len(packages)} packages):[/bold]")
    for name, info in sorted(packages.items()):
        staged_at = info.get("staged_at", "unknown")
        console.print(f"  {name}: {info.get('rpm', '?')} (staged {staged_at})")


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

    # Show manifest (packages tracked by mogrix stage)
    _show_manifest(staging_path)


def _is_protected(filepath: str, protected_patterns: set[str]) -> bool:
    """Check if a file path (relative to staging root) matches any protected pattern."""
    from fnmatch import fnmatch
    for pattern in protected_patterns:
        if fnmatch(filepath, pattern):
            return True
    return False


def _clean_staged_packages(
    staging_path: Path,
    preserve: set,
    preexisting_libs: set,
    preexisting_headers: set,
    protected_patterns: set[str] | None = None,
):
    """Clean staged packages while preserving base setup."""
    import shutil

    lib_dir = staging_path / "usr" / "sgug" / "lib32"
    include_dir = staging_path / "usr" / "sgug" / "include"
    bin_dir = staging_path / "usr" / "sgug" / "bin"
    pkgconfig_dir = lib_dir / "pkgconfig"

    console.print(f"[bold]Cleaning staged packages in:[/bold] {staging_path}\n")

    removed_count = 0

    def is_protected_lib(name: str) -> bool:
        """Check if a lib32/ file is protected (by pattern or legacy name set)."""
        if name in preexisting_libs or name == "libmogrix_compat.so":
            return True
        if protected_patterns:
            relpath = f"usr/sgug/lib32/{name}"
            return _is_protected(relpath, protected_patterns)
        return False

    # Clean libraries (keeping protected and soft_float_stubs)
    if lib_dir.exists():
        for lib in lib_dir.glob("*.so*"):
            if not is_protected_lib(lib.name):
                console.print(f"  Removing: {lib.name}")
                lib.unlink()
                removed_count += 1

        # Remove .a files that aren't protected
        for lib in lib_dir.glob("*.a"):
            if not is_protected_lib(lib.name) and not lib.name.startswith("libsoft_float"):
                console.print(f"  Removing: {lib.name}")
                lib.unlink()
                removed_count += 1

        # Remove .la files
        for la in lib_dir.glob("*.la"):
            if not is_protected_lib(la.name):
                console.print(f"  Removing: {la.name}")
                la.unlink()
                removed_count += 1

        # Preserve .o files that are protected (dlmalloc.o, crt*.o, dso_handle.o)
        for obj in lib_dir.glob("*.o"):
            if not is_protected_lib(obj.name):
                console.print(f"  Removing: {obj.name}")
                obj.unlink()
                removed_count += 1

    # Clean pkgconfig files
    if pkgconfig_dir.exists():
        for pc in pkgconfig_dir.glob("*.pc"):
            # Keep only pthread-stubs.pc (deployed by setup-cross)
            if pc.name not in {"pthread-stubs.pc"}:
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
        # Remove header subdirectories that aren't pre-existing or compat
        preserve_include_dirs = preexisting_headers | {"dicl-clang-compat", "mogrix-compat", "c++"}
        for subdir in include_dir.iterdir():
            if subdir.name in preserve_include_dirs:
                continue
            if subdir.is_symlink():
                console.print(f"  Removing: include/{subdir.name} (symlink)")
                subdir.unlink()
                removed_count += 1
            elif subdir.is_dir():
                console.print(f"  Removing: include/{subdir.name}/")
                shutil.rmtree(subdir)
                removed_count += 1

    # Clean lib32 subdirectories that were installed by RPMs
    # (skip regular files and file symlinks — handled by *.so*/*.a/*.la globs above)
    if lib_dir.exists():
        preserve_lib_dirs = {"pkgconfig"}
        for subdir in lib_dir.iterdir():
            if subdir.name in preserve_lib_dirs:
                continue
            # Only remove actual directories (not symlinks to files like libgcc_s.so)
            if subdir.is_dir() and not subdir.is_symlink():
                console.print(f"  Removing: lib32/{subdir.name}/")
                shutil.rmtree(subdir)
                removed_count += 1

    # Clean binaries (keeping wrappers)
    if bin_dir.exists():
        preserve_bins = {"irix-cc", "irix-cxx", "irix-ld", "fix-anon-relocs", "strip-verneed"}
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

    # Clear the manifest
    _clear_manifest(staging_path)

    if removed_count > 0:
        console.print(f"\n[bold green]Cleaned {removed_count} items[/bold green]")
    else:
        console.print("[dim]Nothing to clean[/dim]")

    console.print("\n[bold]Preserved:[/bold]")
    console.print("  - Compiler wrappers (irix-cc, irix-cxx, irix-ld)")
    console.print("  - Compat headers (dicl-clang-compat, mogrix-compat)")
    console.print("  - libsoft_float_stubs.a")
    console.print("  - Runtime libraries (libstdc++, libgcc_s, libmogrix_compat)")
    console.print("  - CRT objects (crt*.o, dlmalloc.o, dso_handle.o)")


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


@main.command()
@click.argument("package_name")
@click.option("--refresh", is_flag=True, help="Re-download repo metadata and rebuild index")
@click.option("--json", "output_json", is_flag=True, help="Output as JSON")
@click.option("--tree", "output_tree", is_flag=True, help="Show dependency tree")
@click.option(
    "--diff",
    "diff_file",
    type=click.Path(exists=True),
    default=None,
    help="Compare against previous JSON output",
)
@click.option("--depth", type=int, default=0, help="Max recursion depth (0=unlimited)")
@click.option("--stop-at-rules", is_flag=True, help="Stop recursion at packages with existing rules")
@click.option("--show-satisfied", is_flag=True, help="Include built packages in build order")
@click.option("--list-drops", is_flag=True, help="Show all auto-dropped deps with reasons")
@click.option("--release", default="40", help="Fedora release (default: 40)")
@click.option("--base-url", default=None, help="Override base URL for repo metadata")
def roadmap(
    package_name: str,
    refresh: bool,
    output_json: bool,
    output_tree: bool,
    diff_file: str | None,
    depth: int,
    stop_at_rules: bool,
    show_satisfied: bool,
    list_drops: bool,
    release: str,
    base_url: str | None,
):
    """Show the build roadmap for a target package.

    Computes the full transitive build dependency graph for PACKAGE_NAME
    against FC40 repo metadata, then diffs it against mogrix state (rules,
    built RPMs, sysroot) and outputs a topologically sorted build plan.

    \b
    Examples:
      mogrix roadmap gdb             # What's needed to build gdb?
      mogrix roadmap webkit2gtk3     # Full WebKit dependency chain
      mogrix roadmap popt            # Should show fully built
      mogrix roadmap gdb --json      # Machine-readable output
      mogrix roadmap gdb --tree      # Visual dependency tree
    """
    from mogrix.repometa import RepoMetaCache
    from mogrix.roadmap import (
        RoadmapResolver,
        format_diff,
        format_json,
        format_text,
        format_tree,
    )

    # Build or load the repo metadata index
    cache = RepoMetaCache(release=release, base_url=base_url)
    try:
        db = cache.ensure_index(refresh=refresh)
    except Exception as e:
        console.print(f"[red]Error building repo index: {e}[/red]")
        raise SystemExit(1)

    # Verify the target package exists in the index
    row = db.execute(
        "SELECT COUNT(*) FROM source_buildrequires WHERE source_package = ?",
        (package_name,),
    ).fetchone()
    if row[0] == 0:
        # Try as a binary package name -> find source
        src_row = db.execute(
            "SELECT DISTINCT source_package FROM binary_provides WHERE binary_package = ? LIMIT 1",
            (package_name,),
        ).fetchone()
        if src_row:
            real_name = src_row[0]
            console.print(
                f"[yellow]'{package_name}' is a binary package. "
                f"Using source package: {real_name}[/yellow]\n"
            )
            package_name = real_name
        else:
            console.print(
                f"[red]Package '{package_name}' not found in FC{release} repo metadata.[/red]\n"
                f"[dim]Try --refresh to re-download metadata, or check the package name.[/dim]"
            )
            raise SystemExit(1)

    # Resolve the graph
    rule_loader = RuleLoader(RULES_DIR)
    resolver = RoadmapResolver(
        db=db,
        rule_loader=rule_loader,
        rules_dir=RULES_DIR,
        rpms_dir=MOGRIX_OUTPUTS / "RPMS",
        stop_at_rules=stop_at_rules,
        max_depth=depth,
    )

    result = resolver.resolve(package_name)

    # Output
    if diff_file:
        console.print(format_diff(result, diff_file))
    elif output_json:
        # Print JSON to stdout (not via Rich, to allow piping)
        print(format_json(result))
    elif output_tree:
        format_tree(result, console)
    else:
        console.print(format_text(result, list_drops=list_drops, show_satisfied=show_satisfied))


@main.command("roadmap-check")
@click.argument("package_name", required=False)
@click.option("--all", "check_all", is_flag=True, help="Check all built packages")
@click.option("--json", "output_json", is_flag=True, help="Output as JSON")
@click.option(
    "--suggest",
    is_flag=True,
    help="Generate roadmap_config.yaml addition suggestions",
)
@click.option(
    "--min-freq",
    default=3,
    type=int,
    help="Minimum frequency for suggestions (default: 3)",
)
@click.option("--refresh", is_flag=True, help="Re-download repo metadata")
@click.option("--release", default="40", help="Fedora release (default: 40)")
def roadmap_check(
    package_name: str | None,
    check_all: bool,
    output_json: bool,
    suggest: bool,
    min_freq: int,
    refresh: bool,
    release: str,
):
    """Validate roadmap predictions against build reality.

    For a built package, identifies false positives in its roadmap
    (packages predicted as needed but not actually required for the build).

    \b
    Examples:
      mogrix roadmap-check popt             # Check single package
      mogrix roadmap-check --all            # Check all 63 built packages
      mogrix roadmap-check --all --suggest  # Generate config suggestions
      mogrix roadmap-check --all --json     # Machine-readable output
    """
    from mogrix.repometa import RepoMetaCache
    from mogrix.roadmap_check import RoadmapChecker

    if not package_name and not check_all:
        console.print("[red]Error: specify a package name or use --all[/red]")
        raise SystemExit(1)

    cache = RepoMetaCache(release=release)
    db = cache.ensure_index(refresh=refresh)
    rule_loader = RuleLoader(RULES_DIR)

    checker = RoadmapChecker(
        db=db,
        rule_loader=rule_loader,
        rules_dir=RULES_DIR,
        rpms_dir=MOGRIX_OUTPUTS / "RPMS",
    )

    if check_all:
        with console.status("[bold cyan]Running roadmap checks..."):
            results = checker.check_all_built()
        aggregated = checker.aggregate_false_positives(results)

        if output_json:
            print(checker.format_json_report(results, aggregated))
        else:
            console.print(checker.format_aggregate_report(results, aggregated))

        if suggest:
            suggestions = checker.generate_suggestions(aggregated, min_freq=min_freq)
            console.print("")
            console.print(checker.format_suggestions(suggestions, min_freq=min_freq))
    else:
        with console.status(f"[bold cyan]Checking {package_name}..."):
            result = checker.check_package(package_name)

        if output_json:
            import json as json_mod

            data = {
                "target": result.target,
                "need_rules": result.total_need_rules,
                "false_positives": [
                    {
                        "name": fp.name,
                        "category": fp.suggested_category,
                        "confidence": fp.confidence,
                        "reason": fp.reason,
                    }
                    for fp in result.false_positives
                ],
            }
            print(json_mod.dumps(data, indent=2))
        else:
            console.print(checker.format_check_result(result))


@main.command("batch-build")
@click.option(
    "--from-list",
    "list_file",
    type=click.Path(exists=True),
    default=None,
    help="File with package names, one per line",
)
@click.option(
    "--target",
    type=str,
    default=None,
    help="Target package for roadmap-driven build",
)
@click.option(
    "--dry-run",
    is_flag=True,
    help="Show what would be built without building",
)
@click.option(
    "--no-generate-rules",
    is_flag=True,
    help="Skip candidate rule generation for packages without rules",
)
@click.option(
    "--stop-on-error",
    is_flag=True,
    help="Stop on first build failure",
)
@click.option(
    "--output-report",
    type=click.Path(),
    default=None,
    help="Write JSON report to file",
)
@click.option(
    "--skip-fetch",
    is_flag=True,
    help="Only build packages with already-fetched SRPMs",
)
@click.option(
    "--no-skip-built",
    is_flag=True,
    help="Rebuild packages even if RPMs already exist",
)
@click.option(
    "--build-timeout",
    type=int,
    default=600,
    help="Kill build after N seconds (default: 600)",
)
@click.option("--release", default="40", help="Fedora release (default: 40)")
@click.option("--base-url", default=None, help="Override base URL for SRPM fetching")
def batch_build(
    list_file: str | None,
    target: str | None,
    dry_run: bool,
    no_generate_rules: bool,
    stop_on_error: bool,
    output_report: str | None,
    skip_fetch: bool,
    no_skip_built: bool,
    build_timeout: int,
    release: str,
    base_url: str | None,
):
    """Batch fetch, convert, and build packages for IRIX.

    Supports two modes:

    \b
    List mode: Process packages from a file, one name per line.
      mogrix batch-build --from-list packages.txt

    \b
    Roadmap mode: Resolve dependencies for a target and build them all.
      mogrix batch-build --target gdb

    \b
    Pipeline per package:
      1. Fetch SRPM from Fedora (if not already downloaded)
      2. Generate candidate rules (if no rules/packages/<pkg>.yaml exists)
      3. Convert SRPM with mogrix rules
      4. Build with rpmbuild --cross

    Packages without rules get candidate YAML in rules/candidates/ for
    human review. Packages that fail are classified and reported.
    The batch always moves on — it never blocks on a single failure.

    \b
    Workflow:
      mogrix batch-build --from-list tier1.txt --output-report report.json
      # Review rules/candidates/*.yaml, promote to rules/packages/
      mogrix batch-build --from-list tier1.txt   # rebuilds only what's new
    """
    from mogrix.batch_build import (
        BatchBuilder,
        BatchOptions,
        BatchReport,
        print_report,
        write_json_report,
    )

    # Validate: exactly one mode required
    if list_file and target:
        console.print("[red]Error: Use --from-list OR --target, not both[/red]")
        raise SystemExit(1)
    if not list_file and not target:
        console.print("[red]Error: Specify --from-list <file> or --target <package>[/red]")
        raise SystemExit(1)

    options = BatchOptions(
        dry_run=dry_run,
        generate_rules=not no_generate_rules,
        stop_on_error=stop_on_error,
        skip_fetch=skip_fetch,
        skip_built=not no_skip_built,
        build_timeout=build_timeout,
        release=release,
        base_url=base_url,
    )

    builder = BatchBuilder(
        rules_dir=RULES_DIR,
        compat_dir=COMPAT_DIR,
        headers_dir=HEADERS_DIR,
        inputs_dir=MOGRIX_INPUTS,
        outputs_dir=MOGRIX_OUTPUTS,
    )

    # Resolve tasks based on mode
    if list_file:
        mode = "list"
        input_source = list_file
        console.print(f"[bold]Batch build from list:[/bold] {list_file}\n")
        tasks = builder.resolve_tasks_from_list(Path(list_file), options)
    else:
        mode = "roadmap"
        input_source = target
        console.print(f"[bold]Batch build for target:[/bold] {target}\n")
        tasks = builder.resolve_tasks_from_roadmap(target, options)

    if not tasks:
        console.print("[yellow]No packages to build[/yellow]")
        return

    # Show task summary
    has_rules = sum(1 for t in tasks if t.has_rules)
    need_rules = sum(1 for t in tasks if not t.has_rules)
    has_rpms = sum(1 for t in tasks if t.has_rpms)
    need_fetch = sum(1 for t in tasks if t.srpm_path is None)

    console.print(f"[bold]Packages:[/bold] {len(tasks)} total")
    console.print(f"  {has_rules} with rules, {need_rules} without rules")
    console.print(f"  {has_rpms} already built, {need_fetch} need fetch")
    if options.skip_built and has_rpms:
        console.print(f"  [dim]({has_rpms} will be skipped)[/dim]")
    console.print()

    if dry_run:
        console.print("[bold yellow]DRY RUN — no builds will be executed[/bold yellow]\n")

    # Validate cross env unless dry-run
    if not dry_run:
        staging_status = ensure_staging_ready(verbose=False)
        if not staging_status.is_ready:
            console.print("[red]Staging environment is not ready for cross-compilation[/red]")
            for err in staging_status.errors:
                console.print(f"  [red]![/red] {err}")
            console.print("\n[bold]Try running:[/bold] mogrix setup-cross")
            raise SystemExit(1)

    # Run the batch
    report = BatchReport(mode=mode, input_source=input_source)
    builder.run(tasks, options, report)

    # Display results
    console.print()
    print_report(report)

    # Write JSON report if requested
    if output_report:
        write_json_report(report, Path(output_report))


@main.command("create-srpm")
@click.argument("packages", nargs=-1, required=True)
@click.option(
    "--output-dir",
    "-o",
    type=click.Path(),
    default=None,
    help="Directory to save SRPMs (default: ~/mogrix_inputs/SRPMS/)",
)
def create_srpm(packages: tuple[str, ...], output_dir: str | None):
    """Create SRPMs from upstream sources for non-Fedora packages.

    Reads the upstream: block from each package's rules YAML,
    fetches source from git/tarball, generates a spec file,
    and packages into an SRPM.

    The resulting SRPMs enter the normal mogrix pipeline:
    create-srpm → convert → build --cross → stage

    \b
    Examples:
        mogrix create-srpm telescope
        mogrix create-srpm gmi100 gmid libretls
    """
    import tempfile

    from mogrix.emitter.srpm import SRPMEmitter
    from mogrix.upstream import UpstreamSource

    output_path = Path(output_dir) if output_dir else MOGRIX_INPUTS / "SRPMS"
    output_path.mkdir(parents=True, exist_ok=True)

    upstream = UpstreamSource(rules_dir=RULES_DIR)
    emitter = SRPMEmitter()

    success = []
    failed = []

    for pkg in packages:
        console.print(f"\n[bold]Creating SRPM for:[/bold] {pkg}")

        try:
            # Load upstream config from package rules YAML
            config = upstream.load_upstream_config(pkg)
            console.print(
                f"  [dim]upstream:[/dim] {config['url']} "
                f"(v{config['version']}, {config.get('build_system', 'unknown')})"
            )

            # Fetch source into a temp directory
            with tempfile.TemporaryDirectory(prefix="mogrix-upstream-") as tmpdir:
                work_dir = Path(tmpdir)
                tarball = upstream.fetch_source(config, work_dir)

                # Generate spec content
                spec_content = upstream.render_spec(config)
                spec_name = f"{pkg}.spec"

                # Emit SRPM
                srpm_path = emitter.emit_srpm(
                    spec_content=spec_content,
                    spec_name=spec_name,
                    sources=[tarball],
                    output_dir=output_path,
                )

            console.print(f"  [green]✓ Created:[/green] {srpm_path.name}")
            success.append((pkg, srpm_path))

        except Exception as e:
            console.print(f"  [red]✗ Failed:[/red] {e}")
            failed.append((pkg, str(e)))

    # Summary
    console.print(f"\n[bold]Summary:[/bold]")
    console.print(f"  Created: [green]{len(success)}[/green]")
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


@main.command()
@click.argument("packages", nargs=-1, required=True)
@click.option(
    "--output",
    "-o",
    type=click.Path(),
    default=None,
    help="Output directory (default: ~/mogrix_outputs/bundles/)",
)
@click.option(
    "--tarball",
    is_flag=True,
    help="Create .tar.gz instead of self-extracting .run",
)
@click.option(
    "--no-package",
    is_flag=True,
    help="Don't package — just build the bundle directory",
)
@click.option(
    "--include",
    "-i",
    multiple=True,
    help="Extra packages to include in the bundle",
)
@click.option(
    "--name",
    default=None,
    help="Suite name (for multi-package bundles, e.g., mogrix-smallweb)",
)
@click.option(
    "--sysroot",
    type=click.Path(exists=True),
    default="/opt/irix-sysroot",
    help="IRIX sysroot path for native lib detection",
)
@click.option(
    "--test/--no-test",
    default=True,
    help="Test bundle on IRIX after creation (default: enabled)",
)
@click.option(
    "--test-host",
    default="192.168.0.81",
    help="IRIX host for testing (default: 192.168.0.81)",
)
def bundle(
    packages: tuple[str, ...],
    output: str | None,
    tarball: bool,
    no_package: bool,
    include: tuple[str, ...],
    name: str | None,
    sysroot: str,
    test: bool,
    test_host: str,
):
    """Create a self-contained app bundle for IRIX.

    Bundles PACKAGES with all mogrix-built shared library dependencies
    into a self-contained directory with launcher scripts. Output is a
    self-extracting .run installer by default.

    \b
    Single app (produces .run installer):
        mogrix bundle nano

    \b
    Tarball output instead:
        mogrix bundle nano --tarball

    \b
    Suite (multiple apps in one bundle):
        mogrix bundle telescope snownews lynx --name mogrix-smallweb

    \b
    Extra packages (siblings not auto-detected):
        mogrix bundle openssh --include openssh-clients
    """
    from mogrix.bundle import BundleBuilder

    rpms_dir = MOGRIX_OUTPUTS / "RPMS"
    if not rpms_dir.is_dir():
        console.print(f"[red]RPMs directory not found: {rpms_dir}[/red]")
        raise SystemExit(1)

    # Verify symlink points to a valid workspace
    outputs_link = Path.home() / "mogrix_outputs"
    if outputs_link.is_symlink():
        target = outputs_link.resolve()
        if not target.is_dir():
            console.print(f"[red]~/mogrix_outputs symlink is broken: {outputs_link} → {target}[/red]")
            raise SystemExit(1)
        console.print(f"[dim]Using RPMs from: {target / 'RPMS'}[/dim]")

    output_dir = Path(output) if output else MOGRIX_OUTPUTS / "bundles"

    # First package is the "target", rest are extra root packages
    target_package = packages[0]
    extra = list(packages[1:]) + list(include)

    # Auto-infer suite name when multiple packages given without --name
    suite_name = name
    if len(packages) > 1 and not suite_name:
        suite_name = f"{target_package}-suite"

    # Determine output format
    if no_package:
        output_format = "directory"
    elif tarball:
        output_format = "tarball"
    else:
        output_format = "run"

    # Load package-level trampoline exclusions from rules
    trampoline_exclude: set[str] = set()
    for pkg in packages:
        rule_path = RULES_DIR / "packages" / f"{pkg}.yaml"
        if rule_path.exists():
            import yaml

            with open(rule_path) as f:
                rule_data = yaml.safe_load(f) or {}
            pkg_exclude = rule_data.get("bundle_trampoline_exclude", [])
            trampoline_exclude.update(pkg_exclude)

    builder = BundleBuilder(rpms_dir=rpms_dir, irix_sysroot=Path(sysroot))
    manifest = builder.create_bundle(
        target_package=target_package,
        output_dir=output_dir,
        extra_packages=extra if extra else None,
        output_format=output_format,
        suite_name=suite_name,
        trampoline_exclude=trampoline_exclude if trampoline_exclude else None,
    )

    # Auto-test on IRIX after bundle creation
    if test and manifest.bundle_dir and manifest.bundle_dir.is_dir():
        from mogrix.test import (
            TestDiscovery,
            ScriptGenerator,
            IRIXTestRunner,
            TestReport,
            print_report,
        )

        console.print("\n[bold]Testing bundle on IRIX...[/bold]")
        runner = IRIXTestRunner(host=test_host, user="root")
        ok, msg = runner.check_connectivity()
        if not ok:
            console.print(f"[yellow]IRIX not reachable ({msg}) — skipping test[/yellow]")
            return

        discovery = TestDiscovery(RULES_DIR)
        tests = discovery.discover(manifest.bundle_dir)
        if not tests:
            console.print("[yellow]No tests discovered — skipping[/yellow]")
            return

        remote_dir = "/tmp/mogrix-test"
        remote_bundle = f"{remote_dir}/{manifest.bundle_dir.name}"
        ok, msg = runner.deploy_bundle(manifest.bundle_dir, remote_dir)
        if not ok:
            console.print(f"[red]Deploy failed: {msg}[/red]")
            return

        generator = ScriptGenerator()
        script = generator.generate(tests)
        console.print(f"[dim]Running {len(tests)} tests on {test_host}...[/dim]")
        rc, stdout, stderr = runner.run_script(script, remote_bundle)
        runner.cleanup(remote_bundle)

        results = runner.parse_results(stdout)
        report = TestReport(bundle_name=manifest.bundle_dir.name, results=results)
        print_report(report, console)
        if report.failed > 0:
            console.print(f"\n[red]{report.failed} test(s) failed[/red]")


@main.command()
@click.argument("bundle_path", type=click.Path(exists=True))
@click.option(
    "--host",
    default="192.168.0.81",
    help="IRIX host IP/hostname",
)
@click.option(
    "--user",
    default="root",
    help="SSH user for IRIX",
)
@click.option(
    "--keep",
    is_flag=True,
    help="Don't clean up test bundle on IRIX after testing",
)
def test(bundle_path: str, host: str, user: str, keep: bool):
    """Test a bundle on IRIX.

    Deploys the bundle to IRIX, runs auto-tests (--version) for every
    binary and YAML-defined smoke_tests, then reports pass/fail.

    \b
    Examples:
        mogrix test ~/mogrix_outputs/bundles/mogrix-fun-1b-irix-bundle
        mogrix test ~/mogrix_outputs/bundles/mogrix-essentials-1b-irix-bundle --keep
    """
    from mogrix.test import (
        TestDiscovery,
        ScriptGenerator,
        IRIXTestRunner,
        TestReport,
        print_report,
    )

    bundle_dir = Path(bundle_path)
    if not bundle_dir.is_dir():
        if bundle_dir.name.endswith(".tar.gz"):
            console.print(
                "[red]Please provide the bundle directory, not the tarball.[/red]"
            )
            raise SystemExit(1)
        console.print(f"[red]Not a directory: {bundle_dir}[/red]")
        raise SystemExit(1)

    remote_dir = "/tmp/mogrix-test"
    remote_bundle = f"{remote_dir}/{bundle_dir.name}"

    # 1. Check connectivity
    runner = IRIXTestRunner(host=host, user=user)
    console.print("[dim]Checking IRIX connectivity...[/dim]")
    ok, msg = runner.check_connectivity()
    if not ok:
        report = TestReport(
            bundle_name=bundle_dir.name,
            irix_reachable=False,
            error_message=msg,
        )
        print_report(report, console)
        raise SystemExit(2)
    console.print(f"[dim]IRIX: connected to {host}[/dim]")

    # 2. Discover tests
    discovery = TestDiscovery(RULES_DIR)
    tests = discovery.discover(bundle_dir)
    console.print(f"[dim]Discovered {len(tests)} tests for {bundle_dir.name}[/dim]")
    if not tests:
        console.print("[yellow]No testable wrappers found in bundle.[/yellow]")
        raise SystemExit(0)

    # 3. Deploy bundle
    console.print(f"[dim]Deploying bundle to {host}:{remote_bundle}...[/dim]")
    ok, msg = runner.deploy_bundle(bundle_dir, remote_dir)
    if not ok:
        console.print(f"[red]Deploy failed: {msg}[/red]")
        raise SystemExit(1)

    # 4. Generate and run test script
    generator = ScriptGenerator()
    script = generator.generate(tests)

    console.print(f"[dim]Running {len(tests)} tests on IRIX...[/dim]")
    rc, stdout, stderr = runner.run_script(script, remote_bundle)

    # 5. Cleanup (unless --keep)
    if not keep:
        runner.cleanup(remote_bundle)

    # 6. Parse results and report
    results = runner.parse_results(stdout)
    report = TestReport(bundle_name=bundle_dir.name, results=results)

    if not results and stdout:
        # Script ran but no results parsed — show raw output
        console.print("[yellow]No test results parsed. Raw output:[/yellow]")
        console.print(stdout[:2000])
        if stderr:
            console.print(f"[dim]stderr: {stderr[:500]}[/dim]")
        raise SystemExit(1)

    print_report(report, console)
    raise SystemExit(1 if report.failed > 0 else 0)


# Bootstrap manifest: minimum packages for tdnf + rpm on IRIX.
# Format: "glob-pattern" matched against RPM filenames in the RPMS dir.
_BOOTSTRAP_MANIFEST = [
    # Base libraries (no deps on other mogrix packages)
    "zlib-ng-compat-[0-9]*",
    "bzip2-libs-[0-9]*",
    "xz-libs-[0-9]*",
    "popt-[0-9]*",
    "openssl-libs-[0-9]*",
    "lua-libs-[0-9]*",
    "file-libs-[0-9]*",
    "sqlite-libs-[0-9]*",
    # Mid-level (depend on base)
    "libxml2-[0-9]*",
    "libcurl-[0-9]*",
    # RPM and package management
    "rpm-libs-[0-9]*",
    "rpm-[0-9]*",
    "libsolv-[0-9]*",
    "tdnf-cli-libs-[0-9]*",
    "tdnf-[0-9]*",
    # Release/config packages
    "sgugrse-release-common-[0-9]*",
    "sgugrse-release-[0-9]*",
    # Dev symlinks needed at runtime (libsolvext links libz.so not libz.so.1)
    "zlib-ng-compat-devel-[0-9]*",
]


@main.command("create-bootstrap")
@click.option(
    "--rpms-dir",
    type=click.Path(exists=True),
    default=None,
    help="Directory containing built RPMs (default: ~/mogrix_outputs/RPMS/)",
)
@click.option(
    "--output",
    "-o",
    type=click.Path(),
    default=None,
    help="Output .run file path (default: ~/mogrix_outputs/mogrix-bootstrap.TIMESTAMP.run)",
)
def create_bootstrap(
    rpms_dir: str | None,
    output: str | None,
):
    """Create a self-extracting bootstrap installer for IRIX.

    Packages the minimum RPMs needed for tdnf (the package manager) plus
    sgugshell/sgug-exec and repo config into a self-extracting .run file.

    \b
    On IRIX, run as root:  sh mogrix-bootstrap.run
    This creates /usr/sgug with tdnf + rpm + dependencies. Then:
      /usr/sgug/bin/sgugshell
      rpm --initdb
      tdnf makecache
      tdnf install <package>

    \b
    The installer refuses to run if /usr/sgug already exists.
    """
    import fnmatch
    import shutil
    import subprocess
    import tempfile
    from datetime import datetime

    rpms_path = Path(rpms_dir) if rpms_dir else MOGRIX_OUTPUTS / "RPMS"
    if not rpms_path.is_dir():
        console.print(f"[red]RPMs directory not found: {rpms_path}[/red]")
        raise SystemExit(1)

    timestamp = datetime.now().strftime("%m%d%y%H%M")
    default_output = MOGRIX_OUTPUTS / f"mogrix-bootstrap.{timestamp}.run"
    out_path = Path(output) if output else default_output

    # Resolve manifest patterns to actual RPM files
    console.print("[bold]Resolving bootstrap manifest...[/bold]")
    all_rpms = sorted(rpms_path.glob("*.rpm"))
    found_rpms = []
    missing = []

    for pattern in _BOOTSTRAP_MANIFEST:
        # Match pattern against RPM filenames (strip .mips.rpm suffix for matching)
        matches = [
            r for r in all_rpms
            if fnmatch.fnmatch(r.name.replace(".mips.rpm", ""), pattern)
        ]
        if matches:
            # Take the first match (sorted, so deterministic)
            found_rpms.append(matches[0])
            console.print(f"  [green]OK[/green] {matches[0].name}")
        else:
            missing.append(pattern)
            console.print(f"  [red]MISSING[/red] {pattern}")

    if missing:
        console.print(f"\n[red]{len(missing)} required package(s) missing![/red]")
        console.print("[dim]Build them with: mogrix batch-build --from-list ...[/dim]")
        raise SystemExit(1)

    console.print(f"\n[bold]{len(found_rpms)} packages in bootstrap manifest[/bold]")

    # Locate extra tools
    project_dir = Path(__file__).parent.parent
    extras = {
        "tools/sgugshell": "usr/sgug/bin/sgugshell",
        "tools/sgug-exec": "usr/sgug/bin/sgug-exec",
        "configs/tdnf/mogrix.repo": "usr/sgug/etc/yum.repos.d/mogrix.repo",
        "configs/tdnf/tdnf.conf": "usr/sgug/etc/tdnf/tdnf.conf",
    }
    for src_rel in extras:
        src = project_dir / src_rel
        if not src.exists():
            console.print(f"[red]Missing: {src}[/red]")
            raise SystemExit(1)

    with tempfile.TemporaryDirectory(prefix="mogrix-bootstrap-") as tmpdir:
        tree = Path(tmpdir) / "tree"
        tree.mkdir()

        # Extract bootstrap RPMs into the tree
        console.print("[bold]Extracting RPMs...[/bold]")
        for rpm in found_rpms:
            console.print(f"  {rpm.name}")
            subprocess.run(
                f"cd {tree} && rpm2cpio {rpm.absolute()} | cpio -idm 2>/dev/null",
                shell=True,
                capture_output=True,
            )

        sgug_dir = tree / "usr" / "sgug"
        if not sgug_dir.is_dir():
            console.print("[red]Error: no usr/sgug/ found after extraction[/red]")
            raise SystemExit(1)

        # Install extras (sgugshell, sgug-exec, repo config, tdnf.conf)
        console.print("[bold]Installing extras...[/bold]")
        for src_rel, dest_rel in extras.items():
            src = project_dir / src_rel
            dest = tree / dest_rel
            dest.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(src, dest)
            dest.chmod(0o755 if dest_rel.endswith(("sgugshell", "sgug-exec")) else 0o644)
            console.print(f"  /{dest_rel}")

        # Ensure required directories exist
        (tree / "usr/sgug/var/cache/tdnf").mkdir(parents=True, exist_ok=True)

        # Include the RPM files themselves for rpm --initdb registration
        bootstrap_rpms_dir = tree / "tmp" / "bootstrap-rpms"
        bootstrap_rpms_dir.mkdir(parents=True, exist_ok=True)
        for rpm in found_rpms:
            shutil.copy2(rpm, bootstrap_rpms_dir / rpm.name)
        console.print(f"  /tmp/bootstrap-rpms/ ({len(found_rpms)} RPMs)")

        # Verify critical files
        console.print("[bold]Verifying...[/bold]")
        critical = [
            "usr/sgug/bin/rpm", "usr/sgug/bin/tdnf",
            "usr/sgug/bin/sgugshell", "usr/sgug/bin/sgug-exec",
            "usr/sgug/lib32/librpm.so", "usr/sgug/lib32/libtdnf.so",
            "usr/sgug/lib32/libsolv.so",
            "usr/sgug/etc/tdnf/tdnf.conf",
            "usr/sgug/etc/yum.repos.d/mogrix.repo",
        ]
        errors = 0
        for f in critical:
            p = tree / f
            if p.exists() or p.is_symlink():
                console.print(f"  [green]OK[/green] /{f}")
            else:
                console.print(f"  [red]MISSING[/red] /{f}")
                errors += 1
        if errors:
            console.print(f"\n[red]{errors} critical file(s) missing![/red]")
            raise SystemExit(1)

        # Create tarball from tree root (preserves usr/sgug/..., tmp/..., var/...)
        console.print("[bold]Creating tarball...[/bold]")
        tarball = Path(tmpdir) / "bootstrap.tar.gz"
        subprocess.run(
            ["tar", "-czf", str(tarball), "-C", str(tree), "."],
            capture_output=True,
            check=True,
        )
        console.print(f"  {tarball.stat().st_size / 1024 / 1024:.1f} MB")

        # Create self-extracting .run
        console.print("[bold]Creating installer...[/bold]")
        _create_bootstrap_run(out_path, tarball, len(found_rpms))

    run_size = out_path.stat().st_size
    console.print(f"\n[bold green]Bootstrap installer created![/bold green]")
    console.print(f"[bold]Output:[/bold] {out_path}")
    console.print(f"[bold]Size:[/bold] {run_size / 1024 / 1024:.1f} MB")
    console.print(f"\n[dim]Deploy to IRIX and run as root:  sh {out_path.name}[/dim]")
    console.print("[dim]Then:  /usr/sgug/bin/sgugshell[/dim]")
    console.print("[dim]       rpm --initdb[/dim]")
    console.print("[dim]       rpm -Uvh --nodeps /tmp/bootstrap-rpms/*.rpm[/dim]")
    console.print("[dim]       tdnf makecache && tdnf list[/dim]")


_BOOTSTRAP_TEMPLATE = """\
#!/bin/sh
# Mogrix bootstrap installer - minimum tdnf + rpm environment
# Usage: sh {filename}
SKIP={payload_line}
self="$0"
case "$self" in /*) ;; *) self="`/bin/pwd`/$self" ;; esac
if [ -d /usr/sgug ]; then
  echo "Error: /usr/sgug already exists." >&2
  echo "Remove it first if you want to reinstall." >&2
  exit 1
fi
/sbin/mkdir -p /usr/sgug 2>/dev/null
if [ ! -d /usr/sgug ]; then
  echo "Error: cannot create /usr/sgug (are you root?)" >&2
  exit 1
fi
echo "Installing mogrix bootstrap ({rpm_count} packages) ..."
cd / && /bin/tail +$SKIP "$self" | /usr/sbin/gzcat | /sbin/tar xf -
status=$?
if [ $status -ne 0 ]; then
  echo "Error: extraction failed" >&2
  exit 1
fi
echo ""
echo "Done. Bootstrap installed to /usr/sgug"
echo ""
echo "Next steps:"
echo "  /usr/sgug/bin/sgugshell"
echo "  rpm --initdb"
echo "  rpm -Uvh --nodeps /tmp/bootstrap-rpms/*.rpm"
echo "  tdnf makecache"
echo "  tdnf list"
exit 0
"""


def _create_bootstrap_run(
    run_path: Path, tarball_path: Path, rpm_count: int
) -> None:
    """Create a self-extracting bootstrap .run file."""
    script = _BOOTSTRAP_TEMPLATE.format(
        filename=run_path.name,
        payload_line="__PLACEHOLDER__",
        rpm_count=rpm_count,
    )
    line_count = script.count("\n")
    script = script.replace("__PLACEHOLDER__", str(line_count + 1))

    with open(run_path, "wb") as f:
        f.write(script.encode("ascii"))
        with open(tarball_path, "rb") as payload:
            while True:
                chunk = payload.read(65536)
                if not chunk:
                    break
                f.write(chunk)

    run_path.chmod(0o755)


@main.command("patch-crates")
@click.option(
    "--registry-path",
    type=click.Path(exists=True),
    default=None,
    help="Cargo registry source directory (default: auto-detect)",
)
@click.option(
    "--rules-dir",
    type=click.Path(exists=True),
    default=None,
    help="Rules directory (default: rules/)",
)
@click.option(
    "--project-dir",
    type=click.Path(exists=True),
    default=None,
    help="Rust IRIX project directory (default: ../rust-irix)",
)
def patch_crates(
    registry_path: str | None,
    rules_dir: str | None,
    project_dir: str | None,
):
    """Patch Rust crates in cargo registry for IRIX cross-compilation.

    Applies declarative transformation rules from rules/crates/ to all
    crates in the cargo registry. Run after 'cargo fetch' and before
    'cargo build'.

    Rules follow the same pattern as mogrix RPM rules:
    - Generic rules (rules/crates/generic_rust.yaml) applied to all crates
    - Crate-specific rules (rules/crates/<name>.yaml) for overrides

    Idempotent — safe to re-run after any cargo operation.
    """
    from mogrix.crate_patcher import patch_all_crates

    if rules_dir is None:
        rules_dir = str(
            Path(__file__).parent.parent / "rules"
        )

    click.echo("=== mogrix patch-crates ===")
    stats = patch_all_crates(
        registry_dir=registry_path,
        rules_dir=rules_dir,
        project_dir=project_dir,
    )

    if "error" in stats:
        click.echo(f"ERROR: {stats['error']}")
        raise SystemExit(1)

    click.echo(f"\nCrates processed: {stats['crates_processed']}")
    click.echo(f"Crates patched:   {stats['crates_patched']}")
    click.echo(f"Text replacements: {stats['replacements']}")
    click.echo(f"Lines removed:     {stats['lines_removed']}")
    click.echo(f"Sources added:     {stats['sources_added']}")
    click.echo(f"Files created:     {stats['files_created']}")
    click.echo(f"Bare lines fixed:  {stats['bare_lines_fixed']}")
    click.echo(f"Malformed not() fixed: {stats['malformed_not_fixed']}")

    errors = stats.get("errors", [])
    if errors:
        click.echo(f"\n{click.style(f'VALIDATION ERRORS: {len(errors)}', fg='red', bold=True)}")
        click.echo("The following crate-specific patterns did not match.")
        click.echo("This usually means a crate version changed. Update the rule.\n")
        for err in errors:
            click.echo(click.style(str(err), fg='red'))
        click.echo(
            f"\nTo suppress a specific check, add 'expected_count: 0' to the "
            f"replacement entry in the rule YAML."
        )
        raise SystemExit(2)


@main.command()
@click.argument("rules")
@click.option(
    "--source",
    type=click.Path(exists=True),
    default=None,
    help="Override the source directory (default: from rule file).",
)
@click.option(
    "--dry-run",
    is_flag=True,
    help="Show what would change without writing.",
)
@click.option(
    "--check-only",
    is_flag=True,
    help="Run postconditions only (verify a previous transform).",
)
@click.option(
    "--debug-queries",
    is_flag=True,
    help="Print tree-sitter AST structure and query matches for debugging.",
)
def transform(
    rules: str,
    source: str | None,
    dry_run: bool,
    check_only: bool,
    debug_queries: bool,
):
    """Apply declarative source transforms from a rule file.

    RULES is a path to a YAML rule file, or a name that resolves to
    rules/transforms/<name>.yaml.

    \b
    Examples:
      mogrix transform opencode-strip --dry-run
      mogrix transform rules/transforms/opencode-strip.yaml
      mogrix transform opencode-strip --check-only
    """
    import logging

    from mogrix.source_transform import transform_project

    logging.basicConfig(level=logging.INFO, format="%(message)s")

    action = "check" if check_only else ("dry-run" if dry_run else "transform")
    click.echo(f"=== mogrix transform ({action}) ===")

    try:
        stats, postconditions_ok = transform_project(
            rules_arg=rules,
            source_override=source,
            dry_run=dry_run,
            check_only=check_only,
            debug_queries=debug_queries,
        )
    except FileNotFoundError as e:
        click.echo(click.style(str(e), fg="red"))
        raise SystemExit(1)

    if not check_only:
        click.echo(f"\nFiles modified:     {len(stats.files_modified)}")
        click.echo(f"AST transforms:     {stats.ast_applied}")
        click.echo(f"Text replacements:  {stats.replacements}")
        click.echo(f"Lines removed:      {stats.lines_removed}")
        click.echo(f"Files deleted:      {stats.files_deleted}")
        click.echo(f"Blocks removed:     {stats.blocks_removed}")

        if stats.errors:
            click.echo(
                click.style(
                    f"\nVALIDATION ERRORS: {len(stats.errors)}",
                    fg="red",
                    bold=True,
                )
            )
            for err in stats.errors:
                click.echo(click.style(str(err), fg="red"))
            raise SystemExit(2)

    if postconditions_ok:
        click.echo(click.style("\nPostconditions: PASS", fg="green"))
    else:
        click.echo(click.style("\nPostconditions: FAIL", fg="red"))
        raise SystemExit(3)


if __name__ == "__main__":
    main()
