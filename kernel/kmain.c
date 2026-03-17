#include "kmain.h"
#include "kprintf.h"
#include "snprintf.h"
#include "string.h"
#include "vga/vga.h"
#include "splash.h"
#include "core/panic.h"
#include "version.h"

_Noreturn void kmain(void) {
    vga_clear();
    kprintf_init(vga_putc);
    splash();
    kprintf("Welcome to Zuzu386 (version %s)!\n", Z386_VERSION);
    while (1);
}