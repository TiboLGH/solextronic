TARGET		= simMain.exe
CC		= gcc
CFLAGS		= -I$(SIMAVR)/simavr/sim -MMD --std=gnu99 -O2 -Wall -g -D$(GIT_VER) -fPIC
LIBDIR		= $(SIMAVR)/simavr/obj-i486-linux-gnu
LDFLAGS		= -MMD -fPIC -Wl,-rpath,$(LIBDIR) -L$(LIBDIR) -lsimavr -lelf -lutil -lpthread
GIT_VER		:=VERSION_GIT=0x$(shell git rev-parse --short HEAD)

DEPS		= ../varDef.h ../helper.h
C_SRCS		= $(wildcard *.c) ../helper.c
OBJ_FILES	= simMain.o uart_pty.o pulse_input.o analog_input.o timing_analyzer.o  HT_supply.o helper.o
SIMAVR		= /home/tibo/Projets/simavr

all:	$(TARGET)

%.o: %.c $(DEPS)
	@$(CC) -c $(CFLAGS) $< -o $@
	@echo SIM $<

helper.o: ../helper.c $(DEPS)
	@$(CC) -c $(CFLAGS) $< -o $@
	@echo SIM $<

$(TARGET): $(OBJ_FILES)
	@$(CC) $(LDFLAGS) $(OBJ_FILES) -o $@ 
	@echo SIM $@ done

.PHONY: clean

clean: 
	rm -f $(TARGET) *.o *.hex *.d

