#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


#ifdef HAVE_MTRACE
#if DEBUG_MEMORY
#include <mcheck.h>
#endif
#endif

#define LH_DECLARE_SHORT_NAMES 1

#include "lh_buffers.h"
#include "lh_bytes.h"
#include "lh_debug.h"
#include "lh_files.h"
#include "lh_net.h"

#if 0
#include "lh_event.h"
#include "lh_dir.h"
#include "lh_compress.h"
#include "lh_image.h"
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

#define TEST_ARRAY(title,name,check) {                          \
        int sum=0,i,f=0;                                        \
        for(i=0; i<name##_cnt; i++) sum += (int)name##_ptr[i];  \
        f = (sum != (check));                                   \
        fail += f;                                              \
        printf("%s: %s\n", title, PASSFAIL(!f));                 \
    }
        

int test_arrays() {
    printf("\n\n====== Testing expandable arrays ======\n");
    int fail = 0;

    int i,s;

    _BUFi(buf);
    lh_arr_allocate(GAR(buf),200);

    for(i=0; i<buf_cnt; i++) buf_ptr[i] = (uint8_t)i;
    TEST_ARRAY("lh_arr_allocate",buf,199*buf_cnt/2);

    for(i=200; i<1000; i++) {
        arr_resize(GAR(buf),i+1);
        buf_ptr[i] = (uint8_t)i;
    }
    for(i=0,s=0; i<1000; i++) s+=(uint8_t)i;
    TEST_ARRAY("lh_arr_add",buf,s);

    arr_new(GAR(buf)) = 0x55;
    s+=0x55;
    TEST_ARRAY("lh_arr_new",buf,s);

    arr_insert(GAR(buf),900) = 0xAA;
    s+=0xAA;
    TEST_ARRAY("lh_arr_insert",buf,s);

    arr_delete(AR(buf),700);
    arr_delete(AR(buf),500);
    arr_delete(AR(buf),300);
    s -= (700%256)+(500%256)+(300%256);
    TEST_ARRAY("lh_array_delete",buf,s);

    arr_delrange(AR(buf),100,5);
    s -= 100+101+102+103+104;
    TEST_ARRAY("lh_array_delete_range", buf,s);

    lh_arr_free(AR(buf));
    
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

typedef struct {
    short x,y,z;
    float speed;
    uint64_t  cat;
} test_t;

int test_unpack() {
    printf("\n\n====== Testing unpack ======\n");
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

    test_t u;

    ssize_t sz = lh_unpack(buf,buf+sizeof(buf),"SSSfl",&u.x,&u.y,&u.z,&u.speed,&u.cat);
    printf("sz=%zd coord=%d,%d,%d speed=%.3f cat=%016lx\n",sz,u.x,u.y,u.z,u.speed,u.cat);
    
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

#ifdef HAVE_OPENSSL
#include <openssl/md5.h>
#else
#include <md5.h>
#endif

uint8_t pr_init[16] = { 0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,
                        0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,};
#define PRSIZE 1048576
#define PRNAME "testfile.dat"

int create_test_file() {
    lh_create_buf(seq,PRSIZE);
    int i;

    memcpy(seq,pr_init,16);
    for (i=16; i<PRSIZE; i+=16)
#ifdef HAVE_OPENSSL
        MD5(seq+i,16,seq+i-16);
#else
        md5_calc(seq+i,seq+i-16,16);
#endif

    
}

int test_files() {
    printf("\n\n====== Testing bytestream writing ======\n");
    int fail = 0, f;

    fail += create_test_file();

    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}

////////////////////////////////////////////////////////////////////////////////

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
        printf("%2d  %c %04x %s\n",i,x->status[i],x->flags[i],x->name[i]?x->name[i]:"<undef>");
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

    free(x.status);
    free(x.flags);
    free(x.name);
    free(x.coord);


    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}

////////////////////////////////////////////////////////////////////////////////

int test_bprintf() {
    printf("\n\n====== Testing bprintf ======\n");
    int fail = 0, f;

    _BUFi(test);

    int res;
    res = bprintf(AR(test),"Hello World! ptr=%p len=%zd (%s:%d)\n",AR(test),__func__,__LINE__);
    printf("res=%d len=%zd\n",res,test_cnt);
    hexdump(AR(test));
    res = bprintf(AR(test),"Hello World! ptr=%p len=%zd (%s:%d)\n",AR(test),__func__,__LINE__);
    printf("res=%d len=%zd\n",res,test_cnt);
    hexdump(AR(test));
    res = bprintf(AR(test),"Hello World! ptr=%p len=%zd (%s:%d)\n",AR(test),__func__,__LINE__);
    printf("res=%d len=%zd\n",res,test_cnt);
    hexdump(AR(test));

    test_cnt = LH_BUFPRINTF_GRAN;
    res = bprintf(AR(test),"Hello World! ptr=%p len=%zd (%s:%d)\n",AR(test),__func__,__LINE__);
    printf("res=%d len=%zd\n",res,test_cnt);
    hexdump(AR(test));

    test_cnt = 512;
    res = bprintf(AR(test),"Hello World! ptr=%p len=%zd (%s:%d)\n",AR(test),__func__,__LINE__);
    printf("res=%d len=%zd\n",res,test_cnt);
    hexdump(AR(test));
    
    lh_arr_free(AR(test));
    
    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}


#if 0
////////////////////////////////////////////////////////////////////////////////

typedef struct {
    uint8_t * rbuf;
    ssize_t   rlen;
    uint8_t * wbuf;
    ssize_t   wlen;
} client;

void process_requests(client *c, int finalize) {
    int linecount=0;
    int lastpos=-1;

    // find the location of the last newline in the buffer
    int i;
    for(i=0; i<c->rlen; i++) {
        if (c->rbuf[i] == 0x0a) {
            lastpos = i;
            linecount++; // count lines
        }
    }

    // delete all counted lines from the receive buffer
    lh_array_delete_range(c->rbuf, c->rlen, 0, lastpos+1);

    if (finalize) {
        c->rlen = 0; //delete everything
        linecount++;
    }

    // write response
    bprintf(c->wbuf, c->wlen, "Test Server: received %d lines\n",linecount);
    if (finalize) bprintf(c->wbuf, c->wlen, "*** Good Bye ***\n");
}

int test_event() {
    printf("\n\n====== Testing event framework ======\n");
    int fail = 0, f;

    int ss = lh_listen_tcp4_any(23456);
    if (ss<0) {
        printf("-----\ntotal: %s\n", PASSFAIL(0));
        return 1;
    }

    lh_pollarray_create(pa);
    lh_pollgroup_create(server,&pa);
    lh_pollgroup_create(clients,&pa);

    lh_poll_add(&server, ss, MODE_R, NULL);

    int stay = 1, i;
    int maxcount = 100;
    while(stay && maxcount>0) {
        lh_poll(&pa, 1000);
        int fd;
        FILE *fp;

        while ((fd=lh_poll_next_readable(&server,NULL,NULL))>0) {
            // accept new connection
            struct sockaddr_in cadr;
            int cl = lh_accept_tcp4(fd, &cadr);
            if (cl < 0) break;
            printf("Accepted from %s:%d\n",
                   inet_ntoa(cadr.sin_addr),ntohs(cadr.sin_port));

            // Create a FILE*
            FILE * clfp = fdopen(cl, "r+");
            if (!clfp) LH_ERROR(1,"Failed to fdopen");

            fprintf(clfp, "Welcome to the server!\n");
            fflush(clfp);

            // create a new client instance
            CREATE(client, c);
            lh_poll_add_fp(&clients, clfp, MODE_R, c);
        }

        client * c;
        while ((fd=lh_poll_next_readable(&clients,&fp,(void **)&c))>0) {
            int res = lh_poll_read(fd, &c->rbuf, &c->rlen, 65536);
            switch (res) {
            case LH_EVSTATUS_EOF:
                process_requests(c,1);
                break;
            case LH_EVSTATUS_OK:
            case LH_EVSTATUS_WAIT:
                process_requests(c,0);
                break;
            case LH_EVSTATUS_ERROR:
                fprintf(stderr,"error on socket %d, closing\n",fd);
                if (c->rbuf) free(c->rbuf);
                if (c->wbuf) free(c->wbuf);
                free(c);
                lh_poll_remove(&pa, fd);
                break;
            }

            // do we have data to send back to the client?
            if (c->wlen > 0) {
                fprintf(stderr,"Sending response to socket %d\n",fd);
                switch(lh_poll_write_once(fd, c->wbuf, &c->wlen)) {
                case LH_EVSTATUS_ERROR:
                    if (c->rbuf) free(c->rbuf);
                    if (c->wbuf) free(c->wbuf);
                    free(c);
                    lh_poll_remove(&pa, fd);
                    break;
                case LH_EVSTATUS_WAIT:
                    LH_DEBUG("FIXME: handle LH_EVSTATUS_WAIT\n");
                    // remaining data might be send later if we have another request
                    break;
                }
            }
        }
    }
    
    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}

ssize_t process_requests2(lh_conn *conn) {
    int linecount=0;
    int lastpos=-1;

    // find the location of the last newline in the buffer
    int i;
    for(i=0; i<conn->r.len; i++) {
        if (conn->r.ptr[i] == 0x0a) {
            lastpos = i;
            linecount++; // count lines
        }
    }

    // write response
    bprintf(conn->w.ptr, conn->w.len, "Test Server: received %d lines\n",linecount);

    switch(conn->status) {
    case LH_EVSTATUS_EOF:
        bprintf(conn->w.ptr, conn->w.len, "*** Good Bye ***\n");
        return -1;
    case LH_EVSTATUS_ERROR:
        return -1;
    }

    return lastpos+1;
}

int test_event2() {
    printf("\n\n====== Testing event framework (L3) ======\n");
    int fail = 0, f;

    int ss = lh_listen_tcp4_any(23456);
    if (ss<0) {
        printf("-----\ntotal: %s\n", PASSFAIL(0));
        return 1;
    }

    lh_pollarray_create(pa);
    lh_pollgroup_create(server,&pa);
    lh_pollgroup_create(clients,&pa);

    lh_poll_add(&server, ss, MODE_R, NULL);

    int stay = 1, i;
    int maxcount = 100;

    while(stay && --maxcount>0) {
        lh_poll(&pa, 1000);
        int fd;
        FILE *fp;

        while ((fd=lh_poll_next_readable(&server,NULL,NULL))>0) {
            // accept new connection
            struct sockaddr_in cadr;
            int cl = lh_accept_tcp4(fd, &cadr);
            if (cl < 0) break;
            printf("Accepted from %s:%d\n",
                   inet_ntoa(cadr.sin_addr),ntohs(cadr.sin_port));

            // create a new lh_conn
            lh_conn_add(&clients, cl, NULL);
        }

        lh_conn_process(&clients, process_requests2);
        int *fds = lh_conn_cleanup(&clients);
        if (fds) {
            for(i=0; fds[i]>0; i++) {
                printf("Removing client fd=%d\n",fds[i]);
                close(fds[i]);
                lh_conn_remove(&clients, fds[i]);
            }
            free(fds);
        }
    }

    free(clients.r);
    free(clients.w);
    free(clients.e);
    
    free(server.r);
    free(server.w);
    free(server.e);
    
    free(pa.poll);
    free(pa.data);
    
    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}

#define TEST_DNS(name, addr) {                       \
        int f=0;                                     \
        f = (addr != dns_addr_ipv4(name));           \
        fail += f;                                   \
        printf("%s: %s\n", name, PASSFAIL(!f));      \
    }

