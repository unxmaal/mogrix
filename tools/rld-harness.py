#!/usr/bin/env python3
"""
Reusable rld bundle analysis harness.

Analyzes MIPS n32 ELF bundles for IRIX rld compatibility issues.
Three subcommands:

  analyze       - Static ELF checks on every .so in a bundle (Linux-only)
  got-inspect   - Deep GOT analysis for a specific .so file
  test-loading  - Binary-search which library crashes rld (requires IRIX)

Usage:
  python3 tools/rld-harness.py analyze /path/to/bundle
  python3 tools/rld-harness.py got-inspect /path/to/lib.so
  python3 tools/rld-harness.py test-loading /path/to/bundle
"""

import argparse
import json
import os
import struct
import subprocess
import sys
import tempfile
from collections import defaultdict
from pathlib import Path

# --- Thresholds from rld decompilation + production experience ---

THRESHOLDS = {
    "global_got_limit": 4370,       # rld can't fill GOT above this
    "local_gotno_reencounter": 128,  # dlopen re-encounter danger zone
    "multi_got_trigger": 16384,     # LLD default --mips-got-size split
}

PROJECT_ROOT = Path(__file__).resolve().parent.parent
RESULTS_DIR = PROJECT_ROOT / "test-results"
HARNESS_SRC = PROJECT_ROOT / "tests" / "rld" / "src" / "harness_dlopen.c"

# IRIX system libraries — always available, never bundled
IRIX_SYSTEM_LIBS = {
    "libc.so.1", "libm.so", "libpthread.so", "libdl.so",
    "libnsl.so", "libgen.so", "librld.so", "libmalloc.so",
    "libcurses.so", "libtermcap.so", "libw.so",
    "libX11.so.1", "libXext.so", "libXt.so", "libXm.so",
    "libGL.so", "libGLcore.so.1", "libGLU.so",
}


# ============================================================
# MipsElf — struct-based binary ELF parser for MIPS n32
# ============================================================

