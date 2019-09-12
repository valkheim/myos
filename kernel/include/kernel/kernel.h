#ifndef _KERNEL_KERNEL_H
#define _KERNEL_KERNEL_H

#include <stdio.h>
#include <stdlib.h>

#define PANIC(reason)                   \
  do {                                \
    printf("kernel panic: %s at %s:%d\n", reason, __FILE__, __LINE__); \
    abort();                        \
  } while (0)


#endif
