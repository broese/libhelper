#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#if 0
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#endif

#ifdef HAVE_MTRACE
#if DEBUG_MEMORY
#include <mcheck.h>
#endif
#endif

#define LH_DECLARE_SHORT_NAMES 1

#include "lh_buffers.h"
#include "lh_bytes.h"

#if 0
#include "lh_files.h"
#include "lh_compress.h"
#include "lh_image.h"
#include "lh_net.h"
#include "lh_event.h"

#define ALLOC_GRAN 4
#define BUF_GRAN   64
#endif

typedef struct vertex {
    float x, y, z;
    float nx, ny, nz;
    float tx, ty;
} vertex;

typedef struct face {
    int a,b,c;
} face;

typedef struct model {
    vertex * v;
    int      nv;
    face   * f;
    int      nf;
} model;

////////////////////////////////////////////////////////////////////////////////

#define PASSFAIL(cond) ( (cond) ? "\x1b[32mPASS\x1b[0m" : "\x1b[31mFAIL\x1b[0m" )

#define TESTALIGN(n,a,r)                                        \
    printf("align %ju,%ju => %ju (%s)\n",                       \
           (uintmax_t)(n), (uintmax_t)a, (uintmax_t)ALIGN(n,a), \
           PASSFAIL(ALIGN(n,a)==(r))                            \
    );                                                          \
    if (ALIGN(n,a)!=(r)) fail++;


int test_align() {
    printf("\n\n====== Testing Alignment ======\n");
    int fail = 0;

    TESTALIGN(0,4,0);
    TESTALIGN(1,4,4);
    TESTALIGN(2,4,4);
    TESTALIGN(3,4,4);
    TESTALIGN(4,4,4);
    TESTALIGN(5,4,8);

    TESTALIGN(1000,16,1008);
    TESTALIGN(1024,256,1024);

    TESTALIGN(0x100000000LL,0x80000000,0x100000000LL);
    TESTALIGN(0x100000000LL,0x100000000LL,0x100000000LL);
    TESTALIGN(0x100000001LL,0x100000000LL,0x200000000LL);

    TESTALIGN(0xFFFFFFEDLL,0x1000,0x100000000LL);
    TESTALIGN(0xFFFFFFED,0x1000,0);

    uint64_t a = 0xFFFFFFEDLL;
    TESTALIGN(a,0x1000,0x100000000LL);
    uint32_t b = 0xFFFFFFEDLL;
    TESTALIGN(b,0x1000,0);

#if 0
    uint16_t c = 0xFFED;
    TESTALIGN(c,0x1000,0);
#endif

    TESTALIGN(0x100001234LL,0x1000,0x100002000LL);
    TESTALIGN(0x100001234LL,0x100,0x100001300LL);
    TESTALIGN(0x100001234LL,0x10,0x100001240LL);

#if 0
    TESTALIGN(123,100,200);
    TESTALIGN(12345,100,12400);
    TESTALIGN(1234567,100,1234600);
    TESTALIGN(12345678,100,12345700);
    TESTALIGN(123456789,100,123456800);
    TESTALIGN(123456789123456789LL,1000,123456789123457000LL);
#endif

    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}

#define TEST_CLEAR(name,ptr,n,neg) {                        \
        int i,f=0;                                          \
        for (i=0; i<n; i++)                                 \
            if (neg ? !*((ptr)+i) : *((ptr)+i)) {           \
                printf("%c %d\n",*((ptr)+i),neg);           \
                f++;                                        \
            }                                               \
        fail += f;                                          \
        printf( "%s : %s\n", name, PASSFAIL(!f));           \
    }


int test_clear() {
    printf("\n\n====== Testing clear macros ======\n");
    int fail = 0;

    char str[] = "ABCDEF";
    char *s1 = strdup("GHIJKL");
    char *s2 = strdup("MNOPQR");
    char *s3 = strdup("abcdefghijklmnop");

    CLEAR(str);
    TEST_CLEAR("CLEAR",str,6,0);
    CLEARP(s1);
    TEST_CLEAR("CLEARP",s1,1,0);
    CLEARN(s2,4);
    TEST_CLEAR("CLEARN",s2,4,0);
    TEST_CLEAR("CLEARN",s2+4,2,1);
    CLEAR_RANGE(s3,3,7);
    TEST_CLEAR("CLEAR_RANGE",s3,3,1);
    TEST_CLEAR("CLEAR_RANGE",s3+3,7,0);
    TEST_CLEAR("CLEAR_RANGE",s3+10,6,1);

    free(s1);
    free(s2);
    free(s3);

    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}

