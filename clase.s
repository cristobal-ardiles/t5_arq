	.text
	.align	2
	.globl	main
main:
	addi	sp,sp,-64
	li	a0,100
	li      a1,50
	li	t4,-1
	sh	t4,-2(sp)
.destino:
	add	t0,a0,a1
	add	t1,a2,zero
	or	t2,a0,a1
	sb	t4,15(sp)
	lh	t3,-2(sp)
	blt	a0,a1,.destino
	bge	a0,a1,.destino
