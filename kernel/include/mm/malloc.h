#ifndef MALLOC_H
#define MALLOC_H

#include <stdint.h>

#define MAX_PAGE_ALIGNED_ALLOCS (32)
#define NOT_ALLOCATED (0)
#define ALLOCATED (1)

typedef struct {
  uint8_t status;
  uint32_t size;
} alloc_t;

void *malloc(uint32_t size);
void free(void *);

void init_mm(uint32_t *kernel_end);
void mm_status(void);

#endif

