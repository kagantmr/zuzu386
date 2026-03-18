#include "main.h"
#include "kprintf.h"
#include "snprintf.h"
#include "string.h"
#include "vga/vga.h"
#include "splash.h"
#include "core/panic.h"
#include "version.h"
#include "sound/speaker.h"
#include "music/music.h"
#include "arch/x86/interrupt/idt.h"
#include "timer/timer.h"    
#include "keyboard/keyboard.h"  
#include "isr/exceptions.h"
#include "mm/heap.h"
#include "arch/x86/pic/pic.h"
#include "zrd/zrd.h"
#include "arch/x86/symbols.h"
#include "music/music_loader.h"
#include "mem.h"


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

    heap_init();
    zrd_init((void*)&_ramdisk_start);
    pic_remap();
    timer_init(1000); // 1000 Hz for finer timing resolution in audio playback.
    kprintf("Welcome to Zuzu386 (version %s)!\n", Z386_VERSION);
    //__asm__ volatile ("int $0"); // Controlled exception test: divide-by-zero vector.
    keyboard_init();

    __asm__ volatile ("sti"); // Enable interrupts

    // Load and play music from ramdisk
    void *music_data = zrd_open("boot.zuzm");
    if (music_data) {
        uint32_t music_size = zrd_size("boot.zuzm");
        zuzm_song_t *song = zuzm_load(music_data, music_size);
        if (song) {
            zuzm_play(song);
            zuzm_free(song);
        }
    }

    char file[256];
    size_t file_idx = 0;

    while (1) {
        if (keyboard_haschar()) {
            char c = keyboard_getchar();
            if (c == '\n' || c == '\r') {
                file[file_idx] = '\0';
                vga_putc('\n');

                if (file_idx == 0) {
                    continue;
                }

                void *data = zrd_open(file);
                if (data) {
                    uint32_t size = zrd_size(file);
                    kprintf("Opened '%s' (%u bytes)\n", file, size);
                    // For demo, try loading as music
                    zuzm_song_t *song = zuzm_load(data, size);
                    if (song) {
                        kprintf("Playing '%s'...\n", file);
                        zuzm_play(song);
                        zuzm_free(song);
                    } else {
                        kprintf("File '%s' is not a valid music file.\n", file);
                    }
                } else {
                    kprintf("File '%s' not found in ramdisk.\n", file);
                }
                file_idx = 0;
            } else if (c == '\b') {
                if (file_idx > 0) {
                    file_idx--;
                    vga_putc('\b');
                    vga_putc(' ');
                    vga_putc('\b');
                }
            } else if (file_idx < sizeof(file) - 1) {
                file[file_idx++] = c;
                vga_putc(c); // echo input
            }
        }
    }
}