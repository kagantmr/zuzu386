#ifndef SPLASH_H
#define SPLASH_H

#include "vga/vga.h"
art_layout_t draw_centered_art(const char *const *lines, int line_count, uint8_t start_row, uint8_t color);
void splash(void);

#endif