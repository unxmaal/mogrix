#!/usr/bin/env python3
"""
Add IRIX .liblist section to cross-compiled ELF binaries.

IRIX's runtime linker (rld) requires a .liblist section to find shared libraries.
Standard ELF DT_NEEDED entries are not sufficient for IRIX.

This tool post-processes cross-compiled binaries to add the necessary .liblist
section based on the DT_NEEDED entries in the dynamic section.

The .liblist section contains Elf32_Lib entries:
  struct Elf32_Lib {
    uint32_t l_name;       // String table index for library name
    uint32_t l_time_stamp; // Timestamp (0 for cross-compiled)
    uint32_t l_checksum;   // Checksum (0 for cross-compiled)
    uint32_t l_version;    // String table index for version (0)
    uint32_t l_flags;      // Flags (0)
  };

References:
- MIPS ELF Spec: https://irix7.com/techpubs/007-4658-001.pdf
- rld man page: https://nixdoc.net/man-pages/irix/man2/man1/rld.1.html
"""

import argparse
import struct
import sys
from pathlib import Path

try:
    import lief
except ImportError:
    print("Error: LIEF library not installed. Run: pip install lief", file=sys.stderr)
    sys.exit(1)


# MIPS dynamic tag values
DT_MIPS_LIBLIST = 0x70000009
DT_MIPS_LIBLISTNO = 0x70000010

# Elf32_Lib entry size (5 x 4 bytes)
ELF32_LIB_SIZE = 20


def create_liblist_entry(name_offset: int, flags: int = 0) -> bytes:
    """Create an Elf32_Lib entry.

    Args:
        name_offset: Offset into dynamic string table for library name
        flags: Library flags (default 0)

    Returns:
        20-byte Elf32_Lib structure (big-endian for MIPS)
    """
    return struct.pack(
        ">IIIII",  # Big-endian, 5 x uint32
        name_offset,  # l_name
        0,  # l_time_stamp
        0,  # l_checksum
        0,  # l_version
        flags,  # l_flags
    )


def get_dynstr_offset(binary: lief.ELF.Binary, name: str) -> int | None:
    """Get the offset of a string in the dynamic string table.

    Args:
        binary: LIEF ELF binary
        name: String to find

    Returns:
        Offset in .dynstr section, or None if not found
    """
    dynstr = binary.get_section(".dynstr")
    if dynstr is None:
        return None

    content = bytes(dynstr.content)
    # Search for the string (null-terminated)
    search = name.encode("utf-8") + b"\x00"
    offset = content.find(search)
    if offset >= 0:
        return offset
    return None


