/*
 * This is a tool for building a filesystem image disk. Given two arguments:
 * - target directory
 * - number of files
 * it can build an initrd.img file containing contents and meta information
 * about those file so we can use them latter while setting up the file system
 */

#define _XOPEN_SOURCE (700)

#include <stdlib.h>
#include <unistd.h>
#include <ftw.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "../kernel/include/fs/vfs.h"
#include "../kernel/include/fs/initrd.h"

#define IMAGE ("initrd.img")

// Ok let's assume we'll discover and write nnodes nodes...
// You can override it using $(find . | wc -l) as av[2]
static uint8_t nnodes;
static uint32_t offset;

static initrd_node_t *nodes;

static int8_t usage(char const * const filename) {
    fprintf(stderr, "Usage: %s [DIRECTORY] [NNODES]\n", filename);
    return -1;
}

static int register_node(char const *filepath, struct stat const *info,
                         int const typeflag,
                         struct FTW * __attribute__((unused)) pathinfo) {
  static int nnode = 0;
  intmax_t const bytes = info->st_size;

  if (typeflag == FTW_F || typeflag == FTW_D) {
    fprintf(stdout, "Register node %s\n", filepath);
    memset(nodes[nnode].name, '\0', FS_FILENAME_SIZE);
    memcpy(nodes[nnode].name, filepath, FS_FILENAME_SIZE);
    nodes[nnode].size = bytes;
    nodes[nnode].type = typeflag;
    if (typeflag == FTW_F) {
      nodes[nnode].offset = offset;
      fprintf(stdout, "\t node contents offset at 0x%x\n", offset);
      offset += bytes;
    } else {
      nodes[nnode].offset = 0;
    }
    ++nnode;
  }

  return 0;
}

static int traversal(char const * const dirpath) {
  if (dirpath == NULL || *dirpath == '\0')
    return errno = EINVAL;

  int const result = nftw(dirpath, register_node, 15, FTW_PHYS);

  return result >= 0 ? result : errno;
}

static void write_nodes(void) {
  uint8_t *buf = NULL;
  FILE *infile = NULL;
  FILE *wfile = fopen(IMAGE, "w");

  // Write nodes number
  //fwrite(&nnodes, sizeof(nnodes), 1, wfile);
  // Write header
  fwrite(nodes, sizeof(initrd_node_t), nnodes, wfile);

  // Write files
  for (uint_fast64_t i = 0 ; i < nnodes ; ++i) {
    if (nodes[i].type == FTW_F) {
      buf = malloc(nodes[i].size);
      infile = fopen(nodes[i].name, "r");
      fread(buf, 1, nodes[i].size, infile);
      fwrite(buf, 1, nodes[i].size, wfile);
      fclose(infile);
      free(buf);
    }
  }
  fclose(wfile);
}

int main(int ac, char **av) {
  if (ac < 2)
    return usage(av[0]);

  if (ac > 2)
    nnodes = atoi(av[2]);

  fprintf(stdout, "Found %d nodes\n", nnodes);

  offset = sizeof(*nodes) * nnodes;

  fprintf(stdout, "Header size 0x%x\n", offset);

  if ((nodes = malloc(offset)) == NULL) {
    fprintf(stderr, "Cannot allocate sufficient memory for nodes\n");
    return EXIT_FAILURE;
  }

  if (traversal(av[1])) {
    fprintf(stderr, "%s.\n", strerror(errno));
    return EXIT_FAILURE;
  }

  write_nodes();

  free(nodes);

  return EXIT_SUCCESS;
}
