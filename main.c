#include <avr/interrupt.h>
#include <util/delay.h>

#include "matrix.h"
#include "fonts.h"
#include "mtimer.h"
#include "display.h"
#include "ds18x20.h"
#include "rtc.h"
#include "alarm.h"
#include "bmp180.h"
#include "dht22.h"

void hwInit(void)
{
	_delay_ms(250);

	ds18x20SearchDevices();
	bmp180Init();
	dht22Init();

	displayInit();

	mTimerInit();

	matrixScrollAndADCInit();

	rtcReadTime();
	alarmInit();

	rtc.etm = RTC_NOEDIT;

	sei();

	return;
}

int main(void)
{
	uint8_t cmd;
	uint8_t dispMode = MODE_MAIN;

	static int8_t direction = PARAM_UP;

	hwInit();

	showTimeMasked();

	ds18x20Process();
	setSensTimer(TEMP_MEASURE_TIME);

	while(1) {
		/* Update sensors with SENSOR_POLL_INTERVAL period */
		if (getSensTimer() == 0) {
			setSensTimer(SENSOR_POLL_INTERVAL);
			ds18x20Process();
			if (bmp180HaveSensor())
				bmp180Convert();
			dht22Read();
		}

		/* Update brightness only when not in brightness setup */
		if (dispMode != MODE_BRIGHTNESS)
			checkAlarmAndBrightness();

		/* Get command from buttons */
		cmd = getBtnCmd();

		/* Beep on button pressed */
		if (cmd != CMD_EMPTY) {
			if (cmd < CMD_BTN_1_LONG)
				startBeeper(BEEP_SHORT);
			else
				startBeeper(BEEP_LONG);
		}

		/* Stop scrolling on any button pressed */
		if (cmd != CMD_EMPTY)
			matrixHwScroll(MATRIX_SCROLL_STOP);

		/* Handle command */
		switch (cmd) {
		case CMD_BTN_1:
			direction = PARAM_UP;
			switch (dispMode) {
			case MODE_EDIT_TIME:
				rtcNextEditParam();
				break;
			case MODE_EDIT_ALARM:
				alarmNextEditParam();
				break;
			case MODE_BRIGHTNESS:
				break;
			default:
				showTimeMasked();
			}
			break;
		case CMD_BTN_2:
			direction = PARAM_UP;
			switch (dispMode) {
			case MODE_MAIN:
				startScroll(SCROLL_DATE);
				break;
			case MODE_EDIT_TIME:
				rtcChangeTime(PARAM_UP);
				break;
			case MODE_EDIT_ALARM:
				alarmChange(PARAM_UP);
				break;
			case MODE_BRIGHTNESS:
				changeBrightness(PARAM_UP);
				break;
			}
			break;
		case CMD_BTN_3:
			direction = PARAM_DOWN;
			switch (dispMode) {
			case MODE_MAIN:
				startScroll(SCROLL_TEMP);
				break;
			case MODE_EDIT_TIME:
				rtcChangeTime(PARAM_DOWN);
				break;
			case MODE_EDIT_ALARM:
				alarmChange(PARAM_DOWN);
				break;
			case MODE_BRIGHTNESS:
				changeBrightness(PARAM_DOWN);
				break;
			}
			break;
		case CMD_BTN_1_LONG:
			switch (dispMode) {
			case MODE_EDIT_TIME:
				rtc.etm = RTC_NOEDIT;
				dispMode = MODE_MAIN;
				showTimeMasked();
				break;
			case MODE_MAIN:
				dispMode = MODE_EDIT_TIME;
				rtcNextEditParam();
			}
			break;
		case CMD_BTN_2_LONG:
			switch (dispMode) {
			case MODE_EDIT_ALARM:
				alarm.eam = ALARM_NOEDIT;
				alarmSave();
				dispMode = MODE_MAIN;
				showTimeMasked();
				break;
			case MODE_MAIN:
				dispMode = MODE_EDIT_ALARM;
				alarmNextEditParam();
				break;
			}
			break;
		case CMD_BTN_3_LONG:
			switch (dispMode) {
			case MODE_BRIGHTNESS:
				dispMode = MODE_MAIN;
				saveMaxBrightness();
				showTimeMasked();
				break;
			case MODE_MAIN:
				dispMode = MODE_BRIGHTNESS;
				showBrightness(direction, MASK_ALL);
				break;
			}
			break;
		case CMD_BTN_1_2_LONG:
			displaySwitchHourZero();
			dispMode = MODE_MAIN;
			showTimeMasked();
			break;
		case CMD_BTN_2_3_LONG:
			displaySwitchBigNum();
			dispMode = MODE_MAIN;
			showTimeMasked();
			break;
		case CMD_BTN_1_2_3_LONG:
			matrixScreenRotate();
			break;
		}

		/* Show things */
		switch (dispMode) {
		case MODE_MAIN:
			showMainScreen();
			break;
		case MODE_EDIT_TIME:
			showTimeEdit(direction);
			break;
		case MODE_EDIT_ALARM:
			showAlarmEdit(direction);
			break;
		case MODE_BRIGHTNESS:
			showBrightness(direction, MASK_NONE);
			break;
		}
	}

	return 0;
}
