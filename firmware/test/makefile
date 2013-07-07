TARGET		= simMain.exe
CC		= gcc
CFLAGS		= -I$(SIMAVR)/simavr/sim -MMD --std=gnu99 -O2 -Wall -g -D$(FOSSIL_VER) -fPIC
LIBDIR		= $(SIMAVR)/simavr/obj-i486-linux-gnu
LDFLAGS		= -MMD -fPIC -Wl,-rpath,$(LIBDIR) -L$(LIBDIR) -lsimavr -lelf -lutil -lpthread
FOSSIL_VER	:=VERSION_FOSSIL=0x$(shell /home/tibo/tools/fossil/fossil info | grep checkout | awk '{print $$2}' | head -c 8)

C_SRCS		= $(wilcard *.c)
OBJ_FILES	= simMain.o uart_pty.o pulse_input.o
SIMAVR		= /home/tibo/Projets/simavr

all:	$(TARGET)

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@
	@echo SIM $<

$(TARGET): $(OBJ_FILES)
	@$(CC) $(LDFLAGS) $(OBJ_FILES) -o $@ 
	@echo SIM $@ done

.PHONY: clean

clean: 
	rm -f $(TARGET) *.o *.hex *.d

