#include "display.h"

#include "rtc.h"
#include "ds18x20.h"
#include "mtimer.h"
#include "alarm.h"
#include "eeprom.h"
#include "bmp180.h"
#include "dht22.h"

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

uint8_t *txtLabels[LABEL_END];              // Array with text label pointers

#define PARAM_FF    0xFF

static uint8_t paramOld = PARAM_FF;
static uint8_t firstSensor;
static uint8_t scrollType;
static int8_t direction = PARAM_UP;

static char *mkNumberString(int16_t value, uint8_t width, uint8_t lead)
{
    static char strbuf[8];

    uint8_t sign = lead;
    int8_t pos;

    if (value < 0) {
        sign = '-';
        value = -value;
    }

    // Clear buffer and go to it's tail
    for (pos = 0; pos < width; pos++)
        strbuf[pos] = lead;
    strbuf[pos--] = '\0';

    // Fill buffer from right to left
    while (value > 0 || pos > width - 2) {
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
    matrixScrollAddString(mkNumberString(rtc.date, 3, 0x7F));
    matrixScrollAddString(" ");
    matrixScrollAddStringEeprom(txtLabels[LABEL_DECEMBER + (rtc.month >= 12 ? 0 : rtc.month)]);
    matrixScrollAddString(mkNumberString(2000 + rtc.year, 5, ' '));
    matrixScrollAddString(" ");
    matrixScrollAddStringEeprom(txtLabels[LABEL_Y]);
}

static void showCommaIfNeeded()
{
    if (firstSensor) {
        firstSensor = 0;
        matrixScrollAddStringEeprom(txtLabels[LABEL_TEMPERATURE]);
    } else {
        matrixScrollAddString(",");
    }
}

static void loadSensorString(int16_t value, uint8_t label)
{
    matrixScrollAddString(mkNumberString(value / 10, 4, ' '));
    matrixScrollAddString(".");
    matrixScrollAddString(mkNumberString(value % 10, 1, '0'));
    matrixScrollAddStringEeprom(txtLabels[label]);
}

static void loadPlaceString(uint8_t label)
{
    matrixScrollAddString(" ");
    matrixScrollAddStringEeprom(txtLabels[label]);
}

static void loadTempString(void)
{
    uint8_t i;
    uint8_t sm = eeParamGet()->sensMask;

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
}

static uint32_t updateMask(uint32_t mask, uint8_t numSize, uint8_t hour, uint8_t min)
{
    static uint8_t oldHour, oldMin;

    if ((oldHour ^ hour) & 0xF0) {
        if (numSize == NUM_BIG)
            mask |= MASK_BIGHOUR_TENS;
#if MATRIX_CNT == 4
        else if (numSize == NUM_EXTRA)
            mask |= MASK_EXTRAHOUR_TENS;
#endif
        else
            mask |= MASK_HOUR_TENS;
    }
    if ((oldHour ^ hour) & 0x0F) {
        if (numSize == NUM_BIG)
            mask |= MASK_BIGHOUR_UNITS;
#if MATRIX_CNT == 4
        else if (numSize == NUM_EXTRA)
            mask |= MASK_EXTRAHOUR_UNITS;
#endif
        else
            mask |= MASK_HOUR_UNITS;
    }
    oldHour = hour;

    if ((oldMin ^ min) & 0xF0) {
        if (numSize == NUM_BIG)
            mask |= MASK_BIGMIN_TENS;
#if MATRIX_CNT == 4
        else if (numSize == NUM_EXTRA)
            mask |= MASK_EXTRAMIN_TENS;
#endif
        else
            mask |= MASK_MIN_TENS;
    }
    if ((oldMin ^ min) & 0x0F) {
        if (numSize == NUM_BIG)
            mask |= MASK_BIGMIN_UNITS;
#if MATRIX_CNT == 4
        else if (numSize == NUM_EXTRA)
            mask |= MASK_EXTRAMIN_UNITS;
#endif
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

    EE_Param *eep = eeParamGet();

    uint8_t colon = 0;
    uint8_t digit = rtc.sec & 0x01;
    uint8_t bigNum = eep->bigNum;

    if (bigNum == NUM_BIG) {
        fb[11] = (!digit) << 7;
        fb[12] = digit << 7;
#if MATRIX_CNT == 4
    } else if (bigNum == NUM_EXTRA) {
        colon = pgm_read_byte(&colonCode[digit + 2]);
        fb[15] = colon;
        fb[16] = colon;
#endif
    } else {
        colon = pgm_read_byte(&colonCode[digit]);
        fb[10] = colon;
        fb[11] = colon;
        fb[WEEKDAY_POS] = alarmRawWeekday() | (eep->hourSignal ? 0x80 : 0x00);
    }
}

void displayInit(void)
{
    uint8_t i;
    uint8_t *addr;

    eeprom_read_block(eeParamGet(), (void *)EEPROM_HOURSIGNAL, sizeof(EE_Param));

    matrixInit();

    // Read text labels saved in EEPROM
    addr = (uint8_t *)EEPROM_LABELS;
    i = 0;
    while (i < LABEL_END && addr < (uint8_t *)EEPROM_SIZE) {
        if (eeprom_read_byte(addr) != '\0') {
            txtLabels[i] = addr;
            addr++;
            i++;
            while (eeprom_read_byte(addr) != '\0' &&
                    addr < (uint8_t *)EEPROM_SIZE) {
                addr++;
            }
        } else {
            addr++;
        }
    }
}


void displaySetDirection(int8_t dir)
{
    direction = dir;
}

void displayChangeTime()
{
    rtcChangeTime(direction);
}
void displayChangeAlarm()
{
    alarmChange(direction);
}

void startScroll(uint8_t type)
{
    matrixHwScroll(MATRIX_SCROLL_STOP);
    matrixSwitchBuf(MASK_ALL, MATRIX_EFFECT_SCROLL_BOTH);
    if (type == SCROLL_DATE)
        loadDateString();
    else
        loadTempString();
    scrollType = type;
    matrixHwScroll(MATRIX_SCROLL_START);
}

void showTime(uint32_t mask)
{
#if MATRIX_CNT == 4
    static uint8_t oldSec;
    uint8_t digit;
#endif

    paramOld = PARAM_FF;

    EE_Param *eep = eeParamGet();

    uint8_t bigNum = eep->bigNum;

    matrixSetX(0);
    matrixFbNewAddString(mkNumberString(rtc.hour, 2, eep->hourZero ? '0' : ' '), bigNum);

    if (bigNum == NUM_BIG)
        matrixSetX(13);
#if MATRIX_CNT == 4
    else if (bigNum == NUM_EXTRA)
        matrixSetX(19);
#endif
    else
        matrixSetX(13);
    matrixFbNewAddString(mkNumberString(rtc.min, 2, '0'), bigNum);

#if MATRIX_CNT == 4
    if (bigNum != NUM_EXTRA) {
        if (bigNum == NUM_NORMAL)
            matrixSetX(SECONDS_POS);
        else
            matrixSetX(SECONDS_POS_BIG);
        matrixFbNewAddString(mkNumberString(rtc.sec, 2, '0'), NUM_SMALL);
    }
#endif

    mask = updateMask(mask, bigNum, rtcDecToBinDec((uint8_t)rtc.hour), rtcDecToBinDec((uint8_t)rtc.min));

#if MATRIX_CNT == 4
    if (bigNum != NUM_EXTRA) {
        digit = rtcDecToBinDec(rtc.sec);
        if ((oldSec ^ digit) & 0xF0) {
            if (bigNum == NUM_NORMAL)
                mask |= MASK_SEC_TENS;
            else
                mask |= MASK_SEC_TENS_BIG;
        }
        if ((oldSec ^ digit) & 0x0F) {
            if (bigNum == NUM_NORMAL)
                mask |= MASK_SEC_UNITS;
            else
                mask |= MASK_SEC_UNITS_BIG;
        }
        oldSec = digit;
    }
#endif

    matrixSwitchBuf(mask, MATRIX_EFFECT_SCROLL_DOWN);
    updateColon();
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
        if (scrollTimer == 1) {
            if (++scrollType >= SCROLL_END)
                scrollType = SCROLL_DATE;
            startScroll (scrollType);
        }
    }

    modeOld = mode;
}

static void showParamEdit(int8_t value, uint8_t label, char icon)
{
    matrixSetX(0);
    char *num = mkNumberString(value, 2, ' ');
    if (icon == ICON_ALARM && alarm.eam >= ALARM_MON) {
        num[1] = value ? icon : ' ';
    }
    matrixFbNewAddString(num, NUM_NORMAL);

    matrixSetX(13);
    matrixFbNewAddStringEeprom(txtLabels[label]);

#if MATRIX_CNT == 4
    matrixSetX(27);
    char iconStr[] = " ";
    iconStr[0] = icon;
    matrixFbNewAddString(iconStr, NUM_NORMAL);
#endif
}

static void showParam(uint8_t masked, int8_t value, uint8_t label, char icon)
{
    static char oldIcon = '=';

    uint32_t mask = masked ? MASK_ALL : MASK_NONE;

    showParamEdit(value, label, icon);
    mask = updateMask(mask, NUM_NORMAL, rtcDecToBinDec(value), 0);
    if (oldIcon != icon) {
        mask |= MASK_ICON;
        oldIcon = icon;
    }
    matrixSwitchBuf(mask, direction);
    paramOld = value;
}

void showTimeEdit()
{
    int8_t time = *((int8_t *)&rtc + rtc.etm);

    uint8_t masked = (paramOld != rtc.etm);

    showParam(masked, time, LABEL_SECOND + rtc.etm, ICON_TIME);

    paramOld = rtc.etm;
}

void showAlarmEdit()
{
    int8_t alrm = *((int8_t *)&alarm + alarm.eam);

    uint8_t label;

    if (alarm.eam > ALARM_MIN) {
        label = LABEL_MO + alarm.eam - ALARM_MON;
    } else {
        label = LABEL_HOUR - alarm.eam;
    }

    uint8_t masked = (paramOld != alarm.eam);

    showParam(masked, alrm, label, ICON_ALARM);

    paramOld = alarm.eam;
}

void showTest(void)
{
    uint8_t i;

    for (i = 0; i < MATRIX_CNT; i++) {
        matrixSetX(i * 8 + 2);
        matrixFbNewAddString(mkNumberString(i + 1, 1, '0'), NUM_NORMAL);
    }
    matrixSwitchBuf(MASK_ALL, MATRIX_EFFECT_NONE);
}

void showBrightness(uint8_t masked)
{
    int8_t brMax = eeParamGet()->brMax;

    showParam(masked, brMax, LABEL_BRIGHTNESS, ICON_BRIGHTNESS);

    matrixSetBrightness(brMax);
}

void showCorrection(uint8_t masked)
{
    int8_t corr = eeParamGet()->corr;

    char sign = (corr < 0 ? ICON_LESS : (corr > 0 ? ICON_MORE : ICON_EQUAL));

    if (corr < 0)
        corr = -corr;

    showParam(masked, corr, LABEL_CORRECTION, sign);
}

void checkAlarm(void)
{
    static uint8_t firstCheck = 1;
    static uint8_t rtcCorrected = 0;

    rtcReadTime();
    EE_Param *eep = eeParamGet();

    int8_t corr = eep->corr;

    if (!rtcCorrected) {
        if (rtc.wday == 1 && rtc.hour == 3 && rtc.min == 0) {
            if ((rtc.sec == 0) && (corr > 0)) {
                rtc.sec += corr;
                rtcCorrSec();
                rtcCorrected = 1;
            } else if ((rtc.sec == 59) && (corr < 0)) {
                rtc.sec += corr;
                rtcCorrSec();
                rtcCorrected = 1;
            }
        }
    }

    // Once check if it's a new second
    if (rtc.sec == 0) {
        rtcCorrected = 0; // Clean correction flag
        if (firstCheck) {
            firstCheck = 0;
            // Check alarm
            if (rtc.hour == alarm.hour && rtc.min == alarm.min) {
                if (*((int8_t *)&alarm.mon + ((rtc.wday + 5) % 7)))
                    alarmTimer = 60 * (uint16_t)eep->alarmTimeout;
            } else {
                // Check new hour
                if (rtc.hour > alarm.hour && rtc.min == 0 && eep->hourSignal)
                    startBeeper(BEEP_LONG);
            }
        }
    } else {
        firstCheck = 1;
    }
}

void calcBrightness(void)
{
    int8_t br = 0;

    static uint8_t adcOld;
    uint8_t adc = ADCH;
    int8_t brMax = eeParamGet()->brMax;

    // Use ADC if we have photoresistor
    if (adc > 4) {
        adc >>= 3;
        if (adcOld < adc)
            adcOld++;
        else if (adcOld > adc)
            adcOld--;
        br = adcOld >> 1;
    } else {
        // Calculate br(hour) instead
        if (rtc.hour <= 12)
            br = (rtc.hour * 2) - 25 + brMax;
        else
            br = 31 - (rtc.hour * 2) + brMax;
    }

    if (br > brMax)
        br = brMax;
    if (br < 0)
        br = 0;

    matrixSetBrightness(br);
}
