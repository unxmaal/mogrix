/*
 * eh_frame_reg.c — .eh_frame registration and DW.ref fixup for IRIX
 *
 * Linked into every executable and shared library by irix-ld.
 * Uses weak references so it's harmless for C programs.
 *
 * Supports TWO unwinder backends:
 *   - GCC libgcc_s: uses __register_frame_info (registers entire section)
 *   - LLVM libunwind: __register_frame_info is a no-op stub!
 *     Must iterate FDEs and call __register_frame for each one.
 *
 * Fixes two IRIX rld limitations:
 *
 * 1. No .eh_frame registration: IRIX rld has no PT_GNU_EH_FRAME support
 *    and doesn't process DT_INIT_ARRAY.
 *
 * 2. DW.ref.__gxx_personality_v0 stays NULL: IRIX rld does not resolve
 *    R_MIPS_REL32 relocations targeting FUNC-type symbols in executables.
 *
 * Link order: crt1.o crtbeginT.o eh_frame_reg.o <user objects> crtendT.o crtn.o
 * This object MUST be linked before user objects so __EH_FRAME_BEGIN__
 * is at the start of the merged .eh_frame section.
 */

/* Weak references — resolve to NULL if not present (pure C programs) */
extern void __register_frame_info(const void *, void *) __attribute__((weak));
extern void __register_frame(const void *) __attribute__((weak));

extern int __gxx_personality_v0(void) __attribute__((weak));
extern int __gcc_personality_v0(void) __attribute__((weak));

/* DW.ref indirect personality pointers — created by clang in user code
 * when generating .eh_frame CIEs with indirect encoding (0x80).
 * The dots in the symbol name require __asm__ renaming. */
extern void *__mogrix_dw_ref_gxx
    __asm__("DW.ref.__gxx_personality_v0") __attribute__((weak));
extern void *__mogrix_dw_ref_gcc
    __asm__("DW.ref.__gcc_personality_v0") __attribute__((weak));

/* .eh_frame start marker — zero-length array at section start.
 * Because this object is linked first (before user objects),
 * this label is at the beginning of the merged .eh_frame section. */
static const char __EH_FRAME_BEGIN__[]
    __attribute__((section(".eh_frame"), aligned(4), used)) = { };

/* Scratch buffer for GCC unwinder's internal object struct.
 * sizeof(struct object) in GCC's unwind-dw2-fde.h is ~28 bytes on ILP32;
 * 64 bytes provides margin. Not used by LLVM libunwind. */
static char __eh_frame_object[64] __attribute__((aligned(8)));

/*
 * Walk .eh_frame section and register each FDE with LLVM libunwind.
 *
 * .eh_frame format (DWARF):
 *   [length:4][CIE_id:4][data...]     CIE: CIE_id == 0
 *   [length:4][CIE_ptr:4][data...]     FDE: CIE_ptr != 0
 *   [0:4]                              Terminator
 *
 * __register_frame() takes a pointer to a single FDE and adds it
 * to libunwind's DwarfFDECache.
 */
static void __register_fdes_with_libunwind(const char *eh_frame) {
    const unsigned char *p = (const unsigned char *)eh_frame;

    for (;;) {
        /* Read 4-byte length */
        unsigned int length = *(const unsigned int *)p;
        if (length == 0)
            break;  /* Terminator */

        /* Extended length (0xFFFFFFFF) — skip, not expected on N32 */
        if (length == 0xFFFFFFFF)
            break;

        /* Read CIE_id/CIE_pointer at offset 4 */
        unsigned int cie_id = *(const unsigned int *)(p + 4);

        if (cie_id != 0) {
            /* This is an FDE (CIE_pointer != 0) — register it */
            __register_frame(p);
        }
        /* else: CIE record — skip */

        /* Advance: length field (4 bytes) + record data (length bytes) */
        p += 4 + length;
    }
}

/* Constructor called from .ctors */
static void __eh_frame_init(void) {
    /* Fix DW.ref personality pointers before .eh_frame registration.
     * IRIX rld resolved the GOT entries (function addresses) but left
     * the R_MIPS_REL32 targets (DW.ref data) as NULL. Copy from GOT. */
    if (&__mogrix_dw_ref_gxx && &__gxx_personality_v0) {
        __mogrix_dw_ref_gxx = (void *)(unsigned long)&__gxx_personality_v0;
    }
    if (&__mogrix_dw_ref_gcc && &__gcc_personality_v0) {
        __mogrix_dw_ref_gcc = (void *)(unsigned long)&__gcc_personality_v0;
    }

    /* Register .eh_frame with the unwinder.
     * Try __register_frame first (LLVM libunwind — per-FDE registration).
     * Fall back to __register_frame_info (GCC libgcc_s — whole-section). */
    if (__register_frame) {
        __register_fdes_with_libunwind(__EH_FRAME_BEGIN__);
    } else if (__register_frame_info) {
        __register_frame_info(__EH_FRAME_BEGIN__, __eh_frame_object);
    }
}

/* Function pointer in .ctors — crtbeginT.o's __do_global_ctors_aux calls it */
static void (*__eh_ctor)(void)
    __attribute__((section(".ctors"), used)) = __eh_frame_init;
