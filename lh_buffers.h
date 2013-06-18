#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/*
lh_buffers - macros for handling buffers and arrays,
as well as generic preprocessor macros
*/

////////////////////////////////////////////////////////////////////////////////
// Generic macros

/*
determine the length of the __VA_ARGS__ list. This macro only works for up to 16
elements. This can be extended to up to 63 elements before you hit the
preprocessor own limit. This also does not work for the empty list - returns 1
*/

#define VA_LENGTH_16(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,N,...) N
#define VA_LENGTH(...) VA_LENGTH_16(__VA_ARGS__,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)

/*
helper macros used in calculation of granular sizes
//FIXME: these are limited to 32 bit
*/
// calculate the number of remaining elements from 'num' to the next 'gran' boundary
#define GRANREST(num,gran) ((gran)-((unsigned)(num)))%(gran)
// calculate the next 'gran' boundary after 'num'
#define GRANSIZE(num,gran) ((num)+GRANREST(num,gran))



////////////////////////////////////////////////////////////////////////////////
// Clearing macros

// clear a single object (non-pointer)
#define CLEAR(obj) CLEARP(&(obj))

// clear a single object (pointer)
#define CLEARP(ptr) CLEARN(ptr,1)

// clear an array of N objects (pointer)
#define CLEARN(ptr,N) memset((ptr), 0, sizeof(*(ptr))*(N))

// clear a number 'num' elements in the array starting with 'from'
#define CLEAR_RANGE(ptr, from, num) if ( num > 0 ) CLEARN(ptr+from, num)



////////////////////////////////////////////////////////////////////////////////
// Allocation macros

// all allocation macros will clear the resulting array buffer or object

// ALLOC, ALLOCN, ALLOCB - allocate and place in a new pointer variable

// allocate single element of 'type'
#define ALLOC(type,name) ALLOCN(type,name,1)

// allocate a byte buffer of 'size' bytes
#define ALLOCB(name,size) ALLOCN(uint8_t,name,size)

// allocate 'num' elements of 'type'
#define ALLOCN(type,name,num) type * ALLOCNE(name,num)

// ALLOCE, ALLOCNE, ALLOCBE - allocate and place in an existing variable

// allocate single element of 'type'
#define ALLOCE(ptr) ALLOCNE(ptr,1)

// allocate a byte buffer of 'size' bytes
#define ALLOCBE(ptr,size) ALLOCNE(ptr,size)

// allocate 'num' elements of 'type'
#define ALLOCNE(ptr,num)                        \
    ptr = malloc((num)*sizeof(*ptr));           \
    CLEARN(ptr,num);



////////////////////////////////////////////////////////////////////////////////
// Expandable arrays and buffers

/*
The expandable arrays are defined via variable 'ptr' holding the pointer to data
and 'cnt' - an integer variable holding the count of elements. The macros
perform allocating and resizing of these arrays. Allocation granularity can be
additionally specified in the granular versions of the macros
*/

// helper wrapper macro for realloc();
#define RESIZE(ptr, num) ptr = realloc(ptr, num*sizeof(*ptr));

// wrapped macro for declaration of such arrays
#define ARRAY(type,ptr,cnt) type * ptr=NULL; int cnt=0;
#define BUFFER(ptr,cnt) uint8_t * ptr=NULL; ssize_t cnt=0;

// allocate array with 'num' elements, place the pointer to 'ptr' and
// element count to 'cnt'
#define ARRAY_ALLOCG(ptr,cnt,num,gran) {                                \
        ALLOCNE(ptr,GRANSIZE(num,gran));                                \
        cnt=num;                                                        \
    }

// extend the allocated array to 'num' elements
// can be used for downsizing the array as well
#define ARRAY_EXTENDG(ptr,cnt,num,gran) {                               \
        if (GRANSIZE((num),(gran)) > GRANSIZE((cnt),(gran)))            \
            RESIZE((ptr),GRANSIZE((num),(gran)));                       \
        if (GRANSIZE((num),gran) > (cnt))                               \
            CLEAR_RANGE((ptr), (cnt), (num)-(cnt));                     \
        cnt=num;                                                        \
    }
//            CLEAR_RANGE(ptr, cnt, GRANSIZE(num,gran)-(cnt));  \

// add 'num' elements
#define ARRAY_ADDG(ptr,cnt,num,gran)     ARRAY_EXTENDG(ptr,cnt,((cnt)+(num)),gran)

// non-granular allocation - wrapper macros
#define ARRAY_ALLOC(ptr,cnt,num)         ARRAY_ALLOCG(ptr,cnt,num,1)
#define ARRAY_EXTEND(ptr,cnt,num)        ARRAY_EXTENDG(ptr,cnt,num,1)
#define ARRAY_ADD(ptr,cnt,num)           ARRAY_ADDG(ptr,cnt,num,1)

