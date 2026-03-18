#include "exceptions.h"
#include "core/panic.h"

__attribute__((interrupt))
void isr_divide_by_zero(interrupt_frame_t *frame) {
    panic("Divide by zero at 0x%x", frame->ip);
}

__attribute__((interrupt)) void isr_invalid_opcode(interrupt_frame_t *frame) {
    panic("Invalid opcode at 0x%x", frame->ip);
}

__attribute__((interrupt)) void isr_double_fault(interrupt_frame_t *frame, uint32_t error_code) {
    panic("Double Fault at 0x%x (error code: 0x%x)", frame->ip, error_code);
}
__attribute__((interrupt)) void isr_stack_segment_fault(interrupt_frame_t *frame, uint32_t error_code) {
    panic("Stack Segment Fault at 0x%x (error code: 0x%x)", frame->ip, error_code);
}

__attribute__((interrupt)) 
void isr_general_protection_fault(interrupt_frame_t *frame, uint32_t error_code) {
    panic("General Protection Fault at 0x%x (error code: 0x%x)", frame->ip, error_code);
}

__attribute__((interrupt)) void isr_page_fault(interrupt_frame_t *frame, uint32_t error_code) {
    panic("Page Fault at 0x%x (error code: 0x%x)", frame->ip, error_code);
}
__attribute__((interrupt)) void isr_unhandled(interrupt_frame_t *frame) {
    panic("Unhandled exception at 0x%x", frame->ip);
}
