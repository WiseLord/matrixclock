#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

#define CONCAT(x,y)         x ## y

#define DDR(x)              CONCAT(DDR,x)
#define PORT(x)             CONCAT(PORT,x)
#define PIN(x)              CONCAT(PIN,x)

#define OUT(x)              (DDR(x) |= x ## _LINE)
#define IN(x)               (DDR(x) &= ~x ## _LINE)
#define SET(x)              (PORT(x) |= x ## _LINE)
#define CLR(x)              (PORT(x) &= ~x ## _LINE)
#define READ(x)             (PIN(x) & x ## _LINE)

#if !defined(_HT1632) && !defined(_MAX7219) && !defined(_MAX7219_X3)
#define _HT1632
#endif

#if defined(_HT1632)

#define ONE_WIRE            C
#define ONE_WIRE_LINE       (1<<1)

#define BEEPER              C
#define BEEPER_LINE         (1<<0)

#define BUTTON_1            D
#define BUTTON_1_LINE       (1<<7)
#define BUTTON_2            D
#define BUTTON_2_LINE       (1<<6)
#define BUTTON_3            D
#define BUTTON_3_LINE       (1<<5)

#define I2C_SCL             B
#define I2C_SCL_LINE        (1<<0)
#define I2C_SDA             B
#define I2C_SDA_LINE        (1<<2)

#define ADC_CHANNEL         7

#define DHT_DATA            D
#define DHT_DATA_LINE       (1<<3)

#else

#define ONE_WIRE            D
#define ONE_WIRE_LINE       (1<<6)

#define BEEPER              D
#define BEEPER_LINE         (1<<5)

#define BUTTON_1            B
#define BUTTON_2            B
#define BUTTON_3            B
#define BUTTON_1_LINE       (1<<3)
#define BUTTON_2_LINE       (1<<4)
#define BUTTON_3_LINE       (1<<5)

#define I2C_SCL             C
#define I2C_SCL_LINE        (1<<5)
#define I2C_SDA             C
#define I2C_SDA_LINE        (1<<4)

#define ADC_CHANNEL         0

#define DHT_DATA            D
#define DHT_DATA_LINE       (1<<3)

#endif

#define HT1632_CS           B
#define HT1632_CS_LINE      (1<<3)
#define HT1632_WR           B
#define HT1632_WR_LINE      (1<<4)
#define HT1632_DATA         B
#define HT1632_DATA_LINE    (1<<5)

#define MAX7219_LOAD        D
#define MAX7219_LOAD_LINE   (1<<4)
#define MAX7219_CLK         D
#define MAX7219_CLK_LINE    (1<<1)
#define MAX7219_DIN         D
#define MAX7219_DIN_LINE    (1<<0)

#endif // PINS_H
