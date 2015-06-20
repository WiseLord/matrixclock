#include "alarm.h"

#include "eeprom.h"
#include <avr/eeprom.h>

static int8_t alarm[9];
static uint8_t _am = ALARM_NOEDIT;

void initAlarm(void)
{
	int8_t i, rawAlarm;

	alarm[ALARM_HOUR] = eeprom_read_byte(EEPROM_A_HOUR);
	alarm[ALARM_MIN] = eeprom_read_byte(EEPROM_A_MIN);

	rawAlarm = eeprom_read_byte(EEPROM_A_DAYS) & 0x7F;
	for (i = 0; i <= ALARM_SUNDAY - ALARM_MONDAY; i++)
		alarm[i + ALARM_MONDAY] = rawAlarm & (1<<i);

	return;
}

int8_t getAlarm(uint8_t am)
{
	return alarm[am];
}

uint8_t getAlarmMode()
{
	return _am;
}

int8_t *readAlarm(void)
{
	return alarm;
}

int8_t getRawAlarmWeekday(void)
{
	int8_t rawWeekday = 0x00;
	uint8_t i;

	for (i = 0; i <= ALARM_SUNDAY - ALARM_MONDAY; i++) {
		rawWeekday >>= 1;
		if (alarm[ALARM_MONDAY + i])
			rawWeekday |= 0x40;
	}

	return rawWeekday;
}

void writeAlarm(void)
{
	eeprom_update_byte(EEPROM_A_HOUR, alarm[ALARM_HOUR]);
	eeprom_update_byte(EEPROM_A_MIN, alarm[ALARM_MIN]);
	eeprom_update_byte(EEPROM_A_DAYS, getRawAlarmWeekday());

	_am = ALARM_NOEDIT;

	return;
}

uint8_t isAlarmMode(void)
{
	if (_am == ALARM_NOEDIT)
		return 0;
	return 1;
}

void editAlarm(void)
{
	if (_am >= ALARM_SUNDAY)
		_am = ALARM_HOUR;
	else
		_am++;
}

void changeAlarm(int8_t diff)
{
	switch (_am) {
	case ALARM_HOUR:
		alarm[ALARM_HOUR] += diff;
		if (alarm[ALARM_HOUR] > 23)
			alarm[ALARM_HOUR] = 0;
		if (alarm[ALARM_HOUR] < 0)
			alarm[ALARM_HOUR] = 23;
		break;
	case ALARM_MIN:
		alarm[ALARM_MIN] += diff;
		if (alarm[ALARM_MIN] > 59)
			alarm[ALARM_MIN] = 0;
		if (alarm[ALARM_MIN] < 0)
			alarm[ALARM_MIN] = 59;
		break;
	default:
		alarm[_am] = !alarm[_am];
		break;
	}
}
