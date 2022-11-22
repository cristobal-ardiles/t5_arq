	.file	"sw.S"
	.option nopic
	.attribute arch, "rv32i2p0_m2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.align	2
	.globl	main
	.type	main, @function
main:
	addi	sp, sp, -16
	li	a4, 0x1234
	mv	t1, sp
	sw	a4, 8(t1)
	addi	sp, sp, 16
	ret

	.size	main, .-main
	.ident	"GCC: (GNU) 10.2.0"
