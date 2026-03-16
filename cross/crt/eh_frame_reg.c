/*
 * eh_frame_reg.c — .eh_frame registration and DW.ref fixup for IRIX executables
 *
 * Linked into every executable by irix-ld (right after crtbeginT.o).
 * Uses weak references so it's harmless for C programs that don't
 * link libgcc_s or use exceptions.
 *
 * Fixes two IRIX rld limitations:
 *
 * 1. No .eh_frame registration: IRIX rld has no PT_GNU_EH_FRAME support
 *    and doesn't process DT_INIT_ARRAY. The GCC DW2 unwinder uses
 *    registration-based FDE lookup (__register_frame_info), so without
 *    this, _Unwind_Find_FDE fails and _Unwind_RaiseException returns
 *    _URC_END_OF_STACK → __cxa_throw calls terminate().
 *
 * 2. DW.ref.__gxx_personality_v0 stays NULL: IRIX rld does not resolve
 *    R_MIPS_REL32 relocations targeting FUNC-type symbols in executables.
 *    Clang generates DW.ref.* indirect pointers in .data for .eh_frame
 *    personality references (encoding 0x80 = DW_EH_PE_indirect). Without
 *    fixup, the unwinder reads NULL as the personality function pointer.
 *
 * Link order: crt1.o crtbeginT.o eh_frame_reg.o <user objects> crtendT.o crtn.o
 * This object MUST be linked before user objects so __EH_FRAME_BEGIN__
 * is at the start of the merged .eh_frame section.
 */

/* Weak references — resolve to NULL if not present (pure C programs) */
extern void __register_frame_info(const void *, void *) __attribute__((weak));

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

/* Scratch buffer for the unwinder's internal object struct.
 * sizeof(struct object) in GCC's unwind-dw2-fde.h is ~28 bytes on ILP32;
 * 64 bytes provides margin. */
static char __eh_frame_object[64] __attribute__((aligned(8)));

/* Constructor called by crtbeginT.o's __do_global_ctors_aux.
 * .ctors are walked backward from __CTOR_END__, so this entry
 * (early in the array) runs LAST among constructors — after all
 * user constructors have been registered, we register .eh_frame. */
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

    /* Register executable's .eh_frame with the GCC DW2 unwinder */
    if (__register_frame_info) {
        __register_frame_info(__EH_FRAME_BEGIN__, __eh_frame_object);
    }
}

/* Function pointer in .ctors — crtbeginT.o's __do_global_ctors_aux calls it */
static void (*__eh_ctor)(void)
    __attribute__((section(".ctors"), used)) = __eh_frame_init;
