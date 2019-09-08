#include <kernel/tty.h>
#include <kernel/isr.h>

// interrupts.S : common handler stub.
void isr_common_handler(registers_t regs)
{
  (void)regs;
  terminal_writestring("received interrupt !\n");
}
