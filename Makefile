#avr-gcc -Wl,-u,vfprintf -lprintf_flt -lm -Os -mmcu=atmega32 -c *.c && avr-gcc -Wl,-u,vfprintf -lprintf_flt -lm -Os -mmcu=atmega32 -o main.elf *.o && avr-objcopy -j .text -j .data -O ihex main.elf main.hex && rm -f main.elf *.o
MCU = atmega32
CC = avr-gcc $(CFLAGS) $(INCLUDES) $(DEFS)

CFLAGS = -O2 -mmcu=$(MCU) -Wl,-u,vfprintf -lprintf_flt -lm -Wall
INCLUDES = -I usbdrv/ -I.
FILES = usbdrv/*.c usbdrv/*.S *.c
DEFS = -DF_CPU=12000000UL -DF_OSC=12000000UL -DXTAL=12000000UL -DOW_ONE_BUS

main.hex: main.o main.elf main

main: $(main.elf)
	avr-objcopy -j .text -j .data -O ihex $@.elf $@.hex

main.o: $(FILES)
	$(CC) $(FILES) -c

main.elf: $(main)
	$(CC) -o $@ *.o

clean:
	rm *.o main.elf

