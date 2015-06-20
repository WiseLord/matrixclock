#include "ds1307.h"

#include "i2csw.h"

RTC_type rtc;

static RTC_type rtcMin = {0, 0, 0, 1, 1, 1, 1, RTC_NOEDIT};
static RTC_type rtcMax = {59, 59, 23, 7, 31, 12, 99, RTC_NOEDIT};

static void rtcWeekDay(void)
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

static uint8_t rtcDaysInMonth(void)
{
	uint8_t m = rtc.month;

	if (m == 2) {
		if (rtc.year & 0x03)
			return 28;
		return 29;
	}

	if (m == 4 || m == 6 || m == 9 || m == 11)
		return 30;

	return 31;
}

void rtcReadTime(void)
{
	uint8_t temp;
	uint8_t i;

	I2CswStart(DS1307_ADDR);
	I2CswWriteByte(RTC_SEC);
	I2CswStart(DS1307_ADDR | I2C_READ);
	for (i = RTC_SEC; i < RTC_YEAR; i++) {
		temp = I2CswReadByte(I2C_ACK);
		*((int8_t*)&rtc + i) = BD2D(temp);
	}
	temp = I2CswReadByte(I2C_NOACK);
	rtc.year = BD2D(temp);
	I2CswStop();

	return;
}

static void rtcSaveTime(void)
{
	uint8_t i;

	I2CswStart(DS1307_ADDR);
	I2CswWriteByte(RTC_SEC);
	for (i = RTC_SEC; i <= RTC_YEAR; i++)
		I2CswWriteByte(D2BD(*((int8_t*)&rtc + i)));
	I2CswStop();

	return;
}

void rtcStopEditTime(void)
{
	rtc.etm = RTC_NOEDIT;

	return;
}

void rtcNextEditParam(void)
{
	switch (rtc.etm) {
	case RTC_HOUR:
	case RTC_MIN:
		rtc.etm--;
		break;
	case RTC_SEC:
		rtc.etm = RTC_DATE;
		break;
	case RTC_DATE:
	case RTC_MONTH:
		rtc.etm++;
		break;
	default:
		rtc.etm = RTC_HOUR;
		break;
	}

	return;
}

void rtcChangeTime(int8_t diff)
{
	int8_t *time = (int8_t*)&rtc + rtc.etm;
	int8_t timeMax = *((int8_t*)&rtcMax + rtc.etm);
	int8_t timeMin = *((int8_t*)&rtcMin + rtc.etm);

	if (rtc.etm == RTC_DATE)
		timeMax = rtcDaysInMonth();

	*time += diff;

	if (*time > timeMax)
		*time = timeMin;
	if (*time < timeMin)
		*time = timeMax;

	rtcWeekDay();

	rtcSaveTime();
}
