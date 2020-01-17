#define _XOPEN_SOURCE 700

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
static uint_least8_t nnodes;
static uint32_t offset;

static initrd_node_t *nodes;

int8_t usage(char const * const filename) {
    fprintf(stderr, "Usage: %s [DIRECTORY]...\n", filename);
    return -1;
}

static int register_node(char const *filepath, struct stat const *info, int const typeflag, struct FTW *pathinfo) {
  static int nnode = 0;
  double const bytes = (double)info->st_size; /* Not exact if large! */

  if (
      typeflag != FTW_SLN && typeflag != FTW_DNR) {
    puts(filepath);
    memset(nodes[nnode].name, '\0', FS_FILENAME_SIZE);
    memcpy(nodes[nnode].name, filepath, FS_FILENAME_SIZE);
    nodes[nnode].size = bytes;
    nodes[nnode].type = typeflag;
    nodes[nnode].offset = offset;
    offset += bytes;
    ++nnode;
  }

  return 0;
}

static int traversal(char const * const dirpath) {
  /* Invalid directory path? */
  if (dirpath == NULL || *dirpath == '\0')
    return errno = EINVAL;

  int const result = nftw(dirpath, register_node, 15, FTW_PHYS);

  return result >= 0 ? result : errno;
}

static void write_nodes(void) {
  FILE *wfile = fopen(IMAGE, "w");
  fwrite(nodes, sizeof(initrd_node_t), nnodes, wfile);
  fclose(wfile);
}

static void write_files(void) {
  uint8_t *buf = NULL;
  FILE *infile = NULL;
  FILE *wfile = fopen(IMAGE, "w");

  for (uint_fast64_t i = 0 ; i < nnodes ; ++i) {
    buf = malloc(nodes[i].size);
    infile = fopen(nodes[i].name, "r");
    fread(buf, 1, nodes[i].size, infile);
    fwrite(buf, 1, nodes[i].size, wfile);
    fclose(infile);
    free(buf);
  }
  fclose(wfile);
}

int main(int ac, char **av) {
  if (ac < 2)
    return usage(av[0]);

  if (ac > 2)
    nnodes = atoi(av[2]);

  offset = sizeof(*nodes) * nnodes;

  if ((nodes = malloc(offset)) == NULL) {
    fprintf(stderr, "Cannot allocate sufficient memory for nodes\n");
    return EXIT_FAILURE;
  }

  if (traversal(av[1])) {
    fprintf(stderr, "%s.\n", strerror(errno));
    return EXIT_FAILURE;
  }

  write_nodes();

  write_files();

  free(nodes);

  return EXIT_SUCCESS;
}
