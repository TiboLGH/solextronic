TARGET		= computation.exe
CC		= gcc
CFLAGS		= -I "lib" -MMD --std=gnu99 -O2 -Wall -g -D$(GIT_VER) -fPIC
LDFLAGS		= -MMD -fPIC
GIT_VER		:=VERSION_GIT=0x$(shell git rev-parse --short HEAD)

C_SRCS		= $(wilcard *.c)
OBJ_FILES	= computation.o lib/gnuplot_i.o lib/dictionary.o lib/iniparser.o

all:	$(TARGET)

%.o: %.c
	@$(CC) -c $(CFLAGS) $< -o $@
	@echo COMPUTATION $<

$(TARGET): $(OBJ_FILES)
	$(CC) $(LDFLAGS) $(OBJ_FILES) -o $@ 
	@echo COMPUTATION $@ done

.PHONY: clean

clean: 
	rm -f $(TARGET) *.o *.exe *.d

