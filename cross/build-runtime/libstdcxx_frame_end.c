/*
 * libstdcxx_frame_end.c — .eh_frame terminator for libstdc++.so
 *
 * Link LAST so the zero word is at the end of .eh_frame.
 * The GCC unwinder uses this sentinel to know where to stop scanning.
 */

static const int __FRAME_END__[]
    __attribute__((section(".eh_frame"), aligned(4), used)) = { 0 };
