TARG=matrixclock

SRCS = main.c max7219.c font-ks0066-ru-08.c mtimer.c i2c.c ds1307.c ds18x20.c display.c alarm.c
MCU = atmega8
F_CPU = 8000000L

CS = -fexec-charset=ks0066-ru

OPTIMIZE = -Os -mcall-prologues -fshort-enums
DEBUG = -g -Wall -Werror
CFLAGS = $(DEBUG) -lm $(OPTIMIZE) -mmcu=$(MCU) -DF_CPU=$(F_CPU) $(CS)
LDFLAGS = $(DEBUG) -mmcu=$(MCU)

CC = avr-gcc
OBJCOPY = avr-objcopy

AVRDUDE = avrdude
AD_MCU = -p $(MCU)
#AD_PROG = -c stk500v2
#AD_PORT = -P avrdoper

AD_CMDLINE = $(AD_MCU) $(AD_PROG) $(AD_PORT) -V

OBJDIR = obj
OBJS = $(addprefix $(OBJDIR)/, $(SRCS:.c=.o))
ELF = $(OBJDIR)/$(TARG).elf

all: $(TARG)

$(TARG): $(OBJS)
	$(CC) $(LDFLAGS) -o $(ELF) $(OBJS) -lm
	mkdir -p flash
	$(OBJCOPY) -O ihex -R .eeprom -R .nwram $(ELF) flash/$@.hex
	./size.sh $(ELF)

obj/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(OBJDIR)

flash: $(TARG)
	$(AVRDUDE) $(AD_CMDLINE) -U flash:w:flash/$(TARG).hex:i

eeprom:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:$(TARG).eep:r

fuse:
	$(AVRDUDE) $(AD_CMDLINE) -U lfuse:w:0xe4:m -U hfuse:w:0xd1:m
