#include <kernel/tty.h>
#include <kernel/isr.h>

char const * fault_messages[] =
{
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
  (void)regs;
  //terminal_writestring("received soft interrupt !\n");
  if(regs.int_no < 32)
  {
    terminal_writestring("Exception: ");
    terminal_writestring(fault_messages[regs.int_no]);
    terminal_writestring("\n");
    for(;;);
  }
}