class MipsElf:
    """Parse MIPS n32 ELF binary for rld analysis.

    Adapted from cross/bin/fix-got-stubs and cross/bin/fix-anon-relocs.
    Direct binary parsing — no readelf subprocess needed.
    """

    # ELF constants
    PT_LOAD = 1
    PT_DYNAMIC = 2
    SHT_REL = 9
    SHT_DYNSYM = 11
    DT_NEEDED = 1
    DT_INIT = 12
    DT_RPATH = 15
    DT_RUNPATH = 29
    DT_VERNEED = 0x6FFFFFFE
    DT_VERSYM = 0x6FFFFFF0
    DT_VERNEEDNUM = 0x6FFFFFFF
    DT_MIPS_LOCAL_GOTNO = 0x7000000A
    DT_MIPS_SYMTABNO = 0x70000011
    DT_MIPS_GOTSYM = 0x70000013

    def __init__(self, path):
        self.path = Path(path)
        self.data = self.path.read_bytes()
        self.name = self.path.name
        self.valid = False
        self.is_shared = False
        self.is_exec = False
        self.error = None

        # Parsed fields
        self.local_gotno = 0
        self.gotsym = 0
        self.symtabno = 0
        self.got_offset = 0
        self.got_size = 0
        self.got_addr = 0
        self.stubs_addr = 0
        self.stubs_size = 0
        self.load_segments = []
        self.dynamic_tags = {}
        self.dynamic_tag_list = []  # preserve order for NEEDED
        self.sections = {}  # name -> {addr, offset, size, type}
        self.section_names = []
        self.dynsym_offset = 0
        self.dynsym_entsize = 16
        self.dynsym_count = 0
        self.strtab_offset = 0

        self._parse()

    def _r32(self, off):
        return struct.unpack_from('>I', self.data, off)[0]

    def _r16(self, off):
        return struct.unpack_from('>H', self.data, off)[0]

    def _r8(self, off):
        return self.data[off]

    def _rs32(self, off):
        return struct.unpack_from('>i', self.data, off)[0]

    def _read_cstring(self, off):
        end = self.data.index(0, off)
        return self.data[off:end].decode('ascii', errors='replace')

    def _parse(self):
        d = self.data
        if len(d) < 52 or d[:4] != b'\x7fELF':
            self.error = "Not an ELF file"
            return
        if d[4] != 1:  # ELFCLASS32
            self.error = "Not 32-bit ELF"
            return
        if d[5] != 2:  # ELFDATA2MSB
            self.error = "Not big-endian"
            return

        ei_type = self._r16(16)
        self.is_shared = (ei_type == 3)  # ET_DYN
        self.is_exec = (ei_type == 2)    # ET_EXEC
        if not self.is_shared and not self.is_exec:
            self.error = f"Unexpected ELF type {ei_type}"
            return

        self.valid = True

        # ELF header fields
        e_phoff = self._r32(28)
        e_shoff = self._r32(32)
        e_phentsize = self._r16(42)
        e_phnum = self._r16(44)
        e_shentsize = self._r16(46)
        e_shnum = self._r16(48)
        e_shstrndx = self._r16(50)

        # --- Program headers ---
        dyn_offset = 0
        dyn_size = 0
        for i in range(e_phnum):
            ph = e_phoff + i * e_phentsize
            p_type = self._r32(ph)
            p_offset = self._r32(ph + 4)
            p_vaddr = self._r32(ph + 8)
            p_filesz = self._r32(ph + 16)
            p_memsz = self._r32(ph + 20)
            p_flags = self._r32(ph + 24)

            if p_type == self.PT_LOAD:
                self.load_segments.append({
                    'vaddr': p_vaddr, 'offset': p_offset,
                    'filesz': p_filesz, 'memsz': p_memsz,
                    'flags': p_flags,
                })
            elif p_type == self.PT_DYNAMIC:
                dyn_offset = p_offset
                dyn_size = p_filesz

        # --- Section headers ---
        if e_shoff > 0 and e_shnum > 0:
            shstr_sh = e_shoff + e_shstrndx * e_shentsize
            shstr_off = self._r32(shstr_sh + 16)

            for i in range(e_shnum):
                sh = e_shoff + i * e_shentsize
                sh_name_off = self._r32(sh)
                sh_type = self._r32(sh + 4)
                sh_addr = self._r32(sh + 12)
                sh_offset = self._r32(sh + 16)
                sh_size = self._r32(sh + 20)
                sh_link = self._r32(sh + 24)
                sh_entsize = self._r32(sh + 36)

                try:
                    name = self._read_cstring(shstr_off + sh_name_off)
                except (ValueError, IndexError):
                    name = f"<section_{i}>"

                self.section_names.append(name)
                self.sections[name] = {
                    'addr': sh_addr, 'offset': sh_offset,
                    'size': sh_size, 'type': sh_type,
                    'link': sh_link, 'entsize': sh_entsize,
                }

                if name == '.got':
                    self.got_addr = sh_addr
                    self.got_offset = sh_offset
                    self.got_size = sh_size
                elif name == '.MIPS.stubs':
                    self.stubs_addr = sh_addr
                    self.stubs_size = sh_size
                elif sh_type == self.SHT_DYNSYM:
                    self.dynsym_offset = sh_offset
                    self.dynsym_entsize = sh_entsize or 16
                    self.dynsym_count = sh_size // (sh_entsize or 16)
                    # .dynstr is the linked section
                    strtab_sh = e_shoff + sh_link * e_shentsize
                    self.strtab_offset = self._r32(strtab_sh + 16)

        # --- Dynamic tags ---
        if dyn_offset > 0:
            pos = dyn_offset
            while pos < dyn_offset + dyn_size:
                d_tag = self._rs32(pos)
                d_val = self._r32(pos + 4)
                if d_tag == 0:
                    break
                self.dynamic_tag_list.append((d_tag, d_val))
                # Store last value for each tag (some may repeat, e.g. NEEDED)
                if d_tag not in self.dynamic_tags:
                    self.dynamic_tags[d_tag] = d_val
                pos += 8

            self.local_gotno = self.dynamic_tags.get(self.DT_MIPS_LOCAL_GOTNO, 0)
            self.gotsym = self.dynamic_tags.get(self.DT_MIPS_GOTSYM, 0)
            self.symtabno = self.dynamic_tags.get(self.DT_MIPS_SYMTABNO, 0)

    @property
    def global_got_count(self):
        if self.gotsym and self.symtabno:
            return self.symtabno - self.gotsym
        return 0

    def needed_sonames(self):
        """Return list of DT_NEEDED sonames in order."""
        result = []
        for tag, val in self.dynamic_tag_list:
            if tag == self.DT_NEEDED and self.strtab_offset:
                try:
                    name = self._read_cstring(self.strtab_offset + val)
                    result.append(name)
                except (ValueError, IndexError):
                    result.append(f"<strtab+{val}>")
        return result

    def has_rpath(self):
        return (self.DT_RPATH in self.dynamic_tags or
                self.DT_RUNPATH in self.dynamic_tags)

    def has_verneed(self):
        return (self.DT_VERNEED in self.dynamic_tags or
                self.DT_VERSYM in self.dynamic_tags or
                self.DT_VERNEEDNUM in self.dynamic_tags)

    def has_dt_init(self):
        return self.DT_INIT in self.dynamic_tags

    def has_init_array(self):
        return '.init_array' in self.sections

    def has_ctors(self):
        return '.ctors' in self.sections

    def vaddr_to_foff(self, va):
        """Convert virtual address to file offset."""
        for seg in self.load_segments:
            if seg['vaddr'] <= va < seg['vaddr'] + seg['filesz']:
                return seg['offset'] + (va - seg['vaddr'])
        return None

    # --- Check methods (return (passed: bool, message: str)) ---

    def check_load_segments(self):
        count = len(self.load_segments)
        if count == 2:
            return True, f"{count} LOAD segments"
        return False, f"{count} LOAD segments (expected 2, rld crashes on 3)"

    def check_no_verneed(self):
        if self.has_verneed():
            tags = []
            if self.DT_VERNEED in self.dynamic_tags:
                tags.append("VERNEED")
            if self.DT_VERSYM in self.dynamic_tags:
                tags.append("VERSYM")
            return False, f"has version tags: {', '.join(tags)}"
        return True, "no version tags"

    def check_no_init_array(self):
        issues = []
        if self.has_init_array():
            issues.append("has .init_array (rld ignores)")
        if not self.has_ctors():
            issues.append("missing .ctors")
        if issues:
            return False, "; ".join(issues)
        return True, "uses .ctors"

    def check_dt_init(self):
        if self.has_dt_init():
            return True, "DT_INIT present"
        return False, "no DT_INIT tag"

    def check_got_thresholds(self):
        gc = self.global_got_count
        limit = THRESHOLDS["global_got_limit"]
        if gc >= limit:
            return False, f"global GOT {gc} >= {limit} threshold"
        if gc > limit * 0.8:
            return True, f"global GOT {gc} (WARNING: >{int(limit*0.8)}, approaching limit)"
        return True, f"global GOT {gc}"

    def check_no_rpath(self):
        if self.has_rpath():
            return False, "has RPATH/RUNPATH (build paths break rld)"
        return True, "no RPATH"

    def check_stub_alignment(self):
        """Verify GOT[LOCAL_GOTNO+i] = stubs_base + i*16."""
        if self.got_size == 0 or self.stubs_size == 0:
            return True, "no stubs section (static or exe)"
        if self.local_gotno == 0 or self.global_got_count == 0:
            return True, "no global GOT entries"

        first_global_off = self.got_offset + self.local_gotno * 4
        if first_global_off + 4 > len(self.data):
            return False, "GOT offset out of bounds"

        current_first = self._r32(first_global_off)
        expected_first = self.stubs_addr

        if current_first == expected_first:
            return True, "GOT-stub alignment correct"

        # Check reversed pattern
        num_stubs = self.stubs_size // 16
        if num_stubs > 0:
            expected_reversed = self.stubs_addr + (num_stubs - 1) * 16
            if current_first == expected_reversed:
                return False, "GOT-stub REVERSED (fix-got-stubs not applied)"

        return False, f"GOT[{self.local_gotno}]=0x{current_first:08x}, expected 0x{expected_first:08x}"

    def run_all_checks(self):
        """Run all checks, return list of (name, passed, message)."""
        checks = [
            ("load_segments", self.check_load_segments),
            ("no_verneed", self.check_no_verneed),
            ("no_init_array", self.check_no_init_array),
            ("dt_init", self.check_dt_init),
            ("got_thresholds", self.check_got_thresholds),
            ("no_rpath", self.check_no_rpath),
            ("stub_alignment", self.check_stub_alignment),
        ]
        results = []
        for name, fn in checks:
            try:
                passed, msg = fn()
                results.append((name, passed, msg))
            except Exception as e:
                results.append((name, False, f"ERROR: {e}"))
        return results

    # --- GOT inspection methods ---

    def read_got_entries(self):
        """Read all GOT entries as list of 32-bit values."""
        if self.got_size == 0:
            return []
        count = self.got_size // 4
        entries = []
        for i in range(count):
            off = self.got_offset + i * 4
            if off + 4 > len(self.data):
                break
            entries.append(self._r32(off))
        return entries

    def read_dynsym_entries(self):
        """Read .dynsym entries: list of (index, name, value, size, bind, type, vis, shndx)."""
        if self.dynsym_offset == 0:
            return []
        entries = []
        for i in range(self.dynsym_count):
            off = self.dynsym_offset + i * self.dynsym_entsize
            if off + 16 > len(self.data):
                break
            st_name_off = self._r32(off)
            st_value = self._r32(off + 4)
            st_size = self._r32(off + 8)
            st_info = self._r8(off + 12)
            st_other = self._r8(off + 13)
            st_shndx = self._r16(off + 14)

            bind = st_info >> 4
            stype = st_info & 0xf
            vis = st_other & 0x3

            try:
                name = self._read_cstring(self.strtab_offset + st_name_off) if self.strtab_offset else ""
            except (ValueError, IndexError):
                name = f"<strtab+{st_name_off}>"

            entries.append({
                'index': i, 'name': name, 'value': st_value,
                'size': st_size, 'bind': bind, 'type': stype,
                'vis': vis, 'shndx': st_shndx,
            })
        return entries

    def read_rel_entries(self):
        """Read .rel.dyn entries: list of (offset, sym_idx, type)."""
        rel = self.sections.get('.rel.dyn')
        if not rel:
            return []
        entries = []
        off = rel['offset']
        end = off + rel['size']
        while off < end:
            r_offset = self._r32(off)
            r_info = self._r32(off + 4)
            sym_idx = r_info >> 8
            rtype = r_info & 0xFF
            entries.append((r_offset, sym_idx, rtype))
            off += 8
        return entries


