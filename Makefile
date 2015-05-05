LED_DRIVER = MAX7219

MCU = atmega8

# Lowercase argument
lc = $(shell echo $1 | tr A-Z a-z)

TARG=matrixclock_$(call lc,$(LED_DRIVER))_$(MCU)

F_CPU = 8000000L

# Source files
SRCS = $(wildcard *.c)

# Compiler options
OPTIMIZE = -Os -mcall-prologues -fshort-enums -ffunction-sections -fdata-sections
DEBUG = -g -Wall -Werror
CFLAGS = $(DEBUG) -lm $(OPTIMIZE) -mmcu=$(MCU) -DF_CPU=$(F_CPU)
LDFLAGS = $(DEBUG) -mmcu=$(MCU) -Wl,-gc-sections

# AVR toolchain and flasher
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
	$(CC) $(CFLAGS) -D$(LED_DRIVER) -D$(MCU) -c -o $@ $<

clean:
	rm -rf $(OBJDIR)

flash: $(TARG)
	$(AVRDUDE) $(AD_CMDLINE) -U flash:w:flash/$(TARG).hex:i

eeprom_en:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:eeprom/matrixclock_en.bin:r

eeprom_ru:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:eeprom/matrixclock_ru.bin:r

fuse:
	$(AVRDUDE) $(AD_CMDLINE) -U lfuse:w:0x24:m -U hfuse:w:0xd1:m
