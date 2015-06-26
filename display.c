#include "display.h"

#include "rtc.h"
#include "matrix.h"
#include "ds18x20.h"
#include "mtimer.h"
#include "alarm.h"
#include "eeprom.h"
#include "bmp180.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

uint8_t *txtLabels[LABEL_END];				/* Array with text label pointers */

static int8_t brMax;

static uint8_t bigNum = 0;
static uint8_t hourZero = '0';

static uint8_t etmOld = RTC_NOEDIT;
static uint8_t eamOld = ALARM_NOEDIT;

static uint8_t alarmFlag = 1;

static uint8_t oldHourTens, oldHourUnits, oldMinTens, oldMinUnits;

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
	static char strbuf[8];

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
	matrixLoadString(mkNumberString(2000 + rtc.year, 5, 0, ' '));
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
	if (bmp180HaveSensor()) {
		matrixLoadString(", ");
		matrixLoadString(mkNumberString(bmp180GetTemp(), 4, 1, ' '));
		matrixLoadStringEeprom(txtLabels[LABEL_DEGREE]);
		matrixLoadString(" ");
		matrixLoadStringEeprom(txtLabels[LABEL_TEMP3]);
		matrixLoadString(", ");
		matrixLoadStringEeprom(txtLabels[LABEL_PRESSURE]);
		matrixLoadString(" ");
		matrixLoadString(mkNumberString(bmp180GetPressure(), 4, 1, ' '));
		matrixLoadString(" ");
		matrixLoadStringEeprom(txtLabels[LABEL_MMHG]);
	}

	return;
}

static uint32_t updateMask(uint32_t mask, uint8_t numSize, uint8_t hour, uint8_t min)
{
	uint8_t bits = 0;
	uint8_t digit;

	digit = hour / 10;
	if (oldHourTens != digit)
		bits |= 0x80;
	oldHourTens = digit;

	digit = hour % 10;
	if (oldHourUnits != digit)
		bits |= 0x40;
	oldHourUnits = digit;

	digit = min / 10;
	if (oldMinTens != digit)
		bits |= 0x20;
	oldMinTens = digit;

	digit = min % 10;
	if (oldMinUnits != digit)
		bits |= 0x10;
	oldMinUnits = digit;

	if (bits & 0x80) {
		if (numSize == NUM_EXTRA)
			mask |= MASK_EXTRAHOUR_TENS;
		else if (numSize == NUM_BIG)
			mask |= MASK_BIGHOUR_TENS;
		else
			mask |= MASK_HOUR_TENS;
	}
	if (bits & 0x40) {
		if (numSize == NUM_EXTRA)
			mask |= MASK_EXTRAHOUR_UNITS;
		else if (numSize == NUM_BIG)
			mask |= MASK_BIGHOUR_UNITS;
		else
			mask |= MASK_HOUR_UNITS;
	}
	if (bits & 0x20) {
		if (numSize == NUM_EXTRA)
			mask |= MASK_EXTRAMIN_TENS;
		else if (numSize == NUM_BIG)
			mask |= MASK_BIGMIN_TENS;
		else
			mask |= MASK_MIN_TENS;
	}
	if (bits & 0x10) {
		if (numSize == NUM_EXTRA )
			mask |= MASK_EXTRAMIN_UNITS;
		else if (numSize == NUM_BIG)
			mask |= MASK_BIGMIN_UNITS;
		else
			mask |= MASK_MIN_UNITS;
	}

	return mask;
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
	static int8_t br = 0;

	uint8_t adch = ADCH;

	if (adch > 2) {							/* We have photoresistor */
		if (br > (adch >> 4))
			br--;
		if (br < (adch >> 4))
			br++;
	} else {								/* Calculate br(hour) */
		if (rtc.hour <= 12)
			br = (rtc.hour * 2) - 25 + brMax;
		else
			br = 31 - (rtc.hour * 2) + brMax;
	}

	if (br > brMax)
		br = brMax;
	if (br < MATRIX_MIN_BRIGHTNESS)
		br = MATRIX_MIN_BRIGHTNESS;

	return br;
}

