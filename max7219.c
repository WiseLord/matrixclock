#include "max7219.h"
#include "fonts.h"

#include <util/delay.h>
#include <avr/pgmspace.h>

static int16_t _col;
static int16_t _end;

const uint8_t *_font;
static uint8_t fp[FONT_PARAM_COUNT];

static uint8_t scrBuf[MAX7219_ICNUMBER * 8];
static uint8_t strBuf[512];

void max7219Init(void)
{
	uint8_t ic;

	MAX7219_DDR |= (MAX7219_CLK | MAX7219_LOAD1 | MAX7219_LOAD2 | MAX7219_LOAD3 | MAX7219_DIN);

	MAX7219_PORT |= (MAX7219_LOAD1 | MAX7219_LOAD2 | MAX7219_LOAD3);
	MAX7219_PORT &= ~(MAX7219_CLK | MAX7219_DIN);

	for(ic = 0; ic < MAX7219_ICNUMBER; ic++) {
		max7219Send(ic, MAX7219_SHUTDOWN, 1);		/* Power on */
		max7219Send(ic, MAX7219_DISP_TEST, 0);		/* Test mode off */
		max7219Send(ic, MAX7219_DEC_MODE, 0);		/* Use led matrix */
		max7219Send(ic, MAX7219_INTENSITY, 15);		/* Set max. brightness */
		max7219Send(ic, MAX7219_SCAN_LIMIT, 7);		/* Scan all 8 digits (cols) */
	}

	return;
}

void max7219SetBrightness(uint8_t brightness)
{
	uint8_t ic;

	for(ic = 0; ic < MAX7219_ICNUMBER; ic++) {
		max7219Send(ic, MAX7219_INTENSITY, brightness);
	}

	return;
}

void max7219Fill(uint8_t data)
{
	uint8_t i;

	for (i = 0; i < MAX7219_ICNUMBER * 8; i++) {
		max7219Send(i / 8, MAX7219_DIGIT_0 + (i % 8), data);
	}

	return;
}

static void max7219SendByte(uint8_t data)
{
	int8_t j = 0;

	for(j = 7; j >= 0; j--) {
		MAX7219_PORT &= ~MAX7219_CLK;
		if (data & (1<<j))
			MAX7219_PORT |= MAX7219_DIN;
		else
			MAX7219_PORT &= ~MAX7219_DIN;
		MAX7219_PORT |= MAX7219_CLK;
	}

	return;
}

void max7219Send(uint8_t ic, uint8_t reg, uint8_t data)
{
	switch (ic) {
	case 0:
		MAX7219_PORT &= ~MAX7219_LOAD1;
		max7219SendByte(reg);
		max7219SendByte(data);
		MAX7219_PORT |= MAX7219_LOAD1;
		break;
	case 1:
		MAX7219_PORT &= ~MAX7219_LOAD2;
		max7219SendByte(reg);
		max7219SendByte(data);
		MAX7219_PORT |= MAX7219_LOAD2;
		break;
	case 2:
		MAX7219_PORT &= ~MAX7219_LOAD3;
		max7219SendByte(reg);
		max7219SendByte(data);
		MAX7219_PORT |= MAX7219_LOAD3;
		break;
	}

	return;
}

void max7219Show(void)
{
	uint8_t i;

	for (i = 0; i < MAX7219_ICNUMBER * 8; i++) {
		max7219Send(i / 8, MAX7219_DIGIT_0 + (i % 8), scrBuf[i]);
	}

	return;
}

void max7219LoadScr(uint8_t *buf)
{
	uint8_t i;

	for (i = 0; i < MAX7219_ICNUMBER * 8; i++) {
		scrBuf[i] = buf[i];
	}

	return;
}

void max7219PosData(uint8_t pos, uint8_t data)
{
	scrBuf[pos] = data;

	return;
}

void max7219SwitchBuf(uint32_t mask, uint8_t effect)
{
	uint8_t i, j;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < MAX7219_ICNUMBER * 8; j++) {
			if (mask & (1UL<<(MAX7219_ICNUMBER * 8 - 1 - j))) {
				switch (effect) {
				case MAX7219_EFFECT_SCROLL_DOWN:
					scrBuf[j] <<= 1;
					if (strBuf[j] & (1<<(7 - i)))
						scrBuf[j] |= 0x01;
					break;
				case MAX7219_EFFECT_SCROLL_UP:
					scrBuf[j] >>= 1;
					if (strBuf[j] & (1<<i))
						scrBuf[j] |= 0x80;
					break;
				case MAX7219_EFFECT_SCROLL_BOTH:
					if (j & 0x01) {
						scrBuf[j] <<= 1;
						if (strBuf[j] & (1<<(7 - i)))
							scrBuf[j] |= 0x01;
					} else {
						scrBuf[j] >>= 1;
						if (strBuf[j] & (1<<i))
							scrBuf[j] |= 0x80;
					}
					break;
				default:
					scrBuf[j] = strBuf[j];
					break;
				}
			}
		}
		_delay_ms(20);
		max7219Show();
	}

	return;
}

void max7219SetX(int16_t x)
{
	_col = x;

	return;
}

void max7219LoadChar(uint8_t code)
{
	uint8_t i;
	uint8_t pgmData;
	uint8_t spos;	/* Current symbol number in array */
	uint16_t oft;	/* Current symbol offset in array*/
	uint8_t swd;	/* Current symbol width */

	spos = code - ((code >= 128) ? fp[FONT_OFTNA] : fp[FONT_OFTA]);
	oft = 0;

	for (i = 0; i < spos; i++) {
		swd = pgm_read_byte(_font + i);
		oft += swd;
	}
	swd = pgm_read_byte(_font + spos);

	oft += fp[FONT_CCNT];

	for (i = 0; i < swd; i++) {
		pgmData = pgm_read_byte(_font + oft + i);
		strBuf[_col++] = pgmData;
	}
	strBuf[_col++] = 0x00;

	return;
}

void max7219LoadString(char *string)
{
	while(*string)
		max7219LoadChar(*string++);

	_end = _col;

	while(_col < 512)
		strBuf[_col++] = 0x00;

	_col = _end;

	return;
}

void max7219LoadFont(const uint8_t *font)
{
	uint8_t i;

	_font = font + FONT_PARAM_COUNT - 1;
	for (i = 0; i < FONT_PARAM_COUNT - 1; i++)
		fp[i] = pgm_read_byte(font + i);
}

void max7219Scroll(void)
{
	int16_t i;
	int8_t j;

	int16_t end = _end + 23;
	if (end > 512)
		end = 512;

	for (i = 0; i < end; i++) {
		for (j = 0; j < 23; j++) {
			scrBuf[j] = scrBuf[j + 1];
		}
		scrBuf[23] = strBuf[i];
		max7219Show();
		_delay_ms(30);
	}

	return;
}
