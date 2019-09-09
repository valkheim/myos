#include <kernel/idt.h>
#include <kernel/isr.h> // ISR handlers
#include <kernel/irq.h> // IRQ handlers
#include <sys/io.h>
#include <string.h>

extern void load_idt(uint32_t);

idt_entry_t idt_entries[256];
idt_ptr_t idt_ptr;

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
  // Init idt pointer
  idt_ptr.limit = sizeof(idt_entry_t) * 256 -1;
  idt_ptr.base  = (uint32_t)&idt_entries;

  memset(&idt_entries, 0, sizeof(idt_entry_t)*256);

  // Remap PIC

  /*
                MASTER
           +--------------+
           |              |                          SLAVE
           |            0 |-- IRQ 0             +--------------+
           |            1 |-- IRQ 1             |              |
           |            2 |--<-----------+      |            0 |-- IRQ 8
   CPU <---| INT        3 |-- IRQ 3      |      |            1 |-- IRQ 9
           |            4 |-- IRQ 4      |      |            2 |-- IRQ 10
           |            5 |-- IRQ 5      +---<--| INT        3 |-- IRQ 11
           |            6 |-- IRQ 6             |            4 |-- IRQ 12
           |            7 |-- IRQ 7             |            5 |-- IRQ 13
           |              |                     |            6 |-- IRQ 14
           +--------------+                     |            7 |-- IRQ 15
                                                |              |
                                                +--------------+
  */

  // Master/slave ports
  outb(0x20, 0x11); // master PIC's port A
  outb(0xA0, 0x11); // slave PIC's port A
  outb(0x21, 0x20); // master PIC's port B
  outb(0xA1, 0x28); // slave PIC(s port B

  // ICW 1 and 2, OCW 1 and 2
  outb(0x21, 0x04);
  outb(0xA1, 0x02);
  outb(0x21, 0x01);
  outb(0xA1, 0x01);
  outb(0x21, 0x0);
  outb(0xA1, 0x0);

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
  idt_set_gate(32, (uint32_t)irq0 , 0x08, 0x8E); // timer
  idt_set_gate(33, (uint32_t)irq1 , 0x08, 0x8E); // keyboard
  idt_set_gate(34, (uint32_t)irq2 , 0x08, 0x8E); // 8256A slave controller
  idt_set_gate(35, (uint32_t)irq3 , 0x08, 0x8E); // UART (COM2 and COM4)
  idt_set_gate(36, (uint32_t)irq4 , 0x08, 0x8E); // UART (COM1 and COM3)
  idt_set_gate(37, (uint32_t)irq5 , 0x08, 0x8E); // Hard disk in PC/XT ; Parallel port LPT2 in PC/AT
  idt_set_gate(38, (uint32_t)irq6 , 0x08, 0x8E); // Floppy disk controller
  idt_set_gate(39, (uint32_t)irq7 , 0x08, 0x8E); // Parallel port LPT1
  // slave 8256
  idt_set_gate(40, (uint32_t)irq8 , 0x08, 0x8E); // RTC (Real Time Clock)
  idt_set_gate(41, (uint32_t)irq9 , 0x08, 0x8E);
  idt_set_gate(42, (uint32_t)irq10, 0x08, 0x8E);
  idt_set_gate(43, (uint32_t)irq11, 0x08, 0x8E);
  idt_set_gate(44, (uint32_t)irq12, 0x08, 0x8E); // PS/2 mouse controller
  idt_set_gate(45, (uint32_t)irq13, 0x08, 0x8E); // Math coprocessor
  idt_set_gate(46, (uint32_t)irq14, 0x08, 0x8E); // Hard disk controller 1
  idt_set_gate(47, (uint32_t)irq15, 0x08, 0x8E); // Hard disk controller 2

  load_idt((uint32_t)&idt_ptr);
}
