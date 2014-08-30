#ifndef DISPLAY_H
#define DISPLAY_H

#include <avr/io.h>

/* Display modes */
enum {
	MODE_MAIN
};

void scrollDate(void);

void scrollTemp(void);

void showMainScreen(void);

#endif /* DISPLAY_H */
