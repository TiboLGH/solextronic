DEVICE		= 12f675
TARGET		= testboard
AS		    = gpasm
ASFLAGS		= -c -p p$(DEVICE) -w 0
LD			= gplink
LDFLAGS     = -c -m -s /usr/share/gputils/lkr/$(DEVICE).lkr
PROG		= /usr/local/bin/usb_pickit

all:	$(TARGET).hex

%.o: %.asm
	@$(AS) $(ASFLAGS) $<
	@echo AS $<

$(TARGET).hex: testboard.o
	@$(LD) $(LDFLAGS) testboard.o -o $(TARGET).hex 
	@echo LD $<

upload: $(TARGET).hex
	$(PROG) -p $(TARGET).hex


.PHONY: clean

clean: 
	rm -f *.lst *.o *.hex *.map *.cod *.coff *.cof
