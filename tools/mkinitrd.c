// https://github.com/Razbit/razos/blob/master/tools/mk-initrd.c

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "../kernel/include/fs/vfs.h"
#include "../kernel/include/fs/initrd.h"


int8_t usage(char const * const filename) {
    fprintf(stderr, "Usage: %s [FILE]...\n", filename);
    return -1;
}

void get_nodes(char **av, uint32_t nfiles, uint32_t offset, initrd_node_t *nodes) {
  for (uint_fast32_t i = 0; i < nfiles; ++i) {
    printf("Writing node for file %s at 0x%X\n", av[i + 1], offset);
    strncpy(nodes[i].name, av[i + 1], FS_FILENAME_SIZE - 1);

    nodes[i].offset = offset;

    FILE *infile = fopen(av[i + 1], "r");
    fseek(infile, 0, SEEK_END);
    nodes[i].size = ftell(infile);
    offset += nodes[i].size;
    fclose(infile);

    printf("File \"%s\" is at 0x%X and is %d bytes long\n", \
        nodes[i].name, nodes[i].offset, nodes[i].size);
  }
}


void write_nodes(FILE *wfile, uint32_t nfiles, initrd_node_t *nodes) {
  fwrite(&nfiles, sizeof(nfiles), 1, wfile);
  fwrite(nodes, sizeof(initrd_node_t), nfiles, wfile);
}

void write_files(FILE *wfile, char **av, uint32_t nfiles, initrd_node_t *nodes) {
  /* Write the actual contents of the files */
  for (uint_fast32_t i = 0; i < nfiles; ++i) {
    uint8_t *buf = malloc(nodes[i].size);
    FILE *infile = fopen(av[i + 1], "r");
    fread(buf, 1, nodes[i].size, infile);
    fwrite(buf, 1, nodes[i].size, wfile);
    fclose(infile);
    free(buf);
  }
}

int main(int ac, char **av) {
  uint32_t const nfiles = ac - 1; // nodes to be written
  uint32_t offset = 0;            // nodes offset
  initrd_node_t *nodes;           // nodes

  if (ac < 1)
    return usage(av[0]);

  printf("%d files\n", ac - 1);

  // Compute offset
  offset = nfiles * sizeof(initrd_node_t);
  offset += sizeof(uint32_t);

  // Write nodes name, offset, size
  nodes = malloc(sizeof(*nodes) * nfiles);
  get_nodes(av, nfiles, offset, nodes);

  // Write image file

  FILE* wfile = fopen("initrd.img", "w");
  // Write nodes to the image
  write_nodes(wfile, nfiles, nodes);
  // Write files to the image
  write_files(wfile, av, nfiles, nodes);

  fclose(wfile);
  free(nodes);
}
