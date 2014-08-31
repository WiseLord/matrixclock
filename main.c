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
#include "alarm.h"

void hwInit(void)
{
	max7219Init();
	max7219Fill(0x00);
	max7219LoadFont(font_ks0066_ru_08);

	I2CInit();
	mTimerInit();
	scrollTimerInit();

	initAlarm();
	initBrightness();

	sei();

	return;
}

int main(void)
{
	uint8_t cmd = CMD_EMPTY;
	uint8_t dispMode = MODE_MAIN;
	uint8_t dispModePrev = dispMode;
	int8_t lastParam = PARAM_UP;

	uint32_t mask = 0xFFFFFF;

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
			switch (dispMode) {
			case MODE_MAIN:
				max7219HwScroll(MAX7219_SCROLL_STOP);
				if (dispModePrev != dispMode)
					showTime(0xFFFFFF);
				break;
			case MODE_EDIT_TIME:
				editTime();
				lastParam = PARAM_UP;
				break;
			case MODE_ALARM:
				dispMode = MODE_EDIT_ALARM;
				editAlarm();
				lastParam = PARAM_UP;
				break;
			case MODE_EDIT_ALARM:
				editAlarm();
				lastParam = PARAM_UP;
				break;
			case MODE_BRIGHTNESS:
				incBrightnessHour();
				lastParam = PARAM_UP;
				break;
			}
			break;
		case CMD_BTN_2:
			switch (dispMode) {
			case MODE_MAIN:
				scrollDate();
				break;
			case MODE_EDIT_TIME:
				changeTime(PARAM_UP);
				lastParam = PARAM_UP;
				break;
			case MODE_EDIT_ALARM:
				changeAlarm(PARAM_UP);
				lastParam = PARAM_UP;
				break;
			case MODE_BRIGHTNESS:
				changeBrightness(PARAM_UP);
				lastParam = PARAM_UP;
				break;
			}
			break;
		case CMD_BTN_3:
			switch (dispMode) {
			case MODE_MAIN:
				scrollTemp();
				break;
			case MODE_EDIT_TIME:
				changeTime(PARAM_DOWN);
				lastParam = PARAM_DOWN;
				break;
			case MODE_EDIT_ALARM:
				changeAlarm(PARAM_DOWN);
				lastParam = PARAM_DOWN;
				break;
			case MODE_BRIGHTNESS:
				changeBrightness(PARAM_DOWN);
				lastParam = PARAM_DOWN;
				break;
			}
			break;
		case CMD_BTN_1_LONG:
			switch (dispMode) {
			case MODE_MAIN:
				dispMode = MODE_EDIT_TIME;
				editTime();
				lastParam = PARAM_UP;
				break;
			case MODE_EDIT_TIME:
				stopEditTime();
				resetEtmOld();
				dispMode = MODE_MAIN;
				showTime(0xFFFFFF);
				break;
			case MODE_ALARM:
				dispMode = MODE_EDIT_ALARM;
				editAlarm();
				lastParam = PARAM_UP;
				break;
			case MODE_EDIT_ALARM:
				stopEditAlarm();
				resetAmOld();
				writeAlarm();
				dispMode = MODE_ALARM;
				showAlarm(0xFFFFFF);
				break;
			}
			break;
		case CMD_BTN_2_LONG:
			switch (dispMode) {
			case MODE_MAIN:
				dispMode = MODE_ALARM;
				mask = 0xFFFFFF;
				break;
			case MODE_ALARM:
			case MODE_EDIT_ALARM:
				dispMode = MODE_MAIN;
				setTimeMask(0x000000);
				showTime(0xFFFFFF);
				writeAlarm();
				break;
			}
			break;
		case CMD_BTN_3_LONG:
			switch (dispMode) {
			case MODE_MAIN:
				dispMode = MODE_BRIGHTNESS;
				mask = 0xFFFFFF;
				setBrightnessHour();
				break;
			case MODE_BRIGHTNESS:
				dispMode = MODE_MAIN;
				setTimeMask(0x000000);
				showTime(0xFFFFFF);
				writeBrightness();
				break;
			}
			break;
		case CMD_BTN_1_2_3_LONG:
			max7219ScreenRotate();
			break;
		}

		/* Stop scroll if mode has changed */
		if (dispMode != dispModePrev) {
			max7219HwScroll(MAX7219_SCROLL_STOP);
		}

		/* Show things */
		switch (dispMode) {
		case MODE_MAIN:
			showMainScreen();
			checkAlarmAndBrightness();
			break;
		case MODE_EDIT_TIME:
			showTimeEdit(lastParam);
			break;
		case MODE_ALARM:
			showAlarm(mask);
			mask = 0x000000;
			break;
		case MODE_EDIT_ALARM:
			showAlarmEdit(lastParam);
			break;
		case MODE_BRIGHTNESS:
			showBrightness(lastParam, mask);
			mask = 0x000000;
			break;
		}

		/* Save current mode */
		dispModePrev = dispMode;

	}

	return 0;
}
