#include "alarm.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "eeprom.h"

Alarm_type alarm;
const static Alarm_type alarmMin PROGMEM = {0, 0, 0, 0, 0, 0, 0, 0, 0, ALARM_NOEDIT};
const static Alarm_type alarmMax PROGMEM = {23, 59, 1, 1, 1, 1, 1, 1, 1, ALARM_NOEDIT};

void alarmInit(void)
{
	eeprom_read_block(&alarm, EEPROM_ALARM_HOUR, sizeof(int8_t) * ALARM_ETM);

	alarm.eam = ALARM_NOEDIT;

	return;
}

void alarmSave(void)
{
	eeprom_update_block(&alarm, EEPROM_ALARM_HOUR, sizeof(int8_t) * ALARM_ETM);

	alarm.eam = ALARM_NOEDIT;

	return;
}

void alarmNextEditParam(void)
{
	if (alarm.eam >= ALARM_SUN)
		alarm.eam = ALARM_HOUR;
	else
		alarm.eam++;

	return;
}

void alarmChange(int8_t diff)
{
	int8_t *alrm = (int8_t*)&alarm + alarm.eam;
	int8_t alrmMax = pgm_read_byte((int8_t*)&alarmMax + alarm.eam);
	int8_t alrmMin = pgm_read_byte((int8_t*)&alarmMin + alarm.eam);

	*alrm += diff;

	if (*alrm > alrmMax)
		*alrm = alrmMin;
	if (*alrm < alrmMin)
		*alrm = alrmMax;

	return;
}

int8_t alarmRawWeekday(void)
{
	int8_t rawWeekday = 0x00;
	uint8_t i;

	for (i = 0; i <= ALARM_SUN - ALARM_MON; i++) {
		rawWeekday >>= 1;
		if (*((int8_t*)&alarm.mon + i))
			rawWeekday |= 0x40;
	}

	return rawWeekday;
}
