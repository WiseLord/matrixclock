#!/bin/sh

cd src/

MAKEOPTS="-j4 -s"

for LED_DRIVER in HT1632 MAX7219 MAX7219_X3
do
	for MCU in atmega168p atmega328p atmega8
	do
		make clean
		make ${MAKEOPTS} LED_DRIVER=${LED_DRIVER} MCU=${MCU}
	done
done

make clean