#define TEST_ALLOC(name, ptr, size) {                   \
        int i,f=0;                                      \
        uint8_t *p = (uint8_t *)ptr;                    \
        memset(p, 0, size);                             \
        for(i=0;i<size;i++)                             \
            f += (p[i]!=0);                             \
        memset(p, 0xAA, size);                          \
        for(i=0;i<size;i++)                             \
            f += (p[i]!=0xAA);                          \
        fail += f;                                      \
        printf("%s: %s\n", name, PASSFAIL(!f));         \
    }

int test_alloc() {
    printf("\n\n====== Testing allocation macros ======\n");
    int fail = 0;
    
    CREATE(vertex,v);
    TEST_ALLOC("CREATE",v,sizeof(vertex));
    CREATEN(face,fc,30);
    TEST_ALLOC("CREATEN",fc,sizeof(face)*30);
    CREATEB(buf,100);
    TEST_ALLOC("CREATEB",buf,100);

    free(v);
    free(fc);
    free(buf);

    ALLOC(v);
    TEST_ALLOC("ALLOC",v,sizeof(vertex));
    ALLOCN(fc,80);
    TEST_ALLOC("ALLOCN",fc,sizeof(face)*80);
    ALLOCB(buf,10000);
    TEST_ALLOC("ALLOCB",buf,10000);

    free(v);
    free(fc);
    free(buf);

    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}

#define TEST_ARRAY(name,ptr,cnt,check) {                \
        int sum=0,i,f=0;                                \
        for(i=0; i<cnt; i++) sum += (int)ptr[i];        \
        f = (sum != (check));                           \
        fail += f;                                      \
        printf("%s: %s\n", name, PASSFAIL(!f));         \
    }
        

int test_arrays() {
    printf("\n\n====== Testing expandable arrays ======\n");
    int fail = 0;

    int i,s;

    lh_buffer(buf,buflen);
    lh_array_allocate_g(buf,buflen,200,16);
    for(i=0; i<buflen; i++) buf[i] = (uint8_t)i;
    TEST_ARRAY("lh_array_allocate_g",buf,buflen,199*buflen/2);

    for(i=200; i<1000; i++) {
        lh_array_resize_g(buf,buflen,i+1,16);
        buf[i] = (uint8_t)i;
    }
    for(i=0,s=0; i<1000; i++) s+=(uint8_t)i;
    TEST_ARRAY("lh_array_add_g",buf,buflen,sum);

    lh_array_delete_element(buf,buflen,700);
    lh_array_delete_element(buf,buflen,500);
    lh_array_delete_element(buf,buflen,300);
    TEST_ARRAY("lh_array_delete_element",
               buf,buflen,s-(700%256)-(500%256)-(300%256));

    lh_array_delete_range(buf,buflen,100,5);
    TEST_ARRAY("lh_array_delete_element",
               buf,buflen,s-100-101-102-103-104-(700%256)-(500%256)-(300%256));
    
    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}

////////////////////////////////////////////////////////////////////////////////

#define TEST_BSWAP(func,a,b) {                      \
        int f=0;                                    \
        f = ( func(a) != b );                       \
        fail += f;                                  \
        printf("%s: %s\n", #func, PASSFAIL(!f));    \
    }

int test_bswap() {
    printf("\n\n====== Testing byteswap functions ======\n");
    int fail = 0;

    union {
        float vf;
        uint32_t vi;
    } F;

    float f1 = (float) M_PI;
    F.vf = f1;
    F.vi = bswap_int(F.vi);
    float f2 = F.vf;

    union {
        double vd;
        uint64_t vl;
    } D;

    double d1 = M_PI;
    D.vd = d1;
    D.vl = bswap_long(D.vl);
    double d2 = D.vd;

    TEST_BSWAP(bswap_short, 0x1234,0x3412);
    TEST_BSWAP(bswap_int,   0x12345678,0x78563412);
    TEST_BSWAP(bswap_long,  0x123456789ABCDEF0LL,0xF0DEBC9A78563412LL);
    TEST_BSWAP(bswap_float, f1,f2);
    TEST_BSWAP(bswap_double,d1,d2);

    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}

