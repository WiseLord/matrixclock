#ifndef MTIMER_H
#define MTIMER_H

#include <inttypes.h>
#include "pins.h"

#define TEMP_MEASURE_TIME		2
#define SENSOR_POLL_INTERVAL	10

#define BTN_STATE_0				0b000

/* Handling long press actions */
#define SHORT_PRESS				25
#define LONG_PRESS				125

#define BEEP_SHORT				20
#define BEEP_LONG				40
#define BEEP_ALARM				15000

enum {
	CMD_EMPTY = 0,

	CMD_BTN_1,
	CMD_BTN_2,
	CMD_BTN_3,
	CMD_BTN_1_LONG,
	CMD_BTN_2_LONG,
	CMD_BTN_3_LONG,
	CMD_BTN_1_2_LONG,
	CMD_BTN_2_3_LONG,
	CMD_BTN_1_3_LONG,
	CMD_BTN_1_2_3_LONG,
};

void mTimerInit(void);

uint16_t getTempConvertTimer(void);
void setTempConvertTimer(uint16_t val);

uint8_t getSensTimer(void);
void setSensTimer(uint8_t val);

uint8_t getScrollTimer(void);
void setScrollTimer(uint8_t val);

void startBeeper(uint16_t time);
uint16_t getBeepTimer(void);
void stopBeeper(void);

uint8_t getBtnCmd(void);

#endif /* MTIMER_H */
