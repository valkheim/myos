#ifndef INITRD_H_
#define INITRD_H_

#include "vfs.h"

typedef struct {
  char      name[FS_FILENAME_SIZE];
  uint32_t  offset;
  uint32_t  size;
} initrd_node_t;

#endif
