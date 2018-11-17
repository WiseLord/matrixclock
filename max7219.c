#include "max7219.h"

#include <avr/pgmspace.h>
#include "pins.h"

static void max7219SendByte(uint8_t data)
{
    uint8_t i;

    for (i = 0; i < 8; i++) {
        if (data & 0x80)
            SET(MAX7219_DIN);
        else
            CLR(MAX7219_DIN);
        data <<= 1;

        CLR(MAX7219_CLK);
        asm("nop");
        asm("nop");
        SET(MAX7219_CLK);
    }
}

void max7219SendCmd(uint8_t reg, uint8_t data)
{
    uint8_t j;

    CLR(MAX7219_LOAD);
    for (j = 0; j < MATRIX_SMALL_CNT; j++) {
        max7219SendByte(reg);
        max7219SendByte(data);
    }
    SET(MAX7219_LOAD);
}

void max7219SendDataBuf(uint8_t *buf)
{
    int8_t i, j;

    for (i = 0; i < MAX7219_SCAN; i++) {
        CLR(MAX7219_LOAD);
        for (j = MATRIX_SMALL_CNT - 1; j >= 0; j--) {
            max7219SendByte(MAX7219_DIGIT_0 + i);
            max7219SendByte(buf[MAX7219_SCAN * j + i]);
        }
        SET(MAX7219_LOAD);
    }
}

void max7219Init(void)
{
    OUT(MAX7219_LOAD);
    OUT(MAX7219_DIN);
    OUT(MAX7219_CLK);

    CLR(MAX7219_LOAD);
    CLR(MAX7219_DIN);
    CLR(MAX7219_CLK);

    max7219SendCmd(MAX7219_SHUTDOWN, 1);    // Power on
    max7219SendCmd(MAX7219_DISP_TEST, 0);   // Test mode off
    max7219SendCmd(MAX7219_DEC_MODE, 0);    // Use led matrix
    max7219SendCmd(MAX7219_SCAN_LIMIT, MAX7219_SCAN - 1);  // Scan all 8 digits (cols)
}
