"""
Test manifest for IRIX rld test suite.

Each test is a dict with:
  id:       Unique test ID (e.g. "A04")
  name:     Short descriptive name
  priority: 1-4 (1 = most critical)
  sources:  List of source files relative to src/
  libs:     List of shared library specs: {"name": "libfoo.so", "sources": [...], "deps": [...]}
  link:     Extra linker flags for the main binary
  env:      Extra env vars for IRIX execution
  expect:   "exit0", "exit1", "crash", "rld_error", or {"exit": N}
  static_checks: List of static ELF checks to run (optional)
  description: What this test proves
"""

TESTS = [
    # === Priority 1: Most likely to expose MiniBrowser-class bugs ===

    {
        "id": "A04",
        "name": "got_entry_limit",
        "priority": 1,
        "sources": ["a04_got_limit_main.c"],
        "libs": [{"name": "liba04_biggot.so", "sources": ["a04_got_limit_lib.c"]}],
        "expect": "exit0",
        "static_checks": ["check_got_global_count"],
        "description": "Global GOT (SYMTABNO-GOTSYM) must be < ~4370 per .so",
    },
    {
        "id": "A06",
        "name": "gotsym_threshold",
        "priority": 1,
        "sources": ["a06_gotsym_main.c"],
        "libs": [{"name": "liba06_gotsym.so", "sources": ["a06_gotsym_lib.c"]}],
        "expect": "exit0",
        "static_checks": ["check_got_global_count"],
        "description": "Verify GOTSYM partitioning is correct in our toolchain output",
    },
    {
        "id": "C16",
        "name": "reencounter_got",
        "priority": 1,
        "sources": ["c16_reencounter_main.c"],
        "libs": [
            {"name": "libc16a.so", "sources": ["c16_reencounter_liba.c"]},
            {"name": "libc16b.so", "sources": ["c16_reencounter_libb.c"], "deps": ["libc16a.so"]},
        ],
        "link": ["-ldl"],
        "expect": "exit0",
        "description": "dlopen then NEEDED re-encounter: does rld re-zero GOT?",
    },
    {
        "id": "D18",
        "name": "constructor_exec",
        "priority": 1,
        "sources": ["d18_ctor_main.c"],
        "libs": [
            {"name": "libd18a.so", "sources": ["d18_ctor_liba.c"]},
            {"name": "libd18b.so", "sources": ["d18_ctor_libb.c"]},
        ],
        "expect": "exit0",
        "description": "C++ constructors in .so run via .ctors/DT_INIT",
    },
    {
        "id": "D21",
        "name": "funcptr_callback",
        "priority": 1,
        "sources": ["d21_funcptr_main.c"],
        "libs": [{"name": "libd21_cb.so", "sources": ["d21_funcptr_lib.c"]}],
        "expect": "exit0",
        "description": "GType pattern: store func ptr from exe, call via lib (tests $t9/$gp)",
    },

    # === Priority 2: Known production issues ===

    {
        "id": "A01",
        "name": "two_segment_layout",
        "priority": 2,
        "sources": ["a01_segments_main.c"],
        "libs": [{"name": "liba01_seg.so", "sources": ["a01_segments_lib.c"]}],
        "expect": "exit0",
        "static_checks": ["check_load_segments"],
        "description": ".so must have exactly 2 LOAD segments (RE+RW)",
    },
    {
        "id": "A02",
        "name": "no_verneed",
        "priority": 2,
        "sources": ["a02_verneed_main.c"],
        "libs": [{"name": "liba02_ver.so", "sources": ["a02_verneed_lib.c"]}],
        "expect": "exit0",
        "static_checks": ["check_no_verneed"],
        "description": "strip-verneed must remove VERNEED/VERSYM tags",
    },
    {
        "id": "A03",
        "name": "no_init_array",
        "priority": 2,
        "sources": ["a03_initarray_main.c"],
        "libs": [{"name": "liba03_init.so", "sources": ["a03_initarray_lib.c"]}],
        "expect": "exit0",
        "static_checks": ["check_no_init_array"],
        "description": "Must use .ctors+DT_INIT, not .init_array (rld ignores .init_array)",
    },
    {
        "id": "B08",
        "name": "no_exe_preemption",
        "priority": 2,
        "sources": ["b08_preempt_main.c"],
        "libs": [{"name": "libb08_preempt.so", "sources": ["b08_preempt_lib.c"]}],
        "expect": "exit0",
        "description": "IRIX rld does NOT preempt exe symbols into .so (unlike Linux)",
    },
    {
        "id": "B09",
        "name": "rldn32_list_preload",
        "priority": 2,
        "sources": ["b09_preload_main.c"],
        "libs": [
            {"name": "libb09_base.so", "sources": ["b09_preload_base.c"]},
            {"name": "libb09_override.so", "sources": ["b09_preload_override.c"]},
        ],
        "env": {"_RLDN32_LIST": "libb09_override.so:DEFAULT"},
        "expect": "exit0",
        "description": "_RLDN32_LIST preload override works (IRIX LD_PRELOAD)",
    },
    {
        "id": "D22",
        "name": "bsearch_nmemb0",
        "priority": 2,
        "sources": ["d22_bsearch_main.c"],
        "libs": [{"name": "libd22_bsearch.so", "sources": ["d22_bsearch_lib.c"]}],
        "expect": "exit0",
        "description": "bsearch(nmemb=0) must not crash (IRIX libc bug, needs compat override)",
    },

    # === Priority 3: Important sanity checks ===

    {
        "id": "B07",
        "name": "lazy_binding",
        "priority": 3,
        "sources": ["b07_lazy_main.c"],
        "libs": [{"name": "libb07_lazy.so", "sources": ["b07_lazy_lib.c"]}],
        "expect": "exit0",
        "description": "Basic cross-library function call via lazy binding works",
    },
    {
        "id": "C12",
        "name": "dlopen_basic",
        "priority": 3,
        "sources": ["c12_dlopen_main.c"],
        "libs": [{"name": "libc12_plugin.so", "sources": ["c12_dlopen_lib.c"]}],
        "link": ["-ldl"],
        "expect": "exit0",
        "description": "dlopen/dlsym/dlclose basic operation",
    },
    {
        "id": "C13",
        "name": "dlopen_pthread",
        "priority": 3,
        "sources": ["c13_pthread_main.c"],
        "libs": [{"name": "libc13_pthread.so", "sources": ["c13_pthread_lib.c"], "link": ["-lpthread"]}],
        "expect": "exit0",
        "description": "pthread via NEEDED (not dlopen) works correctly",
    },
    {
        "id": "D19",
        "name": "dlmalloc_crossheap",
        "priority": 3,
        "sources": ["d19_crossheap_main.c"],
        "libs": [{"name": "libd19_heap.so", "sources": ["d19_crossheap_lib.c"]}],
        "expect": "exit0",
        "description": "malloc in exe, free in .so works (single allocator via dlmalloc)",
    },
    {
        "id": "D20",
        "name": "safe_memcmp",
        "priority": 3,
        "sources": ["d20_memcmp_main.c"],
        "libs": [],
        "expect": "exit0",
        "description": "memcmp near page boundary doesn't SIGSEGV",
    },
    {
        "id": "A05",
        "name": "rld_obj_head",
        "priority": 3,
        "sources": ["a05_rldobj_main.c"],
        "libs": [{"name": "liba05_rld.so", "sources": ["a05_rldobj_lib.c"]}],
        "expect": "exit0",
        "static_checks": ["check_rld_obj_head"],
        "description": "__rld_obj_head exported in exe's .dynsym",
    },

    # === Priority 4: Edge cases ===

    {
        "id": "B10",
        "name": "strict_needed",
        "priority": 4,
        "sources": ["b10_needed_main.c"],
        "libs": [
            {"name": "libb10_a.so", "sources": ["b10_needed_liba.c"]},
            {"name": "libb10_b.so", "sources": ["b10_needed_libb.c"], "deps": ["libb10_a.so"]},
        ],
        "expect": "exit0",
        "description": "rld only searches NEEDED chain, not all loaded libs",
    },
    {
        "id": "B11",
        "name": "dt_bind_now",
        "priority": 4,
        "sources": ["b11_bindnow_main.c"],
        "libs": [{"name": "libb11_bind.so", "sources": ["b11_bindnow_lib.c"]}],
        "link": ["-Wl,-z,now"],
        "expect": "exit0",
        "description": "Does rld honor DT_BIND_NOW for MIPS GOT? (exploratory)",
    },
    {
        "id": "C14",
        "name": "circular_needed",
        "priority": 4,
        "sources": ["c14_circular_main.c"],
        "libs": [
            {"name": "libc14_x.so", "sources": ["c14_circular_libx.c"]},
            {"name": "libc14_y.so", "sources": ["c14_circular_liby.c"]},
        ],
        "expect": "exit0",
        "description": "Circular NEEDED deps — does rld handle or crash?",
    },
    {
        "id": "C15",
        "name": "deep_deps",
        "priority": 4,
        "sources": ["c15_deep_main.c"],
        "libs": [{"name": f"libc15_d{i}.so", "sources": [f"c15_deep_lib{i}.c"],
                  **({"deps": [f"libc15_d{i-1}.so"]} if i > 0 else {})}
                 for i in range(10)],
        "expect": "exit0",
        "description": "10-deep transitive dependency chain stress test",
    },
    {
        "id": "C17",
        "name": "ld_libraryn32_path",
        "priority": 4,
        "sources": ["c17_ldpath_main.c"],
        "libs": [{"name": "libc17_path.so", "sources": ["c17_ldpath_lib.c"]}],
        "expect": "exit0",
        "description": "LD_LIBRARYN32_PATH resolves system libs correctly",
    },
]


def get_tests(priority=None):
    """Return tests, optionally filtered by max priority level."""
    if priority is None:
        return TESTS
    return [t for t in TESTS if t["priority"] <= priority]


def get_test(test_id):
    """Return a single test by ID."""
    for t in TESTS:
        if t["id"] == test_id:
            return t
    return None


def get_all_sources():
    """Return set of all source files needed."""
    sources = set()
    for t in TESTS:
        sources.update(t["sources"])
        for lib in t.get("libs", []):
            sources.update(lib["sources"])
    return sources
