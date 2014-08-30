#include "mtimer.h"

#include <avr/interrupt.h>

/* Temperature */
static volatile uint16_t tempConvertTimer = 0;
static volatile uint16_t tempStartTimer = 0;

/* Beeper */
static volatile uint16_t beepTimer = 0;
static volatile uint16_t beepSecTimer = 0;

/* Command buffer */
static volatile uint8_t cmdBuf;

void mTimerInit(void)
{
	TIMSK |= (1<<TOIE0);							/* Enable Timer0 overflow interrupt */
	TCCR0 |= (0<<CS02) | (1<<CS01) | (1<<CS00);		/* Set timer prescaller to 64 (125kHz) */

	tempConvertTimer = 0;

	BEEPER_DDR |= BEEPER_PIN;

	/* Setup buttons as inputs with pull-up resistors */
	BTN_DDR &= ~(BTN_MASK);
	BTN_PORT |= BTN_MASK;

	cmdBuf = CMD_EMPTY;

	return;
}

ISR (TIMER0_OVF_vect)								/* 125kHz / (256 - 131) = 1000 polls/sec */
{
	TCNT0 = 131;

	static int16_t btnCnt = 0;					/* Buttons press duration value */
	static uint8_t btnPrev = 0;					/* Previous buttons state */

	uint8_t btnNow = ~BTN_PIN & BTN_MASK;		/* Current buttons state */

	/* If button event has happened, place it to command buffer */
	if (btnNow) {
		if (btnNow == btnPrev) {
			btnCnt++;
			if (btnCnt == LONG_PRESS) {
				switch (btnPrev) {
				case BTN_1:
					cmdBuf = CMD_BTN_1_LONG;
					break;
				case BTN_2:
					cmdBuf = CMD_BTN_2_LONG;
					break;
				case BTN_3:
					cmdBuf = CMD_BTN_3_LONG;
					break;
				}
			}
		} else {
			btnPrev = btnNow;
		}
	} else {
		if ((btnCnt > SHORT_PRESS) && (btnCnt < LONG_PRESS)) {
			switch (btnPrev) {
			case BTN_1:
				cmdBuf = CMD_BTN_1;
				break;
			case BTN_2:
				cmdBuf = CMD_BTN_2;
				break;
			case BTN_3:
				cmdBuf = CMD_BTN_3;
				break;
			}
		}
		btnCnt = 0;
	}


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
	beepSecTimer = 0;
	beepTimer = time;

	return;
}

void stopBeeper(void)
{
	beepTimer = 0;

	return;
}

uint8_t getBtnCmd(void)
{
	uint8_t ret = cmdBuf;
	cmdBuf = CMD_EMPTY;

	return ret;
}
