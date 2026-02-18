"""ELF binary checker for IRIX-specific relocation issues.

Detects R_MIPS_REL32 relocations in .data sections that reference UNDEF
symbols (from shared libraries). IRIX rld silently drops some of these
in large executables, leaving function pointers NULL in widget ClassRec
structures — causing SIGSEGV at PC=0x00000000 at runtime.

Also detects:
- TEXTREL (text segment relocations — always bad for shared libs on IRIX)
- 3 PT_LOAD segments (-z separate-code — crashes rld)
"""

import os
import re
import subprocess
import tempfile
from dataclasses import dataclass, field
from pathlib import Path


@dataclass
class ElfSymbol:
    """A symbol from readelf -sW."""

    address: int
    size: int
    sym_type: str  # FUNC, OBJECT, NOTYPE, etc.
    binding: str  # LOCAL, GLOBAL, WEAK
    section: str  # UND, ABS, or section index
    name: str


@dataclass
class ElfRelocation:
    """An R_MIPS_REL32 relocation entry."""

    offset: int  # Address being relocated
    sym_name: str  # Target symbol name
    sym_value: int  # 0 for UNDEF symbols


@dataclass
class ClassRecFinding:
    """A ClassRec with at-risk relocations that rld may silently skip."""

    name: str
    address: int
    size: int
    binding: str  # LOCAL or GLOBAL
    at_risk_relocs: list[ElfRelocation] = field(default_factory=list)


@dataclass
class ElfCheckResult:
    """Result of checking a single ELF binary."""

    binary_path: Path
    is_mips_elf: bool = False
    elf_type: str = ""  # EXEC or DYN
    rel32_total: int = 0
    rel32_undef: int = 0  # R_MIPS_REL32 targeting UNDEF symbols
    class_rec_findings: list[ClassRecFinding] = field(default_factory=list)
    has_textrel: bool = False
    load_segment_count: int = 0
    warnings: list[str] = field(default_factory=list)

    @property
    def needs_fix_class_recs(self) -> bool:
        return len(self.class_rec_findings) > 0

    @property
    def total_at_risk_relocs(self) -> int:
        return sum(len(f.at_risk_relocs) for f in self.class_rec_findings)


