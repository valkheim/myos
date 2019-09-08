.section .text
.global idt_flush

.type idt_flush, @function
# void load_idt(idt_ptr_t *)
idt_flush:
	movl	4(%esp),%eax
	lidt	(%eax)
	ret

.size idt_flush, . - idt_flush
