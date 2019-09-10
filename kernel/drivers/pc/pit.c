#include <drivers/pc/pit.h>
#include <arch/i386/isr.h> // registers_t
#include <arch/i386/irq.h> // IRQ0_PIT
//#include <stdio.h> // printf
#include <sys/io.h>

uint32_t tick = 0;

static void timer_callback(__attribute__((unused)) registers_t regs) {
  //printf("Tick\n");
  tick++;
}

void init_timer(uint32_t frequency) {
  // Firstly, register our timer callback.
  register_interrupt_handler(IRQ0_PIT, &timer_callback);

  // The value we send to the PIT is the value to divide it's input clock
  // (1193182 Hz) by, to get our required frequency. Important to note is
  // that the divisor must be small enough to fit into 16-bits.
  uint32_t divisor = 1193182 / frequency;

  // Send the command byte.
  // mov $0x36, %al
  // out %al, $0x43
  outb(0x43, 0x36);

  // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
  uint8_t l = (uint8_t)(divisor & 0xFF);
  uint8_t h = (uint8_t)((divisor>>8) & 0xFF);

  // Send the frequency divisor.
  // mov divisor, %ax
  // out %ah, $0x40
  // out %al, $0x40
  outb(0x40, l);
  outb(0x40, h);
}
