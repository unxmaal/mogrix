"""Rule application engine for mogrix."""

from dataclasses import dataclass, field
from mogrix.parser.spec import SpecFile
from mogrix.rules.loader import RuleLoader


@dataclass
class TransformResult:
    """Result of applying rules to a spec file."""

    spec: SpecFile
    applied_rules: list[str] = field(default_factory=list)
    configure_disable: list[str] = field(default_factory=list)
    configure_flags_add: list[str] = field(default_factory=list)
    configure_flags_remove: list[str] = field(default_factory=list)
    header_overlays: list[str] = field(default_factory=list)
    path_rewrites: dict[str, str] = field(default_factory=dict)
    warnings: list[str] = field(default_factory=list)


class RuleEngine:
    """Applies transformation rules to spec files."""

    def __init__(self, loader: RuleLoader):
        """Initialize with a rule loader."""
        self.loader = loader

    def apply(self, spec: SpecFile) -> TransformResult:
        """Apply all applicable rules to a spec file."""
        # Create a copy of the spec to modify
        result = TransformResult(
            spec=SpecFile(
                name=spec.name,
                version=spec.version,
                release=spec.release,
                summary=spec.summary,
                license=spec.license,
                url=spec.url,
                buildrequires=list(spec.buildrequires),
                requires=list(spec.requires),
                sources=dict(spec.sources),
                patches=dict(spec.patches),
                raw_content=spec.raw_content,
            )
        )

        # Load and apply generic rules
        generic = self.loader.load_generic()
        if generic and "generic" in generic:
            self._apply_generic_rules(result, generic["generic"])

        # TODO: Load and apply class rules
        # TODO: Load and apply package-specific rules

        return result

    def _apply_generic_rules(
        self, result: TransformResult, rules: dict
    ) -> None:
        """Apply generic rules to the result."""
        # Drop BuildRequires
        if "drop_buildrequires" in rules:
            drops = set(rules["drop_buildrequires"])
            original = result.spec.buildrequires[:]
            result.spec.buildrequires = [
                br for br in result.spec.buildrequires if br not in drops
            ]
            for br in original:
                if br in drops:
                    result.applied_rules.append(
                        f"drop_buildrequires: removed {br}"
                    )

        # Add BuildRequires
        if "add_buildrequires" in rules:
            for br in rules["add_buildrequires"]:
                if br not in result.spec.buildrequires:
                    result.spec.buildrequires.append(br)
                    result.applied_rules.append(
                        f"add_buildrequires: added {br}"
                    )

        # Collect configure --disable flags
        if "configure_disable" in rules:
            result.configure_disable.extend(rules["configure_disable"])
            result.applied_rules.append(
                f"configure_disable: {rules['configure_disable']}"
            )

        # Collect path rewrites
        if "rewrite_paths" in rules:
            result.path_rewrites.update(rules["rewrite_paths"])
            result.applied_rules.append("rewrite_paths: collected")

        # Collect header overlays
        if "header_overlays" in rules:
            result.header_overlays.extend(rules["header_overlays"])
            result.applied_rules.append(
                f"header_overlays: {rules['header_overlays']}"
            )
