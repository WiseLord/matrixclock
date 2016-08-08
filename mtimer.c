#include "mtimer.h"

#include <avr/interrupt.h>

// Temperature/pressure/humidity sensor poll timer
uint8_t sensTimer = 0;
// Scroll interval timer
uint8_t scrollTimer = 0;
// Beeper timer
static volatile uint8_t beepTimer = 0;

// Seconds timer
static volatile uint8_t secTimer = TIME_SEC;

// Alarm timer
static volatile uint8_t alarmSecTimer = 0;
static volatile uint8_t alarmMinTimer = 0;

// Command buffer
static volatile uint8_t		cmdBuf;

void mTimerInit(void)
{
#if defined(atmega8)
	TIMSK |= (1<<TOIE0);							// Enable Timer0 overflow interrupt
	TCCR0 = (1<<CS02) | (0<<CS01) | (0<<CS00);		// Set timer prescaller to 256 (31250Hz)
#else
	TIMSK0 |= (1<<TOIE0);							// Enable Timer0 overflow interrupt
	TCCR0B = (1<<CS02) | (0<<CS01) | (0<<CS00);		// Set timer prescaller to 256 (31250Hz)
#endif

	DDR(BEEPER) |= BEEPER_LINE;
	PORT(BEEPER) |= BEEPER_LINE;

	// Setup buttons as inputs with pull-up resistors
	DDR(BUTTONS) &= ~(BUTTON_1_LINE | BUTTON_2_LINE | BUTTON_3_LINE);
	PORT(BUTTONS) |= (BUTTON_1_LINE | BUTTON_2_LINE | BUTTON_3_LINE);

	cmdBuf = BTN_STATE_0;

	return;
}

ISR (TIMER0_OVF_vect)								// 31250 / (256 - 131) = 250 polls/sec
{
	TCNT0 = 131;

	static int16_t btnCnt = 0;						// Buttons press duration value
	static uint8_t btnPrev = 0;						// Previous buttons state

	uint8_t btnNow = BTN_STATE_0;

	if (~PIN(BUTTONS) & BUTTON_1_LINE)
		btnNow |= BTN_0;
	if (~PIN(BUTTONS) & BUTTON_2_LINE)
		btnNow |= BTN_1;
	if (~PIN(BUTTONS) & BUTTON_3_LINE)
		btnNow |= BTN_2;

	// If button event has happened, place it to command buffer
	if (btnNow) {
		if (btnNow == btnPrev) {
			btnCnt++;
			if (btnCnt == LONG_PRESS)
				cmdBuf = (btnPrev << 4);
		} else {
			btnPrev = btnNow;
		}
	} else {
		if ((btnCnt > SHORT_PRESS) && (btnCnt < LONG_PRESS))
			cmdBuf = btnPrev;
		btnCnt = 0;
	}

	// 1 second intervals
	if (secTimer) {
		secTimer--;
	} else {
		secTimer = TIME_SEC;
		// Temperature
		if (sensTimer)
			sensTimer--;
		if (scrollTimer)
			scrollTimer--;

		if (alarmSecTimer) {
			alarmSecTimer--;
		} else {
			if (alarmMinTimer) {
				alarmMinTimer--;
				alarmSecTimer = 59;
			}
		}
	}

	// Beeper
	if (beepTimer) {
		beepTimer--;
	} else {
		if (alarmSecTimer || alarmMinTimer)
			beepTimer = TIME_SEC;
	}

	if ((beepTimer & 0x0E) > 8) {
		if (secTimer > TIME_SEC / 2)
			PORT(BEEPER) &= ~BEEPER_LINE;
	} else {
		PORT(BEEPER) |= BEEPER_LINE;
	}

	return;
}

void startBeeper(uint8_t time)
{
	beepTimer = time;
	secTimer = TIME_SEC;
	alarmSecTimer = 0;
	alarmMinTimer = 0;

	return;
}

void startAlarm(uint8_t time)
{
	alarmMinTimer = time;

	return;
}

uint8_t getBtnCmd(void)
{
	uint8_t ret = cmdBuf;
	cmdBuf = BTN_STATE_0;

	return ret;
}