// delete a range of 'num' elements starting from position 'from'
// the following elements are moved to position 'from'
// the freed space at the end is zeroed
// the array is resized but not reallocated
#define ARRAY_DELETE_RANGE(ptr, cnt, from, num) {                       \
        memcpy((ptr)+(from), (ptr)+(from)+(num),                        \
               ((cnt)-(num)-(from))*sizeof(*ptr));                      \
        cnt -= num;                                                     \
        CLEAR_RANGE(ptr,cnt,num);                                       \
    }                                                                   

// delete a single element at position 'idx'
#define ARRAY_DELETE(ptr, cnt, idx) ARRAY_DELETE_RANGE(ptr, cnt, idx, 1)

// macro versions that do not update cnt
#define ARRAY_DELETE_RANGE_NU(ptr, cnt, from, num) {                       \
        memcpy((ptr)+(from), (ptr)+(from)+(num),                        \
               ((cnt)-(num)-(from))*sizeof(*ptr));                      \
        CLEAR_RANGE(ptr,(cnt)-(num),num);                               \
    }                                                                   

// delete a single element at position 'idx'
#define ARRAY_DELETE_NU(ptr, cnt, idx) ARRAY_DELETE_RANGE_NU(ptr, cnt, idx, 1)



////////////////////////////////////////////////////////////////////////////////
// Expandable multi-arrays

/*
The concept is similar to an expandable array, but there are multiple
arrays of the same length. So, we have multiple pointers to data, with 
potentially different sizeof-s of their elements, but with a commont
integer counter for the number of elements and same granularity.
The special macro is capable of simultaneously resizing a semi-arbitrary
list of pointer names. Technical limitation - their number must be between
1 and 8, inclusive.
TODO: resizing does not zero the new elements! Make it similar to ARRAY_EXTEND
*/

#define RESIZE_1(oldnum,num,ptr) RESIZE(ptr,num); CLEARN(ptr+oldnum,num-oldnum);
#define RESIZE_2(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_1(oldnum,num,__VA_ARGS__)
#define RESIZE_3(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_2(oldnum,num,__VA_ARGS__)
#define RESIZE_4(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_3(oldnum,num,__VA_ARGS__)
#define RESIZE_5(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_4(oldnum,num,__VA_ARGS__)
#define RESIZE_6(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_5(oldnum,num,__VA_ARGS__)
#define RESIZE_7(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_6(oldnum,num,__VA_ARGS__)
#define RESIZE_8(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_7(oldnum,num,__VA_ARGS__)
#define RESIZE_9(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_8(oldnum,num,__VA_ARGS__)
#define RESIZE_10(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_9(oldnum,num,__VA_ARGS__)
#define RESIZE_11(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_10(oldnum,num,__VA_ARGS__)
#define RESIZE_12(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_11(oldnum,num,__VA_ARGS__)
#define RESIZE_13(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_12(oldnum,num,__VA_ARGS__)
#define RESIZE_14(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_13(oldnum,num,__VA_ARGS__)
#define RESIZE_15(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_14(oldnum,num,__VA_ARGS__)
#define RESIZE_16(oldnum,num,ptr,...) RESIZE_1(oldnum,num,ptr); RESIZE_15(oldnum,num,__VA_ARGS__)

#define RESIZE_N_(oldnum,num,N,...) RESIZE_ ## N(oldnum,num,__VA_ARGS__)
#define RESIZE_N(oldnum,num,N,...) RESIZE_N_(oldnum,num,N,__VA_ARGS__)

#define ARRAYS_EXTENDG(cnt,num,gran,...) do {                           \
        int ARRAYS_EXTEND_SIZE = GRANSIZE(num,gran);                    \
        if (ARRAYS_EXTEND_SIZE > GRANSIZE(cnt,gran)) {                  \
            RESIZE_N(cnt, ARRAYS_EXTEND_SIZE, VA_LENGTH(__VA_ARGS__) ,__VA_ARGS__);     \
        }                                                               \
        cnt = num;                                                      \
    } while(0);

#define ARRAYS_EXTEND(cnt,num,...)      ARRAYS_EXTENDG(cnt,num,1,__VA_ARGS__)
#define ARRAYS_ADDG(cnt,inc,gran,...)   ARRAYS_EXTENDG(cnt,cnt+inc,gran,__VA_ARGS__)
#define ARRAYS_ADD(cnt,inc,...)         ARRAYS_EXTENDG(cnt,cnt+inc,1,__VA_ARGS__)

//TODO: ARRAYS_DELETE


////////////////////////////////////////////////////////////////////////////////
// Sorting macros


#ifdef __linux

