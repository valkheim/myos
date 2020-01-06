// https://github.com/Razbit/razos/blob/master/kernel/src/fs/vfs.c

#include <stddef.h> // NULL
#include <fs/vfs.h>

// VFS root
inode_t *fs_root = NULL;

uint32_t open_fs(inode_t *node) {
  if (node->open != NULL)
    return node->open(node);
  return 0;
}

uint32_t close_fs(inode_t *node) {
  if (node->close != NULL)
    return node->close(node);
  return 0;
}

uint32_t read_fs(inode_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
  if (node->read != NULL)
    return node->read(node, offset, size, buffer);
  return 0;
}

uint32_t write_fs(inode_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
  if (node->write != NULL)
    return node->write(node, offset, size, buffer);
  return 0;
}

dirent_t *readdir_fs(inode_t *node, uint32_t index) {
  if ((node->flags & 0x7) == FS_DIRECTORY && node->readdir != NULL)
      return node->readdir(node, index);
  return NULL;
}

inode_t *finddir_fs(inode_t *node, char *name) {
  if ((node->flags & 0x7) == FS_DIRECTORY && node->finddir != NULL)
      return node->finddir(node, name);
  return NULL;
}
