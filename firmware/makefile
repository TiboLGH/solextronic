MCU		= atmega328p
TARGET_ARCH	= -mmcu=$(MCU)
TARGET		= solextronic
CC		= avr-gcc
CPPFLAGS	= -mmcu=$(MCU)
CFLAGS		= -Os -g -Wall -I. -DF_CPU=16000000 -D$(GIT_VER) -std=c99
LDFLAGS		= -g -mmcu=$(MCU) -lm -Wl,--gc-sections -Os
PGMER		= -c arduino -b 57600 -P /dev/ttyUSB0
PGMERISP	= -c avrispv2 -P /dev/ttyUSB0
DUDE		= /usr/bin/avrdude -V -p $(MCU)
GIT_VER		:=VERSION_GIT=0x$(shell git rev-parse --short HEAD)

DEPS		= $(wildcard *.h)
C_SRCS		= $(wildcard *.c)
OBJ_FILES	= main.o platform.o command.o helper.o
OBJ_FILES2	= $(C_SRCS:.c=.o)

all:	$(TARGET).hex

%.o: %.c $(DEPS)
	@$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@
	@echo CC $<

$(TARGET).elf: $(OBJ_FILES)
	@$(CC) $(LDFLAGS) $(OBJ_FILES) -o $@ 
	@echo LD $<

$(TARGET).hex: $(TARGET).elf
	avr-objcopy -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex
#avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex $(TARGET).elf eeprom.hex

upload: $(TARGET).hex
	$(DUDE) $(PGMER) -U flash:w:$(TARGET).hex

size: $(TARGET).elf
	avr-size -C --mcu=$(MCU) $(TARGET).elf

sim: $(TARGET).hex
	make -C test/

test: $(TARGET).hex sim
	


.PHONY: tags clean sim

tags:
	ctags -R
	ctags -R -a /usr/lib/avr/include/
	ctags -R -a /home/tibo/Projets/simavr/simavr/sim/

clean: 
	rm -f $(TARGET).elf *.o *.hex *.d
	make -C test clean
