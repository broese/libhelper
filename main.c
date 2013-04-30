#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>


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

#if 0
    ARRAY(face,f,nf);
    ARRAY_ALLOCG(f, nf, 3, 4);

    f[0].a = 1; f[0].b = 2; f[0].c = 3;
    f[1].a = 11; f[1].b = 12; f[1].c = 13;EVFILE_READ_GRAN
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

#if 0
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

#if 0
#define TEST_SORTF(a,b) { char A=a, B=b; printf("A=%d B=%d SORTF=%d\n",a,b,SORTF_SI(&A,&B,(void *)0x01000000)); }

    TEST_SORTF(2,3);
    TEST_SORTF(2,2);
    TEST_SORTF(2,1);
#endif


#if __linux
    printf("-------------------\n");
    char *cstr = "ABQRSTUJKLVWFGCDEMNOPHIXYZ";
    ARRAY(char,ss,len);
    ARRAY_ALLOCG(ss,len,26,16);

    memcpy(ss,cstr,26);
    printf("ss=%8p len=%d >%s<\n",ss,len,ss);
    hexdump(ss, 32);

    SORTD(ss,26,*ss);
    printf("ss=%8p len=%d >%s<\n",ss,len,ss);
    hexdump(ss, 32);

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
    printf("Testing Multiarrays\n");

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
    off_t size;
    FILE *m = open_file_r("Makefile",&size);
    unsigned char * data = read_fp_at(m,34,55);
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
    char * idata = read_file_whole("test.txt", &ilen);
#endif
    

    
    ssize_t olen;
    unsigned char * comp = gzip_encode(idata, ilen, &olen);
    if (!comp) {
        printf("test_compression failed\n");
        exit(1);
    }

    printf("Compressed idata to %zd bytes\n",olen);

    hexdump(comp,olen);
    write_file("compressed.gz", comp, olen);

    ssize_t dlen;
    unsigned char * plain = gzip_decode(comp, olen, &dlen);
    if (!plain) {
        printf("test_decompression failed\n");
        exit(1);
    }

    printf("Decompressed to %zd bytes\n",dlen);
    hexdump(plain,dlen);


    free(comp);
}

