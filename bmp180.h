#ifndef BMP180_H
#define BMP180_H

#include <inttypes.h>

#define BMP180_ADDR					0xEE

/* EEPROM calibration addresses */
#define BMP180_CAL_DATA_START		0xAA

/* Oversampling settings*/
#define BMP180_OSS					0

/* BMP180 registers */
#define BMP180_CTRL_REG				0xF4
#define BMP180_TEMP_MEASURE			0x2E
#define BMP180_PRESSURE_MEASURE		(0x34 | (BMP180_OSS << 6))

/* Conversion parameters */
#define BMP180_CONV_REG				0xF6
#define BMP085_CONV_TIME			5

#define BMP180_CAL_LEN				11

typedef union {
	struct {
		int16_t ac1;
		int16_t ac2;
		int16_t ac3;
		uint16_t ac4;
		uint16_t ac5;
		uint16_t ac6;
		int16_t b1;
		int16_t b2;
		int16_t mb;
		int16_t mc;
		int16_t md;
	};
	struct {
		int16_t arr[BMP180_CAL_LEN];
	};
} bmp180CalData;

void bmp180Init(void);
uint8_t bmp180HaveSensor(void);

void bmp180Convert(void);

int16_t bmp180GetTemp(void);
int16_t bmp180GetPressure(void);

#endif /* BMP180_H */