int test_dns() {
    printf("\n\n====== Testing DNS functions ======\n");
    int fail = 0, f;

    TEST_DNS("0.0.0.0",0x00000000);
    TEST_DNS("127.0.0.1",0x7f000001);
    TEST_DNS("224.0.0.1",0xe0000001);
    TEST_DNS("255.255.255.252",0xfffffffc);
    TEST_DNS("255.255.255.255",0xffffffff);
    TEST_DNS("localhost",0x7f000001);
    TEST_DNS("washuu.no-ip.biz",0x58493c51);
    TEST_DNS("washuu.homeip.net",0xffffffff);
    TEST_DNS("slashdot.org",0xd822b52d);

    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}


#endif

#define iss(m) printf("%-8s : %d\n", #m, m(st.st_mode))
#define sz(path) printf("%s : %jd\n", path, lh_filesize_path(path))

void testshit() {

#if 0
    struct stat st;
    //CLEAR(st);

    int ss = lh_listen_tcp4_local(23456);
    int res = fstat(ss, &st);

    if (res < 0) {
        printf("stat on the server socket failed : %d %s\n", errno, strerror(errno));
        return;
    }
    
    printf("stat on the server socket succeeded: st_mode = %08x st_ino=%d st_dev=%d\n"
           "st_size=%jd\n",st.st_mode,st.st_ino,st.st_dev,(intmax_t)st.st_size);

    iss(S_ISREG);
    iss(S_ISDIR);
    iss(S_ISCHR);
    iss(S_ISBLK);
    iss(S_ISFIFO);
    iss(S_ISLNK);
    iss(S_ISSOCK);

    close(ss);
#endif

    sz("Makefile");
    sz("/dev/urandom");
    sz("/dev/sda");
    sz("/dev/sda1");
    sz(".");
    sz("derp");
    sz("does not exist");    
}


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
    fail += test_multiarrays();
    fail += test_bprintf();
    */
    
    /*
    //// lh_bytes.h
    fail += test_bswap();
    fail += test_stream();
    fail += test_wstream();
    fail += test_unpack();
    */

    //// lh_files.h
    //fail += test_files();
    
    //// lh_net.h
    //// lh_event.h
    //test_event();
    //test_event2();
    //test_dns();

    //lh_dirwalk_test(av[1]?av[1]:".");

    testshit();

    //test_compression();
    //test_image2();
    //test_image_resize();

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

