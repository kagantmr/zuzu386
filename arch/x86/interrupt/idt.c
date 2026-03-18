#include "idt.h"
#include "stdint.h"

static idt_entry_t idt_entries[256];
static idt_descriptor_t idt_desc = {
    (uint16_t)(sizeof(idt_entry_t) * 256 - 1),
    (uint32_t)(uintptr_t)idt_entries
};

void i686_idt_load(const idt_descriptor_t *idt_desc) {
    __asm__ volatile ("lidt (%0)" : : "r" (idt_desc));
}

void i686_idt_set_entry(idt_entry_t *entry, uint32_t handler_addr, uint16_t selector, uint8_t flags) {
    entry->offset_low = handler_addr & 0xFFFF;
    entry->selector = selector;
    entry->reserved = 0;
    entry->type_attr = flags;
    entry->offset_hi = (handler_addr >> 16) & 0xFFFF;
}

void i686_idt_init() {
    // Clear all IDT entries
    for (int i = 0; i < 256; i++) {
        i686_idt_set_entry(&idt_entries[i], 0, 0, 0);
    }

    // Load the IDT
    i686_idt_load(&idt_desc);
}

void i686_idt_set_handler(uint8_t vector, uint32_t handler_addr, uint16_t selector, uint8_t flags) {
    i686_idt_set_entry(&idt_entries[vector], handler_addr, selector, flags);
}

void  i686_idt_enablehandler(uint8_t vector) {
    idt_entries[vector].type_attr |= IDT_FLAG_PRESENT;
}

void  i686_idt_disablehandler(uint8_t vector) {
    idt_entries[vector].type_attr &= ~IDT_FLAG_PRESENT;
}

