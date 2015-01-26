#ifndef MTIMER_H
#define MTIMER_H

#include <avr/io.h>

/* Biper */
#define BEEPER_DDR			DDRD
#define BEEPER_PORT			PORTD
#define BEEPER_PIN			(1<<PD5)

#define TEMP_MEASURE_TIME	750
#define TEMP_POLL_INTERVAL	1000

/* Buttons definitions */
#define BTN_DDR		DDRB
#define BTN_PIN		PINB
#define BTN_PORT	PORTB

#define BTN_1		(1<<PB3)
#define BTN_2		(1<<PB4)
#define BTN_3		(1<<PB5)

#define BTN_MASK	(BTN_1 | BTN_2 | BTN_3)

/* Handling long press actions */
#define SHORT_PRESS		50
#define LONG_PRESS		500

enum {
	CMD_BTN_1,
	CMD_BTN_2,
	CMD_BTN_3,
	CMD_BTN_1_LONG,
	CMD_BTN_2_LONG,
	CMD_BTN_3_LONG,
	CMD_BTN_1_2_3_LONG,
	CMD_EMPTY = 0xFF
};

void mTimerInit(void);

uint16_t getTempConvertTimer(void);
void setTempConvertTimer(uint16_t val);
uint16_t getTempStartTimer(void);
void setTempStartTimer(uint16_t val);

void startBeeper(uint16_t time);
uint16_t getBeepTimer(void);
void stopBeeper(void);

uint8_t getBtnCmd(void);

#endif /* MTIMER_H */
