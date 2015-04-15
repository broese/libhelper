CFLAGS=-g -pg -std=gnu99
DEFS=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
CONFIG=-include config.h
AR=ar

INC=
LIBS=-lpng

UNAME := $(shell uname -s)
ifeq ($(UNAME),SunOS)
        INC  += -I/users/atm/broese/include
	LIBS += -lsocket -lnsl -lmd5 -L/users/atm/broese/lib -lz
endif
ifeq ($(UNAME),Linux)
	LIBS += -lcrypto -lz
	CC=clang
endif

LIBOBJ=lh_debug.o lh_files.o lh_net.o lh_compress.o lh_dir.o lh_event.o lh_image.o

all: libhelper.a

libhelper.a: $(LIBOBJ)
	$(AR) rcs $@ $^

lhtest: lhtest.o libhelper.a
	$(CC) -o $@ $^ $(LIBS)

test: lhtest
	./lhtest testdata

.c.o:
	$(CC) $(CFLAGS) $(DEFS) $(INC) $(CONFIG) -o $@ -c $<

main.o: lh_buffers.h lh_arr.h lh_marr.h lh_strings.h lh_files.h lh_debug.h lh_compress.h lh_image.h lh_event.h lh_dir.h config.h

clean:
	rm -f *.o *~ *.a lhtest

doc:
	doxygen

FORCE:
