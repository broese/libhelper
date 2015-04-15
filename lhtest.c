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

#define LH_DECLARE_SHORT_NAMES 1

#include "lh_buffers.h"
#include "lh_arr.h"
#include "lh_marr.h"
#include "lh_strings.h"
#include "lh_bytes.h"
#include "lh_debug.h"
#include "lh_files.h"
#include "lh_net.h"
#include "lh_compress.h"
#include "lh_dir.h"
#include "lh_event.h"
#include "lh_image.h"



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

#define PASSFAIL(cond) ( (cond) ? "\x1b[32mPASS\x1b[0m" : "\x1b[31mFAIL\x1b[0m" )

char testdir[PATH_MAX];










////////////////////////////////////////////////////////////////////////////////
///// lh_buffers.h

// alignment macro

#define TESTALIGN(n,a,r)                                           \
    printf("align (%2d bits) %ju,%ju => %ju (%s)\n",               \
           (int)sizeof(n)*8,                                       \
           (uintmax_t)(n), (uintmax_t)a, (uintmax_t)lh_align(n,a), \
           PASSFAIL(lh_align(n,a)==(r))                            \
          );                                                       \
    if (lh_align(n,a)!=(r)) fail++;


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
    TESTALIGN(0x100000001LL,0x80000000,0x180000000LL);
    TESTALIGN(0x100000001LL,0x100000000LL,0x200000000LL);

    TESTALIGN(0xFFFFFFEDLL,0x1000,0x100000000LL);
    TESTALIGN(0xFFFFFFED,0x1000,0);

    uint64_t a = 0xFFFFFFEDLL;
    TESTALIGN(a,0x1000,0x100000000LL);
    uint32_t b = 0xFFFFFFEDLL;
    TESTALIGN(b,0x1000,0);
    uint16_t c = 0xFFED;
    TESTALIGN(c,0x1000,0);
    uint8_t d = 0xFD;
    TESTALIGN(d,0x10,0);

    TESTALIGN(0x100001234LL,0x1000,0x100002000LL);
    TESTALIGN(0x100001234LL,0x100,0x100001300LL);
    TESTALIGN(0x100001234LL,0x10,0x100001240LL);

    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}

// clearing in plain arrays/buffers

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

    lh_clear_obj(str);
    TEST_CLEAR("lh_clear_obj",str,6,0);

    lh_clear_ptr(s1);
    TEST_CLEAR("lh_clear_ptr",s1,1,0);

    lh_clear_num(s2,4);
    TEST_CLEAR("lh_clear_num",s2,4,0);
    TEST_CLEAR("lh_clear_num",s2+4,2,1);

    lh_clear_range(s3,3,7);
    TEST_CLEAR("lh_clear_range",s3,3,1);
    TEST_CLEAR("lh_clear_range",s3+3,7,0);
    TEST_CLEAR("lh_clear_range",s3+10,6,1);

    free(s1);
    free(s2);
    free(s3);

    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}

// allocation of arrays/buffers

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
    
    lh_create_obj(vertex,v);
    TEST_ALLOC("lh_create_obj",v,sizeof(vertex));

    lh_create_buf(buf,100);
    TEST_ALLOC("lh_create_buf",buf,100);

    lh_create_num(face,fc,30);
    TEST_ALLOC("lh_create_num",fc,sizeof(face)*30);

    free(v);
    free(fc);
    free(buf);

    lh_alloc_obj(v);
    TEST_ALLOC("lh_alloc_obj",v,sizeof(vertex));

    lh_alloc_num(fc,80);
    TEST_ALLOC("lh_alloc_num",fc,sizeof(face)*80);

    lh_alloc_buf(buf,10000);
    TEST_ALLOC("lh_alloc_buf",buf,10000);

    free(v);
    free(fc);
    free(buf);

    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}









////////////////////////////////////////////////////////////////////////////////
///// lh_arr.h

#define TEST_ARRAY(title,name,check) {                          \
        int sum=0,i,f=0;                                        \
        for(i=0; i<C(name); i++) sum += (int)P(name)[i];        \
        f = (sum != (check));                                   \
        fail += f;                                              \
        printf("%s: %s\n", title, PASSFAIL(!f));                \
    }
        

