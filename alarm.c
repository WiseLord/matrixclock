#include "alarm.h"
#include <avr/eeprom.h>

static int8_t alarm[9];
static alarmMode _am = A_NOEDIT;

void initAlarm(void)
{
	int8_t rawAlarm = 0x00;

	alarm[A_HOUR] = eeprom_read_byte(EEPROM_A_HOUR);
	alarm[A_MIN] = eeprom_read_byte(EEPROM_A_MIN);

	rawAlarm = eeprom_read_byte(EEPROM_A_DAYS) & 0x7F;

	alarm[A_MONDAY] = rawAlarm & 0x01;
	alarm[A_TUESDAY] = rawAlarm & 0x02;
	alarm[A_WEDNESDAY] = rawAlarm & 0x04;
	alarm[A_THURSDAY] = rawAlarm & 0x08;
	alarm[A_FRIDAY] = rawAlarm & 0x10;
	alarm[A_SATURDAY] = rawAlarm & 0x20;
	alarm[A_SUNDAY] = rawAlarm & 0x40;

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

int8_t getRawWeekday(void)
{
	int8_t rawWeekday = 0x00;

	if (alarm[A_MONDAY]) rawWeekday |= 0x01;
	if (alarm[A_TUESDAY]) rawWeekday |= 0x02;
	if (alarm[A_WEDNESDAY]) rawWeekday |= 0x04;
	if (alarm[A_THURSDAY]) rawWeekday |= 0x08;
	if (alarm[A_FRIDAY]) rawWeekday |= 0x10;
	if (alarm[A_SATURDAY]) rawWeekday |= 0x20;
	if (alarm[A_SUNDAY]) rawWeekday |= 0x40;

	return rawWeekday;
}

void writeAlarm(void)
{
	eeprom_update_byte(EEPROM_A_HOUR, alarm[A_HOUR]);
	eeprom_update_byte(EEPROM_A_MIN, alarm[A_MIN]);
	eeprom_update_byte(EEPROM_A_DAYS, getRawWeekday());

	return;
}

void stopEditAlarm(void)
{
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
	readAlarm();
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
