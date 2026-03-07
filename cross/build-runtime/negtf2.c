// negtf2.c - 128-bit float negation (missing from compiler-rt, needed by libstdc++)
// Follows the same pattern as compiler-rt's negdf2.c / negsf2.c

#define QUAD_PRECISION
#include "fp_lib.h"

COMPILER_RT_ABI fp_t __negtf2(fp_t a) { return fromRep(toRep(a) ^ signBit); }
