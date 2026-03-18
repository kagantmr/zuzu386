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

static void vga_scroll(void) {
    for (int i = 0; i < (VGA_ROWS - 1) * VGA_COLS; i++)
        VGA_BUFFER[i] = VGA_BUFFER[i + VGA_COLS];
    for (unsigned short i = (VGA_ROWS - 1) * VGA_COLS; i < VGA_ROWS * VGA_COLS; i++)
        VGA_BUFFER[i] = 0x0700 | ' ';
    cursor = (VGA_ROWS - 1) * VGA_COLS;
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
    if (c == '\n') {
        cursor = (unsigned short)(((cursor / 80) + 1) * 80);
        if (cursor >= 80 * 25) vga_scroll();
        vga_sync_cursor();
        return;
    }

    if (c == '\b') {
        if (cursor > 0) cursor--;
        VGA_BUFFER[cursor] = (unsigned short)((current_color << 8) | ' ');
        vga_sync_cursor();
        return;
    }

    VGA_BUFFER[cursor++] = (unsigned short)((current_color << 8) | (unsigned char)c);
    if (cursor >= 80 * 25) vga_scroll();
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
