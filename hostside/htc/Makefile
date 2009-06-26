VERSION = 0.5
CC = gcc $(CFLAGS) $(LIBS) $(INCLUDES) $(DEFS)
CFLAGS = -O2 -pipe -Wall -pedantic -std=c99
LIBS = -lncurses -lm -lusb
INCLUDES = -I.
FILES =  *.c
DEFS = -D_XOPEN_SOURCE=600 -DVERSION=\"${VERSION}\"
LANG="en_US.UTF-8"

all: clean htc

htc: htc.o

htc.o: $(FILES)
	$(CC) $(FILES) -c

htc: $(htc)
	$(CC) -o $@ *.o

clean:
	rm -f *.o htc