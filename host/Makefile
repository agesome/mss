CC = gcc $(CFLAGS) $(LIBS) $(INCLUDES) $(DEFS)
CFLAGS = -O0 -pipe -g
LIBS = -lncurses -lm -lusb
INCLUDES = -I.
FILES =  *.c
DEFS = -D_XOPEN_SOURCE=600
LANG="en_US.UTF-8"

all: clean htc

htc: htc.o

htc.o: $(FILES)
	$(CC) $(FILES) -c

htc: $(htc)
	$(CC) $(LIBS) -o $@ *.o

clean:
	rm -f *.o htc