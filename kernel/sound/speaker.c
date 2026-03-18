#include "speaker.h"
#include "io.h"
#include "../timer/timer.h"

void speaker_beep(uint32_t freq, uint32_t duration_ms) {
    if (freq == 0) {
        speaker_stop();
        return;
    }

    uint32_t divisor = 1193180 / freq;

    // Set the PIT to the desired frequency
    outb(0x43, 0xB6); // Command byte: channel 2, access mode lobyte/hibyte, mode 3 (square wave)
    outb(0x42, divisor & 0xFF); // Low byte
    outb(0x42, (divisor >> 8) & 0xFF); // High byte

    // Enable the speaker
    uint8_t control = inb(0x61);
    control |= 3; // Set bits 0 and 1 to enable the speaker
    outb(0x61, control);

    timer_sleep_ms(duration_ms);
}

void speaker_stop() {
    // Disable the speaker
    uint8_t control = inb(0x61);
    control &= ~3; // Clear bits 0 and 1 to disable the speaker
    outb(0x61, control);
}