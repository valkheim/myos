// kheap.h -- Interface for kernel heap functions, also provides
//            a placement malloc() for use before the heap is 
//            initialised.
//            Written for JamesM's kernel development tutorials.

#ifndef KHEAP_H
#define KHEAP_H

#include "mm/ordered_array.h"

#define KHEAP_START         (0xC0000000)
#define KHEAP_INITIAL_SIZE  (0x100000) // 1048576 bytes, 1024 KiB, 1 MiB

#define HEAP_INDEX_SIZE (0x20000) // 131072 bytes, 128 KiB
#define HEAP_MIN_SIZE   (0x70000) // 458752 bytes, 448 KiB
#define HEAP_MAGIC      (0xDEADC0DE)

#define REQUEST_ALIGNMENT      (1)
#define DONT_REQUEST_ALIGNMENT (0)

typedef struct {
   uint32_t magic;  // Magic number, used for error checking and identification.
   uint8_t is_hole; // 1 if this is a hole. 0 if this is a block.
   uint32_t size;   // size of the block, including the end footer.
} header_t;

typedef struct {
   uint32_t magic;   // Magic number, same as in header_t.
   header_t *header; // Pointer to the block header.
} footer_t;

typedef struct {
   ordered_array_t index;
   uint32_t start_address; // The start of our allocated space.
   uint32_t end_address;   // The end of our allocated space. May be expanded up to max_address.
   uint32_t max_address;   // The maximum address the heap can be expanded to.
   uint8_t supervisor;     // Should extra pages requested by us be mapped as supervisor-only?
   uint8_t readonly;       // Should extra pages requested by us be mapped as read-only?
} heap_t;

// Creates a heap
heap_t *create_heap(uint32_t start, uint32_t const end_addr, uint32_t const max, uint8_t const supervisor, uint8_t const readonly);

// Allocates a contiguous region of memory 'size' in size.
void *alloc(uint32_t size, uint8_t const page_align, heap_t * const heap);

// General allocation function.
uint32_t kmalloc(uint32_t const size);
// Allocate a page-aligned `size` bytes chunk of memory
uint32_t kmalloc_a(uint32_t const size);
// Allocate a `size` bytes chunk of memory. Chunk physical address is written to `physical_address`
uint32_t kmalloc_p(uint32_t const size, uint32_t * const physical_address);
// Allocate a page-aligned `size` bytes chunk of memory. Chunk physical address is written to `physical_address`
uint32_t kmalloc_ap(uint32_t const size, uint32_t * const physical_address);

// General deallocation function.
void kfree(void *p);

#endif // KHEAP_H