#include <stdio.h>
#include <mm/kheap.h>
#include <mm/paging.h>

extern uint32_t kernel_end;
uint32_t placement_address = (uint32_t)&kernel_end; // truncation?

extern page_directory_t *kernel_directory;

heap_t *kheap = 0;

// Allocate a chunk of `size` memory bytes.
// If alignment is requested (align == REQUEST_ALIGNMENT), the chunk must be page aligned
// If `physical_address` != 0, the physical location of allocated chunk will be placed into it
static uint32_t kmalloc_int(uint32_t const size, int const align, uint32_t * const physical_address) {
    if (kheap != 0) {
        void const * const addr = alloc(size, (uint8_t)align, kheap);
        if (physical_address != 0) {
            page_t *page = get_page((uint32_t)addr, 0, kernel_directory);
            *physical_address = page->frame * PAGE_SIZE + ((uint32_t)addr & 0xFFF);
        }
        return (uint32_t)addr;
    } else {
        if (align == REQUEST_ALIGNMENT && (placement_address & 0xFFFFF000)) {
            // Align the placement address;
            placement_address &= 0xFFFFF000;
            placement_address += PAGE_SIZE;
        }
        if (physical_address) {
            *physical_address = placement_address;
        }
        uint32_t tmp = placement_address;
        placement_address += size;
        return tmp;
    }
}

uint32_t kmalloc_a(uint32_t const size) {
    return kmalloc_int(size, REQUEST_ALIGNMENT, NULL);
}

uint32_t kmalloc_p(uint32_t const size, uint32_t * const physical_address) {
    return kmalloc_int(size, DONT_REQUEST_ALIGNMENT, physical_address);
}

uint32_t kmalloc_ap(uint32_t const size, uint32_t * const physical_address) {
    return kmalloc_int(size, REQUEST_ALIGNMENT, physical_address);
}

uint32_t kmalloc(uint32_t const size) {
    return kmalloc_int(size, DONT_REQUEST_ALIGNMENT, NULL);
}

static void expand(uint32_t new_size, heap_t * const heap) {
    // Sanity check.
    ASSERT(new_size > heap->end_address - heap->start_address);
    // Get the nearest following page boundary.
    if ((new_size & 0xFFFFF000) != 0) {
        new_size &= 0xFFFFF000;
        new_size += PAGE_SIZE;
    }

    // Make sure we are not overreaching ourselves.
    ASSERT(heap->start_address + new_size <= heap->max_address);

    // This should always be on a page boundary.
    uint32_t const old_size = heap->end_address-heap->start_address;

    uint32_t i = old_size;
    while (i < new_size) {
        int8_t const is_kernel = (heap->supervisor) ? 1 : 0;
        int8_t const is_writeable = (heap->readonly) ? 0 : 1;
        alloc_frame(get_page(heap->start_address + i, MAKE_PAGE, kernel_directory), is_kernel, is_writeable);
        i += PAGE_SIZE;
    }
    heap->end_address = heap->start_address+new_size;
}

static uint32_t contract(uint32_t new_size, heap_t * const heap) {
    // Sanity check.
    ASSERT(new_size < heap->end_address-heap->start_address);

    // Get the nearest following page boundary.
    if (new_size & PAGE_SIZE) {
        new_size &= PAGE_SIZE;
        new_size += PAGE_SIZE;
    }

    // Don't contract too far!
    if (new_size < HEAP_MIN_SIZE)
        new_size = HEAP_MIN_SIZE;

    uint32_t const old_size = heap->end_address-heap->start_address;
    uint32_t i = old_size - PAGE_SIZE;
    while (new_size < i) {
        free_frame(get_page(heap->start_address+i, 0, kernel_directory));
        i -= PAGE_SIZE;
    }

    heap->end_address = heap->start_address + new_size;
    return new_size;
}

static uint32_t find_smallest_hole(uint32_t const size, uint8_t const page_align, heap_t * const heap) {
    // Find the smallest hole that will fit.
    uint32_t iterator = 0;
    while (iterator < heap->index.size) {
        header_t const * const header = (header_t *)lookup_ordered_array(iterator, &heap->index);
        // If the user has requested the memory be page-aligned
        if (page_align == REQUEST_ALIGNMENT) {
            // Page-align the starting point of this header.
            uint32_t location = (uint32_t)header;
            int32_t offset = 0;
            if (((location + sizeof(header_t)) & 0xFFFFF000) != 0)
                offset = PAGE_SIZE - (location+sizeof(header_t)) % PAGE_SIZE;
            int32_t hole_size = (int32_t)header->size - offset;
            // Can we fit now?
            if (hole_size >= (int32_t)size)
                break;
        }
        else if (header->size >= size) {
            break;
        }
        iterator++;
    }
    // Why did the loop exit?
    if (iterator == heap->index.size) {
        return -1;
    } else {
        return iterator;
    }
}

static int8_t header_t_less_than(void * const a, void * const b) {
    return (((header_t*)a)->size < ((header_t*)b)->size) ? 1 : 0;
}