int test_arrays() {
    printf("\n\n====== Testing expandable arrays ======\n");
    int fail = 0;

    int i,s;

    BUFI(buf);
    lh_arr_allocate(GAR(buf),200);

    for(i=0; i<C(buf); i++) P(buf)[i] = (uint8_t)i;
    TEST_ARRAY("lh_arr_allocate",buf,199*C(buf)/2);

    for(i=200; i<1000; i++) {
        lh_arr_add(GAR(buf),1);
        P(buf)[i] = (uint8_t)i;
    }
    for(i=0,s=0; i<1000; i++) s+=(uint8_t)i;
    TEST_ARRAY("lh_arr_add",buf,s);

    *lh_arr_new(GAR(buf)) = 0x55;
    s+=0x55;
    TEST_ARRAY("lh_arr_new",buf,s);

    *lh_arr_insert(GAR(buf),900) = 0xAA;
    s+=0xAA;
    TEST_ARRAY("lh_arr_insert",buf,s);

    lh_arr_delete(GAR(buf),700);
    lh_arr_delete(GAR(buf),500);
    lh_arr_delete(GAR(buf),300);
    s -= (700%256)+(500%256)+(300%256);
    TEST_ARRAY("lh_array_delete",buf,s);

    lh_arr_delete_range(GAR(buf),100,5);
    s -= 100+101+102+103+104;
    TEST_ARRAY("lh_array_delete_range", buf,s);

    lh_arr_free(AR(buf));
    
    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}









////////////////////////////////////////////////////////////////////////////////
///// lh_marr.h

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
        printf("%2d  %c %04x %s\n",i,x->status[i],x->flags[i],
               x->name[i]?x->name[i]:"<undef>");
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
///// lh_strings.h


// bprintf
int test_bprintf() {
    printf("\n\n====== Testing bprintf ======\n");
    int fail = 0, f;

    BUFI(test);

    int res;
    res = bprintf(AR(test),"Hello World! ptr=%p len=%zd (%s:%d)\n",AR(test),__func__,__LINE__);
    printf("res=%d len=%zd\n",res,C(test));
    hexdump(AR(test));
    res = bprintf(AR(test),"Hello World! ptr=%p len=%zd (%s:%d)\n",AR(test),__func__,__LINE__);
    printf("res=%d len=%zd\n",res,C(test));
    hexdump(AR(test));
    res = bprintf(AR(test),"Hello World! ptr=%p len=%zd (%s:%d)\n",AR(test),__func__,__LINE__);
    printf("res=%d len=%zd\n",res,C(test));
    hexdump(AR(test));

    C(test) = LH_BUFPRINTF_GRAN;
    res = bprintf(AR(test),"Hello World! ptr=%p len=%zd (%s:%d)\n",AR(test),__func__,__LINE__);
    printf("res=%d len=%zd\n",res,C(test));
    hexdump(AR(test));

    C(test) = 512;
    res = bprintf(AR(test),"Hello World! ptr=%p len=%zd (%s:%d)\n",AR(test),__func__,__LINE__);
    printf("res=%d len=%zd\n",res,C(test));
    hexdump(AR(test));
    
    lh_arr_free(AR(test));
    
    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}







////////////////////////////////////////////////////////////////////////////////
///// lh_bytes.h

// byte swapping
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


// stream parsing
#define TEST_PARSE(type,val,check) {                                    \
        f = (val != check);                                             \
        fail += f;                                                      \
        printf("%s: %s   %016jx %016jx\n", "lh_" #type, PASSFAIL(!f),(uint64_t)val,(uint64_t)check); \
    }

#define Rx(type,var) lh_lread_ ##type##_be(ptr,lim,var, { atlimit=1; break; })
#define Rchar(var)  Rx(char,var)
#define Rshort(var) Rx(short,var)
#define Rint(var)   Rx(int,var)
#define Rlong(var)  Rx(long,var)

