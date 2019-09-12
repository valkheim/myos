#include <drivers/pc/keyboard.h>
#include <arch/i386/descriptors/idt.h> // PIC_(M|S)_(CTRL|DATA)
#include <arch/i386/isr.h> // registers_t
#include <arch/i386/irq.h> // IRQ1_KEYBOARD
#include <stdio.h>
#include <sys/io.h> // inb, outb
#include <stdbool.h>

bool is_capslock = false;
bool is_shift = false;

unsigned char keymap[128] = {
  0,  27, '1', '2', '3', '4', '5', '6', '7', '8',     /* 9 */
  '9', '0', '-', '=', '\b',     /* Backspace */
  '\t',                 /* Tab */
  'q', 'w', 'e', 'r',   /* 19 */
  't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', /* Enter key */
  0,                  /* 29   - Control */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',     /* 39 */
  '\'', '`',   0,                /* Left shift */
  '\\', 'z', 'x', 'c', 'v', 'b', 'n',                    /* 49 */
  'm', ',', '.', '/',   0,                              /* Right shift */
  '*',
  0,  /* Alt */
  ' ',  /* Space bar */
  0,  /* Caps lock */
  0,  /* 59 - F1 key ... > */
  0,   0,   0,   0,   0,   0,   0,   0,
  0,  /* < ... F10 */
  0,  /* 69 - Num lock*/
  0,  /* Scroll Lock */
  0,  /* Home key */
  0,  /* Up Arrow */
  0,  /* Page Up */
  '-',
  0,  /* Left Arrow */
  0,
  0,  /* Right Arrow */
  '+',
  0,  /* 79 - End key*/
  0,  /* Down Arrow */
  0,  /* Page Down */
  0,  /* Insert Key */
  0,  /* Delete Key */
  0,   0,   0,
  0,  /* F11 Key */
  0,  /* F12 Key */
  0,  /* All other keys are undefined */
};

static void keyboard_callback(__attribute__((unused)) registers_t r) {
  signed char const keycode = inb(0x60);

  /*
  if (keycode & 0x80) { // Key released
    printf("released\n");
    // Handle shift
  } else { // Key pressed
    printf("pressed\n");
    // Handle shift
    // Handle caps lock
    // Check if letter and capialize (?)
  }
  */
  /* Only print characters on keydown event that have
   * a non-zero mapping */
  if (keycode >= 0 && keymap[keycode]) {
    putchar(keymap[keycode]);
  }
}

void init_keyboard(void) {
  // Assume we have a PS/2 keyboard in a proper state

  // Register keyboard callback
  register_interrupt_handler(IRQ1_KEYBOARD, &keyboard_callback);

  /* Get current master PIC interrupt mask */
  unsigned char curmask_master = inb(PIC_M_DATA);

  /* 0xFD is 11111101 - enables only IRQ1 (keyboard) on master pic
     by clearing bit 1. bit is clear for enabled and bit is set for disabled */
  outb(PIC_M_DATA, curmask_master & 0xFD);
}