#define SORTF_(name,type,op)                                            \
    static int SORTF_##name                                             \
    (const void * a, const void * b, void * arg) {                      \
        size_t offset = ((size_t)arg) & 0xffffff;                       \
        size_t size   = (size_t)arg >> 24;                              \
        switch (size) {                                                 \
        case 1: {                                                       \
            type ## int8_t * A = (type ## int8_t *)(((char *)a)+offset); \
            type ## int8_t * B = (type ## int8_t *)(((char *)b)+offset); \
            return op ((*A < *B) ? -1 : (*B < *A));                     \
        }                                                               \
        case 2: {                                                       \
            type ## int16_t * A = (type ## int16_t *)(((char *)a)+offset); \
            type ## int16_t * B = (type ## int16_t *)(((char *)b)+offset); \
            return op ((*A < *B) ? -1 : (*B < *A));                     \
        }                                                               \
        case 4: {                                                       \
            type ## int32_t * A = (type ## int32_t *)(((char *)a)+offset); \
            type ## int32_t * B = (type ## int32_t *)(((char *)b)+offset); \
            return op ((*A < *B) ? -1 : (*B < *A));                     \
        }                                                               \
        case 8: {                                                       \
            type ## int64_t * A = (type ## int64_t *)(((char *)a)+offset); \
            type ## int64_t * B = (type ## int64_t *)(((char *)b)+offset); \
            return op ((*A < *B) ? -1 : (*B < *A));                     \
        }                                                               \
        default:                                                        \
            printf("Unsipported size %zd\n",size);                      \
        }                                                               \
    }

SORTF_(SI,,)
SORTF_(UI,u,)
SORTF_(SD,,-)
SORTF_(UD,u,-)


#if 0
static int SORTF_1S_INC(const void * a, const void * b, void * arg) {
    long offset = (long)arg;
    char * A = ((char *)a)+offset;
    char * B = ((char *)b)+offset;
    return (*A < *B) ? -1 : ( *A > *B);
}
#endif

#define SORT_(ptr, cnt, offset, size, suffix) qsort_r(ptr, cnt, sizeof(*ptr), SORTF_ ## suffix, (void *)((((size)&0xff)<<24)+((offset)&0xffffff)))
#define SORT(ptr, cnt, elem) SORT_(ptr, cnt, ((char *)&(elem)-(char *)(ptr)), sizeof(elem), SI)
#define SORTD(ptr, cnt, elem) SORT_(ptr, cnt, ((char *)&(elem)-(char *)(ptr)), sizeof(elem), SD)
#define SORTU(ptr, cnt, elem) SORT_(ptr, cnt, ((char *)&(elem)-(char *)(ptr)), sizeof(elem), UI)
#define SORTUD(ptr, cnt, elem) SORT_(ptr, cnt, ((char *)&(elem)-(char *)(ptr)), sizeof(elem), UD)

#else

#define SORT(ptr, cnt, elem) printf("qsort_r not available\n");

#endif


////////////////////////////////////////////////////////////////////////////////
// Extraction of values from a byte stream

#define GETF *t++ = *p++
#define GETR *t-- = *p++

static inline int8_t parse_char(const char *p) {
    return *p;
}

// Big-Endian extraction (aka network order)

