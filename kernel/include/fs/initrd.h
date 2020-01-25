#ifndef INITRD_H_
#define INITRD_H_

#include "vfs.h"

/*
type can be:
- FTW_F   (regular file)
- FTW_SL  (symlink)
- FTW_SLN (dangling symlink)
- FTW_D   (directory)
- FTW_DP  (directory)
- FTW_DNR (directory which cant be read)
see man 3 nftw, ftw.h
*/
typedef struct __attribute__((packed)) {
  int8_t    name[FS_FILENAME_SIZE];
  uint8_t   type;
  uint32_t  size; // intmax_t
  uint32_t  offset;
} initrd_node_t;

#endif
