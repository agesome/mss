MCU = atmega32
CC = avr-gcc $(CFLAGS) $(INCLUDES) $(DEFS)

CFLAGS = -O2 -mmcu=$(MCU) -Wl,-u,vfprintf -lprintf_flt -lm -Wall
INCLUDES = -I usbdrv/ -I.
FILES = usbdrv/*.c usbdrv/*.S *.c
DEFS = -DF_CPU=12000000UL -DOW_ONE_BUS

all: clean main.hex

main.hex: main.o main.elf main

main: $(main.elf)
	avr-objcopy -j .text -j .data -O ihex $@.elf $@.hex

main.o: $(FILES)
	$(CC) $(FILES) -c

main.elf: $(main)
	$(CC) -o $@ *.o
asm: $(FILES)
	$(CC) $(FILES) -S

clean:
	rm -f *.o main.elf main.hex

