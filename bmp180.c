#include <util/delay.h>

#include "bmp180.h"
#include "i2csw.h"

static bmp180CalData cd;

static int16_t temperature = 0;
static int16_t pressure = 0;

static uint8_t bmp180Sensor = 0;

static uint16_t bmp180ReadI2CWord(uint8_t addr)
{
	uint16_t ret;

	I2CswStart(BMP180_ADDR);
	I2CswWriteByte(addr);
	I2CswStart(BMP180_ADDR | I2C_READ);
	ret = ((uint16_t)I2CswReadByte(I2C_ACK)) << 8;
	ret |= I2CswReadByte(I2C_NOACK);
	I2CswStop();

	return ret;
}

void bmp180Init(void)
{
	uint8_t i;
	uint16_t param;

	for (i = 0; i < BMP180_CAL_LEN; i++) {
		param = bmp180ReadI2CWord(BMP180_CAL_DATA_START + 2 * i);
		if (param == 0x0000 || param == 0xFFFF) {
			bmp180Sensor = 0;										/* No valid sensor on bus */
			return;
		}
		cd.arr[i] = param;
	}

	bmp180Sensor = 1;

	return;
}

uint8_t bmp180HaveSensor(void)
{
	return bmp180Sensor;
}

uint16_t bmp180GetRawData(uint8_t param)
{
	uint16_t ret;

	I2CswStart(BMP180_ADDR);
	I2CswWriteByte(BMP180_CTRL_REG);
	I2CswWriteByte(param);
	I2CswStop();

	_delay_ms(BMP085_CONV_TIME);

	ret = bmp180ReadI2CWord(BMP180_CONV_REG);

	return ret;
}

void bmp180Convert (void)
{
	int32_t x1, x2, x3, b3, b5, b6, b8, p;
	uint32_t b4, b7;

	uint16_t ut = bmp180GetRawData(BMP180_TEMP_MEASURE);
	uint16_t up = bmp180GetRawData(BMP180_PRESSURE_MEASURE);

	x1 = ((int32_t)ut - cd.ac6) * cd.ac5 >> 15;
	x2 = ((int32_t)cd.mc << 11) / (x1 + cd.md);
	b5 = x1 + x2;

	temperature = (b5 + 8) >> 4;							/* Value in 0.1 °C */

	b6 = b5 - 4000;
	b8 = (b6 * b6) >> 12;
	x3 = (b8 * cd.b2 + b6 * cd.ac2) >> 11;
	b3 = (x3 +  4 * (int32_t)cd.ac1 + 2) >> 2;
	x1 = (b6 * cd.ac3) >> 13;
	x2 = (b8 * cd.b1) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	b4 = (cd.ac4 * (uint32_t)(x3 + 32768)) >> 15;
	b7 = (((uint32_t) up - b3) * (50000));

	p = (b7 / b4) << 1;

	x1 = (p >> 8) * (p >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * p) >> 16;

	pressure = (p + ((x1 + x2 + 3791) >> 4)) * 3 / 40;		/* Value in 10*mmHg */
//	pressure = (p + ((x1 + x2 + 3791) >> 4)) * 4 / 400;		/* Value in 10*kPa */

}

int16_t bmp180GetTemp(void)
{
	return temperature;
}

int16_t bmp180GetPressure(void)
{
	return pressure;
}