#define TEST_LREAD(len,rlen,fits,v_rc,v_rs,v_ri,v_rl)        \
    ptr = buf;                                               \
    lim = ptr+len;                                           \
    atlimit=0;                                               \
    {                                                        \
    uint8_t rc;                                              \
    uint16_t rs;                                             \
    uint32_t ri;                                             \
    uint64_t rl;                                             \
    if ( lh_lread_char(ptr,lim,rc) &&                        \
         lh_lread_short_be(ptr,lim,rs) &&                    \
         lh_lread_int_be(ptr,lim,ri) &&                      \
         lh_lread_long_be(ptr,lim,rl) )                      \
        f += (!fits);                                        \
    else                                                     \
        f += fits;                                           \
    }                                                        \
    f += (ptr != buf+rlen);                                  \
    fail += f;                                               \
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

    uint8_t vc = lh_parse_char(buf);
    TEST_PARSE(parse_char,vc,0x12);

    uint16_t vs = lh_parse_short_be(buf);
    TEST_PARSE(parse_short_be,vs,0x1234);
    uint32_t vi = lh_parse_int_be(buf);
    TEST_PARSE(parse_int_be,vi,0x12345678);
    uint64_t vl = lh_parse_long_be(buf);
    TEST_PARSE(parse_long_be,vl,0x123456789ABCDEF0LL);

    vs = lh_parse_short_le(buf);
    TEST_PARSE(parse_short_le,vs,0x3412);
    vi = lh_parse_int_le(buf);
    TEST_PARSE(parse_int_le,vi,0x78563412);
    vl = lh_parse_long_le(buf);
    TEST_PARSE(parse_long_le,vl,0xF0DEBC9A78563412LL);

    ////////////////////////////////////////////////////////////////////////////

    uint8_t *ptr = buf;

    vc = lh_read_char(ptr);
    TEST_PARSE(read_char,vc,0x12);

    vs = lh_read_short_be(ptr);
    TEST_PARSE(read_short_be,vs,0x3456);

    vi = lh_read_int_be(ptr);
    TEST_PARSE(read_int_be,vi,0x789abcde);

    vl = lh_read_long_be(ptr);
    TEST_PARSE(read_long_be,vl,0xf0123456789abcdeLL);

    vs = lh_read_short_le(ptr);
    TEST_PARSE(read_short_be,vs,0x12f0);

    vi = lh_read_int_le(ptr);
    TEST_PARSE(read_int_be,vi,0x9A785634);

    vl = lh_read_long_le(ptr);
    TEST_PARSE(read_long_be,vl,0x9a78563412f0debcLL);

    ////////////////////////////////////////////////////////////////////////////

    uint8_t *lim;
    int atlimit;

    TEST_LREAD(15,15,1,0x12,0x3456,0x789abcde,0xf0123456789abcdeLL);
    TEST_LREAD(14,7,0,0x12,0x3456,0x789abcde,0xf0123456789abcdeLL);
    TEST_LREAD(7,7,0,0x12,0x3456,0x789abcde,0xf0123456789abcdeLL);
    TEST_LREAD(30,15,1,0x12,0x3456,0x789abcde,0xf0123456789abcdeLL);

    ////////////////////////////////////////////////////////////////////////////

    uint8_t v[] = { 0x00, 0x20, 0x80, 0x01, 0xff, 0x80, 0x01 };
    uint8_t *vp = v;

    vi = lh_read_varint(vp);
    TEST_PARSE(parse_varint,vi,0);
    vi = lh_read_varint(vp);
    TEST_PARSE(parse_varint,vi,0x20);
    vi = lh_read_varint(vp);
    TEST_PARSE(parse_varint,vi,0x80);
    vi = lh_read_varint(vp);
    TEST_PARSE(parse_varint,vi,0x407f);

    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}

typedef struct {
    short x,y,z;
    float speed;
    uint64_t  cat;
} test_t;

#if 0
// unpack
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
#endif

// stream writing
#define TEST_WSTREAM(func, buf1, buf2, len) {        \
        int f=0;                                     \
        f = (memcmp(buf1, buf2, len) != 0);          \
        fail += f;                                   \
        printf("%s: %s\n", #func, PASSFAIL(!f));     \
    }

