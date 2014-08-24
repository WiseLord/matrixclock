#include "mtimer.h"

#include <avr/interrupt.h>

void mTimerInit(void)
{
	TIMSK |= (1<<TOIE0);							/* Enable Timer0 overflow interrupt */
	TCCR0 |= (0<<CS02) | (1<<CS01) | (1<<CS00);		/* Set timer prescaller to 64 (125kHz) */

	return;
}

ISR (TIMER0_OVF_vect)								/* 125kHz / 256 = 488 polls/sec */
{
	return;
}
