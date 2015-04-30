#include "matrix.h"

#include "fonts.h"
#include "ht1632.h"
#include "max7219.h"
#include "eeprom.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

static uint8_t rotate = 0;

static int16_t _col;
static int16_t _end;

const uint8_t *_font;
static uint8_t fp[FONT_PARAM_COUNT];

static uint8_t fb[32];
static uint8_t strBuf[512];

static volatile int16_t scrollPos = 0;
static volatile uint8_t scrollMode = 0;

/*
static uint8_t matrixRevBits(uint8_t data)
{
	data = (data & 0xF0) >> 4 | (data & 0x0F) << 4;
	data = (data & 0xCC) >> 2 | (data & 0x33) << 2;
	data = (data & 0xAA) >> 1 | (data & 0x55) << 1;

	return data;
}
*/

static void matrixLoadChar(uint8_t code)
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

static void matrixClearBufTail(void)
{
	_end = _col;
	while(_col < 512)
		strBuf[_col++] = 0x00;
	_col = _end;

	return;
}

static void matrixUpdate(void)
{
#if defined(HT1632)
	uint8_t i, j, ind;
	uint8_t *fbInd = fb;
	static uint8_t data[8];

	ht1632SetAddr(0);

	for (ind = 0; ind < 4; ind++, fbInd += 8) {
		for (i = 0; i < 8; i++) {
			data[i] = 0;
			for (j = 0; j < 8; j++)
				if (fbInd[j] & (1 << i))
					data[i] |= (0x80 >> j);
		}
		ht1632SendByteSeq(data, sizeof(data));
	}
#else
	max7219SendDataBuf(fb);
#endif

	return;
}

void matrixInit(void)
{
#if defined(HT1632)
	ht1632Init();
#else
	max7219Init();
#endif

	return;
}

void matrixScreenRotate(void)
{
	rotate = !rotate;
	eeprom_write_byte(EEPROM_SCREEN_ROTATE, rotate);

	return;
}

void matrixSetBrightness(uint8_t brightness)
{
#if defined(HT1632)
	ht1632SetBrightness(brightness);
#else
	max7219SendCmd(MAX7219_INTENSITY, brightness);
#endif

	return;
}

void matrixFill(uint8_t data)
{
	uint8_t i;

	for (i = 0; i < sizeof(fb); i++)
		fb[i] = data;

	matrixUpdate();

	return;
}

void matrixPosData(uint8_t pos, uint8_t data)
{
	fb[pos] = data;

	return;
}

void matrixSwitchBuf(uint32_t mask, uint8_t effect)
{
	uint8_t i, j;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < MAX7219_ICNUMBER * 8; j++) {
			if (mask & (1UL<<(MAX7219_ICNUMBER * 8 - 1 - j))) {
				switch (effect) {
				case MATRIX_EFFECT_SCROLL_DOWN:
					fb[j] <<= 1;
					if (strBuf[j] & (128>>i))
						fb[j] |= 0x01;
					break;
				case MATRIX_EFFECT_SCROLL_UP:
					fb[j] >>= 1;
					if (strBuf[j] & (1<<i))
						fb[j] |= 0x80;
					break;
				case MATRIX_EFFECT_SCROLL_BOTH:
					if (j & 0x01) {
						fb[j] <<= 1;
						if (strBuf[j] & (128 >> i))
							fb[j] |= 0x01;
					} else {
						fb[j] >>= 1;
						if (strBuf[j] & (1<<i))
							fb[j] |= 0x80;
					}
					break;
				default:
					fb[j] = strBuf[j];
					break;
				}
			}
		}
		_delay_ms(20);
		matrixUpdate();
	}

	return;
}

void matrixSetX(int16_t x)
{
	_col = x;

	return;
}

void matrixLoadString(char *string)
{
	while(*string)
		matrixLoadChar(*string++);

	matrixClearBufTail();

	return;
}

void matrixLoadNumString(char *string)
{
	while(*string)
		matrixLoadChar(0xC0 + *string++);

	matrixClearBufTail();

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

	matrixClearBufTail();

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
			fb[i] = fb[i + 1];
		}
		fb[MAX7219_ICNUMBER * 8 - 1] = strBuf[scrollPos];
		matrixUpdate();

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
