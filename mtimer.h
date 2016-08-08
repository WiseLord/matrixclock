#ifndef MTIMER_H
#define MTIMER_H

#include <inttypes.h>
#include "pins.h"

#define TEMP_MEASURE_TIME		2
#define SENSOR_POLL_INTERVAL	10

#define BTN_STATE_0				0x00
#define BTN_0					(1<<0)
#define BTN_1					(1<<1)
#define BTN_2					(1<<2)
#define BTN_0_LONG				(BTN_0<<4)
#define BTN_1_LONG				(BTN_1<<4)
#define BTN_2_LONG				(BTN_2<<4)

// Handling long press actions
#define SHORT_PRESS				25
#define LONG_PRESS				125

#define BEEP_SHORT				20
#define BEEP_LONG				40

#define TIME_SEC				250

extern uint8_t sensTimer;
extern uint8_t scrollTimer;

void mTimerInit(void);

void startBeeper(uint8_t time);
void startAlarm(uint8_t time);

uint8_t getBtnCmd(void);

#endif // MTIMER_H
