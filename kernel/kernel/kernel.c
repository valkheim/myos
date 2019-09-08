#include <config.h>
#ifdef HAVE_STDIO_H
# include <stdio.h>
#endif

#include <kernel/tty.h>

void kernel_main(void) {
  terminal_initialize();
  printf("Hello kernel !\n");
  for (char i = 'a' ; i < 'd'; ++i)
    printf("Hello, %c\n", i);
}
