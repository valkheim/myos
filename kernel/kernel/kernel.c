#include <config.h>
#ifdef HAVE_STDIO_H
# include <stdio.h>
#endif

#include <kernel/tty.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/pit.h>

void kernel_main(void) {
  init_gdt();
  init_idt();

  /* test terminal */
  terminal_initialize();
  printf("Hello kernel !\n");
  for (char i = 'a' ; i < 'd'; ++i)
    printf("Hello, %c\n", i);

  /* test interrupts */
  asm volatile ("int $0x3");

  /* Enable interrupts after PIC re-mapping */
  asm volatile("sti");

  /* test timer (PIT) */
  asm volatile ("int $0x20"); // trigger with int 0x20
  init_timer(60); // trigger with IRQ0

  while(1) {
    /* Halt CPU waiting for next interrupt */
    asm volatile("hlt");
  }
}
