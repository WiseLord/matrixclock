#include "i2csw.h"

#include <util/delay.h>

static uint8_t I2CswGetBit(void)
{
	uint8_t ret;

	_delay_us(5);
	DDR(I2C_SCL) &= ~I2C_SCL_LINE;			/* Pullup SCL = 1 */
	_delay_us(5);
	ret = PIN(I2C_SDA) & I2C_SDA_LINE;
	DDR(I2C_SCL) |= I2C_SCL_LINE;			/* Active SCL = 0 */

	return ret;
}

static void I2CswSendBit(uint8_t bit)
{
	if (bit)
		DDR(I2C_SDA) &= ~I2C_SDA_LINE;		/* Pullup SDA = 1 */
	else
		DDR(I2C_SDA) |= I2C_SDA_LINE;		/* Active SDA = 0 */
	I2CswGetBit();

	return;
}

void I2CswStart(uint8_t addr)
{
	DDR(I2C_SCL) &= ~I2C_SCL_LINE;			/* Pullup SCL = 1 */
	DDR(I2C_SDA) &= ~I2C_SDA_LINE;			/* Pullup SDA = 1 */
	_delay_us(5);
	DDR(I2C_SDA) |= I2C_SDA_LINE;			/* Active SDA = 0 */
	_delay_us(5);
	DDR(I2C_SCL) |= I2C_SCL_LINE;			/* Active SCL = 0 */

	I2CswWriteByte(addr);

	return;
}

void I2CswStop(void)
{
	DDR(I2C_SCL) |= I2C_SCL_LINE;			/* Active SCL = 0 */
	DDR(I2C_SDA) |= I2C_SDA_LINE;			/* Active SDA = 0 */
	_delay_us(5);
	DDR(I2C_SCL) &= ~I2C_SCL_LINE;			/* Pullup SCL = 1 */
	_delay_us(5);
	DDR(I2C_SDA) &= ~I2C_SDA_LINE;			/* Pullup SDA = 1 */

	return;
}

void I2CswWriteByte(uint8_t data)
{
	uint8_t i = 0;

	for (i = 0; i < 8; i++) {
		I2CswSendBit(data & 0x80);
		data <<= 1;
	}
	I2CswSendBit(I2C_ACK);

	return;
}

uint8_t I2CswReadByte(uint8_t ack)
{
	uint8_t i, ret;

	DDR(I2C_SDA) &= ~I2C_SDA_LINE;			/* Pullup SDA = 1 */

	ret = 0;
	for (i = 0; i < 8; i++) {
		ret <<= 1;
		if (I2CswGetBit())
			ret |= 0x01;
	}
	I2CswSendBit(!ack);

	return ret;
}
