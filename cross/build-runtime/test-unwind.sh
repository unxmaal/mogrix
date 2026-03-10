#!/bin/bash
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
GCC="$SCRIPT_DIR/gcc-9.5.0"
CFG="$SCRIPT_DIR/gcc-config"
CLANG="/opt/cross/bin/clang"
SYSROOT="/opt/irix-sysroot"
STAGING="/opt/sgug-staging/usr/sgug"
OUTDIR="$SCRIPT_DIR/build/test"
mkdir -p "$OUTDIR"

FLAGS="--target=mips-sgi-irix6.5 --sysroot=$SYSROOT -mabi=n32 -march=mips3 -mxgot"
FLAGS="$FLAGS -O2 -fPIC -fvisibility=hidden -std=gnu99 -D__ELF__ -D__c99"
FLAGS="$FLAGS -D_LONGLONG=1 -D_SGI_SOURCE -D_SGI_MP_SOURCE -D_SGI_REENTRANT_FUNCTIONS"
FLAGS="$FLAGS -Dsgi=1 -D__sgi=1 -D_COMPILER_VERSION=730 -D_LANGUAGE_C=1"
FLAGS="$FLAGS -include $SYSROOT/usr/include/sgidefs.h"
FLAGS="$FLAGS -include $CFG/stdarg_shim.h"
FLAGS="$FLAGS -I$CFG -I$GCC/libgcc -I$GCC/gcc -I$GCC/include -I$GCC/libgcc/config"
FLAGS="$FLAGS -Wno-attributes -Wno-builtin-macro-redefined"
# Prevent clang's unwind.h from conflicting with GCC's unwind types
FLAGS="$FLAGS -D__CLANG_UNWIND_H"

for src in unwind-dw2.c unwind-dw2-fde.c unwind-c.c; do
    echo "=== $src ==="
    base="${src%.c}"
    if $CLANG $FLAGS -c "$GCC/libgcc/$src" -o "$OUTDIR/$base.o" 2>"$OUTDIR/$base.err"; then
        echo "OK"
    else
        echo "FAILED"
        head -20 "$OUTDIR/$base.err"
    fi
done
