#include "openiboot.h"
#include "timer.h"
#include "hardware/timer.h"


uint64_t timer_get_system_microtime() {
        uint64_t ticks;
        uint64_t sec_divisor;

        timer_get_rtc_ticks(&ticks, &sec_divisor);          
        return (ticks * uSecPerSec)/sec_divisor;
}

void timer_get_rtc_ticks(uint64_t* ticks, uint64_t* sec_divisor) {
	register uint32_t ticksHigh;
	register uint32_t ticksLow;
	register uint32_t ticksHigh2;

	/* try to get a good read where the lower bits remain the same after reading the higher bits */
	do {
		ticksHigh = GET_REG(TIMER + TIMER_TICKSHIGH);
		ticksLow = GET_REG(TIMER + TIMER_TICKSLOW);
		ticksHigh2 = GET_REG(TIMER + TIMER_TICKSHIGH);
	} while(ticksHigh != ticksHigh2);

	*ticks = (((uint64_t)ticksHigh) << 32) | ticksLow;
	*sec_divisor = 0x16E3600;//TicksPerSec;
}

void udelay(uint64_t delay) {
	uint64_t startTime = timer_get_system_microtime();

	// loop while elapsed time is less than requested delay
	while((timer_get_system_microtime() - startTime) < delay);
}

