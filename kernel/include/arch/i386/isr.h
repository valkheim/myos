#ifndef _KERNEL_ISR_H
#define _KERNEL_ISR_H

#include <stdint.h>

// High level interrupt service routines

typedef struct {
  uint32_t ds; // Data segment selector
  uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax; // Pushed by pusha.
  uint32_t int_no, err_code; // Interrupt number and error code (if applicable)
  uint32_t eip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} registers_t;

typedef void (*isr_t)(registers_t);

#define ISR0_DIVISION_BY_ZERO  (0) // 0x00
#define ISR1                   (1) // 0x01
#define ISR2                   (2) // 0x02
#define ISR3                   (3) // 0x03
#define ISR4                   (4) // 0x04
#define ISR5                   (5) // 0x05
#define ISR6                   (6) // 0x06
#define ISR7                   (7) // 0x07
#define ISR8                   (8) // 0x08
#define ISR9                   (9) // 0x09
#define ISR10                 (10) // 0x0A
#define ISR11                 (11) // 0x0B
#define ISR12                 (12) // 0x0C
#define ISR13                 (13) // 0x0D
#define ISR14_PAGE_FAULT      (14) // 0x0E
// ISR number 15 is reserved
#define ISR16                 (16) // 0x10
#define ISR17                 (17) // 0x11
#define ISR18                 (18) // 0x12
#define ISR19                 (19) // 0x13
#define ISR20                 (20) // 0x14
// ISR numbers 21-29 are reserved
#define ISR30                 (30) // 0x1E
// ISR number 31 is reserved

// Triple fault occurs on double fault handler exception

extern char const * const fault_messages[];

// ISR handlers
extern void isr0 ();
extern void isr1 ();
extern void isr2 ();
extern void isr3 ();
extern void isr4 ();
extern void isr5 ();
extern void isr6 ();
extern void isr7 ();
extern void isr8 ();
extern void isr9 ();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

#endif
