#include "../vga/vga.h"
#include "../sound/speaker.h"

_Noreturn void panic(const char *message) {
    volatile uint16_t *const vga = (volatile uint16_t *)0xB8000;
    const uint8_t panic_bg = VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_RED);

    for (uint16_t i = 0; i < (80 * 25); i++) {
        vga[i] = ((uint16_t)panic_bg << 8) | ' ';
    }

    vga_set_color(panic_bg);
    vga_set_cursor(0, 0);
    vga_puts("ZUZU HAS PANICKED!\n");
    vga_puts("PANIC MESSAGE: ");
    vga_puts(message);
    vga_puts("\n\nPlease restart the system.");
    speaker_beep(440, 4000); // Beep at A4 for 1 second to indicate panic
    speaker_stop();
    
    while (1);
}

