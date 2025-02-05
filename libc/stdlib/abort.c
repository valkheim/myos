#include <stdio.h>
#include <stdlib.h>

__attribute__((__noreturn__))
void abort(void) {
#if defined(__is_libk)
  // TODO: Add proper kernel panic.
  while (1) {
    asm volatile("cli;hlt");
  }
#else
  // TODO: Abnormally terminate the process as if by SIGABRT.
  printf("abort()\n");
#endif
  while (1) { }
  __builtin_unreachable();
}
