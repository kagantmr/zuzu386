#include "../vga/vga.h"
#include "../sound/speaker.h"
#include "string.h"
#include "stdarg.h"

_Noreturn void panic(const char *fmt, ...)
{
    volatile uint16_t *const vga = (volatile uint16_t *)0xB8000;
    const uint8_t panic_bg = VGA_COLOR(VGA_COLOR_WHITE, VGA_COLOR_RED);

    for (uint16_t i = 0; i < (80 * 25); i++)
    {
        vga[i] = ((uint16_t)panic_bg << 8) | ' ';
    }

    vga_set_color(panic_bg);
    vga_set_cursor(0, 0);


    vga_puts("Oops! zuzu386 has panicked.\n");

    vga_puts("Reason: ");
    va_list args;
    va_start(args, fmt);
    vstrfmt(vga_putc, fmt, &args);
    va_end(args);

    vga_puts("\nAn unrecoverable kernel error occurred and the system was halted.\n");
    vga_puts("This may be caused by a kernel bug or an operation not yet supported by zuzu386.\n");
    vga_puts("If you believe this is a bug, please report it at:\n");
    vga_puts("https://github.com/kagantmr/zuzu/issues\n");
    vga_puts("If not, please restart the system.");

    for (volatile int i = 0; i < 100000; i++); 
    // Simple delay loop so the user doesn't pee themselves

    speaker_beep(440, 4000); // Beep at A4 for 1 second to indicate panic
    speaker_stop();

    __asm__ volatile("cli\n"
                     "hlt");
    while (1)
        ;
}
