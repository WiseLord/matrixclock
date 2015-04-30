#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

#define CONCAT(x,y)			x ## y

#define DDR(x)				CONCAT(DDR,x)
#define PORT(x)				CONCAT(PORT,x)
#define PIN(x)				CONCAT(PIN,x)


#define ONE_WIRE			D
#define ONE_WIRE_LINE		(1<<6)

#define MAX7219_LOAD		D
#define MAX7219_LOAD_LINE	(1<<4)
#define MAX7219_CLK			D
#define MAX7219_CLK_LINE	(1<<1)
#define MAX7219_DIN			D
#define MAX7219_DIN_LINE	(1<<0)

#endif /* PINS_H */
