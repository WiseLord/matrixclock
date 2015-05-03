#!/bin/sh

for LED_DRIVER in HT1632 MAX7219
do
	make clean
	make LED_DRIVER=${LED_DRIVER}
done

make clean
