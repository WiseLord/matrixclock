#include "display.h"

#include "rtc.h"
#include "matrix.h"
#include "ds18x20.h"
#include "mtimer.h"
#include "alarm.h"
#include "eeprom.h"
#include "bmp180.h"
#include "dht22.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

uint8_t *txtLabels[LABEL_END];				/* Array with text label pointers */

static int8_t brMax;
static uint8_t sensMask;

static uint8_t bigNum = 0;
static uint8_t hourSignal = 1;
static uint8_t hourZero = 0;

static uint8_t etmOld = RTC_NOEDIT;
static uint8_t eamOld = ALARM_NOEDIT;

static uint8_t alarmFlag = 1;

static uint8_t firstSensor;

static uint8_t scrollType;

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
	strbuf[pos--] = '\0';

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
	matrixScrollAddStringEeprom(txtLabels[(LABEL_SATURDAY + rtc.wday) % 7]);
	matrixScrollAddString(", ");
	matrixScrollAddString(mkNumberString(rtc.date, 3, 0, 0x7F));
	matrixScrollAddString(" ");
	matrixScrollAddStringEeprom(txtLabels[LABEL_DECEMBER + rtc.month % 12]);
	matrixScrollAddString(mkNumberString(2000 + rtc.year, 5, 0, ' '));
	matrixScrollAddString(" ");
	matrixScrollAddStringEeprom(txtLabels[LABEL_Y]);

	return;
}

static void showCommaIfNeeded()
{
	if (firstSensor) {
		firstSensor = 0;
		matrixScrollAddStringEeprom(txtLabels[LABEL_TEMPERATURE]);
	} else {
		matrixScrollAddString(",");
	}

	return;
}

static void loadSensorString(int16_t value, uint8_t label)
{
	matrixScrollAddString(mkNumberString(value, 5, 1, ' '));
	matrixScrollAddStringEeprom(txtLabels[label]);

	return;
}

static void loadPlaceString(uint8_t label)
{
	matrixScrollAddString(" ");
	matrixScrollAddStringEeprom(txtLabels[label]);
}

static void loadTempString(void)
{
	uint8_t i;
	uint8_t sm = sensMask;

	firstSensor = 1;

	for (i = 0; i < ds18x20GetDevCount(); i++) {
		showCommaIfNeeded();
		loadSensorString(ds18x20GetTemp(i), LABEL_DEGREE);
		loadPlaceString(LABEL_TEMP1 + i);
	}

	if (bmp180HaveSensor() && (sm & SENS_MASK_BMP_TEMP)) {
		showCommaIfNeeded();
		loadSensorString(bmp180GetTemp(), LABEL_DEGREE);
		loadPlaceString(LABEL_TEMP3);
	}

	if (dht22HaveSensor() && (sm & SENS_MASK_DHT_TEMP)) {
		showCommaIfNeeded();
		loadSensorString(dht22GetTemp(), LABEL_DEGREE);
		loadPlaceString(LABEL_TEMP4);
	}

	if (bmp180HaveSensor() && (sm & SENS_MASK_BMP_PRES)) {
		showCommaIfNeeded();
		loadPlaceString(LABEL_PRESSURE);
		loadSensorString(bmp180GetPressure(), LABEL_MMHG);
	}

	if (dht22HaveSensor() && (sm & SENS_MASK_DHT_HUMI)) {
		showCommaIfNeeded();
		loadPlaceString(LABEL_HUMIDITY);
		loadSensorString(dht22GetHumidity(), LABEL_PERCENT);
	}

	return;
}

static uint32_t updateMask(uint32_t mask, uint8_t numSize, uint8_t hour, uint8_t min)
{
	static uint8_t oldHour, oldMin;

	if ((oldHour ^ hour) & 0xF0) {
		if (numSize == NUM_EXTRA)
			mask |= MASK_EXTRAHOUR_TENS;
		else if (numSize == NUM_BIG)
			mask |= MASK_BIGHOUR_TENS;
		else
			mask |= MASK_HOUR_TENS;
	}
	if ((oldHour ^ hour) & 0x0F) {
		if (numSize == NUM_EXTRA)
			mask |= MASK_EXTRAHOUR_UNITS;
		else if (numSize == NUM_BIG)
			mask |= MASK_BIGHOUR_UNITS;
		else
			mask |= MASK_HOUR_UNITS;
	}
	oldHour = hour;

	if ((oldMin ^ min) & 0xF0) {
		if (numSize == NUM_EXTRA)
			mask |= MASK_EXTRAMIN_TENS;
		else if (numSize == NUM_BIG)
			mask |= MASK_BIGMIN_TENS;
		else
			mask |= MASK_MIN_TENS;
	}
	if ((oldMin ^ min) & 0x0F) {
		if (numSize == NUM_EXTRA )
			mask |= MASK_EXTRAMIN_UNITS;
		else if (numSize == NUM_BIG)
			mask |= MASK_BIGMIN_UNITS;
		else
			mask |= MASK_MIN_UNITS;
	}
	oldMin = min;

	return mask;
}

