#ifndef MATRIX_H
#define MATRIX_H

#include <inttypes.h>
#include "pins.h"

#define MATRIX_SCROLL_STOP			0
#define MATRIX_SCROLL_START			1

#define MATRIX_MIN_BRIGHTNESS		0
#define MATRIX_MAX_BRIGHTNESS		15

#define MATRIX_BUFFER_SIZE			384

enum {
	MATRIX_EFFECT_NONE = 0,
	MATRIX_EFFECT_SCROLL_DOWN,
	MATRIX_EFFECT_SCROLL_UP,
	MATRIX_EFFECT_SCROLL_BOTH,
};

void matrixInit(void);
void matrixScreenRotate(void);
void matrixSetBrightness(uint8_t brightness);
void matrixFill(uint8_t data);
void matrixPosData(uint8_t pos, uint8_t data);
void matrixSwitchBuf(uint32_t mask, uint8_t effect);
void matrixSetX(int16_t x);
void matrixLoadString(char *string);
void matrixSmallNumString(char *string);
void matrixBigNumString(char *string);
void matrixLoadStringEeprom(uint8_t *string);
void matrixLoadFont(const uint8_t *font);
void matrixScrollTimerInit(void);
void matrixHwScroll(uint8_t status);
uint8_t matrixGetScrollMode(void);

#endif /* MATRIX_H */
