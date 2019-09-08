#ifndef _KERNEL_GDT_H
#define _KERNEL_GDT_H

#include <stdint.h>

// 5 entries
// code and data segment descriptors for the kernel
// code and data segment descriptors for the user mode
// null entry
#define GDT_ENTRIES (5)

// This structure contains the value of one GDT entry.
// We use the attribute 'packed' to tell GCC not to change
// any of the alignment in the structure.
typedef struct  __attribute__((packed)) {
  uint16_t limit_low;   // The lower 16 bits of the limit.
  uint16_t base_low;    // The lower 16 bits of the base.
  uint8_t  base_middle; // The next 8 bits of the base.
  uint8_t  access;      // Access flags, determine what ring this segment can be used in.
  uint8_t  granularity;
  uint8_t  base_high;   // The last 8 bits of the base.
} gdt_entry_t;


// This struct describes a GDT pointer. It points to the start of
// our array of GDT entries, and is in the format required by the
// lgdt instruction.
typedef struct __attribute__((packed)) {
  uint16_t limit;       // The upper 16 bits of all selector limits.
  uint32_t base;        // The address of the first gdt_entry_t struct.
} gdt_ptr_t;

void init_gdt(void);

#endif
