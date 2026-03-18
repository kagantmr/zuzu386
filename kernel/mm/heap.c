#include "heap.h"
#include <stdbool.h>
#include "mem.h"
#include "core/panic.h"
#include "arch/x86/symbols.h"

#define HEAP_START ((uint32_t)&_heap_start)
#define HEAP_END   (HEAP_START + 0x400000)  /* 4MB heap */

block_header_t *heap_head = NULL;

void* kmalloc(size_t size) {
    if (!size) {
        return NULL;
    }
    size_t req = align_up(size, ALIGNMENT); // align area up
    block_header_t* current_block = heap_head;
    while (current_block) {
        if (current_block->free && current_block->size >= req) {    // free block found?
            size_t leftover = current_block->size - req;
            if (leftover >= HDR + MIN_PAYLOAD + (ALIGNMENT - 1)) { // split?
                block_header_t* new_block = (block_header_t*) ((uint8_t*)current_block + HDR + req);
                
                if ((uint8_t*)new_block + HDR > (uint8_t*)HEAP_END) {
                    return NULL;
                }
                
                new_block->size = align_down(leftover - HDR, ALIGNMENT);
                new_block->next = current_block->next;
                new_block->free = true;
                current_block->next = new_block;
                current_block->free = false;
                current_block->size = req;
                return (void*)((uint8_t*)current_block + HDR);
            } else {    // no split
                current_block->free = false;
                current_block->size = req; // Set to requested size for consistent reporting
                return (void*)((uint8_t*)current_block + HDR);
            }
        }
        current_block = current_block->next; 
    }
    panic("kmalloc(): out of heap memory");
    return NULL;
}


void kfree(void* ptr) {
    if (!ptr) {
        return;
    }
    
    // Sanity check: ptr must be aligned
    if (((uintptr_t)ptr % ALIGNMENT) != 0) {
        return;
    }
    
    // Sanity check: ptr must be within heap bounds (use _va for pointer comparisons)
    if ((uint8_t*)ptr < (uint8_t*)HEAP_START || 
        (uint8_t*)ptr >= (uint8_t*)HEAP_END) {

        return;
    }
    
    block_header_t* header  = (block_header_t*)((uint8_t*)ptr - HDR);
    
    // Sanity check: header must also be within heap bounds
    if ((uint8_t*)header < (uint8_t*)HEAP_START || 
        (uint8_t*)header >= (uint8_t*)HEAP_END) {
        return;
    }
    
    if (header->free) {
        return;
    }
    header->free = true;
    
    // Forward merge: merge freed block with its next repeatedly
    while (header->next) {
        // Sanity check: ensure next is within heap bounds
        if ((uint8_t*)header->next < (uint8_t*)HEAP_START ||
            (uint8_t*)header->next >= (uint8_t*)HEAP_END) {
            break;
        }
        
        if (header->next->free) {
            uint8_t* block_end = (uint8_t*)header + HDR + header->size;
            if (block_end == (uint8_t*)header->next) {
                header->size += HDR + header->next->size;
                header->next = header->next->next;
            } else {
                break; // Not adjacent, stop merging
            }
        } else {
            break; // Next not free, stop merging
        }
    }
    
    // Backward merge: find previous block
    block_header_t* prev = NULL;
    if (header != heap_head) {
        prev = heap_head;
        while (prev && prev->next != header) {
            prev = prev->next;
        }
    }
    
    // Only attempt backward merge if we found a valid prev
    if (prev && prev->free) {
        uint8_t* end_of_prev = (uint8_t*)prev + HDR + prev->size;
        if (end_of_prev == (uint8_t*)header) {
            // Merge prev with header
            prev->size += HDR + header->size;
            prev->next = header->next;
            
            // Forward merge again from prev (it might now be adjacent to a free block)
            while (prev->next) {
                // Sanity check: ensure next is within heap bounds
                if ((uint8_t*)prev->next < (uint8_t*)HEAP_START ||
                    (uint8_t*)prev->next >= (uint8_t*)HEAP_END) {
                    break;
                }
                
                if (prev->next->free) {
                    uint8_t* block_end = (uint8_t*)prev + HDR + prev->size;
                    if (block_end == (uint8_t*)prev->next) {
                        prev->size += HDR + prev->next->size;
                        prev->next = prev->next->next;
                    } else {
                        break; // Not adjacent, stop merging
                    }
                } else {
                    break; // Next not free, stop merging
                }
            }
        }
    }
}


void heap_init(void) {
    heap_head = (block_header_t*)HEAP_START;
    heap_head->size = HEAP_END - HEAP_START - HDR;
    heap_head->next = NULL;
    heap_head->free = true;
}
