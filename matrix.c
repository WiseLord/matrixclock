#include "matrix.h"

#include "fonts.h"
#include "ht1632.h"
#include "max7219.h"
#include "eeprom.h"
#include "mtimer.h"

#include "pins.h"

#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>

static uint8_t rotate = 0;
static uint8_t scrollInterval = 0;

static int16_t _col;						/* Current position */

static uint8_t fb[MATRIX_NUMBER * 8];
static uint8_t fbNew[MATRIX_BUFFER_SIZE];

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
		font = (uint8_t*)EEPROM_BIG_NUM_FONT;
		width = MATRIX_BIGNUM_WIDTH;
		memType = MATRIX_FONT_EEPROM;
	} else if (numSize == NUM_EXTRA) {
		font = (uint8_t*)EEPROM_EXTRA_NUM_FONT;
		width = MATRIX_EXTRANUM_WIDTH;
		memType = MATRIX_FONT_EEPROM;
	} else {
		chOft = code - ' ';
		/* TODO: Remove it with full font */
		if (code > 128)
			chOft -= 0x20;
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
			fbNew[_col++] = data;
	}
	fbNew[_col++] = 0x00;

	return;
}

void matrixInit(void)
{
	matrixInitDriver();

	matrixFill(0x00);
	rotate = eeprom_read_byte((uint8_t*)EEPROM_SCREEN_ROTATE);
	scrollInterval = eeprom_read_byte((uint8_t*)EEPROM_SCROLL_INTERVAL);
	setScrollTimer (scrollInterval);

	return;
}

void matrixSetBrightness(uint8_t brightness)
{
	if (scrollMode == MATRIX_SCROLL_OFF) {
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
	eeprom_update_byte((uint8_t*)EEPROM_SCREEN_ROTATE, rotate);

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
		fbNew[ptr++] = 0x00;

	return;
}

void matrixPlaceBuf(uint8_t bufType, uint8_t pos, uint8_t byte)
{
	if (bufType)
		fb[pos] = byte;
	else
		fbNew[pos] = byte;

	return;
}

void matrixSwitchBuf(uint32_t mask, int8_t effect)
{
	uint8_t i, j;
	uint8_t rsBit;
	uint8_t lsBit;

	rsBit = 0x80;
	lsBit = 0x01;
	for (i = 0; i < 8; i++) {
		for (j = 0; j < MATRIX_NUMBER * 8; j++) {
			if (mask & (1UL<<(MATRIX_NUMBER * 8 - 1 - j))) {
				switch (effect) {
				case MATRIX_EFFECT_SCROLL_DOWN:
					fb[j] <<= 1;
					if (fbNew[j] & rsBit)
						fb[j] |= 0x01;
					break;
				case MATRIX_EFFECT_SCROLL_UP:
					fb[j] >>= 1;
					if (fbNew[j] & lsBit)
						fb[j] |= 0x80;
					break;
				case MATRIX_EFFECT_SCROLL_BOTH:
					if (j & 0x01) {
						fb[j] <<= 1;
						if (fbNew[j] & rsBit)
							fb[j] |= 0x01;
					} else {
						fb[j] >>= 1;
						if (fbNew[j] & lsBit)
							fb[j] |= 0x80;
					}
					break;
				default:
					fb[j] = fbNew[j];
					break;
				}
			}
		}
		_delay_ms(25);
		matrixUpdate(fb, rotate);
		rsBit >>= 1;
		lsBit <<= 1;
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
	TCCR2 = (1<<CS22) | (1<<CS21) | (1<<CS20);
#else
	TIMSK2 = (1<<TOIE2);
	TCCR2B |= (1<<CS22) | (1<<CS21) | (1<<CS20);
#endif

	/* Adjust result to left, use VCC as Vref and set ADC channel */
	ADMUX = (1<<ADLAR) | (0<<REFS1) | (1<<REFS0) | ADC_CHANNEL;
	/* Enable ADC */
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
		fb[MATRIX_NUMBER * 8 - 1] = fbNew[scrollPos];
		matrixUpdate(fb, rotate);

		scrollPos++;

		if (scrollPos >= _col + MATRIX_NUMBER * 8 - 1 || scrollPos >= MATRIX_BUFFER_SIZE) {
			scrollMode = MATRIX_SCROLL_OFF;
			scrollPos = 0;
			setScrollTimer (scrollInterval);
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
	setScrollTimer (scrollInterval);

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