int test_wstream() {
    printf("\n\n====== Testing bytestream writing ======\n");
    int fail = 0, f;

    uint8_t buf[512];
    CLEAR(buf);
    uint8_t *ptr;

    ptr = buf;
    lh_write_char(ptr,0x12);
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

// determining resulting varint size from a 32-bit value
#define TEST_VARINT_SIZE(func, v, s) {                               \
        int size = lh_varint_size(v);                                \
        f = (size!=s);                                               \
        fail += f;                                                   \
        printf("%s: %08x => %d %s\n", #func, v, s, PASSFAIL(!f));    \
    }

int test_varint_size() {
    printf("\n\n====== Testing varint_size ======\n");
    int fail = 0, f;

    TEST_VARINT_SIZE(varint_size,0,1);
    TEST_VARINT_SIZE(varint_size,0x7f,1);
    TEST_VARINT_SIZE(varint_size,0x80,2);
    TEST_VARINT_SIZE(varint_size,0x3fff,2);
    TEST_VARINT_SIZE(varint_size,0x4000,3);
    TEST_VARINT_SIZE(varint_size,0x1fffff,3);
    TEST_VARINT_SIZE(varint_size,0x200000,4);
    TEST_VARINT_SIZE(varint_size,0xfffffff,4);
    TEST_VARINT_SIZE(varint_size,0x10000000,5);
    TEST_VARINT_SIZE(varint_size,0xffffffff,5);

    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}







////////////////////////////////////////////////////////////////////////////////
///// lh_files.h

#ifdef HAVE_OPENSSL
#include <openssl/md5.h>
#else
#include <md5.h>
#endif

uint8_t pr_init[16] = { 0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,
                        0x01,0x23,0x45,0x67,0x89,0xAB,0xCD,0xEF,};
#define PRSIZE 1048576
#define PRNAME_NAME "testfile.dat"
char PRNAME[PATH_MAX];

int create_test_file() {
    sprintf(PRNAME, "%s/%s", testdir, PRNAME_NAME);

    lh_create_buf(seq,PRSIZE);
    int i;

    memcpy(seq,pr_init,16);
    for (i=16; i<PRSIZE; i+=16) {
#ifdef HAVE_OPENSSL
        MD5(seq+i-16,16,seq+i);
#else
        md5_calc(seq+i,seq+i-16,16);
#endif
    }

    return !(lh_save(PRNAME,seq,PRSIZE)==PRSIZE);
}

int check_data(uint8_t * data, ssize_t length, const char * hash) {
    uint8_t h[16],md[16];
    if (hex_import(hash, h, sizeof(h))!=16) return 1;

#ifdef HAVE_OPENSSL
    MD5(data,length,md);
#else
    md5_calc(md,data,length);
#endif
    return (memcmp(h,md,16)!=0);
}

#define TESTFILE(name,buf,size,hash) {               \
        int f=(size>=0)?check_data(buf,size,hash):1; \
        fail += f;                                   \
        printf("%s: %s\n", #name, PASSFAIL(!f));     \
    }

int test_files() {
    printf("\n\n====== Testing file reading ======\n");
    int fail = 0, f;

    fail += create_test_file();

    // static buffer

    lh_create_buf(buf,16*1024*1024);
    ssize_t sz;
    sz = lh_load_static(PRNAME,buf);
    fail += (sz != LH_FILE_INVALID);
    sz = lh_load_static(PRNAME,buf,1024);
    TESTFILE(lh_load_static,buf,1024,"7edaf7d57d2d166c0cb96788a447275d");
    sz = lh_load_static(PRNAME,buf,1024,1);
    TESTFILE(lh_load_static,buf,1024,"235f3d4e19505dd5b5de38cf8ed6cf5e");

    // allocated buffer
    uint8_t * abuf;
    sz = lh_load_alloc(PRNAME,&abuf);
    TESTFILE(lh_load_static,abuf,sz,"71D037712F43C91105D83A024A66ED41");
    free(abuf);

    sz = lh_load_alloc(PRNAME,&abuf,1020);
    TESTFILE(lh_load_static,abuf,sz,"24F524BEEA54D38EBF31B4B25DB07155");
    free(abuf);

    sz = lh_load_alloc(PRNAME,&abuf,1020,100);
    TESTFILE(lh_load_static,abuf,sz,"8DD008F7A0448D4A7DE19B4BD3CB6A9A");
    free(abuf);

    // reading from an fd
    int fd = lh_open_read(PRNAME, NULL);
    sz = lh_read_alloc(fd,&abuf,200,1000);
    TESTFILE(lh_read_alloc,abuf,sz,"496DEC308BDC12F96CA305BCD7E92D76");
    free(abuf);

    sz = lh_read_alloc(fd,&abuf,300);
    TESTFILE(lh_read_alloc,abuf,sz,"76BA99024678EC09641234DA9D6270D6");
    free(abuf);

    sz = lh_read_alloc(fd,&abuf,400);
    TESTFILE(lh_read_alloc,abuf,sz,"14BCF897E4075CE84CE27E07C5E9197B");
    free(abuf);

    sz = lh_read_alloc(fd,&abuf);
    TESTFILE(lh_read_alloc,abuf,sz,"74901AF51BA354304E479442139D261D");
    free(abuf);

    sz = lh_read_alloc(fd,&abuf,-1,2000);
    TESTFILE(lh_read_alloc,abuf,sz,"1FEA7309598182135BDDABE162C1F544");
    free(abuf);


    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}








////////////////////////////////////////////////////////////////////////////////
///// lh_net.h


// DNS

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
    TEST_DNS("this.host.does.not.exist.com",0xffffffff);
    TEST_DNS("slashdot.org",0xd822b52d);

    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}






