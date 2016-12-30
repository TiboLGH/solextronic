TARGET		= simMain.exe
CC			= gcc
CFLAGS		= -I$(SIMAVR)/sim -MMD --std=gnu99 -O2 -Wall -g -D$(GIT_VER) -fPIC -DSIM_HOST
LIBDIR		= $(SIMAVR)/obj-x86_64-linux-gnu
LDFLAGS		= -MMD -fPIC -Wl,-rpath,$(LIBDIR) -L$(LIBDIR) -lsimavr -lelf -lutil -lpthread
GIT_VER		:=VERSION_GIT=0x$(shell git rev-parse --short HEAD)

DEPS		= ../varDef.h ../helper.h ../../model/model.h
C_SRCS		= $(wildcard *.c) ../helper.c ../../model/model.c
OBJ_FILES	= simMain.o uart_pty.o pulse_input.o analog_input.o timing_analyzer.o HT_supply.o sim_helpers.o trace_writer.o helper.o model.o push_button.o logger.o
SIMAVR		= /home/tibo/Projets/simavr/simavr

all:	$(TARGET)

%.o: %.c $(DEPS)
	@$(CC) -c $(CFLAGS) $< -o $@
	@echo SIM $<

helper.o: ../helper.c $(DEPS)
	@$(CC) -c $(CFLAGS) $< -o $@
	@echo SIM $<

model.o: ../../model/model.c $(DEPS)
	@$(CC) -c $(CFLAGS) $< -o $@
	@echo SIM $<

$(TARGET): $(OBJ_FILES)
	@$(CC) $(LDFLAGS) $(OBJ_FILES) -o $@ 
	@echo SIM $@ done

run: $(TARGET)
	LD_LIBRARY_PATH=$(LIBDIR):$(LD_LIBRARY_PATH) ./simMain.exe -w -v -t Starting ../solextronic.elf

.PHONY: clean

clean: 
	rm -f $(TARGET) *.o *.vcd *.d

