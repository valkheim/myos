# Macro for interrupt handler which does not push an error (we push a dummy val)
.macro isr_noerr num
.global isr\num
isr\num:
	cli
	push	$0
	push	$\num
	jmp	isr_stub
.endm

# Macro for interrupt handler which pushes an error
.macro isr_err num
.global isr\num
isr\num:
	cli
	push	$\num
	jmp	isr_stub
.endm

isr_noerr 0
isr_noerr 1
isr_noerr 2
isr_noerr 3
isr_noerr 4
isr_noerr 5
isr_noerr 6
isr_noerr 7
isr_err 8
isr_noerr 9
isr_err 10
isr_err 11
isr_err 12
isr_err 13
isr_err 14
isr_noerr 15
isr_noerr 16
isr_noerr 17
isr_noerr 18
isr_noerr 19
isr_noerr 20
isr_noerr 21
isr_noerr 22
isr_noerr 23
isr_noerr 24
isr_noerr 25
isr_noerr 26
isr_noerr 27
isr_noerr 28
isr_noerr 29
isr_noerr 30
isr_noerr 31


.extern isr_common_handler

isr_stub:
	pusha

	mov	 %ds, %ax
	pushl	%eax
	movl	%eax, %esi

	movw	$0x10, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs

	call	isr_common_handler

	pop	%eax
	movl	%esi, %eax

	mov	%ax, %ds
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs

	popa
	add	$8, %esp
	sti
	iret
