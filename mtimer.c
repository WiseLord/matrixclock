#include "mtimer.h"

#include <avr/interrupt.h>

static volatile uint16_t tempTimer;				/* Timer of temperature measuring process */

void mTimerInit(void)
{
	TIMSK |= (1<<TOIE0);							/* Enable Timer0 overflow interrupt */
	TCCR0 |= (0<<CS02) | (1<<CS01) | (1<<CS00);		/* Set timer prescaller to 64 (125kHz) */

	tempTimer = 0;

	return;
}

ISR (TIMER0_OVF_vect)								/* 125kHz / 256 = 488 polls/sec */
{
	if (tempTimer)
		tempTimer--;

	return;
}

uint16_t getTempTimer()
{
	return tempTimer;
}

void setTempTimer(uint16_t val)
{
	tempTimer = val;
}
