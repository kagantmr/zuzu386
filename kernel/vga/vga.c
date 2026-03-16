#include "vga.h"
#include "mem.h"

#define VGA_COLS 80
#define VGA_ROWS 25

static volatile unsigned short* const VGA_BUFFER = (volatile unsigned short*)0xB8000;
static unsigned short cursor;
static uint8_t current_color = 0x0F; // light grey on black

static inline void vga_outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static void vga_sync_cursor(void) {
    vga_outb(0x3D4, 0x0F);
    vga_outb(0x3D5, (uint8_t)(cursor & 0xFF));
    vga_outb(0x3D4, 0x0E);
    vga_outb(0x3D5, (uint8_t)((cursor >> 8) & 0xFF));
}

void vga_clear(void) {
    for (unsigned short i = 0; i < VGA_COLS * VGA_ROWS; i++)
        VGA_BUFFER[i] = 0x0700 | ' ';
    cursor = 0;
    vga_sync_cursor();
}

void vga_putc(char c) {
    if (cursor >= 80 * 25) {
        // cursor = 0; abandon the old ways
        // scroll up by one line
        memmove((void*)VGA_BUFFER, (const void*)(VGA_BUFFER + 80), (VGA_ROWS - 1) * VGA_COLS * sizeof(uint16_t));
        // clear the last line
        for (unsigned short i = (VGA_ROWS - 1) * VGA_COLS; i < VGA_ROWS * VGA_COLS; i++)
            VGA_BUFFER[i] = 0x0700 | ' ';
        cursor = (VGA_ROWS - 1) * VGA_COLS;
    }

    if (c == '\n') {
        cursor = (unsigned short)(((cursor / 80) + 1) * 80);
        // Check bounds after newline in case we're now at/past the end
        if (cursor >= 80 * 25) {
            memmove((void*)VGA_BUFFER, (const void*)(VGA_BUFFER + 80), (VGA_ROWS - 1) * VGA_COLS * sizeof(uint16_t));
            for (unsigned short i = (VGA_ROWS - 1) * VGA_COLS; i < VGA_ROWS * VGA_COLS; i++)
                VGA_BUFFER[i] = 0x0700 | ' ';
            cursor = (VGA_ROWS - 1) * VGA_COLS;
        }
        vga_sync_cursor();
        return;
    }

    VGA_BUFFER[cursor++] = (unsigned short)((current_color << 8) | (unsigned char)c);
    vga_sync_cursor();
}

void vga_set_color(uint8_t color) {
    current_color = color;
}

void vga_set_cursor(uint8_t row, uint8_t col) {
    unsigned short pos = row * VGA_COLS + col;
    if (pos < VGA_COLS * VGA_ROWS) {
        cursor = pos;
        vga_sync_cursor();
    }
}

void vga_puts(const char* str) {
    while (*str) {
        vga_putc(*str++);
    }
}
