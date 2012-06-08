CC=gcc
CFLAGS=-g -pg
LIBS=
DEFS=

all: test

test: main.o lh_files.o lh_debug.o
	$(CC) $(LIBS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) $(DEFS) -o $@ -c $<

main.o : lh_buffers.h lh_files.h lh_debug.h

clean:
	rm -f *.o *~

