LED_DRIVER = MAX7219

MCU = atmega8
F_CPU = 8000000L

TARG = matrixclock_$(MCU)_$(shell echo $(LED_DRIVER) | tr A-Z a-z)

SRCS = main.c
SRCS += mtimer.c i2csw.c rtc.c alarm.c eeprom.c
SRCS += ds18x20.c bmp180.c dht22.c

DEFINES = -D_$(MCU)

# Display source files
FONTS_SRC = $(wildcard font*.c)
ifeq ($(LED_DRIVER), HT1632)
  SRCS += ht1632.c
else
  SRCS +=max7219.c
endif
SRCS += display.c matrix.c $(FONTS_SRC)
DEFINES += -D_$(LED_DRIVER)

# Build directory
BUILDDIR = build

OPTIMIZE = -Os -mcall-prologues -fshort-enums -ffunction-sections -fdata-sections -ffreestanding -flto
WARNLEVEL = -Wall
CFLAGS = $(WARNLEVEL) -lm $(OPTIMIZE) -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CFLAGS += -MMD -MP -MT $(BUILDDIR)/$(*F).o -MF $(BUILDDIR)/$(*D)/$(*F).d
LDFLAGS = $(WARNLEVEL) -mmcu=$(MCU) -Wl,--gc-sections -Wl,--relax

# Main definitions

CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump

AVRDUDE = avrdude
AD_MCU = -p $(MCU)
#AD_PROG = -c stk500v2
#AD_PORT = -P avrdoper

AD_CMDLINE = $(AD_MCU) $(AD_PROG) $(AD_PORT) -V -B 0.5

SUBDIRS =

OBJS = $(addprefix $(BUILDDIR)/, $(SRCS:.c=.o))
ELF = $(BUILDDIR)/$(TARG).elf
HEX = flash/$(TARG).hex

all: $(HEX) size

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex -R .eeprom -R .nwram $(ELF) $(HEX)

$(ELF): $(OBJS)
	@mkdir -p $(addprefix $(BUILDDIR)/, $(SUBDIRS)) flash
	$(CC) $(LDFLAGS) -o $(ELF) $(OBJS)
	$(OBJDUMP) -h -S $(ELF) > $(BUILDDIR)/$(TARG).lss

size: $(ELF)
	@sh ./size.sh $(ELF)

$(BUILDDIR)/%.o: %.c Makefile
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEFINES) -c -o $@ $<

clean:
	rm -rf $(BUILDDIR)

.PHONY: flash
flash:  $(HEX)
	$(AVRDUDE) $(AD_CMDLINE) -U flash:w:flash/$(TARG).hex:i

fuse:
ifeq ($(MCU), atmega8)
	$(AVRDUDE) $(AD_CMDLINE) -U lfuse:w:0x24:m -U hfuse:w:0xd1:m
else
	$(AVRDUDE) $(AD_CMDLINE) -U lfuse:w:0xe2:m -U hfuse:w:0xd1:m -U efuse:w:0xfc:m
endif

eeprom_by:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:eeprom/matrixclock_by.bin:r

eeprom_en:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:eeprom/matrixclock_en.bin:r

eeprom_ru:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:eeprom/matrixclock_ru.bin:r

eeprom_ua:
	$(AVRDUDE) $(AD_CMDLINE) -U eeprom:w:eeprom/matrixclock_ua.bin:r

# Other dependencies
-include $(OBJS:.o=.d)
