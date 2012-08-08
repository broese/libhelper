#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "lh_buffers.h"
#include "lh_files.h"
#include "lh_compress.h"
#include "lh_image.h"
#include "lh_net.h"

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

void test_buffers() {
#if 0
    ALLOC(model,m);
    ARRAY_ALLOCG(vertex,m->v,m->nv,100,ALLOC_GRAN);
    ARRAY_ALLOCG(face,m->f,m->nf,100,ALLOC_GRAN);
    ARRAY_ADDG(vertex,m->v,m->nv,20,ALLOC_GRAN);
#endif

#if 0
    int i;
    for(i=0; i<20; i++)
        printf("%3d %3d %3d\n",i,GRANREST(i,ALLOC_GRAN),GRANSIZE(i,ALLOC_GRAN));
#endif

#if 0
    int *a, n;
    //ARRAY_ALLOCG(int,a,n,0,ALLOC_GRAN);

    while(1) {
        int input;
        printf("Enter next number: ");
        if (scanf("%u",&input)==1) {
            ARRAY_ADDG(int,a,n,1,ALLOC_GRAN);
            a[n-1] = input;
        }
        else {
            printf("Input finished, the resulting array contains %d elements\n",n);
            int i;
            for(i=0; i<n; i++)
                printf(" %3d : %d\n",i,a[i]);
            break;
        }
    }
#endif

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
}

void test_server() {
    int ss = sock_server_ipv4_tcp_any(23456);
    if (ss >= 0) sleep(1000);
}

int main(int ac, char **av) {

    //test_buffers();
    //test_files();
    //test_compression();
    //test_image2();
    //test_stream();

    test_server();

    return 0;
}

