#include "ds1307.h"

#include "i2c.h"

static int8_t time[7];
static timeMode _etm = T_NOEDIT;

int8_t getTime(timeMode tm)
{
	return time[tm];
}

timeMode getEtm()
{
	return _etm;
}

static void calcWeekDay(void)
{
	uint8_t a, y, m;

	a = (time[T_MONTH] > 2 ? 0 : 1);
	y = 12 + time[T_YEAR] - a;
	m = time[T_MONTH] + 12 * a - 2;

	time[T_WEEK] = (time[T_DAY] + y + (y / 4) - 1 + ((31 * m) / 12)) % 7;
	if (time[T_WEEK] == 0)
		time[T_WEEK] = 7;

	return;
}

static uint8_t daysInMonth()
{
	if (time[T_MONTH] == 2) {
		if (time[T_YEAR] & 0x03)
			return 28;
		return 29;
	}

	if (time[T_MONTH] == 4 || time[T_MONTH] == 6 || time[T_MONTH] == 9 || time[T_MONTH] == 11) {
		return 30;
	}

	return 31;
}

int8_t *readTime(void)
{
	uint8_t temp;
	uint8_t i;

	for (i = T_SEC; i <= T_YEAR; i++) {
		I2CStart(DS1307_ADDR);
		I2CWriteByte(i);
		I2CStart(DS1307_ADDR | I2C_READ);
		I2CReadByte(&temp, I2C_NOACK);
		I2CStop();
		time[i] = BD2D(temp);
	}

	return time;
}

static void writeTime(void)
{
	uint8_t i;

	if (time[T_DAY] > daysInMonth())
		time[T_DAY] = daysInMonth();
	if (_etm >= T_DAY)
		calcWeekDay();

	for (i = T_SEC; i <= T_YEAR; i++) {
		I2CStart(DS1307_ADDR);
		I2CWriteByte(i);
		I2CWriteByte(D2BD(time[i]));
		I2CStop();
	}

	return;
}

void stopEditTime(void)
{
	_etm = T_NOEDIT;

	return;
}

uint8_t isETM(void)
{
	if (_etm == T_NOEDIT)
		return 0;
	return 1;
}

void editTime(void)
{
	switch (_etm) {
	case T_NOEDIT:
		_etm = T_HOUR;
		break;
	case T_HOUR:
		_etm = T_MIN;
		break;
	case T_MIN:
		_etm = T_SEC;
		break;
	case T_SEC:
		_etm = T_DAY;
		break;
	case T_DAY:
		_etm = T_MONTH;
		break;
	case T_MONTH:
		_etm = T_YEAR;
		break;
	case T_YEAR:
		_etm = T_HOUR;
		break;
	default:
		_etm = T_NOEDIT;
		break;
	}
}

void changeTime(int8_t diff)
{
	readTime();
	switch (_etm) {
	case T_HOUR:
		time[T_HOUR] += diff;
		if (time[T_HOUR] > 23)
			time[T_HOUR] = 0;
		if (time[T_HOUR] < 0)
			time[T_HOUR] = 23;
		break;
	case T_MIN:
		time[T_MIN] += diff;
		if (time[T_MIN] > 59)
			time[T_MIN] = 0;
		if (time[T_MIN] < 0)
			time[T_MIN] = 59;
		break;
	case T_SEC:
		time[T_SEC] = 0;
		break;
	case T_DAY:
		time[T_DAY] += diff;
		if (time[T_DAY] > daysInMonth())
			time[T_DAY] = 1;
		if (time[T_DAY] < 1)
			time[T_DAY] = daysInMonth();
		break;
	case T_MONTH:
		time[T_MONTH] += diff;
		if (time[T_MONTH] > 12)
			time[T_MONTH] = 1;
		if (time[T_MONTH] < 1)
			time[T_MONTH] = 12;
		break;
	case T_YEAR:
		time[T_YEAR] += diff;
		if (time[T_YEAR] > 99)
			time[T_YEAR] = 0;
		if (time[T_YEAR] < 0)
			time[T_YEAR] = 99;
		break;
	default:
		break;
	}
	writeTime();
}