void displayInit(void)
{
	uint8_t i;
	uint8_t *addr;

	matrixInit();

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
#if defined(MAX7219_X3) || defined(MAX7219MOD_X3)
	if (++bigNum >= NUM_EXTRA)
#else
	if (++bigNum >= NUM_END)
#endif
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

void startScroll(uint8_t type)
{
	matrixSetX(0);
	matrixClearBufTail();
	matrixSwitchBuf(MASK_ALL, MATRIX_EFFECT_SCROLL_BOTH);
	if (type == SCROLL_DATE)
		loadDateString();
	else
		loadTempString();
	matrixHwScroll(MATRIX_SCROLL_START);

	return;
}

void showTime(uint32_t mask)
{
	static uint8_t oldSecTens, oldSecUnits;
	uint8_t digit;

	etmOld = RTC_NOEDIT;
	eamOld = ALARM_NOEDIT;

	matrixSetX(0);
	matrixLoadNumString(mkNumberString(rtc.hour, 2, 0, hourZero), bigNum);

	if (bigNum == NUM_EXTRA)
		matrixSetX(19);
	else
		matrixSetX(13);
	matrixLoadNumString(mkNumberString(rtc.min, 2, 0, '0'), bigNum);

	if (bigNum != NUM_EXTRA) {
		matrixSetX(25);
		matrixLoadNumString(mkNumberString(rtc.sec, 2, 0, '0'), NUM_SMALL);
	}

	mask = updateMask(mask, bigNum, rtc.hour, rtc.min);

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
		showHMColon(digit + 3, 15);
	} else {
		showHMColon(digit, 10);
		matrixPosData(23, alarmRawWeekday());
	}

	matrixSwitchBuf(mask, MATRIX_EFFECT_SCROLL_DOWN);

	return;
}

void showMainScreen(void)
{
	uint8_t mode = matrixGetScrollMode();
	static uint8_t modeOld;

	if (mode == MATRIX_SCROLL_OFF) {
		if (modeOld == MATRIX_SCROLL_ON)
			showTime(MASK_ALL);
		else
			showTime(MASK_NONE);
		if (rtc.sec == 10)
			startScroll(SCROLL_DATE);
		else if (rtc.sec == 40)
			startScroll(SCROLL_TEMP);
	}

	modeOld = mode;

	return;
}

void showTimeEdit(int8_t ch_dir)
{
	uint32_t mask = MASK_NONE;

	int8_t time = *((int8_t*)&rtc + rtc.etm);

	matrixSetX(0);
	matrixLoadString(mkNumberString(time, 2, 0, ' '));
	matrixSetX(13);
	matrixLoadStringEeprom(txtLabels[LABEL_SECOND + rtc.etm]);
	matrixSetX(27);
	matrixLoadString("\xAD");

	mask = updateMask(mask, NUM_NORMAL, time, 0);

	if (etmOld != rtc.etm)
		mask |= MASK_ALL;

	matrixSwitchBuf(mask, ch_dir);

	etmOld = rtc.etm;

	return;
}

void showAlarmEdit(int8_t ch_dir)
{
	uint32_t mask = MASK_NONE;

	int8_t alrm = *((int8_t*)&alarm + alarm.eam);

	matrixSetX(0);

	switch (alarm.eam) {
	case ALARM_HOUR:
		matrixLoadString(mkNumberString(alrm, 2, 0, ' '));
		matrixSetX(13);
		matrixLoadStringEeprom(txtLabels[LABEL_HOUR]);
		break;
	case ALARM_MIN:
		matrixLoadString(mkNumberString(alrm, 2, 0, ' '));
		matrixSetX(13);
		matrixLoadStringEeprom(txtLabels[LABEL_MINUTE]);
		break;
	default:
		if (alrm)
			matrixLoadString(" \xA0");
		else
			matrixLoadString("  ");
		matrixSetX(13);
		matrixLoadStringEeprom(txtLabels[LABEL_MO + alarm.eam - ALARM_MON]);

		break;
	}
	matrixSetX(27);
	matrixLoadString("\xA0");

	mask = updateMask(mask, NUM_NORMAL, alrm, 0);

	if (eamOld != alarm.eam)
		mask |= MASK_ALL;

	matrixSwitchBuf(mask, ch_dir);

	eamOld = alarm.eam;

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

	matrixSwitchBuf(mask, ch_dir);

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
	rtcReadTime();

	/* Check alarm */
	if (rtc.hour == alarm.hour && rtc.min == alarm.min) {
		if (*((int8_t*)&alarm.mon + ((rtc.wday + 5) % 7)))
			startAlarm(60000);
	} else {
		/* Check new hour */
		if (rtc.hour > alarm.hour && rtc.min == 0)
			startAlarm(160);
		else
			alarmFlag = 1;
	}

	/* Check brightness */
	matrixSetBrightness(calcBrightness());

	return;
}
