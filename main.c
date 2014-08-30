#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "max7219.h"
#include "fonts.h"
#include "mtimer.h"
#include "i2c.h"
#include "ds1307.h"
#include "ds18x20.h"


int8_t *dateTime;

uint8_t strbuf[20];


/* TODO: Read labels from EEPROM */
uint8_t *weekLabel[] = {
	(uint8_t*)"Воскресенье",
	(uint8_t*)"Понедельник",
	(uint8_t*)"Вторник",
	(uint8_t*)"Среда",
	(uint8_t*)"Четверг",
	(uint8_t*)"Пятница",
	(uint8_t*)"Суббота",
};
uint8_t *monthLabel[] = {
	(uint8_t*)"декабря",
	(uint8_t*)"января",
	(uint8_t*)"февраля",
	(uint8_t*)"марта",
	(uint8_t*)"апреля",
	(uint8_t*)"мая",
	(uint8_t*)"июня",
	(uint8_t*)"июля",
	(uint8_t*)"августа",
	(uint8_t*)"сентября",
	(uint8_t*)"октября",
	(uint8_t*)"ноября",
};

uint8_t *mkNumberString(int16_t value, uint8_t width, uint8_t prec, uint8_t lead)
{
	uint8_t sign = lead;
	int8_t pos;

	if (value < 0) {
		sign = '-';
		value = -value;
	}

	/* Clear buffer and go to it's tail */
	for (pos = 0; pos < width + prec; pos++)
		strbuf[pos] = lead;
	strbuf[width + prec] = '\0';
	pos = width + prec - 1;

	/* Fill buffer from right to left */
	while (value > 0 || pos > width - 2) {
		if (prec && (width - pos - 1 == 0))
			strbuf[pos--] = '.';
		strbuf[pos] = value % 10 + 0x30;
		pos--;
		value /= 10;
	}

	if (pos >= 0)
		strbuf[pos] = sign;

	return strbuf;
}


void showTime(uint32_t mask)
{
	static int8_t oldDateTime[7];

	dateTime = readTime();

	max7219SetX(0);
	max7219LoadString(mkNumberString(dateTime[HOUR], 2, 0, '0'));
	max7219SetX(12);
	max7219LoadString(mkNumberString(dateTime[MIN], 2, 0, '0'));

	if (oldDateTime[HOUR] / 10 != dateTime[HOUR] / 10)
		mask  |= 0xF00000;
	if (oldDateTime[HOUR] % 10 != dateTime[HOUR] % 10)
		mask  |= 0x078000;
	if (oldDateTime[MIN] / 10 != dateTime[MIN] / 10)
		mask  |= 0x000F00;
	if (oldDateTime[MIN] % 10 != dateTime[MIN] % 10)
		mask  |= 0x000078;

	max7219PosData(10, dateTime[SEC] & 0x01 ? 0x00 : 0x24);
	max7219PosData(23, dateTime[SEC]);

	max7219SwitchBuf(mask, MAX7219_EFFECT_SCROLL_DOWN);

	oldDateTime[HOUR] = dateTime[HOUR];
	oldDateTime[MIN] = dateTime[MIN];

	return;
}

void hwInit(void)
{
	max7219Init();
	max7219Fill(0x00);
	max7219LoadFont(font_ks0066_ru_08);

	I2CInit();
	mTimerInit();

	sei();

//	uint8_t i;
//	BEEPER_DDR |= BEEPER_PIN;
//	for (i = 0; i < 10; i++) {
//		BEEPER_PORT &= ~BEEPER_PIN;
//		_delay_ms(20);
//		BEEPER_PORT |= BEEPER_PIN;
//		_delay_ms(20);
//	}

	return;
}

void loadDateString(void)
{
	max7219SetX(0);
	max7219LoadString((uint8_t*)" ");
	max7219LoadString(weekLabel[dateTime[WEEK] % 7]);
	max7219LoadString((uint8_t*)", ");
	max7219LoadString(mkNumberString(dateTime[DAY], 2, 0, ' '));
	max7219LoadString((uint8_t*)" ");
	max7219LoadString(monthLabel[dateTime[MONTH] % 12]);
	max7219LoadString((uint8_t*)" 20");
	max7219LoadString(mkNumberString(dateTime[YEAR], 2, 0, ' '));
	max7219LoadString((uint8_t*)"г. ");

	return;
}

void loadTempString(void)
{
	max7219SetX(0);
	max7219LoadString((uint8_t*)" ");
	max7219LoadString(mkNumberString(ds18x20GetTemp(0), 4, 1, ' '));
	max7219LoadString((uint8_t*)"·C в комнате, ");
	max7219LoadString(mkNumberString(ds18x20GetTemp(1), 4, 1, ' '));
	max7219LoadString((uint8_t*)"·C на улице");

	return;
}

int main(void)
{
	hwInit();
	ds18x20Process();

	showTime(0xFFFFFF);

	while(1) {
		if (getTempStartTimer() == 0) {
			setTempStartTimer(TEMP_POLL_INTERVAL);
			ds18x20Process();
		}

		showTime(0x000000);

		if (dateTime[SEC] == 10) {
			loadDateString();
			max7219Scroll();
			showTime(0xFFFFFF);
		}

		if (dateTime[SEC] == 40) {
			loadTempString();
			max7219Scroll();
			showTime(0xFFFFFF);
		}

	}

	return 0;
}
