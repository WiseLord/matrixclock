#include "display.h"
#include "ds1307.h"
#include "max7219.h"
#include "ds18x20.h"
#include "mtimer.h"
#include "alarm.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

int8_t *dateTime;
int8_t *alarm;

static int8_t timeOld = 0;
static timeMode etmOld = T_NOEDIT;

static int8_t alarmOld = 0;
static alarmMode amOld = A_NOEDIT;

char strbuf[20];
static uint32_t timeMask = 0xFFFFFFFF;

static int8_t brHour;

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

const char p0[] PROGMEM = "сек";
const char p1[] PROGMEM = "мин";
const char p2[] PROGMEM = "час";
const char p3[] PROGMEM = "нед";
const char p4[] PROGMEM = "чис";
const char p5[] PROGMEM = "мес";
const char p6[] PROGMEM = "год";

const char *parLabel[] = {p0, p1, p2, p3, p4, p5, p6};

const char ws0[] PROGMEM = "вс";
const char ws1[] PROGMEM = "пн";
const char ws2[] PROGMEM = "вт";
const char ws3[] PROGMEM = "ср";
const char ws4[] PROGMEM = "чт";
const char ws5[] PROGMEM = "пт";
const char ws6[] PROGMEM = "сб";

const char *wsLabel[] = {p2, p1, ws1, ws2, ws3, ws4, ws5, ws6, ws0};

static int8_t brArray[24] = {0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 7, 9, 12, 15, 12, 9, 7, 5, 4, 3, 2, 1, 0};

static char *mkNumberString(int16_t value, uint8_t width, uint8_t prec, uint8_t lead)
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

static void loadDateString(void)
{
	max7219SetX(0);
	max7219LoadString(" ");
	max7219LoadStringPgm(weekLabel[dateTime[T_WEEK] % 7]);
	max7219LoadString(", ");
	max7219LoadString(mkNumberString(dateTime[T_DAY], 2, 0, ' '));
	max7219LoadString(" ");
	max7219LoadStringPgm(monthLabel[dateTime[T_MONTH] % 12]);
	max7219LoadString(" 20");
	max7219LoadString(mkNumberString(dateTime[T_YEAR], 2, 0, '0'));
	max7219LoadString("г. ");

	return;
}

static void loadTempString(void)
{
	uint8_t devCount = getDevCount();

	max7219SetX(0);
	if (devCount > 0) {
		max7219LoadString(mkNumberString(ds18x20GetTemp(0), 4, 1, ' '));
		max7219LoadString("·C в комнате");
	}
	if (devCount > 1) {
		max7219LoadString(", ");
		max7219LoadString(mkNumberString(ds18x20GetTemp(1), 4, 1, ' '));
		max7219LoadString("·C на улице");
	}

	return;
}

static uint8_t checkIfAlarmToday(void)
{
	return getRawAlarmWeekday() & (1 << (dateTime[T_WEEK] - 1));
}

void initBrightness(void)
{
	uint8_t i;

	for (i = 0; i < 24; i++)
		brArray[i] = eeprom_read_byte(EEPROM_BR_ADDR + i);

	return;
}

void writeBrightness(void)
{
	uint8_t i;

	for (i = 0; i < 24; i++)
		eeprom_update_byte(EEPROM_BR_ADDR + i, brArray[i]);

	return;
}

