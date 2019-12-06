#ifndef PAGING_H_
#define PAGING_H_

#include <arch/i386/isr.h> // registers_t

#define MAKE_PAGE (1)
#define PAGE_SIZE (0x1000) // 4096 bytes, `getconf PAGE_SIZE`

typedef struct page {
   uint32_t present:  1;  // Page present in memory
   uint32_t rw:       1;  // Read-only if clear, readwrite if set
   uint32_t user:     1;  // Supervisor level only if clear
   uint32_t accessed: 1;  // Has the page been accessed since last refresh?
   uint32_t dirty:    1;  // Has the page been written to since last refresh?
   uint32_t unused:   7;  // Amalgamation of unused and reserved bits
   uint32_t frame:    20; // Frame address (shifted right 12 bits)
} page_t;

#define N_PAGES         (1024)
#define N_PAGE_TABLES   (1024)

typedef struct page_table {
   page_t pages[N_PAGES];
} page_table_t;

typedef struct page_directory {
  // Pointers to page tables
  page_table_t *tables[N_PAGE_TABLES];
  // Pointers the the above page tables giving the physical location
  // for loading into the CR3 register
  uint32_t tablesPhysical[N_PAGE_TABLES];
  // Physical address of tablePhysical. This comes into play
  // when we get our kernel heap allocated and the directory
  // may be in a different location in virtual memory.
   uint32_t physicalAddr;
} page_directory_t;

// Macros used in the bitset algorithms.
#define INDEX_FROM_BIT(a) (a / (8 * 4))
#define OFFSET_FROM_BIT(a) (a % (8 * 4))

// Setup page directories, enable paging, ...
void initialise_paging();

// Returns a pointer to the to the page entry for a particular address.
// if make is MAKE_PAGE (1): create page if required
page_t *get_page(uint32_t const address, int const make, page_directory_t * const dir);

/**
   Handler for page faults.
**/
void page_fault(registers_t *regs);

/**
   Makes a copy of a page directory.
**/
page_directory_t *clone_directory(page_directory_t *src);

#endif