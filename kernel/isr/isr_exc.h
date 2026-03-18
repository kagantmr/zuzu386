#ifndef ISR_EXC_H
#define ISR_EXC_H

#include "stdint.h"

typedef struct {
    uint32_t ip;     // instruction pointer — where you were
    uint32_t cs;     // code segment
    uint32_t flags;  // eflags
} interrupt_frame_t;

__attribute__((interrupt)) void isr_divide_by_zero(interrupt_frame_t *frame); // vector 0
__attribute__((interrupt)) void isr_invalid_opcode(interrupt_frame_t *frame); // vector 6
__attribute__((interrupt)) void isr_double_fault(interrupt_frame_t *frame, uint32_t error_code); // vector 8
__attribute__((interrupt)) void isr_stack_segment_fault(interrupt_frame_t *frame, uint32_t error_code); // vector 12
__attribute__((interrupt)) void isr_general_protection_fault(interrupt_frame_t *frame, uint32_t error_code); // vector 13
__attribute__((interrupt)) void isr_page_fault(interrupt_frame_t *frame, uint32_t error_code); // vector 14
__attribute__((interrupt)) void isr_unhandled(interrupt_frame_t *frame); // catch-all
#endif