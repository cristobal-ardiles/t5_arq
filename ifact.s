	.file	"ifact.c"
	.option nopic
	.attribute arch, "rv32i2p0_m2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.align	2
	.globl	factorial
	.type	factorial, @function
factorial:
	li	a5,1
	bgt	a0,a5,.L8
	li	a0,1
	ret
.L8:
	addi	sp,sp,-16
	sw	ra,12(sp)
	sw	s0,8(sp)
	mv	s0,a0
	addi	a0,a0,-1
	call	factorial
	mul	a0,a0,s0
	lw	ra,12(sp)
	lw	s0,8(sp)
	addi	sp,sp,16
	jr	ra
	.size	factorial, .-factorial
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
.LC0:
	.string	"? "
	.align	2
.LC1:
	.string	"fact("
	.align	2
.LC2:
	.string	")="
	.text
	.align	2
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-128
	sw	ra,124(sp)
	sw	s0,120(sp)
	sw	s1,116(sp)
	sw	s2,112(sp)
	sw	s3,108(sp)
	lui	s3,%hi(.LC0)
	lui	s2,%hi(.LC1)
	lui	s1,%hi(.LC2)
.L10:
	addi	a0,s3,%lo(.LC0)
	call	showStr
	li	a1,80
	addi	a0,sp,12
	call	readLine
	addi	a0,sp,12
	call	atoi
	mv	s0,a0
	addi	a0,s2,%lo(.LC1)
	call	showStr
	mv	a0,s0
	call	showInt
	addi	a0,s1,%lo(.LC2)
	call	showStr
	mv	a0,s0
	call	factorial
	call	showInt
	li	a0,10
	call	showChar
	j	.L10
	.size	main, .-main
	.ident	"GCC: (GNU) 11.1.0"
