#ifndef _KERNEL_IDT_H
#define _KERNEL_IDT_H

#include <stdint.h>

#define IDT_SIZE (256)

#define PIC_M_CTRL (0x20)
#define PIC_S_CTRL (0xA0)
#define PIC_M_DATA (0x21)
#define PIC_S_DATA (0xA1)

  /*
                MASTER (A)
           +--------------+
           |              |                          SLAVE (B)
           |            0 |-- IRQ 0             +--------------+
           |            1 |-- IRQ 1             |              |
           |            2 |--<-----------+      |            0 |-- IRQ 8
   CPU <---| INT        3 |-- IRQ 3      |      |            1 |-- IRQ 9
           |            4 |-- IRQ 4      |      |            2 |-- IRQ 10
           |            5 |-- IRQ 5      +---<--| INT        3 |-- IRQ 11
           |            6 |-- IRQ 6             |            4 |-- IRQ 12
           |            7 |-- IRQ 7             |            5 |-- IRQ 13
           |              |                     |            6 |-- IRQ 14
           +--------------+                     |            7 |-- IRQ 15
                                                |              |
                                                +--------------+
  */


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