////////////////////////////////////////////////////////////////////////////////
///// lh_compress.h


// gzip

int test_gzip() {
    printf("\n\n====== Testing compression ======\n");
    int fail = 0, f;

    fail += create_test_file();

    uint8_t * data;
    ssize_t len = lh_load_alloc(PRNAME, &data);
    if (len!=1048576) { fail++; goto fail; }

    ssize_t clen;
    uint8_t * cdata = lh_gzip_encode(data, len, &clen);
    //TESTFILE(lh_gzip_encode,cdata,clen,"ed0af770dbbb1165df5b72949ad053ea");
    ssize_t dlen;
    uint8_t * ddata = lh_gzip_decode(cdata, clen, &dlen);
    TESTFILE(lh_gzip_decode,ddata,dlen,"71D037712F43C91105D83A024A66ED41");

    free(cdata);
    free(ddata);

 fail:
    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}

// zlib

int test_zlib() {
    printf("\n\n====== Testing compression ======\n");
    int fail = 0, f;

    fail += create_test_file();

    uint8_t * data;
    ssize_t len = lh_load_alloc(PRNAME, &data);
    if (len!=1048576) { fail++; goto fail; }

    ssize_t clen;
    uint8_t * cdata = lh_zlib_encode(data, len, &clen);
    TESTFILE(lh_zlib_encode,cdata,clen,"99612935faaca1f8e3c5c9711ba42f81");
    ssize_t dlen;
    uint8_t * ddata = lh_zlib_decode(cdata, clen, &dlen);
    TESTFILE(lh_zlib_decode,ddata,dlen,"71D037712F43C91105D83A024A66ED41");

    free(cdata);
    free(ddata);

 fail:
    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}







////////////////////////////////////////////////////////////////////////////////
///// lh_dir

// dirwalk

int test_dirwalk() {
    printf("\n\n====== Testing dirwalk ======\n");
    int fail = 0, f;
    
    lh_dirwalk * dw = lh_dirwalk_create(".",LH_DW_DEFAULTS);

    lh_dwres dr;

    while(1) {
        int res = lh_dirwalk_next(dw,&dr);
        if (res < 0) {
            printf("ERROR\n");
            continue;
        }

        int i;
        for(i=0; i<dr.level; i++) printf(" ");
        char *codes = "*FD-S";

        char path[PATH_MAX+1];
        if (dr.path[0])
            sprintf(path, "%s/%s",dr.path,dr.name);
        else
            sprintf(path, "%s", dr.name);

        if (res)
            printf("%c %s\n",codes[dr.type],path);
        else {
            printf("*\n");
            break;
        }
    }

    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}








////////////////////////////////////////////////////////////////////////////////
///// lh_event.h

ssize_t process_requests(lh_conn *conn) {
    int linecount=0;
    int lastpos=-1;

    // find the location of the last newline in the buffer
    int i;
    uint8_t *ptr = P(conn->rbuf.data) + conn->rbuf.ridx;
    int lim = C(conn->rbuf.data) - conn->rbuf.ridx;
    for(i=0; i<lim; i++) {
        if (ptr[i] == 0x0a) {
            lastpos = i;
            linecount++; // count lines
        }
    }

    // write response
    uint8_t buf[4096];
    int slen = sprintf((char *)buf, "Test Server: received %d lines\n",linecount);
    lh_conn_write(conn, buf, slen);

    if (conn->status & CONN_STATUS_REMOTE_EOF) {
        slen = sprintf((char *)buf, "Good Bye\n");
        lh_conn_write(conn, buf, slen);
    }

    return lastpos+1;
}

