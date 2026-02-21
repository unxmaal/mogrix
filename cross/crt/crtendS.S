/*
 * crtendS.S — End sentinels for .ctors/.dtors arrays (shared library version)
 *
 * Identical to crtendT.S. Provides NULL sentinels at the end of .ctors/.dtors.
 */

	.abicalls		/* MIPS PIC ABI — required for shared library compatibility */

/* ---- .ctors NULL sentinel (end of array) ---- */
	.section .ctors,"aw",@progbits
	.p2align 2
	.globl __CTOR_END__
	.hidden __CTOR_END__
	.type __CTOR_END__,@object
__CTOR_END__:
	.4byte	0

/* ---- .dtors NULL sentinel (end of array) ---- */
	.section .dtors,"aw",@progbits
	.p2align 2
	.globl __DTOR_END__
	.hidden __DTOR_END__
	.type __DTOR_END__,@object
__DTOR_END__:
	.4byte	0
