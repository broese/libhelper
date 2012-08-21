#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "lh_buffers.h"
#include "lh_files.h"
#include "lh_compress.h"
#include "lh_image.h"
#include "lh_net.h"
#include "lh_event.h"

typedef struct {
    float x, y, z;
    float nx, ny, nz;
    float tx, ty;
} vertex;

typedef struct {
    int a,b,c;
} face;

typedef struct {
    vertex * v;
    int      nv;
    face   * f;
    int      nf;
} model;

#define ALLOC_GRAN 4
#define BUF_GRAN   64

////////////////////////////////////////////////////////////////////////////////

#define TESTGRAN(num) printf("num=%d gran=%d rest=%d size=%d\n", \
                             num,16,GRANREST(num,16),GRANSIZE(num,16));
void test_pp() {
    printf("Testing generic PP macros\n");

    printf("VA_LENGTH\n");
    printf("%d\n",VA_LENGTH(a,b,c));
    printf("%d\n",VA_LENGTH(a,b,c,d,e));
    printf("%d\n",VA_LENGTH(a));
    printf("%d\n",VA_LENGTH());

    printf("GRAN\n");
    TESTGRAN(0);
    TESTGRAN(1);
    TESTGRAN(13);
    TESTGRAN(15);
    TESTGRAN(16);
    TESTGRAN(17);
    TESTGRAN(18);
    TESTGRAN(31);
    TESTGRAN(32);
    TESTGRAN(1000);
}

void test_clear() {
    printf("Testing clear macros\n");
    char str[] = "ABCDEF";
    char *s = strdup("GHIJKL");

    printf("CLEAR\n");
    hexdump(str,sizeof(str));
    CLEAR(str);
    hexdump(str,sizeof(str));

    printf("CLEARP\n");
    hexdump(s,6);
    CLEARP(s);
    hexdump(s,6);

    printf("CLEARN\n");
    CLEARN(s,4);
    hexdump(s,6);

    free(s);

    s = strdup("MNOPQRSTUVWXYZ");
    printf("CLEAR_RANGE\n");
    hexdump(s,14);
    CLEAR_RANGE(s,4,5);
    hexdump(s,14);
}


void test_buffers() {
    printf("Testing buffers\n");

#if 0

    unsigned char *str, buffer[4096];
    int n;

    BUFFER_ALLOCG(str,n,1,BUF_GRAN);
    str[0] = 0;

    while(1) {
        printf("Enter next string: ");
        fgets(buffer,sizeof(buffer),stdin);
        if (!strncmp(buffer,"END",3)) break;

        int pos = n-1;
        int len = strlen(buffer);
        BUFFER_ADDG(str,n,len,BUF_GRAN);
        memcpy(str+pos,buffer,len);
        str[n-1]=0;
    }

    printf("Total length: %d\n%s\n",n-1,str);
#endif

#if 1
    ARRAY(face,f,nf);
    ARRAY_ALLOCG(f, nf, 3, 4);

    f[0].a = 1; f[0].b = 2; f[0].c = 3;
    f[1].a = 11; f[1].b = 12; f[1].c = 13;
    f[2].a = 21; f[2].b = 22; f[2].c = 23;

    printf("f=%08p nf=%d\n",f,nf);
    hexdump((unsigned char *)f, nf*sizeof(*f));

    ARRAY_ADDG(f, nf, 1, 4);
    f[3].a = 31; f[3].b = 32; f[3].c = 33;

    printf("f=%08p nf=%d\n",f,nf);
    hexdump((unsigned char *)f, nf*sizeof(*f));

    ARRAY_EXTENDG(f, nf, 2, 4);

    printf("f=%08p nf=%d\n",f,nf);
    hexdump((unsigned char *)f, nf*sizeof(*f));

    ARRAY_EXTENDG(f, nf, 4, 4);

    printf("f=%08p nf=%d\n",f,nf);
    hexdump((unsigned char *)f, nf*sizeof(*f));

    ARRAY_ADDG(f, nf, 13, 4);

    printf("f=%08p nf=%d\n",f,nf);
    hexdump((unsigned char *)f, nf*sizeof(*f));

    ARRAY_EXTENDG(f, nf, 3, 4);

    printf("f=%08p nf=%d\n",f,nf);
    hexdump((unsigned char *)f, nf*sizeof(*f));

#endif

#if 1
    char *cstr = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    ARRAY(char,s,len);
    ARRAY_ALLOCG(s,len,26,16);

    memcpy(s,cstr,26);
    printf("s=%08p len=%d >%s<\n",s,len,s);
    hexdump(s, 32);

    ARRAY_DELETE(s,len,3);
    printf("s=%08p len=%d >%s<\n",s,len,s);
    hexdump(s, 32);

    ARRAY_DELETE_RANGE(s,len,13,5);
    printf("s=%08p len=%d >%s<\n",s,len,s);
    hexdump(s, 32);

    ARRAY_DELETE(s,len,19);
    printf("s=%08p len=%d >%s<\n",s,len,s);
    hexdump(s, 32);

    ARRAY_DELETE_RANGE(s,len,17,2);
    printf("s=%08p len=%d >%s<\n",s,len,s);
    hexdump(s, 32);

#endif
}

typedef struct {
    int num;
    int * dongs;
    face * wangs;
    char ** herps;
    void ** derps;
} harbl;

#define TESTHERPS(name) hexdump((char*)name,h.num*sizeof(*name)); printf("\n")

