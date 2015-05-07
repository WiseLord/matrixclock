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

static uint8_t fb[32];
static uint8_t strBuf[MATRIX_BUFFER_SIZE];

static volatile int16_t scrollPos = 0;
static volatile uint8_t scrollMode = 0;

static void matrixLoadChar(uint8_t code)
{
	uint8_t i;
	uint8_t pgmData;
	uint16_t oft;	/* Current symbol offset in array*/

	oft = (code - ' ' - (code > 128 ? 0x20 : 0)) * 5;

	for (i = 0; i < 5; i++) {
		pgmData = pgm_read_byte(font_cp1251_08 + oft + i);
		if (pgmData != 0x99)
			strBuf[_col++] = pgmData;
	}
	strBuf[_col++] = 0x00;

	return;
}

static void matrixLoadBigNumChar(uint8_t code)
{
	uint8_t i;
	uint8_t data;

	for (i = 0; i < 5; i++) {
		if (code < '0' || code > '9')
			data = 0x00;
		else
			data = eeprom_read_byte(EEPROM_BIG_NUM_FONT + (code - 0x30) * 5 + i);
		strBuf[_col++] = data;
	}
	strBuf[_col++] = 0x00;

	return;
}

static void matrixLoadSmallNumChar(uint8_t code)
{
	uint8_t i;
	uint8_t data;

	for (i = 0; i < 3; i++) {
		if (code < '0' || code > '9')
			data = 0x00;
		else
			data = pgm_read_byte(font_smallnum + (code - 0x30) * 3 + i);
		strBuf[_col++] = data;
	}
	strBuf[_col++] = 0x00;

	return;
}

static void matrixClearBufTail(void)
{
	_end = _col;
	while(_col < MATRIX_BUFFER_SIZE)
		strBuf[_col++] = 0x00;
	_col = _end;

	return;
}

static void matrixUpdate(void)
{
#if defined(HT1632)
	ht1632SendDataBuf(fb, rotate);
#else
	max7219SendDataBuf(fb, rotate);
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

	matrixFill(0x00);
	rotate = eeprom_read_byte(EEPROM_SCREEN_ROTATE);

	return;
}

void matrixSetBrightness(uint8_t brightness)
{
	if (!scrollMode) {
#if defined(HT1632)
		ht1632SendCmd(HT1632_CMD_DUTY | brightness);
#else
		max7219SendCmd(MAX7219_INTENSITY, brightness);
#endif
	}

	return;
}

void matrixScreenRotate(void)
{
	rotate = !rotate;
	eeprom_update_byte(EEPROM_SCREEN_ROTATE, rotate);

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
		for (j = 0; j < MATRIX_NUMBER * 8; j++) {
			if (mask & (1UL<<(MATRIX_NUMBER * 8 - 1 - j))) {
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

void matrixBigNumString(char *string)
{
	while(*string)
		matrixLoadBigNumChar(*string++);

	matrixClearBufTail();
}

void matrixSmallNumString(char *string)
{
	while(*string)
		matrixLoadSmallNumChar(*string++);

	matrixClearBufTail();

	return;
}

void matrixLoadStringEeprom(uint8_t *string)
{
	char ch;
	uint8_t i = 0;

	ch = eeprom_read_byte(&string[i++]);
	while(ch) {
		matrixLoadChar(ch);
		ch = eeprom_read_byte(&string[i++]);
	}

	matrixClearBufTail();

	return;
}

void matrixScrollTimerInit(void)
{
#if defined(atmega8)
	TIMSK |= (1<<TOIE2);							/* Enable Timer2 overflow interrupt */
	TCCR2 |= (1<<CS22) | (1<<CS21) | (1<<CS20);		/* Set timer prescaller to 1024 (7812 Hz) */
#else
	TIMSK2 |= (1<<TOIE2);							/* Enable Timer2 overflow interrupt */
	TCCR2B |= (1<<CS22) | (1<<CS21) | (1<<CS20);	/* Set timer prescaller to 1024 (7812 Hz) */
#endif

	return;
}

ISR (TIMER2_OVF_vect)								/* 7812 / 256 = 30 polls/sec */
{

	if (scrollMode) {
		int8_t i;

		for (i = 0; i < MATRIX_NUMBER * 8 - 1; i++) {
			fb[i] = fb[i + 1];
		}
		fb[MATRIX_NUMBER * 8 - 1] = strBuf[scrollPos];
		matrixUpdate();

		scrollPos++;

		if (scrollPos >= _end + MATRIX_NUMBER * 8 - 1 || scrollPos >= MATRIX_BUFFER_SIZE) {
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
