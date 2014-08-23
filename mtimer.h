#ifndef MTIMER_H
#define MTIMER_H

#include <avr/io.h>

typedef struct {
	int8_t hour;
	int8_t min;
	int8_t sec;
} clock;

void mTimerInit(void);
void getClock(clock *clk);

#endif /* MTIMER_H */
