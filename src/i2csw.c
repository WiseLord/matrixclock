#include "i2csw.h"

#include <util/delay.h>
#include "pins.h"

static uint8_t I2CswGetBit(void)
{
    uint8_t ret;

    _delay_us(5);
    IN(I2C_SCL);        // Pullup SCL = 1
    _delay_us(5);
    ret = READ(I2C_SDA);
    OUT(I2C_SCL);       // Active SCL = 0

    return ret;
}

static void I2CswSendBit(uint8_t bit)
{
    if (bit)
        IN(I2C_SDA);    // Pullup SDA = 1
    else
        OUT(I2C_SDA);   // Active SDA = 0

    I2CswGetBit();
}

void I2CswStart(uint8_t addr)
{
    IN(I2C_SCL);        // Pullup SCL = 1
    IN(I2C_SDA);        // Pullup SDA = 1
    _delay_us(5);
    OUT(I2C_SDA);       // Active SDA = 0
    _delay_us(5);
    OUT(I2C_SCL);       // Active SCL = 0

    I2CswWriteByte(addr);
}

void I2CswStop(void)
{
    OUT(I2C_SCL);       // Active SCL = 0
    OUT(I2C_SDA);       // Active SDA = 0
    _delay_us(5);
    IN(I2C_SCL);        // Pullup SCL = 1
    _delay_us(5);
    IN(I2C_SDA);        // Pullup SDA = 1
}

void I2CswWriteByte(uint8_t data)
{
    uint8_t i = 0;

    for (i = 0; i < 8; i++) {
        I2CswSendBit(data & 0x80);
        data <<= 1;
    }
    I2CswSendBit(I2C_ACK);
}

uint8_t I2CswReadByte(uint8_t ack)
{
    uint8_t i, ret;

    IN(I2C_SDA);        // Pullup SDA = 1

    ret = 0;
    for (i = 0; i < 8; i++) {
        ret <<= 1;
        if (I2CswGetBit())
            ret |= 0x01;
    }
    I2CswSendBit(!ack);

    return ret;
}
