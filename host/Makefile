CC = gcc $(CFLAGS) $(LIBS) $(INCLUDES) $(DEFS)
CFLAGS = -O0 -lm -lusb -pipe -g
LIBS = -lncurses
INCLUDES = -I.
FILES =  *.c
DEFS = 
LANG="en_US.UTF-8"

htc: htc.o

htc.o: $(FILES)
	$(CC) $(FILES) -c

htc: $(htc)
	$(CC) -o $@ *.o

clean:
	rm -f *.o htc

