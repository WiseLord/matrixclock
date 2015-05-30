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
static int16_t _end;						/* End of string in buffer */

static uint8_t fb[MATRIX_NUMBER * 8];
static uint8_t strBuf[MATRIX_BUFFER_SIZE];

static volatile int16_t scrollPos = 0;
static volatile uint8_t scrollMode = 0;

static void matrixLoadChar(uint8_t code)
{
	uint8_t i;
	uint8_t pgmData;
	uint16_t oft;

	if (code > 128)
		oft = code - ' ' - 0x20;
	else
		oft = code - ' ';
	oft *= MATRIX_FONT_WIDTH;

	for (i = 0; i < MATRIX_FONT_WIDTH; i++) {
		pgmData = pgm_read_byte(font_cp1251_08 + oft + i);
		if (pgmData != 0x99)
			strBuf[_col++] = pgmData;
	}
	strBuf[_col++] = 0x00;

	return;
}

static void matrixLoadNumChar(uint8_t code, uint8_t memType, const uint8_t *font, uint8_t width)
{
	uint8_t i;
	uint8_t data;
	const uint8_t *oft;

	for (i = 0; i < width; i++) {
		if (code < '0' || code > '9') {
			data = 0x00;
		} else {
			oft = font + (code - 0x30) * width + i;
			if (memType == MATRIX_FONT_EEPROM)
				data = eeprom_read_byte(oft);
			else if (memType == MATRIX_FONT_PROGMEM)
				data = pgm_read_byte(oft);
			else
				data = *oft;
		}
		strBuf[_col++] = data;
	}
	strBuf[_col++] = 0x00;

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

void matrixClearBufTail(void)
{
	_end = _col;
	while(_col < MATRIX_BUFFER_SIZE)
		strBuf[_col++] = 0x00;
	_col = _end;

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

void matrixLoadNumString(char *string, uint8_t numType)
{
	switch (numType) {
	case NUM_SMALL:
		while(*string)
			matrixLoadNumChar(*string++, MATRIX_FONT_PROGMEM, font_smallnum, MATRIX_SMALLNUM_WIDTH);
		break;
	case NUM_BIG:
		while(*string)
			matrixLoadNumChar(*string++, MATRIX_FONT_EEPROM, EEPROM_BIG_NUM_FONT, MATRIX_BIGNUM_WIDTH);
		break;
	case NUM_EXTRA:
		while(*string)
			matrixLoadNumChar(*string++, MATRIX_FONT_EEPROM, EEPROM_EXTRA_NUM_FONT, MATRIX_EXTRANUM_WIDTH);
		break;
	default:
		matrixLoadString(string);
		break;
	}

	matrixClearBufTail();
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

	// Start ADC conversion to brightness from photoresistor
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
