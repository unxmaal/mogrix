/* IRIX-compatible shared library linker script
 * Discards .MIPS.abiflags section which IRIX doesn't understand
 */
SECTIONS
{
    /DISCARD/ : {
        *(.MIPS.abiflags)
    }
}
