#ifndef MATRIX_H
#define MATRIX_H

#include <inttypes.h>

#define MATRIX_NUMBER				4

#define MATRIX_SCROLL_STOP			0
#define MATRIX_SCROLL_START			1

#define MATRIX_FONT_WIDTH			5
#define MATRIX_SMALLNUM_WIDTH		3
#define MATRIX_BIGNUM_WIDTH			5
#define MATRIX_EXTRANUM_WIDTH		6
#define MATRIX_BUFFER_SIZE			640

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
#define matrixInitDriver() ht1632Init()
#define matrixUpdate(x, y) ht1632SendDataBuf(x, y)
#else
#define matrixInitDriver() max7219Init()
#define matrixUpdate(x, y) max7219SendDataBuf(x, y)
#endif

enum {
	NUM_SMALL = 0,
	NUM_NORMAL,
	NUM_BIG,
	NUM_EXTRA,

	NUM_END,
};

enum {
	MATRIX_SCROLL_OFF = 0,
	MATRIX_SCROLL_ON,
};

enum {
	BUF_STRING = 0,
	BUF_FRAME,

	BUF_END
};

void matrixInit(void);

void matrixSetBrightness(uint8_t brightness);
void matrixScreenRotate(void);

void matrixFill(uint8_t data);
void matrixClearBufTail(void);

void matrixPlaceBuf(uint8_t bufType, uint8_t pos, uint8_t byte);

void matrixSwitchBuf(uint32_t mask, int8_t effect);

void matrixSetX(int16_t x);

void matrixScrollAndADCInit(void);
void matrixHwScroll(uint8_t status);
uint8_t matrixGetScrollMode(void);

void matrixLoadNumString(char *string, uint8_t numSize);
void matrixLoadString(char *string);
void matrixLoadStringEeprom(uint8_t *string);

#endif /* MATRIX_H */
