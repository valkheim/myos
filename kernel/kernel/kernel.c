#include <config.h>
#ifdef HAVE_STDIO_H
# include <stdio.h>
#endif

#include <kernel/tty.h>
#include <mm/gdt.h>

void kernel_main(void) {
  init_descriptor_tables();

  terminal_initialize();
  printf("Hello kernel !\n");
  for (char i = 'a' ; i < 'd'; ++i)
    printf("Hello, %c\n", i);
}
