#include "display.h"
#include "ds1307.h"
#include "max7219.h"
#include "ds18x20.h"
#include "mtimer.h"

#include <avr/pgmspace.h>

int8_t *dateTime;

char strbuf[20];
static uint32_t timeMask = 0xFFFFFF;

const char wd0[] PROGMEM = "Воскресенье";
const char wd1[] PROGMEM = "Понедельник";
const char wd2[] PROGMEM = "Вторник";
const char wd3[] PROGMEM = "Среда";
const char wd4[] PROGMEM = "Четверг";
const char wd5[] PROGMEM = "Пятница";
const char wd6[] PROGMEM = "Суббота";

const char *weekLabel[] = {wd0, wd1, wd2, wd3, wd4, wd5, wd6};

const char m0[] PROGMEM = "декабря";
const char m1[] PROGMEM = "января";
const char m2[] PROGMEM = "февраля";
const char m3[] PROGMEM = "марта";
const char m4[] PROGMEM = "апреля";
const char m5[] PROGMEM = "мая";
const char m6[] PROGMEM = "июня";
const char m7[] PROGMEM = "июля";
const char m8[] PROGMEM = "августа";
const char m9[] PROGMEM = "сентября";
const char m10[] PROGMEM = "октября";
const char m11[] PROGMEM = "ноября";

const char *monthLabel[] = {m0, m1, m2, m3, m4, m5, m6, m7, m8, m9, m10, m11};

char *mkNumberString(int16_t value, uint8_t width, uint8_t prec, uint8_t lead)
{
	uint8_t sign = lead;
	int8_t pos;

	if (value < 0) {
		sign = '-';
		value = -value;
	}

	/* Clear buffer and go to it's tail */
	for (pos = 0; pos < width + prec; pos++)
		strbuf[pos] = lead;
	strbuf[width + prec] = '\0';
	pos = width + prec - 1;

	/* Fill buffer from right to left */
	while (value > 0 || pos > width - 2) {
		if (prec && (width - pos - 1 == 0))
			strbuf[pos--] = '.';
		strbuf[pos] = value % 10 + 0x30;
		pos--;
		value /= 10;
	}

	if (pos >= 0)
		strbuf[pos] = sign;

	return strbuf;
}


void showTime(uint32_t mask)
{
	static int8_t oldDateTime[7];

	dateTime = readTime();

	max7219SetX(0);
	max7219LoadString(mkNumberString(dateTime[HOUR], 2, 0, '0'));
	max7219SetX(12);
	max7219LoadString(mkNumberString(dateTime[MIN], 2, 0, '0'));

	if (oldDateTime[HOUR] / 10 != dateTime[HOUR] / 10)
		mask  |= 0xF00000;
	if (oldDateTime[HOUR] % 10 != dateTime[HOUR] % 10)
		mask  |= 0x078000;
	if (oldDateTime[MIN] / 10 != dateTime[MIN] / 10)
		mask  |= 0x000F00;
	if (oldDateTime[MIN] % 10 != dateTime[MIN] % 10)
		mask  |= 0x000078;

	max7219PosData(10, dateTime[SEC] & 0x01 ? 0x00 : 0x24);
	max7219PosData(23, dateTime[SEC]);

	max7219SwitchBuf(mask, MAX7219_EFFECT_SCROLL_DOWN);

	oldDateTime[HOUR] = dateTime[HOUR];
	oldDateTime[MIN] = dateTime[MIN];

	return;
}

void loadDateString(void)
{
	max7219SetX(0);
	max7219LoadString(" ");
	max7219LoadStringPgm(weekLabel[dateTime[WEEK] % 7]);
	max7219LoadString(", ");
	max7219LoadString(mkNumberString(dateTime[DAY], 2, 0, ' '));
	max7219LoadString(" ");
	max7219LoadStringPgm(monthLabel[dateTime[MONTH] % 12]);
	max7219LoadString(" 20");
	max7219LoadString(mkNumberString(dateTime[YEAR], 2, 0, ' '));
	max7219LoadString("г. ");

	return;
}

void loadTempString(void)
{
	max7219SetX(0);
	max7219LoadString(" ");
	max7219LoadString(mkNumberString(ds18x20GetTemp(0), 4, 1, ' '));
	max7219LoadString("·C в комнате, ");
	max7219LoadString(mkNumberString(ds18x20GetTemp(1), 4, 1, ' '));
	max7219LoadString("·C на улице");

	return;
}

void scrollDate(void)
{
	loadDateString();
	max7219StartHwScroll();
	timeMask = 0xFFFFFF;

	return;
}

void scrollTemp(void)
{
	loadTempString();
	max7219StartHwScroll();
	timeMask = 0xFFFFFF;

	return;
}

void showMainScreen(void)
{
	if (getTempStartTimer() == 0) {
		setTempStartTimer(TEMP_POLL_INTERVAL);
		ds18x20Process();
	}

	if (getScrollMode() == 0) {
		showTime(timeMask);
		if (dateTime[SEC] == 10) {
			scrollDate();
		} else if (dateTime[SEC] == 40) {
			scrollTemp();
		} else {
			timeMask = 0x000000;
		}
	}

	return;
}
