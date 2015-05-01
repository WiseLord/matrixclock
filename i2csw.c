#include "i2csw.h"

#include <util/delay.h>

static uint8_t I2CGetBit(void)
{
	uint8_t ret;

	_delay_us(5);
	DDR(I2C_SCL) &= ~I2C_SCL_LINE;			/* Pullup SCL = 1 */
	_delay_us(5);
	ret = PIN(I2C_SDA) & I2C_SDA_LINE;
	DDR(I2C_SCL) |= I2C_SCL_LINE;			/* Active SCL = 0 */

	return ret;
}

static void I2CSendBit(uint8_t bit)
{
	if (bit)
		DDR(I2C_SDA) &= ~I2C_SDA_LINE;		/* Pullup SDA = 1 */
	else
		DDR(I2C_SDA) |= I2C_SDA_LINE;		/* Active SDA = 0 */
	I2CGetBit();

	return;
}

void I2CStart(uint8_t addr)
{
	DDR(I2C_SCL) &= ~I2C_SCL_LINE;			/* Pullup SCL = 1 */
	DDR(I2C_SDA) &= ~I2C_SDA_LINE;			/* Pullup SDA = 1 */
	_delay_us(5);
	DDR(I2C_SDA) |= I2C_SDA_LINE;			/* Active SDA = 0 */
	_delay_us(5);
	DDR(I2C_SCL) |= I2C_SCL_LINE;			/* Active SCL = 0 */

	I2CWriteByte(addr);

	return;
}

void I2CStop(void)
{
	DDR(I2C_SCL) |= I2C_SCL_LINE;			/* Active SCL = 0 */
	DDR(I2C_SDA) |= I2C_SDA_LINE;			/* Active SDA = 0 */
	_delay_us(5);
	DDR(I2C_SCL) &= ~I2C_SCL_LINE;			/* Pullup SCL = 1 */
	_delay_us(5);
	DDR(I2C_SDA) &= ~I2C_SDA_LINE;			/* Pullup SDA = 1 */

	return;
}

uint8_t I2CWriteByte(uint8_t data)
{
	int8_t i = 0;

	for (i = 7; i >= 0; i--)
		I2CSendBit(data & (1<<i));
	I2CSendBit(I2C_ACK);

	return 0;
}

uint8_t I2CReadByte(uint8_t *data, uint8_t ack)
{
	int8_t i = 0;
	*data = 0;

	DDR(I2C_SDA) &= ~I2C_SDA_LINE;			/* Pullup SDA = 1 */
	for (i = 7; i >= 0; i--) {
		if (I2CGetBit())
			*data |= (1<<i);
	}
	I2CSendBit(!ack);

	return 0;
}
