#include <stdint.h>

#include <config.h>
#ifdef HAVE_SYS_IO_H
# include <sys/io.h>
#endif

#if defined(__is_libk)
// Write byte value to port
void outb(uint16_t port, uint8_t value)
{
  asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

// Read byte from port
uint8_t inb(uint16_t port)
{
  uint8_t ret;
  asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
  return ret;
}

// Read word from port
uint16_t inw(uint16_t port)
{
  uint16_t ret;
  asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
  return ret;
}
#endif
