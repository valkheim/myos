// paging.c -- Defines the interface for and structures relating to paging.
//             Written for JamesM's kernel development tutorials.

#include <stdio.h>
#include <string.h> // memset
#include <mm/paging.h>
#include <mm/kheap.h>
#include <arch/i386/irq.h>

// The kernel's page directory (consistent between tasks)
page_directory_t *kernel_directory = 0;

// The current page directory
page_directory_t *current_directory = 0;

// Defined in kheap.c
extern uint32_t placement_address;
extern heap_t *kheap;

extern uint32_t *copy_page_physical(uint32_t src, uint32_t dst);

// A bitset of frames - used or free.
uint32_t *frames;
uint32_t nframes;

// Static function to set a bit in the frames bitset
static void set_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] |= (0x1 << off);
}

// Static function to clear a bit in the frames bitset
static void clear_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    uint32_t idx = INDEX_FROM_BIT(frame);
    uint32_t off = OFFSET_FROM_BIT(frame);
    frames[idx] &= ~(0x1 << off);
}

// Static function to find the first free frame.
static uint32_t first_frame(void) {
    for (uint32_t i = 0; i < INDEX_FROM_BIT(nframes); ++i) {
        if (frames[i] != 0xFFFFFFFF) { // nothing free, exit early.
            // at least one bit is free here.
            for (uint32_t j = 0; j < 32; ++j) {
                if (!(frames[i] & (0x1 << j))) {
                    return i * 4 * 8 + j;
                }
            }
        }
    }
    PANIC("cannot find first frame");
    return 0;
}

void alloc_frame(page_t * const page, int8_t const is_kernel, int8_t const is_writeable) {
    if (page->frame == 0 ) {
        uint32_t idx = first_frame();
        if (idx == (uint32_t) -1) { // uint32 by 1 underflow -> u32int max value
            PANIC("No free frames");
        }
        set_frame(idx * PAGE_SIZE);
        page->present = 1;
        page->rw = (is_writeable == 1) ? 1 : 0;
        page->user = (is_kernel == 1) ? 0 : 1;
        page->frame = idx;
    }
}

void free_frame(page_t * const page) {
    uint32_t frame;
    if (!(frame = page->frame)) {
        return;
    } else {
        clear_frame(frame);
        page->frame = 0x0;
    }
}

page_t *get_page(uint32_t const address, int8_t const make, page_directory_t * const dir) {
    // Generate page index from address
    uint32_t const index = address / PAGE_SIZE;
    // Find the page table containing this address.
    uint32_t const table_idx = index / N_PAGES;

    if (dir->tables[table_idx]) { // If this table is already assigned
        return &dir->tables[table_idx]->pages[index % N_PAGES];
    } else if (make) { // Create page
        uint32_t tmp;

        // Aligned alloc page table
        dir->tables[table_idx] = (page_table_t *)kmalloc_ap(sizeof(page_table_t), &tmp);
        // Ensure page table is clean
        memset(dir->tables[table_idx], 0, PAGE_SIZE);
        // Set default flags
        dir->tablesPhysical[table_idx] = tmp | 0x07; // PRESENT, RW, US.
        // Return new page address
        return &dir->tables[table_idx]->pages[index % N_PAGES];
    } else {
        return 0;
    }
}

static page_table_t *clone_table(page_table_t *src, uint32_t *physAddr) {
    // Make a new page table, which is page aligned.
    page_table_t *table = (page_table_t*)kmalloc_ap(sizeof(page_table_t), physAddr);

    // Ensure that the new table is blank.
    memset(table, 0, sizeof(page_directory_t));

    // For every entry in the table...
    for (uint16_t i = 0; i < N_PAGES; ++i) {
        // If the source entry has a frame associated with it...
        if (!src->pages[i].frame)
            continue;
        // Get a new frame.
        alloc_frame(&table->pages[i], 0, 0);
        // Clone the flags from source to destination.
        if (src->pages[i].present)
            table->pages[i].present = 1;
        if (src->pages[i].rw)
            table->pages[i].rw = 1;
        if (src->pages[i].user)
            table->pages[i].user = 1;
        if (src->pages[i].accessed)
            table->pages[i].accessed = 1;
        if (src->pages[i].dirty)
            table->pages[i].dirty = 1;
        // Physically copy the data across (process.S)
        copy_page_physical(src->pages[i].frame * PAGE_SIZE, table->pages[i].frame * PAGE_SIZE);
    }
    return table;
}

