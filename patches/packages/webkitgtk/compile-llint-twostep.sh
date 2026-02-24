#!/bin/sh
# Three-step compilation wrapper for LowLevelInterpreter.cpp
# Workaround: clang 16/18 MIPS backend uses 56GB+ RAM compiling this
# file due to a bug in cc1's inline assembly handling for large TUs.
# Split into: clang -emit-llvm → llc -filetype=asm → GNU as
#
# Usage: Set as RULE_LAUNCH_COMPILE on the LowLevelInterpreterLib cmake target.
# cmake invokes: <launcher> <compiler> <flags...>
# So $1 is the compiler, $2... are the flags.

set -e

# Handle ccache: cmake invokes <launcher> [ccache] <compiler> <flags...>
CCACHE=""
case "$1" in
    */ccache) CCACHE="$1"; shift ;;
esac

COMPILER="$1"
shift

# Collect args, find -o and -c flags
OUTPUT=""
BC_OUTPUT=""
EMIT_ARGS=""
IS_COMPILE=0
PREV_WAS_O=0

for arg in "$@"; do
    if [ "$PREV_WAS_O" = "1" ]; then
        OUTPUT="$arg"
        BC_OUTPUT="${arg%.o}.bc"
        EMIT_ARGS="$EMIT_ARGS -o $BC_OUTPUT"
        PREV_WAS_O=0
        continue
    fi
    case "$arg" in
        -o)
            PREV_WAS_O=1
            ;;
        -c)
            IS_COMPILE=1
            # Replace -c with -emit-llvm -c for step 1
            EMIT_ARGS="$EMIT_ARGS -emit-llvm -c"
            ;;
        *)
            EMIT_ARGS="$EMIT_ARGS $arg"
            ;;
    esac
done

if [ "$IS_COMPILE" = "0" ] || [ -z "$OUTPUT" ]; then
    # Not a compile command, pass through
    exec $CCACHE "$COMPILER" "$@"
fi

echo "[llint-twostep] Step 1: Frontend → LLVM bitcode" >&2

# Step 1: C++ frontend → LLVM bitcode (126MB RAM, 5 seconds)
# Use ccache if available (helps on rebuilds)
$CCACHE $COMPILER $EMIT_ARGS 2>&1 | grep -v '^warning: register names' >&2 || true

if [ ! -f "$BC_OUTPUT" ]; then
    echo "[llint-twostep] FATAL: step 1 failed — $BC_OUTPUT not produced" >&2
    exit 1
fi

echo "[llint-twostep] Step 2: LLVM bitcode → MIPS N32 assembly (via llc)" >&2

# Step 2: LLVM bitcode → MIPS assembly text (42MB RAM, 3 seconds)
# Must use -target-abi n32 for correct N32 code (32-bit addresses, 64-bit regs).
# Without it, mips64 triple generates 64-bit address relocations (R_MIPS_HIGHER).
# xgot: enables 32-bit GOT pairs for function calls (CALL_HI16/CALL_LO16).
LLC_DIR=$(dirname "$COMPILER")
LLC="$LLC_DIR/llc"
if [ ! -x "$LLC" ]; then
    LLC="/opt/cross/bin/llc"
fi

ASM_OUTPUT="${OUTPUT%.o}.s"

$LLC -mtriple=mips64-sgi-irix6.5 \
     -target-abi n32 \
     -mattr=+xgot \
     -mcpu=mips4 \
     -relocation-model=pic \
     -O0 \
     -filetype=asm \
     "$BC_OUTPUT" -o "$ASM_OUTPUT" 2>&1 | grep -v '^warning:' >&2

if [ ! -f "$ASM_OUTPUT" ]; then
    echo "[llint-twostep] FATAL: step 2 failed — $ASM_OUTPUT not produced" >&2
    exit 1
fi

echo "[llint-twostep] Step 2.5: Post-process assembly" >&2

# Step 2.5: Fix assembly for GNU as + lld compatibility.
#
# Strip problematic directives:
# - .size: llc emits non-absolute .size expressions (LLVM MIPS bug). Safe to
#     remove — IRIX rld uses .MIPS.options, not ELF .size.
# - .file: GNU as places FILE symbol after SECTION symbols, corrupting sh_info.
#     lld rejects as "invalid binding: 0".
# - $tmpN: llc generates temp labels as LOCAL symbols interleaved with GLOBALs,
#     violating ELF symbol ordering. Only definitions, never referenced.
#
# Convert GOT access to xgot (32-bit pairs):
# - %got(SYM)($gp) and %got_disp(SYM)($gp) → lui+addu+lw via %got_hi/%got_lo.
#   libjavascriptcoregtk's GOT exceeds 64KB, so 16-bit GOT entries overflow.
#   Clang natively uses GOT_PAGE/GOT_OFST pairs; llc uses GOT16/GOT_DISP which
#   are 16-bit and overflow. This converts them to 32-bit GOT_HI16/GOT_LO16.

# Strip directives and labels
sed -i '/^[[:space:]]*\.size/d; /^[[:space:]]*\.file/d; /^\$tmp[0-9]*:$/d' "$ASM_OUTPUT"

# Convert %got(SYM)($gp) → xgot pair using $at ($1) as temp
perl -i -pe '
    s/^\t(lw)\s+(\$\d+),\s*%got\(([^)]+)\)\(\$gp\)/\tlui \$1, %got_hi($3)\n\taddu \$1, \$1, \$gp\n\t$1 $2, %got_lo($3)(\$1)/g;
    s/^\t(lw)\s+(\$\d+),\s*%got_disp\(([^)]+)\)\(\$gp\)/\tlui \$1, %got_hi($3)\n\taddu \$1, \$1, \$gp\n\t$1 $2, %got_lo($3)(\$1)/g;
' "$ASM_OUTPUT"

echo "[llint-twostep] Step 3: Assembly → object (via GNU as)" >&2

# Step 3: Assemble with GNU as (not clang's integrated assembler).
# clang cc1as crashes with "Size expression must be absolute" on MIPS.
# GNU as works after the post-processing above.
CROSS_AS="/opt/cross/bin/mips-sgi-irix6.5-as"
if [ ! -x "$CROSS_AS" ]; then
    echo "[llint-twostep] FATAL: $CROSS_AS not found" >&2
    exit 1
fi
$CROSS_AS -mabi=n32 -march=mips4 -mfp64 -KPIC \
     -o "$OUTPUT" "$ASM_OUTPUT" 2>&1 | grep -v '^warning:\|Warning:' >&2

if [ ! -s "$OUTPUT" ]; then
    echo "[llint-twostep] FATAL: step 3 failed — $OUTPUT not produced or empty" >&2
    exit 1
fi

rm -f "$BC_OUTPUT" "$ASM_OUTPUT"
echo "[llint-twostep] Done: $OUTPUT" >&2
