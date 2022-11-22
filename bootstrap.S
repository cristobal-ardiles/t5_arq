	.file	"test-swap.S"
	.option nopic
	.attribute arch, "rv32i2p0_m2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.align	2
	.globl	_start, swap
	.type	_start, @function
_start:
	bge	zero, zero, .LCbegin  # 0
	.word	0	# 4 address of function _start
	.word	0	# 8 address of function stop
.LCbegin:
	li	sp, 1		# c
	slli	sp, sp, 26	# 10 int *sp = (void*)(intptr_t)(64*1024*1024);
	lw	a3, 4(zero)	# 14 int (*a3)() = *(int*)4;
	jalr	ra, a3		# 18 (*a3)();
	lw	a3, 8(zero)	# a3= *(int*)8;
	jalr	ra, a3		# (*a3)(a0)
.LCloop:				# .LCloop:
	bge	zero, zero, .LCloop	# goto .LCloop;

	.size   _start, .-_start
	.section        .srodata.cst8,"aM",@progbits,8
	.align  3
.LC32:
	.word   -1564363139

	.ident  "GCC: (GNU) 10.2.0"
