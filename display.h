#ifndef DISPLAY_H
#define DISPLAY_H

#include <avr/io.h>

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

void initBrightness(void);
void writeBrightness(void);
void showTime(uint32_t mask);
void scrollDate(void);
void scrollTemp(void);
void setTimeMask(uint32_t tmsk);
void showMainScreen(void);
void showTimeEdit(int8_t ch_dir);
void resetEtmOld(void);
void resetAmOld(void);
void showAlarm(uint32_t mask);
void showAlarmEdit(int8_t ch_dir);
void setBrightnessHour(void);
void incBrightnessHour(void);
void changeBrightness(int8_t diff);
void showBrightness(int8_t ch_dir, uint32_t mask);
void checkAlarmAndBrightness(void);

#endif /* DISPLAY_H */
