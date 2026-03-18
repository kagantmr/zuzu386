#include "kmain.h"
#include "kprintf.h"
#include "snprintf.h"
#include "string.h"
#include "vga/vga.h"
#include "splash.h"
#include "core/panic.h"
#include "version.h"
#include "sound/speaker.h"
#include "music/music.h"
#include "../arch/irq/idt.h"
#include "isr/isr_exc.h"
#include "../arch/pic/pic.h"

void play_zuzu(void) {
    static const note_t notes[] = {
        {NOTE_C4, 4},
        {NOTE_DS4, 4},
        {NOTE_G3, 4},
        {NOTE_C4, 4},
        {NOTE_E4, 4},
        {NOTE_C4, 4},
        {NOTE_E4, 4},
        {NOTE_C5, 4},
        {NOTE_G4, 4},
        {NOTE_C5, 4},
        {NOTE_G4, 4},
        {NOTE_E4, 4},
        {NOTE_E5, 16},
    };

    static const song_t song = {
        .notes  = notes,
        .length = 13,
        .tempo  = 80,
    };

    play_melody(&song);
}

_Noreturn void kmain(void) {
    vga_clear();
    kprintf_init(vga_putc);
    splash();
    //play_zuzu();
    i686_idt_init();
    i686_idt_set_handler(0, (uint32_t)isr_divide_by_zero, 0x08, IDT_FLAG_GATE_32INT | IDT_FLAG_RING0);
    i686_idt_set_handler(6, (uint32_t)isr_invalid_opcode, 0x08, IDT_FLAG_GATE_32INT | IDT_FLAG_RING0);
    i686_idt_set_handler(8, (uint32_t)isr_double_fault, 0x08, IDT_FLAG_GATE_32INT | IDT_FLAG_RING0);
    i686_idt_set_handler(12, (uint32_t)isr_stack_segment_fault, 0x08, IDT_FLAG_GATE_32INT | IDT_FLAG_RING0);
    i686_idt_set_handler(13, (uint32_t)isr_general_protection_fault, 0x08, IDT_FLAG_GATE_32INT | IDT_FLAG_RING0);
    i686_idt_set_handler(14, (uint32_t)isr_page_fault, 0x08, IDT_FLAG_GATE_32INT | IDT_FLAG_RING0);
    i686_idt_set_handler(255, (uint32_t)isr_unhandled, 0x08, IDT_FLAG_GATE_32INT | IDT_FLAG_RING0); // catch-all for unhandled exceptions

    i686_idt_enablehandler(0); 
    i686_idt_enablehandler(6);
    i686_idt_enablehandler(8);
    i686_idt_enablehandler(12);
    i686_idt_enablehandler(13);
    i686_idt_enablehandler(14);
    i686_idt_enablehandler(255); 

    pic_remap();

    
    i686_idt_set_handler(33, (uint32_t)isr_unhandled, 0x08, IDT_FLAG_GATE_32INT | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    i686_idt_enablehandler(33);

    pic_unmask_irq(1); // Unmask keyboard IRQ (IRQ1)

    __asm__ volatile ("sti"); // Enable interrupts
    kprintf("Welcome to Zuzu386 (version %s)!\n", Z386_VERSION);
    //__asm__ volatile ("int $0"); // Controlled exception test: divide-by-zero vector.
    
    while (1);
}