#ifndef _STDIO_H
#define _STDIO_H 1

#include <stdint.h>
#include <stddef.h>
#include <sys/cdefs.h>

#define EOF (-1)

int printf(const char* __restrict, ...);
int putchar(int);
int puts(const char*);

#define PANIC(msg) panic(msg, __FILE__, __LINE__);
#define ASSERT(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b))

void panic(char const * const message, char const * const file, uint32_t const line);
void panic_assert(char const * const file, uint32_t const line, char const * const desc);

#endif