#define TEST_PARSE(type,val,check) {                            \
        f = (val != check);                                 \
        fail += f;                                              \
        printf("%s: %s\n", "lh_" #type, PASSFAIL(!f));    \
    }

#define Rx(type,var) lh_lread_ ##type##_be(ptr,lim,var, { atlimit=1; break; })
#define Rchar(var)  Rx(char,var)
#define Rshort(var) Rx(short,var)
#define Rint(var)   Rx(int,var)
#define Rlong(var)  Rx(long,var)

#define TEST_LREAD(len,rlen,v_atlimit,v_rc,v_rs,v_ri,v_rl)  \
    ptr = buf;                                          \
    lim = ptr+len;                                      \
    atlimit=0;                                          \
    do {                                                \
        Rchar(rc);                                      \
        Rshort(rs);                                     \
        Rint(ri);                                       \
        Rlong(rl);                                      \
        f = (rc!=v_rc) || (rs!=v_rs) ||                 \
            (ri!=v_ri) || (rl!=v_rl);                   \
    } while(0);                                         \
    f += (ptr != buf+rlen);                             \
    if (atlimit) f=0;                                   \
    f += (atlimit!=v_atlimit);                          \
    fail += f;                                          \
    printf("lh_lread len=%d %s\n", len, PASSFAIL(!f));



int test_stream() {
    printf("\n\n====== Testing bytestream reading ======\n");
    int fail = 0, f;

    uint8_t buf[64] = { 
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
        0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
    };

    uint8_t vc = lh_parse_char_be(buf);
    TEST_PARSE(parse_char_be,vc,0x12);

    uint16_t vs = lh_parse_short_be(buf);
    TEST_PARSE(parse_short_be,vs,0x1234);

    uint32_t vi = lh_parse_int_be(buf);
    TEST_PARSE(parse_int_be,vi,0x12345678);

    uint64_t vl = lh_parse_long_be(buf);
    TEST_PARSE(parse_long_be,vl,0x123456789ABCDEF0LL);

    ////////////////////////////////////////////////////////////////////////////

    uint8_t *ptr = buf;

    vc = lh_read_char_be(ptr);
    TEST_PARSE(read_char_be,vc,0x12);

    vs = lh_read_short_be(ptr);
    TEST_PARSE(read_short_be,vs,0x3456);

    vi = lh_read_int_be(ptr);
    TEST_PARSE(read_int_be,vi,0x789abcde);

    vl = lh_read_long_be(ptr);
    TEST_PARSE(read_long_be,vl,0xf0123456789abcdeLL);

    ////////////////////////////////////////////////////////////////////////////

    uint8_t *lim;
    int atlimit;

    TEST_LREAD(15,15,0,0x12,0x3456,0x789abcde,0xf0123456789abcdeLL);
    TEST_LREAD(14,15,1,0x12,0x3456,0x789abcde,0xf0123456789abcdeLL);
    TEST_LREAD(7,7,1,0x12,0x3456,0x789abcde,0xf0123456789abcdeLL);
    TEST_LREAD(30,15,0,0x12,0x3456,0x789abcde,0xf0123456789abcdeLL);

    

    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}

#define TEST_WSTREAM(func, buf1, buf2, len) {        \
        int f=0;                                     \
        f = (memcmp(buf1, buf2, len) != 0);          \
        fail += f;                                   \
        printf("%s: %s\n", #func, PASSFAIL(!f));     \
    }

int test_wstream() {
    printf("\n\n====== Testing bytestream writing ======\n");
    int fail = 0, f;

    char buf[512];
    CLEAR(buf);
    uint8_t *ptr;

    ptr = buf;
    lh_write_char_be(ptr,0x12);
    lh_write_short_be(ptr,0x3456);
    lh_write_short_le(ptr,0x789A);
    lh_write_int_be(ptr,0xABCDEF01);
    lh_write_int_le(ptr,0xABCDEF01);
    lh_write_long_be(ptr,0xfedcba9876543210LL);
    lh_write_long_le(ptr,0xfedcba9876543210LL);
    lh_write_float_be(ptr,(float)M_PI);
    lh_write_double_be(ptr,M_PI);
    lh_write_float_le(ptr,(float)M_E);
    lh_write_double_le(ptr,M_E);

    char *check =
        "\x12"
        "\x34\x56" "\x9a\x78"
        "\xab\xcd\xef\x01" "\x01\xef\xcd\xab"
        "\xfe\xdc\xba\x98\x76\x54\x32\x10"
        "\x10\x32\x54\x76\x98\xba\xdc\xfe"
        "\x40\x49\x0f\xdb"
        "\x40\x09\x21\xfb\x54\x44\x2d\x18"
        "\x54\xf8\x2d\x40"
        "\x69\x57\x14\x8b\x0a\xbf\x05\x40"
        ;

    TEST_WSTREAM(lh_write,buf,check,53);
    //hexdump(buf,64);

    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}

