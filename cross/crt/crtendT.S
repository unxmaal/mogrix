/*
 * crtendT.S — End sentinels for .ctors/.dtors arrays
 *
 * Link order: crt1.o crtbeginT.o <user objects> crtendT.o crtn.o
 */

/* ---- .ctors NULL sentinel (end of array) ---- */
	.section .ctors,"aw",@progbits
	.p2align 2
	.globl __CTOR_END__
	.type __CTOR_END__,@object
__CTOR_END__:
	.4byte	0

/* ---- .dtors NULL sentinel (end of array) ---- */
	.section .dtors,"aw",@progbits
	.p2align 2
	.globl __DTOR_END__
	.type __DTOR_END__,@object
__DTOR_END__:
	.4byte	0
