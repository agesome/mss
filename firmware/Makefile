MCU = atmega324p
CC = avr-gcc $(CFLAGS) $(INCLUDES) $(DEFS)
OBJCOPY = avr-objcopy -j .text -j .data -O ihex

CFLAGS = -O3 -mmcu=$(MCU) -Wl,-u,vfprintf -lprintf_flt -lm \
-Wall -pedantic -std=c99
INCLUDES = -Iusbdrv/ -I.
FILES = crc8.c ds18x20.c humidity.c lcd.c main.c onewire.c temperature.c \
usbdrv/oddebug.c usbdrv/usbdrv.c usbdrv/usbdrvasm.S
DEFS = -DF_CPU=20000000UL -DOW_ONE_BUS

all: clean main.hex

main.hex: main.o main.elf main

main: $(main.elf)
	@echo OBJCOPY $@.elf $@.hex
	@$(OBJCOPY) $@.elf $@.hex

main.o: $(FILES)
	@echo CC $(FILES) -c
	@$(CC) $(FILES) -c

main.elf: $(main)
	@echo "CC $@"
	@$(CC) -o $@ *.o

asm: $(FILES)
	@$(CC) $(FILES) -S

clean:
	@rm -f *.o main.elf main.hex
