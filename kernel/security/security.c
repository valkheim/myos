#include <config.h>
#ifdef HAVE_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif
#ifdef HAVE_STDIO_H
# include <stdio.h>
#endif

#include <security/security.h>

static void __attribute__ ((constructor)) __guard_setup(void) {
  unsigned char *p;

  if (__stack_chk_guard != 0)
    return;

#ifdef HAVE_FCNTL
  int const fd = open("/dev/urandom", O_RDONLY);
  if (fd != -1) {
    ssize_t const size = read(fd, &__stack_chk_guard, sizeof (__stack_chk_guard));
    close (fd);
    if (size == sizeof(__stack_chk_guard) && __stack_chk_guard != 0)
      return;
  }
#endif
  /* terminator canary */
  p = (unsigned char *) &__stack_chk_guard;
  p[sizeof(__stack_chk_guard)-1] = 255;
  p[sizeof(__stack_chk_guard)-2] = '\n';
  p[0] = 0;
}

void __stack_chk_fail(void) {
  printf("Stack smashing detected");
  abort();
}
