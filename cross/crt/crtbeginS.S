/*
 * crtbeginS.S — PIC C++ constructor/destructor support for IRIX shared libraries
 *
 * Same purpose as crtbeginT.S but uses PIC addressing for shared libraries.
 * IRIX rld doesn't process .init_array — it only calls the .init function.
 * This object provides .init code that walks the .ctors array.
 *
 * Link order: crtbeginS.o <user objects> crtendS.o
 */

	.set	noreorder
	.set	nomacro
	.abicalls		/* MIPS PIC ABI */

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

/* ---- Constructor processing function (PIC .text) ---- */
	.text
	.p2align 2
	.globl	__do_global_ctors_aux
	.type	__do_global_ctors_aux,@function
	.ent	__do_global_ctors_aux
__do_global_ctors_aux:
	.frame	$sp, 32, $ra
	.cpload	$25			/* set up $gp from $t9 */
	addiu	$sp, $sp, -32
	.cprestore 16			/* save $gp at 16($sp) */
	sw	$ra, 28($sp)
	sw	$s0, 24($sp)

	/* Load address of __CTOR_END__ via GOT */
	lw	$s0, %got(__CTOR_END__)($gp)
	beqz	$s0, .Ldone		/* guard: rld may zero GOT on re-encounter */
	nop
	addiu	$s0, $s0, -4		/* point to last entry */

	/* Walk .ctors array from end to start */
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
	lw	$gp, 16($sp)		/* restore $gp after call */

	addiu	$s0, $s0, -4		/* move to previous entry */
	b	.Lloop
	nop

.Ldone:
	/* Epilogue */
	lw	$ra, 28($sp)
	lw	$s0, 24($sp)
	jr	$ra
	addiu	$sp, $sp, 32		/* delay slot */
	.end	__do_global_ctors_aux

/* ---- Code contributed to .init section ---- */
/* IRIX rld calls .init (DT_INIT) when loading the DSO */
	.section .init,"ax",@progbits
	.set	noreorder
	.globl	_init
	.type	_init,@function
_init:
	/* Save ra so the caller's return works after our call */
	addiu	$sp, $sp, -16
	sw	$ra, 12($sp)
	sw	$gp, 8($sp)

	/* Call __do_global_ctors_aux via PIC */
	/* Use bal to compute $gp, then load from GOT */
	bal	1f
	nop
1:
	.set	push
	.set	noat
	lui	$1, %hi(%neg(%gp_rel(1b)))
	addiu	$1, $1, %lo(%neg(%gp_rel(1b)))
	addu	$gp, $1, $ra
	.set	pop

	lw	$25, %got(__do_global_ctors_aux)($gp)
	jalr	$25
	nop

	/* Restore ra and gp, return */
	lw	$gp, 8($sp)
	lw	$ra, 12($sp)
	jr	$ra
	addiu	$sp, $sp, 16		/* delay slot */