#define GROUP_SERVER 0
#define GROUP_CLIENTS 1

int test_event() {
    printf("\n\n====== Testing event framework (L3) ======\n");
    int fail = 0, f;

    int ss = lh_listen_tcp4_any(23456);
    if (ss<0) {
        printf("-----\ntotal: %s\n", PASSFAIL(0));
        return 1;
    }

    lh_pollarray pa;
    lh_clear_obj(pa);

    lh_poll_add(&pa, ss, POLLIN, GROUP_SERVER, NULL);

    int stay = 1, i;
    int maxcount = 100;

    while(stay && --maxcount>0) {
        lh_poll(&pa, 1000);
        lh_polldata *pd;
        int pos=0;
        while ( (pd=lh_poll_getnext(&pa, &pos, GROUP_SERVER, POLLIN)) ) {
            // accept new connection
            struct sockaddr_in cadr;
            int cl = lh_accept_tcp4(pd->fd, &cadr);
            if (cl < 0) break;
            printf("Accepted from %s:%d\n",
                   inet_ntoa(cadr.sin_addr),ntohs(cadr.sin_port));

            // create a new lh_conn
            lh_conn_add(&pa, cl, GROUP_CLIENTS, NULL);
        }

        lh_conn_process(&pa, GROUP_CLIENTS, process_requests);
    }

    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}



////////////////////////////////////////////////////////////////////////////////
// lh_image.h


int test_png() {
    printf("\n\n====== Testing image handling ======\n");
    int fail = 0, f;

    char path[PATH_MAX], opath[PATH_MAX];
    sprintf(path, "%s/%s", testdir, "sheep.png");
    sprintf(opath, "%s/%s", testdir, "sheep_out.png");

    lhimage * img = import_png_file(path);
    if (!img) { fail ++; goto end; }
    printf("Loaded image %s, %dx%d (stride=%d)\n",
           path, img->width, img->height, img->stride);

    resize_image(img,50,50,10,10,0xff000000,-1);
    printf("Resized to: %dx%d (stride=%d)\n",
           img->width, img->height, img->stride);

    ssize_t sz = export_png_file(img, opath);
    if (sz<0) { fail++; }


 end:
    printf("-----\ntotal: %s\n", PASSFAIL(!fail));
    return fail;
}










////////////////////////////////////////////////////////////////////////////////

int test_module_buffers() {
    int fail=0;

    fail += test_align();
    fail += test_clear();
    fail += test_alloc();

    return fail;
}

int test_module_arr() {
    int fail=0;

    fail += test_arrays();

    return fail;
}

int test_module_marr() {
    int fail=0;

    fail += test_multiarrays();

    return fail;
}

int test_module_strings() {
    int fail=0;

    fail += test_bprintf();
    //fail += test_heximport();
    //fail += test_regexp();
    //fail += test_args();

    return fail;
}

int test_module_bytes() {
    int fail=0;

    fail += test_bswap();
    fail += test_stream();
    fail += test_wstream();
    //fail += test_unpack();
    fail += test_varint_size();

    return fail;
}

int test_module_files() {
    int fail=0;

    fail += test_files();

    return fail;
}

int test_module_net() {
    int fail=0;

    //fail += test_tcp();
    //fail += test_udp();
    fail += test_dns();

    return fail;
}

int test_module_compress() {
    int fail=0;

    fail += test_gzip();
    fail += test_zlib();

    return fail;
}

int test_module_dir() {
    int fail=0;

    fail += test_dirwalk();

    return fail;
}

int test_module_event() {
    int fail=0;

    fail += test_event();

    return fail;
}

int test_module_image() {
    int fail=0;

    fail += test_png();

    return fail;
}


////////////////////////////////////////////////////////////////////////////////


int main(int ac, char **av) {
    strcpy(testdir, av[1] ? av[1] : ".");

    int fail = 0;

    fail += test_module_buffers();
    fail += test_module_arr();
    fail += test_module_marr();
    fail += test_module_strings();
    fail += test_module_bytes();
    fail += test_module_files();
    fail += test_module_net();
    fail += test_module_compress();
    fail += test_module_dir();
    //fail += test_module_event();
    fail += test_module_image();

    printf("========== TOTAL: %s ==========\n", PASSFAIL(!fail));

    return 0;
}

