#include <stdint.h>
#include <kernel/irq.h>
#include <kernel/tty.h>
#include <sys/io.h>

isr_t interrupt_handlers[256];

// interrupts.S : IRQ handler.
void irq_handler(registers_t regs) {
  // Send an EOI (end of interrupt) signal to the PICs.
  // If this interrupt involved the slave.
  terminal_writestring("received hard interrupt !\n");
  if (regs.int_no >= 40)
  {
    // Send reset signal to slave.
    outb(0xA0, 0x20);
  }
  // Send reset signal to master. (As well as slave, if necessary).
  outb(0x20, 0x20);

  if (interrupt_handlers[regs.int_no] != 0)
  {
    isr_t handler = interrupt_handlers[regs.int_no];
    handler(regs);
  }
}

void register_interrupt_handler(uint8_t n, isr_t handler) {
  terminal_writestring("register int handler\n");
  interrupt_handlers[n] = handler;
}
