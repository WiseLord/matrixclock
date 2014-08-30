#ifndef MAX7219_H
#define MAX7219_H

#include <avr/io.h>

#define MAX7219_DDR			DDRD
#define MAX7219_PORT		PORTD

#define MAX7219_LOAD1		(1<<PD4)
#define MAX7219_LOAD2		(1<<PD3)
#define MAX7219_LOAD3		(1<<PD2)
#define MAX7219_CLK			(1<<PD1)
#define MAX7219_DIN			(1<<PD0)

// Number of chips
#define MAX7219_ICNUMBER	3

// Registers
#define MAX7219_NO_OP		0x00
#define MAX7219_DIGIT_0		0x01
#define MAX7219_DIGIT_1		0x02
#define MAX7219_DIGIT_2		0x03
#define MAX7219_DIGIT_3		0x04
#define MAX7219_DIGIT_4		0x05
#define MAX7219_DIGIT_5		0x06
#define MAX7219_DIGIT_6		0x07
#define MAX7219_DIGIT_7		0x08
#define MAX7219_DEC_MODE	0x09
#define MAX7219_INTENSITY	0x0A
#define MAX7219_SCAN_LIMIT	0x0B
#define MAX7219_SHUTDOWN	0x0C
#define MAX7219_DISP_TEST	0x0F

enum {
	MAX7219_EFFECT_NONE = 0,
	MAX7219_EFFECT_SCROLL_DOWN = 1,
	MAX7219_EFFECT_SCROLL_UP = 2,
	MAX7219_EFFECT_SCROLL_BOTH = 3
};

void max7219Init(void);

void max7219SetBrightness(uint8_t brightness);

void max7219Fill(uint8_t data);

void max7219Send(uint8_t ic, uint8_t reg, uint8_t data);

void max7219Show(void);

void max7219LoadScr(uint8_t *buf);

void max7219PosData(uint8_t pos, uint8_t data);

void max7219SwitchBuf(uint32_t mask, uint8_t effect);

void max7219SetX(int16_t x);

void max7219LoadChar(uint8_t code);

void max7219LoadString(char *string);

void max7219LoadFont(const uint8_t *font);

void max7219Scroll(void);

#endif /* MAX7219_H */