# ============================================================
# analyze — Static bundle analysis
# ============================================================

def find_bundle_libs(bundle_path):
    """Find all .so files in a bundle directory."""
    bundle = Path(bundle_path)
    libs = []

    # Check common bundle layouts
    for pattern in ['lib/*.so*', 'lib32/*.so*', '_lib32/*.so*', '_lib/*.so*',
                    '*.so*', 'usr/sgug/lib32/*.so*']:
        for p in bundle.glob(pattern):
            if p.is_file() and not p.is_symlink():
                libs.append(p)

    # Deduplicate (glob patterns can overlap)
    seen = set()
    unique = []
    for p in libs:
        rp = p.resolve()
        if rp not in seen:
            seen.add(rp)
            unique.append(p)

    return sorted(unique, key=lambda p: p.name)


def build_dep_graph(elfs):
    """Build transitive NEEDED dependency graph.

    Returns dict: soname -> {depth, needed, needed_by, transitive_deps}
    """
    by_name = {}
    for elf in elfs:
        by_name[elf.name] = elf
        # Index by soname prefix patterns for version matching:
        #   libfoo.so.1.2.3 should be findable as libfoo.so.1, libfoo.so
        name = elf.name
        while '.so' in name:
            # Strip last .N component
            dot_pos = name.rfind('.')
            if dot_pos <= 0:
                break
            prefix = name[:dot_pos]
            # Only register if the prefix still contains .so
            if '.so' in prefix or prefix.endswith('.so'):
                if prefix not in by_name:
                    by_name[prefix] = elf
            name = prefix

    def resolve_dep(soname):
        """Resolve a NEEDED soname to a graph key (filename) via by_name."""
        if soname in by_name:
            return by_name[soname].name
        return None

    graph = {}
    for elf in elfs:
        # Resolve NEEDED sonames to actual filenames in the bundle
        raw_needed = elf.needed_sonames()
        resolved_needed = []
        for dep in raw_needed:
            real = resolve_dep(dep)
            if real:
                resolved_needed.append(real)
            else:
                resolved_needed.append(dep)  # keep for unresolved tracking

        graph[elf.name] = {
            'needed': raw_needed,
            'resolved_needed': resolved_needed,
            'needed_by': [],
            'depth': 0,
            'transitive_count': 0,
        }

    # Build reverse edges using resolved names
    for name, info in graph.items():
        for dep in info['resolved_needed']:
            if dep in graph:
                graph[dep]['needed_by'].append(name)

    # BFS from roots (libraries not needed by anyone in the bundle) to compute depths
    roots = [n for n, info in graph.items() if not info['needed_by']]
    if not roots:
        # Circular — just pick all as roots
        roots = list(graph.keys())

    visited = set()
    queue = [(r, 0) for r in roots]
    while queue:
        name, depth = queue.pop(0)
        if name in visited:
            continue
        visited.add(name)
        graph[name]['depth'] = max(graph[name]['depth'], depth)
        for dep in graph[name]['resolved_needed']:
            if dep in graph:
                queue.append((dep, depth + 1))

    # Count transitive deps
    def count_transitive(name, seen=None):
        if seen is None:
            seen = set()
        if name in seen or name not in graph:
            return 0
        seen.add(name)
        total = 0
        for dep in graph[name]['resolved_needed']:
            if dep in graph and dep not in seen:
                total += 1 + count_transitive(dep, seen)
        return total

    for name in graph:
        graph[name]['transitive_count'] = count_transitive(name)

    return graph


