#!/bin/bash
# Analyze symbol gaps between our clang-built libstdc++/libgcc_s and SGUG-RSE versions
#
# Usage: ./analyze-symbols.sh [--libgcc]
#
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
STAGING="${MOGRIX_STAGING:-/opt/sgug-staging/usr/sgug}"
NM="/opt/cross/bin/mips-sgi-irix6.5-nm"
READELF="/opt/cross/bin/mips-sgi-irix6.5-readelf"
TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

if [ "${1:-}" = "--libgcc" ]; then
    OUR_LIB="$SCRIPT_DIR/libgcc_s.so.1"
    SGUG_LIB="$STAGING/lib32/libgcc_s.so.1"
    LABEL="libgcc_s"
else
    OUR_LIB="$SCRIPT_DIR/libstdc++.so.6"
    SGUG_LIB="$STAGING/lib32/libstdc++.so.6"
    LABEL="libstdc++"
fi

if [ ! -f "$OUR_LIB" ]; then
    echo "ERROR: Our $LABEL not found at $OUR_LIB"
    exit 1
fi
if [ ! -f "$SGUG_LIB" ]; then
    echo "ERROR: SGUG-RSE $LABEL not found at $SGUG_LIB"
    exit 1
fi

echo "=== $LABEL Symbol Gap Analysis ==="
echo ""
echo "Our build:  $OUR_LIB ($(ls -lh "$OUR_LIB" | awk '{print $5}'))"
echo "SGUG-RSE:   $SGUG_LIB ($(ls -lh "$SGUG_LIB" | awk '{print $5}'))"
echo ""

# Extract dynamic FUNC GLOBAL symbols
$READELF --dyn-syms "$OUR_LIB" 2>/dev/null \
    | awk '/FUNC.*GLOBAL/ {print $NF}' \
    | sort -u > "$TMPDIR/our_syms.txt"

$READELF --dyn-syms "$SGUG_LIB" 2>/dev/null \
    | awk '/FUNC.*GLOBAL/ {print $NF}' \
    | sort -u > "$TMPDIR/sgug_syms.txt"

our_count=$(wc -l < "$TMPDIR/our_syms.txt")
sgug_count=$(wc -l < "$TMPDIR/sgug_syms.txt")

echo "Our exported FUNC GLOBAL:   $our_count"
echo "SGUG-RSE exported FUNC GLOBAL: $sgug_count"
echo ""

# Find symbols in SGUG but not in ours (the gap)
comm -23 "$TMPDIR/sgug_syms.txt" "$TMPDIR/our_syms.txt" > "$TMPDIR/missing.txt"
# Find symbols in ours but not in SGUG (extras)
comm -13 "$TMPDIR/sgug_syms.txt" "$TMPDIR/our_syms.txt" > "$TMPDIR/extra.txt"

missing_count=$(wc -l < "$TMPDIR/missing.txt")
extra_count=$(wc -l < "$TMPDIR/extra.txt")

echo "Missing (in SGUG, not ours): $missing_count"
echo "Extra (in ours, not SGUG):   $extra_count"
echo ""

if [ "$missing_count" -gt 0 ]; then
    echo "=== Missing symbols by category ==="

    # Categorize missing symbols
    fs_count=$(grep -c "filesystem\|path\|directory_iterator\|fs_err\|_Dir\b" "$TMPDIR/missing.txt" 2>/dev/null || echo 0)
    strstream_count=$(grep -c "strstream\|strstreambuf" "$TMPDIR/missing.txt" 2>/dev/null || echo 0)
    pmr_count=$(grep -c "memory_resource\|pmr\|monotonic_buffer\|polymorphic\|synchronized_pool\|unsynchronized_pool" "$TMPDIR/missing.txt" 2>/dev/null || echo 0)
    cow_count=$(grep -c "cow\|_GLIBCXX_USE_CXX11_ABI\|basic_fstream\|basic_ifstream\|basic_ofstream\|basic_stringstream\|basic_istringstream\|basic_ostringstream" "$TMPDIR/missing.txt" 2>/dev/null || echo 0)
    aligned_new_count=$(grep -c "del_opa\|del_opant\|del_opsa\|del_opva\|del_opvant\|del_opvsa\|new_opa\|new_opant\|new_opva\|new_opvant\|aligned\|align_val" "$TMPDIR/missing.txt" 2>/dev/null || echo 0)
    hash_count=$(grep -c "hash\|_Hash" "$TMPDIR/missing.txt" 2>/dev/null || echo 0)
    limit_count=$(grep -c "numeric_limits\|__int_traits" "$TMPDIR/missing.txt" 2>/dev/null || echo 0)
    personality_count=$(grep -c "personality\|__gcc_personality" "$TMPDIR/missing.txt" 2>/dev/null || echo 0)
    clrsb_count=$(grep -c "clrsb" "$TMPDIR/missing.txt" 2>/dev/null || echo 0)

    echo "  filesystem/path:     $fs_count"
    echo "  strstream:           $strstream_count"
    echo "  pmr/memory_resource: $pmr_count"
    echo "  cow ABI (fstream/sstream): $cow_count"
    echo "  aligned new/delete:  $aligned_new_count"
    echo "  hash:                $hash_count"
    echo "  numeric_limits:      $limit_count"
    echo "  personality:         $personality_count"
    echo "  clrsb:               $clrsb_count"
    echo ""

    echo "=== All missing symbols ==="
    cat "$TMPDIR/missing.txt"
fi

if [ "$extra_count" -gt 0 ]; then
    echo ""
    echo "=== Extra symbols (in ours, not SGUG) ==="
    cat "$TMPDIR/extra.txt"
fi
