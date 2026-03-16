#include "kmain.h"
#include "kprintf.h"
#include "snprintf.h"
#include "string.h"
#include "vga/vga.h"
#include "splash.h"

_Noreturn void kmain(void) {
    vga_clear();
    kprintf_init(vga_putc);
    splash();
    while (1);
}