def cmd_analyze(args):
    """Run static ELF analysis on all .so files in a bundle."""
    bundle_path = Path(args.path)
    if not bundle_path.exists():
        print(f"ERROR: {bundle_path} does not exist", file=sys.stderr)
        return 1

    if bundle_path.is_file() and bundle_path.suffix == '.so' or '.so.' in bundle_path.name:
        # Single file mode
        libs = [bundle_path]
        bundle_name = bundle_path.stem
    else:
        libs = find_bundle_libs(bundle_path)
        bundle_name = bundle_path.name

    if not libs:
        print(f"ERROR: No .so files found in {bundle_path}", file=sys.stderr)
        return 1

    print(f"Analyzing {len(libs)} libraries in {bundle_path}")
    print(f"{'='*70}")

    # Parse all ELFs
    elfs = []
    for lib_path in libs:
        elf = MipsElf(lib_path)
        if elf.valid:
            elfs.append(elf)
        else:
            print(f"  SKIP: {lib_path.name}: {elf.error}")

    # Run checks on each
    report = {
        'bundle': str(bundle_path),
        'library_count': len(elfs),
        'libraries': {},
        'dependency_graph': {},
        'risk_ranking': [],
        'unresolved_deps': [],
        'summary': {'pass': 0, 'fail': 0, 'total_checks': 0},
    }

    all_sonames = {e.name for e in elfs}
    total_pass = 0
    total_fail = 0

    for elf in elfs:
        results = elf.run_all_checks()
        lib_info = {
            'checks': {},
            'global_got': elf.global_got_count,
            'local_gotno': elf.local_gotno,
            'symtabno': elf.symtabno,
            'gotsym': elf.gotsym,
            'load_segments': len(elf.load_segments),
            'needed': elf.needed_sonames(),
            'has_ctors': elf.has_ctors(),
            'has_init_array': elf.has_init_array(),
        }

        lib_pass = 0
        lib_fail = 0
        for name, passed, msg in results:
            lib_info['checks'][name] = {'passed': passed, 'message': msg}
            if passed:
                lib_pass += 1
            else:
                lib_fail += 1

        total_pass += lib_pass
        total_fail += lib_fail
        lib_info['pass_count'] = lib_pass
        lib_info['fail_count'] = lib_fail
        report['libraries'][elf.name] = lib_info

        # Print per-library results
        status = "CLEAN" if lib_fail == 0 else f"{lib_fail} ISSUES"
        marker = " " if lib_fail == 0 else "!"
        print(f"\n{marker} {elf.name} [{status}]  (GOT: {elf.global_got_count} global, {elf.local_gotno} local)")
        for name, passed, msg in results:
            icon = "+" if passed else "X"
            print(f"    [{icon}] {name}: {msg}")

    # Dependency graph
    print(f"\n{'='*70}")
    print("Dependency Graph:")
    dep_graph = build_dep_graph(elfs)
    report['dependency_graph'] = dep_graph

    # Find unresolved — check both raw NEEDED and resolved names
    for name, info in dep_graph.items():
        for i, dep in enumerate(info['needed']):
            resolved = info['resolved_needed'][i]
            # Unresolved if resolved name not in graph and not a system lib
            if resolved not in dep_graph and dep not in IRIX_SYSTEM_LIBS:
                report['unresolved_deps'].append({
                    'library': name,
                    'missing': dep,
                })

    # Print dep graph sorted by depth
    by_depth = sorted(dep_graph.items(), key=lambda x: (-x[1]['depth'], x[0]))
    max_depth = max((info['depth'] for _, info in by_depth), default=0)
    for name, info in by_depth:
        indent = "  " * info['depth']
        needed_str = f" -> {', '.join(info['needed'][:5])}" if info['needed'] else ""
        if len(info['needed']) > 5:
            needed_str += f" (+{len(info['needed'])-5} more)"
        print(f"  depth={info['depth']:2d} {indent}{name}{needed_str}")
    print(f"  Max depth: {max_depth}")

    if report['unresolved_deps']:
        print(f"\nUnresolved dependencies:")
        for ud in report['unresolved_deps']:
            print(f"  {ud['library']} needs {ud['missing']}")

    # Risk ranking
    print(f"\n{'='*70}")
    print("Risk Ranking (by global GOT count):")
    ranked = sorted(elfs, key=lambda e: e.global_got_count, reverse=True)
    for i, elf in enumerate(ranked[:20]):
        gc = elf.global_got_count
        limit = THRESHOLDS['global_got_limit']
        flag = " OVER LIMIT!" if gc >= limit else (" DANGER" if gc > limit * 0.8 else "")
        print(f"  {i+1:3d}. {elf.name:50s}  global_got={gc:6d}  local_gotno={elf.local_gotno:6d}{flag}")
        report['risk_ranking'].append({
            'rank': i + 1, 'library': elf.name,
            'global_got': gc, 'local_gotno': elf.local_gotno,
            'over_limit': gc >= limit,
        })

    # Summary
    report['summary'] = {
        'pass': total_pass,
        'fail': total_fail,
        'total_checks': total_pass + total_fail,
        'libraries_analyzed': len(elfs),
        'max_depth': max_depth,
        'unresolved_count': len(report['unresolved_deps']),
        'over_got_limit': sum(1 for e in elfs if e.global_got_count >= THRESHOLDS['global_got_limit']),
    }

    print(f"\n{'='*70}")
    print(f"Summary: {total_pass} checks passed, {total_fail} failed across {len(elfs)} libraries")
    print(f"  Libraries over GOT limit ({THRESHOLDS['global_got_limit']}): {report['summary']['over_got_limit']}")
    print(f"  Unresolved deps: {len(report['unresolved_deps'])}")
    print(f"  Max dep depth: {max_depth}")

    # Save report
    RESULTS_DIR.mkdir(parents=True, exist_ok=True)
    # Clean bundle name for filename
    clean_name = bundle_name.replace('/', '_').replace(' ', '_')
    report_path = RESULTS_DIR / f"{clean_name}-rld-analysis.json"
    with open(report_path, 'w') as f:
        json.dump(report, f, indent=2, default=str)
    print(f"\nReport saved: {report_path}")

    return 1 if total_fail > 0 else 0


