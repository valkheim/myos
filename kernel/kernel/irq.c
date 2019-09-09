#include <stdint.h>
#include <kernel/irq.h>
#include <kernel/idt.h> // PIC_(M|S)_CTRL
#include <kernel/tty.h>
#include <sys/io.h>

isr_t interrupt_handlers[IDT_SIZE] = {0};

static void send_eoi(unsigned char const irq) {
  // Send End Of Interrupt to the PIC(s)
  if (irq >= 40) // Interrupt involved the slave (IRQ > 7, int number >= 40)
    outb(PIC_S_CTRL, PIC_EOI); // ack the slave
  outb(PIC_M_CTRL, PIC_EOI); // ack the master
}

// interrupts.S : IRQ handler.
void irq_handler(registers_t regs) {
  send_eoi(regs.int_no);
  if (interrupt_handlers[regs.int_no] != 0)
  {
    isr_t handler = interrupt_handlers[regs.int_no];
    handler(regs);
  }
}

void register_interrupt_handler(uint8_t const n, isr_t const handler) {
  interrupt_handlers[n] = handler;
}
