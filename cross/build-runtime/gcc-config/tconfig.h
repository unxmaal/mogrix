/* Minimal tconfig.h for cross-compiling GCC unwind with clang for IRIX n32 */
#ifndef GCC_TCONFIG_H
#define GCC_TCONFIG_H

/* We're building libgcc, not using it */
#ifndef USED_FOR_TARGET
#define USED_FOR_TARGET
#endif

/* IRIX uses DWARF2 exceptions */
#define DWARF2_UNWIND_INFO 1

/* POSIX threads for thread-safe unwind */
#define SUPPORTS_WEAK 1
#define GTHREAD_USE_WEAK 1

/* Target: MIPS n32 */
#define __GCC_HAVE_DWARF2_CFI_ASM 1

/* MIPS register count: 188 pseudo-registers in GCC's MIPS backend.
   This defines the size of the DWARF register save arrays in unwind. */
#define FIRST_PSEUDO_REGISTER 188
#define __LIBGCC_DWARF_FRAME_REGISTERS__ FIRST_PSEUDO_REGISTER

/* Default: DWARF register numbers map directly to unwind columns */
#define DWARF_REG_TO_UNWIND_COLUMN(REGNO) (REGNO)

/* Stack grows downward on MIPS */
#define __LIBGCC_STACK_GROWS_DOWNWARD__ 1

/* MIPS return address is in $31 (ra) = DWARF register 31 */
#define DWARF_FRAME_RETURN_COLUMN 31
#define __LIBGCC_DWARF_FRAME_RETURN_COLUMN__ 31

/* DWARF CIE data alignment factor */
#define __LIBGCC_DWARF_CIE_DATA_ALIGNMENT__ -4

/* EH return data registers for MIPS (from gcc/config/mips/mips.h) */
#define __LIBGCC_EH_TABLES_CAN_BE_READ_ONLY__ 0
#define EH_RETURN_DATA_REGNO(N) ((N) < 4 ? (N) + 4 : INVALID_REGNUM)
#define INVALID_REGNUM (~(unsigned int)0)

/* Word size */
#define __LIBGCC_UNITS_PER_WORD__ 4

#endif /* GCC_TCONFIG_H */
