#include "mtimer.h"

#include <avr/interrupt.h>

/* Temperature */
static volatile uint16_t tempConvertTimer = 0;
static volatile uint16_t tempStartTimer = 0;

/* Beeper */
static volatile uint16_t beepTimer = 0;
static volatile uint16_t beepSecTimer = 0;

void mTimerInit(void)
{
	TIMSK |= (1<<TOIE0);							/* Enable Timer0 overflow interrupt */
	TCCR0 |= (0<<CS02) | (1<<CS01) | (1<<CS00);		/* Set timer prescaller to 64 (125kHz) */

	tempConvertTimer = 0;

	BEEPER_DDR |= BEEPER_PIN;

	return;
}

ISR (TIMER0_OVF_vect)								/* 125kHz / (256 - 131) = 1000 polls/sec */
{
	TCNT0 = 131;

	/* Temperature */
	if (tempConvertTimer)
		tempConvertTimer--;
	if (tempStartTimer)
		tempStartTimer--;

	/* Beeper */
	if (beepTimer)
		beepTimer--;
	if (beepSecTimer)
		beepSecTimer--;
	else
		beepSecTimer = 1000;

	if (((beepTimer >> 3) & 7) > 4) {
		if (beepSecTimer > 500)
			BEEPER_PORT &= ~BEEPER_PIN;
	} else {
		BEEPER_PORT |= BEEPER_PIN;
	}

	return;
}

uint16_t getTempConvertTimer(void)
{
	return tempConvertTimer;
}
void setTempConvertTimer(uint16_t val)
{
	tempConvertTimer = val;

	return;
}

uint16_t getTempStartTimer(void)
{
	return tempStartTimer;
}
void setTempStartTimer(uint16_t val)
{
	tempStartTimer = val;

	return;
}

void startBeeper(uint16_t time)
{
	beepTimer = time;

	return;
}

void stopBeeper(void)
{
	beepTimer = 0;

	return;
}