heap_t *create_heap(uint32_t start, uint32_t const end_addr, uint32_t const max, uint8_t const supervisor, uint8_t const readonly) {
    heap_t * const heap = (heap_t*)kmalloc(sizeof(heap_t));

    // All our assumptions are made on startAddress and endAddress being page-aligned.
    ASSERT(start % PAGE_SIZE == 0);
    ASSERT(end_addr % PAGE_SIZE == 0);
    
    // Initialise the index.
    heap->index = place_ordered_array( (void*)start, HEAP_INDEX_SIZE, &header_t_less_than);
    
    // Shift the start address forward to resemble where we can start putting data.
    start += sizeof(type_t)*HEAP_INDEX_SIZE;

    // Make sure the start address is page-aligned.
    if ((start & 0xFFFFF000) != 0) {
        start &= 0xFFFFF000;
        start += PAGE_SIZE;
    }
    // Write the start, end and max addresses into the heap structure.
    heap->start_address = start;
    heap->end_address = end_addr;
    heap->max_address = max;
    heap->supervisor = supervisor;
    heap->readonly = readonly;

    // We start off with one large hole in the index.
    header_t * const hole = (header_t *)start;
    hole->size = end_addr-start;
    hole->magic = HEAP_MAGIC;
    hole->is_hole = 1;
    insert_ordered_array((void *)hole, &heap->index);     

    return heap;
}

void *alloc(uint32_t size, uint8_t const page_align, heap_t * const heap) {
    // Make sure we take the size of header/footer into account.
    uint32_t new_size = size + sizeof(header_t) + sizeof(footer_t);
    // Find the smallest hole that will fit.
    uint32_t iterator = find_smallest_hole(new_size, page_align, heap);

    if (iterator == (uint32_t) -1) { // If we didn't find a suitable hole (uint32 by 1 underflow -> u32int max value)
        // Save some previous data.
        uint32_t const old_length = heap->end_address - heap->start_address;
        uint32_t const old_end_address = heap->end_address;

        // We need to allocate some more space.
        expand(old_length + new_size, heap);
        uint32_t const new_length = heap->end_address-heap->start_address;

        // Find the endmost header. (Not endmost in size, but in location).
        iterator = 0;
        // Vars to hold the index of, and value of, the endmost header found so far.
        uint32_t idx = (uint32_t) -1; // uint32 by 1 underflow -> u32int max value
        uint32_t value = 0x0;
        while (iterator < heap->index.size) {
            uint32_t tmp = (uint32_t)lookup_ordered_array(iterator, &heap->index);
            if (tmp > value) {
                value = tmp;
                idx = iterator;
            }
            iterator++;
        }

        // If we didn't find ANY headers, we need to add one.
        if (idx == (uint32_t) -1) { // uint32 by 1 underflow -> u32int max value
            header_t *header = (header_t *)old_end_address;
            header->magic = HEAP_MAGIC;
            header->size = new_length - old_length;
            header->is_hole = 1;
            footer_t *footer = (footer_t *)(old_end_address + header->size - sizeof(footer_t));
            footer->magic = HEAP_MAGIC;
            footer->header = header;
            insert_ordered_array((void*)header, &heap->index);
        } else {
            // The last header needs adjusting.
            header_t *header = lookup_ordered_array(idx, &heap->index);
            header->size += new_length - old_length;
            // Rewrite the footer.
            footer_t *footer = (footer_t *)((uint32_t)header + header->size - sizeof(footer_t));
            footer->header = header;
            footer->magic = HEAP_MAGIC;
        }
        // We now have enough space. Recurse, and call the function again.
        return alloc(size, page_align, heap);
    }

    header_t const * const orig_hole_header = (header_t *)lookup_ordered_array(iterator, &heap->index);
    uint32_t orig_hole_pos = (uint32_t)orig_hole_header;
    uint32_t orig_hole_size = orig_hole_header->size;
    // Here we work out if we should split the hole we found into two parts.
    // Is the original hole size - requested hole size less than the overhead for adding a new hole?
    if (orig_hole_size-new_size < sizeof(header_t) + sizeof(footer_t)) {
        // Then just increase the requested size to the size of the hole we found.
        size += orig_hole_size-new_size;
        new_size = orig_hole_size;
    }

    // If we need to page-align the data, do it now and make a new hole in front of our block.
    if (page_align && orig_hole_pos & 0xFFFFF000) {
        uint32_t new_location   = orig_hole_pos + PAGE_SIZE - (orig_hole_pos & 0xFFF) - sizeof(header_t);
        header_t *hole_header = (header_t *)orig_hole_pos;
        hole_header->size     = PAGE_SIZE - (orig_hole_pos & 0xFFF) - sizeof(header_t);
        hole_header->magic    = HEAP_MAGIC;
        hole_header->is_hole  = 1;
        footer_t *hole_footer = (footer_t *) ( (uint32_t)new_location - sizeof(footer_t) );
        hole_footer->magic    = HEAP_MAGIC;
        hole_footer->header   = hole_header;
        orig_hole_pos         = new_location;
        orig_hole_size        = orig_hole_size - hole_header->size;
    } else {
        // Else we don't need this hole any more, delete it from the index.
        remove_ordered_array(iterator, &heap->index);
    }

    // Overwrite the original header...
    header_t * const block_header  = (header_t *)orig_hole_pos;
    block_header->magic     = HEAP_MAGIC;
    block_header->is_hole   = 0;
    block_header->size      = new_size;
    // ...And the footer
    footer_t * const block_footer  = (footer_t *)(orig_hole_pos + sizeof(header_t) + size);
    block_footer->magic     = HEAP_MAGIC;
    block_footer->header    = block_header;

    // We may need to write a new hole after the allocated block.
    // We do this only if the new hole would have positive size...
    if (orig_hole_size - new_size > 0) {
        header_t * const hole_header = (header_t *)(orig_hole_pos + sizeof(header_t) + size + sizeof(footer_t));
        hole_header->magic    = HEAP_MAGIC;
        hole_header->is_hole  = 1;
        hole_header->size     = orig_hole_size - new_size;
        footer_t * const hole_footer = (footer_t *)((uint32_t)hole_header + orig_hole_size - new_size - sizeof(footer_t));
        if ((uint32_t)hole_footer < heap->end_address) {
            hole_footer->magic = HEAP_MAGIC;
            hole_footer->header = hole_header;
        }
        // Put the new hole in the index;
        insert_ordered_array((void*)hole_header, &heap->index);
    }
    
    // ...And we're done!
    return (void *)((uint32_t)block_header + sizeof(header_t));
}

