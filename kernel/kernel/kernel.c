#include <config.h>
#ifdef HAVE_STDIO_H
# include <stdio.h>
#endif

#include <kernel/tty.h>

void kernel_main(void) {
  terminal_initialize();
  printf("Hello, kernel World!\n");
}
