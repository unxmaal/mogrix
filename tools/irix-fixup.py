#!/usr/bin/env python3
"""
IRIX ELF fixup tool - removes problematic sections and program headers.

IRIX 6.5 has issues with certain ELF features:
- PT_MIPS_ABIFLAGS (type 0x70000003) - crashes IRIX ELF parser
- SHT_MIPS_ABIFLAGS (type 0x7000002a) - unknown section type
- PT_PHDR in shared libraries - not present in working IRIX .so files

Usage: irix-fixup.py input.so output.so
"""

import sys
import struct
from pathlib import Path

# ELF constants
SHT_MIPS_ABIFLAGS = 0x7000002a
PT_MIPS_ABIFLAGS = 0x70000003
PT_PHDR = 6
PT_LOAD = 1

# ELF32 header offsets
E_PHOFF = 28
E_SHOFF = 32
E_PHENTSIZE = 42
E_PHNUM = 44
E_SHENTSIZE = 46
E_SHNUM = 48

# Program header entry (32-bit)
PH_TYPE = 0

# Section header entry (32-bit)
SH_TYPE = 4


def read_u16(data, offset):
    return struct.unpack('>H', data[offset:offset+2])[0]

def read_u32(data, offset):
    return struct.unpack('>I', data[offset:offset+4])[0]

def write_u16(data, offset, value):
    data[offset:offset+2] = struct.pack('>H', value)


def fixup_elf(input_path, output_path):
    with open(input_path, 'rb') as f:
        data = bytearray(f.read())

    if data[:4] != b'\x7fELF':
        print(f"Error: {input_path} is not an ELF file")
        return False

    if data[5] != 2:
        print(f"Error: Expected big-endian ELF")
        return False

    phoff = read_u32(data, E_PHOFF)
    shoff = read_u32(data, E_SHOFF)
    phentsize = read_u16(data, E_PHENTSIZE)
    phnum = read_u16(data, E_PHNUM)
    shentsize = read_u16(data, E_SHENTSIZE)
    shnum = read_u16(data, E_SHNUM)

    print(f"Program headers: {phnum} at offset {phoff:#x}")
    print(f"Section headers: {shnum} at offset {shoff:#x}")

    # Collect program headers to remove
    phdrs_to_remove = []
    for i in range(phnum):
        ph_offset = phoff + i * phentsize
        ph_type = read_u32(data, ph_offset + PH_TYPE)
        if ph_type == PT_MIPS_ABIFLAGS:
            print(f"Found PT_MIPS_ABIFLAGS at program header {i} - marking for removal")
            phdrs_to_remove.append(i)
        elif ph_type == PT_PHDR:
            print(f"Found PT_PHDR at program header {i} - marking for removal")
            phdrs_to_remove.append(i)

    # Find section to zero
    abiflags_shdr_idx = None
    for i in range(shnum):
        sh_offset = shoff + i * shentsize
        sh_type = read_u32(data, sh_offset + SH_TYPE)
        if sh_type == SHT_MIPS_ABIFLAGS:
            print(f"Found SHT_MIPS_ABIFLAGS at section header {i}")
            abiflags_shdr_idx = i
            break

    if not phdrs_to_remove and abiflags_shdr_idx is None:
        print("Nothing to fix")
        with open(output_path, 'wb') as f:
            f.write(data)
        return True

    # Remove program headers (in reverse order to maintain indices)
    phdrs_to_remove.sort(reverse=True)
    current_phnum = phnum
    for idx in phdrs_to_remove:
        print(f"Removing program header {idx}")
        src_start = phoff + (idx + 1) * phentsize
        dst_start = phoff + idx * phentsize
        remaining = (current_phnum - idx - 1) * phentsize
        if remaining > 0:
            data[dst_start:dst_start+remaining] = data[src_start:src_start+remaining]
        last_start = phoff + (current_phnum - 1) * phentsize
        for j in range(phentsize):
            data[last_start + j] = 0
        current_phnum -= 1

    write_u16(data, E_PHNUM, current_phnum)
    print(f"Updated e_phnum to {current_phnum}")

    # Zero out section header
    if abiflags_shdr_idx is not None:
        sh_offset = shoff + abiflags_shdr_idx * shentsize
        print(f"Zeroing section header {abiflags_shdr_idx}")
        for j in range(shentsize):
            data[sh_offset + j] = 0

    with open(output_path, 'wb') as f:
        f.write(data)

    print(f"Wrote fixed ELF to {output_path}")
    return True


def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} input.so output.so")
        sys.exit(1)

    if fixup_elf(sys.argv[1], sys.argv[2]):
        sys.exit(0)
    else:
        sys.exit(1)


if __name__ == '__main__':
    main()