page_directory_t *clone_directory(page_directory_t *src) {
    uint32_t physical_address;
    // Make a new page directory and obtain its physical address.
    page_directory_t *dir = (page_directory_t*)kmalloc_ap(sizeof(page_directory_t), &physical_address);
    // Ensure new page directory is clean
    memset(dir, 0, sizeof(page_directory_t));

    // Get the offset of tablesPhysical from the start of the page_directory_t structure.
    uint32_t const offset = (uint32_t)dir->tablesPhysical - (uint32_t)dir;

    // Then the physical address of dir->tablesPhysical is:
    dir->physicalAddr = (uint32_t *)(physical_address + offset);

    // Go through each page table. If the page table is in the kernel directory, do not make a new copy.
    for (uint16_t i = 0; i < N_PAGE_TABLES; ++i) {
        if (!src->tables[i])
            continue;

        if (kernel_directory->tables[i] == src->tables[i]) {
            // It's in the kernel, so just use the same pointer.
            dir->tables[i] = src->tables[i];
            dir->tablesPhysical[i] = src->tablesPhysical[i];
        } else {
            // Copy the table.
            uint32_t phys;
            dir->tables[i] = clone_table(src->tables[i], &phys);
            dir->tablesPhysical[i] = phys | 0x07; // PRESENT, RW, US.
        }
    }
    return dir;
}

// Load the page directory into the CR3 register
static void switch_page_directory(page_directory_t * const dir) {
    uint32_t cr0;

    current_directory = dir;
    asm volatile("mov %0, %%cr3":: "r"(dir->physicalAddr));
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging by OR-ing min int32 value
    asm volatile("mov %0, %%cr0":: "r"(cr0));
}

static void pagefault_callback(registers_t regs) {
    // A page fault has occurred.
    // The faulting address is stored in the CR2 register.
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r" (faulting_address));
    
    // The error code gives us details of what happened.
    int32_t present = !(regs.err_code & 0x1); // Page not present
    int32_t rw = regs.err_code & 0x2;           // Write operation?
    int32_t us = regs.err_code & 0x4;           // Processor was in user-mode?
    int32_t reserved = regs.err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
    int32_t id = regs.err_code & 0x10;          // Caused by an instruction fetch?

    // Output an error message.
    printf("Page fault! ( ");
    if (present)
      printf("present ");
    if (rw)
      printf("read-only ");
    if (us)
      printf("user-mode ");
    if (reserved)
      printf("reserved ");
    printf("id: %d", id);
    printf(") at 0x%x - EIP: %x\n", faulting_address, regs.eip);
    PANIC("Page fault");
}

void initialise_paging(void) {
    // Size of physical memory.
    uint32_t mem_end_page = 0x1000000; // 16777216 bytes or 16 MiB
    
    nframes = mem_end_page / PAGE_SIZE;
    frames = (uint32_t *)kmalloc(INDEX_FROM_BIT(nframes));
    memset(frames, 0, INDEX_FROM_BIT(nframes));
    
    // Let's make a page directory.
    kernel_directory = (page_directory_t *)kmalloc_a(sizeof(page_directory_t));
    memset(kernel_directory, 0, sizeof(page_directory_t));
    kernel_directory->physicalAddr = kernel_directory->tablesPhysical;

    // Map some pages in the kernel heap area.
    // Here we call get_page but not alloc_frame. This causes page_table_t's 
    // to be created where necessary. We can't allocate frames yet because they
    // they need to be identity mapped first below, and yet we can't increase
    // placement_address between identity mapping and enabling the heap!
    uint32_t location = KHEAP_START;
    for (location = KHEAP_START ; location < KHEAP_START + KHEAP_INITIAL_SIZE; location += PAGE_SIZE)
        get_page(location, MAKE_PAGE, kernel_directory);

    // We need to identity map (phys addr = virt addr) from
    // 0x0 to the end of used memory, so we can access this
    // transparently, as if paging wasn't enabled.
    location = 0x0;
    while (location < placement_address + PAGE_SIZE) { // 0x400000)
        // Kernel code is readable but not writeable from userspace.
        alloc_frame(get_page(location, MAKE_PAGE, kernel_directory), 0, 0);
        location += PAGE_SIZE;
    }

    // We can now allocate the mapped pages
    for (location = KHEAP_START; location < KHEAP_START + KHEAP_INITIAL_SIZE; location += PAGE_SIZE)
        alloc_frame(get_page(location, MAKE_PAGE, kernel_directory), 0, 0);

    // Register page fault handler before enabling paging
    register_interrupt_handler(ISR14_PAGE_FAULT, &pagefault_callback);

    // Enable paging
    switch_page_directory(kernel_directory);
    // Initialise the kernel heap
    kheap = create_heap(KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE, 0xCFFFF000, 0, 0);

    // Make a copy of kernel directory
    current_directory = clone_directory(kernel_directory);
    switch_page_directory(current_directory);
}