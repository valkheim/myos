#ifndef _KERNEL_IRQ_H
#define _KERNEL_IRQ_H

#include <stdint.h>
#include <arch/i386/isr.h> // registers_t

#define IRQ0_PIT       (32) // 0x20
#define IRQ1_KEYBOARD  (33) // 0x21
#define IRQ2           (34) // 0x22
#define IRQ3           (35) // 0x23
#define IRQ4           (36) // 0x24
#define IRQ5           (37) // 0x25
#define IRQ6           (38) // 0x26
#define IRQ7           (39) // 0x27
#define IRQ8           (40) // 0x28
#define IRQ9           (41) // 0x29
#define IRQ10          (42) // 0x2A
#define IRQ11          (43) // 0x2B
#define IRQ12          (44) // 0x2C
#define IRQ13          (45) // 0x2D
#define IRQ14          (46) // 0x2E
#define IRQ15          (47) // 0x2F

// Enables registration of callbacks for interrupts or IRQs.
// For IRQs, to ease confusion, use the #defines above as the
// first parameter.
typedef void (*isr_t)(registers_t);
void register_interrupt_handler(uint8_t const n, isr_t const handler);

// IRQ handlers
extern void irq0 ();
extern void irq1 ();
extern void irq2 ();
extern void irq3 ();
extern void irq4 ();
extern void irq5 ();
extern void irq6 ();
extern void irq7 ();
extern void irq8 ();
extern void irq9 ();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

#endif
