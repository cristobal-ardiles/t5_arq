	.file	"test-swap.S"
	.option nopic
	.attribute arch, "rv32i2p0_m2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.align	2
	.globl	main
	.type	main, @function
main:				# void main() {
	addi	sp,sp,-16	#   registro de activacion de 16 bytes
	sw	ra, 12(sp)	#   guarda direccion de retorno en 12(sp)
	li	a0, 0x123	#   int x= 0x123;
	sw	a0, 8(sp)	#   // x esta en 8(sp)
	jal	ra, showHexInt	#   showHexInt(x);
	li	a0, ' '
	jal	ra, showChar	#   showChar(' ');
	li	a0, 0x456	#   int y= 0x456;
	sw	a0, 4(sp)	#   // y esta en 4(sp)
	jal	ra, showHexInt	#   showHexInt(y);
	li	a0, '\n'
	jal	ra, showChar	#   showChar('\n');
	lw	a1, 4(sp)	#   // a1 es y
	addi	a2, sp, 8	#   // a2 es &x
.swap:
#	swap	a0, a1, (a2)        Rd es a0=x10, Rs2 es a1=x11, Rs1 es a2=x12
#	swap instruction is not recognized by the assembler
#	so it is encoded by hand
	.word	0x09 + (10<<7)+(2<<12)+(12<<15)+(11<<20)
#	     opcode=0x9 Rd=10  func3=2  Rs1=12  Rs2=11
	sw	a0, 4(sp)	#   // y
	lw	a0, 8(sp)	#   // x
	jal	ra, showHexInt	#   showHexInt(x);
	li	a0, ' '
	jal	ra, showChar	#   showChar(' ');
	lw	a0, 4(sp)
	jal	ra, showHexInt	#   showHexInt(y);
	li	a0, '\n'
	jal	ra, showChar	#   showChar('\n');
	li	a0, 0
	jal	ra, stop	#   stop(0);
				# }
	.size   main, .-main
#	.section        .srodata.cst8,"aM",@progbits,8
#	.align  3
#.LC32:
#	.word   -1564363139
#
#	.ident  "GCC: (GNU) 10.2.0"
