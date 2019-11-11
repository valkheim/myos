#include <arch/i386/descriptors/gdt.h>
#include <arch/i386/descriptors/idt.h>
#include <kernel/kernel.h>
#include <kernel/terminal.h>
#include <mm/malloc.h>
#include <drivers/pc/pit.h>
#include <drivers/pc/keyboard.h>
#include <stdio.h>

extern uint32_t kernel_base;
extern uint32_t kernel_end;

void kernel_main(void) {

  init_terminal();

  printf("Hello kernel!\n");
  printf("Kernel base is 0x%x, end is 0x%x\n", &kernel_base, &kernel_end);

  init_mm(&kernel_end);
  mm_status();
  init_gdt();
  init_idt();

  /* test soft interrupts */
  //asm volatile ("int $0x0"); // Div by zero interrupt
  //asm volatile("mov $0, %edx\n"
  //    "mov $0, %eax\n"
  //    "mov $0, %ecx\n"
  //    "div %ecx");

  /* test hard interrupts */

  /* test timer (PIT) */
  init_timer(50); // IRQ0
  /* test keyboard */
  init_keyboard(); // IRQ1

  PANIC("testing a panic");

  for (;;) {
    /* Halt CPU waiting for next interrupt */
    asm volatile("hlt");
  }
}
