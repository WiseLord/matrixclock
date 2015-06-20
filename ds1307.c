#include "ds1307.h"

#include "i2csw.h"

static RTC_type rtc;

static uint8_t _etm = NOEDIT;

uint8_t getEtm()
{
	return _etm;
}

static void calcWeekDay(void)
{
	uint8_t a, y, m;

	a = (rtc.month > 2 ? 0 : 1);
	y = 12 + rtc.year - a;
	m = rtc.month + 12 * a - 2;

	rtc.wday = (rtc.date + y + (y / 4) + ((31 * m) / 12)) % 7;
	if (rtc.wday == 0)
		rtc.wday = 7;

	return;
}

static uint8_t daysInMonth()
{
	uint8_t m = rtc.month;

	if (m == 2) {
		if (rtc.year & 0x03)
			return 28;
		return 29;
	}

	if (m == 4 || m == 6 || m == 9 || m == 11) {
		return 30;
	}

	return 31;
}

int8_t *readTime(void)
{
	uint8_t temp;
	uint8_t i;

	I2CswStart(DS1307_ADDR);
	I2CswWriteByte(DS1307_SEC);
	I2CswStart(DS1307_ADDR | I2C_READ);
	for (i = DS1307_SEC; i < DS1307_YEAR; i++) {
		temp = I2CswReadByte(I2C_ACK);
		*((uint8_t*)(&rtc) + i) = BD2D(temp);
	}
	temp = I2CswReadByte(I2C_NOACK);
	rtc.year = BD2D(temp);
	I2CswStop();

	return (int8_t*)&rtc;
}

static void writeTime(void)
{
	uint8_t i;

	if (rtc.date > daysInMonth())
		rtc.date = daysInMonth();
	if (_etm >= DS1307_DATE)
		calcWeekDay();

	I2CswStart(DS1307_ADDR);
	I2CswWriteByte(DS1307_SEC);
	for (i = DS1307_SEC; i <= DS1307_YEAR; i++)
		I2CswWriteByte(D2BD(*((uint8_t*)(&rtc) + i)));
	I2CswStop();

	return;
}

void stopEditTime(void)
{
	_etm = NOEDIT;

	return;
}

uint8_t isETM(void)
{
	if (_etm == NOEDIT)
		return 0;
	return 1;
}

void editTime(void)
{
	switch (_etm) {
	case DS1307_HOUR:
	case DS1307_MIN:
		_etm--;
		break;
	case DS1307_SEC:
		_etm = DS1307_DATE;
		break;
	case DS1307_DATE:
	case DS1307_MONTH:
		_etm++;
		break;
	default:
		_etm = DS1307_HOUR;
		break;
	}

	return;
}

void changeTime(int8_t diff)
{
	switch (_etm) {
	case DS1307_HOUR:
		rtc.hour += diff;
		if (rtc.hour > 23)
			rtc.hour = 0;
		if (rtc.hour < 0)
			rtc.hour = 23;
		break;
	case DS1307_MIN:
		rtc.min += diff;
		if (rtc.min > 59)
			rtc.min = 0;
		if (rtc.min < 0)
			rtc.min = 59;
		break;
	case DS1307_SEC:
		rtc.sec = 0;
		break;
	case DS1307_DATE:
		rtc.date += diff;
		if (rtc.date > daysInMonth())
			rtc.date = 1;
		if (rtc.date < 1)
			rtc.date = daysInMonth();
		break;
	case DS1307_MONTH:
		rtc.month += diff;
		if (rtc.month > 12)
			rtc.month = 1;
		if (rtc.month < 1)
			rtc.month = 12;
		break;
	case DS1307_YEAR:
		rtc.year += diff;
		if (rtc.year > 99)
			rtc.year = 0;
		if (rtc.year < 0)
			rtc.year = 99;
		break;
	default:
		break;
	}
	writeTime();
}
