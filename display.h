#ifndef DISPLAY_H
#define DISPLAY_H

#include <inttypes.h>
#include "matrix.h"

#define PARAM_UP                1
#define PARAM_DOWN              -1

// Display modes
enum {
    MODE_MAIN,
    MODE_EDIT_TIME,
    MODE_EDIT_ALARM,
    MODE_BRIGHTNESS,
    MODE_CORRECTION,
    MODE_TEST,

    MODE_END
};

#define MASK_HOUR_TENS          0x0000001FUL
#define MASK_HOUR_UNITS         0x000003E0UL
#define MASK_MIN_TENS           0x0003E000UL
#define MASK_MIN_UNITS          0x007C0000UL

#define MASK_BIGHOUR_TENS       0x0000001FUL
#define MASK_BIGHOUR_UNITS      0x000007C0UL
#define MASK_BIGMIN_TENS        0x0003E000UL
#define MASK_BIGMIN_UNITS       0x00F80000UL

#define MASK_EXTRAHOUR_TENS     0x0000003FUL
#define MASK_EXTRAHOUR_UNITS    0x00001F80UL
#define MASK_EXTRAMIN_TENS      0x01F80000UL
#define MASK_EXTRAMIN_UNITS     0xFC000000UL

#if MATRIX_CNT == 4
#define MASK_SEC_TENS           0x03C00000UL
#define MASK_SEC_UNITS          0x3C000000UL
#define SECONDS_POS             23
#define MASK_SEC_TENS_BIG       0x0F000000UL
#define MASK_SEC_UNITS_BIG      0xF0000000UL
#define SECONDS_POS_BIG         25
#define WEEKDAY_POS             31
#else
#define WEEKDAY_POS             23
#endif

#define MASK_NONE               0x00000000UL
#define MASK_ALL                0xFFFFFFFFUL
#define MASK_ICON               0xF8000000UL

#define ICON_ALARM              '\xA0'
#define ICON_TIME               '\xAD'
#define ICON_BRIGHTNESS         '\xA4'
#define ICON_LESS               '<'
#define ICON_EQUAL              '='
#define ICON_MORE               '>'


#define SENS_MASK_BMP_TEMP      0x10
#define SENS_MASK_DHT_TEMP      0x20
#define SENS_MASK_BMP_PRES      0x40
#define SENS_MASK_DHT_HUMI      0x80

enum {
    SCROLL_DATE = 0,
    SCROLL_TEMP,

    SCROLL_END,
};

void displayInit(void);
void displaySetDirection(int8_t dir);
void displayChangeTime();
void displayChangeAlarm();

void startScroll(uint8_t type);

void showTime(uint32_t mask);
void showTimeMasked(void);
void showMainScreen(void);
void showTimeEdit();
void showAlarmEdit();
void showTest(void);

void showBrightness(uint8_t masked);
void showCorrection(uint8_t masked);
void checkAlarm(void);
void calcBrightness(void);

#endif // DISPLAY_H
