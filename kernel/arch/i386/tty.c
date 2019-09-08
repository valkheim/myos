#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/tty.h>
#include <sys/io.h>

#include "vga.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

static inline uint8_t vga_entry_color(enum vga_color fg, enum vga_color bg) {
  return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
  return (uint16_t) uc | (uint16_t) color << 8;
}

static void move_cursor()
{
  uint16_t cursor = terminal_row * VGA_WIDTH + terminal_column;
  outb(0x3D4, 14); // Tell the VGA board we are setting the high cursor byte.
  outb(0x3D5, cursor >> 8); // Send the high cursor byte.
  outb(0x3D4, 15); // Tell the VGA board we are setting the low cursor byte.
  outb(0x3D5, cursor); // Send the low cursor byte.
}

static void scroll(void) {
  if (terminal_row >= VGA_HEIGHT)
  {
    size_t i;
    for (i = 0 * VGA_WIDTH; i < (VGA_HEIGHT - 1) * VGA_WIDTH; ++i)
      terminal_buffer[i] = terminal_buffer[i + 80];
    for (i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; ++i)
      terminal_buffer[i] = vga_entry(' ', terminal_color);
    terminal_row = VGA_HEIGHT - 1;
  }
}

void terminal_initialize(void) {
  terminal_row = 0;
  terminal_column = 0;
  terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
  terminal_buffer = VGA_MEMORY;
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = y * VGA_WIDTH + x;
      terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
  }
}

void terminal_setcolor(uint8_t color) {
  terminal_color = color;
}

void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
  const size_t index = y * VGA_WIDTH + x;
  terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c) {
  unsigned char uc = c;
  if (uc == 0x08) { // backspace
    --terminal_column;
  } else if (uc == 0x09) { // tab
    terminal_column = (terminal_column + 8) & ~(8 - 1);
  } else if (uc == '\r') { // carriage return
    terminal_column = 0;
  } else if (uc == '\n') { // newline
    ++terminal_row;
    terminal_column = 0;
  } else if (uc >= ' ') { // printable character
    terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);
    ++terminal_column;
  }

  if (terminal_column > VGA_WIDTH) {
    terminal_column = 0;
    ++terminal_row;
  }

  scroll();
  move_cursor();
}

void terminal_write(const char* data, size_t size) {
  for (size_t i = 0; i < size; i++)
    terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
  terminal_write(data, strlen(data));
}
