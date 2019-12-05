.section .text
.global copy_page_physical
.type copy_page_physical, @function

copy_page_physical:
	push %ebx # save the base pointer (see c declaration (__cdecl))
	pushf # save eflags

	cli # disable interrupt

	mov	12(%esp), %ebx # load source address
	mov	16(%esp), %ecx # load destination address

	mov %cr0, %edx # retrieve the control register under edx
	and $0x7fffffff, %edx # AND-ing max value for an signed 32 bit int. AND-ing it will set the sign bit to zero which ...
	mov %edx, %cr0 # ...disable paging

	mov $1024, %edx # 1024 * 4 bytes = 4096 bytes

.loop:
	mov %ebx, %eax # get word at source address
	mov %eax, %ecx # store it at the destination address
	add $4, %ebx # source address += 4 (size of a word)
	add $4, %ecx # destination address += 4
	dec %edx # one less word to process
	jnz .loop

	mov %cr0, %edx # retrieve control register
	or $0x80000000, %edx # OR-ing smallest 32 bit int value. OR-ing it will ...
	mov %edx, %cr0 # ...enable paging

	popf # retrieve eflags
	pop %ebx # retrieve base pointer
	ret # gtfo
