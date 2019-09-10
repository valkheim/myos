#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H

#include <stddef.h>

void init_terminal(void);
void terminal_putchar(char c);
void terminal_write(char const *data, size_t size);
void terminal_writestring(char const *data);

#endif
