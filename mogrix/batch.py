"""Batch conversion for mogrix."""

from pathlib import Path

from mogrix.compat.injector import CompatInjector
from mogrix.emitter.spec import SpecWriter
from mogrix.headers.overlay import HeaderOverlayManager
from mogrix.parser.spec import SpecParser
from mogrix.rules.engine import RuleEngine
from mogrix.rules.loader import RuleLoader

# Default directories (relative to package)
RULES_DIR = Path(__file__).parent.parent / "rules"
HEADERS_DIR = Path(__file__).parent.parent / "headers"
COMPAT_DIR = Path(__file__).parent.parent / "compat"


class BatchConverter:
    """Converts multiple spec files in batch."""

    def __init__(
        self,
        specs_dir: Path,
        rules_dir: Path | None = None,
        headers_dir: Path | None = None,
        compat_dir: Path | None = None,
    ):
        """Initialize batch converter.

        Args:
            specs_dir: Directory containing spec files
            rules_dir: Path to rules directory
            headers_dir: Path to headers directory
            compat_dir: Path to compat sources directory
        """
        self.specs_dir = specs_dir
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

    def discover_specs(self) -> list[Path]:
        """Find all spec files in the specs directory.

        Returns:
            List of paths to spec files
        """
        return sorted(self.specs_dir.glob("*.spec"))

    def convert_one(self, spec_path: Path, output_dir: Path) -> dict:
        """Convert a single spec file.

        Args:
            spec_path: Path to input spec file
            output_dir: Directory for output

        Returns:
            Result dict with package name, status, and details
        """
        result = {
            "package": spec_path.stem,
            "input": str(spec_path),
            "status": "success",
            "applied_rules": [],
            "error": None,
        }

        try:
            # Parse spec
            spec = self.parser.parse(spec_path)

            # Apply rules
            transform = self.engine.apply(spec)
            result["applied_rules"] = transform.applied_rules

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

            # Write converted spec
            content = self.writer.write(
                transform,
                drops=drops,
                adds=adds,
                cppflags=cppflags,
                compat_sources=compat_sources,
                compat_prep=compat_prep,
                compat_build=compat_build,
                ac_cv_overrides=transform.ac_cv_overrides or None,
                drop_requires=transform.drop_requires or None,
                remove_lines=transform.remove_lines or None,
            )

            # Save output
            output_path = output_dir / spec_path.name
            output_path.write_text(content)
            result["output"] = str(output_path)

        except Exception as e:
            result["status"] = "error"
            result["error"] = str(e)

        return result

    def convert_all(
        self, output_dir: Path, specs: list[Path] | None = None
    ) -> list[dict]:
        """Convert all spec files.

        Args:
            output_dir: Directory for output files
            specs: Optional list of specific specs to convert

        Returns:
            List of result dicts for each conversion
        """
        output_dir.mkdir(parents=True, exist_ok=True)

        if specs is None:
            specs = self.discover_specs()

        results = []
        for spec_path in specs:
            result = self.convert_one(spec_path, output_dir)
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