// Releases a block allocated with alloc()
static void alloc_free(void const * const p, heap_t * const heap) {
    // Exit gracefully for null pointers.
    if (p == 0)
        return;

    // Get the header and footer associated with this pointer.
    header_t *header = (header_t *)((uint32_t)p - sizeof(header_t));
    footer_t *footer = (footer_t *)((uint32_t)header + header->size - sizeof(footer_t));

    // Sanity checks.
    ASSERT(header->magic == HEAP_MAGIC);
    ASSERT(footer->magic == HEAP_MAGIC);

    // Make us a hole.
    header->is_hole = 1;

    // Do we want to add this header into the 'free holes' index?
    char do_add = 1;

    // Unify left
    // If the thing immediately to the left of us is a footer...
    footer_t *test_footer = (footer_t *)((uint32_t)header - sizeof(footer_t));
    if (test_footer->magic == HEAP_MAGIC && test_footer->header->is_hole == 1) {
        uint32_t cache_size = header->size; // Cache our current size.
        header = test_footer->header;     // Rewrite our header with the new one.
        footer->header = header;          // Rewrite our footer to point to the new header.
        header->size += cache_size;       // Change the size.
        do_add = 0;                       // Since this header is already in the index, we don't want to add it again.
    }

    // Unify right
    // If the thing immediately to the right of us is a header...
    header_t *test_header = (header_t *)((uint32_t)footer + sizeof(footer_t));
    if (test_header->magic == HEAP_MAGIC && test_header->is_hole) {
        header->size += test_header->size; // Increase our size.
        // Rewrite it's footer to point to our header.
        test_footer = (footer_t*)((uint32_t)test_header + test_header->size - sizeof(footer_t));
        footer = test_footer;
        // Find and remove this header from the index.
        uint32_t iterator = 0;
        while ((iterator < heap->index.size) && (lookup_ordered_array(iterator, &heap->index) != (void*)test_header))
            iterator++;
        // Make sure we actually found the item.
        ASSERT(iterator < heap->index.size);
        // Remove it.
        remove_ordered_array(iterator, &heap->index);
    }

    // If the footer location is the end address, we can contract.
    if ( (uint32_t)footer+sizeof(footer_t) == heap->end_address) {
        uint32_t const old_length = heap->end_address-heap->start_address;
        uint32_t const new_length = contract( (uint32_t)header - heap->start_address, heap);
        // Check how big we will be after resizing.
        if (header->size - (old_length - new_length) > 0) {
            // We will still exist, so resize us.
            header->size -= old_length - new_length;
            footer = (footer_t *)((uint32_t)header + header->size - sizeof(footer_t));
            footer->magic = HEAP_MAGIC;
            footer->header = header;
        } else {
            // We will no longer exist :(. Remove us from the index.
            uint32_t iterator = 0;
            while ((iterator < heap->index.size) && (lookup_ordered_array(iterator, &heap->index) != (void*)test_header))
                iterator++;
            // If we didn't find ourselves, we have nothing to remove.
            if (iterator < heap->index.size)
                remove_ordered_array(iterator, &heap->index);
        }
    }

    // If required, add us to the index.
    if (do_add == 1)
        insert_ordered_array((void *)header, &heap->index);
}

void kfree(void * const p) {
    alloc_free(p, kheap);
}
