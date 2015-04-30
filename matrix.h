#ifndef MATRIX_H
#define MATRIX_H

#include <inttypes.h>
#include "pins.h"

#define EEPROM_SCREEN_ROTATE		((void*)0x04)

#define MATRIX_SCROLL_STOP			0
#define MATRIX_SCROLL_START			1

#define MAXTRIX_MIN_BRIGHTNESS		0
#define MATRIX_MAX_BRIGHTNESS		15

enum {
	MAX7219_EFFECT_NONE = 0,
	MAX7219_EFFECT_SCROLL_DOWN,
	MAX7219_EFFECT_SCROLL_UP,
	MAX7219_EFFECT_SCROLL_BOTH,
};

void matrixInit(void);
void matrixScreenRotate(void);
void matrixSetBrightness(uint8_t brightness);
void matrixFill(uint8_t data);
void matrixPosData(uint8_t pos, uint8_t data);
void matrixSwitchBuf(uint32_t mask, uint8_t effect);
void matrixSetX(int16_t x);
void matrixLoadString(char *string);
void matrixLoadNumString(char *string);
void matrixLoadStringPgm(const char *string);
void matrixLoadFont(const uint8_t *font);
void matrixScrollTimerInit(void);
void matrixHwScroll(uint8_t status);
uint8_t matrixGetScrollMode(void);

#endif /* MATRIX_H */