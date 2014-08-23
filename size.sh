#!/bin/bash

sizes=($(avr-size -A $1 | grep '^.text\|^.data\|^.bss' | sort | sed -e 's/.* \([0-9]\+\) .*/\1/'))

rm -f $1

echo
echo "AVR Memory Usage:"
echo
echo "Program:  $((${sizes[2]} + ${sizes[1]})) bytes (.text + .data)"
echo "Data:     $((${sizes[1]} + ${sizes[0]})) bytes (.data + .bss)"
echo
