#include <arch/i386/descriptors/gdt.h>
#include <arch/i386/descriptors/idt.h>
#include <kernel/kernel.h>
#include <kernel/terminal.h>
#include <drivers/pc/pit.h>
#include <drivers/pc/keyboard.h>
#include <stdio.h>

void kernel_main(void) {
  /* test terminal */
  init_terminal();

  printf("Hello kernel !\n");
  for (char c = 'a' ; c < 'd'; ++c)
    printf("%c\n", c);

  init_gdt();
  init_idt();

  /* test soft interrupts */
  //asm volatile ("int $0x0"); // Div by zero interrupt
  //asm volatile("mov $0, %edx\n"
  //    "mov $0, %eax\n"
  //    "mov $0, %ecx\n"
  //    "div %ecx");

  /* test hard interrupts */

  /* test timer (PIT) */
  init_timer(50); // IRQ0
  /* test keyboard */
  init_keyboard(); // IRQ1

  //PANIC("testing a panic");

  for (;;) {
    /* Halt CPU waiting for next interrupt */
    asm volatile("hlt");
  }
}