void test_multiarrays() {
    harbl h;
    CLEAR(h);

    ARRAYS_ADD(h.num,3,h.dongs,h.wangs,h.herps,h.derps);
    hexdump((char *)&h,sizeof(h));
    TESTHERPS(h.dongs);
    TESTHERPS(h.wangs);
    TESTHERPS(h.herps);
    TESTHERPS(h.derps);

    ARRAYS_ADD(h.num,7,h.dongs,h.wangs,h.herps,h.derps);
    hexdump((char *)&h,sizeof(h));
    TESTHERPS(h.dongs);
    TESTHERPS(h.wangs);
    TESTHERPS(h.herps);
    TESTHERPS(h.derps);
}

void test_files() {
    ssize_t size;
    FILE *m = open_file_r("Makefile",&size);
    unsigned char * data = read_froma(m,34,55);
    hexdump(data+3, 55);
    free(data);
}

void test_compression() {

#if 0
    ssize_t ilen;
    char * idata = 
        "This is a test string to be compressed. "
        "We will try compression methods gzip and zlib and store the output to a file, "
        "then load the data again and try to decode it.";
    ilen = strlen(idata)+1;
#else
    ssize_t ilen;
    char * idata = read_file("test.txt", &ilen);
#endif
    

    
    ssize_t olen;
    unsigned char * comp = gzip_encode(idata, ilen, &olen);
    if (!comp) {
        printf("test_compression failed\n");
        exit(1);
    }

    printf("Compressed idata to %d bytes\n",olen);

    hexdump(comp,olen);
    write_file("compressed.gz", comp, olen);

    ssize_t dlen;
    unsigned char * plain = gzip_decode(comp, olen, &dlen);
    if (!plain) {
        printf("test_decompression failed\n");
        exit(1);
    }

    printf("Decompressed to %d bytes\n",dlen);
    hexdump(plain,dlen);


    free(comp);
}

void test_compression2() {
    ssize_t gsize;
    unsigned char * gzip = read_file("test.gz",&gsize);
    hexdump(gzip,gsize);

    ssize_t psize;
    unsigned char * plain = zlib_decode(gzip, gsize, &psize);
    if (!plain) {
        printf("test_decompression failed\n");
        exit(1);
    }
    printf("Decompressed to %d bytes\n",psize);
    hexdump(plain,psize);

    free(gzip);
}

void test_image() {
    lhimage *img = allocate_image(80,80);
    int i;
    for(i=0; i<img->height*img->width; i++)
        img->data[i] = 0x00ffff00;

    ssize_t osize = export_png_file(img, "test.png");
    printf("Exported size : %zd\n",osize);
}

void test_image2() {
    lhimage *img = import_png_file("input.png");
    printf("Imported a %dx%d image\n",img->width,img->height);

    int i;
    for(i=0; i<img->height*img->width; i++)
        img->data[i] ^= 0x00ffffff;

    ssize_t osize = export_png_file(img, "output.png");
    printf("Exported size : %zd\n",osize);
}

void test_stream() {

#if 0
    char *buf = "This is a test stream with some shit in it";
    char *p = buf;

    hexdump(buf, strlen(buf));

#if 0
    int8_t  a = parse_char(p);
    int16_t b = parse_short(p);
    int32_t c = parse_int(p);
    int64_t d = parse_long(p);
#else
    int8_t  a = read_char_le(p);
    int16_t b = read_short_le(p);
    int32_t c = read_int_le(p);
    int64_t d = read_long_le(p);
#endif

    printf("a=%d(%02x) b=%d(%04x) c=%d(%08x) d=%lld(%016llx) p=%08x buf=%08x\n",
           a,a,b,b,c,c,d,d,(int)p,(int)buf);
#endif

    union {
        double d;
        float f;
        uint8_t b[8];
    } n;

    n.f = 3.3;
    hexdump(n.b, 8);
    float f = parse_float(n.b);

    n.d = 3.3;
    hexdump(n.b, 8);
    double d = parse_double(n.b);

    char *s = "\xC0\xB1\x4B\x4F\x89\x69\x79\x9E";
    double x = parse_double(s);
    hexdump(s, 8);
    printf("%f %f %f\n",f,d,x);

}

void test_server() {
    int ss = sock_server_ipv4_tcp_any(23456);
    if (ss >= 0) sleep(1000);
}

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void test_event() {
    int ss = sock_server_ipv4_tcp_any(23456);
    if (ss<0) return;

    pollarray pa;
    CLEAR(pa);

    pollgroup sg;
    CLEAR(sg);

    pollgroup cg;
    CLEAR(cg);

    pollarray_add(&pa, &sg, ss, MODE_R, NULL);

    int stay = 1, i;
    while(stay) {
        evfile_poll(&pa, 100);

        #if 0
        // handle server requests
        for(i=0; i<sg.rnum; i++) {
            struct sockaddr_in cadr;
            int size = sizeof(cadr);
            int cl = accept(sg.rfds[i], (struct sockaddr *)&cadr, &size);
            if (cl < 0) printf("Failed to accept, %s\n",strerror(errno));
            printf("Accepted from %s:%d\n",inet_ntoa(cadr.sin_addr),ntohs(cadr.sin_port));

            FILE * csock = fdopen(cl, "r+");
            if (!csock) printf("Failed to fdopen, %s\n",strerror(errno));
            fprintf(csock, "Welcome to the leet server\n");

            pollarray_add_file(&pa, &cg, csock, MODE_RW, NULL);
            printf("Added successfully\n");
        }
        #endif

        #if 0
        // handle client requests
        for(i=0; i<cg.rnum; i++) {
            
        }
        #endif
    }
}

int main(int ac, char **av) {

    //test_pp();
    //test_clear();
    //test_buffers();
    test_multiarrays();
    //test_files();
    //test_compression();
    //test_image2();
    //test_stream();

    //test_server();
    //test_event();

    return 0;
}

