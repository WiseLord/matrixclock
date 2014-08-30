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

	hwInit();

	while(1) {
		cmd = getBtnCmd();

		switch (cmd) {
		case CMD_BTN_1:
			startBeeper(1000);
			break;
		case CMD_BTN_2:
			startBeeper(2000);
			break;
		case CMD_BTN_3:
			startBeeper(3000);
		default:
			break;
		}

		showMainScreen();
	}

	return 0;
}
