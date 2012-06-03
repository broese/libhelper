CC=gcc
CFLAGS=-g -pg
LIBS=
DEFS=

all: test

test: main.o
	$(CC) $(LIBS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) $(DEFS) -o $@ -c $<

main.o : buffers.h

clean:
	rm -f *.o *~

