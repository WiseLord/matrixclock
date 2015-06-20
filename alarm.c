#include "alarm.h"

#include "eeprom.h"
#include <avr/eeprom.h>

Alarm_type alrm;

static int8_t *alarm = (int8_t*)&alrm;

void initAlarm(void)
{
	int8_t i, rawAlarm;

	alrm.hour = eeprom_read_byte(EEPROM_ALARM_HOUR);
	alrm.min = eeprom_read_byte(EEPROM_ALARM_MIN);

	rawAlarm = eeprom_read_byte(EEPROM_ALARM_MON) & 0x7F;
	for (i = 0; i <= ALARM_SUN - ALARM_MON; i++)
		alarm[i + ALARM_MON] = rawAlarm & (1<<i);

	alrm.eam = ALARM_NOEDIT;

	return;
}

int8_t getAlarm(uint8_t am)
{
	return alarm[am];
}

uint8_t getAlarmMode()
{
	return alrm.eam;
}

int8_t *readAlarm(void)
{
	return alarm;
}

int8_t getRawAlarmWeekday(void)
{
	int8_t rawWeekday = 0x00;
	uint8_t i;

	for (i = 0; i <= ALARM_SUN - ALARM_MON; i++) {
		rawWeekday >>= 1;
		if (alarm[ALARM_MON + i])
			rawWeekday |= 0x40;
	}

	return rawWeekday;
}

void writeAlarm(void)
{
	eeprom_update_byte(EEPROM_ALARM_HOUR, alrm.hour);
	eeprom_update_byte(EEPROM_ALARM_MIN, alrm.min);
	eeprom_update_byte(EEPROM_ALARM_MON, getRawAlarmWeekday());

	alrm.eam = ALARM_NOEDIT;

	return;
}

uint8_t isAlarmMode(void)
{
	if (alrm.eam == ALARM_NOEDIT)
		return 0;
	return 1;
}

void editAlarm(void)
{
	if (alrm.eam >= ALARM_SUN)
		alrm.eam = ALARM_HOUR;
	else
		alrm.eam++;
}

void changeAlarm(int8_t diff)
{
	switch (alrm.eam) {
	case ALARM_HOUR:
		alrm.hour += diff;
		if (alrm.hour > 23)
			alrm.hour = 0;
		if (alrm.hour < 0)
			alrm.hour = 23;
		break;
	case ALARM_MIN:
		alrm.min += diff;
		if (alrm.min > 59)
			alrm.min = 0;
		if (alrm.min < 0)
			alrm.min = 59;
		break;
	default:
		alarm[alrm.eam] = !alarm[alrm.eam];
		break;
	}
}
