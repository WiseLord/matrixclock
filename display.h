#ifndef DISPLAY_H
#define DISPLAY_H

#include <inttypes.h>

#define PARAM_UP				1
#define PARAM_DOWN				-1

/* Display modes */
enum {
	MODE_MAIN,
	MODE_EDIT_TIME,
	MODE_ALARM,
	MODE_EDIT_ALARM,
	MODE_BRIGHTNESS
};

#define MASK_HOUR_TENS			0xF0000000UL
#define MASK_HOUR_UNITS			0x07800000UL
#define MASK_MIN_TENS			0x00078000UL
#define MASK_MIN_UNITS			0x00003C00UL

#define MASK_BIGHOUR_TENS		0xF8000000UL
#define MASK_BIGHOUR_UNITS		0x03E00000UL
#define MASK_BIGMIN_TENS		0x0007C000UL
#define MASK_BIGMIN_UNITS		0x00001F00UL

#define MASK_EXTRAHOUR_TENS		0x7E000000UL
#define MASK_EXTRAHOUR_UNITS	0x00FC0000UL
#define MASK_EXTRAMIN_TENS		0x00003F00UL
#define MASK_EXTRAMIN_UNITS		0x0000007EUL

#define MASK_SEC_TENS			0x00000070UL
#define MASK_SEC_UNITS			0x00000007UL

#define MASK_BR_TENS			0x0001E000UL
#define MASK_BR_UNITS			0x00000F00UL

#define MASK_ALARM				0x07C00000UL

#define MASK_NONE				0x00000000UL
#define MASK_ALL				0xFFFFFFFFUL

enum {
	HOUR_CURRENT = 0,
	HOUR_NEXT,
	HOUR_PREV,
};

enum {
	NUM_NORMAL,
	NUM_BIG,
	NUM_EXTRA,
	NUM_END,
};

void displayInit(void);
void displaySwitchBigNum(void);
void displaySwitchHourZero(void);
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
