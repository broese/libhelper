CFLAGS=-g -pg -std=gnu99
DEFS=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
CONFIG=-include config.h
INC=
LIBS=-lpng

LIBSRC=lh_debug.c lh_files.c lh_net.c lh_compress.c lh_dir.c lh_event.c lh_image.c
LIBHDR=config.h lh_arr.h lh_buffers.h lh_bytes.h lh_compress.h lh_debug.h lh_dir.h lh_event.h lh_files.h lh_image.h lh_marr.h lh_net.h lh_strings.h
LIBOBJ=$(LIBSRC:.c=.o)

TSTSRC=lhtest.c
TSTHDR=
TSTOBJ=$(TSTSRC:.c=.o)
TSTBIN=lhtest
TSTDIR=testdata

DEPFILE=make.depend

UNAME := $(shell uname -s)
ifeq ($(UNAME),SunOS)
        INC  += -I/users/atm/broese/include
	LIBS += -lsocket -lnsl -lmd5 -L/users/atm/broese/lib -lz
endif
ifeq ($(UNAME),Linux)
	LIBS += -lcrypto -lz
	CC=clang
endif




all: libhelper.a

libhelper.a: $(LIBOBJ)
	$(AR) rcs $@ $(LIBOBJ)

$(TSTBIN): $(TSTOBJ) libhelper.a
	$(CC) -o $@ $^ $(LIBS)

test: $(TSTBIN)
	./$(TSTBIN) $(TSTDIR)

.c.o: $(DEPFILE)
	$(CC) $(CFLAGS) $(DEFS) $(INC) $(CONFIG) -o $@ -c $<

$(DEPFILE): $(LIBSRC) $(LIBHDR) $(TSTSRC) $(TSTHDR)
	@rm -rf $(DEPFILE) $(DEPFILE).bak
	@touch $(DEPFILE)
	makedepend -Y -f $(DEPFILE) $(LIBSRC) $(TSTSRC) 2> /dev/null

doc:
	doxygen

clean:
	rm -f *.o *~ *.a $(TSTBIN)

FORCE:

sinclude $(DEPFILE)
