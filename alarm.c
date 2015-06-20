#include "alarm.h"

#include "eeprom.h"
#include <avr/eeprom.h>

static int8_t alarm[9];
static alarmMode _am = A_NOEDIT;

void initAlarm(void)
{
	int8_t i, rawAlarm;

	alarm[A_HOUR] = eeprom_read_byte(EEPROM_A_HOUR);
	alarm[A_MIN] = eeprom_read_byte(EEPROM_A_MIN);

	rawAlarm = eeprom_read_byte(EEPROM_A_DAYS) & 0x7F;
	for (i = 0; i <= A_SUNDAY - A_MONDAY; i++)
		alarm[i + A_MONDAY] = rawAlarm & (1<<i);

	return;
}

int8_t getAlarm(alarmMode am)
{
	return alarm[am];
}

alarmMode getAlarmMode()
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

	for (i = 0; i <= A_SUNDAY - A_MONDAY; i++) {
		rawWeekday >>= 1;
		if (alarm[A_MONDAY + i])
			rawWeekday |= 0x40;
	}

	return rawWeekday;
}

void writeAlarm(void)
{
	eeprom_update_byte(EEPROM_A_HOUR, alarm[A_HOUR]);
	eeprom_update_byte(EEPROM_A_MIN, alarm[A_MIN]);
	eeprom_update_byte(EEPROM_A_DAYS, getRawAlarmWeekday());

	_am = A_NOEDIT;

	return;
}

uint8_t isAlarmMode(void)
{
	if (_am == A_NOEDIT)
		return 0;
	return 1;
}

void editAlarm(void)
{
	if (_am >= A_SUNDAY)
		_am = A_HOUR;
	else
		_am++;
}

void changeAlarm(int8_t diff)
{
	switch (_am) {
	case A_HOUR:
		alarm[A_HOUR] += diff;
		if (alarm[A_HOUR] > 23)
			alarm[A_HOUR] = 0;
		if (alarm[A_HOUR] < 0)
			alarm[A_HOUR] = 23;
		break;
	case A_MIN:
		alarm[A_MIN] += diff;
		if (alarm[A_MIN] > 59)
			alarm[A_MIN] = 0;
		if (alarm[A_MIN] < 0)
			alarm[A_MIN] = 59;
		break;
	default:
		alarm[_am] = !alarm[_am];
		break;
	}
}
