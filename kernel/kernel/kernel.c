#include <arch/i386/descriptors/gdt.h>
#include <arch/i386/descriptors/idt.h>
#include <kernel/kernel.h>
#include <kernel/terminal.h>
#include <mm/malloc.h>
#include <mm/paging.h>
#include <mm/kheap.h>
#include <drivers/pc/pit.h>
#include <drivers/pc/keyboard.h>
#include <stdio.h>

extern uint32_t kernel_base;
extern uint32_t kernel_end;

void kernel_main(void) {

  init_terminal();

  printf("Hello kernel!\n");
  //printf("Kernel base is 0x%x, end is 0x%x\n", &kernel_base, &kernel_end);

  init_gdt();
  init_idt();

  /* test soft interrupts */
  //asm volatile ("int $0x0"); // ISR0_DIVISION_BY_ZERO
  /*
  asm volatile("mov $0, %edx\n"
      "mov $0, %eax\n"
      "mov $0, %ecx\n"
      "div %ecx");
  */

  /* test hard interrupts */

  /* test timer (PIT) */
  init_timer(50); // IRQ0
  /* test keyboard */
  init_keyboard(); // IRQ1

  printf("kernel end is 0x%x\n", &kernel_end);

  // PANIC("testing a panic");

  uint32_t a = kmalloc(8); // Leaked after paging initalisation
  printf("a=%x\n", a);
  initialise_paging();
  uint32_t b0 = kmalloc(8);
  printf("b0=%x\n", b0);
  kfree((void *)b0);
  uint32_t b1 = kmalloc(8);
  printf("b0=%x\n", b0);

  uint32_t c = kmalloc(8);
  printf("c=%x\n", c);
  kfree((void *)c);
  kfree((void *)b1);

  // Page fault not working ?! Have a segmentation fault on host (kind of page fault anyway?)
  uint32_t *ptr = (uint32_t*)0x0;
  uint32_t __attribute__((unused)) do_page_fault = *ptr; // force a page fault by reading location 0xA0000000

  uint32_t d = kmalloc(12);
  printf("d=%x\n", d);
  kfree((void *)d);
  
  for (;;) {
    /* Halt CPU waiting for next interrupt */
    asm volatile("hlt");
  }
}
