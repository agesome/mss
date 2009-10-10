VERSION = 0.1
CC = gcc $(CFLAGS) $(LIBS) $(INCLUDES) $(DEFS)
CFLAGS = -O2 -pipe -pedantic -Wall -Wextra -std=c99 `pkg-config --cflags gtk+-2.0 libglade-2.0`
LIBS = -lusb `pkg-config --libs gtk+-2.0 libglade-2.0`
INCLUDES = -I.
FILES =  src/*.c
DEFS = -D_XOPEN_SOURCE=600 -DVERSION=\"${VERSION}\"

all: clean ghtc

ghtc: htc.o

htc.o: $(FILES)
	$(CC) $(FILES) -c

ghtc: $(ghtc)
	$(CC) -o $@ *.o

clean:
	rm -f *.o ghtc