#include "display.h"

#include "ds1307.h"
#include "matrix.h"
#include "ds18x20.h"
#include "mtimer.h"
#include "alarm.h"
#include "eeprom.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

int8_t *alarm;

char strbuf[8];
static uint32_t timeMask = MASK_ALL;

uint8_t *txtLabels[LABEL_END];				/* Array with text label pointers */

static int8_t brMax;

static uint8_t bigNum = 0;
static uint8_t hourZero = '0';

static uint8_t etmOld = RTC_NOEDIT;
static alarmMode amOld = A_NOEDIT;
static uint8_t alarmFlag = 1;

static void startAlarm(uint16_t duration)
{
	if (getBeepTimer() == 0 && alarmFlag) {
		alarmFlag = 0;
		startBeeper(duration);
	}

	return;
}
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
	matrixLoadStringEeprom(txtLabels[(LABEL_SATURDAY + rtc.wday) % 7]);
	matrixLoadString(", ");
	matrixLoadString(mkNumberString(rtc.date, 2, 0, ' '));
	matrixLoadString(" ");
	matrixLoadStringEeprom(txtLabels[LABEL_DECEMBER + rtc.month % 12]);
	matrixLoadString(" ");
	matrixLoadString(mkNumberString(2000 + rtc.year, 4, 0, '0'));
	matrixLoadString(" ");
	matrixLoadStringEeprom(txtLabels[LABEL_Y]);

	return;
}

static void loadTempString(void)
{
	uint8_t i;

	matrixLoadStringEeprom(txtLabels[LABEL_TEMPERATURE]);

	for (i = 0; i < ds18x20GetDevCount(); i++) {
		if (i > 0)
			matrixLoadString(", ");
		matrixLoadString(mkNumberString(ds18x20GetTemp(i), 4, 1, ' '));
		matrixLoadStringEeprom(txtLabels[LABEL_DEGREE]);
		matrixLoadString(" ");
		matrixLoadStringEeprom(txtLabels[LABEL_TEMP1 + i]);
	}

	return;
}

void displayInit(void)
{
	uint8_t i;
	uint8_t *addr;

	/* Read text labels saved in EEPROM */
	addr = EEPROM_LABELS;
	i = 0;
	while (i < LABEL_END && addr < (uint8_t*)EEPROM_SIZE) {
		if (eeprom_read_byte(addr) != '\0') {
			txtLabels[i] = addr;
			addr++;
			i++;
			while (eeprom_read_byte(addr) != '\0' &&
				   addr < (uint8_t*)EEPROM_SIZE) {
				addr++;
			}
		} else {
			addr++;
		}
	}

	bigNum = eeprom_read_byte(EEPROM_BIGNUM);
	hourZero = eeprom_read_byte(EEPROM_HOURZERO);
	brMax = eeprom_read_byte(EEPROM_BR_MAX);

	return;
}

void displaySwitchBigNum(void)
{
	if (++bigNum >= NUM_END)
		bigNum = NUM_NORMAL;

	eeprom_update_byte(EEPROM_BIGNUM, bigNum);

	return;
}

void displaySwitchHourZero(void) {
	if (hourZero == '0')
		hourZero = ' ';
	else
		hourZero = '0';

	eeprom_update_byte(EEPROM_HOURZERO, hourZero);
	return;
}

static void showHMColon(uint8_t step, uint8_t pos)
{
	uint8_t value;

	if (step == 4)
		value = 0x62;
	else if (step == 3)
		value = 0x46;
	else if (step == 2)
		value = 0x36;
	else if (step == 1)
		value = 0x26;
	else
		value = 0x32;

	matrixPosData(pos, value);
	matrixPosData(pos + 1, value);

	return;
}

static uint8_t calcBrightness(void)
{
	int8_t br = 0;

	if (ADCH > 1) {							/* We have photoresistor */
		if (br > (ADCH >> 4))
			br--;
		if (br < (ADCH >> 4))
			br++;
	} else {								/* Calculate br(hour) */
		if (rtc.hour <= 12)
			br = (rtc.hour * 2) - 25 + brMax;
		else
			br = 31 - (rtc.hour * 2) + brMax;
	}

	if (br > MATRIX_MAX_BRIGHTNESS)
		br = MATRIX_MAX_BRIGHTNESS;
	if (br < MATRIX_MIN_BRIGHTNESS)
		br = MATRIX_MIN_BRIGHTNESS;

	return br;
}

