#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "matrix.h"
#include "fonts.h"
#include "mtimer.h"
#include "display.h"
#include "ds18x20.h"
#include "ds1307.h"
#include "alarm.h"

static uint8_t dsOnBus = 0;

void hwInit(void)
{
	_delay_ms(100);
	sei();
	ds18x20SearchDevices();


	matrixInit();
	matrixFill(0x00);
	matrixLoadFont(font_ks0066_ru_08);

	mTimerInit();
	matrixScrollTimerInit();

	initAlarm();
	initBrightness();

	dsOnBus = ds18x20Process();			/* Try to find temperature sensor */

	return;
}

int main(void)
{
	uint8_t cmd = CMD_EMPTY;
	uint8_t dispMode = MODE_MAIN;
	uint8_t dispModePrev = dispMode;
	int8_t lastParam = PARAM_UP;

	uint32_t mask = MASK_ALL;

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
			lastParam = PARAM_UP;
			switch (dispMode) {
			case MODE_MAIN:
				matrixHwScroll(MATRIX_SCROLL_STOP);
				if (dispModePrev != dispMode)
					showTime(MASK_ALL);
				break;
			case MODE_EDIT_TIME:
				editTime();
				break;
			case MODE_ALARM:
				dispMode = MODE_EDIT_ALARM;
				editAlarm();
				break;
			case MODE_EDIT_ALARM:
				editAlarm();
				break;
			case MODE_BRIGHTNESS:
				incBrightnessHour();
				break;
			}
			break;
		case CMD_BTN_2:
			lastParam = PARAM_UP;
			switch (dispMode) {
			case MODE_MAIN:
				scrollDate();
				break;
			case MODE_EDIT_TIME:
				changeTime(PARAM_UP);
				break;
			case MODE_EDIT_ALARM:
				changeAlarm(PARAM_UP);
				break;
			case MODE_BRIGHTNESS:
				changeBrightness(PARAM_UP);
				break;
			}
			break;
		case CMD_BTN_3:
			lastParam = PARAM_DOWN;
			switch (dispMode) {
			case MODE_MAIN:
				scrollTemp();
				break;
			case MODE_EDIT_TIME:
				changeTime(PARAM_DOWN);
				break;
			case MODE_EDIT_ALARM:
				changeAlarm(PARAM_DOWN);
				break;
			case MODE_BRIGHTNESS:
				changeBrightness(PARAM_DOWN);
				break;
			}
			break;
		case CMD_BTN_1_LONG:
			if (dispMode == MODE_EDIT_TIME) {
				stopEditTime();
				resetEtmOld();
				dispMode = MODE_MAIN;
				showTime(MASK_ALL);
			} else {
				dispMode = MODE_EDIT_TIME;
				editTime();
				stopEditAlarm();
				resetAmOld();
			}
			break;
		case CMD_BTN_2_LONG:
			if (dispMode == MODE_ALARM || dispMode == MODE_EDIT_ALARM) {
				dispMode = MODE_MAIN;
				setTimeMask(MASK_NONE);
				showTime(MASK_ALL);
				stopEditAlarm();
				resetAmOld();
				writeAlarm();
			} else {
				stopEditTime();
				resetEtmOld();
				dispMode = MODE_ALARM;
				mask = MASK_ALL;
				break;
			}
			break;
		case CMD_BTN_3_LONG:
			if (dispMode == MODE_BRIGHTNESS) {
				dispMode = MODE_MAIN;
				setTimeMask(MASK_NONE);
				showTime(MASK_ALL);
				writeBrightness();
			} else {
				dispMode = MODE_BRIGHTNESS;
				mask = MASK_ALL;
				setBrightnessHour();
			}
			break;
		case CMD_BTN_1_2_3_LONG:
			matrixScreenRotate();
			break;
		}

		/* Stop scroll if mode has changed */
		if (dispMode != dispModePrev) {
			matrixHwScroll(MATRIX_SCROLL_STOP);
		}

		/* Show things */
		switch (dispMode) {
		case MODE_MAIN:
			showMainScreen();
			checkAlarmAndBrightness();
			break;
		case MODE_EDIT_TIME:
			showTimeEdit(lastParam);
			checkAlarmAndBrightness();
			break;
		case MODE_ALARM:
			showAlarm(mask);
			mask = MASK_NONE;
			checkAlarmAndBrightness();
			break;
		case MODE_EDIT_ALARM:
			showAlarmEdit(lastParam);
			checkAlarmAndBrightness();
			break;
		case MODE_BRIGHTNESS:
			showBrightness(lastParam, mask);
			mask = MASK_NONE;
			break;
		}

		/* Save current mode */
		dispModePrev = dispMode;

	}

	return 0;
}
