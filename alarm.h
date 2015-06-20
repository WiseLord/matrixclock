#ifndef ALARM_H
#define ALARM_H

#include <inttypes.h>

enum {
	ALARM_HOUR = 0,
	ALARM_MIN,
	ALARM_MONDAY,
	ALARM_TUESDAY,
	ALARM_WEDNESDAY,
	ALARM_THURSDAY,
	ALARM_FRIDAY,
	ALARM_SATURDAY,
	ALARM_SUNDAY,
};

#define ALARM_NOEDIT			0xFF


void initAlarm(void);

int8_t getAlarm(uint8_t am);

uint8_t getAlarmMode();

int8_t *readAlarm(void);

int8_t getRawAlarmWeekday(void);

void writeAlarm(void);

void editAlarm(void);

void changeAlarm(int8_t diff);

#endif // ALARM_H
