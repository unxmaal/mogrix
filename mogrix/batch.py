"""Batch conversion for mogrix."""

import shutil
from pathlib import Path

from mogrix.compat.injector import CompatInjector
from mogrix.emitter.spec import SpecWriter
from mogrix.emitter.srpm import SRPMEmitter
from mogrix.headers.overlay import HeaderOverlayManager
from mogrix.parser.spec import SpecParser
from mogrix.parser.srpm import SRPMExtractor
from mogrix.rules.engine import RuleEngine
from mogrix.rules.loader import RuleLoader

# Default directories (relative to package)
RULES_DIR = Path(__file__).parent.parent / "rules"
HEADERS_DIR = Path(__file__).parent.parent / "headers"
COMPAT_DIR = Path(__file__).parent.parent / "compat"
PATCHES_DIR = Path(__file__).parent.parent / "patches"


class BatchConverter:
    """Converts multiple SRPMs in batch."""

    def __init__(
        self,
        srpms_dir: Path,
        rules_dir: Path | None = None,
        headers_dir: Path | None = None,
        compat_dir: Path | None = None,
    ):
        """Initialize batch converter.

        Args:
            srpms_dir: Directory containing SRPM files
            rules_dir: Path to rules directory
            headers_dir: Path to headers directory
            compat_dir: Path to compat sources directory
        """
        self.srpms_dir = srpms_dir
        self.rules_dir = rules_dir or RULES_DIR
        self.headers_dir = headers_dir or HEADERS_DIR
        self.compat_dir = compat_dir or COMPAT_DIR

        # Initialize components
        self.parser = SpecParser()
        self.loader = RuleLoader(self.rules_dir)
        self.engine = RuleEngine(self.loader)
        self.writer = SpecWriter()
        self.overlay_mgr = HeaderOverlayManager(self.headers_dir)
        self.injector = CompatInjector(self.compat_dir)

    def discover_srpms(self) -> list[Path]:
        """Find all SRPM files in the directory.

        Returns:
            List of paths to SRPM files
        """
        return sorted(self.srpms_dir.glob("*.src.rpm"))

    def convert_one(self, srpm_path: Path, output_dir: Path) -> dict:
        """Convert a single SRPM.

        Args:
            srpm_path: Path to input SRPM file
            output_dir: Directory for output

        Returns:
            Result dict with package name, status, and details
        """
        result = {
            "package": srpm_path.name.rsplit("-", 2)[0],  # Extract package name from SRPM filename
            "input": str(srpm_path),
            "status": "success",
            "applied_rules": [],
            "compat_functions": [],
            "output_srpm": None,
            "error": None,
        }

        # Create package-specific output directory
        pkg_output_dir = output_dir / result["package"]
        pkg_output_dir.mkdir(parents=True, exist_ok=True)

        extracted_dir = None
        try:
            # Extract SRPM
            extractor = SRPMExtractor(srpm_path)
            extracted_dir, spec_path = extractor.extract_spec()

            # Parse spec
            spec = self.parser.parse(spec_path)

            # Apply rules
            transform = self.engine.apply(spec)
            result["applied_rules"] = transform.applied_rules
            result["compat_functions"] = transform.compat_functions

            # Calculate drops/adds
            original_br = set(spec.buildrequires)
            final_br = set(transform.spec.buildrequires)
            drops = list(original_br - final_br)
            adds = list(final_br - original_br)

            # Generate CPPFLAGS for header overlays
            cppflags = None
            if transform.header_overlays:
                cppflags = self.overlay_mgr.get_cppflags(transform.header_overlays)

            # Generate compat source injection
            compat_sources = None
            compat_prep = None
            compat_build = None
            if transform.compat_functions:
                compat_sources = self.injector.get_source_entries(
                    transform.compat_functions
                )
                compat_prep = self.injector.get_prep_commands(
                    transform.compat_functions
                )
                compat_build = self.injector.get_build_commands(
                    transform.compat_functions
                )

            # Copy all files from extracted SRPM to output directory
            for src_file in extracted_dir.iterdir():
                if src_file.is_file():
                    dest_file = pkg_output_dir / src_file.name
                    shutil.copy2(src_file, dest_file)

            # Copy compat source files if needed
            if transform.compat_functions:
                compat_files = self.injector.resolve_functions(transform.compat_functions)
                extra_files = self.injector.get_extra_files(transform.compat_functions)
                for compat_file in compat_files + extra_files:
                    dest_file = pkg_output_dir / compat_file.name
                    shutil.copy2(compat_file, dest_file)

            # Copy patch files from mogrix patches directory
            patch_files = []
            if transform.add_patches:
                patches_pkg_dir = PATCHES_DIR / "packages" / spec.name
                for patch_name in transform.add_patches:
                    patch_path = patches_pkg_dir / patch_name
                    if patch_path.exists():
                        dest_file = pkg_output_dir / patch_name
                        shutil.copy2(patch_path, dest_file)
                        patch_files.append(patch_name)

            # Copy extra source files
            source_files = []
            if transform.add_sources:
                patches_pkg_dir = PATCHES_DIR / "packages" / spec.name
                for source_name in transform.add_sources:
                    source_path = patches_pkg_dir / source_name
                    if source_path.exists():
                        dest_file = pkg_output_dir / source_name
                        shutil.copy2(source_path, dest_file)
                        source_files.append(source_name)

            # Generate patch/source entries for spec
            patch_sources = None
            patch_prep = None
            if patch_files:
                patch_entries = []
                patch_cmds = []
                for i, patch_name in enumerate(patch_files):
                    patch_num = 500 + i
                    patch_entries.append(f"Patch{patch_num}: {patch_name}")
                    patch_cmds.append(f"%patch -P{patch_num} -p1")
                patch_sources = "\n".join(patch_entries)
                patch_prep = "\n".join(patch_cmds)

            extra_sources = None
            if source_files:
                source_entries = []
                for i, source_name in enumerate(source_files):
                    source_num = 200 + i
                    source_entries.append(f"Source{source_num}: {source_name}")
                extra_sources = "\n".join(source_entries)

            # Generate converted spec content
            content = self.writer.write(
                transform,
                drops=drops,
                adds=adds,
                cppflags=cppflags,
                compat_sources=compat_sources,
                compat_prep=compat_prep,
                compat_build=compat_build,
                patch_sources=patch_sources,
                patch_prep=patch_prep,
                extra_sources=extra_sources,
                ac_cv_overrides=transform.ac_cv_overrides or None,
                drop_requires=transform.drop_requires or None,
                add_requires=transform.add_requires or None,
                remove_lines=transform.remove_lines or None,
                rpm_macros=transform.rpm_macros or None,
                export_vars=transform.export_vars or None,
                skip_find_lang=transform.skip_find_lang,
                skip_check=transform.skip_check,
                install_cleanup=transform.install_cleanup or None,
                spec_replacements=transform.spec_replacements or None,
            )

            # Write converted spec (overwriting the original)
            converted_spec_path = pkg_output_dir / spec_path.name
            converted_spec_path.write_text(content)

            # Build new SRPM
            sources = [
                f for f in pkg_output_dir.iterdir()
                if f.is_file() and not f.name.endswith(".spec")
            ]

            emitter = SRPMEmitter()
            new_srpm = emitter.emit_srpm(
                spec_content=content,
                spec_name=spec_path.name,
                sources=sources,
                output_dir=pkg_output_dir,
            )

            result["output_srpm"] = str(new_srpm)
            result["output_dir"] = str(pkg_output_dir)

        except Exception as e:
            result["status"] = "error"
            result["error"] = str(e)

        finally:
            # Clean up extracted temp directory
            if extracted_dir and extracted_dir.exists():
                shutil.rmtree(extracted_dir)

        return result

    def convert_all(
        self, output_dir: Path, srpms: list[Path] | None = None
    ) -> list[dict]:
        """Convert all SRPM files.

        Args:
            output_dir: Directory for output files
            srpms: Optional list of specific SRPMs to convert

        Returns:
            List of result dicts for each conversion
        """
        output_dir.mkdir(parents=True, exist_ok=True)

        if srpms is None:
            srpms = self.discover_srpms()

        results = []
        for srpm_path in srpms:
            result = self.convert_one(srpm_path, output_dir)
            results.append(result)

        return results

    def get_summary(self, results: list[dict]) -> dict:
        """Generate summary of batch conversion.

        Args:
            results: List of conversion results

        Returns:
            Summary dict with counts and lists
        """
        success = [r for r in results if r["status"] == "success"]
        errors = [r for r in results if r["status"] == "error"]

        return {
            "total": len(results),
            "success": len(success),
            "errors": len(errors),
            "success_packages": [r["package"] for r in success],
            "error_packages": [
                {"package": r["package"], "error": r["error"]} for r in errors
            ],
        }