static void updateColon(void)
{
	const static uint8_t colonCode[] PROGMEM = {
		0x32, 0x26, 0x46, 0x62
	};

	uint8_t colon = 0;
	uint8_t digit = rtc.sec & 0x01;

	if (bigNum == NUM_EXTRA) {
		colon = pgm_read_byte(&colonCode[digit + 2]);
		matrixPlace(15, colon);
		matrixPlace(16, colon);
	} else if (bigNum == NUM_NORMAL) {
		colon = pgm_read_byte(&colonCode[digit]);
		matrixPlace(10, colon);
		matrixPlace(11, colon);
		matrixPlace(23, alarmRawWeekday() | (hourSignal ? 0x80 : 0x00));
	} else {
		matrixPlace(11, (!digit) << 7);
		matrixPlace(12, digit << 7);
	}

	return;
}

static uint8_t calcBrightness(void)
{
	int8_t br = 0;

	static uint8_t adcOld;
	uint8_t adc = ADCH;

	/* Use ADC if we have photoresistor */
	if (adc) {
		adc >>= 3;
		if (adcOld < adc) {
			adcOld++;
		} else if (adcOld > adc) {
			adcOld--;
		}
		br = adcOld >> 1;
	} else {
		/* Calculate br(hour) instead */
		if (rtc.hour <= 12)
			br = (rtc.hour * 2) - 25 + brMax;
		else
			br = 31 - (rtc.hour * 2) + brMax;
	}

	if (br > brMax)
		br = brMax;
	if (br < 0)
		br = 0;

	return br;
}

void displayInit(void)
{
	uint8_t i;
	uint8_t *addr;

	matrixInit();

	/* Read text labels saved in EEPROM */
	addr = (uint8_t*)EEPROM_LABELS;
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

	bigNum = eeprom_read_byte((uint8_t*)EEPROM_BIGNUM);
	hourZero = eeprom_read_byte((uint8_t*)EEPROM_HOURZERO);
	brMax = eeprom_read_byte((uint8_t*)EEPROM_BR_MAX);
	sensMask = eeprom_read_byte((uint8_t*)EEPROM_SENS_MASK);
	hourSignal = eeprom_read_byte((uint8_t*)EEPROM_HOURSIGNAL);

	return;
}

void displaySwitchBigNum(void)
{
#if defined(MAX7219_X3) || defined(MAX7219MOD_X3) || defined(MAX7219MOD2_X3)
	if (++bigNum >= NUM_EXTRA)
#else
	if (++bigNum >= NUM_END)
#endif
		bigNum = NUM_NORMAL;

	eeprom_update_byte((uint8_t*)EEPROM_BIGNUM, bigNum);

	return;
}

void displaySwitchHourSignal(void) {
	hourSignal = !hourSignal;

	eeprom_update_byte((uint8_t*)EEPROM_HOURSIGNAL, hourSignal);
	return;
}

void displaySwitchHourZero(void) {
	hourZero = !hourZero;

	eeprom_update_byte((uint8_t*)EEPROM_HOURZERO, hourZero);
	return;
}

void startScroll(uint8_t type)
{
	matrixSwitchBuf(MASK_ALL, MATRIX_EFFECT_SCROLL_BOTH);
	if (type == SCROLL_DATE)
		loadDateString();
	else
		loadTempString();
	scrollType = type;
	matrixHwScroll(MATRIX_SCROLL_START);

	return;
}