void showTime(uint32_t mask)
{
	static uint8_t oldHourTens, oldHourUnits, oldMinTens, oldMinUnits, oldSecTens, oldSecUnits;
	uint8_t digit;

	etmOld = RTC_NOEDIT;

	if (bigNum == NUM_EXTRA)
		matrixSetX(1);
	else
		matrixSetX(0);
	mkNumberString(rtc.hour, 2, 0, hourZero);
	matrixLoadNumString(strbuf, bigNum);

	if (bigNum == NUM_EXTRA)
		matrixSetX(18);
	else
		matrixSetX(13);
	mkNumberString(rtc.min, 2, 0, '0');
	matrixLoadNumString(strbuf, bigNum);

	if (bigNum != NUM_EXTRA) {
		matrixSetX(25);
		matrixLoadNumString(mkNumberString(rtc.sec, 2, 0, '0'), NUM_SMALL);
	}

	digit = rtc.hour / 10;
	if (oldHourTens != digit) {
		if (bigNum == NUM_EXTRA)
			mask |= MASK_EXTRAHOUR_TENS;
		else if (bigNum == NUM_BIG)
			mask |= MASK_BIGHOUR_TENS;
		else
			mask |= MASK_HOUR_TENS;
	}
	oldHourTens = digit;

	digit = rtc.hour % 10;
	if (oldHourUnits != digit) {
		if (bigNum == NUM_EXTRA)
			mask |= MASK_EXTRAHOUR_UNITS;
		else if (bigNum == NUM_BIG)
			mask |= MASK_BIGHOUR_UNITS;
		else
			mask |= MASK_HOUR_UNITS;
	}
	oldHourUnits = digit;

	digit = rtc.min / 10;
	if (oldMinTens != digit) {
		if (bigNum == NUM_EXTRA)
			mask |= MASK_EXTRAMIN_TENS;
		else if (bigNum == NUM_BIG)
			mask |= MASK_BIGMIN_TENS;
		else
			mask |= MASK_MIN_TENS;
	}
	oldMinTens = digit;

	digit = rtc.min % 10;
	if (oldMinUnits != digit) {
		if (bigNum == NUM_EXTRA)
			mask |= MASK_EXTRAMIN_UNITS;
		else if (bigNum == NUM_BIG)
			mask |= MASK_BIGMIN_UNITS;
		else
			mask |= MASK_MIN_UNITS;
	}
	oldMinUnits = digit;

	if (bigNum != NUM_EXTRA) {
		digit = rtc.sec / 10;
		if (oldSecTens != digit)
			mask |= MASK_SEC_TENS;
		oldSecTens = digit;

		digit = rtc.sec % 10;
		if (oldSecUnits != digit)
			mask |= MASK_SEC_UNITS;
		oldSecUnits = digit;
	}
	digit = rtc.sec & 0x01;
	if (bigNum == NUM_BIG) {
		matrixPosData(11, (!digit) << 7);
		matrixPosData(12, digit << 7);
	} else if (bigNum == NUM_EXTRA) {
		matrixPosData(0, 0x00);
		showHMColon(digit + 3, 15);
	} else {
		showHMColon(digit, 10);
		matrixPosData(23, getRawAlarmWeekday());
	}

	matrixSwitchBuf(mask, MATRIX_EFFECT_SCROLL_DOWN);

	return;
}

void scroll(uint8_t type)
{
	matrixSetX(0);
	matrixClearBufTail();
	matrixSwitchBuf(MASK_ALL, MATRIX_EFFECT_SCROLL_BOTH);
	if (type == SCROLL_DATE)
		loadDateString();
	else
		loadTempString();
	matrixHwScroll(MATRIX_SCROLL_START);
	timeMask = MASK_ALL;

	return;
}

void setTimeMask(uint32_t tmsk)
{
	timeMask = tmsk;

	return;
}

void showMainScreen(void)
{
	if (matrixGetScrollMode() == 0) {
		showTime(timeMask);
		if (rtc.sec == 20) {
			if (rtc.min & 0x01)
				scroll(SCROLL_DATE);
			else
				scroll(SCROLL_TEMP);
		} else {
			timeMask = MASK_NONE;
		}
	}

	return;
}

void showTimeEdit(int8_t ch_dir)
{
	uint32_t mask = MASK_NONE;

	static int8_t timeOld = 0;
	uint8_t time = *((uint8_t*)(&rtc) + rtc.etm);

	matrixSetX(0);
	matrixLoadString(mkNumberString(time, 2, 0, ' '));
	matrixSetX(13);
	matrixLoadStringEeprom(txtLabels[LABEL_SECOND + rtc.etm]);

	if (timeOld / 10 != time / 10)
		mask  |= MASK_HOUR_TENS;
	if (timeOld % 10 != time % 10)
		mask  |= MASK_HOUR_UNITS;

	if (etmOld != rtc.etm)
		mask |= MASK_ALL;

	if (ch_dir == PARAM_UP)
		matrixSwitchBuf(mask, MATRIX_EFFECT_SCROLL_DOWN);
	else
		matrixSwitchBuf(mask, MATRIX_EFFECT_SCROLL_UP);

	timeOld = time;
	etmOld = rtc.etm;

	return;
}