void test_compression2() {
    ssize_t gsize;
    unsigned char * gzip = read_file_whole("test.gz",&gsize);
    hexdump(gzip,gsize);

    ssize_t psize;
    unsigned char * plain = zlib_decode(gzip, gsize, &psize);
    if (!plain) {
        printf("test_decompression failed\n");
        exit(1);
    }
    printf("Decompressed to %zd bytes\n",psize);
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

void test_image_resize() {
    lhimage *img = import_png_file("photo.png");
    printf("Imported a %dx%d image\n",img->width,img->height);
    int i;
    for(i=0; i<100; i++) {
        resize_image(img,img->width+50,img->height+50,20,20,0x00ff00ff);
    }
    ssize_t osize = export_png_file(img, "photo_resized.png");
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

void test_wstream() {
    char buf[4096];
    char *p = buf;

    write_char(p,0x11);
    write_short(p,0x2233);
    write_int(p,0x44556677);
    write_long(p,0x8899AABBCCDDEEFFLL);

    hexdump(buf, p-buf);
}

void test_server() {
    int ss = sock_server_ipv4_tcp_any(23456);
    if (ss >= 0) sleep(1000);
}

typedef struct {
    uint8_t * data;
    ssize_t len;
} buffer_t;

#if 1
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
    int maxcount = 100;
    while(stay && maxcount>0) {
        evfile_poll(&pa, 1000);
        for(i=0; i<cg.en; i++) {
            int idx = cg.e[i];
            FILE * file = pa.files[idx];
            printf("Error on client %d, fd=%d\n",idx, fileno(file));
        }

        // handle server requests
        for(i=0; i<sg.rn; i++) {
            struct sockaddr_in cadr;
            int size = sizeof(cadr);
            int cl = accept(pa.p[sg.r[i]].fd, (struct sockaddr *)&cadr, &size);
            if (cl < 0) printf("Failed to accept, %s\n",strerror(errno));
            printf("Accepted from %s:%d\n",inet_ntoa(cadr.sin_addr),ntohs(cadr.sin_port));

            if (fcntl(cl, F_SETFL, O_NONBLOCK) < 0) printf("Failed to set non-block, %s\n",strerror(errno));

            FILE * csock = fdopen(cl, "r+");
            if (!csock) printf("Failed to fdopen, %s\n",strerror(errno));
            fprintf(csock, "Welcome to the leet server\n");
            fflush(csock);

            ALLOC(buffer_t,buf);

            pollarray_add_file(&pa, &cg, csock, MODE_R, buf);
        }

        // handle client requests
        for(i=0; i<cg.rn; i++) {
            int idx = cg.r[i];
            FILE * fd = pa.files[idx];
            buffer_t * b = pa.data[idx];
            //uint8_t buf[256];
            //ssize_t len = 0;

            //int res = evfile_read_once(fileno(fd),buf,sizeof(buf),&len);
            int res = evfile_read(fileno(fd),&b->data,&b->len,128);
            printf("evfile_read returned %d, len=%zd fd=%d\n",res,b->len,fileno(fd));
            hexdump(b->data,b->len);

            int kill=0;
            switch (res) {
            case EVFILE_OK:
                printf("Have read as much as possible, buffer is full, processing %zd bytes\n",b->len);
                break;
            case EVFILE_WAIT:
                printf("No more input data for now, processing %zd bytes\n",b->len);
                break;
            case EVFILE_EOF:
                printf("End of file, removing client %d and processing %zd bytes\n",fileno(fd),b->len);
                kill=1;
                break;
            case EVFILE_ERROR:
                printf("Error occured, removing client %d\n",fileno(fd));
                kill=1;
                break;
            }

            if (kill) {
                buffer_t * b = pa.data[idx];
                printf("buffer : %8p\n",b);
                if (b->data) free(b->data);
                free(b);
                pollarray_remove_file(&pa, fd);
                fclose(fd);
            }

        }

        if (cg.en > 0) {
            printf("EV error\n");
            exit(1);
        }

#if 0
        for(i=0; i<cg.en; i++) {
            exit(1);
            printf("%s:%d\n",__func__,__LINE__);
            int idx = cg.e[i];
            FILE * fd = pa.files[idx];
            buffer_t * b = pa.data[idx];
            printf("%s:%d file=%08p fd=%d\n",__func__,__LINE__,fd,fileno(fd));
            if (b->data) free(b->data);
            //free(b);
            fclose(fd);
            printf("%s:%d\n",__func__,__LINE__);
            printf("Closed FILE %d\n",pa.p[cg.e[i]].fd);
            pollarray_remove_file(&pa, fd);
            //i--;
            printf("%s:%d\n",__func__,__LINE__);
            //exit(1);
        }
#endif
    }
}
#endif

#include <time.h>

void benchmark_allocation(int narrays, int gran) {
    srand(time(NULL));

    ALLOCN(uint8_t *,ptr,narrays);
    ALLOCN(int,cnt,narrays);
    uint8_t * oldptr;

    int n=0;

    int i,j;
    for(i=0; i<100000; i++) {
        for (j=0; j<narrays; j++) {
            oldptr = ptr[j];
            int inc = rand()&0xff;
            ARRAY_ADDG(ptr[j],cnt[j],inc,gran);
            if (ptr[j] != oldptr) n++;
        }
    }

#if 0
    ARRAY(uint8_t,ptr,cnt);
    uint8_t *oldptr;

    int i,n=0;
    for(i=1; i<1000000000; i++) {
        oldptr = ptr;
        ARRAY_EXTEND(ptr,cnt,i);
        if (ptr != oldptr) {
            printf("%i => %08x\n",i,ptr);
            n++;
        }
    }
#endif

    printf("Total: %d\n",n);
}

int main(int ac, char **av) {

    //test_pp();
    //test_clear();
    //test_buffers();
    //test_multiarrays();
    //test_files();
    test_compression();
    //test_image2();
    //test_image_resize();
    //test_stream();
    //test_wstream();

    //test_server();
    //test_event();

    //printf("%s %s\n",av[1],av[2]);
    //benchmark_allocation(atoi(av[1]),atoi(av[2]));

    return 0;
}