# ============================================================
# got-inspect — Deep GOT analysis
# ============================================================

def cmd_got_inspect(args):
    """Deep GOT analysis for a specific .so file."""
    path = Path(args.path)
    if not path.exists():
        print(f"ERROR: {path} does not exist", file=sys.stderr)
        return 1

    elf = MipsElf(path)
    if not elf.valid:
        print(f"ERROR: {elf.error}", file=sys.stderr)
        return 1

    print(f"GOT Inspection: {path.name}")
    print(f"{'='*70}")

    # Basic info
    print(f"\nELF Type:        {'shared library' if elf.is_shared else 'executable'}")
    print(f"LOAD segments:   {len(elf.load_segments)}")
    for i, seg in enumerate(elf.load_segments):
        flags = ""
        if seg['flags'] & 4:
            flags += "R"
        if seg['flags'] & 2:
            flags += "W"
        if seg['flags'] & 1:
            flags += "X"
        print(f"  [{i}] vaddr=0x{seg['vaddr']:08x}  filesz=0x{seg['filesz']:08x}  memsz=0x{seg['memsz']:08x}  {flags}")

    print(f"\nGOT Structure:")
    print(f"  .got section:   offset=0x{elf.got_offset:08x}  addr=0x{elf.got_addr:08x}  size=0x{elf.got_size:08x}")
    print(f"  LOCAL_GOTNO:    {elf.local_gotno}")
    print(f"  GOTSYM:         {elf.gotsym}")
    print(f"  SYMTABNO:       {elf.symtabno}")
    print(f"  Global GOT:     {elf.global_got_count}")
    print(f"  GOT entries:    {elf.got_size // 4}")

    if elf.stubs_size:
        num_stubs = elf.stubs_size // 16
        print(f"\n.MIPS.stubs:")
        print(f"  addr=0x{elf.stubs_addr:08x}  size=0x{elf.stubs_size:08x}  stubs={num_stubs}")

    # Threshold check
    limit = THRESHOLDS['global_got_limit']
    gc = elf.global_got_count
    print(f"\nThreshold Analysis:")
    if gc >= limit:
        print(f"  OVER LIMIT: {gc} global GOT entries >= {limit}")
        print(f"  rld CANNOT resolve all symbols — expect unresolved/SIGBUS")
        overshoot = gc - limit
        print(f"  Over by {overshoot} entries ({overshoot * 100 / limit:.1f}%)")
    elif gc > limit * 0.8:
        print(f"  WARNING: {gc} global GOT entries approaching limit of {limit}")
    else:
        print(f"  OK: {gc} global GOT entries, well within {limit} limit")

    multi_got = THRESHOLDS['multi_got_trigger']
    total_got = elf.got_size // 4
    if total_got > multi_got:
        print(f"  MULTI-GOT: total {total_got} entries > {multi_got} trigger")

    # Read and analyze GOT entries
    print(f"\nGOT Entry Analysis:")
    got_entries = elf.read_got_entries()
    if not got_entries:
        print("  No GOT entries to analyze")
        return 0

    zero_count = 0
    ffff_count = 0
    local_entries = got_entries[:elf.local_gotno]
    global_entries = got_entries[elf.local_gotno:elf.local_gotno + elf.global_got_count]

    for v in local_entries:
        if v == 0:
            zero_count += 1
        elif v == 0xFFFFFFFF:
            ffff_count += 1

    print(f"  Local GOT ({elf.local_gotno} entries):")
    print(f"    Zero entries:     {zero_count}")
    print(f"    0xFFFFFFFF:       {ffff_count}")
    if ffff_count > 0:
        print(f"    WARNING: 0xFFFFFFFF in local GOT — possible corruption")

    # Analyze global GOT entries
    global_zero = 0
    global_ffff = 0
    global_stub_match = 0
    global_stub_mismatch = 0

    for i, v in enumerate(global_entries):
        if v == 0:
            global_zero += 1
        elif v == 0xFFFFFFFF:
            global_ffff += 1
        elif elf.stubs_addr > 0:
            expected = elf.stubs_addr + i * 16
            if v == expected:
                global_stub_match += 1
            else:
                global_stub_mismatch += 1

    print(f"\n  Global GOT ({len(global_entries)} entries):")
    print(f"    Zero entries:     {global_zero}")
    print(f"    0xFFFFFFFF:       {global_ffff}")
    if elf.stubs_addr > 0:
        print(f"    Stub-aligned:     {global_stub_match}")
        print(f"    Stub-misaligned:  {global_stub_mismatch}")
    if global_ffff > 0:
        print(f"    WARNING: 0xFFFFFFFF in global GOT — unresolved or corrupt")
    if global_zero > 0:
        print(f"    WARNING: zero entries in global GOT — unresolved symbols")

    # Check for symbols below GOTSYM that are GLOBAL DEFAULT (invisible to rld)
    print(f"\nSymbol Analysis:")
    dynsym = elf.read_dynsym_entries()
    if dynsym:
        # STB_GLOBAL=1, STB_WEAK=2; SHN_UNDEF=0
        pre_gotsym_globals = []
        for sym in dynsym:
            if sym['index'] < elf.gotsym and sym['bind'] in (1, 2) and sym['shndx'] != 0:
                pre_gotsym_globals.append(sym)

        if pre_gotsym_globals:
            print(f"  GLOBAL/WEAK symbols BELOW GOTSYM ({len(pre_gotsym_globals)}):")
            print(f"  These are invisible to rld's GOT resolution!")
            for sym in pre_gotsym_globals[:20]:
                print(f"    [{sym['index']:5d}] {sym['name']:40s}  val=0x{sym['value']:08x}  bind={sym['bind']}")
            if len(pre_gotsym_globals) > 20:
                print(f"    ... and {len(pre_gotsym_globals) - 20} more")
        else:
            print(f"  No GLOBAL/WEAK symbols below GOTSYM — good")

        # Count undefined symbols in global GOT range
        undef_in_global = [s for s in dynsym if s['index'] >= elf.gotsym and s['shndx'] == 0]
        print(f"  Undefined symbols in global GOT range: {len(undef_in_global)}")
        if undef_in_global and args.verbose:
            for sym in undef_in_global[:30]:
                print(f"    [{sym['index']:5d}] {sym['name']}")
            if len(undef_in_global) > 30:
                print(f"    ... and {len(undef_in_global) - 30} more")

    # R_MIPS_REL32 analysis
    print(f"\nRelocation Analysis:")
    rels = elf.read_rel_entries()
    if rels:
        r_mips_rel32 = [(off, sym, t) for off, sym, t in rels if t == 3]
        anon = [r for r in r_mips_rel32 if r[1] == 0]
        named = [r for r in r_mips_rel32 if r[1] != 0]
        print(f"  Total .rel.dyn entries: {len(rels)}")
        print(f"  R_MIPS_REL32: {len(r_mips_rel32)} ({len(anon)} anonymous, {len(named)} named)")
        if anon:
            print(f"  WARNING: {len(anon)} anonymous R_MIPS_REL32 — fix-anon-relocs may not have run")
    else:
        print(f"  No .rel.dyn section")

    # NEEDED list
    needed = elf.needed_sonames()
    print(f"\nNEEDED ({len(needed)}):")
    for dep in needed:
        sys_flag = " [system]" if dep in IRIX_SYSTEM_LIBS else ""
        print(f"  {dep}{sys_flag}")

    # Save report
    report = {
        'library': str(path),
        'global_got': elf.global_got_count,
        'local_gotno': elf.local_gotno,
        'symtabno': elf.symtabno,
        'gotsym': elf.gotsym,
        'got_entries_total': len(got_entries),
        'local_got_zeros': zero_count,
        'local_got_ffff': ffff_count,
        'global_got_zeros': global_zero,
        'global_got_ffff': global_ffff,
        'over_limit': gc >= limit,
        'needed': needed,
        'pre_gotsym_globals': len(pre_gotsym_globals) if dynsym else 0,
    }

    if args.output:
        out_path = Path(args.output)
        with open(out_path, 'w') as f:
            json.dump(report, f, indent=2)
        print(f"\nReport saved: {out_path}")

    return 0


