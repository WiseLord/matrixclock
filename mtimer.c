#include "mtimer.h"

#include <avr/interrupt.h>

static volatile uint16_t tempConvertTimer = 0;
static volatile uint16_t tempStartTimer = 0;

void mTimerInit(void)
{
	TIMSK |= (1<<TOIE0);							/* Enable Timer0 overflow interrupt */
	TCCR0 |= (0<<CS02) | (1<<CS01) | (1<<CS00);		/* Set timer prescaller to 64 (125kHz) */

	tempConvertTimer = 0;

	return;
}

ISR (TIMER0_OVF_vect)								/* 125kHz / (256 - 131) = 1000 polls/sec */
{
	TCNT0 = 131;

	if (tempConvertTimer)
		tempConvertTimer--;

	if (tempStartTimer)
		tempStartTimer--;

	return;
}

uint16_t getTempConvertTimer(void)
{
	return tempConvertTimer;
}
void setTempConvertTimer(uint16_t val)
{
	tempConvertTimer = val;
}

uint16_t getTempStartTimer(void)
{
	return tempStartTimer;
}
void setTempStartTimer(uint16_t val)
{
	tempStartTimer = val;
}
