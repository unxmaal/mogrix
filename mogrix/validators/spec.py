"""Spec file validation for mogrix.

Uses the packit/specfile library to parse converted specs and catch
structural issues before rpmbuild encounters them.
"""

import re
import tempfile
from dataclasses import dataclass, field
from pathlib import Path


@dataclass
class SpecValidationIssue:
    """A validation issue found in a spec file."""

    severity: str  # "error", "warning"
    message: str
    line: int | None = None


@dataclass
class SpecValidationResult:
    """Result of validating a spec file."""

    issues: list[SpecValidationIssue] = field(default_factory=list)

    @property
    def errors(self) -> list[SpecValidationIssue]:
        return [i for i in self.issues if i.severity == "error"]

    @property
    def warnings(self) -> list[SpecValidationIssue]:
        return [i for i in self.issues if i.severity == "warning"]

    @property
    def is_valid(self) -> bool:
        return len(self.errors) == 0


# Required preamble tags for a valid spec
REQUIRED_TAGS = {"Name", "Version", "Release", "Summary", "License"}

# Sections that should exist in a buildable spec
REQUIRED_SECTIONS = {"prep", "build", "install", "files"}

# RPM section markers (these start new sections, not commands)
RPM_SECTIONS = {
    "prep", "build", "install", "check", "clean",
    "files", "changelog", "package", "description",
    "pre", "post", "preun", "postun",
    "pretrans", "posttrans", "triggerprein",
    "triggerin", "triggerun", "triggerpostun",
    "verifyscript",
}


class SpecValidator:
    """Validates converted spec file content.

    Uses the specfile library (packit) for parsing and adds
    mogrix-specific structural checks.
    """

    def validate(self, content: str, filename: str = "spec") -> SpecValidationResult:
        """Validate spec file content.

        Args:
            content: The spec file content as a string.
            filename: Name to use in error messages.

        Returns:
            SpecValidationResult with any issues found.
        """
        result = SpecValidationResult()

        self._check_specfile_parse(content, filename, result)
        self._check_required_tags(content, result)
        self._check_required_sections(content, result)
        self._check_balanced_conditionals(content, result)
        self._check_empty_sections(content, result)

        return result

    def _check_specfile_parse(
        self, content: str, filename: str, result: SpecValidationResult
    ) -> None:
        """Check that the specfile library can parse the content."""
        try:
            from specfile import Specfile

            with tempfile.NamedTemporaryFile(
                mode="w", suffix=".spec", delete=False
            ) as f:
                f.write(content)
                f.flush()
                tmp_path = f.name

            try:
                spec = Specfile(tmp_path)
                # Check that we can read basic tags
                with spec.tags() as tags:
                    tag_names = {t.name for t in tags}

                with spec.sources() as sources:
                    _ = list(sources)

                with spec.patches() as patches:
                    _ = list(patches)

            except Exception as e:
                result.issues.append(
                    SpecValidationIssue(
                        severity="error",
                        message=f"specfile parse error: {e}",
                    )
                )
            finally:
                Path(tmp_path).unlink(missing_ok=True)

        except ImportError:
            result.issues.append(
                SpecValidationIssue(
                    severity="warning",
                    message="specfile library not installed, skipping parse check",
                )
            )

    def _check_required_tags(
        self, content: str, result: SpecValidationResult
    ) -> None:
        """Check that required preamble tags are present."""
        # Match tags at the start of a line (ignoring commented-out lines)
        found_tags = set()
        for line in content.splitlines():
            stripped = line.strip()
            if stripped.startswith("#"):
                continue
            match = re.match(r"^(\w+)\s*:", stripped)
            if match:
                found_tags.add(match.group(1))

        for tag in REQUIRED_TAGS:
            if tag not in found_tags:
                result.issues.append(
                    SpecValidationIssue(
                        severity="error",
                        message=f"missing required tag: {tag}",
                    )
                )

    def _check_required_sections(
        self, content: str, result: SpecValidationResult
    ) -> None:
        """Check that required sections exist."""
        found_sections = set()
        for line in content.splitlines():
            stripped = line.strip()
            if stripped.startswith("#"):
                continue
            # Match section markers like %prep, %build, %install, %files
            match = re.match(r"^%(\w+)", stripped)
            if match:
                keyword = match.group(1).lower()
                if keyword in RPM_SECTIONS:
                    found_sections.add(keyword)

        for section in REQUIRED_SECTIONS:
            if section not in found_sections:
                result.issues.append(
                    SpecValidationIssue(
                        severity="warning",
                        message=f"missing section: %{section}",
                    )
                )

    def _check_balanced_conditionals(
        self, content: str, result: SpecValidationResult
    ) -> None:
        """Check that %if/%endif blocks are balanced."""
        # %if, %ifarch, %ifnarch, %ifos, %ifnos all open conditional blocks
        if_pattern = re.compile(r"^%(if|ifarch|ifnarch|ifos|ifnos)\b")
        # %endif may be followed by whitespace, comments, or line continuations
        endif_pattern = re.compile(r"^%endif(\s|$|#|\\)")
        depth = 0
        for i, line in enumerate(content.splitlines(), 1):
            stripped = line.strip()
            if stripped.startswith("#"):
                continue
            if if_pattern.match(stripped):
                depth += 1
            elif stripped == "%endif" or endif_pattern.match(stripped):
                depth -= 1
                if depth < 0:
                    result.issues.append(
                        SpecValidationIssue(
                            severity="error",
                            message=f"unmatched %endif at line {i}",
                            line=i,
                        )
                    )
                    depth = 0

        if depth > 0:
            result.issues.append(
                SpecValidationIssue(
                    severity="error",
                    message=f"%if without %endif ({depth} unclosed)",
                )
            )

    def _check_empty_sections(
        self, content: str, result: SpecValidationResult
    ) -> None:
        """Warn about empty %prep, %build, or %install sections."""
        lines = content.splitlines()
        section_pattern = re.compile(r"^%(\w+)")
        check_sections = {"prep", "build", "install"}

        current_section = None
        section_start = None
        section_has_content = False

        for i, line in enumerate(lines):
            stripped = line.strip()
            match = section_pattern.match(stripped)

            if match:
                keyword = match.group(1).lower()
                # Only treat actual RPM section markers as section boundaries
                if keyword in RPM_SECTIONS:
                    # Check if previous section was empty
                    if (
                        current_section in check_sections
                        and not section_has_content
                    ):
                        result.issues.append(
                            SpecValidationIssue(
                                severity="warning",
                                message=f"empty section: %{current_section} (line {section_start})",
                                line=section_start,
                            )
                        )

                    current_section = keyword
                    section_start = i + 1
                    section_has_content = False
                    continue

            if current_section and stripped and not stripped.startswith("#"):
                section_has_content = True
