#include <arch/i386/descriptors/idt.h>
#include <arch/i386/descriptors/gdt.h>
#include <arch/i386/isr.h> // ISR handlers
#include <arch/i386/irq.h> // IRQ handlers
#include <sys/io.h> // inb, outb
#include <string.h> // memset

// Implementation in load_idt.S
extern void load_idt(uint32_t);

idt_entry_t idt_entries[IDT_SIZE] = {0};

static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
  // Base
  idt_entries[num].base_lo = base & 0xFFFF;
  idt_entries[num].base_hi = (base >> 16) & 0xFFFF;
  // Selector
  idt_entries[num].sel = sel;
  // Zero and flags
  idt_entries[num].always0 = 0;
  // We must uncomment the OR below when we get to using user-mode.
  // It sets the interrupt gate's privilege level to 3.
  idt_entries[num].flags = flags /* | 0x60 */;
}

void init_idt(void) {
  idt_ptr_t idt_ptr;

  // Init idt pointer
  idt_ptr.limit = sizeof(*idt_entries) * IDT_SIZE -1;
  idt_ptr.base = (uint32_t)&idt_entries;

  // clear idt entries
  memset(&idt_entries, 0, sizeof(*idt_entries) * IDT_SIZE);

  // Remap PIC

  uint8_t m1, m2;

  // save masks
  m1 = inb(PIC_M_DATA);
  m2 = inb(PIC_S_DATA);

  // ICW1 - begin initialization (0x11 being the initialise command)
  outb(PIC_M_CTRL, ICW1_INIT | ICW1_ICW4); // 0x10 | 0x01 = 0x11
  outb(PIC_S_CTRL, ICW1_INIT | ICW1_ICW4); // 0x10 | 0x01 = 0x11

  // ICW2 - remap offset address of idt_entries
  outb(PIC_M_DATA, 0x20); // Master PIC vector offset
  outb(PIC_S_DATA, 0x28); // Slave PIC vector offset

  // ICW3 tell Master PIC that there is a slave PIC at IRQ2 (4 = 0000 0100)
  outb(PIC_M_DATA, 0x04);
  // ICW3: tell Slave PIC its cascade identity (2 = 0000 0010)
  outb(PIC_S_DATA, 0x02);

  // ICW3 setup cascading
  outb(PIC_M_DATA, 0x0);
  outb(PIC_S_DATA, 0x0);

  // mask interrupts
  outb(PIC_M_DATA, ICW4_8086);
  outb(PIC_S_DATA, ICW4_8086);

  // restore saved masks.
  outb(PIC_M_DATA, m1);
  outb(PIC_S_DATA, m2);

  // CPU interrupt layout
  idt_set_gate( 0, (uint32_t)isr0 , 0x08, 0x8E); // Divide by zero
  idt_set_gate( 1, (uint32_t)isr1 , 0x08, 0x8E); // Reserved
  idt_set_gate( 2, (uint32_t)isr2 , 0x08, 0x8E); // NMI (Non Maskable Interrupt)
  idt_set_gate( 3, (uint32_t)isr3 , 0x08, 0x8E); // Breakpoint (INT3)
  idt_set_gate( 4, (uint32_t)isr4 , 0x08, 0x8E); // Overflow (INT0)
  idt_set_gate( 5, (uint32_t)isr5 , 0x08, 0x8E); // Bounds range exceeded (BOUND)
  idt_set_gate( 6, (uint32_t)isr6 , 0x08, 0x8E); // Invalid opcode (UD2)
  idt_set_gate( 7, (uint32_t)isr7 , 0x08, 0x8E); // Device not available (WAIT/FWAIT)
  idt_set_gate( 8, (uint32_t)isr8 , 0x08, 0x8E); // Double fault
  idt_set_gate( 9, (uint32_t)isr9 , 0x08, 0x8E); // Coprocessor segment overrun
  idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E); // Invalid TSS
  idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E); // Segment not present
  idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E); // Stack-segment fault
  idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E); // General protection fault
  idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E); // Page fault
  idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E); // Reserved
  idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E); // x87 FPU error
  idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E); // Alignment check
  idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E); // Machine check
  idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E); // SIMD Floating-Point Exception
  idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E); // Reserved
  idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E); // Reserved
  idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E); // Reserved
  idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E); // Reserved
  idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E); // Reserved
  idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E); // Reserved
  idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E); // Reserved
  idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E); // Reserved
  idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E); // Reserved
  idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E); // Reserved
  idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E); // Reserved
  idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E); // Reserved
  // 32 (0x20) to 255 (0xFF) is user definable

  // Default hardware interrupt layout

  // master
  idt_set_gate(IRQ0_PIT,      (uint32_t)irq0 , 0x08, 0x8E); // timer
  idt_set_gate(IRQ1_KEYBOARD, (uint32_t)irq1 , 0x08, 0x8E); // keyboard
  idt_set_gate(34,            (uint32_t)irq2 , 0x08, 0x8E); // 8256A slave controller
  idt_set_gate(35,            (uint32_t)irq3 , 0x08, 0x8E); // UART (COM2 and COM4)
  idt_set_gate(36,            (uint32_t)irq4 , 0x08, 0x8E); // UART (COM1 and COM3)
  idt_set_gate(37,            (uint32_t)irq5 , 0x08, 0x8E); // Hard disk in PC/XT ; Parallel port LPT2 in PC/AT
  idt_set_gate(38,            (uint32_t)irq6 , 0x08, 0x8E); // Floppy disk controller
  idt_set_gate(39,            (uint32_t)irq7 , 0x08, 0x8E); // Parallel port LPT1
  // slave 8256
  idt_set_gate(40,            (uint32_t)irq8 , 0x08, 0x8E); // RTC (Real Time Clock)
  idt_set_gate(41,            (uint32_t)irq9 , 0x08, 0x8E);
  idt_set_gate(42,            (uint32_t)irq10, 0x08, 0x8E);
  idt_set_gate(43,            (uint32_t)irq11, 0x08, 0x8E);
  idt_set_gate(44,            (uint32_t)irq12, 0x08, 0x8E); // PS/2 mouse controller
  idt_set_gate(45,            (uint32_t)irq13, 0x08, 0x8E); // Math coprocessor
  idt_set_gate(46,            (uint32_t)irq14, 0x08, 0x8E); // Hard disk controller 1
  idt_set_gate(47,            (uint32_t)irq15, 0x08, 0x8E); // Hard disk controller 2

  load_idt((uint32_t)&idt_ptr);
}
