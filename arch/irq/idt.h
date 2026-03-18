#ifndef IDT_H
#define IDT_H

#include "stdint.h"

typedef struct {
    uint16_t offset_low;   // Lower 16 bits of handler function address
    uint16_t selector;     // Code segment selector in GDT
    uint8_t  reserved;     // Reserved, set to zero
    uint8_t  type_attr;    // Type and attributes
    uint16_t offset_hi;   // Middle 16 bits of handler function address
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;       // Size of the IDT in bytes - 1
    uint32_t base;        // Base address of the IDT
} __attribute__((packed)) idt_descriptor_t;


typedef enum {

    IDT_FLAG_GATE_TASK = 0x5,   // 32-bit Task Gate
    IDT_FLAG_GATE_16INT = 0x6,  // 16-bit
    IDT_FLAG_GATE_16TRAP = 0x7, // 16-bit Trap Gate
    IDT_FLAG_GATE_32INT = 0xE,  // 32-bit
    IDT_FLAG_GATE_32TRAP = 0xF, // 32-bit Trap Gate

    IDT_FLAG_RING0 = 0x00, // Ring 0 (kernel)
    IDT_FLAG_RING1 = 0x20, // Ring 1
    IDT_FLAG_RING2 = 0x40, // Ring 2
    IDT_FLAG_RING3 = 0x60, // Ring 3 (user)

    IDT_FLAG_PRESENT = 0x80 // Present flag

} idt_flags_t;

void i686_idt_load(const idt_descriptor_t *idt_desc);
void i686_idt_set_entry(idt_entry_t *entry, uint32_t handler_addr, uint16_t selector, uint8_t flags);
void i686_idt_init();
void i686_idt_set_handler(uint8_t vector, uint32_t handler_addr, uint16_t selector, uint8_t flags);
void i686_idt_enablehandler(uint8_t vector);
void i686_idt_disablehandler(uint8_t vector);

#endif // IDT_H