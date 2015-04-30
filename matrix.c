#include "matrix.h"
#include "fonts.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

static uint8_t rotate = 0;

static int16_t _col;
static int16_t _end;

const uint8_t *_font;
static uint8_t fp[FONT_PARAM_COUNT];

static uint8_t scrBuf[MAX7219_ICNUMBER * 8];
static uint8_t strBuf[512];

static volatile int16_t scrollPos = 0;
static volatile uint8_t scrollMode = 0;

void matrixScreenRotate(void)
{
	rotate = !rotate;
	eeprom_write_byte(EEPROM_SCREEN_ROTATE, rotate);

	return;
}

void matrixInit(void)
{
	uint8_t ic = 0;

	MAX7219_DDR |= (MAX7219_CLK_ | MAX7219_LOAD_ | MAX7219_DIN_);

	MAX7219_PORT |= MAX7219_LOAD_;
	MAX7219_PORT &= ~(MAX7219_CLK_ | MAX7219_DIN_);

	for(ic = 0; ic < MAX7219_ICNUMBER; ic++) {
		matrixSend(ic, MAX7219_SHUTDOWN, 1);		/* Power on */
		matrixSend(ic, MAX7219_DISP_TEST, 0);		/* Test mode off */
		matrixSend(ic, MAX7219_DEC_MODE, 0);		/* Use led matrix */
		matrixSend(ic, MAX7219_INTENSITY, 15);		/* Set brightness */
		matrixSend(ic, MAX7219_SCAN_LIMIT, 7);		/* Scan all 8 digits (cols) */
	}

	return;
}

void matrixSetBrightness(uint8_t brightness)
{
	uint8_t ic;

	for(ic = 0; ic < MAX7219_ICNUMBER; ic++) {
		matrixSend(ic, MAX7219_INTENSITY, brightness);
	}

	return;
}

void matrixFill(uint8_t data)
{
	uint8_t i;

	for (i = 0; i < MAX7219_ICNUMBER * 8; i++) {
		matrixSend(i / 8, MAX7219_DIGIT_0 + (i % 8), data);
	}

	return;
}

static void max7219SendByte(uint8_t data)
{
	int8_t j = 0;

	for(j = 7; j >= 0; j--) {
		MAX7219_PORT &= ~MAX7219_CLK_;
		if (data & (1<<j))
			MAX7219_PORT |= MAX7219_DIN_;
		else
			MAX7219_PORT &= ~MAX7219_DIN_;
		MAX7219_PORT |= MAX7219_CLK_;
	}

	return;
}

static uint8_t revBits(uint8_t data)
{
	data = (data & 0xF0) >> 4 | (data & 0x0F) << 4;
	data = (data & 0xCC) >> 2 | (data & 0x33) << 2;
	data = (data & 0xAA) >> 1 | (data & 0x55) << 1;

	return data;
}

void matrixSend(uint8_t ic, uint8_t reg, uint8_t data)
{
	if (rotate) {
		ic = MAX7219_ICNUMBER - 1 - ic;
		if (reg >= MAX7219_DIGIT_0 && reg <= MAX7219_DIGIT_7) {
			reg = MAX7219_DIGIT_7 + 1 - reg;
			data = revBits(data);
		}
	}

	uint8_t i = 0;

	if (ic < MAX7219_ICNUMBER) {
		MAX7219_PORT &= ~MAX7219_LOAD_;

		/* Send NO_OP to following ics */
		for(i = ic; i < (MAX7219_ICNUMBER - 1); i++) {
			max7219SendByte(MAX7219_NO_OP); /* NO_OP reg */
			max7219SendByte(MAX7219_NO_OP); /* NO_OP data */
		}

		/* Send info to selected ic */
		max7219SendByte(reg);
		max7219SendByte(data);

		/* Send NO_OP to previous ics */
		for(i=0; i<ic; i++) {
			max7219SendByte(MAX7219_NO_OP); /* NO_OP reg */
			max7219SendByte(MAX7219_NO_OP); /* NO_OP data */
		}

		MAX7219_PORT |= MAX7219_LOAD_;
	}

	return;
}

void matrixShow(void)
{
	uint8_t i;

	for (i = 0; i < MAX7219_ICNUMBER * 8; i++) {
		matrixSend(i / 8, MAX7219_DIGIT_0 + (i % 8), scrBuf[i]);
	}

	return;
}

void matrixPosData(uint8_t pos, uint8_t data)
{
	scrBuf[pos] = data;

	return;
}

void matrixSwitchBuf(uint32_t mask, uint8_t effect)
{
	uint8_t i, j;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < MAX7219_ICNUMBER * 8; j++) {
			if (mask & (1UL<<(MAX7219_ICNUMBER * 8 - 1 - j))) {
				switch (effect) {
				case MAX7219_EFFECT_SCROLL_DOWN:
					scrBuf[j] <<= 1;
					if (strBuf[j] & (128>>i))
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
						if (strBuf[j] & (128 >> i))
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
		_delay_ms(25);
		matrixShow();
	}

	return;
}

void matrixSetX(int16_t x)
{
	_col = x;

	return;
}

void matrixLoadChar(uint8_t code)
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

static void max7219ClearBufTail(void)
{
	_end = _col;
	while(_col < 512)
		strBuf[_col++] = 0x00;
	_col = _end;

	return;
}

void matrixLoadString(char *string)
{
	while(*string)
		matrixLoadChar(*string++);

	max7219ClearBufTail();

	return;
}

void matrixLoadNumString(char *string)
{
	while(*string)
		matrixLoadChar(0xC0 + *string++);

	max7219ClearBufTail();

	return;
}

void matrixLoadStringPgm(const char *string)
{
	char ch;
	uint8_t i = 0;

	ch = pgm_read_byte(&string[i++]);
	while(ch) {
		matrixLoadChar(ch);
		ch = pgm_read_byte(&string[i++]);
	}

	max7219ClearBufTail();

	return;
}

void matrixLoadFont(const uint8_t *font)
{
	uint8_t i;

	_font = font + FONT_PARAM_COUNT - 1;
	for (i = 0; i < FONT_PARAM_COUNT - 1; i++)
		fp[i] = pgm_read_byte(font + i);
}

void matrixScrollTimerInit(void)
{
	TIMSK |= (1<<TOIE2);							/* Enable Timer2 overflow interrupt */
	TCCR2 |= (1<<CS22) | (1<<CS21) | (1<<CS20);		/* Set timer prescaller to 1024 (7812 Hz) */

	return;
}

ISR (TIMER2_OVF_vect)								/* 7812 / 256 = 30 polls/sec */
{

	if (scrollMode) {
		int8_t i;

		for (i = 0; i < MAX7219_ICNUMBER * 8 - 1; i++) {
			scrBuf[i] = scrBuf[i + 1];
		}
		scrBuf[MAX7219_ICNUMBER * 8 - 1] = strBuf[scrollPos];
		matrixShow();

		scrollPos++;

		if (scrollPos >= _end + MAX7219_ICNUMBER * 8 - 1 || scrollPos >= 512) {
			scrollMode = 0;
			scrollPos = 0;
		}

	}

	return;
}

void matrixHwScroll(uint8_t status)
{
	scrollPos = 0;
	scrollMode = status;

	return;
}

uint8_t matrixGetScrollMode(void)
{
	return scrollMode;
}