def _is_mips_elf(path: Path) -> tuple[bool, str]:
    """Check if file is a MIPS ELF and return its type."""
    try:
        with open(path, "rb") as f:
            magic = f.read(4)
        if magic != b"\x7fELF":
            return False, ""
    except (OSError, PermissionError):
        return False, ""

    result = subprocess.run(
        ["readelf", "-h", str(path)],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        return False, ""

    is_mips = "MIPS" in result.stdout
    elf_type = ""
    for line in result.stdout.splitlines():
        if "Type:" in line:
            if "EXEC" in line:
                elf_type = "EXEC"
            elif "DYN" in line:
                elf_type = "DYN"
            break
    return is_mips, elf_type


def _parse_symbols(binary: Path) -> list[ElfSymbol]:
    """Parse symbol table from readelf -sW."""
    result = subprocess.run(
        ["readelf", "-sW", str(binary)],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        return []

    symbols = []
    # Match: "  1234: 011d4354   200 OBJECT  GLOBAL DEFAULT   15 xmlFolderClassRec"
    pattern = re.compile(
        r"^\s+\d+:\s+([0-9a-f]+)\s+(\d+)\s+(\w+)\s+(\w+)\s+\w+\s+(\w+)\s+(\S+)",
    )
    seen = set()
    for line in result.stdout.splitlines():
        m = pattern.match(line)
        if not m:
            continue
        addr = int(m.group(1), 16)
        size = int(m.group(2))
        sym_type = m.group(3)
        binding = m.group(4)
        section = m.group(5)
        name = m.group(6)
        key = (name, addr)
        if key in seen:
            continue
        seen.add(key)
        symbols.append(
            ElfSymbol(
                address=addr,
                size=size,
                sym_type=sym_type,
                binding=binding,
                section=section,
                name=name,
            )
        )
    return symbols


def _parse_relocations(binary: Path) -> list[ElfRelocation]:
    """Parse R_MIPS_REL32 relocations from readelf -r."""
    result = subprocess.run(
        ["readelf", "-r", str(binary)],
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        return []

    relocs = []
    # Match: "0117db50  00010803 R_MIPS_REL32      00000000   xmPrimitiveClassRec"
    pattern = re.compile(
        r"^([0-9a-f]+)\s+[0-9a-f]+\s+(R_MIPS_\w+)\s+([0-9a-f]+)\s+(\S+)",
    )
    for line in result.stdout.splitlines():
        m = pattern.match(line)
        if not m:
            continue
        if m.group(2) != "R_MIPS_REL32":
            continue
        relocs.append(
            ElfRelocation(
                offset=int(m.group(1), 16),
                sym_name=m.group(4),
                sym_value=int(m.group(3), 16),
            )
        )
    return relocs


def _check_dynamic(binary: Path) -> tuple[bool, int]:
    """Check for TEXTREL and count LOAD segments."""
    has_textrel = False
    load_count = 0

    # Check TEXTREL
    result = subprocess.run(
        ["readelf", "-d", str(binary)],
        capture_output=True,
        text=True,
    )
    if result.returncode == 0:
        has_textrel = "TEXTREL" in result.stdout

    # Count LOAD segments
    result = subprocess.run(
        ["readelf", "-l", str(binary)],
        capture_output=True,
        text=True,
    )
    if result.returncode == 0:
        for line in result.stdout.splitlines():
            if line.strip().startswith("LOAD"):
                load_count += 1

    return has_textrel, load_count


def _is_class_rec(sym: ElfSymbol) -> bool:
    """Check if a symbol looks like a widget ClassRec."""
    name_lower = sym.name.lower()
    return (
        name_lower.endswith("classrec")
        and sym.size > 0
        and sym.section != "UND"
        and sym.sym_type == "OBJECT"
    )


# Known Xt/Motif CoreClassPart field offsets for MIPS n32 (4-byte pointers).
# These help generate human-readable output.
_CORE_CLASS_FIELDS = {
    0x00: "superclass",
    0x20: "realize",
    0x28: "resize",
    0x2C: "expose",
    0x44: "set_values_almost",
    0x50: "query_geometry",
}

# Known symbols and their likely field roles
_SYMBOL_HINTS = {
    "_XtInherit": "inherited method",
    "_XtInheritTranslations": "translations",
    "xmManagerClassRec": "superclass (Manager)",
    "xmPrimitiveClassRec": "superclass (Primitive)",
    "xmPushButtonClassRec": "superclass (PushButton)",
    "objectClassRec": "superclass (Object)",
}


def check_binary(binary_path: Path) -> ElfCheckResult:
    """Check a single MIPS ELF binary for IRIX relocation issues."""
    result = ElfCheckResult(binary_path=binary_path)

    is_mips, elf_type = _is_mips_elf(binary_path)
    if not is_mips:
        return result

    result.is_mips_elf = True
    result.elf_type = elf_type

    # Parse symbols and relocations
    symbols = _parse_symbols(binary_path)
    relocs = _parse_relocations(binary_path)

    result.rel32_total = len(relocs)
    undef_relocs = [r for r in relocs if r.sym_value == 0]
    result.rel32_undef = len(undef_relocs)

    # Find ClassRec symbols
    class_recs = [s for s in symbols if _is_class_rec(s)]

    # Cross-reference: for each ClassRec, find R_MIPS_REL32 relocs within it
    for crec in class_recs:
        at_risk = [
            r
            for r in undef_relocs
            if crec.address <= r.offset < crec.address + crec.size
        ]
        if at_risk:
            result.class_rec_findings.append(
                ClassRecFinding(
                    name=crec.name,
                    address=crec.address,
                    size=crec.size,
                    binding=crec.binding,
                    at_risk_relocs=at_risk,
                )
            )

    # Check TEXTREL and LOAD segments
    has_textrel, load_count = _check_dynamic(binary_path)
    result.has_textrel = has_textrel
    result.load_segment_count = load_count

    # Generate warnings
    if elf_type == "DYN" and has_textrel:
        result.warnings.append("TEXTREL present in shared library (bad for IRIX)")
    if elf_type == "DYN" and load_count == 3:
        result.warnings.append(
            "3 LOAD segments (-z separate-code) — crashes IRIX rld"
        )
    if result.rel32_undef > 20 and not result.class_rec_findings:
        result.warnings.append(
            f"High R_MIPS_REL32 UNDEF count ({result.rel32_undef}) "
            f"but no *ClassRec symbols found — check for non-standard naming"
        )

    return result


def check_rpm(rpm_path: Path) -> list[ElfCheckResult]:
    """Extract an RPM and check all ELF binaries inside."""
    results = []
    with tempfile.TemporaryDirectory() as tmpdir:
        # Extract RPM
        subprocess.run(
            f"cd {tmpdir} && rpm2cpio {rpm_path.absolute()} | cpio -idm 2>/dev/null",
            shell=True,
            capture_output=True,
        )
        # Walk for ELF files
        for root, _dirs, files in os.walk(tmpdir):
            for fname in files:
                fpath = Path(root) / fname
                if not fpath.is_file() or fpath.is_symlink():
                    continue
                try:
                    with open(fpath, "rb") as f:
                        if f.read(4) != b"\x7fELF":
                            continue
                except (OSError, PermissionError):
                    continue
                r = check_binary(fpath)
                if r.is_mips_elf:
                    # Store the original RPM-relative path for display
                    rel = fpath.relative_to(tmpdir)
                    r.binary_path = Path("/") / rel
                    results.append(r)
    return results


def check_path(path: Path) -> list[ElfCheckResult]:
    """Check a path — RPM file, directory, or individual binary."""
    if path.suffix == ".rpm":
        return check_rpm(path)
    elif path.is_dir():
        results = []
        for root, _dirs, files in os.walk(path):
            for fname in files:
                fpath = Path(root) / fname
                if not fpath.is_file() or fpath.is_symlink():
                    continue
                try:
                    with open(fpath, "rb") as f:
                        if f.read(4) != b"\x7fELF":
                            continue
                except (OSError, PermissionError):
                    continue
                r = check_binary(fpath)
                if r.is_mips_elf:
                    results.append(r)
        return results
    else:
        r = check_binary(path)
        return [r] if r.is_mips_elf else []


def format_results(results: list[ElfCheckResult]) -> str:
    """Format check results for console output."""
    lines = []
    for r in results:
        if not r.is_mips_elf:
            continue

        lines.append(
            f"  {r.binary_path} [MIPS n32 {r.elf_type}, "
            f"{r.rel32_total} R_MIPS_REL32, {r.rel32_undef} UNDEF]"
        )

        if r.class_rec_findings:
            lines.append("")
            lines.append(
                f"  ⚠ {len(r.class_rec_findings)} ClassRec(s) "
                f"with {r.total_at_risk_relocs} at-risk relocations:"
            )
            lines.append("")
            for f in r.class_rec_findings:
                lines.append(
                    f"  {f.name} ({f.binding}, {f.size} bytes):"
                )
                for rel in f.at_risk_relocs:
                    offset = rel.offset - f.address
                    hint = _SYMBOL_HINTS.get(rel.sym_name, "")
                    hint_str = f"  [{hint}]" if hint else ""
                    lines.append(
                        f"    +0x{offset:03x} → {rel.sym_name}{hint_str}"
                    )
                lines.append("")

        for w in r.warnings:
            lines.append(f"  ⚠ {w}")

        if not r.class_rec_findings and not r.warnings:
            lines.append("  ✓ No issues found")

        lines.append("")

    return "\n".join(lines)


def generate_fix_scaffold(results: list[ElfCheckResult]) -> str:
    """Generate a fix_class_recs.c scaffold from check results."""
    findings = []
    for r in results:
        findings.extend(r.class_rec_findings)

    if not findings:
        return "/* No at-risk ClassRec relocations found. */\n"

    lines = []
    lines.append("/*")
    lines.append(
        " * fix_class_recs.c — Patch widget class record pointers broken by IRIX rld"
    )
    lines.append(" *")
    lines.append(
        " * AUTO-GENERATED by: mogrix check-elf --generate-fix"
    )
    lines.append(
        " * REVIEW THIS FILE — field names at offsets are best guesses."
    )
    lines.append(" *")
    for f in findings:
        lines.append(f" * {f.name} @ 0x{f.address:08x} ({f.size} bytes):")
        for rel in f.at_risk_relocs:
            offset = rel.offset - f.address
            lines.append(f" *   +0x{offset:03x} → {rel.sym_name}")
    lines.append(" */")
    lines.append("")
    lines.append("#include <X11/IntrinsicP.h>")
    lines.append("#include <X11/ObjectP.h>")
    lines.append("#include <Xm/XmP.h>")
    lines.append("#include <Xm/ManagerP.h>")
    lines.append("#include <Xm/PrimitiveP.h>")
    lines.append("")
    lines.append("extern void _XtInherit(void);")
    lines.append("extern int _XtInheritTranslations;")
    lines.append("")

    # Determine parent types for extern declarations
    for f in findings:
        parent = _guess_parent_type(f)
        lines.append(f"extern {parent} {f.name};")
    lines.append("")

    lines.append(
        "static void fix_class_superclass(void) __attribute__((constructor));"
    )
    lines.append("")
    lines.append("static void fix_class_superclass(void)")
    lines.append("{")

    for f in findings:
        lines.append(f"    /* {f.name} */")
        parent = _guess_parent_type(f)
        for rel in f.at_risk_relocs:
            offset = rel.offset - f.address
            field_name = _CORE_CLASS_FIELDS.get(offset)

            if field_name == "superclass" and offset == 0:
                # Superclass pointer — guess the target
                lines.append(
                    f"    if ({f.name}.core_class.superclass == (WidgetClass)0)"
                )
                lines.append(
                    f"        {f.name}.core_class.superclass = "
                    f"(WidgetClass)&{rel.sym_name};"
                )
            elif rel.sym_name == "_XtInheritTranslations":
                lines.append(
                    f"    /* +0x{offset:03x} → _XtInheritTranslations "
                    f"(use pointer arithmetic if field unknown) */"
                )
                lines.append(
                    f"    /* TODO: identify which translations field "
                    f"at offset +0x{offset:03x} */",
                )
            elif rel.sym_name == "_XtInherit":
                if field_name:
                    lines.append(
                        f"    /* +0x{offset:03x} → _XtInherit "
                        f"({field_name}) */"
                    )
                else:
                    lines.append(
                        f"    /* +0x{offset:03x} → _XtInherit "
                        f"(identify field at this offset) */"
                    )
            else:
                lines.append(
                    f"    /* +0x{offset:03x} → {rel.sym_name} */"
                )
        lines.append("")

    lines.append("    /* TODO: Fill in NULL-check + assignment for each field.")
    lines.append("     * Pattern:")
    lines.append(
        "     *   if (classRec.core_class.field == (Type)0)"
    )
    lines.append(
        "     *       classRec.core_class.field = (Type)_XtInherit;"
    )
    lines.append(
        "     * For custom class parts beyond the parent type,"
    )
    lines.append("     * use pointer arithmetic:")
    lines.append(
        "     *   void **pp = (void **)((char *)&classRec + sizeof(ParentClassRec) + offset);"
    )
    lines.append("     *   if (*pp == NULL) *pp = (void *)_XtInherit;")
    lines.append("     */")
    lines.append("}")
    lines.append("")
    return "\n".join(lines)


def _guess_parent_type(finding: ClassRecFinding) -> str:
    """Guess the parent Xt/Motif class type for extern declaration."""
    # Check what superclass symbol is referenced
    for rel in finding.at_risk_relocs:
        offset = rel.offset - finding.address
        if offset == 0:  # superclass pointer
            if "Manager" in rel.sym_name or "manager" in rel.sym_name:
                return "XmManagerClassRec"
            elif "Primitive" in rel.sym_name or "primitive" in rel.sym_name:
                return "XmPrimitiveClassRec"
            elif "PushButton" in rel.sym_name or "pushButton" in rel.sym_name:
                return "XmPushButtonClassRec"
            elif "object" in rel.sym_name.lower():
                return "ObjectClassRec"
    # Default
    return "XmManagerClassRec"
