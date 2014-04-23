CC=gcc -std=gnu99
CFLAGS=-g -pg
DEFS=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -DDEBUG_MEMORY=1
CONFIG=-include config.h
AR=ar

LIBS=-lz -lpng
UNAME := $(shell uname -s)
ifeq ($(UNAME),SunOS)
	LIBS += -lsocket -lnsl -lmd5
endif
ifeq ($(UNAME),Linux)
	LIBS += -lcrypto
endif

#LIBOBJ=lh_image.o lh_compress.o 
LIBOBJ=lh_debug.o lh_files.o lh_net.o lh_dir.o lh_event.o lh_compress.o

all: test libhelper.a

libhelper.a: $(LIBOBJ)
	$(AR) rcs $@ $^

test2: test2.o
	$(CC) -o $@ $^

test: main.o $(LIBOBJ)
	$(CC) -o $@ $^ $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(DEFS) $(CONFIG) -o $@ -c $<

main.o : lh_buffers.h lh_files.h lh_debug.h lh_compress.h lh_image.h lh_event.h lh_dir.h config.h

clean:
	rm -f *.o *~ *.a test test2 mtrace

doc:
	doxygen

FORCE:

mtrace: FORCE
	MALLOC_TRACE=mtrace ./test
	mtrace mtrace | perl -e 'while (<>) { if (/^(0x\S+)\s+(0x\S+)\s+at\s+(0x\S+)/) { print "$$1 $$2 at ".`addr2line -e test $$3`; } }'

