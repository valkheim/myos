#include <stdint.h>
#include <stdio.h>

void panic(char const * const message, char const * const file, uint32_t const line) {
    // We encountered a massive problem and have to stop.
    asm volatile("cli"); // Disable interrupts.

    printf("PANIC(%s) at %s:%u\n", message, file, line);

    // Halt by going into an infinite loop.
    for(;;);
}

void panic_assert(char const * const file, uint32_t const line, char const * const desc) {
    // An assertion failed, and we have to panic.
    asm volatile("cli"); // Disable interrupts.

    printf("ASSERT FAILED(%s) at %s:%u\n", desc, file, line);

    // Halt by going into an infiniteloop.
    for(;;);
}