void showAlarm(uint32_t mask)
{
	static uint8_t oldHourTens, oldHourUnits, oldMinTens, oldMinUnits;
	uint8_t digit;

	uint8_t hour = alarm[A_HOUR];
	uint8_t min = alarm[A_MIN];

	amOld = A_NOEDIT;

	matrixSetX(0);
	matrixLoadString(mkNumberString(hour, 2, 0, ' '));
	matrixSetX(13);
	matrixLoadString(mkNumberString(min, 2, 0, '0'));
	matrixSetX(26);
	matrixLoadString("\xA0");

	digit = hour / 10;
	if (oldHourTens != digit)
		mask  |= MASK_HOUR_TENS;
	oldHourTens = digit;

	digit = hour % 10;
	if (oldHourUnits != digit)
		mask  |= MASK_HOUR_UNITS;
	oldHourUnits = digit;

	digit = min / 10;
	if (oldMinTens != digit)
		mask  |= MASK_MIN_TENS;
	oldMinTens = digit;

	digit = min % 10;
	if (oldMinUnits != digit)
		mask  |= MASK_MIN_UNITS;
	oldMinUnits = digit;

	showHMColon(2, 10);
	matrixPosData(23, getRawAlarmWeekday());

	matrixSwitchBuf(mask, MATRIX_EFFECT_SCROLL_DOWN);

	return;
}

void showAlarmEdit(int8_t ch_dir)
{
	uint32_t mask = MASK_NONE;

	int8_t alarm;
	alarmMode am;
	static int8_t alarmOld = 0;

	am = getAlarmMode();
	alarm = getAlarm(am);

	matrixSetX(0);

	switch (am) {
	case A_HOUR:
		matrixLoadString(mkNumberString(alarm, 2, 0, ' '));
		matrixSetX(13);
		matrixLoadStringEeprom(txtLabels[LABEL_HOUR]);
		break;
	case A_MIN:
		matrixLoadString(mkNumberString(alarm, 2, 0, ' '));
		matrixSetX(13);
		matrixLoadStringEeprom(txtLabels[LABEL_MINUTE]);
		break;
	default:
		if (alarm)
			matrixLoadString(" \xA0 ");
		else {
			matrixLoadString("   ");
		}
		matrixSetX(13);
		matrixLoadStringEeprom(txtLabels[LABEL_MO + am - A_MONDAY]);

		break;
	}

	if (alarmOld / 10 != alarm / 10)
		mask  |= MASK_HOUR_TENS;
	if (alarmOld % 10 != alarm % 10)
		mask  |= MASK_ALARM;

	if (amOld != am)
		mask |= MASK_ALL;

	if (ch_dir == PARAM_UP)
		matrixSwitchBuf(mask, MATRIX_EFFECT_SCROLL_DOWN);
	else
		matrixSwitchBuf(mask, MATRIX_EFFECT_SCROLL_UP);

	alarmOld = alarm;
	amOld = am;

	return;
}

void changeBrightness(int8_t diff)
{
	brMax += diff;

	if (brMax > MATRIX_MAX_BRIGHTNESS) {
		brMax = MATRIX_MIN_BRIGHTNESS;
	} else if (brMax < MATRIX_MIN_BRIGHTNESS) {
		brMax = MATRIX_MAX_BRIGHTNESS;
	}

	return;
}

void showBrightness(int8_t ch_dir, uint32_t mask)
{
	static uint8_t oldBrMax;

	matrixSetX(0);
	matrixLoadString(mkNumberString(brMax, 2, 0, ' '));
	matrixSetX(12);
	matrixLoadString("\xA4\xA4\xA4");

	if (oldBrMax / 10 != brMax / 10)
		mask  |= MASK_BR_TENS;
	if (oldBrMax % 10 != brMax % 10)
		mask  |= MASK_BR_UNITS;

	if (ch_dir == PARAM_UP)
		matrixSwitchBuf(mask, MATRIX_EFFECT_SCROLL_DOWN);
	else
		matrixSwitchBuf(mask, MATRIX_EFFECT_SCROLL_UP);

	oldBrMax = brMax;

	matrixSetBrightness(brMax);

	return;
}

void saveMaxBrightness(void)
{
	eeprom_update_byte(EEPROM_BR_MAX, brMax);

	return;
}

void checkAlarmAndBrightness(void)
{
	readTime();
	alarm = readAlarm();

	/* Check alarm */
	if (rtc.hour == alarm[A_HOUR] && rtc.min == alarm[A_MIN]) {
		if (getRawAlarmWeekday() & (1 << ((rtc.wday + 5) % 7)))
			startAlarm(60000);
	} else {
		/* Check new hour */
		if (rtc.hour > alarm[A_HOUR] && rtc.min == 0)
			startAlarm(160);
		else
			alarmFlag = 1;
	}

	/* Check brightness */
	matrixSetBrightness(calcBrightness());

	return;
}
