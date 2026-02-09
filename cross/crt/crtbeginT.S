/*
 * crtbeginT.S — C++ constructor/destructor support for IRIX cross-compilation
 *
 * IRIX crt1.o and crtn.o provide .init prologue/epilogue but don't process
 * .ctors/.dtors arrays. This object adds constructor processing code to .init.
 *
 * Link order: crt1.o crtbeginT.o <user objects> crtendT.o crtn.o
 *
 * .init section layout after linking:
 *   [crt1.o: nops]
 *   [crtbeginT.o: save ra, call __do_global_ctors_aux, restore ra]
 *   [crtn.o: jr ra]
 */

	.set	noreorder
	.set	nomacro

/* ---- .ctors sentinel (start of array) ---- */
	.section .ctors,"aw",@progbits
	.p2align 2
__CTOR_LIST__:
	.4byte	0xffffffff

/* ---- .dtors sentinel (start of array) ---- */
	.section .dtors,"aw",@progbits
	.p2align 2
__DTOR_LIST__:
	.4byte	0xffffffff

/* ---- Constructor processing function (normal .text) ---- */
	.text
	.p2align 2
	.type	__do_global_ctors_aux,@function
__do_global_ctors_aux:
	/* Prologue */
	addiu	$sp, $sp, -32
	sw	$ra, 28($sp)
	sw	$s0, 24($sp)
	sw	$gp, 20($sp)

	/* Load address of __CTOR_END__ - 4 (last real entry before NULL sentinel) */
	lui	$s0, %hi(__CTOR_END__)
	addiu	$s0, $s0, %lo(__CTOR_END__)
	addiu	$s0, $s0, -4		/* point to last entry */

	/* Load the sentinel value -1 */
.Lloop:
	lw	$25, 0($s0)		/* load function pointer */
	lui	$2, 0xffff
	ori	$2, $2, 0xffff		/* $2 = -1 (sentinel) */
	beq	$25, $2, .Ldone		/* stop at -1 sentinel */
	nop
	beq	$25, $0, .Ldone		/* also stop at NULL */
	nop

	jalr	$25			/* call the constructor */
	nop

	addiu	$s0, $s0, -4		/* move to previous entry */
	b	.Lloop
	nop

.Ldone:
	/* Epilogue */
	lw	$ra, 28($sp)
	lw	$s0, 24($sp)
	lw	$gp, 20($sp)
	jr	$ra
	addiu	$sp, $sp, 32		/* delay slot */

/* ---- Code contributed to .init section ---- */
/* This gets concatenated between crt1.o's .init and crtn.o's .init */
	.section .init,"ax",@progbits
	/* Save ra so crtn.o's "jr ra" returns correctly after our call */
	addiu	$sp, $sp, -16
	sw	$ra, 12($sp)

	/* Call __do_global_ctors_aux */
	lui	$25, %hi(__do_global_ctors_aux)
	addiu	$25, $25, %lo(__do_global_ctors_aux)
	jalr	$25
	nop

	/* Restore ra */
	lw	$ra, 12($sp)
	addiu	$sp, $sp, 16
