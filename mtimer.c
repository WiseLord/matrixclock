#include "mtimer.h"

#include <avr/interrupt.h>

static volatile uint16_t clockTimer;				/* 1 second timer */
static volatile clock time = {16, 57, 53};

void mTimerInit(void)
{
	TIMSK |= (1<<OCIE2);							/* Enable Timer0 compare match A interrupt */
	TCCR2 |= (0<<CS22) | (1<<CS21) | (1<<CS20);		/* Set timer prescaller to 32 (250kHz) */

	TCCR2 |= (1<<WGM21) | (0<<WGM20);				/* Clear Timer0 on match */

	OCR2 = 249;									/* Set Timer0 period to 1ms */
	TCNT2 = 0;										/* Reset Timer0 value */

	return;
}

static void incClock()
{
	time.sec++;
	if (time.sec >= 60) {
		time.sec = 0;
		time.min++;
	}
	if (time.min >= 60) {
		time.min = 0;
		time.hour++;
	}
	if (time.hour >= 24)
		time.hour = 0;

	return;
}

ISR (TIMER2_COMP_vect)
{
	if (++clockTimer >= 1000) {
		incClock();
		clockTimer = 0;
	}
}

void getClock(clock *clk)
{
	clk->hour = time.hour;
	clk->min = time.min;
	clk->sec = time.sec;

	return;
}
