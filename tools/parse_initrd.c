/*
 * This tool is there to parse / validate the initrd.img generated with
 * the make_initrd utility
 */
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ftw.h>

#include "../kernel/include/fs/initrd.h"

static int8_t usage(char const * const filename) {
    fprintf(stderr, "Usage: %s [IMAGE] [NNODES]\n", filename);
    return -1;
}

static void print_node(initrd_node_t const * const node,
                       int const fd) {
  fprintf(stdout, "Filename: %s\n", node->name);
  fprintf(stdout, "Type: %d ", node->type);
  switch (node->type) {
    case FTW_F:
      puts("(regular file)");
      break;
    case FTW_D:
      puts("(directory)");
      break;
    default:
      puts("(unknow, see ftw.h)");
  }
  fprintf(stdout, "Size: %d\n", node->size);
  fprintf(stdout, "Offset: %x\n", node->offset);
  if (node->type == FTW_F) {
    char *contents = malloc(sizeof(*contents) * node->size);
    pread(fd, contents, node->size, node->offset);
    puts(contents);
    free(contents);
  }
}

static void parse(char * const image, uint8_t const nnodes) {
  int const fd = open(image, O_RDONLY);
  initrd_node_t node;

  for (uint_least8_t i = 0 ; i < nnodes ; ++i) {
    pread(fd, &node, sizeof(node), sizeof(node) * i);
    print_node(&node, fd);
  }
  close(fd);
}

int main(int ac, char **av) {
  if (ac < 2)
    return usage(av[0]);

  parse(av[1], (uint8_t)atoi(av[2]));
}
