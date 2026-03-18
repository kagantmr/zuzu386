#ifndef IRQ_H
#define IRQ_H
#include "stdint.h"

typedef struct {
    uint32_t ip;     // instruction pointer — where you were
    uint32_t cs;     // code segment
    uint32_t flags;  // eflags
} interrupt_frame_t;

#endif