def add_liblist_section(
    input_path: Path, output_path: Path, verbose: bool = False
) -> bool:
    """Add .liblist section to an ELF binary.

    This uses a two-pass approach because LIEF relocates sections when modifying:
    1. First pass: Add placeholder liblist section and dynamic tags
    2. Second pass: Read the modified binary, get correct string offsets, update liblist

    Args:
        input_path: Path to input ELF file
        output_path: Path to output ELF file
        verbose: Print verbose output

    Returns:
        True on success, False on failure
    """
    # Parse the ELF file
    binary = lief.ELF.parse(str(input_path))
    if binary is None:
        print(f"Error: Could not parse {input_path}", file=sys.stderr)
        return False

    # Check if it's MIPS
    if binary.header.machine_type != lief.ELF.ARCH.MIPS:
        print(f"Error: {input_path} is not a MIPS binary", file=sys.stderr)
        return False

    # Check for existing .liblist
    if binary.get_section(".liblist") is not None:
        print(f"Warning: {input_path} already has .liblist section", file=sys.stderr)
        return True

    # Get NEEDED libraries from dynamic section
    needed_libs = []
    for entry in binary.dynamic_entries:
        if entry.tag == lief.ELF.DynamicEntry.TAG.NEEDED:
            needed_libs.append(entry.name)

    if not needed_libs:
        if verbose:
            print(f"No NEEDED libraries in {input_path}")
        return True

    if verbose:
        print(f"Found {len(needed_libs)} NEEDED libraries:")
        for lib in needed_libs:
            print(f"  - {lib}")

    # Create placeholder liblist content (will be updated in second pass)
    # Use exact size needed: num_libs * 20 bytes per entry
    liblist_size = len(needed_libs) * ELF32_LIB_SIZE
    liblist_content = bytearray(liblist_size)

    # Create the .liblist section with exact size
    liblist_section = lief.ELF.Section(".liblist")
    liblist_section.type = lief.ELF.Section.TYPE.MIPS_LIBLIST
    liblist_section.flags = lief.ELF.Section.FLAGS.ALLOC
    liblist_section.entry_size = ELF32_LIB_SIZE
    liblist_section.content = list(liblist_content)

    # Link to .dynstr section
    dynstr = binary.get_section(".dynstr")
    if dynstr is not None:
        for i, sec in enumerate(binary.sections):
            if sec.name == ".dynstr":
                liblist_section.link = i
                break

    # Add the section
    added_section = binary.add(liblist_section, loaded=True)

    # Add MIPS_LIBLIST dynamic tag
    liblist_entry = lief.ELF.DynamicEntry()
    liblist_entry.tag = lief.ELF.DynamicEntry.TAG.MIPS_LIBLIST
    liblist_entry.value = added_section.virtual_address
    binary.add(liblist_entry)

    # Add MIPS_LIBLISTNO dynamic tag
    liblistno_entry = lief.ELF.DynamicEntry()
    liblistno_entry.tag = lief.ELF.DynamicEntry.TAG.MIPS_LIBLISTNO
    liblistno_entry.value = len(needed_libs)
    binary.add(liblistno_entry)

    if verbose:
        print(f"Pass 1: Added placeholder .liblist section")

    # Write the modified binary (first pass)
    binary.write(str(output_path))

    # ---- SECOND PASS: Update liblist with correct offsets ----

    # Re-parse the modified binary
    binary2 = lief.ELF.parse(str(output_path))
    if binary2 is None:
        print(f"Error: Could not re-parse {output_path}", file=sys.stderr)
        return False

    # Get the correct string offsets from the modified dynstr
    liblist_content = bytearray()
    for lib in needed_libs:
        offset = get_dynstr_offset(binary2, lib)
        if offset is None:
            print(f"Warning: Could not find '{lib}' in modified .dynstr", file=sys.stderr)
            offset = 0

        entry = create_liblist_entry(offset)
        liblist_content.extend(entry)

        if verbose:
            print(f"  Added liblist entry for '{lib}' at offset {offset}")

    # Update the .liblist section content
    liblist_sec = binary2.get_section(".liblist")
    if liblist_sec is None:
        print(f"Error: Could not find .liblist in modified binary", file=sys.stderr)
        return False

    # Set the content (LIEF may have padded it, so we need to be careful)
    liblist_sec.content = list(liblist_content)

    if verbose:
        print(f"Pass 2: Updated .liblist with correct offsets")
        print(f"  Section size: {len(liblist_content)} bytes ({len(needed_libs)} entries)")

    # Write final binary
    binary2.write(str(output_path))

    if verbose:
        print(f"Wrote modified binary to {output_path}")

    return True


def main():
    parser = argparse.ArgumentParser(
        description="Add IRIX .liblist section to cross-compiled ELF binaries"
    )
    parser.add_argument("input", type=Path, help="Input ELF file")
    parser.add_argument(
        "-o", "--output", type=Path, help="Output file (default: overwrite input)"
    )
    parser.add_argument("-v", "--verbose", action="store_true", help="Verbose output")

    args = parser.parse_args()

    if not args.input.exists():
        print(f"Error: {args.input} does not exist", file=sys.stderr)
        sys.exit(1)

    output = args.output or args.input

    success = add_liblist_section(args.input, output, verbose=args.verbose)
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
