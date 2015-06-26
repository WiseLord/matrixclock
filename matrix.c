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

static int16_t _col;						/* Current position */

static uint8_t fb[MATRIX_NUMBER * 8];
static uint8_t strBuf[MATRIX_BUFFER_SIZE];

static volatile int16_t scrollPos = 0;
static volatile uint8_t scrollMode = MATRIX_SCROLL_OFF;

static void matrixLoadChar(uint8_t numSize, uint8_t code)
{
	uint8_t i;
	uint8_t data;
	const uint8_t *oft;
	uint8_t chOft;

	const uint8_t *font = font_cp1251_08;
	uint8_t width = MATRIX_FONT_WIDTH;
	uint8_t memType = MATRIX_FONT_PROGMEM;

	chOft = code - '0';

	if (numSize == NUM_SMALL) {
		font = font_smallnum;
		width = MATRIX_SMALLNUM_WIDTH;
	} else if (numSize == NUM_BIG) {
		font = EEPROM_BIG_NUM_FONT;
		width = MATRIX_BIGNUM_WIDTH;
		memType = MATRIX_FONT_EEPROM;
	} else if (numSize == NUM_EXTRA) {
		font = EEPROM_EXTRA_NUM_FONT;
		width = MATRIX_EXTRANUM_WIDTH;
		memType = MATRIX_FONT_EEPROM;
	} else {
		chOft = code - ' ';
		/* TODO: Remove it with full font */
		if (code > 128)
			chOft -= 0x21;
	}

	for (i = 0; i < width; i++) {
		if (numSize != NUM_NORMAL && (code < '0' || code > '9')) {
			data = 0x00;
		} else {
			oft = font + chOft * width + i;
			if (memType == MATRIX_FONT_EEPROM)
				data = eeprom_read_byte(oft);
			else if (memType == MATRIX_FONT_PROGMEM)
				data = pgm_read_byte(oft);
			else
				data = *oft;
		}
		if (data != VOID)
			strBuf[_col++] = data;
	}
	strBuf[_col++] = 0x00;

	return;
}

void matrixInit(void)
{
	matrixInitDriver();

	matrixFill(0x00);
	rotate = eeprom_read_byte(EEPROM_SCREEN_ROTATE);

	return;
}

void matrixSetBrightness(uint8_t brightness)
{
#if defined(HT1632)
	ht1632SendCmd(HT1632_CMD_DUTY | brightness);
#else
	max7219SendCmd(MAX7219_INTENSITY, brightness);
#endif

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

	matrixUpdate(fb, rotate);

	return;
}

void matrixClearBufTail(void)
{
	int16_t ptr = _col;

	while(ptr < MATRIX_BUFFER_SIZE)
		strBuf[ptr++] = 0x00;

	return;
}

void matrixPosData(uint8_t pos, uint8_t data)
{
	fb[pos] = data;

	return;
}

void matrixSwitchBuf(uint32_t mask, int8_t effect)
{
	uint8_t i, j;

	for (i = 0; i < 8; i++) {
		for (j = 0; j < MATRIX_NUMBER * 8; j++) {
			if (mask & (1UL<<(MATRIX_NUMBER * 8 - 1 - j))) {
				switch (effect) {
				case MATRIX_EFFECT_SCROLL_DOWN:
					fb[j] <<= 1;
					if (strBuf[j] & (0x80 >> i))
						fb[j] |= 0x01;
					break;
				case MATRIX_EFFECT_SCROLL_UP:
					fb[j] >>= 1;
					if (strBuf[j] & (0x01 << i))
						fb[j] |= 0x80;
					break;
				case MATRIX_EFFECT_SCROLL_BOTH:
					if (j & 0x01) {
						fb[j] <<= 1;
						if (strBuf[j] & (0x80 >> i))
							fb[j] |= 0x01;
					} else {
						fb[j] >>= 1;
						if (strBuf[j] & (0x01 << i))
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
		matrixUpdate(fb, rotate);
	}

	return;
}

void matrixSetX(int16_t x)
{
	_col = x;

	return;
}

void matrixScrollAndADCInit(void)
{
	/* Enable Timer2 overflow interrupt and set prescaler to 1024 (7812 Hz) */
#if defined(atmega8)
	TIMSK |= (1<<TOIE2);
	TCCR2 |= (1<<CS22) | (1<<CS21) | (1<<CS20);
#else
	TIMSK2 |= (1<<TOIE2);
	TCCR2B |= (1<<CS22) | (1<<CS21) | (1<<CS20);
#endif

	ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); /* Set ADC prescaler to 128	*/
	ADMUX = (1<<ADLAR) | (0<<REFS1) | (1<<REFS0);	/* Adjust result to left, use VCC as Vref */
	ADMUX |= ADC_CHANNEL;							/* Set ADC channel */

	ADCSRA |= (1<<ADEN);
	return;
}

/* Interrupt will be executed 7812 / 256 = 30 times/sec */
ISR (TIMER2_OVF_vect)
{
	if (scrollMode == MATRIX_SCROLL_ON) {
		int8_t i;

		for (i = 0; i < MATRIX_NUMBER * 8 - 1; i++)
			fb[i] = fb[i + 1];
		fb[MATRIX_NUMBER * 8 - 1] = strBuf[scrollPos];
		matrixUpdate(fb, rotate);

		scrollPos++;

		if (scrollPos >= _col + MATRIX_NUMBER * 8 - 1 || scrollPos >= MATRIX_BUFFER_SIZE) {
			scrollMode = MATRIX_SCROLL_OFF;
			scrollPos = 0;
		}

	}

	// Start ADC conversion to get brightness from photoresistor
	ADCSRA |= 1<<ADSC;

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


void matrixLoadNumString(char *string, uint8_t numSize)
{
	while(*string)
		matrixLoadChar(numSize, *string++);

	matrixClearBufTail();
}

void matrixLoadString(char *string)
{
	matrixLoadNumString(string, NUM_NORMAL);

	return;
}

void matrixLoadStringEeprom(uint8_t *string)
{
	char ch;
	uint8_t i = 0;

	ch = eeprom_read_byte(&string[i++]);
	while(ch) {
		matrixLoadChar(NUM_NORMAL, ch);
		ch = eeprom_read_byte(&string[i++]);
	}

	matrixClearBufTail();

	return;
}
