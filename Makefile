CC=gcc
CFLAGS=-g -pg
LIBS=-lz -lpng
DEFS=
AR=ar

LIBOBJ=lh_files.o lh_debug.o lh_compress.o lh_image.o

all: test libhelper.a

libhelper.a: $(LIBOBJ)
	$(AR) rcs $@ $^

test: main.o $(LIBOBJ)
	$(CC) -o $@ $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(DEFS) -o $@ -c $<

main.o : lh_buffers.h lh_files.h lh_debug.h lh_compress.h lh_image.h

clean:
	rm -f *.o *~

