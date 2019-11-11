#include <stdint.h>
#include <string.h> // memset
#include <stdio.h> // printf
#include <mm/malloc.h>
#include <kernel/kernel.h>

uint32_t last_alloc = 0;
uint32_t heap_end = 0;
uint32_t heap_begin = 0;
uint32_t pheap_begin = 0;
uint32_t pheap_end = 0;
uint8_t *pheap_desc = 0;
uint32_t memory_used = 0;

void* mymemset (void * ptr, int32_t value, int32_t num )
{
	int32_t* p=ptr;
	while(num--)
		*p++ = (int32_t)value;
	return ptr;
}

void init_mm(uint32_t *kernel_end)
{
/*
            +---------------------------+
            |                           |
            |                           |
            |                           |
            |                           |
0x003xxxxx  +---------------------------+ <-- heap end
            |                           |
    32 KiB {|           page 32         |
            |                           |
            +---------------------------+
            |                           |
    32 Kib {|            ...            |
            |                           |
            +---------------------------+
            |                           |
    32 KiB {|           page 1          |
            |                           |
0x001x1xxx  +---------------------------+ <-- heap begin
            |      1K skip for align    |
0x001x0xxx  +---------------------------+ <-- kernel end
            |                           |
            |        kernel map         |
            |   see linker.ld mapping   |
            |                           |
0x00100000  +---------------------------+ <-- kernel base
            |                           |
            |                           |
            |                           |
0x00000000  +---------------------------+
*/

  last_alloc = ((uint32_t)kernel_end + 0x1000); // Skip 1K
  heap_begin = last_alloc;
  pheap_end = 0x400000; // 4194304 bytes = 4096 Kib = 32 MiB
  pheap_begin = pheap_end - (MAX_PAGE_ALIGNED_ALLOCS * 4096); // 32 * 4096 = 131072 bytes = 128 KiB
  heap_end = pheap_begin;
  memset((char *)heap_begin, 0, heap_end - heap_begin); // Clear heap
  pheap_desc = (uint8_t *)malloc(MAX_PAGE_ALIGNED_ALLOCS);
  printf("Kernel heap starts at 0x%x\n", last_alloc);
  return;
}

void mm_status() {
	printf("=== Memory status:");
	printf("Memory used: %d bytes (memory_used)\n", memory_used);
	printf("Memory free: %d bytes (heap_end - heap_begin - memory_used)\n", heap_end - heap_begin - memory_used);
	printf("Heap size: %d bytes (heap_end - heap_begin)\n", heap_end - heap_begin);
	printf("Heap start: 0x%x (heap_begin)\n", heap_begin);
	printf("Heap end: 0x%x (heap_end)\n", heap_end);
	printf("Pheap start: 0x%x (pheap_begin)\n", pheap_begin);
  printf("Pheap end: 0x%x (pheap_end)\n", pheap_end);
}

void *malloc(uint32_t /*const*/ size) {
  if (!size) return 0;

  /* Loop through blocks and find a block sized the same or bigger */
  uint8_t *mem = (uint8_t *)heap_begin;

  while ((uint32_t)mem < last_alloc) {
    alloc_t * const current_block = (alloc_t *)mem;

    if (!current_block->size) /* No size means the end of allocation */
      break /* the block search */;

    if (current_block->status == ALLOCATED) {
      mem += current_block->size;
      mem += sizeof(alloc_t); // 8
      mem += sizeof(alloc_t *); // 4
      continue /* looking */;
    }

    /* If the is not allocated, and its size is bigger or equal to the
     * requested size, then adjust its size, set status and return the location.
     */
    if (current_block->size >= size) {
      current_block->status = ALLOCATED;
      printf("RE:Allocated %d bytes from 0x%x to 0x%x\n", size, mem + sizeof(alloc_t), mem + sizeof(alloc_t) + size);
      memset(mem + sizeof(alloc_t), 0, size);
      memory_used += size + sizeof(alloc_t);
      return (char *)(mem + sizeof(alloc_t));
    }

    /* If it isn't allocated, but the size is not good, then
     * add its size and the sizeof alloc_t to the pointer and
     * continue;
     */
    mem += current_block->size;
    mem += sizeof(alloc_t); // 8
    mem += sizeof(alloc_t *); // 4
  }

	if (last_alloc + size + sizeof(alloc_t) >= heap_end) {
		//set_task(0);
		printf("Cannot allocate %d bytes! Out of memory.\n", size);
		PANIC("");
	}

	alloc_t *alloc = (alloc_t *)last_alloc;
	alloc->status = ALLOCATED;
	alloc->size = size;

	uint32_t const used = size + sizeof(alloc_t) + sizeof(alloc_t *);
	last_alloc += used;
	memory_used += used;
	printf("Allocated %d bytes from 0x%x to 0x%x\n", size, (uint32_t)alloc + sizeof(alloc_t), last_alloc);
	memset(alloc + sizeof(alloc_t), 0, size); // Clear allocation
	return alloc + sizeof(alloc_t);
}

void free(void *mem) {
	alloc_t * const alloc = (mem - sizeof(alloc_t));
	memory_used -= alloc->size + sizeof(alloc_t);
	alloc->status = NOT_ALLOCATED;
}
