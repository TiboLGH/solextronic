MCU		= atmega328p
TARGET_ARCH	= -mmcu=$(MCU)
TARGET		= solextronic
CC		= avr-gcc
CPPFLAGS	= -mmcu=$(MCU)
CFLAGS		= -Os -g -Wall -I. -DF_CPU=16000000 -D$(FOSSIL_VER)
LDFLAGS		= -g -mmcu=$(MCU) -lm -Wl,--gc-sections -Os
PGMER		= -c arduino -b 57600 -P /dev/ttyUSB0
PGMERISP	= -c avrispv2 -P /dev/ttyUSB0
DUDE		= /usr/bin/avrdude -V -p $(MCU)
FOSSIL_VER	:=VERSION_FOSSIL=0x$(shell /home/tibo/tools/fossil/fossil info | grep checkout | awk '{print $$2}' | head -c 8)

C_SRCS		= $(wilcard *.c)
OBJ_FILES	= main.o platform.o command.o
OBJ_FILES2	= $(C_SRCS:.c=.o)
SIMAVR		= /home/tibo/Projets/simavr

all:	$(TARGET).hex

%.o: %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

$(TARGET).elf: $(OBJ_FILES)
	$(CC) $(LDFLAGS) $(OBJ_FILES) -o $@ 

$(TARGET).hex: $(TARGET).elf
	avr-objcopy -j .text -j .data -O ihex $(TARGET).elf $(TARGET).hex
#avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O ihex $(TARGET).elf eeprom.hex

upload: $(TARGET).hex
	$(DUDE) $(PGMER) -U flash:w:$(TARGET).hex

size: $(TARGET).elf
	avr-size -C --mcu=$(MCU) $(TARGET).elf

sim: test/simMain.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) test/test.c -o test/test.o
	$(CC) $(LDFLAGS) test/test.o -o test/test.elf
	$(CC) -c $(CPPFLAGS) $(CFLAGS) test/atTest.c -o test/atTest.o
	$(CC) $(LDFLAGS) test/atTest.o -o test/atTest.elf
	gcc --std=gnu99 -MMD -fPIC -Wall -I$(SIMAVR)/simavr/sim -c test/simMain.c -o test/simMain.o
	gcc --std=gnu99 -MMD -fPIC -Wall -I$(SIMAVR)/simavr/sim -c test/pulse_input.c -o test/pulse_input.o
	gcc --std=gnu99 -MMD -fPIC -Wall -I$(SIMAVR)/simavr/sim -c test/uart_pty.c -o test/uart_pty.o
	gcc -MMD -fPIC -Wl,-rpath,$(SIMAVR)/simavr/obj-i486-linux-gnu -L$(SIMAVR)/simavr/obj-i486-linux-gnu -lpthread -lutil -lsimavr -lelf test/uart_pty.o test/pulse_input.o test/simMain.o -o test/simMain.exe



.PHONY: tags clean sim

tags:
	ctags -R
	ctags -R -a /usr/lib/avr/include/

clean: 
	rm -f $(TARGET).elf *.o *.hex

