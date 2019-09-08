#ifndef _SYS_IO_H
#define _SYS_IO_H 1

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);

#endif
