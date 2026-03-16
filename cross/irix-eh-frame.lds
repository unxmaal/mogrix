/* IRIX .eh_frame placement fix for LLD.
 *
 * .eh_frame and .gcc_except_table are renamed to .data.eh_frame and
 * .data.gcc_except_table (with SHF_WRITE) in .o files before linking.
 * This script collects them and outputs them as .eh_frame and
 * .gcc_except_table in the final binary, placed in the RW data segment.
 *
 * The renaming is needed because LLD strips SHF_WRITE from .eh_frame
 * during its special CIE/FDE merge. A differently-named section is
 * treated as regular data and respects SHF_WRITE for segment placement.
 *
 * After this, .eh_frame is in the writable segment where IRIX rld can
 * safely apply R_MIPS_REL32 relocations at load time.
 */

SECTIONS {
  .eh_frame : { KEEP (*(.data.eh_frame)) }
  .gcc_except_table : { *(.data.gcc_except_table) }
} INSERT BEFORE .ctors;
