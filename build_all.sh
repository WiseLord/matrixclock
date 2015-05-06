#!/bin/sh

for LED_DRIVER in HT1632 MAX7219
do
	for MCU in atmega168 atmega328 atmega8
	do
		make clean
		make LED_DRIVER=${LED_DRIVER} MCU=${MCU}
	done
done

make clean
