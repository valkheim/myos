#ifndef _KERNEL_IDT_H
#define _KERNEL_IDT_H

#include <stdint.h>

#define IDT_SIZE (256)

#define PIC_M (0x20)  // IO base address for master PIC
#define PIC_S (0xA0)  // IO base address for slave PIC
#define PIC_M_CTRL (PIC_M)     // 0x20
#define PIC_M_DATA (PIC_M + 1) // 0x21
#define PIC_S_CTRL (PIC_S)     // 0xA0
#define PIC_S_DATA (PIC_S + 1) // 0xA1

#define ICW1_ICW4       (0x01) // ICW4 (not) needed
#define ICW1_SINGLE     (0x02) // Single (cascade) mode
#define ICW1_INTERVAL4  (0x04) // Call address interval 4 (8)
#define ICW1_LEVEL      (0x08) // Level triggered (edge) mode
#define ICW1_INIT       (0x10) // Initialization - required!

#define ICW4_8086       (0x01) // 8086/88 (MCS-80/85) mode
#define ICW4_AUTO       (0x02) // Auto (normal) EOI
#define ICW4_BUF_SLAVE  (0x08) // Buffered mode/slave
#define ICW4_BUF_MASTER (0x0C) // Buffered mode/master
#define ICW4_SFNM       (0x10) // Special fully nested (not)

#define PIC_EOI (0x20) // End Of Interrupt

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