////////////////////////////////////////////////////////////////////////////////

int test_files() {
    printf("\n\n====== Testing bytestream writing ======\n");
    int fail = 0, f;

    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}

typedef struct {
    int cnt;
    char              * status;
    uint16_t          * flags;
    const char       ** name;
    double            * coord;
} ma;

int print_ma(ma *x) {
    int i;
    printf("cnt:%d status:%p flags:%p name:%p\n",
           x->cnt,x->status,x->flags,x->name);
    for(i=0; i<x->cnt; i++) {
        printf("%2d  %c %04x %s\n",i,x->status[i],x->flags[i],x->name[i]);
    }

    return 0;
}

int test_multiarrays() {
    printf("\n\n====== Testing multiarrays ======\n");
    int fail = 0, f;

    ma x; CLEAR(x);
    print_ma(&x);

    lh_multiarray_allocate(x.cnt, 3, MAF(x.status), MAF(x.flags), MAF(x.name));
    x.status[0]='A'; x.status[1]='B'; x.status[2]='C';
    x.flags[0]=0x000a;x.flags[1]=0x0b00;x.flags[2]=0xC000;
    x.name[0]="Alice";x.name[1]="Bob";x.name[2]="Charlie";
    print_ma(&x);

    lh_multiarray_resize(x.cnt, 17, MAF(x.status), MAF(x.flags), MAF(x.name));
    x.status[6]='G'; x.status[7]='H'; x.status[8]='I';
    x.flags[6]=0xaaaa;x.flags[7]=0x1111;x.flags[8]=0x5555;
    x.name[6]="George";x.name[7]="Henry";x.name[8]="Irene";
    print_ma(&x);

    lh_multiarray_resize(x.cnt, 8, MAF(x.status), MAF(x.flags), MAF(x.name));
    print_ma(&x);

    lh_multiarray_delete_range(x.cnt, 3, 2, MAF(x.status), MAF(x.flags), MAF(x.name));
    print_ma(&x);


    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}









#if 0
////////////////////////////////////////////////////////////////////////////////


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


#if 0
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
#endif


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

#define TESTDNS(n) printf("%s => %08x\n",n,dns_addr_ipv4(n))

void test_dns() {
    TESTDNS("0.0.0.0");
    TESTDNS("127.0.0.1");
    TESTDNS("224.0.0.1");
    TESTDNS("255.255.255.252");
    TESTDNS("255.255.255.255");
    TESTDNS("localhost");
    TESTDNS("washuu.no-ip.biz");
    TESTDNS("washuu.homeip.net");
    TESTDNS("slashdot.org");
}

#endif

////////////////////////////////////////////////////////////////////////////////

int main(int ac, char **av) {
#ifdef HAVE_MTRACE
#if DEBUG_MEMORY
    mtrace();
#endif
#endif

    int fail = 0;

    /*
    //// lh_buffers.h
    fail += test_align();
    fail += test_clear();
    fail += test_alloc();
    fail += test_arrays();

    //// lh_bytes.h
    fail += test_bswap();
    fail += test_stream();
    fail += test_wstream();
    */



    fail += test_multiarrays();
    //test_files();
    //test_compression();
    //test_image2();
    //test_image_resize();

    //test_server();
    //test_event();

    //test_dns();

    //printf("%s %s\n",av[1],av[2]);
    //benchmark_allocation(atoi(av[1]),atoi(av[2]));

    printf("========== TOTAL: %s ==========\n", PASSFAIL(!fail));

#ifdef HAVE_MTRACE
#if DEBUG_MEMORY
    muntrace();
#endif
#endif
    return 0;
}

