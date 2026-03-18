#include "pic.h"
#include "../../include/io.h"

void pic_remap(void)
{
    // ICW1 — start initialization
    outb(0x20, 0x11); // master
    outb(0xA0, 0x11); // slave

    // ICW2 — set vector offsets
    outb(0x21, 0x20); // master IRQs start at vector 32
    outb(0xA1, 0x28); // slave IRQs start at vector 40

    // ICW3 — tell master/slave about each other
    outb(0x21, 0x04); // master: slave is on IRQ2
    outb(0xA1, 0x02); // slave: i am on IRQ2

    // ICW4 — set 8086 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // mask all IRQs for now
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);
}

void pic_mask_all(void)
{
    outb(0x21, 0xFF); // mask all master IRQs
    outb(0xA1, 0xFF); // mask all slave IRQs
}

void pic_unmask_irq(uint8_t irq) {
    if (irq < 8) {
        uint8_t mask = inb(0x21);
        mask &= ~(1 << irq);
        outb(0x21, mask);
    } else {
        // unmask IRQ2 on master (slave line)
        uint8_t master_mask = inb(0x21);
        master_mask &= ~(1 << 2);
        outb(0x21, master_mask);
        // unmask the actual IRQ on slave
        uint8_t slave_mask = inb(0xA1);
        slave_mask &= ~(1 << (irq - 8));
        outb(0xA1, slave_mask);
    }
}

void pic_mask_irq(uint8_t irq)
{
    if (irq < 8)
    {
        uint8_t mask = inb(0x21);
        mask |= (1 << irq);
        outb(0x21, mask);
    }
    else
    {
        uint8_t mask = inb(0xA1);
        mask |= (1 << (irq - 8));
        outb(0xA1, mask);
    }
}

void pic_send_eoi(uint8_t irq)
{
    // for IRQs 8-15 you must notify the slave too
    if (irq >= 8)
        outb(0xA0, 0x20); // EOI to slave
    outb(0x20, 0x20);     // EOI to master
}
