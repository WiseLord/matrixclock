TARG=max7219lib

SRCS = main.c max7219.c font-ks0066-ru-08.c mtimer.c
MCU = atmega8
F_CPU = 8000000L

CS = -fexec-charset=ks0066-ru

OPTIMIZE = -Os -mcall-prologues
CFLAGS = -g -Wall -Werror -lm $(OPTIMIZE) $(CS) -mmcu=$(MCU) -DF_CPU=$(F_CPU)
LDFLAGS = -g -Wall -Werror -mmcu=$(MCU)
OBJS = $(SRCS:.c=.o)

CC = avr-gcc
OBJCOPY = avr-objcopy

AVRDUDE = avrdude
AD_MCU = -p $(MCU)
#AD_PROG = -c avr109
#AD_PORT = -P /dev/ttyACM0
#AD_PROG = -c usbasp
#AD_PORT = -P usbasp

AD_CMDLINE = $(AD_MCU) $(AD_PROG) $(AD_PORT)

all: $(TARG)

$(TARG): $(OBJS)
	$(CC) $(LDFLAGS) -o $@.elf  $(OBJS) -lm
	$(OBJCOPY) -O ihex -R .eeprom -R .nwram  $@.elf $@.hex
	./size.sh $@.elf

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARG).elf $(TARG).bin $(TARG).hex $(OBJS) *.map

flash: $(TARG)
	$(AVRDUDE) $(AD_CMDLINE) -V -B 1.1 -U flash:w:$(TARG).hex:i
