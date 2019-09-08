#ifndef _KERNEL_IDT_H
#define _KERNEL_IDT_H

#include <stdint.h>

// A struct describing an interrupt gate.
typedef struct __attribute__((packed)) {
   uint16_t base_lo;  // The lower 16 bits of the address to jump to when this interrupt fires.
   uint16_t sel;      // Kernel segment selector.
   uint8_t  always0;  // This must always be zero.
   uint8_t  flags;    // More flags. See documentation.
   uint16_t base_hi;  // The upper 16 bits of the address to jump to.
} idt_entry_t;

// A struct describing a pointer to an array of interrupt handlers.
// This is in a format suitable for giving to 'lidt'.
typedef struct __attribute__((packed)) {
   uint16_t limit;
   uint32_t base;     // The address of the first element in our idt_entry_t array.
} idt_ptr_t;

void init_idt(void);

#endif
