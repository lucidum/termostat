PROGRAM = termostat
MCU = attiny26
CC = avr-gcc
OBJCOPY = avr-objcopy
CFLAGS += -Wall -g -Os -mmcu=$(MCU) -std=gnu11
LDFLAGS +=
OBJS = $(PROGRAM).o

all: $(PROGRAM).hex

$(PROGRAM).elf: termostat.o task.o adc.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(PROGRAM).hex: $(PROGRAM).elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $<

flash: $(PROGRAM).hex
	avrdude -c usbasp -p t26 -U flash:w:$(PROGRAM).hex

clean:
	rm -f $(OBJS)
	rm -f *.elf
	rm -f *.hex

