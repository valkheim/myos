#include <arch/i386/isr.h>
#include <stdio.h>

char const *fault_messages[] = {
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
void isr_handler(registers_t regs) {
  if (regs.int_no < 32) {
    printf("Exception: %s\n", fault_messages[regs.int_no]);
    // busy looping (jmp, jmp, jmp, ...)
    for(;;);
  }
}