void showTime(uint32_t mask)
{
	static uint8_t oldSec;
	uint8_t digit;

	etmOld = RTC_NOEDIT;
	eamOld = ALARM_NOEDIT;

	matrixSetX(0);
	matrixFbNewAddString(mkNumberString(rtc.hour, 2, 0, hourZero ? '0' : ' '), bigNum);

	if (bigNum == NUM_EXTRA)
		matrixSetX(19);
	else
		matrixSetX(13);
	matrixFbNewAddString(mkNumberString(rtc.min, 2, 0, '0'), bigNum);

	if (bigNum != NUM_EXTRA) {
		matrixSetX(25);
		matrixFbNewAddString(mkNumberString(rtc.sec, 2, 0, '0'), NUM_SMALL);
	}

	mask = updateMask(mask, bigNum, rtcDecToBinDec(rtc.hour), rtcDecToBinDec(rtc.min));

	if (bigNum != NUM_EXTRA) {
		digit = rtcDecToBinDec(rtc.sec);
		if ((oldSec ^ digit) & 0xF0)
			mask |= MASK_SEC_TENS;
		if ((oldSec ^ digit) & 0x0F)
			mask |= MASK_SEC_UNITS;
		oldSec = digit;
	}

	matrixSwitchBuf(mask, MATRIX_EFFECT_SCROLL_DOWN);
	updateColon();

	return;
}

void showTimeMasked()
{
	showTime(MASK_ALL);
}

void showMainScreen(void)
{
	uint8_t mode = matrixGetScrollMode();
	static uint8_t modeOld;

	if (mode == MATRIX_SCROLL_OFF) {
		if (modeOld == MATRIX_SCROLL_ON)
			showTimeMasked();
		else
			showTime(MASK_NONE);
		if (getScrollTimer () == 0) {
			if (++scrollType >= SCROLL_END)
				scrollType = SCROLL_DATE;
			startScroll (scrollType);
		}
	}

	modeOld = mode;

	return;
}

static void showParamEdit(int8_t value, char *aFlag, uint8_t label, char *icon)
{
	matrixSetX(0);
	if (aFlag) {
		matrixFbNewAddString(aFlag, NUM_NORMAL);
	} else  {
		matrixFbNewAddString(mkNumberString(value, 2, 0, ' '), NUM_NORMAL);
	}
	matrixSetX(13);
	matrixFbNewAddStringEeprom(txtLabels[label]);
	matrixSetX(27);
	matrixFbNewAddString(icon, NUM_NORMAL);

	return;
}

void showTimeEdit(int8_t ch_dir)
{
	uint32_t mask = MASK_NONE;

	int8_t time = *((int8_t*)&rtc + rtc.etm);

	showParamEdit(time, 0, LABEL_SECOND + rtc.etm, "\xAD");

	mask = updateMask(mask, NUM_NORMAL, rtcDecToBinDec(time), 0);

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

	uint8_t label;
	char *aFlag;

	char *aIcon = "\xA0";
	char *aTrue = " \xA0";
	char *aFalse = "  ";

	if (alarm.eam > ALARM_MIN) {
		label = LABEL_MO + alarm.eam - ALARM_MON;
		aFlag = alrm ? aTrue : aFalse;
	} else {
		label = LABEL_HOUR - alarm.eam;
		aFlag = 0;
	}

	showParamEdit(alrm, aFlag, label, aIcon);

	mask = updateMask(mask, NUM_NORMAL, rtcDecToBinDec(alrm), 0);

	if (eamOld != alarm.eam)
		mask |= MASK_ALL;

	matrixSwitchBuf(mask, ch_dir);

	eamOld = alarm.eam;

	return;
}

void changeBrightness(int8_t diff)
{
	brMax += diff;
	brMax &= 0x0F;

	return;
}

void showBrightness(int8_t ch_dir, uint32_t mask)
{
	static uint8_t oldBrMax;

	showParamEdit(brMax, 0, LABEL_BRIGHTNESS, "\xA4");

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
	eeprom_update_byte((uint8_t*)EEPROM_BR_MAX, brMax);

	return;
}

void checkAlarmAndBrightness(void)
{
	rtcReadTime();

	/* Check alarm */
	if (rtc.hour == alarm.hour && rtc.min == alarm.min) {
		if (*((int8_t*)&alarm.mon + ((rtc.wday + 5) % 7)))
			startAlarm(BEEP_ALARM);
	} else {
		/* Check new hour */
		if (rtc.hour > alarm.hour && rtc.min == 0 && hourSignal)
			startAlarm(BEEP_LONG);
		else
			alarmFlag = 1;
	}

	/* Check brightness */
	matrixSetBrightness(calcBrightness());

	return;
}
