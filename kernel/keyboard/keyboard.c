#include "keyboard.h"
#include "../../arch/pic/pic.h"
#include "../../arch/irq/irq.h"
#include "../../arch/irq/idt.h"
#include "io.h"
#include "kprintf.h"

static const char scancode_normal[] = {
    0,    0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

static const char scancode_shifted[] = {
    0,    0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,   '|',  'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' '
};

#define KEYBOARD_BUFFER_SIZE 256
static uint8_t buffer[KEYBOARD_BUFFER_SIZE];
static uint8_t head = 0;
static uint8_t tail = 0;

static uint8_t shift_held = 0;
static uint8_t caps_lock = 0;

__attribute__((interrupt)) static void keyboard_isr(interrupt_frame_t *frame)
{
    (void)frame;
    uint8_t scancode = inb(0x60);
    if (scancode == 0x2A || scancode == 0x36)      // left/right shift pressed
    shift_held = 1;
    else if (scancode == 0xAA || scancode == 0xB6)  // left/right shift released
        shift_held = 0;
    if (scancode == 0x3A)   // caps lock pressed
        caps_lock = !caps_lock;
    uint8_t use_upper = shift_held ^ caps_lock;
    const char *table = use_upper ? scancode_shifted : scancode_normal;
    uint8_t next_head = (head + 1) % KEYBOARD_BUFFER_SIZE;
    if (next_head != tail)
    { // Check for buffer overflow

        if (scancode >= 0x80)
        {
            // Key release event - ignore for now
        }
        else
        {
            char c = table[scancode];
            if (c) {
                buffer[head] = c;
                head = next_head;
            }
    

        }
    }
    pic_send_eoi(1);
}

void keyboard_init()
{
    i686_idt_set_handler(33, (uint32_t)keyboard_isr, 0x08, IDT_FLAG_GATE_32INT | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    i686_idt_enablehandler(33);
    pic_unmask_irq(1);
}

int keyboard_haschar() {
    return head != tail;
}

int keyboard_getchar() {
    if (!keyboard_haschar()) return 0;
    char c = buffer[tail];
    tail = (tail + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}