void showTime(uint32_t mask)
{
	static int8_t oldDateTime[7];

	dateTime = readTime();

	max7219SetX(0);
	max7219LoadString(mkNumberString(dateTime[T_HOUR], 2, 0, '0'));
	max7219SetX(12);
	max7219LoadString(mkNumberString(dateTime[T_MIN], 2, 0, '0'));
	max7219SetX(25);
	max7219LoadNumString(mkNumberString(dateTime[T_SEC], 2, 0, '0'));

	if (oldDateTime[T_HOUR] / 10 != dateTime[T_HOUR] / 10)
		mask  |= 0xF0000000;
	if (oldDateTime[T_HOUR] % 10 != dateTime[T_HOUR] % 10)
		mask  |= 0x07800000;
	if (oldDateTime[T_MIN] / 10 != dateTime[T_MIN] / 10)
		mask  |= 0x000F0000;
	if (oldDateTime[T_MIN] % 10 != dateTime[T_MIN] % 10)
		mask  |= 0x00007800;
	if (oldDateTime[T_SEC] / 10 != dateTime[T_SEC] / 10)
		mask  |= 0x00000070;
	if (oldDateTime[T_SEC] % 10 != dateTime[T_SEC] % 10)
		mask  |= 0x00000007;

	max7219PosData(10, dateTime[T_SEC] & 0x01 ? 0x00 : 0x24);
	max7219PosData(23, checkIfAlarmToday() ? dateTime[T_SEC] | 0x80 : dateTime[T_SEC]);

	max7219SwitchBuf(mask, MAX7219_EFFECT_SCROLL_DOWN);

	oldDateTime[T_HOUR] = dateTime[T_HOUR];
	oldDateTime[T_MIN] = dateTime[T_MIN];
	oldDateTime[T_SEC] = dateTime[T_SEC];

	return;
}

void scrollDate(void)
{
	loadDateString();
	max7219HwScroll(MAX7219_SCROLL_START);
	timeMask = 0xFFFFFFFF;

	return;
}

void scrollTemp(void)
{
	loadTempString();
	max7219HwScroll(MAX7219_SCROLL_START);
	timeMask = 0xFFFFFFFF;

	return;
}

void setTimeMask(uint32_t tmsk)
{
	timeMask = tmsk;
}

void showMainScreen(void)
{
	if (getScrollMode() == 0) {
		showTime(timeMask);
		if (dateTime[T_SEC] == 10) {
			scrollDate();
		} else if (dateTime[T_SEC] == 40) {
			scrollTemp();
		} else {
			timeMask = 0x0000000;
		}
	}

	return;
}

void showTimeEdit(int8_t ch_dir)
{
	uint32_t mask = 0x00000000;

	int8_t time;
	timeMode etm;

	readTime();
	etm = getEtm();
	time = getTime(etm);

	max7219SetX(0);
	max7219LoadString(mkNumberString(time, 2, 0, '0'));
	max7219SetX(13);
	max7219LoadStringPgm(parLabel[etm]);

	if (timeOld / 10 != time / 10)
		mask  |= 0xF0000000;
	if (timeOld % 10 != time % 10)
		mask  |= 0x07800000;

	if (etmOld != etm)
		mask |= 0xFFFFFFFF;

	if (ch_dir == PARAM_UP)
		max7219SwitchBuf(mask, MAX7219_EFFECT_SCROLL_DOWN);
	else
		max7219SwitchBuf(mask, MAX7219_EFFECT_SCROLL_UP);

	timeOld = time;
	etmOld = etm;

	return;
}

void resetEtmOld(void)
{
	etmOld = T_NOEDIT;

	return;
}

void resetAmOld(void)
{
	amOld = A_NOEDIT;

	return;
}

void showAlarm(uint32_t mask)
{
	static int8_t oldAlarm[3];

	alarm = readAlarm();

	max7219SetX(0);
	max7219LoadString(mkNumberString(alarm[A_HOUR], 2, 0, '0'));
	max7219SetX(12);
	max7219LoadString(mkNumberString(alarm[A_MIN], 2, 0, '0'));

	if (oldAlarm[A_HOUR] / 10 != alarm[A_HOUR] / 10)
		mask  |= 0xF0000000;
	if (oldAlarm[A_HOUR] % 10 != alarm[A_HOUR] % 10)
		mask  |= 0x07800000;
	if (oldAlarm[A_MIN] / 10 != alarm[A_MIN] / 10)
		mask  |= 0x000F0000;
	if (oldAlarm[A_MIN] % 10 != alarm[A_MIN] % 10)
		mask  |= 0x00007800;

	max7219PosData(10, 0x24);
	max7219PosData(23, getRawAlarmWeekday());

	max7219SwitchBuf(mask, MAX7219_EFFECT_SCROLL_DOWN);

	oldAlarm[A_HOUR] = alarm[A_HOUR];
	oldAlarm[A_MIN] = alarm[A_MIN];

	return;
}

