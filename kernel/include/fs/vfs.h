#ifndef VFS_H_
#define VFS_H_

#include <stdint.h> // uint32_t, uint8_t

#define FS_FILENAME_SIZE (256)

#define FS_FILE        (1)
#define FS_DIRECTORY   (2)
#define FS_CHARDEVICE  (3)
#define FS_BLOCKDEVICE (4)
#define FS_PIPE        (5)
#define FS_SYMLINK     (6)
#define FS_MOUNTPOINT  (8) // Active mountpoint flag (8 is for OR'ing with FS_DIRECTORY)

struct inode_s;
typedef struct inode_s inode_t;

typedef uint32_t (*inode_open_t)(inode_t *);
typedef uint32_t (*inode_close_t)(inode_t *);
typedef uint32_t (*inode_read_t)(inode_t *, uint32_t, uint32_t, uint8_t *);
typedef uint32_t (*inode_write_t)(inode_t *, uint32_t, uint32_t, uint8_t *);
typedef struct dirent *(*inode_readdir_t)(inode_t *, uint32_t);
typedef inode_t *(*inode_finddir_t)(inode_t *, char *name);

typedef struct inode_s {
   char           name[FS_FILENAME_SIZE];  // The filename.
   uint32_t       mask;                    // The permissions mask.
   uint32_t       uid;                     // The owning user.
   uint32_t       gid;                     // The owning group.
   uint32_t       flags;                   // Includes the node type.
   uint32_t       inode;                   // This is device-specific - provides a way for a filesystem to identify files.
   uint32_t       length;                  // Size of the file, in bytes.
   uint32_t       impl;                    // An implementation-defined number.

   inode_open_t    open;
   inode_close_t   close;
   inode_read_t    read;
   inode_write_t   write;
   /*
    inode_create_t    create;
    inode_mkdir_t     mkdir;
    inode_ioctl_t     ioctl;
    inode_getsize_t   getsize;
    inode_chmod_t     chmod;
    inode_unlink_t    unlink;
    inode_symlink_t   symlink;
    inode_readlink_t  readlink;
    */

   inode_readdir_t readdir;
   inode_finddir_t finddir;

   struct inode_s *ptr;                  // Used by mountpoints and symlinks.
} inode_t;

typedef inode_t DIR; // POSIX alias

// POSIX directory entry (as in glibc's dirent.h)
typedef struct dirent {
  uint32_t  ino;                     // Inode number, required by POSIX
} dirent_t;

// VFS root
extern inode_t *fs_root;

uint32_t open_fs(inode_t *node);
uint32_t close_fs(inode_t *node);
uint32_t read_fs(inode_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t write_fs(inode_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);

dirent_t *readdir_fs(inode_t *node, uint32_t index);
inode_t *finddir_fs(inode_t *node, char *name);

#endif
