CFLAGS=-g -pg -std=gnu99
DEFS=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
CONFIG=-include config.h
INC=-I.
LIBS=-lpng

LIBSRCN=lh_debug lh_files lh_net lh_compress lh_dir lh_event lh_image
LIBSRC=$(addsuffix .c, $(LIBSRCN))
LIBHDRN=config lh_arr lh_buffers lh_bytes lh_compress lh_debug lh_dir lh_event lh_files lh_image lh_marr lh_net lh_strings
LIBHDR=$(addsuffix .h, $(LIBHDRN))
LIBOBJ=$(LIBSRC:.c=.o)

TSTSRCN=lhtest test_debug
TSTSRC=$(addprefix test/, $(addsuffix .c, $(TSTSRCN)))
TSTHDRN=lhtest
TSTHDR=$(addprefix test/, $(addsuffix .h, $(TSTHDRN)))
TSTOBJ=$(TSTSRC:.c=.o)
TSTBIN=lhtest
TSTDIR=test

DEPFILE=make.depend

UNAME := $(shell uname -s)
ifeq ($(UNAME),SunOS)
	INC  += -I~/include
	LIBS += -lsocket -lnsl -lmd5 -L~/lib -lz
endif
ifeq ($(UNAME),Linux)
	LIBS += -lcrypto -lz
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
	rm -f *.o test/*.o *~ *.a $(TSTBIN)

FORCE:

sinclude $(DEPFILE)
