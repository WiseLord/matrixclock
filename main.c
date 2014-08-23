#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "max7219.h"
#include "fonts.h"
#include "mtimer.h"

#define BIPER_DDR	DDRD
#define BIPER_PORT	PORTD
#define BIPER_PIN	(1<<PD5)

clock time;

uint8_t strbuf[20];

uint8_t *mkNumString(int16_t number, uint8_t width, uint8_t lead)
{
	uint8_t numdiv;
	uint8_t sign = lead;
	int8_t i;

	if (number < 0) {
		sign = '-';
		number = -number;
	}

	for (i = 0; i < width; i++)
		strbuf[i] = lead;
	strbuf[width] = '\0';
	i = width - 1;

	while (number > 0 || i == width - 1) {
		numdiv = number % 10;
		strbuf[i] = numdiv + 0x30;
		if (numdiv >= 10)
			strbuf[i] += 7;
		i--;
		number /= 10;
	}

	if (i >= 0)
		strbuf[i] = sign;

	return strbuf;
}

void showTime(uint32_t mask)
{
	static clock oldtime;

	max7219SetX(0);
	max7219LoadString(mkNumString(time.hour, 2, '0'));
	max7219SetX(12);
	max7219LoadString(mkNumString(time.min, 2, '0'));

	if (oldtime.hour / 10 != time.hour / 10)
		mask  |= 0xF00000;
	if (oldtime.hour % 10 != time.hour % 10)
		mask  |= 0x078000;
	if (oldtime.min / 10 != time.min / 10)
		mask  |= 0x000F00;
	if (oldtime.min % 10 != time.min % 10)
		mask  |= 0x000078;

	max7219PosData(10, time.sec & 0x01 ? 0x14 : 0x00);
	max7219PosData(23, time.sec);

	max7219SwitchBuf(mask, MAX7219_EFFECT_SCROLL_DOWN);

	oldtime.hour = time.hour;
	oldtime.min = time.min;

	return;
}

int main(void)
{
	max7219Init();
	max7219Fill(0x00);
	max7219LoadFont(font_ks0066_ru_08);

	mTimerInit();
	sei();

	uint8_t i;
	BIPER_DDR |= BIPER_PIN;
	for (i = 0; i < 10; i++) {
		BIPER_PORT &= ~BIPER_PIN;
		_delay_ms(20);
		BIPER_PORT |= BIPER_PIN;
		_delay_ms(20);
	}


	while(1) {

		getClock(&time);

		showTime(0x000000);

		if (time.sec == 10) {
			max7219SetX(0);
			max7219LoadString((uint8_t*)" ПОНЕДЕЛЬНИК, 4 АВГУСТА 2014г. ");
			max7219Scroll();
			getClock(&time);
			showTime(0xFFFFFF);
		}

		if (time.sec == 40) {
			max7219SetX(0);
			max7219LoadString((uint8_t*)" Температура 24.1·C ");
			max7219Scroll();
			getClock(&time);
			showTime(0xFFFFFF);
		}
	}

	return 0;
}
