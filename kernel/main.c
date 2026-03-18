#include "main.h"
#include "kprintf.h"
#include "snprintf.h"
#include "string.h"
#include "ctype.h"
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

static char *skip_spaces(char *s) {
    while (*s && isspace((unsigned char)*s)) {
        s++;
    }
    return s;
}

static void rstrip_spaces(char *s) {
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[len - 1] = '\0';
        len--;
    }
}

static int shell_play_file(const char *name) {
    void *data = zrd_open(name);
    if (!data) {
        kprintf("File '%s' not found in ramdisk.\n", name);
        return 0;
    }

    uint32_t size = zrd_size(name);
    zuzm_song_t *song = zuzm_load(data, size);
    if (!song) {
        kprintf("File '%s' is not a valid music file.\n", name);
        return 0;
    }

    kprintf("Playing '%s'...\n", name);
    zuzm_play(song);
    zuzm_free(song);
    return 1;
}

static void shell_print_help(void) {
    kprintf("Commands:\n");
    kprintf("  help           Show this help\n");
    kprintf("  ls             List ramdisk files\n");
    kprintf("  play <file>    Play a .zuzm file\n");
    kprintf("  version        Show kernel version\n");
    kprintf("  clear          Clear screen\n");
    kprintf("Tip: typing a filename directly also tries to play it.\n");
}

static void shell_run_line(char *line) {
    rstrip_spaces(line);
    char *cmd = skip_spaces(line);

    if (*cmd == '\0') {
        return;
    }

    char *arg = cmd;
    while (*arg && !isspace((unsigned char)*arg)) {
        arg++;
    }

    if (*arg) {
        *arg = '\0';
        arg = skip_spaces(arg + 1);
    } else {
        arg = arg;
    }

    if (strcmp(cmd, "help") == 0) {
        shell_print_help();
    } else if (strcmp(cmd, "ls") == 0) {
        zrd_stat();
    } else if (strcmp(cmd, "play") == 0) {
        if (*arg == '\0') {
            kprintf("Usage: play <file>\n");
        } else {
            shell_play_file(arg);
        }
    } else if (strcmp(cmd, "version") == 0) {
        kprintf("zuzu386 version %s\n", Z386_VERSION);
    } else if (strcmp(cmd, "clear") == 0) {
        vga_clear();
        splash();
    } else {
        shell_play_file(cmd);
    }
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

    heap_init();
    zrd_init((void*)&_ramdisk_start);
    pic_remap();
    timer_init(1000); // 1000 Hz for finer timing resolution in audio playback.
    kprintf("Welcome to Zuzu386 (version %s)!\n", Z386_VERSION);
    //__asm__ volatile ("int $0"); // Controlled exception test: divide-by-zero vector.
    keyboard_init();

    __asm__ volatile ("sti"); // Enable interrupts

    // Load and play music from ramdisk
    shell_play_file("boot.zuzm");

    shell_print_help();
    kprintf("zuzu> ");

    char file[256];
    size_t file_idx = 0;

    while (1) {
        if (keyboard_haschar()) {
            char c = keyboard_getchar();
            if (c == '\n' || c == '\r') {
                file[file_idx] = '\0';
                vga_putc('\n');

                if (file_idx == 0) {
                    kprintf("zuzu> ");
                    continue;
                }

                shell_run_line(file);
                file_idx = 0;
                kprintf("zuzu> ");
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