/*
 * libstdcxx_frame_reg.c — .eh_frame registration for libstdc++.so
 *
 * Registers libstdc++.so's .eh_frame with the GCC DW2 unwinder so
 * _Unwind_RaiseException can find FDEs for functions like __cxa_throw
 * and __gxx_personality_v0.
 *
 * Without this, the unwinder returns _URC_END_OF_STACK because it can't
 * step through libstdc++ frames, and __cxa_throw calls terminate().
 *
 * Uses .ctors section (NOT .init_array) because IRIX rld doesn't
 * process DT_INIT_ARRAY. irix-ld auto-adds crtbeginS.o which provides
 * _init → __do_global_ctors_aux to walk .ctors at DSO load time.
 */

extern void __register_frame_info(const void *, void *);
extern void __deregister_frame_info(const void *);

/* Zero-length array at the start of .eh_frame */
static const char __EH_FRAME_BEGIN__[]
    __attribute__((section(".eh_frame"), aligned(4), used)) = { };

/* Object struct for the unwinder's internal list.
 * GCC's struct object is ~28 bytes on ILP32; 64 bytes is safe. */
static char __eh_object[64] __attribute__((aligned(8)));

static void __libstdcxx_register_frame(void) {
    __register_frame_info(__EH_FRAME_BEGIN__, __eh_object);
}

static void __libstdcxx_deregister_frame(void) {
    __deregister_frame_info(__EH_FRAME_BEGIN__);
}

/* Put function pointers in .ctors/.dtors so crtbeginS.o's _init calls them */
static void (*__reg_ctor)(void)
    __attribute__((section(".ctors"), used)) = __libstdcxx_register_frame;
static void (*__dereg_dtor)(void)
    __attribute__((section(".dtors"), used)) = __libstdcxx_deregister_frame;
