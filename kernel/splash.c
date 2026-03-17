#include "vga/vga.h"
#include "string.h"
#include "snprintf.h"
#include "splash.h"

#define NONE ""

#define SPLASH_COLOR VGA_COLOR(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK)
#define SPLASH_LOGO_COLOR VGA_COLOR(VGA_COLOR_LIGHT_BROWN, VGA_COLOR_BLACK)
#define SPLASH_RULE_COLOR VGA_COLOR(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK)

typedef struct art_layout {
    uint8_t col;
    uint8_t width;
} art_layout_t;

static int line_leading_NONEs(const char *line) {
    int count = 0;

    while (line[count] == ' ') {
        count++;
    }

    return count;
}

static int line_right_edge(const char *line) {
    int offset = 0;
    int right_edge = 0;

    while (*line) {
        if (*line != ' ') {
            right_edge = offset + 1;
        }
        offset++;
        line++;
    }

    return right_edge;
}

static art_layout_t draw_centered_art(const char *const *lines, int line_count, uint8_t start_row, uint8_t color) {
    int left_trim = 80;
    int max_right_edge = 0;
    art_layout_t layout = {0, 0};

    for (int i = 0; i < line_count; i++) {
        int leading_NONEs = line_leading_NONEs(lines[i]);
        int right_edge = line_right_edge(lines[i]);

        if (right_edge == 0) {
            continue;
        }

        if (leading_NONEs < left_trim) {
            left_trim = leading_NONEs;
        }

        if (right_edge > max_right_edge) {
            max_right_edge = right_edge;
        }
    }

    if (left_trim == 80) {
        left_trim = 0;
    }

    if (max_right_edge < left_trim) {
        max_right_edge = left_trim;
    }

    layout.width = (uint8_t)(max_right_edge - left_trim);
    if (layout.width > 80) {
        layout.width = 80;
    }

    layout.col = (uint8_t)((80 - layout.width) / 2);

    vga_set_color(color);
    for (int i = 0; i < line_count; i++) {
        int offset = 0;
        const char *line = lines[i] + left_trim;

        while (*line) {
            if (*line != ' ') {
                vga_set_cursor((uint8_t)(start_row + i), (uint8_t)(layout.col + offset));
                vga_putc(*line);
            }
            offset++;
            line++;
        }
    }

    return layout;
}

void splash(void) {
    char status_text[32];
    art_layout_t logo_layout;

    static const char* const panic_logo[] = {
        "      " NONE "@@@@@@" NONE,
        "        " NONE "%@@" NONE "         " NONE "%@@@  @" NONE,
        "       " NONE "@@" NONE "           " NONE "@@" NONE "   " NONE "@@" NONE,
        "     " NONE "@@" NONE "             " NONE "@" NONE "    " NONE "@=" NONE,
        "    " NONE "@@@@@@@@@@@@@@" NONE " " NONE "@@" NONE "   " NONE "@@" NONE,
        "        " NONE "@@" NONE "          " NONE "@@@@" NONE,
        "        " NONE "@@" NONE "             " NONE "@" NONE,
        "        " NONE "@@  %@@@@@@@  *@" NONE,
        "        " NONE "@@  @" NONE "      " NONE "@@ @@" NONE,
        "        " NONE "@@  @" NONE "      " NONE "@@ @@" NONE,
        "         " NONE "@@@" NONE "        " NONE "@@@" NONE,
    };

    static const char* const logo[] = {
        "                 ____ ___   __ ",
        " ____  _ ____  _|__ /( _ ) / / ",
        "|_ / || |_ / || ||_ \\/ _ \\/ _ \\",
        "/__|\\_,_/__|\\_,_|___/\\___/\\___/"
    };

    vga_clear();

    draw_centered_art(panic_logo, 11, 2, SPLASH_COLOR);
    logo_layout = draw_centered_art(logo, 4, 14, SPLASH_LOGO_COLOR);

    vga_set_color(SPLASH_RULE_COLOR);
    vga_set_cursor(19, 0);
    for (int i = 0; i < 80; i++)
        vga_putc(0xC4);  // ─

    snprintf(status_text, sizeof(status_text), "(%s)", "experimental");
    vga_set_color(SPLASH_RULE_COLOR);
    vga_set_cursor(18, (uint8_t)(logo_layout.col + (logo_layout.width - visible_len(status_text)) / 2));
    vga_puts(status_text);

    // reset to normal
    vga_set_color(0x0F);
    vga_set_cursor(21, 0);
}