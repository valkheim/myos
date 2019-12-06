#include <arch/i386/descriptors/idt.h> // IDT_SIZE
#include <arch/i386/isr.h>
#include <stdio.h>

extern isr_t interrupt_handlers[IDT_SIZE];

char const * const fault_messages[] = {
  "Division By Zero",
  "Debug",
  "Non Maskable Interrupt",
  "Breakpoint",
  "Into Detected Overflow",
  "Out of Bounds",
  "Invalid Opcode",
  "No Coprocessor",

  "Double Fault",
  "Coprocessor Segment Overrun",
  "Bad TSS",
  "Segment Not Present",
  "Stack Fault",
  "General Protection Fault",
  "Page Fault",
  "Unknown Interrupt",

  "Coprocessor Fault",
  "Alignment Check",
  "Machine Check",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",

  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved",
  "Reserved"
};

// interrupts.S : ISR handler stub.
void isr_handler(registers_t regs)
{
    // This line is important. When the processor extends the 8-bit interrupt number
    // to a 32bit value, it sign-extends, not zero extends. So if the most significant
    // bit (0x80) is set, regs.int_no will be very large (about 0xffffff80).
    uint8_t int_no = regs.int_no & 0xFF;
    if (interrupt_handlers[int_no] != 0) {
        interrupt_handlers[int_no](regs);
    } else {
      printf("Exception: %s\n", fault_messages[regs.int_no]);
      // busy looping (jmp, jmp, jmp, ...)
      for(;;);
    }
}
