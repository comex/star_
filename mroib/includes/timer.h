#ifndef TIMER_H
#define TIMER_H

#include "openiboot.h"

uint64_t timer_get_system_microtime();
void timer_get_rtc_ticks(uint64_t* ticks, uint64_t* sec_divisor);

void udelay(uint64_t delay);

#endif

