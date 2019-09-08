#include <config.h>
#ifdef HAVE_STDIO_H
# include <stdio.h>
#endif

#include <kernel/tty.h>
#include <kernel/gdt.h>
#include <kernel/idt.h>

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
}
