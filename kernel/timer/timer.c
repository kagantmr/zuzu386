#include "timer.h"
#include "../../arch/pic/pic.h"
#include "../../arch/irq/irq.h"
#include "../../arch/irq/idt.h"
#include "io.h"
#include "kprintf.h"

static volatile uint64_t ticks = 0;

__attribute__((interrupt)) static void timer_interrupt_handler(interrupt_frame_t *frame) {
    (void)frame;
    ticks++;
    pic_send_eoi(0);
}

void timer_init(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency;
    outb(0x43, 0x36); // Command byte: channel 0, access mode lobyte/hibyte, mode 3 (square wave)
    outb(0x40, divisor & 0xFF); // Low byte of divisor
    outb(0x40, (divisor >> 8) & 0xFF); // High byte of divisor

    i686_idt_set_handler(32, (uint32_t)timer_interrupt_handler, 0x08, IDT_FLAG_GATE_32INT | IDT_FLAG_RING0);
    i686_idt_enablehandler(32);
    pic_unmask_irq(0); // Unmask IRQ0 for timer interrupts
}

void timer_sleep_ms(uint32_t ms) {
    uint64_t target = ticks + (ms / 10);
    while (ticks < target);
}

uint64_t timer_get_ticks() {
    return ticks;
}
