#ifndef EEPROM_H
#define EEPROM_H

#include <inttypes.h>

#define EEPROM_ALARM_HOUR           0x00
#define EEPROM_ALARM_MIN            0x01
#define EEPROM_ALARM_MON            0x02

#define EEPROM_HOURSIGNAL           0x10
#define EEPROM_SCREEN_ROTATE        0x11
#define EEPROM_BIGNUM               0x12
#define EEPROM_HOURZERO             0x13
#define EEPROM_SCROLL_INTERVAL      0x14
#define EEPROM_BR_MAX               0x15
#define EEPROM_SENS_MASK            0x16
#define EEPROM_ALARM_TIMEOUT        0x17
#define EEPROM_CORRECTION           0x18

// Text labels (maximum 15 byte followed by \0)
#define EEPROM_LABELS               0x20

// Big numbers font (10 * 5 bytes)
#define EEPROM_BIG_NUM_FONT         0x1C0
// Extra numbers font (10 * 6 bytes)
#define EEPROM_EXTRA_NUM_FONT       0x180

#define EEPROM_SIZE                 0x200

typedef struct {
    uint8_t hourSignal;
    uint8_t rotate;
    uint8_t bigNum;
    uint8_t hourZero;
    uint8_t scrollInterval;
    int8_t brMax;
    uint8_t sensMask;
    uint8_t alarmTimeout;
    int8_t corr;
} EE_Param;

enum {
    LABEL_MONDAY,
    LABEL_TUESDAY,
    LABEL_WEDNESDAY,
    LABEL_THURSDAY,
    LABEL_FRIDAY,
    LABEL_SATURDAY,
    LABEL_SUNDAY,

    LABEL_MO,
    LABEL_TU,
    LABEL_WE,
    LABEL_TH,
    LABEL_FR,
    LABEL_SA,
    LABEL_SU,

    LABEL_DECEMBER,
    LABEL_JANUARY,
    LABEL_FEBRUARY,
    LABEL_MARCH,
    LABEL_APRIL,
    LABEL_MAY,
    LABEL_JUNE,
    LABEL_JULY,
    LABEL_AUGUST,
    LABEL_SEPTEMBER,
    LABEL_OCTOBER,
    LABEL_NOVEMBER,

    LABEL_SECOND,
    LABEL_MINUTE,
    LABEL_HOUR,
    LABEL_WEEK,
    LABEL_DATA,
    LABEL_MONTH,
    LABEL_YEAR,
    LABEL_Y,

    LABEL_TEMPERATURE,
    LABEL_DEGREE,
    LABEL_TEMP1,
    LABEL_TEMP2,
    LABEL_TEMP3,
    LABEL_TEMP4,

    LABEL_PRESSURE,
    LABEL_MMHG,
    LABEL_KPA,

    LABEL_HUMIDITY,
    LABEL_PERCENT,
    LABEL_BRIGHTNESS,
    LABEL_CORRECTION,

    LABEL_END
};

EE_Param *eeParamGet();

void saveEeParam();

void changeCorrection(int8_t direction);
void displaySwitchHourSignal(void);
void displaySwitchHourZero(void);
void displaySwitchBigNum(void);
void displayChangeRotate(int8_t direction);
void changeBrightness(int8_t direction);

#endif // EEPROM_H