void showAlarmEdit(int8_t ch_dir)
{
	uint32_t mask = 0x00000000;

	int8_t alarm;
	alarmMode am;

	readTime();
	am = getAlarmMode();
	alarm = getAlarm(am);

	max7219SetX(0);

	switch (am) {
	case A_HOUR:
		max7219LoadString(mkNumberString(alarm, 2, 0, '0'));
		max7219SetX(13);
		max7219LoadStringPgm(p2);
		break;
	case A_MIN:
		max7219LoadString(mkNumberString(alarm, 2, 0, '0'));
		max7219SetX(13);
		max7219LoadStringPgm(p1);
		break;
	default:
		if (alarm)
			max7219LoadString(" ♩ ");
		else {
			max7219LoadString("   ");
		}
		max7219SetX(13);
		max7219LoadStringPgm(wsLabel[am]);
		break;
	}

	if (alarmOld / 10 != alarm / 10)
		mask  |= 0xF0000000;
	if (alarmOld % 10 != alarm % 10)
		mask  |= 0x07C00000;

	if (amOld != am)
		mask |= 0xFFFFFFFF;

	if (ch_dir == PARAM_UP)
		max7219SwitchBuf(mask, MAX7219_EFFECT_SCROLL_DOWN);
	else
		max7219SwitchBuf(mask, MAX7219_EFFECT_SCROLL_UP);

	alarmOld = alarm;
	amOld = am;

	return;
}

void setBrightnessHour(void)
{
	dateTime = readTime();
	brHour = dateTime[T_HOUR];

	max7219SetBrightness(brArray[brHour]);

	return;
}

void incBrightnessHour(void)
{
	brHour++;
	if (brHour >= 24)
		brHour = 0;

	max7219SetBrightness(brArray[brHour]);

	return;
}

void changeBrightness(int8_t diff)
{
	brArray[brHour] += diff;

	if (brArray[brHour] >= MAX7219_MAX_BRIGHTNESS)
		brArray[brHour] = MAX7219_MAX_BRIGHTNESS;
	if (brArray[brHour] <= MAX7219_MIN_BRIGHTNESS)
		brArray[brHour] = MAX7219_MIN_BRIGHTNESS;

	max7219SetBrightness(brArray[brHour]);

	return;
}

void showBrightness(int8_t ch_dir, uint32_t mask)
{
	static int8_t oldHour;
	static uint8_t oldBrightness;
	uint8_t i;

	max7219SetX(0);
	max7219LoadString(mkNumberString(brHour, 2, 0, '0'));
	max7219SetX(15);
	max7219LoadString(mkNumberString(brArray[brHour], 2, 0, '0'));

	if (oldHour / 10 != brHour / 10)
		mask  |= 0xF0000000;
	if (oldHour % 10 != brHour % 10)
		mask  |= 0x07800000;
	if (oldBrightness / 10 != brArray[brHour] / 10)
		mask  |= 0x0001E000;
	if (oldBrightness % 10 != brArray[brHour] % 10)
		mask  |= 0x00000F00;

	for (i = 10; i <= 13; i++)
		max7219PosData(i, 0x7F);

	max7219SwitchBuf(mask, MAX7219_EFFECT_SCROLL_DOWN);

	oldHour = brHour;
	oldBrightness = brArray[brHour];

	return;
}

void checkAlarmAndBrightness(void)
{
	static uint8_t alarmFlag = 1;

	/* Check alarm */
	dateTime = readTime();
	alarm = readAlarm();

	if (dateTime[T_HOUR] == alarm[A_HOUR] && dateTime[T_MIN] == alarm[A_MIN]) {
		if (checkIfAlarmToday()) {
			if (getBeepTimer() == 0 && alarmFlag) {
				alarmFlag = 0;
				startBeeper(60000);
			}
		}
	} else {
		alarmFlag = 1;
	}

	/* Check brightness */
	max7219SetBrightness(brArray[dateTime[T_HOUR]]);

	return;
}
