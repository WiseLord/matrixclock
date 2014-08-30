#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "max7219.h"
#include "fonts.h"
#include "i2c.h"
#include "mtimer.h"
#include "display.h"
#include "ds18x20.h"
#include "ds1307.h"

void hwInit(void)
{
	max7219Init();
	max7219Fill(0x00);
	max7219LoadFont(font_ks0066_ru_08);

	I2CInit();
	mTimerInit();
	scrollTimerInit();

	sei();

//	startBeeper(1000);

	return;
}

int main(void)
{
	uint8_t cmd = CMD_EMPTY;
	uint8_t dispMode = MODE_MAIN;
	uint8_t dispModePrev = dispMode;
	int8_t lastParam = PARAM_UP;

	hwInit();

	while(1) {
		if (getTempStartTimer() == 0) {
			setTempStartTimer(TEMP_POLL_INTERVAL);
			ds18x20Process();
		}

		cmd = getBtnCmd();

		/* Beep on command */
		if (cmd != CMD_EMPTY) {
			if (cmd < CMD_BTN_1_LONG)
				startBeeper(80);
			else
				startBeeper(160);
		}

		/* Handle command */
		switch (cmd) {
		case CMD_BTN_1:
			if (dispMode == MODE_MAIN) {
				max7219HwScroll(MAX7219_SCROLL_STOP);
				showTime(0xFFFFFF);
			} else if (dispMode == MODE_EDIT_TIME) {
				editTime();
			}
			break;
		case CMD_BTN_2:
			if (dispMode == MODE_MAIN) {
				scrollDate();
			} else {
				changeTime(PARAM_UP);
				lastParam = PARAM_UP;
			}
			break;
		case CMD_BTN_3:
			if (dispMode == MODE_MAIN) {
				scrollTemp();
			} else {
				changeTime(PARAM_DOWN);
				lastParam = PARAM_DOWN;
			}
			break;
		case CMD_BTN_1_LONG:
			if (dispMode == MODE_MAIN) {
				dispMode = MODE_EDIT_TIME;
				editTime();
				lastParam = PARAM_UP;
			} else {
				stopEditTime();
				resetEtmOld();
				dispMode = MODE_MAIN;
				showTime(0xFFFFFF);
			}
			break;
		case CMD_BTN_2_LONG:
			break;
		case CMD_BTN_3_LONG:
			break;
		default:
			break;
		}

		/* Clear screen if mode has changed */
		if (dispMode != dispModePrev) {
			max7219HwScroll(MAX7219_SCROLL_STOP);
			max7219Fill(0x00);
		}

		/* Show things */
		switch (dispMode) {
		case MODE_MAIN:
			showMainScreen();
			break;
		case MODE_EDIT_TIME:
			showTimeEdit(lastParam);
			break;
		}

		/* Save current mode */
		dispModePrev = dispMode;


	}

	return 0;
}
