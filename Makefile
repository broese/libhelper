CC=gcc
CFLAGS=-g -pg
LIBSSOL=-lsocket -lnsl
LIBS=-lz -lpng #$(LIBSSOL)
DEFS=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -DDEBUG_MEMORY=1
CONFIG=-include config.h
AR=ar

#LIBOBJ=lh_files.o lh_debug.o lh_compress.o lh_image.o lh_net.o lh_event.o
LIBOBJ=lh_debug.o

all: test libhelper.a

libhelper.a: $(LIBOBJ)
	$(AR) rcs $@ $^

test: main.o $(LIBOBJ)
	$(CC) -o $@ $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(DEFS) $(CONFIG) -o $@ -c $<

main.o : lh_buffers.h lh_files.h lh_debug.h lh_compress.h lh_image.h config.h

clean:
	rm -f *.o *~

doc:
	doxygen

FORCE:

mtrace: FORCE
	MALLOC_TRACE=mtrace ./test
	mtrace mtrace | perl -e 'while (<>) { if (/^(0x\S+)\s+(0x\S+)\s+at\s+(0x\S+)/) { print "$$1 $$2 at ".`addr2line -e test $$3`; } }'

