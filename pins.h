#ifndef PINS_H
#define PINS_H

#include <avr/io.h>

#define CONCAT(x,y)			x ## y

#define DDR(x)				CONCAT(DDR,x)
#define PORT(x)				CONCAT(PORT,x)
#define PIN(x)				CONCAT(PIN,x)

/* 1-wire definitions */
#define ONE_WIRE			D
#define ONE_WIRE_LINE		(1<<6)

#endif /* PINS_H */
