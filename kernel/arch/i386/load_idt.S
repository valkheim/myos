.section .text
.global load_idt
.type load_idt, @function

# void load_gdt(gdt_ptr_t *)
load_idt:
	movl	4(%esp),%eax # get argument (idt pointer)
	lidt	(%eax) # pass idt pointer to lidt
	sti
	ret

.size load_idt, . - load_idt
