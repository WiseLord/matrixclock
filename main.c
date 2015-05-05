#include <avr/interrupt.h>
#include <util/delay.h>

#include "matrix.h"
#include "fonts.h"
#include "mtimer.h"
#include "display.h"
#include "ds18x20.h"
#include "ds1307.h"
#include "alarm.h"

void hwInit(void)
{
	_delay_ms(100);
	sei();
	ds18x20SearchDevices();

	displayInit();
	matrixInit();

	mTimerInit();
	matrixScrollTimerInit();

	initAlarm();

	return;
}

int main(void)
{
	uint8_t cmd = CMD_EMPTY;
	uint8_t dispMode = MODE_MAIN;
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

		if (cmd != CMD_EMPTY)
			matrixHwScroll(MATRIX_SCROLL_STOP);

		/* Handle command */
		switch (cmd) {
		case CMD_BTN_1:
			lastParam = PARAM_UP;
			switch (dispMode) {
			case MODE_EDIT_TIME:
				editTime();
				break;
			case MODE_ALARM:
				dispMode = MODE_EDIT_ALARM;
				editAlarm();
				break;
			case MODE_EDIT_ALARM:
				if (getAlarmMode() == A_SUNDAY) {
					dispMode = MODE_ALARM;
					showAlarm(MASK_ALL);
				} else {
					editAlarm();
				}
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
			switch (dispMode) {
			case MODE_EDIT_TIME:
				stopEditTime();
				resetEtmOld();
				dispMode = MODE_MAIN;
				showTime(MASK_ALL);
				break;
			case MODE_MAIN:
				dispMode = MODE_EDIT_TIME;
				editTime();
			}
			break;
		case CMD_BTN_2_LONG:
			switch (dispMode) {
			case MODE_ALARM:
			case MODE_EDIT_ALARM:
				dispMode = MODE_MAIN;
				showTime(MASK_ALL);
				stopEditAlarm();
				resetAmOld();
				writeAlarm();
				break;
			case MODE_MAIN:
				dispMode = MODE_ALARM;
				mask = MASK_ALL;
				break;
			}
			break;
		case CMD_BTN_3_LONG:
			switch (dispMode) {
			case MODE_BRIGHTNESS:
				dispMode = MODE_MAIN;
				showTime(MASK_ALL);
				writeBrightness();
				break;
			case MODE_MAIN:
				dispMode = MODE_BRIGHTNESS;
				mask = MASK_ALL;
				setBrightnessHour();
				break;
			}
			break;
		case CMD_BTN_2_3_LONG:
			displaySwitchBigNum();
			showTime(MASK_ALL);
			break;
		case CMD_BTN_1_2_3_LONG:
//			matrixScreenRotate();
			break;
		}

		if (dispMode != MODE_BRIGHTNESS)
			checkAlarmAndBrightness();

		/* Show things */
		switch (dispMode) {
		case MODE_MAIN:
			showMainScreen();
			break;
		case MODE_EDIT_TIME:
			showTimeEdit(lastParam);
			break;
		case MODE_ALARM:
			showAlarm(mask);
			mask = MASK_NONE;
			break;
		case MODE_EDIT_ALARM:
			showAlarmEdit(lastParam);
			break;
		case MODE_BRIGHTNESS:
			showBrightness(lastParam, mask);
			mask = MASK_NONE;
			break;
		}
	}

	return 0;
}
