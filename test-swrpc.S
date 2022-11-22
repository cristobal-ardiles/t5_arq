	.file	"test-swrpc.c"
	.option nopic
	.attribute arch, "rv32i2p0_m2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
.LC1:
	.string	"escr. "
	.align	2
.LC2:
	.string	" en "
	.align	2
.LC3:
	.string	"*** Error: se escribio "
	.align	2
.LC4:
	.string	" ***\n"
	.align	2
.LC5:
	.string	"Felicitaciones!\n"
	.text
	.align	2
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-32
	sw	ra,28(sp)
	sw	s0,24(sp)
	sw	s1,20(sp)
	sw	s2,16(sp)
	sw	s3,12(sp)
	sw	s4,8(sp)
	sw	s5,4(sp)
	# beq	a0,zero,.L7
.L2:
	li	s0,0
	lui	s4,%hi(.LC1)
	lui	s3,%hi(.LC2)
	li	s5,3
.L4:
#	store word s0 relative .L7-.L4 bytes to pc, write address in s1
#	*** assembler : swrpc s1, s0,0x70(pc)
#		opcode=0x50 Rd=s1=x9 func3=2   Rs1=s0=x8 RPC=0x70 en bits 31-25
#                                                                  y bits 19-15
	.word	0x50 +      (9<<7) + (2<<12) + (8<<20) + (0x3<<25) + (0x10<<15)
	addi	a0,s4,%lo(.LC1)
	call	showStr
	mv	a0,s0
	call	showInt
	addi	a0,s3,%lo(.LC2)
	call	showStr
	mv	a0,s1
	call	showHexInt
	li	a0,10
	call	showChar
	lw	s2,0(s1)
	bne	s2,s0,.L8
	addi	s0,s0,1
	bne	s0,s5,.L4
	lui	a0,%hi(.LC5)
	addi	a0,a0,%lo(.LC5)
	call	showStr
	li	a0,0
	lw	ra,28(sp)
	lw	s0,24(sp)
	lw	s1,20(sp)
	lw	s2,16(sp)
	lw	s3,12(sp)
	lw	s4,8(sp)
	lw	s5,4(sp)
	addi	sp,sp,32
	jr	ra
.L7:
	.zero	4
.L8:
	lui	a0,%hi(.LC3)
	addi	a0,a0,%lo(.LC3)
	call	showStr
	mv	a0,s2
	call	showInt
	lui	a0,%hi(.LC4)
	addi	a0,a0,%lo(.LC4)
	call	showStr
	li	a0,1
	call	exit
	.size	main, .-main
	.ident	"GCC: (GNU) 11.1.0"
