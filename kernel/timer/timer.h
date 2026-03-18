#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

void timer_init(uint32_t frequency);
void timer_sleep_ms(uint32_t ms);
uint64_t timer_get_ticks();

#endif