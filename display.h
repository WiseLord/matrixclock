#ifndef DISPLAY_H
#define DISPLAY_H

#include <avr/io.h>

#define PARAM_UP			1
#define PARAM_DOWN			-1

/* Display modes */
enum {
	MODE_MAIN,

	MODE_EDIT_TIME
};

void scrollDate(void);

void scrollTemp(void);

void showMainScreen(void);

void showTime(uint32_t mask);

void showTimeEdit(int8_t ch_dir);

void resetEtmOld(void);

#endif /* DISPLAY_H */
