#include "max7219.h"

#include <avr/pgmspace.h>
#include "pins.h"

static void max7219SendByte(uint8_t data)
{
	uint8_t i;

	for(i = 0; i < 8; i++) {
		if (data & 0x80)
			PORT(MAX7219_DIN) |= MAX7219_DIN_LINE;
		else
			PORT(MAX7219_DIN) &= ~MAX7219_DIN_LINE;
		data <<= 1;

		PORT(MAX7219_CLK) &= ~MAX7219_CLK_LINE;
		asm("nop");
		asm("nop");
		PORT(MAX7219_CLK) |= MAX7219_CLK_LINE;
	}

	return;
}

void max7219SendCmd(uint8_t reg, uint8_t data)
{
	uint8_t j;

	PORT(MAX7219_LOAD) &= ~MAX7219_LOAD_LINE;
	for (j = 0; j < MATRIX_CNT; j++) {
		max7219SendByte(reg);
		max7219SendByte(data);
	}
	PORT(MAX7219_LOAD) |= MAX7219_LOAD_LINE;

	return;
}

void max7219SendDataBuf(uint8_t *buf)
{
	uint8_t i, j;

	for (i = 0; i < 8; i++) {
		PORT(MAX7219_LOAD) &= ~MAX7219_LOAD_LINE;
		for (j = 0; j < MATRIX_CNT; j++) {
			max7219SendByte(MAX7219_DIGIT_0 + i);
			max7219SendByte(buf[8 * j + i]);
		}
		PORT(MAX7219_LOAD) |= MAX7219_LOAD_LINE;
	}

	return;
}

void max7219Init(void)
{
	DDR(MAX7219_LOAD) |= MAX7219_LOAD_LINE;
	DDR(MAX7219_DIN) |= MAX7219_DIN_LINE;
	DDR(MAX7219_CLK) |= MAX7219_CLK_LINE;

	PORT(MAX7219_LOAD) &= ~MAX7219_LOAD_LINE;
	PORT(MAX7219_DIN) &= ~MAX7219_DIN_LINE;
	PORT(MAX7219_CLK) &= ~MAX7219_CLK_LINE;

	max7219SendCmd(MAX7219_SHUTDOWN, 1);		// Power on
	max7219SendCmd(MAX7219_DISP_TEST, 0);		// Test mode off
	max7219SendCmd(MAX7219_DEC_MODE, 0);		// Use led matrix
	max7219SendCmd(MAX7219_SCAN_LIMIT, 7);		// Scan all 8 digits (cols)

	return;
}
