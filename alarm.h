#ifndef ALARM_H
#define ALARM_H

#include <inttypes.h>

enum {
	ALARM_HOUR = 0,
	ALARM_MIN,
	ALARM_MON,
	ALARM_TUE,
	ALARM_WED,
	ALARM_THU,
	ALARM_FRI,
	ALARM_SAT,
	ALARM_SUN,
};

typedef struct {
	int8_t hour;
	int8_t min;
	int8_t mon;
	int8_t tue;
	int8_t wed;
	int8_t thu;
	int8_t fri;
	int8_t sat;
	int8_t sun;
	int8_t eam;
} Alarm_type;

extern Alarm_type alrm;

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