# ============================================================
# test-loading — Binary-search which library crashes rld
# ============================================================

def cmd_test_loading(args):
    """Binary-search which library causes rld to fail when loaded."""
    bundle_path = Path(args.path)
    if not bundle_path.exists():
        print(f"ERROR: {bundle_path} does not exist", file=sys.stderr)
        return 1

    libs = find_bundle_libs(bundle_path)
    if not libs:
        print(f"ERROR: No .so files found in {bundle_path}", file=sys.stderr)
        return 1

    print(f"Test-loading: {len(libs)} libraries from {bundle_path}")
    print(f"{'='*70}")

    # Parse all to get metadata
    elfs = []
    for lib_path in libs:
        elf = MipsElf(lib_path)
        if elf.valid and elf.is_shared:
            elfs.append(elf)

    # Build load orders to test
    sonames = [e.name for e in elfs]
    print(f"Libraries to test: {len(sonames)}")

    # Check harness source exists
    if not HARNESS_SRC.exists():
        print(f"ERROR: {HARNESS_SRC} not found", file=sys.stderr)
        return 1

    # Cross-compile the harness
    print(f"\nCross-compiling harness...")
    harness_bin = Path(tempfile.mktemp(suffix='_harness_dlopen'))

    # Find irix-cc
    irix_cc = "irix-cc"
    for candidate in ["/opt/sgug-staging/usr/sgug/bin/irix-cc",
                      str(PROJECT_ROOT / "cross" / "bin" / "irix-cc")]:
        if Path(candidate).exists():
            irix_cc = candidate
            break

    cc_cmd = [
        irix_cc, "-o", str(harness_bin),
        str(HARNESS_SRC), "-ldl",
    ]
    result = subprocess.run(cc_cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"ERROR: Cross-compile failed:\n{result.stderr}", file=sys.stderr)
        return 1
    print(f"  Built: {harness_bin}")

    # Find the lib directory in the bundle (for IRIX deployment)
    bundle_lib_dir = None
    for d in ['lib', 'lib32', 'usr/sgug/lib32']:
        if (bundle_path / d).is_dir():
            bundle_lib_dir = d
            break
    if not bundle_lib_dir:
        # Libs might be at the root
        bundle_lib_dir = "."

    # Deploy to IRIX via MCP tools
    print(f"\nDeploying to IRIX...")

    # We use subprocess to call the irix MCP tools indirectly through
    # the existing test infrastructure. For direct use, the caller
    # should use the MCP tools directly.
    #
    # For now, we output the test plan and let the caller execute it.

    print(f"\n{'='*70}")
    print("TEST PLAN")
    print(f"{'='*70}")
    print(f"\nHarness binary: {harness_bin}")
    print(f"Bundle lib dir: {bundle_path / bundle_lib_dir}")
    print(f"\nTo run on IRIX:")
    print(f"  1. Copy harness: irix_copy_to {harness_bin} /tmp/harness_dlopen")
    print(f"  2. Deploy bundle to IRIX (if not already there)")
    print(f"  3. Run: echo '<sonames>' | /tmp/harness_dlopen /path/to/libs")

    # Generate test sequences
    print(f"\n--- Load Order: Alphabetical ---")
    alpha_order = sorted(sonames)
    print(f"  {len(alpha_order)} libraries")

    print(f"\n--- Load Order: By Dependency Depth (leaves first) ---")
    dep_graph = build_dep_graph(elfs)
    depth_order = sorted(sonames, key=lambda n: dep_graph.get(n, {}).get('depth', 0))
    print(f"  {len(depth_order)} libraries")

    print(f"\n--- Load Order: By Global GOT (smallest first) ---")
    got_order = sorted(elfs, key=lambda e: e.global_got_count)
    got_names = [e.name for e in got_order]
    print(f"  {len(got_names)} libraries")

    print(f"\n--- Load Order: By Global GOT (largest first) ---")
    got_rev_names = [e.name for e in reversed(got_order)]
    print(f"  {len(got_rev_names)} libraries")

    # Binary search function
    def binary_search_plan(lib_list, order_name):
        """Generate binary search test commands."""
        print(f"\n{'='*70}")
        print(f"Binary Search Plan: {order_name}")
        print(f"{'='*70}")

        n = len(lib_list)
        step = 0
        ranges = [(0, n)]

        while ranges:
            new_ranges = []
            for lo, hi in ranges:
                if lo >= hi:
                    continue
                mid = (lo + hi) // 2
                step += 1
                subset = lib_list[lo:hi]
                print(f"\n  Step {step}: Test libs[{lo}:{hi}] ({hi-lo} libs)")
                print(f"    echo '{chr(10).join(subset)}' | /tmp/harness_dlopen /path/to/libs")

                if hi - lo > 1:
                    print(f"    If FAIL: split into [{lo}:{mid}] and [{mid}:{hi}]")
                    new_ranges.append((lo, mid))
                    new_ranges.append((mid, hi))
                else:
                    print(f"    Single library: {subset[0]}")
            ranges = new_ranges
            if step > 20:
                print(f"  (truncated after 20 steps)")
                break

    binary_search_plan(alpha_order, "Alphabetical")

    # Save the soname lists for scripted execution
    report = {
        'bundle': str(bundle_path),
        'harness_binary': str(harness_bin),
        'library_count': len(sonames),
        'orders': {
            'alphabetical': alpha_order,
            'by_depth': depth_order,
            'by_got_asc': got_names,
            'by_got_desc': got_rev_names,
        },
    }

    RESULTS_DIR.mkdir(parents=True, exist_ok=True)
    clean_name = bundle_path.name.replace('/', '_').replace(' ', '_')
    report_path = RESULTS_DIR / f"{clean_name}-load-test-plan.json"
    with open(report_path, 'w') as f:
        json.dump(report, f, indent=2)
    print(f"\nLoad test plan saved: {report_path}")

    return 0


# ============================================================
# Main
# ============================================================

def main():
    parser = argparse.ArgumentParser(
        description="Reusable rld bundle analysis harness",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    sub = parser.add_subparsers(dest='command', help='Subcommand')

    # analyze
    p_analyze = sub.add_parser('analyze',
        help='Static ELF analysis on bundle or .so file')
    p_analyze.add_argument('path', help='Path to bundle directory or .so file')

    # got-inspect
    p_got = sub.add_parser('got-inspect',
        help='Deep GOT analysis for a specific .so file')
    p_got.add_argument('path', help='Path to .so file')
    p_got.add_argument('-v', '--verbose', action='store_true',
        help='Show detailed symbol listings')
    p_got.add_argument('-o', '--output', help='Save JSON report to file')

    # test-loading
    p_load = sub.add_parser('test-loading',
        help='Binary-search which library crashes rld')
    p_load.add_argument('path', help='Path to bundle directory')

    args = parser.parse_args()

    if args.command == 'analyze':
        return cmd_analyze(args)
    elif args.command == 'got-inspect':
        return cmd_got_inspect(args)
    elif args.command == 'test-loading':
        return cmd_test_loading(args)
    else:
        parser.print_help()
        return 0


if __name__ == '__main__':
    sys.exit(main())
