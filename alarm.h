#ifndef ALARM_H
#define ALARM_H

#include <inttypes.h>

#define EEPROM_A_HOUR		((void*)0x00)
#define EEPROM_A_MIN		((void*)0x01)
#define EEPROM_A_DAYS		((void*)0x02)

typedef enum {
	A_HOUR  = 0,
	A_MIN   = 1,
	A_MONDAY  = 2,
	A_TUESDAY  = 3,
	A_WEDNESDAY  = 4,
	A_THURSDAY  = 5,
	A_FRIDAY  = 6,
	A_SATURDAY  = 7,
	A_SUNDAY  = 8,
	A_NOEDIT = 9
} alarmMode;

void initAlarm(void);

int8_t getAlarm(alarmMode am);

alarmMode getAlarmMode();

int8_t *readAlarm(void);

int8_t getRawWeekday(void);

void writeAlarm(void);

void editAlarm(void);

void stopEditAlarm(void);

void changeAlarm(int8_t diff);

#endif // ALARM_H
