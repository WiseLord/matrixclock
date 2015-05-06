#ifndef DISPLAY_H
#define DISPLAY_H

#include <inttypes.h>

#define PARAM_UP			1
#define PARAM_DOWN			-1

/* Display modes */
enum {
	MODE_MAIN,
	MODE_EDIT_TIME,
	MODE_ALARM,
	MODE_EDIT_ALARM,
	MODE_BRIGHTNESS
};

#define MASK_HOUR_TENS		0xF0000000
#define MASK_HOUR_UNITS		0x07800000
#define MASK_MIN_TENS		0x00078000
#define MASK_MIN_UNITS		0x00003C00
#define MASK_SEC_TENS		0x00000070
#define MASK_SEC_UNITS		0x00000007

#define MASK_BIGHOUR_TENS	0xF8000000
#define MASK_BIGHOUR_UNITS	0x03E00000
#define MASK_BIGMIN_TENS	0x0007C000
#define MASK_BIGMIN_UNITS	0x00001F00

#define MASK_BR_TENS		0x0001E000
#define MASK_BR_UNITS		0x00000F00

#define MASK_ALARM			0x07C00000

#define MASK_NONE			0x00000000
#define MASK_ALL			0xFFFFFFFF

enum {
	HOUR_CURRENT = 0,
	HOUR_NEXT,
	HOUR_PREV,
};

void displayInit(void);
void displaySwitchBigNum(void);
void writeBrightness(void);
void showTime(uint32_t mask);
void scrollDate(void);
void scrollTemp(void);
void setTimeMask(uint32_t tmsk);
void showMainScreen(void);
void showTimeEdit(int8_t ch_dir);
void showAlarm(uint32_t mask);
void showAlarmEdit(int8_t ch_dir);
void setBrightnessHour(uint8_t mode);
void changeBrightness(int8_t diff);
void showBrightness(int8_t ch_dir, uint32_t mask);
void checkAlarmAndBrightness(void);

#endif /* DISPLAY_H */
