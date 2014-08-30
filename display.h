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
	MODE_EDIT_ALARM
};

void scrollDate(void);

void scrollTemp(void);

void setTimeMask(uint32_t tmsk);

void showMainScreen(void);

void showTime(uint32_t mask);

void showTimeEdit(int8_t ch_dir);

void resetEtmOld(void);

void resetAmOld(void);

void showAlarm(uint32_t mask);

void showAlarmEdit(int8_t ch_dir);

void checkAlarm(void);

#endif /* DISPLAY_H */
