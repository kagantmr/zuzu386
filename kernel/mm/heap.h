#ifndef HEAP_H
#define HEAP_H

#include "stdint.h"

typedef struct block_header {
    uint32_t size;           // size of the block (not including header)
    uint8_t  free;           // 1 = free, 0 = used
    struct block_header *next; // next block in list
} block_header_t;


#define ALIGNMENT    4
#define HDR          sizeof(block_header_t)
#define MIN_PAYLOAD  4

void  heap_init(void);
void *kmalloc(uint32_t size);
void  kfree(void *ptr);

#endif