static inline int16_t parse_short(const char *p) {
    union {
        char buf[2];
        int16_t val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[1];
    GETR; GETR;
#else
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
#endif
    return temp.val;
}

static inline int32_t parse_int(const char *p) {
    union {
        char buf[4];
        int32_t val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[3];
    GETR; GETR;
    GETR; GETR;
#else
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
    GETF; GETF;
#endif
    return temp.val;
}

static inline int64_t parse_long(const char *p) {
    union {
        char buf[8];
        int64_t val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[7];
    GETR; GETR;
    GETR; GETR;
    GETR; GETR;
    GETR; GETR;
#else
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
    GETF; GETF;
    GETF; GETF;
    GETF; GETF;
#endif
    return temp.val;
}

#define read_char(p)  parse_char(p);  (p)++
#define read_short(p) parse_short(p); (p)+=2
#define read_int(p)   parse_int(p);   (p)+=4
#define read_long(p)  parse_long(p);  (p)+=8

// Little-Endian extraction (aka Intel order)

static inline int16_t parse_short_le(const char *p) {
    union {
        char buf[2];
        int16_t val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
#else
    unsigned char *t = &temp.buf[1];
    GETR; GETR;
#endif
    return temp.val;
}

static inline int32_t parse_int_le(const char *p) {
    union {
        char buf[4];
        int32_t val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
    GETF; GETF;
#else
    unsigned char *t = &temp.buf[3];
    GETR; GETR;
    GETR; GETR;
#endif
    return temp.val;
}

static inline int64_t parse_long_le(const char *p) {
    union {
        char buf[8];
        int64_t val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
    GETF; GETF;
    GETF; GETF;
    GETF; GETF;
#else
    unsigned char *t = &temp.buf[7];
    GETR; GETR;
    GETR; GETR;
    GETR; GETR;
    GETR; GETR;
#endif
    return temp.val;
}

#define read_char_le(p)  parse_char(p);     (p)++
#define read_short_le(p) parse_short_le(p); (p)+=2
#define read_int_le(p)   parse_int_le(p);   (p)+=4
#define read_long_le(p)  parse_long_le(p);  (p)+=8

////////////////////////////////////////////////////////////////////////////////

static inline float parse_float(const char *p) {
    union {
        char buf[4];
        float val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[3];
    GETR; GETR;
    GETR; GETR;
#else
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
    GETF; GETF;
#endif
    return temp.val;
}

static inline double parse_double(const char *p) {
    union {
        char buf[8];
        double val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[7];
    GETR; GETR;
    GETR; GETR;
    GETR; GETR;
    GETR; GETR;
#else
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
    GETF; GETF;
    GETF; GETF;
    GETF; GETF;
#endif
    return temp.val;
}

#define read_float(p)   parse_float(p);   p+=4
#define read_double(p)  parse_double(p);  p+=8

////////////////////////////////////////////////////////////////////////////////

static inline float parse_float_le(const char *p) {
    union {
        char buf[4];
        float val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
    GETF; GETF;
#else
    unsigned char *t = &temp.buf[3];
    GETR; GETR;
    GETR; GETR;
#endif
    return temp.val;
}

static inline double parse_double_le(const char *p) {
    union {
        char buf[8];
        double val;
    } temp;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned char *t = &temp.buf[0];
    GETF; GETF;
    GETF; GETF;
    GETF; GETF;
    GETF; GETF;
#else
    unsigned char *t = &temp.buf[7];
    GETR; GETR;
    GETR; GETR;
    GETR; GETR;
    GETR; GETR;
#endif
    return temp.val;
}

#define read_float_le(p)   parse_float_le(p);   p+=4
#define read_double_le(p)  parse_double_le(p);  p+=8

////////////////////////////////////////////////////////////////////////////////
// Writing to a stream

#define PUTF *p++ = *t++
#define PUTR *p++ = *t--

static inline char * place_char(char *p, int8_t v) {
    *p++ = v;
    return p;
}

static inline char * place_short(char *p, int16_t v) {
    union {
        int8_t  buf[2];
        int16_t val;
    } temp;

    temp.val = v;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    int8_t *t = &temp.buf[1];
    PUTR; PUTR;
#else
    int8_t *t = &temp.buf[0];
    PUTF; PUTF;
#endif

    return p;
}

static inline char * place_int(char *p, int32_t v) {
    union {
        int8_t  buf[4];
        int32_t val;
    } temp;

    temp.val = v;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    int8_t *t = &temp.buf[3];
    PUTR; PUTR;
    PUTR; PUTR;
#else
    int8_t *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
#endif

    return p;
}

static inline char * place_long(char *p, int64_t v) {
    union {
        int8_t  buf[8];
        int64_t val;
    } temp;

    temp.val = v;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    int8_t *t = &temp.buf[7];
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
#else
    int8_t *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
#endif

    return p;
}

#define write_char(p,v)  p=place_char(p,v)
#define write_short(p,v) p=place_short(p,v)
#define write_int(p,v)   p=place_int(p,v)
#define write_long(p,v)  p=place_long(p,v)


static inline uint8_t * place_float(char *p, float v) {
    union {
        uint8_t buf[4];
        float val;
    } temp;

    temp.val = v;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t *t = &temp.buf[3];
    PUTR; PUTR;
    PUTR; PUTR;
#else
    uint8_t *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
#endif
    return p;
}

static inline uint8_t * place_double(char *p, double v) {
    union {
        uint8_t buf[8];
        double val;
    } temp;

    temp.val = v;

#if __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t *t = &temp.buf[7];
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
#else
    uint8_t *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
#endif
    return p;
}

#define write_float(p,v)   place_float(p,v);   p+=4
#define write_double(p,v)  place_double(p,v);  p+=8

////////////////////////////////////////////////////////////////////////////////

static inline uint16_t swap_short(uint16_t v) {
    return (v>>8)|(v<<8);
    //return __builtin_bswap16(v);
}

static inline uint32_t swap_int(uint32_t v) {
    return __builtin_bswap32(v);
}

static inline uint64_t swap_long(uint64_t v) {
    return __builtin_bswap64(v);
}

static inline float swap_float(float v) {
    union {
        float vf;
        uint32_t vi;
    } S;
    S.vf = v;
    S.vi = swap_int(S.vi);
    return S.vf;
}

static inline double swap_double(double v) {
    union {
        double vd;
        uint64_t vl;
    } S;
    S.vd = v;
    S.vl = swap_long(S.vl);
    return S.vd;
}
