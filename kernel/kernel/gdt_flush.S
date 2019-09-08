# This will set up our new segment registers. We need to do
# something special in order to set CS. We do what is called a
# far jump. A jump that includes a segment as well as an offset.
# This is declared in C as 'extern void gdt_flush();'
.section .text
.global load_gdt   # Allows the C code to link to this
.type load_gdt, @function
.extern gdt_ptr

load_gdt:
	lgdt (gdt_ptr)    # Load the GDT with our 'gdt_ptr' which is a special pointer
	mov $0x10, %ax    # 0x10 is the offset in the GDT to our data segment
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	mov %ax, %ss
	ljmp $0x08, $.flush # 0x08 is the offset to our code segment: Far jump!
.flush:
	ret               # Returns back to the C code!

.size load_gdt, . - load_gdt
