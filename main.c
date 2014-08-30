#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "max7219.h"
#include "fonts.h"
#include "i2c.h"
#include "mtimer.h"
#include "display.h"

void hwInit(void)
{
	max7219Init();
	max7219Fill(0x00);
	max7219LoadFont(font_ks0066_ru_08);

	I2CInit();
	mTimerInit();
	scrollTimerInit();

	sei();

	startBeeper(1000);

	return;
}

int main(void)
{
	uint8_t cmd = CMD_EMPTY;
	uint8_t dispMode = MODE_MAIN;

	hwInit();

	while(1) {
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
			break;
		case CMD_BTN_2:
			scrollDate();
			break;
		case CMD_BTN_3:
			scrollTemp();
			break;
		case CMD_BTN_1_LONG:
			break;
		case CMD_BTN_2_LONG:
			break;
		case CMD_BTN_3_LONG:
			break;
		default:
			break;
		}

		/* Show things */
		switch (dispMode) {
		case MODE_MAIN:
			showMainScreen();
			break;
		}

	}

	return 0;
}
