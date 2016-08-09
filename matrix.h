#ifndef MATRIX_H
#define MATRIX_H

#include <inttypes.h>

#define MATRIX_SCROLL_STOP			0
#define MATRIX_SCROLL_START			1

#define MATRIX_FONT_WIDTH			5
#define MATRIX_SMALLNUM_WIDTH		3
#define MATRIX_BIGNUM_WIDTH			5
#define MATRIX_EXTRANUM_WIDTH		6
#define MATRIX_BUFFER_SIZE			(MATRIX_CNT * 8)
#define MATRIX_STRING_LEN			250

enum {
	MATRIX_FONT_RAM = 0,
	MATRIX_FONT_PROGMEM,
	MATRIX_FONT_EEPROM,
};

enum {
	MATRIX_EFFECT_SCROLL_DOWN = -1,
	MATRIX_EFFECT_NONE = 0,
	MATRIX_EFFECT_SCROLL_UP = 1,
	MATRIX_EFFECT_SCROLL_BOTH = 2,
};

#if defined(HT1632)
#include "ht1632.h"
#define matrixInitDriver() ht1632Init()
#define matrixUpdate(buf) ht1632SendDataBuf(buf)
#else
#include "max7219.h"
#define matrixInitDriver() max7219Init()
#define matrixUpdate(buf) max7219SendDataBuf(buf)
#endif

enum {
	NUM_NORMAL = 0,
	NUM_BIG,
	NUM_SMALL,

	NUM_END,
};

enum {
	MATRIX_SCROLL_OFF = 0,
	MATRIX_SCROLL_ON,
};

extern uint8_t fb[MATRIX_BUFFER_SIZE];

void matrixInit(void);

void matrixSetBrightness(uint8_t brightness);

void matrixSwitchBuf(uint32_t mask, int8_t effect);

void matrixSetX(int16_t x);

void matrixScrollAndADCInit(void);
void matrixHwScroll(uint8_t status);
uint8_t matrixGetScrollMode(void);

void matrixScrollAddStringEeprom(uint8_t *string);
void matrixScrollAddString(char *string);

void matrixFbNewAddString(char *string, uint8_t numSize);
void matrixFbNewAddStringEeprom(uint8_t *string);

void matrixChangeRotate(int8_t diff);
void matrixWrite(void);

#endif // MATRIX_H
