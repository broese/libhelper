#pragma once

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
*/
// calculate the number of remaining elements from 'num' to the next 'gran' boundary
#define GRANREST(num,gran) ((gran)-((unsigned)num))%(gran)
// calculate the next 'gran' boundary after 'num'
#define GRANSIZE(num,gran) ((num)+GRANREST(num,gran))



////////////////////////////////////////////////////////////////////////////////
// Clearing macros

// clear a single object (pointer)
#define CLEARP(ptr) CLEARN(ptr,1)

// clear a single object (non-pointer)
#define CLEAR(obj) CLEARP(&(obj))

// clear a set of objects (pointer)
#define CLEARN(ptr,N) memset((ptr), 0, sizeof(*(ptr))*(N))

// clear a number 'num' elements in the array
#define CLEAR_RANGE(ptr, from, num) if ( num > 0 ) CLEARN(ptr+from, num)


////////////////////////////////////////////////////////////////////////////////
// Allocation macros

// all allocation macros will clear the resulting array buffer or object

// ALLOC, ALLOCN, ALLOCB - allocate and place in a new pointer variable

// allocate single element of 'type'
#define ALLOC(type,name) ALLOCN(type,name,1)

// allocate a byte buffer of 'size' bytes
#define ALLOCB(name,size) ALLOCN(unsigned char,name,size)

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
// Sorting macros

//#define SORT(ptr, cnt, elem) qsort(ptr, cnt, sizeof(*ptr), 



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
#define ARRAY(type,ptr,cnt) type * ptr; int cnt;

// allocate array with 'num' elements, place the pointer to 'ptr' and
// element count to 'cnt'
#define ARRAY_ALLOCG(ptr,cnt,num,gran) {                                \
        ALLOCNE(ptr,GRANSIZE(num,gran));                                \
        cnt=num;                                                        \
    }

// extend the allocated array to 'num' elements
// can be used for downsizing the array as well
#define ARRAY_EXTENDG(ptr,cnt,num,gran) {                               \
        if (GRANSIZE(num,gran) > GRANSIZE(cnt,gran))                    \
            RESIZE(ptr,GRANSIZE(num,gran));                             \
        if (GRANSIZE(num,gran) > cnt)                                   \
            CLEAR_RANGE(ptr, cnt, GRANSIZE(num,gran)-cnt);              \
        cnt=num;                                                        \
    }

// add 'num' elements
#define ARRAY_ADDG(ptr,cnt,num,gran)     ARRAY_EXTENDG(ptr,cnt,cnt+num,gran)

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
// Extraction of values from a byte stream

#define PUTF *t++ = *p++
#define PUTR *t-- = *p++

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
    PUTR; PUTR;
#else
    unsigned char *t = &temp.buf[0];
    PUTF; PUTF;
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
    PUTR; PUTR;
    PUTR; PUTR;
#else
    unsigned char *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
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
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
#else
    unsigned char *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
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
    PUTF; PUTF;
#else
    unsigned char *t = &temp.buf[1];
    PUTR; PUTR;
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
    PUTF; PUTF;
    PUTF; PUTF;
#else
    unsigned char *t = &temp.buf[3];
    PUTR; PUTR;
    PUTR; PUTR;
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
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
#else
    unsigned char *t = &temp.buf[7];
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
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
    PUTR; PUTR;
    PUTR; PUTR;
#else
    unsigned char *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
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
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
#else
    unsigned char *t = &temp.buf[0];
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
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
    PUTF; PUTF;
    PUTF; PUTF;
#else
    unsigned char *t = &temp.buf[3];
    PUTR; PUTR;
    PUTR; PUTR;
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
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
    PUTF; PUTF;
#else
    unsigned char *t = &temp.buf[7];
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
    PUTR; PUTR;
#endif
    return temp.val;
}

#define read_float_le(p)   parse_float_le(p);   p+=4
#define read_double_le(p)  parse_double_le(p);  p+=8

