#include "display.h"

#include "ds1307.h"
#include "matrix.h"
#include "ds18x20.h"
#include "mtimer.h"
#include "alarm.h"
#include "eeprom.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

int8_t *dateTime;
int8_t *alarm;

char strbuf[8];
static uint32_t timeMask = MASK_ALL;

uint8_t *txtLabels[LABEL_END];				/* Array with text label pointers */

static int8_t brHour;
static int8_t brightness;

static uint8_t bigNum = 0;

static uint8_t etmOld = NOEDIT;
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
	matrixSetX(0);
	matrixLoadStringEeprom(txtLabels[dateTime[DS1307_WDAY] % 7]);
	matrixLoadString(mkNumberString(dateTime[DS1307_DATE], 2, 0, ' '));
	matrixLoadStringEeprom(txtLabels[LABEL_DECEMBER + dateTime[DS1307_MONTH] % 12]);
	matrixLoadString(mkNumberString(2000 + dateTime[DS1307_YEAR], 4, 0, '0'));
	matrixLoadStringEeprom(txtLabels[LABEL_Y]);

	return;
}

static void loadTempString(void)
{
	matrixSetX(0);
	matrixLoadString(mkNumberString(ds18x20GetTemp(0), 4, 1, ' '));
	matrixLoadStringEeprom(txtLabels[LABEL_DEGREE]);
	matrixLoadStringEeprom(txtLabels[LABEL_TEMP1]);

	if (getDevCount() > 1) {
		matrixLoadString(", ");
		matrixLoadString(mkNumberString(ds18x20GetTemp(1), 4, 1, ' '));
		matrixLoadStringEeprom(txtLabels[LABEL_DEGREE]);
		matrixLoadStringEeprom(txtLabels[LABEL_TEMP2]);
	}

	return;
}

static uint8_t checkIfAlarmToday(void)
{
	return getRawAlarmWeekday() & (1 << (dateTime[DS1307_WDAY] - 1));
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

	dateTime = readTime();
	bigNum = eeprom_read_byte(EEPROM_BIGNUM);

	return;
}

void displaySwitchBigNum(void)
{
	bigNum = !bigNum;
	eeprom_update_byte(EEPROM_BIGNUM, bigNum);

	return;
}


static void showHMColon(uint8_t step)
{
	switch (step) {
	case 2:
		matrixPosData(10, 0x36);
		matrixPosData(11, 0x36);
		break;
	case 1:
		matrixPosData(10, 0x26);
		matrixPosData(11, 0x26);
		break;
	default:
		matrixPosData(10, 0x32);
		matrixPosData(11, 0x32);
		break;
	}

	return;
}

void showTime(uint32_t mask)
{
	static uint8_t oldHourTens, oldHourUnits, oldMinTens, oldMinUnits, oldSecTens, oldSecUnits;
	uint8_t digit;

	uint8_t hour = dateTime[DS1307_HOUR];
	uint8_t min = dateTime[DS1307_MIN];
	uint8_t sec = dateTime[DS1307_SEC];

	etmOld = NOEDIT;

	matrixSetX(0);
	mkNumberString(hour, 2, 0, ' ');
	if (bigNum)
		matrixBigNumString(strbuf);
	else
		matrixLoadString(strbuf);
	matrixSetX(13);
	mkNumberString(min, 2, 0, '0');
	if (bigNum)
		matrixBigNumString(strbuf);
	else
		matrixLoadString(strbuf);
	matrixSetX(25);
	matrixSmallNumString(mkNumberString(sec, 2, 0, '0'));

	digit = hour / 10;
	if (oldHourTens != digit) {
		if (bigNum)
			mask |= MASK_BIGHOUR_TENS;
		else
			mask |= MASK_HOUR_TENS;
	}
	oldHourTens = digit;

	digit = hour % 10;
	if (oldHourUnits != digit) {
		if (bigNum)
			mask |= MASK_BIGHOUR_UNITS;
		else
			mask |= MASK_HOUR_UNITS;
	}
	oldHourUnits = digit;

	digit = min / 10;
	if (oldMinTens != digit) {
		if (bigNum)
			mask |= MASK_BIGMIN_TENS;
		else
			mask |= MASK_MIN_TENS;
	}
	oldMinTens = digit;

	digit = min % 10;
	if (oldMinUnits != digit) {
		if (bigNum)
			mask |= MASK_BIGMIN_UNITS;
		else
			mask |= MASK_MIN_UNITS;
	}
	oldMinUnits = digit;

	digit = sec / 10;
	if (oldSecTens != digit)
		mask |= MASK_SEC_TENS;
	oldSecTens = digit;

	digit = sec % 10;
	if (oldSecUnits != digit)
		mask |= MASK_SEC_UNITS;
	oldSecUnits = digit;

	digit = sec & 0x01;
	if (bigNum) {
		if (digit) {
			matrixPosData(11, 0x00);
			matrixPosData(12, 0x80);
		} else {
			matrixPosData(11, 0x80);
			matrixPosData(12, 0x00);
		}
	} else {
		showHMColon(digit);
		matrixPosData(23, getRawAlarmWeekday());
	}

	brightness = eeprom_read_byte(EEPROM_BR_ADDR + hour);

	matrixSwitchBuf(mask, MATRIX_EFFECT_SCROLL_DOWN);

	return;
}

void scrollDate(void)
{
	loadDateString();
	matrixHwScroll(MATRIX_SCROLL_START);
	timeMask = MASK_ALL;

	return;
}

void scrollTemp(void)
{
	if (getDevCount() > 0) {
		loadTempString();
		matrixHwScroll(MATRIX_SCROLL_START);
		timeMask = MASK_ALL;
	}
	return;
}

void setTimeMask(uint32_t tmsk)
{
	timeMask = tmsk;
}

void showMainScreen(void)
{
	if (matrixGetScrollMode() == 0) {
		showTime(timeMask);
		if (dateTime[DS1307_SEC] == 10) {
			scrollDate();
		} else if (dateTime[DS1307_SEC] == 40) {
			scrollTemp();
		} else {
			timeMask = MASK_NONE;
		}
	}

	return;
}

void showTimeEdit(int8_t ch_dir)
{
	uint32_t mask = MASK_NONE;
	uint8_t etm;

	static int8_t timeOld = 0;

	etm = getEtm();

	matrixSetX(0);
	matrixLoadString(mkNumberString(dateTime[etm], 2, 0, ' '));
	matrixSetX(13);
	matrixLoadStringEeprom(txtLabels[LABEL_SECOND + etm]);

	if (timeOld / 10 != dateTime[etm] / 10)
		mask  |= MASK_HOUR_TENS;
	if (timeOld % 10 != dateTime[etm] % 10)
		mask  |= MASK_HOUR_UNITS;

	if (etmOld != etm)
		mask |= MASK_ALL;

	if (ch_dir == PARAM_UP)
		matrixSwitchBuf(mask, MATRIX_EFFECT_SCROLL_DOWN);
	else
		matrixSwitchBuf(mask, MATRIX_EFFECT_SCROLL_UP);

	timeOld = dateTime[etm];
	etmOld = etm;

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

	showHMColon(2);
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

void setBrightnessHour(uint8_t mode)
{
	eeprom_update_byte(EEPROM_BR_ADDR + brHour, brightness);

	if (mode == HOUR_CURRENT) {
		brHour = dateTime[DS1307_HOUR];
	} else if (mode == HOUR_NEXT) {
		if (++brHour >= 24)
			brHour = 0;
	}

	brightness = eeprom_read_byte(EEPROM_BR_ADDR + brHour);

	return;
}

void changeBrightness(int8_t diff)
{
	brightness += diff;

	if (brightness > MATRIX_MAX_BRIGHTNESS) {
		brightness = MATRIX_MIN_BRIGHTNESS;
	} else if (brightness < MATRIX_MIN_BRIGHTNESS) {
		brightness = MATRIX_MAX_BRIGHTNESS;
	}

	return;
}

void showBrightness(int8_t ch_dir, uint32_t mask)
{
	static int8_t oldHour;
	static uint8_t oldBrightness;

	matrixSetX(0);
	matrixLoadString(mkNumberString(brHour, 2, 0, ' '));
	matrixLoadString("\xBB");
	matrixSetX(15);
	matrixLoadString(mkNumberString(brightness, 2, 0, ' '));
	matrixLoadString("\xA4");

	if (oldHour / 10 != brHour / 10)
		mask  |= MASK_HOUR_TENS;
	if (oldHour % 10 != brHour % 10)
		mask  |= MASK_HOUR_UNITS;
	if (oldBrightness / 10 != brightness / 10)
		mask  |= MASK_BR_TENS;
	if (oldBrightness % 10 != brightness % 10)
		mask  |= MASK_BR_UNITS;

	if (ch_dir == PARAM_UP)
		matrixSwitchBuf(mask, MATRIX_EFFECT_SCROLL_DOWN);
	else
		matrixSwitchBuf(mask, MATRIX_EFFECT_SCROLL_UP);

	oldHour = brHour;
	oldBrightness = brightness;

	return;
}

void checkAlarmAndBrightness(void)
{
	dateTime = readTime();
	alarm = readAlarm();

	/* Check alarm */
	if (dateTime[DS1307_HOUR] == alarm[A_HOUR] && dateTime[DS1307_MIN] == alarm[A_MIN]) {
		if (checkIfAlarmToday())
			startAlarm(60000);
	} else {
		/* Check new hour */
		if (dateTime[DS1307_HOUR] > alarm[A_HOUR] && dateTime[DS1307_MIN] == 0)
			startAlarm(160);
		else
			alarmFlag = 1;
	}

	/* Check brightness */
	matrixSetBrightness(brightness);

	return;
}
